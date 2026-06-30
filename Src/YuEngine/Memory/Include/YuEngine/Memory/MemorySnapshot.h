// 模块: YuEngine Memory
// 文件: Src/YuEngine/Memory/Include/YuEngine/Memory/MemorySnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"

namespace yuengine::memory {
struct MemorySnapshot {
    std::uint64_t allocation_count;
    std::uint64_t free_count;
    std::size_t retained_bytes;
    std::size_t peak_retained_bytes;
    std::size_t leak_count;
    MemoryAccountingStatus last_status = MemoryAccountingStatus::Success;

    /**
     * @comment 检查 保留的 allocations 保留.
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool HasLeaks() const {
        if (retained_bytes != 0U) {
            return true;
        }

        return leak_count != 0U;
    }
};
}
