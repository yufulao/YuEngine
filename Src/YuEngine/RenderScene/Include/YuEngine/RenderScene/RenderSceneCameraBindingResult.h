// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneCameraBindingResult.h

#pragma once

#include "YuEngine/RenderScene/RenderSceneCameraCaptureMetadata.h"
#include "YuEngine/RenderScene/RenderSceneCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneStatus.h"

namespace yuengine::renderscene {
struct RenderSceneCameraBindingResult final {
    RenderSceneStatus status = RenderSceneStatus::Success;
    RenderSceneCameraRecord camera{};
    RenderSceneCameraCaptureMetadata capture{};
};
}
