// Module: YuEngine Memory
// File: Src/YuEngine/Memory/Include/YuEngine/Memory/MemoryOwnerId.h

#pragma once

#include <string_view>

namespace yuengine::memory {
struct MemoryOwnerId {
    std::string_view value;

    /**
     * @comment Checks whether the value is valid.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsValid() const {
        return !value.empty();
    }
};
}
