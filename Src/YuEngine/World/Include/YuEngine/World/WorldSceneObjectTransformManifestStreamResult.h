// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformManifestStreamResult.h

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
     * @comment Creates a successful object-transform manifest stream result.
     * @param state Input operation state.
     * @return Explicit operation result.
     */
    static WorldSceneObjectTransformManifestStreamResult Success(
        const WorldSceneObjectTransformManifestStreamState &state) {
        return WorldSceneObjectTransformManifestStreamResult{
            WorldSceneObjectTransformManifestStreamStatus::Success,
            yuengine::serialize::SerializeStatus::Success,
            state};
    }

    /**
     * @comment Creates a failed object-transform manifest stream result.
     * @param status Input manifest stream status.
     * @param serialize_status Input serialize status.
     * @return Explicit operation result.
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
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldSceneObjectTransformManifestStreamStatus::Success;
    }
};
}
