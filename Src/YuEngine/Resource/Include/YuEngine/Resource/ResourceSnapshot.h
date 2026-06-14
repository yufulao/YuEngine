#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"

namespace yuengine::resource {
struct ResourceSnapshot final {
    std::uint32_t resource_capacity;
    std::uint32_t type_capacity;
    std::uint32_t dependency_edge_capacity;
    std::uint32_t registered_resource_count;
    std::uint32_t type_count;
    std::uint64_t acquired_handle_count;
    std::uint64_t released_handle_count;
    std::uint32_t retired_resource_count;
    std::uint32_t dependency_edge_count;
    std::uint32_t dependency_validation_count;
    std::uint32_t failed_operation_count;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status;
    ResourceStatus last_status;
};
}
