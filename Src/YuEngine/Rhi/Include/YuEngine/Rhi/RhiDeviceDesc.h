#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiConstants.h"

namespace yuengine::rhi {
struct RhiDeviceDesc final {
    RhiBackendKind backend_kind = RhiBackendKind::Null;
    std::size_t color_target_capacity = MAX_COLOR_TARGETS;
    std::size_t command_list_capacity = MAX_COMMANDS;
};
}
