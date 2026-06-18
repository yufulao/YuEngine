// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingSnapshotBridge.h

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
     * @comment 构造 world component resource binding snapshot bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldComponentResourceBindingSnapshotBridge(
        WorldComponentResourceBindingSnapshotBridgeDesc desc=
            WorldComponentResourceBindingSnapshotBridgeDesc{});

    /**
     * @comment 通过调用方持有的 writer 写入 active binding records。
     * @param writer 函数更新的 Serialize writer。
     * @param source_bridge 函数读取的 source binding bridge。
     * @return 显式操作结果。
     */
    WorldComponentResourceBindingSnapshotResult WriteSnapshot(
        yuengine::serialize::SerializeWriter *writer,
        const WorldComponentResourceBindingBridge *source_bridge);
    /**
     * @comment 通过调用方持有的 reader 读取 binding records。
     * @param reader 函数更新的 Serialize reader。
     * @param output_bindings 调用方持有的 output binding records。
     * @param output_capacity 输出 binding record capacity。
     * @param out_binding_count 成功时写入输出 read binding count。
     * @return 显式操作结果。
     */
    WorldComponentResourceBindingSnapshotResult ReadSnapshot(
        yuengine::serialize::SerializeReader *reader,
        WorldComponentResourceBinding *output_bindings,
        std::uint32_t output_capacity,
        std::uint32_t *out_binding_count);
    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
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
