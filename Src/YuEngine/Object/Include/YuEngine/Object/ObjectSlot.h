// 模块: YuEngine Object
// 文件: Src/YuEngine/Object/Include/YuEngine/Object/ObjectSlot.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectConstants.h"
#include "YuEngine/Object/ObjectTypeId.h"

namespace yuengine::object {
struct ObjectSlot final {
    ObjectTypeId type;
    std::uint32_t generation = INVALID_OBJECT_GENERATION;
    std::uint32_t reference_count = 0U;
    bool is_active = false;
};
}
