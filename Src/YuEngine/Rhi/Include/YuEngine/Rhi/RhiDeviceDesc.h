// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiNativeSurfaceDesc.h"

namespace yuengine::rhi {
struct RhiDeviceDesc final {
    RhiBackendKind backend_kind = RhiBackendKind::Null;
    RhiNativeSurfaceDesc native_surface{};
    std::size_t color_target_capacity = MAX_COLOR_TARGETS;
    std::size_t command_list_capacity = MAX_COMMANDS;
    bool requires_native_surface = false;
};
}
