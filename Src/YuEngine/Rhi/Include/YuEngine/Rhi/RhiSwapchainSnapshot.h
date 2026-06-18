// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSwapchainSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiFormat.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
struct RhiSwapchainSnapshot final {
    RhiExtent2D extent{};
    RhiFormat color_format = RhiFormat::Rgba8Unorm;
    RhiTextureHandle color_target{};
    std::uint64_t resize_count = 0U;
    std::uint64_t rejected_resize_count = 0U;
    bool valid = false;
    bool presented = false;
};
}
