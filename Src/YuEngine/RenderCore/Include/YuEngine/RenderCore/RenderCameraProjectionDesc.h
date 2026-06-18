// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraProjectionDesc.h

#pragma once

#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"

namespace yuengine::rendercore {
struct RenderCameraProjectionDesc final {
    RenderCameraProjectionKind kind = RenderCameraProjectionKind::Perspective;
    float vertical_fov_radians = 1.0471975512F;
    float aspect_ratio = 1.0F;
    float near_z = 0.1F;
    float far_z = 1000.0F;
    float orthographic_height = 10.0F;
};
}
