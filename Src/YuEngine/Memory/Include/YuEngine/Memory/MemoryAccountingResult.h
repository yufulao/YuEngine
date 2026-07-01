// 模块: YuEngine Memory
// 文件: Src/YuEngine/Memory/Include/YuEngine/Memory/MemoryAccountingResult.h

#pragma once

#include <array>
#include <cstddef>

#include "YuEngine/Memory/ActiveAllocationRecord.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Memory/MemoryAllocationId.h"
#include "YuEngine/Memory/MemoryBudgetClass.h"

namespace yuengine::memory {
struct MemoryAccountingResult {
    MemoryAccountingStatus status = MemoryAccountingStatus::Success;
    MemoryAllocationId allocation_id{0U};
    std::size_t required_allocation_count = 0U;
    MemoryAccountingStatus capacity_entry_status = MemoryAccountingStatus::Success;
    std::size_t capacity_entry_requested_bytes = 0U;
    std::array<char, MAX_MEMORY_OWNER_ID_BYTES> capacity_entry_owner{};
    std::size_t capacity_entry_owner_length = 0U;
    std::size_t capacity_entry_owner_capacity = 0U;
    std::size_t capacity_entry_owner_required_bytes = 0U;
    std::array<char, MAX_MEMORY_TAG_BYTES> capacity_entry_tag{};
    std::size_t capacity_entry_tag_length = 0U;
    std::size_t capacity_entry_tag_capacity = 0U;
    std::size_t capacity_entry_tag_required_bytes = 0U;
    MemoryBudgetClass capacity_entry_budget_class = MemoryBudgetClass::Setup;
    std::size_t capacity_entry_allocation_capacity = 0U;
    std::size_t capacity_entry_active_allocation_count = 0U;
    std::size_t capacity_entry_retained_bytes = 0U;

    /**
     * @comment 创建成功结果。
     * @param allocation_id 输入 allocation id。
     * @return 显式操作结果。
     */
    static MemoryAccountingResult Success(MemoryAllocationId allocation_id) {
        MemoryAccountingResult result{};
        result.status = MemoryAccountingStatus::Success;
        result.allocation_id = allocation_id;
        return result;
    }

    /**
     * @comment 创建失败结果。
     * @param status 输入 状态。
     * @return 显式操作结果。
     */
    static MemoryAccountingResult Failure(MemoryAccountingStatus status) {
        MemoryAccountingResult result{};
        result.status = status;
        return result;
    }

    /**
     * @comment 检查结果是否成功。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == MemoryAccountingStatus::Success;
    }
};
}
