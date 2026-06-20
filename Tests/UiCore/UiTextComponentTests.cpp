// 模块: Tests UiCore
// 文件: Tests/UiCore/UiTextComponentTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiDirtyDomain.h"
#include "YuEngine/UiCore/UiDirtyState.h"
#include "YuEngine/UiCore/UiDirtyTracker.h"
#include "YuEngine/UiCore/UiFontGlyphAtlas.h"
#include "YuEngine/UiCore/UiNodeDesc.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiNodeTree.h"
#include "YuEngine/UiCore/UiNodeTreeDesc.h"
#include "YuEngine/UiCore/UiNodeTreeResult.h"
#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiCore/UiTextComponent.h"
#include "YuEngine/UiCore/UiTextComponentDesc.h"
#include "YuEngine/UiCore/UiTextComponentResult.h"
#include "YuEngine/UiCore/UiTextComponentStatus.h"
#include "YuEngine/UiCore/UiTextDrawRecord.h"

using yuengine::uicore::UiDirtyState;
using yuengine::uicore::UiDirtyTracker;
using yuengine::uicore::UiFontAssetDesc;
using yuengine::uicore::UiFontGlyphAtlasDesc;
using yuengine::uicore::UiFontGlyphDesc;
using yuengine::uicore::UiFontGlyphKey;
using yuengine::uicore::UiNodeDesc;
using yuengine::uicore::UiNodeId;
using yuengine::uicore::UiNodeTree;
using yuengine::uicore::UiNodeTreeDesc;
using yuengine::uicore::UiNodeTreeResult;
using yuengine::uicore::UiRect;
using yuengine::uicore::UiRectTransform;
using yuengine::uicore::UiStaticAtlasPageDesc;
using yuengine::uicore::UiStaticAtlasSpriteDesc;
using yuengine::uicore::UiTextChangeReason;
using yuengine::uicore::UiTextComponent;
using yuengine::uicore::UiTextComponentDesc;
using yuengine::uicore::UiTextComponentResult;
using yuengine::uicore::UiTextComponentStatus;
using yuengine::uicore::UiTextDrawRecord;
using yuengine::uicore::UiTextEffectKind;
using yuengine::uicore::UiTextHorizontalAlignment;
using yuengine::uicore::UiTextSourceType;
using yuengine::uicore::UiTextVerticalAlignment;
using yuengine::uicore::UiTextWrapMode;
using yuengine::uicore::UI_DIRTY_LAYOUT;
using yuengine::uicore::UI_DIRTY_PAINT;
using yuengine::uicore::UI_DIRTY_TEXT;

