#pragma once

#include <cstdint>

namespace yuengine::serialize {
struct SerializeRecordId final {
    std::uint32_t Value = 0U;

    bool IsValid() const {
        return Value != 0U;
    }
};
}
