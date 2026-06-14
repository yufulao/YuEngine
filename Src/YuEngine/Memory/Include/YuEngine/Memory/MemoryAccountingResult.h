#pragma once

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Memory/MemoryAllocationId.h"

namespace yuengine::memory {
struct MemoryAccountingResult {
    MemoryAccountingStatus status;
    MemoryAllocationId allocation_id;

    static MemoryAccountingResult Success(MemoryAllocationId allocation_id) {
        return MemoryAccountingResult{MemoryAccountingStatus::Success, allocation_id};
    }

    static MemoryAccountingResult Failure(MemoryAccountingStatus status) {
        return MemoryAccountingResult{status, MemoryAllocationId{0U}};
    }

    bool Succeeded() const {
        return status == MemoryAccountingStatus::Success;
    }
};
}
