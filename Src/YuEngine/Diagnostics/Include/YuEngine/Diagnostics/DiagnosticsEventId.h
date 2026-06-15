// Module: YuEngine Diagnostics
// File: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DiagnosticsEventId.h

#pragma once

#include <cstdint>

namespace yuengine::diagnostics {
struct DiagnosticsEventId {
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
