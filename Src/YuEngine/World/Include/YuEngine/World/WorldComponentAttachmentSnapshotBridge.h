// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentSnapshotBridge.h

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
     * @comment 构造 world component attachment snapshot bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldComponentAttachmentSnapshotBridge(
        WorldComponentAttachmentSnapshotBridgeDesc desc=WorldComponentAttachmentSnapshotBridgeDesc{});

    /**
     * @comment 通过调用方持有的 writer 写入 active attachment records。
     * @param writer 函数更新的 Serialize writer。
     * @param source_bridge 函数读取的 source attachment bridge。
     * @return 显式操作结果。
     */
    WorldComponentAttachmentSnapshotResult WriteSnapshot(
        yuengine::serialize::SerializeWriter *writer,
        const WorldComponentAttachmentBridge *source_bridge);
    /**
     * @comment 通过调用方持有的 reader 读取 attachment records。
     * @param reader 函数更新的 Serialize reader。
     * @param destination_bridge 成功时恢复的 destination attachment bridge。
     * @return 显式操作结果。
     */
    WorldComponentAttachmentSnapshotResult ReadSnapshot(
        yuengine::serialize::SerializeReader *reader,
        WorldComponentAttachmentBridge *destination_bridge);
    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
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
