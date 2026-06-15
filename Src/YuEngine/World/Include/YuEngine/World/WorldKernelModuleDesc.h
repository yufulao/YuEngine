// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldKernelModuleDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldServiceIds.h"

namespace yuengine::world {
struct WorldKernelModuleDesc final {
    const char *module_name = WORLD_KERNEL_MODULE_NAME;
    const char *world_service_id = WORLD_INSTANCE_SERVICE_ID;
    std::uint64_t fixed_step_duration = 16U;
    bool publish_world_service = true;
};
}
