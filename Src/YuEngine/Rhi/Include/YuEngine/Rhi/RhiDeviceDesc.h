// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiNativeSurfaceDesc.h"
#include "YuEngine/Rhi/RhiSwapchainDesc.h"

namespace yuengine::rhi {
struct RhiDeviceDesc final {
    RhiBackendKind backend_kind = RhiBackendKind::Null;
    RhiNativeSurfaceDesc native_surface{};
    std::size_t color_target_capacity = MAX_COLOR_TARGETS;
    std::size_t command_list_capacity = MAX_COMMANDS;
    RhiSwapchainDesc swapchain{};
    bool requires_native_surface = false;
    bool requires_swapchain = false;
};
}
