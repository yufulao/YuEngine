// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneRecordValueStreamResult.h

#pragma once

#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/World/WorldSceneRecordValueStreamState.h"
#include "YuEngine/World/WorldSceneRecordValueStreamStatus.h"

namespace yuengine::world {
struct WorldSceneRecordValueStreamResult final {
    WorldSceneRecordValueStreamStatus status =
        WorldSceneRecordValueStreamStatus::Success;
    yuengine::serialize::SerializeStatus serialize_status =
        yuengine::serialize::SerializeStatus::Success;
    WorldSceneRecordValueStreamState state{};

    /**
     * @comment 创建成功 scene record value stream result。
     * @param state 输入 operation state。
     * @return 显式操作结果。
     */
    static WorldSceneRecordValueStreamResult Success(
        const WorldSceneRecordValueStreamState &state) {
        return WorldSceneRecordValueStreamResult{
            WorldSceneRecordValueStreamStatus::Success,
            yuengine::serialize::SerializeStatus::Success,
            state};
    }

    /**
     * @comment 创建失败 scene record value stream result。
     * @param status 输入 value stream status。
     * @param serialize_status 输入 serialize status。
     * @return 显式操作结果。
     */
    static WorldSceneRecordValueStreamResult Failure(
        WorldSceneRecordValueStreamStatus status,
        yuengine::serialize::SerializeStatus serialize_status=
            yuengine::serialize::SerializeStatus::Success) {
        return WorldSceneRecordValueStreamResult{
            status,
            serialize_status,
            WorldSceneRecordValueStreamState{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldSceneRecordValueStreamStatus::Success;
    }
};
}
