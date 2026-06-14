#pragma once

#include <cstdint>

namespace yuengine::resource {
struct ResourceTypeId final {
    std::uint32_t Value = 0U;

    bool IsValid() const {
        return Value != 0U;
    }
};
}
