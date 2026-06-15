// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingSnapshotBridge.h

#pragma once

#include "YuEngine/World/WorldComponentResourceBinding.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotBridgeDesc.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotBridgeSnapshot.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotResult.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotStatus.h"

namespace yuengine::serialize {
class SerializeReader;
class SerializeWriter;
}

namespace yuengine::world {
class WorldComponentResourceBindingBridge;

class WorldComponentResourceBindingSnapshotBridge final {
public:
    /**
     * @comment Constructs a world component resource binding snapshot bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldComponentResourceBindingSnapshotBridge(
        WorldComponentResourceBindingSnapshotBridgeDesc desc=
            WorldComponentResourceBindingSnapshotBridgeDesc{});

    /**
     * @comment Writes active binding records through a caller-owned writer.
     * @param writer Serialize writer updated by the function.
     * @param source_bridge Source binding bridge read by the function.
     * @return Explicit operation result.
     */
    WorldComponentResourceBindingSnapshotResult WriteSnapshot(
        yuengine::serialize::SerializeWriter *writer,
        const WorldComponentResourceBindingBridge *source_bridge);
    /**
     * @comment Reads binding records through a caller-owned reader.
     * @param reader Serialize reader updated by the function.
     * @param output_bindings Caller-owned output binding records.
     * @param output_capacity Output binding record capacity.
     * @param out_binding_count Output read binding count written on success.
     * @return Explicit operation result.
     */
    WorldComponentResourceBindingSnapshotResult ReadSnapshot(
        yuengine::serialize::SerializeReader *reader,
        WorldComponentResourceBinding *output_bindings,
        std::uint32_t output_capacity,
        std::uint32_t *out_binding_count);
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldComponentResourceBindingSnapshotBridgeSnapshot Snapshot() const;

private:
    WorldComponentResourceBindingSnapshotResult RecordFailure(
        WorldComponentResourceBindingSnapshotStatus status);
    WorldComponentResourceBindingSnapshotResult RecordRejectedFailure(
        WorldComponentResourceBindingSnapshotStatus status);
    WorldComponentResourceBindingSnapshotResult RecordSerializeFailure(
        yuengine::serialize::SerializeStatus status);
    WorldComponentResourceBindingSnapshotResult RecordWriteSuccess(
        const WorldComponentResourceBindingSnapshotState &state);
    WorldComponentResourceBindingSnapshotResult RecordReadSuccess(
        const WorldComponentResourceBindingSnapshotState &state);
    WorldComponentResourceBindingSnapshotStatus ValidateBridgeCapacity() const;
    WorldComponentResourceBindingSnapshotStatus ValidateRecords(
        const WorldComponentResourceBindingSnapshotRecord *records,
        std::uint32_t record_count) const;
    void WriteOutputBindings(
        const WorldComponentResourceBindingSnapshotRecord *records,
        std::uint32_t record_count,
        WorldComponentResourceBinding *output_bindings) const;

    std::uint32_t binding_capacity_;
    WorldComponentResourceBindingSnapshotBridgeSnapshot snapshot_;
};
}
