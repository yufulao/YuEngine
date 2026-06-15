// Module: YuEngine Script
// File: Src/YuEngine/Script/Include/YuEngine/Script/ScriptCallId.h

#pragma once

#include <cstdint>

#include "YuEngine/Script/ScriptConstants.h"

namespace yuengine::script {
struct ScriptCallId final {
    std::uint32_t value = INVALID_SCRIPT_CALL_ID_VALUE;

    /**
     * @comment Checks whether the script call id is valid.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsValid() const {
        return value != INVALID_SCRIPT_CALL_ID_VALUE;
    }
};
}
