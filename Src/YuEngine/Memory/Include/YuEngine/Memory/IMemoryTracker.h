#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingResult.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Memory/MemoryAllocationId.h"
#include "YuEngine/Memory/MemoryBudgetClass.h"
#include "YuEngine/Memory/MemoryOwnerId.h"
#include "YuEngine/Memory/MemorySnapshot.h"
#include "YuEngine/Memory/MemoryTag.h"

namespace yuengine::memory {
class IMemoryTracker {
public:
    virtual ~IMemoryTracker() = default;

    virtual MemoryAccountingResult RecordAllocation(
        MemoryOwnerId owner,
        MemoryTag tag,
        MemoryBudgetClass budget_class,
        std::size_t bytes,
        std::size_t alignment) = 0;
    virtual MemoryAccountingStatus RecordFree(MemoryAllocationId allocation_id, MemoryOwnerId owner, MemoryTag tag) = 0;
    virtual MemorySnapshot Snapshot() const = 0;
    virtual std::uint64_t AllocationCountForBudget(MemoryBudgetClass budget_class) const = 0;
};
}
