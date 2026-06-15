// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingSnapshotResult.h

#pragma once

#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotState.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotStatus.h"

namespace yuengine::world {
struct WorldComponentResourceBindingSnapshotResult final {
    WorldComponentResourceBindingSnapshotStatus status =
        WorldComponentResourceBindingSnapshotStatus::Success;
    yuengine::serialize::SerializeStatus serialize_status =
        yuengine::serialize::SerializeStatus::Success;
    WorldComponentResourceBindingSnapshotState state{};

    /**
     * @comment Creates a successful snapshot result.
     * @param state Input operation state.
     * @return Explicit operation result.
     */
    static WorldComponentResourceBindingSnapshotResult Success(
        const WorldComponentResourceBindingSnapshotState &state) {
        return WorldComponentResourceBindingSnapshotResult{
            WorldComponentResourceBindingSnapshotStatus::Success,
            yuengine::serialize::SerializeStatus::Success,
            state};
    }

    /**
     * @comment Creates a failed snapshot result.
     * @param status Input snapshot status.
     * @param serialize_status Input serialize status.
     * @return Explicit operation result.
     */
    static WorldComponentResourceBindingSnapshotResult Failure(
        WorldComponentResourceBindingSnapshotStatus status,
        yuengine::serialize::SerializeStatus serialize_status=yuengine::serialize::SerializeStatus::Success) {
        return WorldComponentResourceBindingSnapshotResult{
            status,
            serialize_status,
            WorldComponentResourceBindingSnapshotState{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldComponentResourceBindingSnapshotStatus::Success;
    }
};
}
