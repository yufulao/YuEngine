// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeFrameRequest.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/RenderScene/RenderSceneCameraBindingResult.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameEntityRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRecord.h"

namespace yuengine::renderscene {
struct RenderSceneRuntimeFrameRequest final {
    std::uint32_t frame_id = 0U;
    RenderSceneCameraBindingResult camera{};
    RenderSceneRuntimeMaterialRecord material{};
    std::span<const RenderSceneRuntimeMaterialRecord> materials{};
    std::span<const RenderSceneRuntimeFrameEntityRequest> entities{};
};
}
