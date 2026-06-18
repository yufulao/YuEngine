// 模块：Tests Serialize
// 文件：Tests/Serialize/StreamFixture.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"

namespace yuengine::serialize::Tests {
struct StreamFixture final {
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer;
    std::uint32_t byte_count = 0U;
    SerializeSnapshot snapshot{};
};
}
