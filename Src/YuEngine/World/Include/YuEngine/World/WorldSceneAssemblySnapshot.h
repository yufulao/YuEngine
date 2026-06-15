// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblySnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldSceneAssemblyStatus.h"

namespace yuengine::world {
struct WorldSceneAssemblySnapshot final {
    std::uint32_t attachment_capacity = 0U;
    std::uint32_t binding_capacity = 0U;
    std::uint64_t assembly_attempt_count = 0U;
    std::uint64_t restored_attachment_count = 0U;
    std::uint64_t restored_binding_count = 0U;
    std::uint64_t rejected_record_count = 0U;
    std::uint32_t rollback_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    yuengine::resource::ResourceStatus last_resource_status =
        yuengine::resource::ResourceStatus::Success;
    WorldComponentAttachmentStatus last_attachment_status =
        WorldComponentAttachmentStatus::Success;
    WorldComponentResourceBindingStatus last_binding_status =
        WorldComponentResourceBindingStatus::Success;
    WorldComponentResourceBindingRestoreStatus last_binding_restore_status =
        WorldComponentResourceBindingRestoreStatus::Success;
    WorldSceneAssemblyStatus last_status = WorldSceneAssemblyStatus::Success;
};
}
