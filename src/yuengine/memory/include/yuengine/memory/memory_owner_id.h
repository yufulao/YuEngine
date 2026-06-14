#pragma once

#include <string_view>

namespace yuengine::memory {
struct memory_owner_id_t {
    std::string_view Value;

    bool IsValid() const {
        return !Value.empty();
    }
};
}
