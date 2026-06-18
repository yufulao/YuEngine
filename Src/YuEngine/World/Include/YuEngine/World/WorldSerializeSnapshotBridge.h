// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSerializeSnapshotBridge.h

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
     * @comment 构造 world serialize snapshot bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldSerializeSnapshotBridge(
        WorldSerializeSnapshotBridgeDesc desc=WorldSerializeSnapshotBridgeDesc{});

    /**
     * @comment 通过 caller-owned writer 写入 world snapshot records。
     * @param writer 函数更新的 Serialize writer。
     * @param world_snapshot 输入 world snapshot。
     * @param phase_trace 输入 phase trace records。
     * @param phase_trace_count 输入 phase trace count。
     * @param transform_snapshot 可选 transform counter snapshot。
     * @return 显式操作结果。
     */
    WorldSerializeSnapshotResult WriteSnapshot(
        yuengine::serialize::SerializeWriter *writer,
        const WorldSnapshot &world_snapshot,
        const WorldPhaseTrace *phase_trace,
        std::uint32_t phase_trace_count,
        const WorldTransformSnapshot *transform_snapshot=nullptr);
    /**
     * @comment 通过调用方持有的 reader 读取 world snapshot records。
     * @param reader 函数更新的 Serialize reader。
     * @param out_world_snapshot 成功时写入输出 world snapshot。
     * @param out_phase_trace 成功时写入输出 phase trace records。
     * @param out_phase_trace_capacity 输出 phase trace capacity。
     * @param out_phase_trace_count 成功时写入输出 phase trace count。
     * @param out_transform_snapshot 成功时写入的 optional transform counter output。
     * @return 显式操作结果。
     */
    WorldSerializeSnapshotResult ReadSnapshot(
        yuengine::serialize::SerializeReader *reader,
        WorldSnapshot *out_world_snapshot,
        WorldPhaseTrace *out_phase_trace,
        std::uint32_t out_phase_trace_capacity,
        std::uint32_t *out_phase_trace_count,
        WorldTransformSnapshot *out_transform_snapshot=nullptr);
    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
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
