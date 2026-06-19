// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneCameraRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderCameraFrame.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::renderscene {
struct RenderSceneCameraRecord final {
    std::uint32_t camera_id = 0U;
    yuengine::rendercore::RenderCameraFrame frame{};
    yuengine::rhi::RhiTextureHandle target{};
    yuengine::rhi::RhiColor clear_color{};
    bool is_active = false;
};
}
