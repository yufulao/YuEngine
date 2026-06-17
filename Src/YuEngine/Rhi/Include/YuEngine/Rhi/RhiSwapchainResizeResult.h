// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSwapchainResizeResult.h

#pragma once

#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainSnapshot.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
struct RhiSwapchainResizeResult final {
    RhiStatus status = RhiStatus::InvalidDescriptor;
    RhiExtent2D previous_extent{};
    RhiTextureHandle previous_color_target{};
    RhiSwapchainSnapshot snapshot{};
    bool resized = false;
};
}