namespace {
constexpr const char *TEST_LAYOUT =
    "UiCore_TextComponent_RendersAlignedWrappedPlainText";
constexpr const char *TEST_LOCALIZE_FALLBACK =
    "UiCore_TextComponent_LocalizePlaceholderUsesFallback";
constexpr const char *TEST_OVERFLOW_EFFECT =
    "UiCore_TextComponent_ClipsOverflowAndWritesEffects";
constexpr const char *TEST_DIRTY =
    "UiCore_TextComponent_TextDirtyDoesNotTriggerLayout";
constexpr const char *TEST_MISSING_GLYPH =
    "UiCore_TextComponent_ReportsMissingGlyphWithoutMutation";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiCore_TextComponent_RejectsSmallOutputWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t FONT_PRIMARY = 100U;
constexpr std::uint32_t FONT_FALLBACK = 200U;
constexpr std::uint32_t PAGE_KEY = 7U;
constexpr std::uint32_t TEXTURE_KEY = 77U;
constexpr std::uint32_t TEXT_KEY = 300U;
constexpr std::uint32_t CODEPOINT_A = 65U;
constexpr std::uint32_t CODEPOINT_B = 66U;
constexpr std::uint32_t CODEPOINT_C = 67U;
constexpr std::uint32_t CODEPOINT_D = 68U;
constexpr std::uint32_t CODEPOINT_NEWLINE = 10U;
constexpr std::uint32_t SENTINEL_CODEPOINT = 9999U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool FloatClose(float left, float right) {
    float diff = left - right;
    if (diff < 0.0F) {
        diff = -diff;
    }

    return diff < 0.0001F;
}

bool RectClose(const UiRect &rect, float x, float y, float width, float height) {
    if (!FloatClose(rect.x, x)) {
        return false;
    }

    if (!FloatClose(rect.y, y)) {
        return false;
    }

    if (!FloatClose(rect.width, width)) {
        return false;
    }

    return FloatClose(rect.height, height);
}

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

UiNodeTreeDesc MakeTreeDesc() {
    UiNodeTreeDesc desc{};
    desc.node_capacity = 4U;
    desc.viewport_rect = UiRect{0.0F, 0.0F, 800.0F, 600.0F};
    return desc;
}

UiRectTransform FixedTransform(float x, float y, float width, float height) {
    UiRectTransform transform{};
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.pivot = {0.5F, 0.5F};
    transform.offset_min = {x, y};
    transform.offset_max = {x + width - 800.0F, y + height - 600.0F};
    return transform;
}

UiNodeDesc MakeNodeDesc(UiRect rect) {
    UiNodeDesc desc{};
    desc.node_id = NodeId(1U);
    desc.parent_id = UiNodeId{};
    desc.rect_transform = FixedTransform(rect.x, rect.y, rect.width, rect.height);
    desc.sibling_order = 1U;
    desc.layer = 3;
    desc.is_visible = true;
    desc.is_enabled = true;
    desc.is_hit_testable = true;
    return desc;
}

int CreateNode(UiNodeTree &tree, UiRect rect) {
    const UiNodeDesc node_desc = MakeNodeDesc(rect);
    const UiNodeTreeResult result = tree.CreateNode(node_desc);
    if (result.Succeeded()) {
        return 0;
    }

    return Fail("node create failed");
}

UiStaticAtlasPageDesc MakePage() {
    UiStaticAtlasPageDesc desc{};
    desc.page_key = PAGE_KEY;
    desc.texture_key = TEXTURE_KEY;
    desc.width = 256U;
    desc.height = 128U;
    return desc;
}

UiStaticAtlasSpriteDesc MakeSprite(std::uint32_t sprite_key, std::uint32_t x) {
    UiStaticAtlasSpriteDesc desc{};
    desc.sprite_key = sprite_key;
    desc.page_key = PAGE_KEY;
    desc.x = x;
    desc.y = 0U;
    desc.width = 8U;
    desc.height = 12U;
    return desc;
}

UiFontAssetDesc MakeFont(std::uint32_t font_key) {
    UiFontAssetDesc desc{};
    desc.font_key = font_key;
    return desc;
}

UiFontGlyphKey MakeGlyphKey(std::uint32_t font_key, std::uint32_t codepoint) {
    UiFontGlyphKey key{};
    key.font_key = font_key;
    key.codepoint = codepoint;
    key.size_px = 12U;
    return key;
}

UiFontGlyphDesc MakeGlyph(std::uint32_t font_key, std::uint32_t codepoint, std::uint32_t sprite_key, std::uint32_t x) {
    UiFontGlyphDesc glyph{};
    glyph.key = MakeGlyphKey(font_key, codepoint);
    glyph.sprite = MakeSprite(sprite_key, x);
    glyph.advance_x = 10;
    glyph.bearing_x = 0;
    glyph.bearing_y = 10;
    return glyph;
}

UiTextComponentDesc MakeTextDesc(std::span<const std::uint32_t> codepoints) {
    UiTextComponentDesc desc{};
    desc.node_id = NodeId(1U);
    desc.text_codepoints = codepoints;
    desc.font_key = MakeGlyphKey(FONT_PRIMARY, CODEPOINT_A);
    desc.text_key = TEXT_KEY;
    desc.style_key = 11U;
    desc.material_key = 13U;
    desc.line_height_px = 12.0F;
    return desc;
}

UiTextDrawRecord SentinelRecord() {
    UiTextDrawRecord record{};
    record.codepoint = SENTINEL_CODEPOINT;
    record.draw_element.text_key = SENTINEL_CODEPOINT;
    return record;
}

bool RecordMatchesSentinel(const UiTextDrawRecord &record) {
    if (record.codepoint != SENTINEL_CODEPOINT) {
        return false;
    }

    return record.draw_element.text_key == SENTINEL_CODEPOINT;
}

int UiCoreTextComponentRendersAlignedWrappedPlainText() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateNode(tree, UiRect{10.0F, 20.0F, 24.0F, 40.0F});
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<std::uint32_t, 3U> codepoints{CODEPOINT_A, CODEPOINT_B, CODEPOINT_C};
    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiFontAssetDesc, 1U> fonts{MakeFont(FONT_PRIMARY)};
    const std::array<UiFontGlyphDesc, 3U> glyphs{
        MakeGlyph(FONT_PRIMARY, CODEPOINT_A, 1001U, 0U),
        MakeGlyph(FONT_PRIMARY, CODEPOINT_B, 1002U, 8U),
        MakeGlyph(FONT_PRIMARY, CODEPOINT_C, 1003U, 16U)};
    const UiFontGlyphAtlasDesc font_atlas{pages, fonts, glyphs};
    UiTextComponentDesc desc = MakeTextDesc(codepoints);
    desc.horizontal_alignment = UiTextHorizontalAlignment::Center;
    desc.vertical_alignment = UiTextVerticalAlignment::Middle;
    desc.wrap_mode = UiTextWrapMode::Character;

