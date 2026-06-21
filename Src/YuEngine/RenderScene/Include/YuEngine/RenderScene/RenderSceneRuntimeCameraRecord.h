// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeCameraRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderCameraPose.h"
#include "YuEngine/RenderCore/RenderCameraProjectionDesc.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::renderscene {
struct RenderSceneRuntimeCameraRecord final {
    std::uint32_t camera_id = 0U;
    yuengine::rendercore::RenderCameraPose pose{};
    yuengine::rendercore::RenderCameraProjectionDesc projection{};
    yuengine::rhi::RhiTextureHandle target{};
    yuengine::rhi::RhiColor clear_color{};
    bool is_active = false;
};
}
