#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"

namespace yuengine::serialize::Tests {
struct StreamFixture final {
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> Buffer;
    std::uint32_t ByteCount = 0U;
    SerializeSnapshot Snapshot{};
};
}
