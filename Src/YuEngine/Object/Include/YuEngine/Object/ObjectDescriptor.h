#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectTypeId.h"

namespace yuengine::object {
struct ObjectDescriptor final {
    ObjectTypeId Type;
    std::uint32_t InitialReferenceCount = 0U;
};
}
