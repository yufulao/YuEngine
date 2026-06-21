// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneStatus.h

#pragma once

namespace yuengine::renderscene {
enum class RenderSceneStatus {
    Success,
    NullPointer,
    InvalidFrameId,
    MissingCamera,
    InvalidCameraRecord,
    InvalidCameraPose,
    InvalidCameraProjection,
    MissingEntity,
    MissingMeshAsset,
    MissingMaterialAsset,
    MissingTextureReadyRecord,
    InvalidEntityRecord,
    InvalidMaterialRecord,
    InvalidDrawRecord,
    OutputCapacityExceeded
};
}
