#pragma once

#include <cstddef>

#include "yuengine/rhi/RhiBackendKind.h"
#include "yuengine/rhi/RhiConstants.h"

namespace yuengine::rhi {
struct RhiDeviceDesc final {
    RhiBackendKind BackendKind = RhiBackendKind::Null;
    std::size_t ColorTargetCapacity = MAX_COLOR_TARGETS;
    std::size_t CommandListCapacity = MAX_COMMANDS;
};
}
