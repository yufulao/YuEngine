// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraPose.h

#pragma once

#include "YuEngine/RenderCore/RenderCameraVector3.h"

namespace yuengine::rendercore {
struct RenderCameraPose final {
    RenderCameraVector3 position{};
    RenderCameraVector3 target{0.0F, 0.0F, 1.0F};
    RenderCameraVector3 up{0.0F, 1.0F, 0.0F};
};
}
