// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSerializeSnapshotResult.h

#pragma once

#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/World/WorldSerializeSnapshotState.h"
#include "YuEngine/World/WorldSerializeSnapshotStatus.h"

namespace yuengine::world {
struct WorldSerializeSnapshotResult final {
    WorldSerializeSnapshotStatus status = WorldSerializeSnapshotStatus::Success;
    yuengine::serialize::SerializeStatus serialize_status = yuengine::serialize::SerializeStatus::Success;
    WorldSerializeSnapshotState state{};

    /**
     * @comment Creates a successful result.
     * @param state Input operation state.
     * @return Explicit operation result.
     */
    static WorldSerializeSnapshotResult Success(const WorldSerializeSnapshotState &state) {
        return WorldSerializeSnapshotResult{
            WorldSerializeSnapshotStatus::Success,
            yuengine::serialize::SerializeStatus::Success,
            state};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input bridge status.
     * @param serialize_status Input serialize status.
     * @return Explicit operation result.
     */
    static WorldSerializeSnapshotResult Failure(WorldSerializeSnapshotStatus status,
        yuengine::serialize::SerializeStatus serialize_status=yuengine::serialize::SerializeStatus::Success) {
        return WorldSerializeSnapshotResult{status, serialize_status, WorldSerializeSnapshotState{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldSerializeSnapshotStatus::Success;
    }
};
}
