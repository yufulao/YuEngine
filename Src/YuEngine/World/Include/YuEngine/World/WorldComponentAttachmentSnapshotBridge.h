// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentSnapshotBridge.h

#pragma once

#include "YuEngine/World/WorldComponentAttachmentSnapshotBridgeDesc.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotBridgeSnapshot.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotResult.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotStatus.h"

namespace yuengine::serialize {
class SerializeReader;
class SerializeWriter;
}

namespace yuengine::world {
class WorldComponentAttachmentBridge;

class WorldComponentAttachmentSnapshotBridge final {
public:
    /**
     * @comment Constructs a world component attachment snapshot bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldComponentAttachmentSnapshotBridge(
        WorldComponentAttachmentSnapshotBridgeDesc desc=WorldComponentAttachmentSnapshotBridgeDesc{});

    /**
     * @comment Writes active attachment records through a caller-owned writer.
     * @param writer Serialize writer updated by the function.
     * @param source_bridge Source attachment bridge read by the function.
     * @return Explicit operation result.
     */
    WorldComponentAttachmentSnapshotResult WriteSnapshot(
        yuengine::serialize::SerializeWriter *writer,
        const WorldComponentAttachmentBridge *source_bridge);
    /**
     * @comment Reads attachment records through a caller-owned reader.
     * @param reader Serialize reader updated by the function.
     * @param destination_bridge Destination attachment bridge restored on success.
     * @return Explicit operation result.
     */
    WorldComponentAttachmentSnapshotResult ReadSnapshot(
        yuengine::serialize::SerializeReader *reader,
        WorldComponentAttachmentBridge *destination_bridge);
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldComponentAttachmentSnapshotBridgeSnapshot Snapshot() const;

private:
    WorldComponentAttachmentSnapshotResult RecordFailure(WorldComponentAttachmentSnapshotStatus status);
    WorldComponentAttachmentSnapshotResult RecordRejectedFailure(
        WorldComponentAttachmentSnapshotStatus status);
    WorldComponentAttachmentSnapshotResult RecordSerializeFailure(
        yuengine::serialize::SerializeStatus status);
    WorldComponentAttachmentSnapshotResult RecordWriteSuccess(
        const WorldComponentAttachmentSnapshotState &state);
    WorldComponentAttachmentSnapshotResult RecordReadSuccess(
        const WorldComponentAttachmentSnapshotState &state);
    WorldComponentAttachmentSnapshotStatus ValidateBridgeCapacity() const;
    WorldComponentAttachmentSnapshotStatus ValidateRecords(
        const WorldComponentAttachmentSnapshotRecord *records,
        std::uint32_t record_count) const;
    WorldComponentAttachmentSnapshotStatus RestoreRecords(
        WorldComponentAttachmentBridge *destination_bridge,
        const WorldComponentAttachmentSnapshotRecord *records,
        std::uint32_t record_count) const;

    std::uint32_t attachment_capacity_;
    WorldComponentAttachmentSnapshotBridgeSnapshot snapshot_;
};
}
