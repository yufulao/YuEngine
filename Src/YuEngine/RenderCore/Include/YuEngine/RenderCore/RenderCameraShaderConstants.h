// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraShaderConstants.h

#pragma once

#include <array>
#include <cstddef>

namespace yuengine::rendercore {
/**
 * @comment 存储 first-slice shader 使用的紧凑 camera constant block。
 */
struct RenderCameraShaderConstants final {
    std::array<float, 16U> view_projection_values{};
};

/**
 * @comment RenderCameraShaderConstants 的字节大小。
 */
constexpr std::size_t RENDER_CAMERA_SHADER_CONSTANT_BYTES = 64U;
}
