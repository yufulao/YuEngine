// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptCallId.h

#pragma once

#include <cstdint>

#include "YuEngine/Script/ScriptConstants.h"

namespace yuengine::script {
struct ScriptCallId final {
    std::uint32_t value = INVALID_SCRIPT_CALL_ID_VALUE;

    /**
     * @comment 检查 script call id 是否有效。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return value != INVALID_SCRIPT_CALL_ID_VALUE;
    }
};
}
