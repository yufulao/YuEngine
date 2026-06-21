// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderSceneCameraFrameBinder.cpp

#include "YuEngine/RenderScene/RenderSceneCameraFrameBinder.h"

namespace yuengine::renderscene {
RenderSceneStatus RenderSceneCameraFrameBinder::BuildActiveCameraFrame(
    const RenderSceneCameraBindingRequest &request,
    RenderSceneCameraBindingResult *out_result) {
    if (out_result == nullptr) {
        return RenderSceneStatus::NullPointer;
    }

    RenderSceneCameraBindingResult result{};
    result.capture.frame_id = request.frame_id;
    result.capture.output_byte_budget = request.capture_byte_budget;
    result.capture.capture_requested = request.capture_requested;

    if (request.frame_id == 0U) {
        result.status = RenderSceneStatus::InvalidFrameId;
        result.capture.status = result.status;
        *out_result = result;
        return result.status;
    }

    const RenderSceneRuntimeCameraRecord *runtime_camera = FindActiveCamera(request);
    if (runtime_camera == nullptr) {
        result.status = RenderSceneStatus::MissingCamera;
        result.capture.status = result.status;
        *out_result = result;
        return result.status;
    }

    result.camera.camera_id = runtime_camera->camera_id;
    result.camera.target = runtime_camera->target;
    result.camera.clear_color = runtime_camera->clear_color;
    result.camera.is_active = true;
    result.capture.camera_id = runtime_camera->camera_id;
    result.capture.pose = runtime_camera->pose;
    result.capture.target = runtime_camera->target;

    const yuengine::rendercore::RenderCameraStatus camera_status =
        camera_.BuildFrame(runtime_camera->pose, runtime_camera->projection, &result.camera.frame);
    result.status = MapCameraStatus(camera_status);
    result.capture.status = result.status;
    *out_result = result;
    return result.status;
}

const RenderSceneRuntimeCameraRecord *RenderSceneCameraFrameBinder::FindActiveCamera(
    const RenderSceneCameraBindingRequest &request) const {
    for (const RenderSceneRuntimeCameraRecord &camera : request.cameras) {
        if (!camera.is_active) {
            continue;
        }

        if (camera.camera_id != request.active_camera_id) {
            continue;
        }

        return &camera;
    }

    return nullptr;
}

RenderSceneStatus RenderSceneCameraFrameBinder::MapCameraStatus(
    yuengine::rendercore::RenderCameraStatus status) const {
    if (status == yuengine::rendercore::RenderCameraStatus::Success) {
        return RenderSceneStatus::Success;
    }

    if (status == yuengine::rendercore::RenderCameraStatus::InvalidPose) {
        return RenderSceneStatus::InvalidCameraPose;
    }

    if (status == yuengine::rendercore::RenderCameraStatus::InvalidProjection) {
        return RenderSceneStatus::InvalidCameraProjection;
    }

    return RenderSceneStatus::InvalidCameraRecord;
}
}
