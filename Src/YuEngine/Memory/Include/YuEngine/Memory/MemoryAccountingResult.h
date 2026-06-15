// Module: YuEngine Memory
// File: Src/YuEngine/Memory/Include/YuEngine/Memory/MemoryAccountingResult.h

#pragma once

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Memory/MemoryAllocationId.h"

namespace yuengine::memory {
struct MemoryAccountingResult {
    MemoryAccountingStatus status;
    MemoryAllocationId allocation_id;

    /**
     * @comment Creates a successful result.
     * @param allocation_id Input allocation id.
     * @return Explicit operation result.
     */
    static MemoryAccountingResult Success(MemoryAllocationId allocation_id) {
        return MemoryAccountingResult{MemoryAccountingStatus::Success, allocation_id};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input status.
     * @return Explicit operation result.
     */
    static MemoryAccountingResult Failure(MemoryAccountingStatus status) {
        return MemoryAccountingResult{status, MemoryAllocationId{0U}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool Succeeded() const {
        return status == MemoryAccountingStatus::Success;
    }
};
}
