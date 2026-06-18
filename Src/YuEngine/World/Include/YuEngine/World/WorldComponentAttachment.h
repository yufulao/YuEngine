// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachment.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldComponentTypeId final {
    std::uint32_t value = 0U;

    /**
     * @comment 检查 component type id 是否有效。
     * @return id 有效时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return value != 0U;
    }
};

struct WorldComponentSlotId final {
    std::uint32_t value = 0U;

    /**
     * @comment 检查 component slot id 是否有效。
     * @return id 有效时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return value != 0U;
    }
};

struct WorldComponentAttachment final {
    WorldObjectId world_object_id{};
    WorldComponentTypeId component_type_id{};
    WorldComponentSlotId component_slot_id{};
    bool is_attached = false;
};
}
