// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSwapchainSnapshot.h

#pragma once

#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiFormat.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
struct RhiSwapchainSnapshot final {
    RhiExtent2D extent{};
    RhiFormat color_format = RhiFormat::Rgba8Unorm;
    RhiTextureHandle color_target{};
    bool valid = false;
    bool presented = false;
};
}
