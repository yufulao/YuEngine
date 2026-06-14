#pragma once

#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/memory/memory_allocation_id.h"

namespace yuengine::memory {
struct MemoryAccountingResult {
    MEMORY_ACCOUNTING_STATUS Status;
    MemoryAllocationId AllocationId;

    static MemoryAccountingResult Success(MemoryAllocationId allocationId) {
        return MemoryAccountingResult{MEMORY_ACCOUNTING_STATUS::Success, allocationId};
    }

    static MemoryAccountingResult Failure(MEMORY_ACCOUNTING_STATUS status) {
        return MemoryAccountingResult{status, MemoryAllocationId{0U}};
    }

    bool Succeeded() const {
        return Status == MEMORY_ACCOUNTING_STATUS::Success;
    }
};
}
