#pragma once

#include <string_view>

namespace yuengine::memory {
struct MemoryTag {
    std::string_view value;

    bool IsValid() const {
        return !value.empty();
    }
};
}
