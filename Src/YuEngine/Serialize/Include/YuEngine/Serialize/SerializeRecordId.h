#pragma once

#include <cstdint>

namespace yuengine::serialize {
struct SerializeRecordId final {
    std::uint32_t value = 0U;

    bool IsValid() const {
        return value != 0U;
    }
};
}
