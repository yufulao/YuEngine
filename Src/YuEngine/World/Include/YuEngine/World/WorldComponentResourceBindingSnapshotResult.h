// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingSnapshotResult.h

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
     * @comment 创建成功 snapshot result。
     * @param state 输入 operation state。
     * @return 显式操作结果。
     */
    static WorldComponentResourceBindingSnapshotResult Success(
        const WorldComponentResourceBindingSnapshotState &state) {
        return WorldComponentResourceBindingSnapshotResult{
            WorldComponentResourceBindingSnapshotStatus::Success,
            yuengine::serialize::SerializeStatus::Success,
            state};
    }

    /**
     * @comment 创建失败 snapshot result。
     * @param status 输入 snapshot status。
     * @param serialize_status 输入 serialize status。
     * @return 显式操作结果。
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
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldComponentResourceBindingSnapshotStatus::Success;
    }
};
}
