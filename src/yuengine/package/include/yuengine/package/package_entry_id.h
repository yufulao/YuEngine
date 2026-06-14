#pragma once

#include <cstdint>

namespace yuengine::package {
struct package_entry_id_t final {
    std::uint32_t Value = 0U;

    bool IsValid() const {
        return Value != 0U;
    }
};
}
