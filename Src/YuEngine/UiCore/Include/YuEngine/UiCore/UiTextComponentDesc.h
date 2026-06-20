// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiTextComponentDesc.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiFontGlyphAtlas.h"
#include "YuEngine/UiCore/UiImageTint.h"
#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
constexpr std::uint32_t MAX_UI_TEXT_CODEPOINT_COUNT = 128U;

enum class UiTextSourceType {
    PlainText,
    LocalizeKeyPlaceholder
};

enum class UiTextHorizontalAlignment {
    Left,
    Center,
    Right
};

enum class UiTextVerticalAlignment {
    Top,
    Middle,
    Bottom
};

enum class UiTextWrapMode {
    None,
    Character
};

enum class UiTextOverflowMode {
    Clip
};

enum class UiTextChangeReason {
    Content,
    PaintStyle,
    LayoutStyle
};

struct UiTextComponentDesc final {
    UiNodeId node_id;
    UiTextSourceType source_type = UiTextSourceType::PlainText;
    std::span<const std::uint32_t> text_codepoints;
    std::uint32_t localize_key = 0U;
    std::span<const std::uint32_t> localize_placeholder_codepoints;
    UiFontGlyphKey font_key;
    std::span<const std::uint32_t> fallback_font_keys;
    UiTextHorizontalAlignment horizontal_alignment = UiTextHorizontalAlignment::Left;
    UiTextVerticalAlignment vertical_alignment = UiTextVerticalAlignment::Top;
    UiTextWrapMode wrap_mode = UiTextWrapMode::None;
    UiTextOverflowMode overflow_mode = UiTextOverflowMode::Clip;
    UiTextChangeReason change_reason = UiTextChangeReason::Content;
    std::uint32_t text_key = 0U;
    std::uint32_t style_key = 0U;
    std::uint32_t material_key = 0U;
    float line_height_px = 0.0F;
    UiImageTint tint;
    bool outline_enabled = false;
    float outline_expand_px = 0.0F;
    UiImageTint outline_tint;
    bool shadow_enabled = false;
    float shadow_offset_x = 0.0F;
    float shadow_offset_y = 0.0F;
    UiImageTint shadow_tint;
    bool scissor_enabled = true;
};
}
