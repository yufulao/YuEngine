// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiInputLayoutDesc.h

#pragma once

#include <array>
#include <cstddef>

namespace yuengine::rhi {
enum class RhiInputElementSemantic {
    Position,
    Color,
    TexCoord,
    Unsupported
};

enum class RhiInputElementFormat {
    Float32x2,
    Float32x3,
    Float32x4,
    Unsupported
};

struct RhiInputElementDesc final {
    RhiInputElementSemantic semantic = RhiInputElementSemantic::Unsupported;
    RhiInputElementFormat format = RhiInputElementFormat::Unsupported;
    std::size_t offset_bytes = 0U;
};

constexpr std::size_t MAX_RHI_INPUT_ELEMENTS = 2U;

struct RhiInputLayoutDesc final {
    std::array<RhiInputElementDesc, MAX_RHI_INPUT_ELEMENTS> elements{};
    std::size_t element_count = 0U;
    std::size_t stride_bytes = 0U;
};
}
