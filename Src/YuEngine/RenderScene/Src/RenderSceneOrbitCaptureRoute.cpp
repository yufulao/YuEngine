// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderSceneOrbitCaptureRoute.cpp

#include "YuEngine/RenderScene/RenderSceneOrbitCaptureRoute.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

#include "YuEngine/RenderScene/RenderSceneCameraBindingRequest.h"
#include "YuEngine/RenderScene/RenderSceneCameraFrameBinder.h"
#include "YuEngine/RenderScene/RenderSceneStatus.h"

namespace yuengine::renderscene {
namespace {
constexpr float FULL_ORBIT_RADIANS = 6.28318530718F;
constexpr std::size_t FRAME_PATH_STEM_BYTE_COUNT = 6U;
constexpr std::size_t FRAME_PATH_DIGIT_BYTE_COUNT = 3U;
constexpr std::size_t FRAME_PATH_EXTENSION_BYTE_COUNT = 4U;
constexpr char FRAME_PATH_STEM[] = ".Frame";
constexpr char FRAME_PATH_EXTENSION[] = ".rvf";

bool IsFinite(float value) {
    return std::isfinite(value);
}

bool IsFiniteVector(const yuengine::rendercore::RenderCameraVector3 &value) {
    if (!IsFinite(value.x)) {
        return false;
    }

    if (!IsFinite(value.y)) {
        return false;
    }

    return IsFinite(value.z);
}

bool IsOrbitRequestUsable(const RenderSceneOrbitCaptureRequest &request) {
    if (request.frame_count <= 1U) {
        return false;
    }

    if (request.frame_count > MAX_RENDER_SCENE_ORBIT_CAPTURE_FRAME_COUNT) {
        return false;
    }

    if (!IsFiniteVector(request.target)) {
        return false;
    }

    if (!IsFinite(request.orbit_radius) || request.orbit_radius <= 0.0F) {
        return false;
    }

    return IsFinite(request.orbit_height);
}

bool IsOutputPathPrefixUsable(const RenderSceneOrbitCaptureRequest &request) {
    if (request.output_path_prefix == nullptr) {
        return false;
    }

    if (request.output_path_prefix_byte_count == 0U) {
        return false;
    }

    const std::size_t suffix_byte_count =
        FRAME_PATH_STEM_BYTE_COUNT +
        FRAME_PATH_DIGIT_BYTE_COUNT +
        FRAME_PATH_EXTENSION_BYTE_COUNT;
    const std::size_t output_path_byte_count =
        request.output_path_prefix_byte_count + suffix_byte_count;
    return output_path_byte_count < MAX_RENDER_SCENE_ORBIT_CAPTURE_OUTPUT_PATH_BYTES;
}

bool IsCaptureStorageUsable(const RenderSceneOrbitCaptureRequest &request) {
    if (request.capture_byte_budget_per_entity == 0U) {
        return false;
    }

    if (request.capture_output.data() == nullptr) {
        return false;
    }

    const std::size_t max_value = std::numeric_limits<std::size_t>::max();
    if (request.capture_byte_budget_per_entity > max_value / RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return false;
    }

    const std::size_t frame_capture_byte_budget =
        request.capture_byte_budget_per_entity * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    if (request.frame_count > max_value / frame_capture_byte_budget) {
        return false;
    }

    const std::size_t total_capture_byte_budget =
        frame_capture_byte_budget * static_cast<std::size_t>(request.frame_count);
    return request.capture_output.size() >= total_capture_byte_budget;
}

yuengine::rendercore::RenderCameraPose BuildOrbitPose(
    const RenderSceneOrbitCaptureRequest &request,
    float orbit_angle_radians) {
    yuengine::rendercore::RenderCameraPose pose{};
    pose.target = request.target;
    pose.up = {0.0F, 1.0F, 0.0F};

    const float orbit_sin = std::sin(orbit_angle_radians);
    const float orbit_cos = std::cos(orbit_angle_radians);
    pose.position.x = request.target.x + orbit_sin * request.orbit_radius;
    pose.position.y = request.target.y + request.orbit_height;
    pose.position.z = request.target.z - orbit_cos * request.orbit_radius;
    return pose;
}

void CopyBytes(const char *source, std::size_t byte_count, char *destination, std::size_t *inout_offset) {
    if (source == nullptr) {
        return;
    }

    if (destination == nullptr) {
        return;
    }

    if (inout_offset == nullptr) {
        return;
    }

    for (std::size_t index = 0U; index < byte_count; ++index) {
        destination[*inout_offset] = source[index];
        ++(*inout_offset);
    }
}

void CopyFrameDigits(std::uint32_t frame_index, char *destination, std::size_t *inout_offset) {
    if (destination == nullptr) {
        return;
    }

    if (inout_offset == nullptr) {
        return;
    }

    const std::uint32_t hundreds = frame_index / 100U;
    const std::uint32_t tens = (frame_index / 10U) % 10U;
    const std::uint32_t ones = frame_index % 10U;
    destination[*inout_offset] = static_cast<char>('0' + hundreds);
    ++(*inout_offset);
    destination[*inout_offset] = static_cast<char>('0' + tens);
    ++(*inout_offset);
    destination[*inout_offset] = static_cast<char>('0' + ones);
    ++(*inout_offset);
}
}

RenderSceneOrbitCaptureStatus RenderSceneOrbitCaptureRoute::Execute(
    const RenderSceneOrbitCaptureRequest &request,
    RenderSceneOrbitCaptureResult *out_result) {
    if (out_result == nullptr) {
        return RenderSceneOrbitCaptureStatus::InvalidArgument;
    }

    RenderSceneOrbitCaptureResult result{};
    result.first_frame_id = request.first_frame_id;
    result.requested_frame_count = request.frame_count;
    result.target = request.target;
    result.orbit_radius = request.orbit_radius;
    result.orbit_height = request.orbit_height;

    if (!ValidateRequest(request, &result)) {
        *out_result = result;
        return result.status;
    }

    result.frame_capture_byte_budget =
        request.capture_byte_budget_per_entity * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;

    RenderSceneCameraFrameBinder camera_binder;
    RenderSceneThreePrimitiveCaptureRoute capture_route;
    for (std::uint32_t frame_index = 0U; frame_index < request.frame_count; ++frame_index) {
        RenderSceneOrbitCaptureFrameReport &frame_report = result.frames[frame_index];
        frame_report.frame_index = frame_index;
        frame_report.frame_id = request.first_frame_id + frame_index;
        std::uint32_t orbit_step_count = request.frame_count - 1U;
        if (!request.close_orbit_loop) {
            orbit_step_count = request.frame_count;
        }

        const float frame_ratio =
            static_cast<float>(frame_index) / static_cast<float>(orbit_step_count);
        frame_report.orbit_angle_radians = FULL_ORBIT_RADIANS * frame_ratio;
        frame_report.target = request.target;
        frame_report.camera_pose = BuildOrbitPose(request, frame_report.orbit_angle_radians);

        if (!BuildFrameOutputPath(request, frame_index, &frame_report)) {
            result.status = RenderSceneOrbitCaptureStatus::Fail;
            result.first_missing_layer = RenderSceneOrbitCaptureMissingLayer::OutputPath;
            *out_result = result;
            return result.status;
        }

        RenderSceneRuntimeCameraRecord camera = request.camera_template;
        camera.pose = frame_report.camera_pose;
        const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{camera};

        RenderSceneCameraBindingRequest camera_request{};
        camera_request.frame_id = frame_report.frame_id;
        camera_request.active_camera_id = camera.camera_id;
        camera_request.cameras = cameras;
        camera_request.capture_byte_budget = result.frame_capture_byte_budget;
        camera_request.capture_requested = true;

        RenderSceneCameraBindingResult camera_result{};
        const RenderSceneStatus camera_status =
            camera_binder.BuildActiveCameraFrame(camera_request, &camera_result);
        frame_report.capture = camera_result.capture;
        if (camera_status != RenderSceneStatus::Success) {
            frame_report.status = RenderSceneOrbitCaptureStatus::Fail;
            frame_report.first_missing_layer = RenderSceneOrbitCaptureMissingLayer::Camera;
            result.status = frame_report.status;
            result.first_missing_layer = frame_report.first_missing_layer;
            *out_result = result;
            return result.status;
        }

        const std::size_t frame_capture_offset =
            static_cast<std::size_t>(frame_index) * result.frame_capture_byte_budget;
        const std::span<std::uint8_t> frame_capture_output =
            request.capture_output.subspan(frame_capture_offset, result.frame_capture_byte_budget);

        RenderSceneThreePrimitiveCaptureRequest capture_request{};
        capture_request.frame_id = frame_report.frame_id;
        capture_request.camera = camera_result;
        capture_request.material = request.material;
        capture_request.entities = request.entities;
        capture_request.rhi_device = request.rhi_device;
        capture_request.output_path = frame_report.output_path;
        capture_request.output_path_byte_count = frame_report.output_path_byte_count;
        capture_request.capture_output = frame_capture_output;
        capture_request.capture_byte_budget_per_entity = request.capture_byte_budget_per_entity;

        capture_route.Execute(capture_request, &frame_report.capture_result);
        frame_report.capture = frame_report.capture_result.capture;
        frame_report.output_status = frame_report.capture_result.output_status;
        frame_report.capture_bytes_written = frame_report.capture_result.capture_bytes_written;
        const RenderSceneOrbitCaptureStatus frame_status =
            MapFrameStatus(frame_report.capture_result, &frame_report);
        if (frame_status != RenderSceneOrbitCaptureStatus::Success) {
            result.status = frame_status;
            result.first_missing_layer = frame_report.first_missing_layer;
            *out_result = result;
            return result.status;
        }

        ++result.completed_frame_count;
        result.capture_bytes_written += frame_report.capture_bytes_written;
    }

    result.status = RenderSceneOrbitCaptureStatus::Success;
    result.first_missing_layer = RenderSceneOrbitCaptureMissingLayer::None;
    *out_result = result;
    return result.status;
}

bool RenderSceneOrbitCaptureRoute::ValidateRequest(
    const RenderSceneOrbitCaptureRequest &request,
    RenderSceneOrbitCaptureResult *out_result) const {
    if (out_result == nullptr) {
        return false;
    }

    if (request.first_frame_id == 0U) {
        out_result->status = RenderSceneOrbitCaptureStatus::InvalidArgument;
        return false;
    }

    if (!IsOrbitRequestUsable(request)) {
        out_result->status = RenderSceneOrbitCaptureStatus::InvalidArgument;
        out_result->first_missing_layer = RenderSceneOrbitCaptureMissingLayer::CameraOrbit;
        return false;
    }

    if (!IsOutputPathPrefixUsable(request)) {
        out_result->status = RenderSceneOrbitCaptureStatus::Fail;
        out_result->first_missing_layer = RenderSceneOrbitCaptureMissingLayer::OutputPath;
        return false;
    }

    if (!IsCaptureStorageUsable(request)) {
        out_result->status = RenderSceneOrbitCaptureStatus::InvalidArgument;
        out_result->first_missing_layer = RenderSceneOrbitCaptureMissingLayer::CaptureStorage;
        return false;
    }

    out_result->status = RenderSceneOrbitCaptureStatus::Success;
    out_result->first_missing_layer = RenderSceneOrbitCaptureMissingLayer::None;
    return true;
}

bool RenderSceneOrbitCaptureRoute::BuildFrameOutputPath(
    const RenderSceneOrbitCaptureRequest &request,
    std::uint32_t frame_index,
    RenderSceneOrbitCaptureFrameReport *out_report) const {
    if (out_report == nullptr) {
        return false;
    }

    if (frame_index >= 1000U) {
        return false;
    }

    if (!IsOutputPathPrefixUsable(request)) {
        return false;
    }

    std::size_t offset = 0U;
    CopyBytes(request.output_path_prefix, request.output_path_prefix_byte_count, out_report->output_path, &offset);
    CopyBytes(FRAME_PATH_STEM, FRAME_PATH_STEM_BYTE_COUNT, out_report->output_path, &offset);
    CopyFrameDigits(frame_index, out_report->output_path, &offset);
    CopyBytes(FRAME_PATH_EXTENSION, FRAME_PATH_EXTENSION_BYTE_COUNT, out_report->output_path, &offset);
    out_report->output_path[offset] = '\0';
    out_report->output_path_byte_count = offset;
    return true;
}

RenderSceneOrbitCaptureStatus RenderSceneOrbitCaptureRoute::MapFrameStatus(
    const RenderSceneThreePrimitiveCaptureResult &capture_result,
    RenderSceneOrbitCaptureFrameReport *out_report) const {
    if (out_report == nullptr) {
        return RenderSceneOrbitCaptureStatus::InvalidArgument;
    }

    out_report->first_missing_layer = MapMissingLayer(capture_result.first_missing_layer);
    if (capture_result.status == RenderSceneThreePrimitiveCaptureStatus::Success) {
        out_report->status = RenderSceneOrbitCaptureStatus::Success;
        out_report->first_missing_layer = RenderSceneOrbitCaptureMissingLayer::None;
        return out_report->status;
    }

    if (capture_result.status == RenderSceneThreePrimitiveCaptureStatus::BlockedByEnv) {
        out_report->status = RenderSceneOrbitCaptureStatus::BlockedByEnv;
        return out_report->status;
    }

    if (capture_result.status == RenderSceneThreePrimitiveCaptureStatus::InvalidArgument) {
        out_report->status = RenderSceneOrbitCaptureStatus::InvalidArgument;
        if (out_report->first_missing_layer == RenderSceneOrbitCaptureMissingLayer::None) {
            out_report->first_missing_layer = RenderSceneOrbitCaptureMissingLayer::CaptureStorage;
        }

        return out_report->status;
    }

    out_report->status = RenderSceneOrbitCaptureStatus::Fail;
    return out_report->status;
}

RenderSceneOrbitCaptureMissingLayer RenderSceneOrbitCaptureRoute::MapMissingLayer(
    RenderSceneThreePrimitiveCaptureMissingLayer layer) const {
    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::None) {
        return RenderSceneOrbitCaptureMissingLayer::None;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::Camera) {
        return RenderSceneOrbitCaptureMissingLayer::Camera;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::GeometryModel) {
        return RenderSceneOrbitCaptureMissingLayer::GeometryModel;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::MaterialTextureSlots) {
        return RenderSceneOrbitCaptureMissingLayer::MaterialTextureSlots;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::ShaderPipeline) {
        return RenderSceneOrbitCaptureMissingLayer::ShaderPipeline;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement) {
        return RenderSceneOrbitCaptureMissingLayer::ScenePlacement;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::RenderSceneSubmission) {
        return RenderSceneOrbitCaptureMissingLayer::RenderSceneSubmission;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::RenderCoreRhiDrawCapture) {
        return RenderSceneOrbitCaptureMissingLayer::RenderCoreRhiDrawCapture;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::RhiCaptureTarget) {
        return RenderSceneOrbitCaptureMissingLayer::RhiCaptureTarget;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::OutputPath) {
        return RenderSceneOrbitCaptureMissingLayer::OutputPath;
    }

    return RenderSceneOrbitCaptureMissingLayer::RenderCoreRhiDrawCapture;
}
}
