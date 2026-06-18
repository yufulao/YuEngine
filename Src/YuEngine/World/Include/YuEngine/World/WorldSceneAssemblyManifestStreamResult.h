// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyManifestStreamResult.h

#pragma once

#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamState.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamStatus.h"

namespace yuengine::world {
struct WorldSceneAssemblyManifestStreamResult final {
    WorldSceneAssemblyManifestStreamStatus status =
        WorldSceneAssemblyManifestStreamStatus::Success;
    yuengine::serialize::SerializeStatus serialize_status =
        yuengine::serialize::SerializeStatus::Success;
    WorldSceneAssemblyManifestStreamState state{};

    /**
     * @comment 创建成功 manifest stream result。
     * @param state 输入 operation state。
     * @return 显式操作结果。
     */
    static WorldSceneAssemblyManifestStreamResult Success(
        const WorldSceneAssemblyManifestStreamState &state) {
        return WorldSceneAssemblyManifestStreamResult{
            WorldSceneAssemblyManifestStreamStatus::Success,
            yuengine::serialize::SerializeStatus::Success,
            state};
    }

    /**
     * @comment 创建失败 manifest stream result。
     * @param status 输入 manifest stream status。
     * @param serialize_status 输入 serialize status。
     * @return 显式操作结果。
     */
    static WorldSceneAssemblyManifestStreamResult Failure(
        WorldSceneAssemblyManifestStreamStatus status,
        yuengine::serialize::SerializeStatus serialize_status=
            yuengine::serialize::SerializeStatus::Success) {
        return WorldSceneAssemblyManifestStreamResult{
            status,
            serialize_status,
            WorldSceneAssemblyManifestStreamState{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldSceneAssemblyManifestStreamStatus::Success;
    }
};
}
