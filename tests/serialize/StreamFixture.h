#pragma once

#include <array>
#include <cstdint>

#include "yuengine/serialize/serialize_constants.h"
#include "yuengine/serialize/serialize_snapshot.h"

namespace yuengine::serialize::tests {
struct StreamFixture final {
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> Buffer;
    std::uint32_t ByteCount = 0U;
    SerializeSnapshot Snapshot{};
};
}
