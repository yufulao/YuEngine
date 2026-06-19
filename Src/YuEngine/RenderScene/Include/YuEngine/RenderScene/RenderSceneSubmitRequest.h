// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneSubmitRequest.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderScene/RenderSceneCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneConstants.h"
#include "YuEngine/RenderScene/RenderSceneEntityRecord.h"

namespace yuengine::renderscene {
struct RenderSceneSubmitRequest final {
    std::uint32_t frame_id = 0U;
    std::uint32_t active_camera_id = INVALID_RENDER_SCENE_CAMERA_ID;
    std::span<const RenderSceneCameraRecord> cameras{};
    std::span<const RenderSceneEntityRecord> entities{};
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget = 0U;
};
}
