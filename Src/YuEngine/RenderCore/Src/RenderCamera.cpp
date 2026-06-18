// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Src/RenderCamera.cpp

#include "YuEngine/RenderCore/RenderCamera.h"

#include <cmath>
#include <cstddef>

namespace yuengine::rendercore {
namespace {
constexpr float MIN_CAMERA_EPSILON = 0.000001F;
constexpr float PI_VALUE = 3.14159265358979323846F;

bool IsFinite(float value) {
    return std::isfinite(value);
}

bool IsFiniteVector(const RenderCameraVector3 &value) {
    if (!IsFinite(value.x)) {
        return false;
    }

    if (!IsFinite(value.y)) {
        return false;
    }

    return IsFinite(value.z);
}

RenderCameraVector3 SubtractVector(const RenderCameraVector3 &left, const RenderCameraVector3 &right) {
    RenderCameraVector3 result{};
    result.x = left.x - right.x;
    result.y = left.y - right.y;
    result.z = left.z - right.z;
    return result;
}

float DotVector(const RenderCameraVector3 &left, const RenderCameraVector3 &right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

RenderCameraVector3 CrossVector(const RenderCameraVector3 &left, const RenderCameraVector3 &right) {
    RenderCameraVector3 result{};
    result.x = left.y * right.z - left.z * right.y;
    result.y = left.z * right.x - left.x * right.z;
    result.z = left.x * right.y - left.y * right.x;
    return result;
}

float VectorLength(const RenderCameraVector3 &value) {
    return std::sqrt(DotVector(value, value));
}

bool NormalizeVector(const RenderCameraVector3 &value, RenderCameraVector3 *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    const float length = VectorLength(value);
    if (length <= MIN_CAMERA_EPSILON) {
        return false;
    }

    out_value->x = value.x / length;
    out_value->y = value.y / length;
    out_value->z = value.z / length;
    return true;
}

RenderCameraMatrix MakeIdentityMatrix() {
    RenderCameraMatrix result{};
    result.values[0U] = 1.0F;
    result.values[5U] = 1.0F;
    result.values[10U] = 1.0F;
    result.values[15U] = 1.0F;
    return result;
}

RenderCameraMatrix MultiplyMatrix(const RenderCameraMatrix &left, const RenderCameraMatrix &right) {
    RenderCameraMatrix result{};
    for (std::size_t row = 0U; row < 4U; ++row) {
        for (std::size_t column = 0U; column < 4U; ++column) {
            float value = 0.0F;
            for (std::size_t index = 0U; index < 4U; ++index) {
                value += left.values[row * 4U + index] * right.values[index * 4U + column];
            }

            result.values[row * 4U + column] = value;
        }
    }

    return result;
}

RenderCameraStatus ValidateProjection(const RenderCameraProjectionDesc &projection) {
    if (!IsFinite(projection.aspect_ratio) || projection.aspect_ratio <= MIN_CAMERA_EPSILON) {
        return RenderCameraStatus::InvalidProjection;
    }

    if (!IsFinite(projection.near_z) || !IsFinite(projection.far_z)) {
        return RenderCameraStatus::InvalidProjection;
    }

    if (projection.near_z <= MIN_CAMERA_EPSILON || projection.far_z <= projection.near_z) {
        return RenderCameraStatus::InvalidProjection;
    }

    if (projection.kind == RenderCameraProjectionKind::Perspective) {
        if (!IsFinite(projection.vertical_fov_radians)) {
            return RenderCameraStatus::InvalidProjection;
        }

        if (projection.vertical_fov_radians <= MIN_CAMERA_EPSILON || projection.vertical_fov_radians >= PI_VALUE) {
            return RenderCameraStatus::InvalidProjection;
        }

        return RenderCameraStatus::Success;
    }

    if (projection.kind == RenderCameraProjectionKind::Orthographic) {
        if (!IsFinite(projection.orthographic_height)) {
            return RenderCameraStatus::InvalidProjection;
        }

        if (projection.orthographic_height <= MIN_CAMERA_EPSILON) {
            return RenderCameraStatus::InvalidProjection;
        }

        return RenderCameraStatus::Success;
    }

    return RenderCameraStatus::InvalidProjection;
}

RenderCameraStatus BuildViewMatrix(const RenderCameraPose &pose, RenderCameraMatrix *out_view) {
    if (out_view == nullptr) {
        return RenderCameraStatus::InvalidOutput;
    }

    if (!IsFiniteVector(pose.position) || !IsFiniteVector(pose.target) || !IsFiniteVector(pose.up)) {
        return RenderCameraStatus::InvalidPose;
    }

    RenderCameraVector3 forward{};
    const RenderCameraVector3 forward_source = SubtractVector(pose.target, pose.position);
    if (!NormalizeVector(forward_source, &forward)) {
        return RenderCameraStatus::InvalidPose;
    }

    RenderCameraVector3 up{};
    if (!NormalizeVector(pose.up, &up)) {
        return RenderCameraStatus::InvalidPose;
    }

    RenderCameraVector3 right{};
    const RenderCameraVector3 right_source = CrossVector(up, forward);
    if (!NormalizeVector(right_source, &right)) {
        return RenderCameraStatus::InvalidPose;
    }

    const RenderCameraVector3 corrected_up = CrossVector(forward, right);
    RenderCameraMatrix view = MakeIdentityMatrix();
    view.values[0U] = right.x;
    view.values[1U] = corrected_up.x;
    view.values[2U] = forward.x;
    view.values[4U] = right.y;
    view.values[5U] = corrected_up.y;
    view.values[6U] = forward.y;
    view.values[8U] = right.z;
    view.values[9U] = corrected_up.z;
    view.values[10U] = forward.z;
    view.values[12U] = -DotVector(right, pose.position);
    view.values[13U] = -DotVector(corrected_up, pose.position);
    view.values[14U] = -DotVector(forward, pose.position);
    *out_view = view;
    return RenderCameraStatus::Success;
}

RenderCameraMatrix BuildPerspectiveMatrix(const RenderCameraProjectionDesc &projection) {
    RenderCameraMatrix result{};
    const float y_scale = 1.0F / std::tan(projection.vertical_fov_radians * 0.5F);
    const float x_scale = y_scale / projection.aspect_ratio;
    const float z_scale = projection.far_z / (projection.far_z - projection.near_z);
    result.values[0U] = x_scale;
    result.values[5U] = y_scale;
    result.values[10U] = z_scale;
    result.values[11U] = 1.0F;
    result.values[14U] = -projection.near_z * z_scale;
    return result;
}

RenderCameraMatrix BuildOrthographicMatrix(const RenderCameraProjectionDesc &projection) {
    RenderCameraMatrix result = MakeIdentityMatrix();
    const float height = projection.orthographic_height;
    const float width = height * projection.aspect_ratio;
    result.values[0U] = 2.0F / width;
    result.values[5U] = 2.0F / height;
    result.values[10U] = 1.0F / (projection.far_z - projection.near_z);
    result.values[14U] = -projection.near_z / (projection.far_z - projection.near_z);
    return result;
}
}

RenderCameraStatus RenderCamera::BuildFrame(
    const RenderCameraPose &pose,
    const RenderCameraProjectionDesc &projection,
    RenderCameraFrame *out_frame) {
    if (out_frame == nullptr) {
        RecordStatus(RenderCameraStatus::InvalidOutput);
        return RenderCameraStatus::InvalidOutput;
    }

    RenderCameraFrame frame{};
    RenderCameraStatus status = BuildViewMatrix(pose, &frame.view);
    if (status != RenderCameraStatus::Success) {
        RecordStatus(status);
        return status;
    }

    status = ValidateProjection(projection);
    if (status != RenderCameraStatus::Success) {
        RecordStatus(status);
        return status;
    }

    if (projection.kind == RenderCameraProjectionKind::Perspective) {
        frame.projection = BuildPerspectiveMatrix(projection);
    }

    if (projection.kind == RenderCameraProjectionKind::Orthographic) {
        frame.projection = BuildOrthographicMatrix(projection);
    }

    frame.view_projection = MultiplyMatrix(frame.view, frame.projection);
    *out_frame = frame;
    RecordStatus(RenderCameraStatus::Success);
    return RenderCameraStatus::Success;
}

RenderCameraSnapshot RenderCamera::Snapshot() const {
    return snapshot_;
}

void RenderCamera::Reset() {
    snapshot_ = {};
}

void RenderCamera::RecordStatus(RenderCameraStatus status) {
    snapshot_.last_status = status;
    if (status == RenderCameraStatus::Success) {
        ++snapshot_.accepted_frame_count;
        return;
    }

    ++snapshot_.rejected_frame_count;
}
}
