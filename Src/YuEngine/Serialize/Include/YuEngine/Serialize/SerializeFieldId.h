// Module: YuEngine Serialize
// File: Src/YuEngine/Serialize/Include/YuEngine/Serialize/SerializeFieldId.h

#pragma once

#include <cstdint>

namespace yuengine::serialize {
struct SerializeFieldId final {
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
