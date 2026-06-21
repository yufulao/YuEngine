// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneCameraCaptureMetadata.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderCameraPose.h"
#include "YuEngine/RenderScene/RenderSceneStatus.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::renderscene {
struct RenderSceneCameraCaptureMetadata final {
    RenderSceneStatus status = RenderSceneStatus::Success;
    std::uint32_t frame_id = 0U;
    std::uint32_t camera_id = 0U;
    yuengine::rendercore::RenderCameraPose pose{};
    yuengine::rhi::RhiTextureHandle target{};
    std::size_t output_byte_budget = 0U;
    bool capture_requested = false;
};
}
