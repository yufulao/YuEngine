// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSerializeSnapshotResult.h

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
     * @comment 创建成功 result。
     * @param state 输入 operation state。
     * @return 显式操作结果。
     */
    static WorldSerializeSnapshotResult Success(const WorldSerializeSnapshotState &state) {
        return WorldSerializeSnapshotResult{
            WorldSerializeSnapshotStatus::Success,
            yuengine::serialize::SerializeStatus::Success,
            state};
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 bridge status。
     * @param serialize_status 输入 serialize status。
     * @return 显式操作结果。
     */
    static WorldSerializeSnapshotResult Failure(WorldSerializeSnapshotStatus status,
        yuengine::serialize::SerializeStatus serialize_status=yuengine::serialize::SerializeStatus::Success) {
        return WorldSerializeSnapshotResult{status, serialize_status, WorldSerializeSnapshotState{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldSerializeSnapshotStatus::Success;
    }
};
}
