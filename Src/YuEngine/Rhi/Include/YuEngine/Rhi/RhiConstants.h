// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiConstants.h

#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::rhi {
constexpr std::size_t MAX_COLOR_TARGETS = 8U;
constexpr std::uint16_t MAX_COLOR_TARGET_EXTENT = 16U;
constexpr std::uint16_t MAX_CAPTURE_FIXTURE_EXTENT = 4U;
constexpr std::size_t MAX_COMMANDS = 32U;
constexpr std::size_t RGBA8_BYTES_PER_PIXEL = 4U;
constexpr std::size_t MAX_RHI_BUFFERS = 8U;
constexpr std::size_t MAX_RHI_TEXTURES = 8U;
constexpr std::size_t MAX_RHI_SAMPLERS = 8U;
constexpr std::size_t MAX_RHI_SAMPLED_TEXTURE_SLOTS = 4U;
constexpr std::size_t MAX_RHI_SAMPLER_SLOTS = 4U;
constexpr std::size_t MAX_RHI_SHADER_MODULES = 8U;
constexpr std::size_t MAX_RHI_PIPELINES = 4U;
constexpr std::size_t MAX_RHI_PRIMITIVE_RETIREMENTS = 16U;
constexpr std::size_t MAX_RHI_BUFFER_BYTES = 256U;
constexpr std::size_t MAX_RHI_SHADER_BYTECODE_BYTES = 8192U;
constexpr std::size_t RHI_CONSTANT_BUFFER_ALIGNMENT = 16U;
}
