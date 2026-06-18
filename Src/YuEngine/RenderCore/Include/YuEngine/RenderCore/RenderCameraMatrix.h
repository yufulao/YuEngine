// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraMatrix.h

#pragma once

#include <array>

namespace yuengine::rendercore {
struct RenderCameraMatrix final {
    std::array<float, 16U> values{};
};
}
