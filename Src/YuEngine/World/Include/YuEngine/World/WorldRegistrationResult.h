// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldRegistrationResult.h

#pragma once

#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldStatus.h"

namespace yuengine::world {
struct WorldRegistrationResult final {
    WorldStatus status = WorldStatus::Success;
    WorldObjectId id{};

    /**
     * @comment Creates a successful result.
     * @param id Input world object id.
     * @return Explicit operation result.
     */
    static WorldRegistrationResult Success(WorldObjectId id) {
        return WorldRegistrationResult{WorldStatus::Success, id};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input status.
     * @return Explicit operation result.
     */
    static WorldRegistrationResult Failure(WorldStatus status) {
        return WorldRegistrationResult{status, WorldObjectId{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldStatus::Success;
    }
};
}
