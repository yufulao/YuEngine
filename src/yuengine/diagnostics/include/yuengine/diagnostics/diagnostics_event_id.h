#pragma once

#include <cstdint>

namespace yuengine::diagnostics {
struct DiagnosticsEventId {
    std::uint32_t Value;

    bool IsValid() const {
        return Value != 0U;
    }
};
}
