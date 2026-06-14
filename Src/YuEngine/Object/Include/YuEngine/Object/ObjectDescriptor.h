#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectTypeId.h"

namespace yuengine::object {
struct ObjectDescriptor final {
    ObjectTypeId type;
    std::uint32_t initial_reference_count = 0U;
};
}
