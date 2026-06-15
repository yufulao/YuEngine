// Module: YuEngine Object
// File: Src/YuEngine/Object/Include/YuEngine/Object/ObjectTypeId.h

#pragma once

#include <cstdint>

namespace yuengine::object {
struct ObjectTypeId final {
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
