// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiBlendUtility.h

#pragma once

#include <cstdint>

#include "YuEngine/Rhi/RhiBlendStateDesc.h"
#include "YuEngine/Rhi/RhiColor.h"

namespace yuengine::rhi {
constexpr std::uint8_t ClampRhiBlendByte(std::uint32_t value) {
    if (value > 255U) {
        return 255U;
    }

    return static_cast<std::uint8_t>(value);
}

constexpr RhiColor BlendRhiColor(
    RhiColor source,
    RhiColor destination,
    const RhiBlendStateDesc &blend_state) {
    if (blend_state.mode == RhiBlendMode::Opaque) {
        RhiColor result = source;
        result.a = 255U;
        return result;
    }

    const std::uint32_t source_alpha =
        (static_cast<std::uint32_t>(source.a) *
            static_cast<std::uint32_t>(blend_state.constant_alpha) + 127U) / 255U;
    const std::uint32_t inverse_alpha = 255U - source_alpha;
    RhiColor result{};
    result.r = ClampRhiBlendByte(
        (static_cast<std::uint32_t>(source.r) * source_alpha +
            static_cast<std::uint32_t>(destination.r) * inverse_alpha + 127U) / 255U);
    result.g = ClampRhiBlendByte(
        (static_cast<std::uint32_t>(source.g) * source_alpha +
            static_cast<std::uint32_t>(destination.g) * inverse_alpha + 127U) / 255U);
    result.b = ClampRhiBlendByte(
        (static_cast<std::uint32_t>(source.b) * source_alpha +
            static_cast<std::uint32_t>(destination.b) * inverse_alpha + 127U) / 255U);
    result.a = ClampRhiBlendByte(
        source_alpha +
            (static_cast<std::uint32_t>(destination.a) * inverse_alpha + 127U) / 255U);
    return result;
}
}
