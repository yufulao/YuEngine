#pragma once

#include <cstdint>

namespace yuengine::diagnostics {
struct diagnostics_counter_id_t {
    std::uint32_t Value;

    bool IsValid() const {
        return Value != 0U;
    }
};
}
