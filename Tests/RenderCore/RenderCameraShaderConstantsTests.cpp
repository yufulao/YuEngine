// 模块：Tests RenderCore
// 文件：Tests/RenderCore/RenderCameraShaderConstantsTests.cpp

#include <cmath>
#include <cstdio>
#include <string_view>

#include "YuEngine/RenderCore/RenderCamera.h"
#include "YuEngine/RenderCore/RenderCameraFrame.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstants.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstantsStatus.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstantsWriter.h"

using RenderCamera = yuengine::rendercore::RenderCamera;
using RenderCameraFrame = yuengine::rendercore::RenderCameraFrame;
using RenderCameraPose = yuengine::rendercore::RenderCameraPose;
using RenderCameraProjectionDesc = yuengine::rendercore::RenderCameraProjectionDesc;
using RenderCameraShaderConstants = yuengine::rendercore::RenderCameraShaderConstants;
using RenderCameraShaderConstantsWriter = yuengine::rendercore::RenderCameraShaderConstantsWriter;
using yuengine::rendercore::RenderCameraProjectionKind;
using yuengine::rendercore::RenderCameraShaderConstantsStatus;

namespace {
constexpr const char *TEST_WRITES_VIEW_PROJECTION = "RenderCore_CameraShaderConstantsWritesViewProjection";
constexpr const char *TEST_REJECTS_NULL_OUTPUT = "RenderCore_CameraShaderConstantsRejectsNullOutput";
constexpr const char *TEST_REJECTS_INVALID_FRAME = "RenderCore_CameraShaderConstantsRejectsInvalidFrameWithoutOutputMutation";
constexpr const char *TEST_SNAPSHOT = "RenderCore_CameraShaderConstantsSnapshotTracksCounters";
constexpr const char *TEST_BOUNDARY = "RenderCore_CameraShaderConstantsHasNarrowDependencyBoundary";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr float HALF_PI = 1.57079632679F;
constexpr float TOLERANCE = 0.0001F;
constexpr float SENTINEL_VALUE = 77.0F;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool Approx(float left, float right) {
    const float delta = std::fabs(left - right);
    return delta <= TOLERANCE;
}

RenderCameraFrame BuildDefaultFrame() {
    RenderCameraPose pose{};
    pose.position = {0.0F, 0.0F, -5.0F};
    pose.target = {0.0F, 0.0F, 0.0F};
    pose.up = {0.0F, 1.0F, 0.0F};

    RenderCameraProjectionDesc projection{};
    projection.kind = RenderCameraProjectionKind::Perspective;
    projection.vertical_fov_radians = HALF_PI;
    projection.aspect_ratio = 1.0F;
    projection.near_z = 0.1F;
    projection.far_z = 100.0F;

    RenderCamera camera;
    RenderCameraFrame frame{};
    camera.BuildFrame(pose, projection, &frame);
    return frame;
}

RenderCameraShaderConstants SentinelConstants() {
    RenderCameraShaderConstants constants{};
    constants.view_projection_values.fill(SENTINEL_VALUE);
    return constants;
}

bool ConstantsMatchSentinel(const RenderCameraShaderConstants &constants) {
    for (float value : constants.view_projection_values) {
        if (!Approx(value, SENTINEL_VALUE)) {
            return false;
        }
    }

    return true;
}

int RenderCoreCameraShaderConstantsWritesViewProjection() {
    RenderCameraShaderConstantsWriter writer;
    const RenderCameraFrame frame = BuildDefaultFrame();
    RenderCameraShaderConstants constants{};
    const RenderCameraShaderConstantsStatus status = writer.WriteViewProjection(frame, &constants);
    if (status != RenderCameraShaderConstantsStatus::Success) {
        return Fail("camera constants rejected valid frame");
    }

    if (!Approx(constants.view_projection_values[0U], frame.view_projection.values[0U])) {
        return Fail("camera constants missed view projection value");
    }

    if (!Approx(constants.view_projection_values[14U], frame.view_projection.values[14U])) {
        return Fail("camera constants missed translation value");
    }

    const auto snapshot = writer.Snapshot();
    if (snapshot.accepted_write_count != 1U || snapshot.last_status != RenderCameraShaderConstantsStatus::Success) {
        return Fail("camera constants snapshot missed accepted write");
    }

    return 0;
}

int RenderCoreCameraShaderConstantsRejectsNullOutput() {
    RenderCameraShaderConstantsWriter writer;
    const RenderCameraShaderConstantsStatus status = writer.WriteViewProjection(BuildDefaultFrame(), nullptr);
    if (status != RenderCameraShaderConstantsStatus::InvalidArgument) {
        return Fail("camera constants accepted null output");
    }

    const auto snapshot = writer.Snapshot();
    if (snapshot.rejected_write_count != 1U) {
        return Fail("camera constants snapshot missed null output");
    }

    return 0;
}

int RenderCoreCameraShaderConstantsRejectsInvalidFrameWithoutOutputMutation() {
    RenderCameraShaderConstantsWriter writer;
    RenderCameraFrame frame = BuildDefaultFrame();
    frame.view_projection.values[3U] = NAN;

    RenderCameraShaderConstants constants = SentinelConstants();
    const RenderCameraShaderConstantsStatus status = writer.WriteViewProjection(frame, &constants);
    if (status != RenderCameraShaderConstantsStatus::InvalidFrame) {
        return Fail("camera constants accepted invalid frame");
    }

    if (!ConstantsMatchSentinel(constants)) {
        return Fail("camera constants mutated output after invalid frame");
    }

    return 0;
}

int RenderCoreCameraShaderConstantsSnapshotTracksCounters() {
    RenderCameraShaderConstantsWriter writer;
    RenderCameraFrame frame = BuildDefaultFrame();
    frame.view_projection.values[2U] = NAN;

    RenderCameraShaderConstants constants = SentinelConstants();
    if (writer.WriteViewProjection(frame, &constants) != RenderCameraShaderConstantsStatus::InvalidFrame) {
        return Fail("camera constants failed rejection setup");
    }

    const auto rejected_snapshot = writer.Snapshot();
    if (rejected_snapshot.accepted_write_count != 0U || rejected_snapshot.rejected_write_count != 1U) {
        return Fail("camera constants snapshot missed rejected write");
    }

    if (rejected_snapshot.last_status != RenderCameraShaderConstantsStatus::InvalidFrame) {
        return Fail("camera constants snapshot missed rejected status");
    }

    if (!ConstantsMatchSentinel(constants)) {
        return Fail("camera constants mutated output during rejected write");
    }

    const RenderCameraFrame valid_frame = BuildDefaultFrame();
    if (writer.WriteViewProjection(valid_frame, &constants) != RenderCameraShaderConstantsStatus::Success) {
        return Fail("camera constants failed success after rejection");
    }

    const auto snapshot = writer.Snapshot();
    if (snapshot.accepted_write_count != 1U || snapshot.rejected_write_count != 1U) {
        return Fail("camera constants snapshot counters were not stable");
    }

    if (snapshot.last_status != RenderCameraShaderConstantsStatus::Success) {
        return Fail("camera constants snapshot did not clear status after success");
    }

    writer.Reset();
    const auto reset_snapshot = writer.Snapshot();
    if (reset_snapshot.accepted_write_count != 0U || reset_snapshot.rejected_write_count != 0U) {
        return Fail("camera constants reset did not clear counters");
    }

    if (reset_snapshot.last_status != RenderCameraShaderConstantsStatus::InvalidArgument) {
        return Fail("camera constants reset did not restore status");
    }

    return 0;
}

int RenderCoreCameraShaderConstantsHasNarrowDependencyBoundary() {
    RenderCameraShaderConstantsWriter writer;
    RenderCameraShaderConstants constants{};
    const RenderCameraShaderConstantsStatus status = writer.WriteViewProjection(BuildDefaultFrame(), &constants);
    if (status != RenderCameraShaderConstantsStatus::Success) {
        return Fail("camera constants dependency smoke setup failed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_WRITES_VIEW_PROJECTION) {
        return RenderCoreCameraShaderConstantsWritesViewProjection();
    }

    if (name == TEST_REJECTS_NULL_OUTPUT) {
        return RenderCoreCameraShaderConstantsRejectsNullOutput();
    }

    if (name == TEST_REJECTS_INVALID_FRAME) {
        return RenderCoreCameraShaderConstantsRejectsInvalidFrameWithoutOutputMutation();
    }

    if (name == TEST_SNAPSHOT) {
        return RenderCoreCameraShaderConstantsSnapshotTracksCounters();
    }

    if (name == TEST_BOUNDARY) {
        return RenderCoreCameraShaderConstantsHasNarrowDependencyBoundary();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
