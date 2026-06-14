#pragma once

#include <cstdint>

#include "yuengine/object/ObjectConstants.h"

namespace yuengine::object
{
struct ObjectHandle final
{
    std::uint32_t Slot = INVALID_OBJECT_SLOT;
    std::uint32_t Generation = INVALID_OBJECT_GENERATION;

    bool IsValid() const
    {
        if (Slot == INVALID_OBJECT_SLOT)
        {
            return false;
        }

        return Generation != INVALID_OBJECT_GENERATION;
    }
};
}
