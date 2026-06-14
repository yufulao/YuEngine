#pragma once

#include <cstdint>

#include "yuengine/object/object_type_id.h"

namespace yuengine::object {
struct object_descriptor_t final {
    object_type_id_t Type;
    std::uint32_t InitialReferenceCount = 0U;
};
}
