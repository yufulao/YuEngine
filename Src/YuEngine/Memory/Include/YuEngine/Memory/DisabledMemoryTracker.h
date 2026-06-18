// 模块: YuEngine Memory
// 文件: Src/YuEngine/Memory/Include/YuEngine/Memory/DisabledMemoryTracker.h

#pragma once

#include "YuEngine/Memory/IMemoryTracker.h"

namespace yuengine::memory {
class DisabledMemoryTracker final : public IMemoryTracker {
public:
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
};
}
