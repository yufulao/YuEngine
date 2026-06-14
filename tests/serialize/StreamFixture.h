#pragma once

#include <array>
#include <cstdint>

#include "yuengine/serialize/SerializeConstants.h"
#include "yuengine/serialize/SerializeSnapshot.h"

namespace yuengine::serialize::tests
{
struct StreamFixture final
{
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> Buffer;
    std::uint32_t ByteCount = 0U;
    SerializeSnapshot Snapshot{};
};
}
