// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeFrameStatus.h

#pragma once

namespace yuengine::renderscene {
enum class RenderSceneRuntimeFrameStatus {
    Success,
    NullPointer,
    InvalidFrameId,
    MissingCamera,
    MissingEntity,
    InvalidEntityRecord,
    MissingGeometryRecord,
    InvalidGeometryRecord,
    MissingMaterialRecord,
    InvalidMaterialRecord,
    DuplicateWorldObject,
    DuplicateTransform,
    OutputCapacityExceeded
};
}
