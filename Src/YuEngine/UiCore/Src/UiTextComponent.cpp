// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiTextComponent.cpp

#include "YuEngine/UiCore/UiTextComponent.h"

#include <array>
#include <cstddef>

#include "YuEngine/UiCore/UiDrawElementType.h"
#include "YuEngine/UiCore/UiNodeTreeResult.h"

namespace yuengine::uicore {
namespace {
constexpr std::uint32_t NEWLINE_CODEPOINT = 10U;

struct UiTextGlyphPlacement final {
    UiFontGlyphResolveResult glyph;
    std::uint32_t codepoint_index = 0U;
    std::uint32_t line_index = 0U;
    float relative_x = 0.0F;
    float relative_y = 0.0F;
    float advance_x = 0.0F;
};

bool IsFontStyleKnown(UiFontStyle style) {
    switch (style) {
        case UiFontStyle::Regular:
            return true;
        case UiFontStyle::Bold:
            return true;
        case UiFontStyle::Italic:
            return true;
        case UiFontStyle::BoldItalic:
            return true;
        default:
            break;
    }

    return false;
}

bool IsSourceTypeKnown(UiTextSourceType source_type) {
    switch (source_type) {
        case UiTextSourceType::PlainText:
            return true;
        case UiTextSourceType::LocalizeKeyPlaceholder:
            return true;
        default:
            break;
    }

    return false;
}

bool IsHorizontalAlignmentKnown(UiTextHorizontalAlignment alignment) {
    switch (alignment) {
        case UiTextHorizontalAlignment::Left:
            return true;
        case UiTextHorizontalAlignment::Center:
            return true;
        case UiTextHorizontalAlignment::Right:
            return true;
        default:
            break;
    }

    return false;
}

bool IsVerticalAlignmentKnown(UiTextVerticalAlignment alignment) {
    switch (alignment) {
        case UiTextVerticalAlignment::Top:
            return true;
        case UiTextVerticalAlignment::Middle:
            return true;
        case UiTextVerticalAlignment::Bottom:
            return true;
        default:
            break;
    }

    return false;
}

bool IsWrapModeKnown(UiTextWrapMode mode) {
    switch (mode) {
        case UiTextWrapMode::None:
            return true;
        case UiTextWrapMode::Character:
            return true;
        default:
            break;
    }

    return false;
}

bool IsOverflowModeKnown(UiTextOverflowMode mode) {
    switch (mode) {
        case UiTextOverflowMode::Clip:
            return true;
        default:
            break;
    }

    return false;
}

bool IsChangeReasonKnown(UiTextChangeReason reason) {
    switch (reason) {
        case UiTextChangeReason::Content:
            return true;
        case UiTextChangeReason::PaintStyle:
            return true;
        case UiTextChangeReason::LayoutStyle:
            return true;
        default:
            break;
    }

    return false;
}

bool IsTintChannelValid(float value) {
    if (value < 0.0F) {
        return false;
    }

    return value <= 1.0F;
}

bool IsTintValid(const UiImageTint &tint) {
    if (!IsTintChannelValid(tint.red)) {
        return false;
    }

    if (!IsTintChannelValid(tint.green)) {
        return false;
    }

    if (!IsTintChannelValid(tint.blue)) {
        return false;
    }

    return IsTintChannelValid(tint.alpha);
}

bool IsGlyphKeyValid(const UiFontGlyphKey &key) {
    if (key.font_key == 0U || key.codepoint == 0U || key.size_px == 0U) {
        return false;
    }

    return IsFontStyleKnown(key.style);
}

std::span<const std::uint32_t> SelectSourceCodepoints(const UiTextComponentDesc &desc) {
    if (desc.source_type == UiTextSourceType::LocalizeKeyPlaceholder) {
        return desc.localize_placeholder_codepoints;
    }

    return desc.text_codepoints;
}

UiTextComponentStatus ValidateCodepoints(std::span<const std::uint32_t> codepoints) {
    if (codepoints.size() == 0U) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (codepoints.size() > static_cast<std::size_t>(MAX_UI_TEXT_CODEPOINT_COUNT)) {
        return UiTextComponentStatus::TextCapacityExceeded;
    }

    if (codepoints.data() == nullptr) {
        return UiTextComponentStatus::InvalidDesc;
    }

    return UiTextComponentStatus::Success;
}

UiTextComponentStatus ValidateOutputStorage(
    std::span<UiTextDrawRecord> out_records,
    std::uint32_t record_count) {
    if (record_count == 0U) {
        return UiTextComponentStatus::Success;
    }

    if (out_records.size() < static_cast<std::size_t>(record_count)) {
        return UiTextComponentStatus::OutputCapacityExceeded;
    }

    if (out_records.data() == nullptr) {
        return UiTextComponentStatus::InvalidOutputBuffer;
    }

    return UiTextComponentStatus::Success;
}

UiTextComponentStatus MapGlyphStatus(UiFontGlyphAtlasStatus status) {
    if (status == UiFontGlyphAtlasStatus::Success) {
        return UiTextComponentStatus::Success;
    }

    if (status == UiFontGlyphAtlasStatus::MissingGlyph) {
        return UiTextComponentStatus::FontGlyphMissing;
    }

    if (status == UiFontGlyphAtlasStatus::InvalidGlyphKey) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (status == UiFontGlyphAtlasStatus::FallbackCapacityExceeded) {
        return UiTextComponentStatus::InvalidDesc;
    }

    return UiTextComponentStatus::InvalidFontGlyphAtlas;
}

float ResolveGlyphAdvance(const UiFontGlyphResolveResult &glyph) {
    if (glyph.advance_x > 0) {
        return static_cast<float>(glyph.advance_x);
    }

    return static_cast<float>(glyph.width);
}

bool IsRectIntersecting(const UiRect &rect, const UiRect &clip_rect) {
    const float rect_right = rect.x + rect.width;
    const float rect_bottom = rect.y + rect.height;
    const float clip_right = clip_rect.x + clip_rect.width;
    const float clip_bottom = clip_rect.y + clip_rect.height;
    if (rect_right <= clip_rect.x) {
        return false;
    }

    if (rect_bottom <= clip_rect.y) {
        return false;
    }

    if (rect.x >= clip_right) {
        return false;
    }

    return rect.y < clip_bottom;
}

bool IsRectInside(const UiRect &rect, const UiRect &clip_rect) {
    const float rect_right = rect.x + rect.width;
    const float rect_bottom = rect.y + rect.height;
    const float clip_right = clip_rect.x + clip_rect.width;
    const float clip_bottom = clip_rect.y + clip_rect.height;
    if (rect.x < clip_rect.x || rect.y < clip_rect.y) {
        return false;
    }

    if (rect_right > clip_right) {
        return false;
    }

    return rect_bottom <= clip_bottom;
}

float ResolveHorizontalOffset(
    UiTextHorizontalAlignment alignment,
    float content_width,
    float line_width) {
    if (content_width <= line_width) {
        return 0.0F;
    }

    if (alignment == UiTextHorizontalAlignment::Center) {
        return (content_width - line_width) * 0.5F;
    }

    if (alignment == UiTextHorizontalAlignment::Right) {
        return content_width - line_width;
    }

    return 0.0F;
}

float ResolveVerticalOffset(
    UiTextVerticalAlignment alignment,
    float content_height,
    float block_height) {
    if (content_height <= block_height) {
        return 0.0F;
    }

    if (alignment == UiTextVerticalAlignment::Middle) {
        return (content_height - block_height) * 0.5F;
    }

    if (alignment == UiTextVerticalAlignment::Bottom) {
        return content_height - block_height;
    }

    return 0.0F;
}

UiRect BuildPlacementRect(
    const UiNodeRecord &node_record,
    const UiTextComponentDesc &desc,
    const UiTextGlyphPlacement &placement,
    std::span<const float> line_widths,
    std::uint32_t line_count) {
    const float line_width = line_widths[placement.line_index];
    const float block_height = static_cast<float>(line_count) * desc.line_height_px;
    const float horizontal_offset = ResolveHorizontalOffset(
        desc.horizontal_alignment,
        node_record.content_rect.width,
        line_width);
    const float vertical_offset = ResolveVerticalOffset(
        desc.vertical_alignment,
        node_record.content_rect.height,
        block_height);

    UiRect rect{};
    rect.x = node_record.content_rect.x + horizontal_offset + placement.relative_x;
    rect.y = node_record.content_rect.y + vertical_offset + placement.relative_y;
    rect.width = static_cast<float>(placement.glyph.width);
    rect.height = static_cast<float>(placement.glyph.height);
    return rect;
}

std::uint32_t CountEffectRecords(const UiTextComponentDesc &desc) {
    std::uint32_t count = 1U;
    if (desc.outline_enabled) {
        ++count;
    }

    if (desc.shadow_enabled) {
        ++count;
    }

    return count;
}

UiTextComponentStatus BuildPlacements(
    const UiFontGlyphAtlasDesc &font_atlas,
    const UiTextComponentDesc &desc,
    const UiNodeRecord &node_record,
    std::span<UiTextGlyphPlacement> placements,
    std::span<float> line_widths,
    UiTextComponentResult *out_result,
    std::uint32_t *out_placement_count,
    std::uint32_t *out_line_count) {
    const std::span<const std::uint32_t> codepoints = SelectSourceCodepoints(desc);
    UiTextComponentStatus status = ValidateCodepoints(codepoints);
    if (status != UiTextComponentStatus::Success) {
        return status;
    }

    UiFontGlyphAtlas atlas{};
    std::uint32_t placement_count = 0U;
    std::uint32_t line_index = 0U;
    float current_x = 0.0F;
    line_widths[0U] = 0.0F;

    const std::size_t codepoint_count = codepoints.size();
    for (std::size_t index = 0U; index < codepoint_count; ++index) {
        const std::uint32_t codepoint = codepoints[index];
        if (codepoint == NEWLINE_CODEPOINT) {
            ++line_index;
            if (line_index >= static_cast<std::uint32_t>(line_widths.size())) {
                return UiTextComponentStatus::TextCapacityExceeded;
            }

            line_widths[line_index] = 0.0F;
            current_x = 0.0F;
            continue;
        }

        UiFontGlyphResolveRequest request{};
        request.key = desc.font_key;
        request.key.codepoint = codepoint;
        request.fallback_font_keys = desc.fallback_font_keys;
        const UiFontGlyphResolveResult glyph = atlas.ResolveGlyph(font_atlas, request);
        status = MapGlyphStatus(glyph.status);
        if (status != UiTextComponentStatus::Success) {
            out_result->missing_codepoint = codepoint;
            return status;
        }

        const float glyph_advance = ResolveGlyphAdvance(glyph);
        const bool should_wrap = desc.wrap_mode == UiTextWrapMode::Character &&
            current_x > 0.0F &&
            (current_x + glyph_advance) > node_record.content_rect.width;
        if (should_wrap) {
            ++line_index;
            if (line_index >= static_cast<std::uint32_t>(line_widths.size())) {
                return UiTextComponentStatus::TextCapacityExceeded;
            }

            line_widths[line_index] = 0.0F;
            current_x = 0.0F;
        }

        if (placement_count >= static_cast<std::uint32_t>(placements.size())) {
            return UiTextComponentStatus::TextCapacityExceeded;
        }

        UiTextGlyphPlacement placement{};
        placement.glyph = glyph;
        placement.codepoint_index = static_cast<std::uint32_t>(index);
        placement.line_index = line_index;
        placement.relative_x = current_x;
        placement.relative_y = static_cast<float>(line_index) * desc.line_height_px;
        placement.advance_x = glyph_advance;
        placements[placement_count] = placement;
        ++placement_count;

        current_x += glyph_advance;
        line_widths[line_index] = current_x;
    }

    *out_placement_count = placement_count;
    *out_line_count = line_index + 1U;
    out_result->source_codepoint_count = static_cast<std::uint32_t>(codepoint_count);
    out_result->line_count = *out_line_count;
    return UiTextComponentStatus::Success;
}

std::uint32_t CountRequiredRecords(
    const UiNodeRecord &node_record,
    const UiTextComponentDesc &desc,
    std::span<const UiTextGlyphPlacement> placements,
    std::span<const float> line_widths,
    std::uint32_t line_count,
    UiTextComponentResult *out_result) {
    std::uint32_t required_count = 0U;
    const std::uint32_t effect_count = CountEffectRecords(desc);
    for (const UiTextGlyphPlacement &placement : placements) {
        const UiRect rect = BuildPlacementRect(node_record, desc, placement, line_widths, line_count);
        if (!IsRectInside(rect, node_record.content_rect)) {
            out_result->overflowed = true;
        }

        if (!IsRectIntersecting(rect, node_record.content_rect)) {
            continue;
        }

        ++out_result->visible_codepoint_count;
        required_count += effect_count;
    }

    return required_count;
}

UiTextDrawRecord BuildRecord(
    const UiNodeRecord &node_record,
    const UiTextComponentDesc &desc,
    const UiTextGlyphPlacement &placement,
    UiRect rect,
    UiTextEffectKind effect,
    UiImageTint tint) {
    UiTextDrawRecord record{};
    record.draw_element.node_id = node_record.node_id;
    record.draw_element.type = UiDrawElementType::Text;
    record.draw_element.rect = rect;
    record.draw_element.clip_rect = node_record.content_rect;
    record.draw_element.layer = node_record.layer;
    record.draw_element.sibling_order = node_record.sibling_order;
    record.draw_element.style_key = desc.style_key;
    record.draw_element.material_key = desc.material_key;
    record.draw_element.texture_key = placement.glyph.texture_key;
    record.draw_element.text_key = desc.text_key;
    record.draw_element.scissor_enabled = desc.scissor_enabled;
    record.uv_rect = placement.glyph.uv_rect;
    record.tint = tint;
    record.effect = effect;
    record.codepoint = placement.glyph.resolved_codepoint;
    record.codepoint_index = placement.codepoint_index;
    record.font_key = placement.glyph.resolved_font_key;
    record.sprite_key = placement.glyph.sprite_key;
    return record;
}

UiRect BuildOutlineRect(UiRect rect, float expand) {
    rect.x -= expand;
    rect.y -= expand;
    rect.width += expand * 2.0F;
    rect.height += expand * 2.0F;
    return rect;
}

UiRect BuildShadowRect(UiRect rect, float offset_x, float offset_y) {
    rect.x += offset_x;
    rect.y += offset_y;
    return rect;
}

void WriteRecords(
    const UiNodeRecord &node_record,
    const UiTextComponentDesc &desc,
    std::span<const UiTextGlyphPlacement> placements,
    std::span<const float> line_widths,
    std::uint32_t line_count,
    std::span<UiTextDrawRecord> out_records,
    UiTextComponentResult *out_result) {
    std::uint32_t record_index = 0U;
    for (const UiTextGlyphPlacement &placement : placements) {
        const UiRect rect = BuildPlacementRect(node_record, desc, placement, line_widths, line_count);
        if (!IsRectIntersecting(rect, node_record.content_rect)) {
            continue;
        }

        if (desc.shadow_enabled) {
            const UiRect shadow_rect = BuildShadowRect(rect, desc.shadow_offset_x, desc.shadow_offset_y);
            out_records[record_index] = BuildRecord(
                node_record,
                desc,
                placement,
                shadow_rect,
                UiTextEffectKind::Shadow,
                desc.shadow_tint);
            ++record_index;
        }

        if (desc.outline_enabled) {
            const UiRect outline_rect = BuildOutlineRect(rect, desc.outline_expand_px);
            out_records[record_index] = BuildRecord(
                node_record,
                desc,
                placement,
                outline_rect,
                UiTextEffectKind::Outline,
                desc.outline_tint);
            ++record_index;
        }

        out_records[record_index] = BuildRecord(
            node_record,
            desc,
            placement,
            rect,
            UiTextEffectKind::Main,
            desc.tint);
        ++record_index;
    }

    out_result->draw_record_count = record_index;
}

void SetResult(UiTextComponentResult *out_result, UiTextComponentStatus status, std::uint32_t required_count) {
    out_result->status = status;
    out_result->required_draw_record_count = required_count;
}
}

UiTextComponentStatus UiTextComponent::Build(
    const UiNodeTree &tree,
    const UiFontGlyphAtlasDesc &font_atlas,
    const UiTextComponentDesc &desc,
    std::span<UiTextDrawRecord> out_records,
    UiTextComponentResult *out_result) const {
    if (out_result == nullptr) {
        return UiTextComponentStatus::InvalidOutputBuffer;
    }

    *out_result = UiTextComponentResult{};
    out_result->node_id = desc.node_id;
    out_result->source_type = desc.source_type;
    out_result->text_key = desc.text_key;
    out_result->localize_key = desc.localize_key;
    out_result->dirty_change_type = ResolveDirtyChangeType(desc.change_reason);

    UiTextComponentStatus status = ValidateDesc(desc);
    if (status != UiTextComponentStatus::Success) {
        SetResult(out_result, status, 0U);
        return status;
    }

    const UiNodeTreeResult node_result = tree.QueryNode(desc.node_id);
    if (!node_result.Succeeded()) {
        SetResult(out_result, UiTextComponentStatus::NodeNotFound, 0U);
        return UiTextComponentStatus::NodeNotFound;
    }

    if (node_result.record.content_rect.width <= 0.0F || node_result.record.content_rect.height <= 0.0F) {
        SetResult(out_result, UiTextComponentStatus::InvalidDesc, 0U);
        return UiTextComponentStatus::InvalidDesc;
    }

    if (!node_result.record.is_visible) {
        SetResult(out_result, UiTextComponentStatus::Success, 0U);
        return UiTextComponentStatus::Success;
    }

    std::array<UiTextGlyphPlacement, MAX_UI_TEXT_CODEPOINT_COUNT> placements{};
    std::array<float, MAX_UI_TEXT_CODEPOINT_COUNT> line_widths{};
    std::uint32_t placement_count = 0U;
    std::uint32_t line_count = 0U;
    status = BuildPlacements(
        font_atlas,
        desc,
        node_result.record,
        placements,
        line_widths,
        out_result,
        &placement_count,
        &line_count);
    if (status != UiTextComponentStatus::Success) {
        SetResult(out_result, status, 0U);
        return status;
    }

    const std::span<const UiTextGlyphPlacement> placement_span(placements.data(), placement_count);
    const std::span<const float> line_width_span(line_widths.data(), line_count);
    const std::uint32_t required_count = CountRequiredRecords(
        node_result.record,
        desc,
        placement_span,
        line_width_span,
        line_count,
        out_result);
    out_result->required_draw_record_count = required_count;
    status = ValidateOutputStorage(out_records, required_count);
    if (status != UiTextComponentStatus::Success) {
        SetResult(out_result, status, required_count);
        return status;
    }

    WriteRecords(node_result.record, desc, placement_span, line_width_span, line_count, out_records, out_result);
    out_result->status = UiTextComponentStatus::Success;
    return UiTextComponentStatus::Success;
}

UiTextComponentStatus UiTextComponent::ValidateDesc(const UiTextComponentDesc &desc) const {
    if (desc.node_id.value == 0U) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (!IsSourceTypeKnown(desc.source_type)) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (!IsGlyphKeyValid(desc.font_key)) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (desc.fallback_font_keys.size() > static_cast<std::size_t>(MAX_UI_FONT_FALLBACK_COUNT)) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (!IsHorizontalAlignmentKnown(desc.horizontal_alignment)) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (!IsVerticalAlignmentKnown(desc.vertical_alignment)) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (!IsWrapModeKnown(desc.wrap_mode)) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (!IsOverflowModeKnown(desc.overflow_mode)) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (!IsChangeReasonKnown(desc.change_reason)) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (desc.text_key == 0U || desc.line_height_px <= 0.0F) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (desc.outline_expand_px < 0.0F) {
        return UiTextComponentStatus::InvalidDesc;
    }

    if (!IsTintValid(desc.tint) || !IsTintValid(desc.outline_tint) || !IsTintValid(desc.shadow_tint)) {
        return UiTextComponentStatus::InvalidDesc;
    }

    const std::span<const std::uint32_t> codepoints = SelectSourceCodepoints(desc);
    const UiTextComponentStatus status = ValidateCodepoints(codepoints);
    if (status != UiTextComponentStatus::Success) {
        return status;
    }

    if (desc.source_type == UiTextSourceType::LocalizeKeyPlaceholder && desc.localize_key == 0U) {
        return UiTextComponentStatus::InvalidDesc;
    }

    return UiTextComponentStatus::Success;
}

UiDirtyChangeType UiTextComponent::ResolveDirtyChangeType(UiTextChangeReason reason) const {
    if (reason == UiTextChangeReason::PaintStyle) {
        return UiDirtyChangeType::PaintOnly;
    }

    if (reason == UiTextChangeReason::LayoutStyle) {
        return UiDirtyChangeType::Layout;
    }

    return UiDirtyChangeType::Text;
}
}
