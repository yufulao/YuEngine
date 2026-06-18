// 模块: YuEngine Memory
// 文件: Src/YuEngine/Memory/Include/YuEngine/Memory/MemoryAccountingResult.h

#pragma once

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Memory/MemoryAllocationId.h"

namespace yuengine::memory {
struct MemoryAccountingResult {
    MemoryAccountingStatus status;
    MemoryAllocationId allocation_id;

    /**
     * @comment 创建成功结果。
     * @param allocation_id 输入 allocation id。
     * @return 显式操作结果。
     */
    static MemoryAccountingResult Success(MemoryAllocationId allocation_id) {
        return MemoryAccountingResult{MemoryAccountingStatus::Success, allocation_id};
    }

    /**
     * @comment 创建失败结果。
     * @param status 输入 状态。
     * @return 显式操作结果。
     */
    static MemoryAccountingResult Failure(MemoryAccountingStatus status) {
        return MemoryAccountingResult{status, MemoryAllocationId{0U}};
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
