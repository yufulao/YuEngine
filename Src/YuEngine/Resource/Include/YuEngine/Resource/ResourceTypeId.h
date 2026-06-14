#pragma once

#include <cstdint>

namespace yuengine::resource {
struct ResourceTypeId final {
    std::uint32_t value = 0U;

    bool IsValid() const {
        return value != 0U;
    }
};
}
