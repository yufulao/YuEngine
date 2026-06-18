// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldObjectIdentitySnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"

namespace yuengine::world {
struct WorldObjectIdentitySnapshot final {
    std::uint32_t bridge_capacity = 0U;
    std::uint32_t binding_count = 0U;
    std::uint32_t acquired_handle_count = 0U;
    std::uint64_t released_handle_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    yuengine::object::ObjectStatus last_object_status = yuengine::object::ObjectStatus::Success;
    WorldObjectIdentityStatus last_status = WorldObjectIdentityStatus::Success;
};
}
