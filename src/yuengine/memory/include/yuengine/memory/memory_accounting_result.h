#pragma once

#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/memory/memory_allocation_id.h"

namespace yuengine::memory {
struct MemoryAccountingResult {
    MemoryAccountingStatus Status;
    MemoryAllocationId AllocationId;

    static MemoryAccountingResult Success(MemoryAllocationId allocationId) {
        return MemoryAccountingResult{MemoryAccountingStatus::Success, allocationId};
    }

    static MemoryAccountingResult Failure(MemoryAccountingStatus status) {
        return MemoryAccountingResult{status, MemoryAllocationId{0U}};
    }

    bool Succeeded() const {
        return Status == MemoryAccountingStatus::Success;
    }
};
}
