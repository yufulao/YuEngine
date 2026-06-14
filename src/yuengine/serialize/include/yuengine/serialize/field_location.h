#pragma once

#include <cstdint>

#include "yuengine/serialize/serialize_type_tag.h"

namespace yuengine::serialize {
struct FieldLocation final {
    SerializeTypeTag Type = SerializeTypeTag::UInt32;
    std::uint32_t PayloadOffset = 0U;
    std::uint32_t PayloadByteCount = 0U;
    bool Found = false;
};
}
