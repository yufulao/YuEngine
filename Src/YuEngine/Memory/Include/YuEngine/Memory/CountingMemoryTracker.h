// 模块: YuEngine Memory
// 文件: Src/YuEngine/Memory/Include/YuEngine/Memory/CountingMemoryTracker.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Memory/IMemoryTracker.h"
#include "YuEngine/Memory/ActiveAllocationRecord.h"

namespace yuengine::memory {
constexpr std::size_t MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS = 64U;

class CountingMemoryTracker final : public IMemoryTracker {
public:
    /**
     * @comment 构造 CountingMemoryTracker 实例。
     */
    CountingMemoryTracker();

    /**
     * @comment 记录分配。
     * @param owner 输入所有者。
     * @param tag 输入标签。
     * @param budget_class 输入 预算类别。
     * @param bytes 输入字节数或字节载荷。
     * @param alignment 输入 对齐。
     * @return 显式操作结果。
     */
    MemoryAccountingResult RecordAllocation(
        MemoryOwnerId owner,
        MemoryTag tag,
        MemoryBudgetClass budget_class,
        std::size_t bytes,
        std::size_t alignment) override;
    /**
     * @comment 记录释放。
     * @param allocation_id 输入 allocation id。
     * @param owner 输入所有者。
     * @param tag 输入标签。
     * @return 显式操作状态。
     */
    MemoryAccountingStatus RecordFree(MemoryAllocationId allocation_id, MemoryOwnerId owner, MemoryTag tag) override;
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    MemorySnapshot Snapshot() const override;
    /**
     * @comment 返回指定预算的计数。
     * @param budget_class 输入 预算类别。
     * @return budget 对应的 allocation 计数值。
     */
    std::uint64_t AllocationCountForBudget(MemoryBudgetClass budget_class) const override;

private:
    ActiveAllocationRecord* FindActiveAllocation(MemoryAllocationId allocation_id);
    ActiveAllocationRecord* FindFreeAllocationRecord();
    static void ResetAllocationRecord(ActiveAllocationRecord& record);

    std::array<ActiveAllocationRecord, MAX_COUNTING_MEMORY_TRACKER_ACTIVE_ALLOCATIONS> active_allocations_;
    std::array<std::uint64_t, MEMORY_BUDGET_CLASS_COUNT> budget_allocation_counts_;
    MemorySnapshot snapshot_;
    std::uint64_t next_allocation_id_;
    std::size_t active_allocation_count_;
};
}
