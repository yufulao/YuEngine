// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSerializeSnapshotBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/World/WorldPhaseTrace.h"
#include "YuEngine/World/WorldSerializeSnapshotBridgeDesc.h"
#include "YuEngine/World/WorldSerializeSnapshotBridgeSnapshot.h"
#include "YuEngine/World/WorldSerializeSnapshotConstants.h"
#include "YuEngine/World/WorldSerializeSnapshotResult.h"
#include "YuEngine/World/WorldSerializeSnapshotStatus.h"
#include "YuEngine/World/WorldSnapshot.h"
#include "YuEngine/World/WorldTransformSnapshot.h"

namespace yuengine::serialize {
class SerializeReader;
class SerializeWriter;
}

namespace yuengine::world {
class WorldSerializeSnapshotBridge final {
public:
    /**
     * @comment Constructs a world serialize snapshot bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldSerializeSnapshotBridge(
        WorldSerializeSnapshotBridgeDesc desc=WorldSerializeSnapshotBridgeDesc{});

    /**
     * @comment Writes world snapshot records through a caller-owned writer.
     * @param writer Serialize writer updated by the function.
     * @param world_snapshot Input world snapshot.
     * @param phase_trace Input phase trace records.
     * @param phase_trace_count Input phase trace count.
     * @param transform_snapshot Optional transform counter snapshot.
     * @return Explicit operation result.
     */
    WorldSerializeSnapshotResult WriteSnapshot(
        yuengine::serialize::SerializeWriter *writer,
        const WorldSnapshot &world_snapshot,
        const WorldPhaseTrace *phase_trace,
        std::uint32_t phase_trace_count,
        const WorldTransformSnapshot *transform_snapshot=nullptr);
    /**
     * @comment Reads world snapshot records through a caller-owned reader.
     * @param reader Serialize reader updated by the function.
     * @param out_world_snapshot Output world snapshot written on success.
     * @param out_phase_trace Output phase trace records written on success.
     * @param out_phase_trace_capacity Output phase trace capacity.
     * @param out_phase_trace_count Output phase trace count written on success.
     * @param out_transform_snapshot Optional transform counter output written on success.
     * @return Explicit operation result.
     */
    WorldSerializeSnapshotResult ReadSnapshot(
        yuengine::serialize::SerializeReader *reader,
        WorldSnapshot *out_world_snapshot,
        WorldPhaseTrace *out_phase_trace,
        std::uint32_t out_phase_trace_capacity,
        std::uint32_t *out_phase_trace_count,
        WorldTransformSnapshot *out_transform_snapshot=nullptr);
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldSerializeSnapshotBridgeSnapshot Snapshot() const;

private:
    WorldSerializeSnapshotResult RecordFailure(WorldSerializeSnapshotStatus status);
    WorldSerializeSnapshotResult RecordSerializeFailure(yuengine::serialize::SerializeStatus status);
    WorldSerializeSnapshotResult RecordWriteSuccess(const WorldSerializeSnapshotState &state);
    WorldSerializeSnapshotResult RecordReadSuccess(const WorldSerializeSnapshotState &state);
    WorldSerializeSnapshotStatus ValidateWorldSnapshot(const WorldSnapshot &world_snapshot,
        std::uint32_t phase_trace_count) const;
    WorldSerializeSnapshotStatus ValidateTransformSnapshot(const WorldTransformSnapshot &transform_snapshot) const;
    WorldSerializeSnapshotStatus ValidatePhaseTrace(const WorldPhaseTrace *phase_trace,
        std::uint32_t phase_trace_count) const;
    WorldSerializeSnapshotStatus ValidateReadOutputs(WorldSnapshot *out_world_snapshot,
        WorldPhaseTrace *out_phase_trace,
        std::uint32_t out_phase_trace_capacity,
        std::uint32_t *out_phase_trace_count) const;

    std::uint32_t phase_trace_capacity_;
    WorldSerializeSnapshotBridgeSnapshot snapshot_;
};
}
