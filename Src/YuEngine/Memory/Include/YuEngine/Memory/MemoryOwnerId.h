#pragma once

#include <string_view>

namespace yuengine::memory {
struct MemoryOwnerId {
    std::string_view value;

    bool IsValid() const {
        return !value.empty();
    }
};
}
