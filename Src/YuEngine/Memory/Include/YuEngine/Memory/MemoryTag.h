// 模块: YuEngine Memory
// 文件: Src/YuEngine/Memory/Include/YuEngine/Memory/MemoryTag.h

#pragma once

#include <string_view>

namespace yuengine::memory {
struct MemoryTag {
    std::string_view value;

    /**
     * @comment 检查值是否合法。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return !value.empty();
    }
};
}