    std::array<UiTextDrawRecord, 3U> records{};
    UiTextComponentResult result{};
    UiTextComponent component{};
    const UiTextComponentStatus status = component.Build(tree, font_atlas, desc, records, &result);
    if (status != UiTextComponentStatus::Success || !result.Succeeded()) {
        return Fail("text component rejected wrapped plain text");
    }

    if (result.draw_record_count != 3U || result.line_count != 2U) {
        return Fail("text component wrapped count mismatch");
    }

    if (!RectClose(records[0U].draw_element.rect, 12.0F, 28.0F, 8.0F, 12.0F)) {
        return Fail("text component first glyph rect mismatch");
    }

    if (!RectClose(records[2U].draw_element.rect, 17.0F, 40.0F, 8.0F, 12.0F)) {
        return Fail("text component wrapped glyph rect mismatch");
    }

    if (records[0U].draw_element.type != yuengine::uicore::UiDrawElementType::Text) {
        return Fail("text component draw element type mismatch");
    }

    return 0;
}

int UiCoreTextComponentLocalizePlaceholderUsesFallback() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateNode(tree, UiRect{0.0F, 0.0F, 64.0F, 24.0F});
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<std::uint32_t, 1U> placeholder{CODEPOINT_D};
    const std::array<std::uint32_t, 1U> fallback_fonts{FONT_FALLBACK};
    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiFontAssetDesc, 2U> fonts{MakeFont(FONT_PRIMARY), MakeFont(FONT_FALLBACK)};
    const std::array<UiFontGlyphDesc, 1U> glyphs{
        MakeGlyph(FONT_FALLBACK, CODEPOINT_D, 2001U, 24U)};
    const UiFontGlyphAtlasDesc font_atlas{pages, fonts, glyphs};
    UiTextComponentDesc desc = MakeTextDesc(placeholder);
    desc.source_type = UiTextSourceType::LocalizeKeyPlaceholder;
    desc.localize_key = 44U;
    desc.localize_placeholder_codepoints = placeholder;
    desc.fallback_font_keys = fallback_fonts;

    std::array<UiTextDrawRecord, 1U> records{};
    UiTextComponentResult result{};
    UiTextComponent component{};
    const UiTextComponentStatus status = component.Build(tree, font_atlas, desc, records, &result);
    if (status != UiTextComponentStatus::Success || result.source_type != UiTextSourceType::LocalizeKeyPlaceholder) {
        return Fail("text component localize placeholder rejected");
    }

    if (records[0U].font_key != FONT_FALLBACK || result.localize_key != 44U) {
        return Fail("text component fallback localize metadata mismatch");
    }

    return 0;
}

