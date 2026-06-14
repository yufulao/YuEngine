#pragma once

#include <cstdint>

namespace yuengine::diagnostics {
struct DiagnosticsCounterId {
    std::uint32_t Value;

    bool IsValid() const {
        return Value != 0U;
    }
};
}
