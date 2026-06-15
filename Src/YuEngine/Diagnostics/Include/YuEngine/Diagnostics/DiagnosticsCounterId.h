// Module: YuEngine Diagnostics
// File: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DiagnosticsCounterId.h

#pragma once

#include <cstdint>

namespace yuengine::diagnostics {
struct DiagnosticsCounterId {
    std::uint32_t value;

    /**
     * @comment Checks whether the value is valid.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsValid() const {
        return value != 0U;
    }
};
}