int UiCoreTextComponentClipsOverflowAndWritesEffects() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateNode(tree, UiRect{0.0F, 0.0F, 64.0F, 12.0F});
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<std::uint32_t, 3U> codepoints{CODEPOINT_A, CODEPOINT_NEWLINE, CODEPOINT_B};
    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiFontAssetDesc, 1U> fonts{MakeFont(FONT_PRIMARY)};
    const std::array<UiFontGlyphDesc, 2U> glyphs{
        MakeGlyph(FONT_PRIMARY, CODEPOINT_A, 1001U, 0U),
        MakeGlyph(FONT_PRIMARY, CODEPOINT_B, 1002U, 8U)};
    const UiFontGlyphAtlasDesc font_atlas{pages, fonts, glyphs};
    UiTextComponentDesc desc = MakeTextDesc(codepoints);
    desc.outline_enabled = true;
    desc.outline_expand_px = 1.0F;
    desc.shadow_enabled = true;
    desc.shadow_offset_x = 2.0F;
    desc.shadow_offset_y = 1.0F;

    std::array<UiTextDrawRecord, 3U> records{};
    UiTextComponentResult result{};
    UiTextComponent component{};
    const UiTextComponentStatus status = component.Build(tree, font_atlas, desc, records, &result);
    if (status != UiTextComponentStatus::Success || !result.overflowed) {
        return Fail("text component did not report vertical overflow");
    }

    if (result.draw_record_count != 3U || result.visible_codepoint_count != 1U) {
        return Fail("text component overflow effect record count mismatch");
    }

    if (records[0U].effect != UiTextEffectKind::Shadow || records[1U].effect != UiTextEffectKind::Outline) {
        return Fail("text component effect ordering mismatch");
    }

    if (!RectClose(records[0U].draw_element.rect, 2.0F, 1.0F, 8.0F, 12.0F)) {
        return Fail("text component shadow rect mismatch");
    }

    return 0;
}

int UiCoreTextComponentTextDirtyDoesNotTriggerLayout() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateNode(tree, UiRect{0.0F, 0.0F, 64.0F, 24.0F});
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<std::uint32_t, 1U> codepoints{CODEPOINT_A};
    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiFontAssetDesc, 1U> fonts{MakeFont(FONT_PRIMARY)};
    const std::array<UiFontGlyphDesc, 1U> glyphs{
        MakeGlyph(FONT_PRIMARY, CODEPOINT_A, 1001U, 0U)};
    const UiFontGlyphAtlasDesc font_atlas{pages, fonts, glyphs};
    UiTextComponentDesc desc = MakeTextDesc(codepoints);
    desc.change_reason = UiTextChangeReason::Content;

    std::array<UiTextDrawRecord, 1U> records{};
    UiTextComponentResult result{};
    UiTextComponent component{};
    const UiTextComponentStatus status = component.Build(tree, font_atlas, desc, records, &result);
    if (status != UiTextComponentStatus::Success) {
        return Fail("text component dirty fixture rejected");
    }

    UiDirtyTracker tracker{};
    const UiDirtyState state = tracker.ApplyChange(result.dirty_change_type);
    if (!state.HasDomain(UI_DIRTY_TEXT) || !state.HasDomain(UI_DIRTY_PAINT)) {
        return Fail("text dirty did not mark text and paint domains");
    }

    if (state.HasDomain(UI_DIRTY_LAYOUT) || state.layout_rebuild_count != 0U) {
        return Fail("text dirty unexpectedly marked layout");
    }

    desc.change_reason = UiTextChangeReason::LayoutStyle;
    const UiTextComponentStatus layout_status = component.Build(tree, font_atlas, desc, records, &result);
    if (layout_status != UiTextComponentStatus::Success) {
        return Fail("text component layout dirty fixture rejected");
    }

    const UiDirtyState layout_state = tracker.ApplyChange(result.dirty_change_type);
    if (!layout_state.HasDomain(UI_DIRTY_LAYOUT)) {
        return Fail("layout text change did not mark layout");
    }

    return 0;
}

