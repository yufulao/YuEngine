#pragma once

#include <cstdint>

#include "yuengine/serialize/SerializeTypeTag.h"

namespace yuengine::serialize
{
struct FieldLocation final
{
    SerializeTypeTag Type = SerializeTypeTag::UInt32;
    std::uint32_t PayloadOffset = 0U;
    std::uint32_t PayloadByteCount = 0U;
    bool Found = false;
};
}
