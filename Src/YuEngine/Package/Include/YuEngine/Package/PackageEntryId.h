// Module: YuEngine Package
// File: Src/YuEngine/Package/Include/YuEngine/Package/PackageEntryId.h

#pragma once

#include <cstdint>

namespace yuengine::package {
struct PackageEntryId final {
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