int UiCoreTextComponentReportsMissingGlyphWithoutMutation() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateNode(tree, UiRect{0.0F, 0.0F, 64.0F, 24.0F});
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<std::uint32_t, 1U> codepoints{CODEPOINT_A};
    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiFontAssetDesc, 1U> fonts{MakeFont(FONT_PRIMARY)};
    const std::array<UiFontGlyphDesc, 0U> glyphs{};
    const UiFontGlyphAtlasDesc font_atlas{pages, fonts, glyphs};
    UiTextComponentDesc desc = MakeTextDesc(codepoints);

    std::array<UiTextDrawRecord, 1U> records{SentinelRecord()};
    UiTextComponentResult result{};
    UiTextComponent component{};
    const UiTextComponentStatus status = component.Build(tree, font_atlas, desc, records, &result);
    if (status != UiTextComponentStatus::FontGlyphMissing || result.missing_codepoint != CODEPOINT_A) {
        return Fail("text component missing glyph status mismatch");
    }

    if (!RecordMatchesSentinel(records[0U])) {
        return Fail("text component mutated output after missing glyph");
    }

    return 0;
}

int UiCoreTextComponentRejectsSmallOutputWithoutMutation() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateNode(tree, UiRect{0.0F, 0.0F, 64.0F, 24.0F});
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<std::uint32_t, 2U> codepoints{CODEPOINT_A, CODEPOINT_B};
    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiFontAssetDesc, 1U> fonts{MakeFont(FONT_PRIMARY)};
    const std::array<UiFontGlyphDesc, 2U> glyphs{
        MakeGlyph(FONT_PRIMARY, CODEPOINT_A, 1001U, 0U),
        MakeGlyph(FONT_PRIMARY, CODEPOINT_B, 1002U, 8U)};
    const UiFontGlyphAtlasDesc font_atlas{pages, fonts, glyphs};
    UiTextComponentDesc desc = MakeTextDesc(codepoints);

    std::array<UiTextDrawRecord, 1U> records{SentinelRecord()};
    UiTextComponentResult result{};
    UiTextComponent component{};
    const UiTextComponentStatus status = component.Build(tree, font_atlas, desc, records, &result);
    if (status != UiTextComponentStatus::OutputCapacityExceeded) {
        return Fail("text component accepted undersized output");
    }

    if (result.required_draw_record_count != 2U || !RecordMatchesSentinel(records[0U])) {
        return Fail("text component small output mutation mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_LAYOUT) {
        return UiCoreTextComponentRendersAlignedWrappedPlainText();
    }

    if (name == TEST_LOCALIZE_FALLBACK) {
        return UiCoreTextComponentLocalizePlaceholderUsesFallback();
    }

    if (name == TEST_OVERFLOW_EFFECT) {
        return UiCoreTextComponentClipsOverflowAndWritesEffects();
    }

    if (name == TEST_DIRTY) {
        return UiCoreTextComponentTextDirtyDoesNotTriggerLayout();
    }

    if (name == TEST_MISSING_GLYPH) {
        return UiCoreTextComponentReportsMissingGlyphWithoutMutation();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiCoreTextComponentRejectsSmallOutputWithoutMutation();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
