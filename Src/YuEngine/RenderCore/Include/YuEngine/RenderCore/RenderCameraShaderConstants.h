// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraShaderConstants.h

#pragma once

#include <array>
#include <cstddef>

namespace yuengine::rendercore {
/**
 * @comment Stores the compact camera constant block used by first-slice shaders.
 */
struct RenderCameraShaderConstants final {
    std::array<float, 16U> view_projection_values{};
};

/**
 * @comment Byte size of RenderCameraShaderConstants.
 */
constexpr std::size_t RENDER_CAMERA_SHADER_CONSTANT_BYTES = 64U;
}
