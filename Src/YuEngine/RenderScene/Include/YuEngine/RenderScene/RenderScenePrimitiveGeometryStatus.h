// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderScenePrimitiveGeometryStatus.h

#pragma once

namespace yuengine::renderscene {
enum class RenderScenePrimitiveGeometryStatus {
    Success,
    NullPointer,
    InvalidPrimitiveKind,
    InvalidSegmentCount,
    InvalidGeometryAsset,
    InvalidDrawRecord,
    MissingGeometryRecord
};
}
