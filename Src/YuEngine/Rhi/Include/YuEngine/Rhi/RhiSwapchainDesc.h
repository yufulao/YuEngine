// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSwapchainDesc.h

#pragma once

#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiFormat.h"

namespace yuengine::rhi {
struct RhiSwapchainDesc final {
    RhiFormat color_format = RhiFormat::Rgba8Unorm;
    RhiExtent2D extent{};
    bool vsync_enabled = false;
};
}
