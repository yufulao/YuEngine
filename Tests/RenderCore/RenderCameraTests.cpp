// 模块：Tests RenderCore
// 文件：Tests/RenderCore/RenderCameraTests.cpp

#include <cmath>
#include <cstdio>
#include <string_view>

#include "YuEngine/RenderCore/RenderCamera.h"
#include "YuEngine/RenderCore/RenderCameraFrame.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderCore/RenderCameraStatus.h"

using RenderCamera = yuengine::rendercore::RenderCamera;
using RenderCameraFrame = yuengine::rendercore::RenderCameraFrame;
using RenderCameraPose = yuengine::rendercore::RenderCameraPose;
using RenderCameraProjectionDesc = yuengine::rendercore::RenderCameraProjectionDesc;
using yuengine::rendercore::RenderCameraProjectionKind;
using yuengine::rendercore::RenderCameraStatus;

namespace {
constexpr const char *TEST_PERSPECTIVE = "RenderCore_CameraBuildsPerspectiveFrame";
constexpr const char *TEST_ORTHOGRAPHIC = "RenderCore_CameraBuildsOrthographicFrame";
constexpr const char *TEST_INVALID_PROJECTION = "RenderCore_CameraRejectsInvalidProjectionWithoutOutputMutation";
constexpr const char *TEST_DEGENERATE_POSE = "RenderCore_CameraRejectsDegeneratePoseWithoutOutputMutation";
constexpr const char *TEST_SNAPSHOT = "RenderCore_CameraSnapshotTracksCounters";
constexpr const char *TEST_BOUNDARY = "RenderCore_CameraHasNarrowDependencyBoundary";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr float HALF_PI = 1.57079632679F;
constexpr float TOLERANCE = 0.0001F;
constexpr float SENTINEL_VALUE = 99.0F;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool Approx(float left, float right) {
    const float delta = std::fabs(left - right);
    return delta <= TOLERANCE;
}

RenderCameraPose DefaultPose() {
    RenderCameraPose pose{};
    pose.position = {0.0F, 0.0F, -5.0F};
    pose.target = {0.0F, 0.0F, 0.0F};
    pose.up = {0.0F, 1.0F, 0.0F};
    return pose;
}

RenderCameraProjectionDesc PerspectiveProjection() {
    RenderCameraProjectionDesc projection{};
    projection.kind = RenderCameraProjectionKind::Perspective;
    projection.vertical_fov_radians = HALF_PI;
    projection.aspect_ratio = 1.0F;
    projection.near_z = 0.1F;
    projection.far_z = 100.0F;
    return projection;
}

RenderCameraProjectionDesc OrthographicProjection() {
    RenderCameraProjectionDesc projection{};
    projection.kind = RenderCameraProjectionKind::Orthographic;
    projection.aspect_ratio = 1.0F;
    projection.near_z = 0.1F;
    projection.far_z = 100.0F;
    projection.orthographic_height = 4.0F;
    return projection;
}

RenderCameraFrame SentinelFrame() {
    RenderCameraFrame frame{};
    frame.view.values.fill(SENTINEL_VALUE);
    frame.projection.values.fill(SENTINEL_VALUE);
    frame.view_projection.values.fill(SENTINEL_VALUE);
    return frame;
}

bool FrameMatchesSentinel(const RenderCameraFrame &frame) {
    for (float value : frame.view.values) {
        if (!Approx(value, SENTINEL_VALUE)) {
            return false;
        }
    }

    for (float value : frame.projection.values) {
        if (!Approx(value, SENTINEL_VALUE)) {
            return false;
        }
    }

    for (float value : frame.view_projection.values) {
        if (!Approx(value, SENTINEL_VALUE)) {
            return false;
        }
    }

    return true;
}

int RenderCoreCameraBuildsPerspectiveFrame() {
    RenderCamera camera;
    RenderCameraFrame frame{};
    const RenderCameraStatus status = camera.BuildFrame(DefaultPose(), PerspectiveProjection(), &frame);
    if (status != RenderCameraStatus::Success) {
        return Fail("camera rejected valid perspective projection");
    }

    if (!Approx(frame.view.values[0U], 1.0F) || !Approx(frame.view.values[5U], 1.0F)) {
        return Fail("camera view basis was not stable");
    }

    if (!Approx(frame.view.values[10U], 1.0F) || !Approx(frame.view.values[14U], 5.0F)) {
        return Fail("camera view translation was not stable");
    }

    if (!Approx(frame.projection.values[0U], 1.0F) || !Approx(frame.projection.values[5U], 1.0F)) {
        return Fail("camera perspective scale was not stable");
    }

    if (!Approx(frame.projection.values[11U], 1.0F)) {
        return Fail("camera perspective depth term was not stable");
    }

    const auto snapshot = camera.Snapshot();
    if (snapshot.accepted_frame_count != 1U || snapshot.last_status != RenderCameraStatus::Success) {
        return Fail("camera snapshot missed accepted perspective frame");
    }

    return 0;
}

int RenderCoreCameraBuildsOrthographicFrame() {
    RenderCamera camera;
    RenderCameraFrame frame{};
    const RenderCameraStatus status = camera.BuildFrame(DefaultPose(), OrthographicProjection(), &frame);
    if (status != RenderCameraStatus::Success) {
        return Fail("camera rejected valid orthographic projection");
    }

    if (!Approx(frame.projection.values[0U], 0.5F) || !Approx(frame.projection.values[5U], 0.5F)) {
        return Fail("camera orthographic scale was not stable");
    }

    if (!Approx(frame.projection.values[15U], 1.0F)) {
        return Fail("camera orthographic matrix was not affine");
    }

    return 0;
}

int RenderCoreCameraRejectsInvalidProjectionWithoutOutputMutation() {
    RenderCamera camera;
    RenderCameraProjectionDesc projection = PerspectiveProjection();
    projection.near_z = 10.0F;
    projection.far_z = 1.0F;

    RenderCameraFrame frame = SentinelFrame();
    const RenderCameraStatus status = camera.BuildFrame(DefaultPose(), projection, &frame);
    if (status != RenderCameraStatus::InvalidProjection) {
        return Fail("camera accepted invalid projection");
    }

    if (!FrameMatchesSentinel(frame)) {
        return Fail("camera mutated output after invalid projection");
    }

    const auto snapshot = camera.Snapshot();
    if (snapshot.rejected_frame_count != 1U || snapshot.last_status != RenderCameraStatus::InvalidProjection) {
        return Fail("camera snapshot missed invalid projection");
    }

    return 0;
}

int RenderCoreCameraRejectsDegeneratePoseWithoutOutputMutation() {
    RenderCamera camera;
    RenderCameraPose pose = DefaultPose();
    pose.target = pose.position;

    RenderCameraFrame frame = SentinelFrame();
    const RenderCameraStatus status = camera.BuildFrame(pose, PerspectiveProjection(), &frame);
    if (status != RenderCameraStatus::InvalidPose) {
        return Fail("camera accepted degenerate pose");
    }

    if (!FrameMatchesSentinel(frame)) {
        return Fail("camera mutated output after degenerate pose");
    }

    return 0;
}

int RenderCoreCameraSnapshotTracksCounters() {
    RenderCamera camera;
    RenderCameraFrame frame{};
    if (camera.BuildFrame(DefaultPose(), PerspectiveProjection(), &frame) != RenderCameraStatus::Success) {
        return Fail("camera failed first snapshot setup");
    }

    RenderCameraProjectionDesc projection = PerspectiveProjection();
    projection.aspect_ratio = 0.0F;
    if (camera.BuildFrame(DefaultPose(), projection, &frame) != RenderCameraStatus::InvalidProjection) {
        return Fail("camera failed second snapshot setup");
    }

    const auto snapshot = camera.Snapshot();
    if (snapshot.accepted_frame_count != 1U || snapshot.rejected_frame_count != 1U) {
        return Fail("camera snapshot counters were not stable");
    }

    camera.Reset();
    const auto reset_snapshot = camera.Snapshot();
    if (reset_snapshot.accepted_frame_count != 0U || reset_snapshot.rejected_frame_count != 0U) {
        return Fail("camera reset did not clear counters");
    }

    return 0;
}

int RenderCoreCameraHasNarrowDependencyBoundary() {
    RenderCamera camera;
    RenderCameraFrame frame{};
    const RenderCameraStatus status = camera.BuildFrame(DefaultPose(), PerspectiveProjection(), &frame);
    if (status != RenderCameraStatus::Success) {
        return Fail("camera dependency smoke setup failed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_PERSPECTIVE) {
        return RenderCoreCameraBuildsPerspectiveFrame();
    }

    if (name == TEST_ORTHOGRAPHIC) {
        return RenderCoreCameraBuildsOrthographicFrame();
    }

    if (name == TEST_INVALID_PROJECTION) {
        return RenderCoreCameraRejectsInvalidProjectionWithoutOutputMutation();
    }

    if (name == TEST_DEGENERATE_POSE) {
        return RenderCoreCameraRejectsDegeneratePoseWithoutOutputMutation();
    }

    if (name == TEST_SNAPSHOT) {
        return RenderCoreCameraSnapshotTracksCounters();
    }

    if (name == TEST_BOUNDARY) {
        return RenderCoreCameraHasNarrowDependencyBoundary();
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
