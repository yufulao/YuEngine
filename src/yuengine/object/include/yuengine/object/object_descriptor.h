#pragma once

#include <cstdint>

#include "yuengine/object/object_type_id.h"

namespace yuengine::object {
struct ObjectDescriptor final {
    ObjectTypeId Type;
    std::uint32_t InitialReferenceCount = 0U;
};
}
