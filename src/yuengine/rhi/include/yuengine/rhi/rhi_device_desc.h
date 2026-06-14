#pragma once

#include <cstddef>

#include "yuengine/rhi/rhi_backend_kind.h"
#include "yuengine/rhi/rhi_constants.h"

namespace yuengine::rhi {
struct RhiDeviceDesc final {
    RHI_BACKEND_KIND BackendKind = RHI_BACKEND_KIND::Null;
    std::size_t ColorTargetCapacity = MAX_COLOR_TARGETS;
    std::size_t CommandListCapacity = MAX_COMMANDS;
};
}
