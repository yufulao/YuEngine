#pragma once

#include <cstdint>

namespace yuengine::diagnostics {
struct DiagnosticsCounterId {
    std::uint32_t value;

    bool IsValid() const {
        return value != 0U;
    }
};
}
