// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentSnapshotResult.h

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
     * @comment 创建成功 snapshot result。
     * @param state 输入 operation state。
     * @return 显式操作结果。
     */
    static WorldComponentAttachmentSnapshotResult Success(
        const WorldComponentAttachmentSnapshotState &state) {
        return WorldComponentAttachmentSnapshotResult{
            WorldComponentAttachmentSnapshotStatus::Success,
            yuengine::serialize::SerializeStatus::Success,
            state};
    }

    /**
     * @comment 创建失败 snapshot result。
     * @param status 输入 snapshot status。
     * @param serialize_status 输入 serialize status。
     * @return 显式操作结果。
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
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldComponentAttachmentSnapshotStatus::Success;
    }
};
}
