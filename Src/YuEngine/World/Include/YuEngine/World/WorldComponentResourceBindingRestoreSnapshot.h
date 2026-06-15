// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingRestoreSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"

namespace yuengine::world {
struct WorldComponentResourceBindingRestoreSnapshot final {
    std::uint32_t binding_capacity = 0U;
    std::uint64_t restore_attempt_count = 0U;
    std::uint64_t restored_binding_count = 0U;
    std::uint64_t rejected_record_count = 0U;
    std::uint32_t rollback_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    yuengine::resource::ResourceStatus last_resource_status =
        yuengine::resource::ResourceStatus::Success;
    WorldComponentResourceBindingStatus last_binding_status =
        WorldComponentResourceBindingStatus::Success;
    WorldComponentResourceBindingRestoreStatus last_status =
        WorldComponentResourceBindingRestoreStatus::Success;
};
}
