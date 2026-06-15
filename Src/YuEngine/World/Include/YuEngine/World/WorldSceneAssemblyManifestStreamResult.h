// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyManifestStreamResult.h

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
     * @comment Creates a successful manifest stream result.
     * @param state Input operation state.
     * @return Explicit operation result.
     */
    static WorldSceneAssemblyManifestStreamResult Success(
        const WorldSceneAssemblyManifestStreamState &state) {
        return WorldSceneAssemblyManifestStreamResult{
            WorldSceneAssemblyManifestStreamStatus::Success,
            yuengine::serialize::SerializeStatus::Success,
            state};
    }

    /**
     * @comment Creates a failed manifest stream result.
     * @param status Input manifest stream status.
     * @param serialize_status Input serialize status.
     * @return Explicit operation result.
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
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldSceneAssemblyManifestStreamStatus::Success;
    }
};
}
