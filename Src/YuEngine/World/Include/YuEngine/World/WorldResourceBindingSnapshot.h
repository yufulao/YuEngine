// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldResourceBindingSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldResourceBindingStatus.h"

namespace yuengine::world {
struct WorldResourceBindingSnapshot final {
    std::uint32_t bridge_capacity = 0U;
    std::uint32_t active_binding_count = 0U;
    std::uint32_t acquired_binding_count = 0U;
    std::uint64_t released_binding_count = 0U;
    std::uint64_t cleared_binding_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    yuengine::resource::ResourceStatus last_resource_status =
        yuengine::resource::ResourceStatus::Success;
    WorldResourceBindingStatus last_status = WorldResourceBindingStatus::Success;
};
}
