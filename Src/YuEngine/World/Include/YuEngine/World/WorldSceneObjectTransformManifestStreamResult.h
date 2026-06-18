// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformManifestStreamResult.h

#pragma once

#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamState.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamStatus.h"

namespace yuengine::world {
struct WorldSceneObjectTransformManifestStreamResult final {
    WorldSceneObjectTransformManifestStreamStatus status =
        WorldSceneObjectTransformManifestStreamStatus::Success;
    yuengine::serialize::SerializeStatus serialize_status =
        yuengine::serialize::SerializeStatus::Success;
    WorldSceneObjectTransformManifestStreamState state{};

    /**
     * @comment 创建成功 object-transform manifest stream result。
     * @param state 输入 operation state。
     * @return 显式操作结果。
     */
    static WorldSceneObjectTransformManifestStreamResult Success(
        const WorldSceneObjectTransformManifestStreamState &state) {
        return WorldSceneObjectTransformManifestStreamResult{
            WorldSceneObjectTransformManifestStreamStatus::Success,
            yuengine::serialize::SerializeStatus::Success,
            state};
    }

    /**
     * @comment 创建失败 object-transform manifest stream result。
     * @param status 输入 manifest stream status。
     * @param serialize_status 输入 serialize status。
     * @return 显式操作结果。
     */
    static WorldSceneObjectTransformManifestStreamResult Failure(
        WorldSceneObjectTransformManifestStreamStatus status,
        yuengine::serialize::SerializeStatus serialize_status=
            yuengine::serialize::SerializeStatus::Success) {
        return WorldSceneObjectTransformManifestStreamResult{
            status,
            serialize_status,
            WorldSceneObjectTransformManifestStreamState{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldSceneObjectTransformManifestStreamStatus::Success;
    }
};
}
