// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DiagnosticsEventId.h

#pragma once

#include <cstdint>

namespace yuengine::diagnostics {
struct DiagnosticsEventId {
    std::uint32_t value;

    /**
     * @comment 检查 value 是否有效。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return value != 0U;
    }
};
}
