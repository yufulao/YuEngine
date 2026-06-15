// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachment.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldComponentTypeId final {
    std::uint32_t value = 0U;

    /**
     * @comment Checks whether the component type id is valid.
     * @return True when the id is valid; false otherwise.
     */
    bool IsValid() const {
        return value != 0U;
    }
};

struct WorldComponentSlotId final {
    std::uint32_t value = 0U;

    /**
     * @comment Checks whether the component slot id is valid.
     * @return True when the id is valid; false otherwise.
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
