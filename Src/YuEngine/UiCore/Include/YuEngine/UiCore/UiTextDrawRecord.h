// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiTextDrawRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiDrawElement.h"
#include "YuEngine/UiCore/UiImageTint.h"
#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"

namespace yuengine::uicore {
enum class UiTextEffectKind {
    Main,
    Outline,
    Shadow
};

struct UiTextDrawRecord final {
    UiDrawElement draw_element;
    UiStaticAtlasUvRect uv_rect;
    UiImageTint tint;
    UiTextEffectKind effect = UiTextEffectKind::Main;
    std::uint32_t codepoint = 0U;
    std::uint32_t codepoint_index = 0U;
    std::uint32_t font_key = 0U;
    std::uint32_t sprite_key = 0U;
};
}
