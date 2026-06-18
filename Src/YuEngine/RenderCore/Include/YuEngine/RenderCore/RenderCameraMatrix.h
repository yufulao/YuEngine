// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraMatrix.h

#pragma once

#include <array>

namespace yuengine::rendercore {
struct RenderCameraMatrix final {
    std::array<float, 16U> values{};
};
}
