// 模块: YuEngine Object
// 文件: Src/YuEngine/Object/Include/YuEngine/Object/ObjectSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectDescriptor.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/Object/ObjectTypeId.h"

namespace yuengine::object {
using memory::MemoryAccountingStatus;

struct ObjectSnapshot final {
    std::uint32_t object_capacity;
    std::uint32_t type_capacity;
    std::uint32_t type_count;
    std::uint32_t last_required_object_count;
    std::uint32_t last_required_type_count;
    std::uint32_t last_capacity_entry_object_capacity;
    std::uint32_t last_capacity_entry_object_count;
    std::uint32_t last_capacity_entry_required_object_count;
    std::uint32_t last_capacity_entry_type_capacity;
    std::uint32_t last_capacity_entry_type_count;
    std::uint32_t last_capacity_entry_required_type_count;
    ObjectDescriptor last_capacity_entry_descriptor;
    ObjectTypeId last_failed_type_capacity_type;
    std::uint32_t last_failed_type_capacity;
    std::uint32_t last_failed_type_count;
    std::uint32_t last_failed_required_type_count;
    std::uint32_t alive_object_count;
    std::uint32_t destroyed_object_count;
    std::uint64_t created_object_count;
    std::uint64_t referenced_object_count;
    std::uint64_t released_reference_count;
    std::uint32_t accepted_operation_count;
    std::uint32_t failed_operation_count;
    MemoryAccountingStatus allocation_accounting_status;
    ObjectStatus last_status;
};
}
