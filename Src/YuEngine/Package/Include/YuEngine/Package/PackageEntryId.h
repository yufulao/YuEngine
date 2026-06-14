#pragma once

#include <cstdint>

namespace yuengine::package {
struct PackageEntryId final {
    std::uint32_t value = 0U;

    bool IsValid() const {
        return value != 0U;
    }
};
}
