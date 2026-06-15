// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceTypeId.h

#pragma once

#include <cstdint>

namespace yuengine::resource {
struct ResourceTypeId final {
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
