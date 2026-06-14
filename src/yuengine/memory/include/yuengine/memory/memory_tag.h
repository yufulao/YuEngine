#pragma once

#include <string_view>

namespace yuengine::memory {
struct MemoryTag {
    std::string_view Value;

    bool IsValid() const {
        return !Value.empty();
    }
};
}
