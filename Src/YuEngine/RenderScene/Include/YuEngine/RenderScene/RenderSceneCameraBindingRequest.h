// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneCameraBindingRequest.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderScene/RenderSceneConstants.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeCameraRecord.h"

namespace yuengine::renderscene {
struct RenderSceneCameraBindingRequest final {
    std::uint32_t frame_id = 0U;
    std::uint32_t active_camera_id = INVALID_RENDER_SCENE_CAMERA_ID;
    std::span<const RenderSceneRuntimeCameraRecord> cameras{};
    std::size_t capture_byte_budget = 0U;
    bool capture_requested = false;
};
}
