#pragma once

#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/memory/memory_allocation_id.h"

namespace yuengine::memory {
struct memory_accounting_result_t {
    MemoryAccountingStatus Status;
    memory_allocation_id_t AllocationId;

    static memory_accounting_result_t Success(memory_allocation_id_t allocationId) {
        return memory_accounting_result_t{MemoryAccountingStatus::Success, allocationId};
    }

    static memory_accounting_result_t Failure(MemoryAccountingStatus status) {
        return memory_accounting_result_t{status, memory_allocation_id_t{0U}};
    }

    bool Succeeded() const {
        return Status == MemoryAccountingStatus::Success;
    }
};
}
