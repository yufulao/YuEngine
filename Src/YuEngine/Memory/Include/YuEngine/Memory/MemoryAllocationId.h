// 模块: YuEngine Memory
// 文件: Src/YuEngine/Memory/Include/YuEngine/Memory/MemoryAllocationId.h

#pragma once

#include <cstdint>

namespace yuengine::memory {
struct MemoryAllocationId {
    std::uint64_t value;

    /**
     * @comment 检查值是否合法。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return value != 0U;
    }
};
}
