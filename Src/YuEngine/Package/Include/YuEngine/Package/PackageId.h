// Module: YuEngine Package
// File: Src/YuEngine/Package/Include/YuEngine/Package/PackageId.h

#pragma once

#include <cstdint>

namespace yuengine::package {
struct PackageId final {
    std::uint32_t value = 0U;

    /**
     * @comment Checks whether the value is valid.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsValid() const {
        return value != 0U;
    }
};
}
