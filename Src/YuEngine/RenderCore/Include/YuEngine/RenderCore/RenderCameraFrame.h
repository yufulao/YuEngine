// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraFrame.h

#pragma once

#include "YuEngine/RenderCore/RenderCameraMatrix.h"

namespace yuengine::rendercore {
struct RenderCameraFrame final {
    RenderCameraMatrix view{};
    RenderCameraMatrix projection{};
    RenderCameraMatrix view_projection{};
};
}
