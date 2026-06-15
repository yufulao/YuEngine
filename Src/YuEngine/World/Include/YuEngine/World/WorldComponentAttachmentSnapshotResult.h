// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentSnapshotResult.h

#pragma once

#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotState.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotStatus.h"

namespace yuengine::world {
struct WorldComponentAttachmentSnapshotResult final {
    WorldComponentAttachmentSnapshotStatus status = WorldComponentAttachmentSnapshotStatus::Success;
    yuengine::serialize::SerializeStatus serialize_status = yuengine::serialize::SerializeStatus::Success;
    WorldComponentAttachmentSnapshotState state{};

    /**
     * @comment Creates a successful snapshot result.
     * @param state Input operation state.
     * @return Explicit operation result.
     */
    static WorldComponentAttachmentSnapshotResult Success(
        const WorldComponentAttachmentSnapshotState &state) {
        return WorldComponentAttachmentSnapshotResult{
            WorldComponentAttachmentSnapshotStatus::Success,
            yuengine::serialize::SerializeStatus::Success,
            state};
    }

    /**
     * @comment Creates a failed snapshot result.
     * @param status Input snapshot status.
     * @param serialize_status Input serialize status.
     * @return Explicit operation result.
     */
    static WorldComponentAttachmentSnapshotResult Failure(
        WorldComponentAttachmentSnapshotStatus status,
        yuengine::serialize::SerializeStatus serialize_status=yuengine::serialize::SerializeStatus::Success) {
        return WorldComponentAttachmentSnapshotResult{
            status,
            serialize_status,
            WorldComponentAttachmentSnapshotState{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldComponentAttachmentSnapshotStatus::Success;
    }
};
}
