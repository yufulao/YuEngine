#pragma once

#include <cstdint>

#include "yuengine/serialize/serialize_type_tag.h"

namespace yuengine::serialize {
struct FieldLocation final {
    SERIALIZE_TYPE_TAG Type = SERIALIZE_TYPE_TAG::UInt32;
    std::uint32_t PayloadOffset = 0U;
    std::uint32_t PayloadByteCount = 0U;
    bool Found = false;
};
}
