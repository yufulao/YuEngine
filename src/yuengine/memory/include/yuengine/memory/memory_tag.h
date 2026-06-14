#pragma once

#include <string_view>

namespace yuengine::memory {
struct memory_tag_t {
    std::string_view Value;

    bool IsValid() const {
        return !Value.empty();
    }
};
}
