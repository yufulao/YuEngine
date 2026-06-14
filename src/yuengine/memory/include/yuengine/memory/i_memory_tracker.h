#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/memory/memory_accounting_result.h"
#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/memory/memory_allocation_id.h"
#include "yuengine/memory/memory_budget_class.h"
#include "yuengine/memory/memory_owner_id.h"
#include "yuengine/memory/memory_snapshot.h"
#include "yuengine/memory/memory_tag.h"

namespace yuengine::memory {
class IMemoryTracker {
public:
    virtual ~IMemoryTracker() = default;

    virtual memory_accounting_result_t RecordAllocation(
        memory_owner_id_t owner,
        memory_tag_t tag,
        MEMORY_BUDGET_CLASS budgetClass,
        std::size_t bytes,
        std::size_t alignment) = 0;
    virtual MEMORY_ACCOUNTING_STATUS RecordFree(memory_allocation_id_t allocationId, memory_owner_id_t owner, memory_tag_t tag) = 0;
    virtual memory_snapshot_t Snapshot() const = 0;
    virtual std::uint64_t AllocationCountForBudget(MEMORY_BUDGET_CLASS budgetClass) const = 0;
};
}
