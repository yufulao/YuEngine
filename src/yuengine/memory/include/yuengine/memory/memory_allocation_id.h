#pragma once

#include <cstdint>

namespace yuengine::memory {
struct memory_allocation_id_t {
    std::uint64_t Value;

    bool IsValid() const {
        return Value != 0U;
    }
};
}
