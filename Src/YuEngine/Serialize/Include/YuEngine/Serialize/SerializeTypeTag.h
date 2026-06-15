// Module: YuEngine Serialize
// File: Src/YuEngine/Serialize/Include/YuEngine/Serialize/SerializeTypeTag.h

#pragma once

#include <cstdint>

namespace yuengine::serialize {
enum class SerializeTypeTag : std::uint32_t {
    UInt32 = 1U,
    Int32 = 2U,
    UInt64 = 3U,
    Int64 = 4U,
    FixedBytes = 5U
};
}
