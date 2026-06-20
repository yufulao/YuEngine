// 模块: UiComponentSmokeSample
// 文件: Samples/UiComponentSmoke/Source/UiComponentSmokeSample.cpp

#include "UiComponentSmokeSample.h"

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

#include "YuEngine/UiCore/UiButtonActivationSource.h"
#include "YuEngine/UiCore/UiButtonComponent.h"
#include "YuEngine/UiCore/UiButtonComponentDesc.h"
#include "YuEngine/UiCore/UiButtonComponentResult.h"
#include "YuEngine/UiCore/UiButtonComponentStatus.h"
#include "YuEngine/UiCore/UiButtonVisualStateDesc.h"
#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiDrawBatch.h"
#include "YuEngine/UiCore/UiDrawBatcher.h"
#include "YuEngine/UiCore/UiDrawBatchResult.h"
#include "YuEngine/UiCore/UiDrawBatchStatus.h"
#include "YuEngine/UiCore/UiDrawElement.h"
#include "YuEngine/UiCore/UiFontGlyphAtlas.h"
#include "YuEngine/UiCore/UiGridViewSemantics.h"
#include "YuEngine/UiCore/UiGridViewVirtualizer.h"
#include "YuEngine/UiCore/UiImageComponent.h"
#include "YuEngine/UiCore/UiImageComponentDesc.h"
#include "YuEngine/UiCore/UiImageComponentResult.h"
#include "YuEngine/UiCore/UiImageComponentStatus.h"
#include "YuEngine/UiCore/UiImageDrawRecord.h"
#include "YuEngine/UiCore/UiImageTint.h"
#include "YuEngine/UiCore/UiInvalidatedNode.h"
#include "YuEngine/UiCore/UiInvalidationModel.h"
#include "YuEngine/UiCore/UiInvalidationRequest.h"
#include "YuEngine/UiCore/UiInvalidationResult.h"
#include "YuEngine/UiCore/UiInvalidationScope.h"
#include "YuEngine/UiCore/UiInvalidationStatus.h"
#include "YuEngine/UiCore/UiNodeDesc.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiNodeTree.h"
#include "YuEngine/UiCore/UiNodeTreeDesc.h"
#include "YuEngine/UiCore/UiNodeTreeResult.h"
#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiCore/UiSliderAdjustmentSource.h"
#include "YuEngine/UiCore/UiSliderComponent.h"
#include "YuEngine/UiCore/UiSliderComponentDesc.h"
#include "YuEngine/UiCore/UiSliderComponentResult.h"
#include "YuEngine/UiCore/UiSliderComponentStatus.h"
#include "YuEngine/UiCore/UiSliderVisualDesc.h"
#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"
#include "YuEngine/UiCore/UiTextComponent.h"
#include "YuEngine/UiCore/UiTextComponentDesc.h"
#include "YuEngine/UiCore/UiTextComponentResult.h"
#include "YuEngine/UiCore/UiTextComponentStatus.h"
#include "YuEngine/UiCore/UiTextDrawRecord.h"

namespace ui_component_smoke_sample {
namespace {
using yuengine::uicore::INVALID_UI_GRID_INDEX;
using yuengine::uicore::UiButtonActivationSource;
using yuengine::uicore::UiButtonComponent;
using yuengine::uicore::UiButtonComponentDesc;
using yuengine::uicore::UiButtonComponentResult;
using yuengine::uicore::UiButtonComponentStatus;
using yuengine::uicore::UiButtonVisualStateDesc;
using yuengine::uicore::UiDirtyChangeType;
using yuengine::uicore::UiDrawBatch;
using yuengine::uicore::UiDrawBatcher;
using yuengine::uicore::UiDrawBatchResult;
using yuengine::uicore::UiDrawBatchStatus;
using yuengine::uicore::UiDrawElement;
using yuengine::uicore::UiFontAssetDesc;
using yuengine::uicore::UiFontGlyphAtlasDesc;
using yuengine::uicore::UiFontGlyphDesc;
using yuengine::uicore::UiFontGlyphKey;
using yuengine::uicore::UiGridViewDesc;
using yuengine::uicore::UiGridViewKind;
using yuengine::uicore::UiGridViewStatus;
using yuengine::uicore::UiGridViewVirtualCellRecord;
using yuengine::uicore::UiGridViewVirtualizationDesc;
using yuengine::uicore::UiGridViewVirtualizationResult;
using yuengine::uicore::UiGridViewVirtualizer;
using yuengine::uicore::UiImageComponent;
using yuengine::uicore::UiImageComponentDesc;
using yuengine::uicore::UiImageComponentResult;
using yuengine::uicore::UiImageComponentStatus;
using yuengine::uicore::UiImageDrawRecord;
using yuengine::uicore::UiImageTint;
using yuengine::uicore::UiInvalidatedNode;
using yuengine::uicore::UiInvalidationModel;
using yuengine::uicore::UiInvalidationRequest;
using yuengine::uicore::UiInvalidationResult;
using yuengine::uicore::UiInvalidationScope;
using yuengine::uicore::UiInvalidationStatus;
using yuengine::uicore::UiNodeDesc;
using yuengine::uicore::UiNodeId;
using yuengine::uicore::UiNodeTree;
using yuengine::uicore::UiNodeTreeDesc;
using yuengine::uicore::UiNodeTreeResult;
using yuengine::uicore::UiRect;
using yuengine::uicore::UiRectTransform;
using yuengine::uicore::UiSliderAdjustmentSource;
using yuengine::uicore::UiSliderComponent;
using yuengine::uicore::UiSliderComponentDesc;
using yuengine::uicore::UiSliderComponentResult;
using yuengine::uicore::UiSliderComponentStatus;
using yuengine::uicore::UiSliderVisualDesc;
using yuengine::uicore::UiStaticAtlasMetadataDesc;
using yuengine::uicore::UiStaticAtlasPageDesc;
using yuengine::uicore::UiStaticAtlasSpriteDesc;
using yuengine::uicore::UiTextComponent;
using yuengine::uicore::UiTextComponentDesc;
using yuengine::uicore::UiTextComponentResult;
using yuengine::uicore::UiTextComponentStatus;
using yuengine::uicore::UiTextDrawRecord;

constexpr std::uint32_t ROOT_NODE_ID = 1U;
constexpr std::uint32_t WINDOW_NODE_ID = 2U;
constexpr std::uint32_t TEXT_NODE_ID = 3U;
constexpr std::uint32_t IMAGE_NODE_ID = 4U;
constexpr std::uint32_t BUTTON_NODE_ID = 5U;
constexpr std::uint32_t BUTTON_IMAGE_NODE_ID = 6U;
constexpr std::uint32_t BUTTON_TEXT_NODE_ID = 7U;
constexpr std::uint32_t SLIDER_TRACK_NODE_ID = 8U;
constexpr std::uint32_t SLIDER_FILL_NODE_ID = 9U;
constexpr std::uint32_t SLIDER_HANDLE_NODE_ID = 10U;
constexpr std::uint32_t GRID_VIEW_NODE_ID = 11U;
constexpr std::uint32_t LAYOUT_NODE_COUNT = 11U;
constexpr std::uint32_t VIEWPORT_WIDTH = 960U;
constexpr std::uint32_t VIEWPORT_HEIGHT = 540U;
constexpr std::uint32_t PAGE_KEY = 700U;
constexpr std::uint32_t TEXTURE_KEY = 701U;
constexpr std::uint32_t FONT_KEY = 710U;
constexpr std::uint32_t FONT_SIZE = 18U;
constexpr std::uint32_t SPRITE_IMAGE_KEY = 800U;
constexpr std::uint32_t SPRITE_BUTTON_NORMAL_KEY = 801U;
constexpr std::uint32_t SPRITE_BUTTON_HOVER_KEY = 802U;
constexpr std::uint32_t SPRITE_BUTTON_PRESSED_KEY = 803U;
constexpr std::uint32_t SPRITE_BUTTON_DISABLED_KEY = 804U;
constexpr std::uint32_t SPRITE_BUTTON_SELECTED_KEY = 805U;
constexpr std::uint32_t SPRITE_SLIDER_FILL_KEY = 806U;
constexpr std::uint32_t SPRITE_SLIDER_HANDLE_KEY = 807U;
constexpr std::uint32_t BUTTON_EVENT_KEY = 900U;
constexpr std::uint32_t BUTTON_SOUND_KEY = 901U;
constexpr std::uint32_t SLIDER_EVENT_KEY = 910U;
constexpr std::uint32_t GRID_ITEM_COUNT = 1000U;
constexpr std::uint32_t GRID_AXIS_CELL_COUNT = 5U;
constexpr std::uint32_t GRID_VISIBLE_GROUP_COUNT = 2U;
constexpr std::uint32_t GRID_BUFFER_GROUP_COUNT = 1U;
constexpr std::uint32_t GRID_POOL_GROUP_COUNT = 4U;
constexpr std::uint32_t GRID_POOL_CELL_COUNT = GRID_AXIS_CELL_COUNT * GRID_POOL_GROUP_COUNT;
constexpr std::uint32_t DIAGNOSTIC_DRAW_ELEMENT_CAPACITY = 16U;
constexpr std::uint32_t DIAGNOSTIC_INVALIDATION_CAPACITY = 16U;
constexpr float SLIDER_VALUE = 25.0F;
constexpr float SLIDER_MAX_VALUE = 100.0F;

struct ComponentLayout final {
    std::uint32_t node_count = 0U;
    std::uint32_t root_node_id = 0U;
    std::uint32_t window_node_id = 0U;
};

struct ComponentDiagnosticsScratch final {
    std::array<UiDrawElement, DIAGNOSTIC_DRAW_ELEMENT_CAPACITY> draw_elements{};
    std::uint32_t draw_element_count = 0U;
    std::uint32_t atlas_page_count = 0U;
};

bool ContainsText(std::string_view text, std::string_view expected) {
    return text.find(expected) != std::string_view::npos;
}

bool LoadComponentLayout(std::string_view text, ComponentLayout *layout) {
    if (layout == nullptr) {
        return false;
    }

    if (text.empty()) {
        return false;
    }

    if (!ContainsText(text, "\"schema\": \"YuEngine.UI.Layout\"")) {
        return false;
    }

    if (!ContainsText(text, "\"layoutId\": \"UiComponentSmoke.ComponentWindow\"")) {
        return false;
    }

    if (!ContainsText(text, "\"rootNodeId\": 1")) {
        return false;
    }

    if (!ContainsText(text, "\"type\": \"Text\"")) {
        return false;
    }

    if (!ContainsText(text, "\"type\": \"Image\"")) {
        return false;
    }

    if (!ContainsText(text, "\"type\": \"Button\"")) {
        return false;
    }

    if (!ContainsText(text, "\"type\": \"Slider\"")) {
        return false;
    }

    if (!ContainsText(text, "\"type\": \"GridView\"")) {
        return false;
    }

    layout->node_count = LAYOUT_NODE_COUNT;
    layout->root_node_id = ROOT_NODE_ID;
    layout->window_node_id = WINDOW_NODE_ID;
    return true;
}

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

UiRectTransform StretchTransform() {
    UiRectTransform transform{};
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.pivot = {0.5F, 0.5F};
    return transform;
}

UiRectTransform AbsoluteTransform(float x, float y, float width, float height) {
    UiRectTransform transform{};
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {0.0F, 0.0F};
    transform.pivot = {0.5F, 0.5F};
    transform.offset_min = {x, y};
    transform.offset_max = {x + width, y + height};
    return transform;
}

UiNodeDesc MakeNodeDesc(
    std::uint32_t node_id,
    std::uint32_t parent_node_id,
    UiRectTransform transform,
    std::uint32_t sibling_order,
    std::int32_t layer) {
    UiNodeDesc desc{};
    desc.node_id = NodeId(node_id);
    desc.parent_id = NodeId(parent_node_id);
    desc.rect_transform = transform;
    desc.sibling_order = sibling_order;
    desc.layer = layer;
    desc.is_visible = true;
    desc.is_enabled = true;
    desc.is_hit_testable = true;
    return desc;
}

bool CreateNode(UiNodeTree *tree, const UiNodeDesc &desc) {
    if (tree == nullptr) {
        return false;
    }

    const UiNodeTreeResult result = tree->CreateNode(desc);
    return result.Succeeded();
}

bool BuildNodeTree(const ComponentLayout &layout, UiNodeTree *tree) {
    if (tree == nullptr) {
        return false;
    }

    UiNodeDesc root_desc = MakeNodeDesc(layout.root_node_id, 0U, StretchTransform(), 0U, 0);
    root_desc.parent_id = UiNodeId{};
    if (!CreateNode(tree, root_desc)) {
        return false;
    }

    const UiRectTransform window_transform = AbsoluteTransform(48.0F, 48.0F, 864.0F, 420.0F);
    if (!CreateNode(tree, MakeNodeDesc(layout.window_node_id, layout.root_node_id, window_transform, 0U, 10))) {
        return false;
    }

    if (!CreateNode(tree, MakeNodeDesc(TEXT_NODE_ID, layout.window_node_id, AbsoluteTransform(32.0F, 340.0F, 360.0F, 48.0F), 0U, 20))) {
        return false;
    }

    if (!CreateNode(tree, MakeNodeDesc(IMAGE_NODE_ID, layout.window_node_id, AbsoluteTransform(32.0F, 212.0F, 96.0F, 96.0F), 1U, 20))) {
        return false;
    }

    if (!CreateNode(tree, MakeNodeDesc(BUTTON_NODE_ID, layout.window_node_id, AbsoluteTransform(160.0F, 232.0F, 200.0F, 56.0F), 2U, 20))) {
        return false;
    }

    if (!CreateNode(tree, MakeNodeDesc(BUTTON_IMAGE_NODE_ID, layout.window_node_id, AbsoluteTransform(160.0F, 232.0F, 200.0F, 56.0F), 3U, 21))) {
        return false;
    }

    if (!CreateNode(tree, MakeNodeDesc(BUTTON_TEXT_NODE_ID, layout.window_node_id, AbsoluteTransform(184.0F, 246.0F, 152.0F, 28.0F), 4U, 22))) {
        return false;
    }

    if (!CreateNode(tree, MakeNodeDesc(SLIDER_TRACK_NODE_ID, layout.window_node_id, AbsoluteTransform(32.0F, 148.0F, 300.0F, 24.0F), 5U, 20))) {
        return false;
    }

    if (!CreateNode(tree, MakeNodeDesc(SLIDER_FILL_NODE_ID, layout.window_node_id, AbsoluteTransform(32.0F, 148.0F, 300.0F, 24.0F), 6U, 21))) {
        return false;
    }

    if (!CreateNode(tree, MakeNodeDesc(SLIDER_HANDLE_NODE_ID, layout.window_node_id, AbsoluteTransform(20.0F, 136.0F, 24.0F, 48.0F), 7U, 22))) {
        return false;
    }

    return CreateNode(tree, MakeNodeDesc(GRID_VIEW_NODE_ID, layout.window_node_id, AbsoluteTransform(420.0F, 116.0F, 384.0F, 260.0F), 8U, 20));
}

UiImageTint MakeTint(float red, float green, float blue, float alpha) {
    UiImageTint tint{};
    tint.red = red;
    tint.green = green;
    tint.blue = blue;
    tint.alpha = alpha;
    return tint;
}

UiStaticAtlasPageDesc MakePage() {
    UiStaticAtlasPageDesc desc{};
    desc.page_key = PAGE_KEY;
    desc.texture_key = TEXTURE_KEY;
    desc.width = 512U;
    desc.height = 256U;
    return desc;
}

UiStaticAtlasSpriteDesc MakeSprite(
    std::uint32_t sprite_key,
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t width,
    std::uint32_t height) {
    UiStaticAtlasSpriteDesc desc{};
    desc.sprite_key = sprite_key;
    desc.page_key = PAGE_KEY;
    desc.x = x;
    desc.y = y;
    desc.width = width;
    desc.height = height;
    return desc;
}

UiFontAssetDesc MakeFont() {
    UiFontAssetDesc desc{};
    desc.font_key = FONT_KEY;
    return desc;
}

UiFontGlyphKey MakeGlyphKey(std::uint32_t codepoint) {
    UiFontGlyphKey key{};
    key.font_key = FONT_KEY;
    key.codepoint = codepoint;
    key.size_px = FONT_SIZE;
    return key;
}

UiFontGlyphDesc MakeGlyph(
    std::uint32_t codepoint,
    std::uint32_t sprite_key,
    std::uint32_t x) {
    UiFontGlyphDesc desc{};
    desc.key = MakeGlyphKey(codepoint);
    desc.sprite = MakeSprite(sprite_key, x, 96U, 10U, 16U);
    desc.advance_x = 12;
    desc.bearing_x = 0;
    desc.bearing_y = 16;
    return desc;
}

UiStaticAtlasMetadataDesc MakeAtlasDesc(
    std::span<const UiStaticAtlasPageDesc> pages,
    std::span<const UiStaticAtlasSpriteDesc> sprites) {
    UiStaticAtlasMetadataDesc desc{};
    desc.pages = pages;
    desc.sprites = sprites;
    return desc;
}

UiFontGlyphAtlasDesc MakeFontAtlasDesc(
    std::span<const UiStaticAtlasPageDesc> pages,
    std::span<const UiFontAssetDesc> fonts,
    std::span<const UiFontGlyphDesc> glyphs) {
    UiFontGlyphAtlasDesc desc{};
    desc.pages = pages;
    desc.fonts = fonts;
    desc.glyphs = glyphs;
    return desc;
}

UiTextComponentDesc MakeTextDesc(std::span<const std::uint32_t> codepoints) {
    UiTextComponentDesc desc{};
    desc.node_id = NodeId(TEXT_NODE_ID);
    desc.text_codepoints = codepoints;
    desc.font_key = MakeGlyphKey(codepoints[0U]);
    desc.text_key = 1001U;
    desc.style_key = 1002U;
    desc.material_key = 1003U;
    desc.line_height_px = 20.0F;
    desc.tint = MakeTint(1.0F, 1.0F, 1.0F, 1.0F);
    return desc;
}

UiImageComponentDesc MakeImageDesc() {
    UiImageComponentDesc desc{};
    desc.node_id = NodeId(IMAGE_NODE_ID);
    desc.sprite_key = SPRITE_IMAGE_KEY;
    desc.style_key = 1010U;
    desc.material_key = 1011U;
    desc.tint = MakeTint(0.8F, 0.9F, 1.0F, 1.0F);
    return desc;
}

UiButtonVisualStateDesc MakeButtonVisual(std::uint32_t sprite_key, std::uint32_t style_key, float tint_seed) {
    UiButtonVisualStateDesc desc{};
    desc.image_sprite_key = sprite_key;
    desc.image_style_key = style_key;
    desc.image_material_key = style_key + 100U;
    desc.image_tint = MakeTint(tint_seed, tint_seed + 0.1F, tint_seed + 0.2F, 1.0F);
    desc.text_style_key = style_key + 200U;
    desc.text_material_key = style_key + 300U;
    desc.text_tint = MakeTint(1.0F, 1.0F, 1.0F, 1.0F);
    return desc;
}

UiButtonComponentDesc MakeButtonDesc() {
    UiButtonComponentDesc desc{};
    desc.node_id = NodeId(BUTTON_NODE_ID);
    desc.image_node_id = NodeId(BUTTON_IMAGE_NODE_ID);
    desc.text_node_id = NodeId(BUTTON_TEXT_NODE_ID);
    desc.interaction.hit_node_id = NodeId(BUTTON_NODE_ID);
    desc.interaction.pointer_released = true;
    desc.interaction.pointer_pressed_on_button = true;
    desc.hooks.activation_event_key = BUTTON_EVENT_KEY;
    desc.hooks.activation_sound_key = BUTTON_SOUND_KEY;
    desc.normal_visual = MakeButtonVisual(SPRITE_BUTTON_NORMAL_KEY, 1020U, 0.2F);
    desc.hover_visual = MakeButtonVisual(SPRITE_BUTTON_HOVER_KEY, 1021U, 0.3F);
    desc.pressed_visual = MakeButtonVisual(SPRITE_BUTTON_PRESSED_KEY, 1022U, 0.4F);
    desc.disabled_visual = MakeButtonVisual(SPRITE_BUTTON_DISABLED_KEY, 1023U, 0.1F);
    desc.selected_visual = MakeButtonVisual(SPRITE_BUTTON_SELECTED_KEY, 1024U, 0.5F);
    return desc;
}

UiSliderVisualDesc MakeSliderVisual(std::uint32_t sprite_key, std::uint32_t style_key, float tint_seed) {
    UiSliderVisualDesc desc{};
    desc.sprite_key = sprite_key;
    desc.style_key = style_key;
    desc.material_key = style_key + 100U;
    desc.tint = MakeTint(tint_seed, tint_seed + 0.1F, tint_seed + 0.2F, 1.0F);
    return desc;
}

UiSliderComponentDesc MakeSliderDesc() {
    UiSliderComponentDesc desc{};
    desc.node_id = NodeId(SLIDER_TRACK_NODE_ID);
    desc.fill_node_id = NodeId(SLIDER_FILL_NODE_ID);
    desc.handle_node_id = NodeId(SLIDER_HANDLE_NODE_ID);
    desc.interaction.hit_node_id = NodeId(SLIDER_TRACK_NODE_ID);
    desc.hooks.value_changed_event_key = SLIDER_EVENT_KEY;
    desc.fill_visual = MakeSliderVisual(SPRITE_SLIDER_FILL_KEY, 1030U, 0.15F);
    desc.handle_visual = MakeSliderVisual(SPRITE_SLIDER_HANDLE_KEY, 1031U, 0.25F);
    desc.min_value = 0.0F;
    desc.max_value = SLIDER_MAX_VALUE;
    desc.value = SLIDER_VALUE;
    desc.step_size = 5.0F;
    return desc;
}

UiGridViewDesc MakeGridDesc() {
    UiGridViewDesc desc{};
    desc.item_count = GRID_ITEM_COUNT;
    desc.axis_cell_count = GRID_AXIS_CELL_COUNT;
    desc.visible_group_count = GRID_VISIBLE_GROUP_COUNT;
    desc.buffer_group_count = GRID_BUFFER_GROUP_COUNT;
    desc.pool_group_count = GRID_POOL_GROUP_COUNT;
    desc.kind = UiGridViewKind::GridView;
    return desc;
}

bool AppendDrawElement(ComponentDiagnosticsScratch *scratch, const UiDrawElement &draw_element) {
    if (scratch == nullptr) {
        return false;
    }

    if (scratch->draw_element_count >= DIAGNOSTIC_DRAW_ELEMENT_CAPACITY) {
        return false;
    }

    scratch->draw_elements[scratch->draw_element_count] = draw_element;
    ++scratch->draw_element_count;
    return true;
}

bool BuildBatchDiagnostics(
    const ComponentDiagnosticsScratch &scratch,
    UiComponentSmokeSampleResult *sample_result) {
    if (sample_result == nullptr) {
        return false;
    }

    std::array<UiDrawBatch, DIAGNOSTIC_DRAW_ELEMENT_CAPACITY> batches{};
    UiDrawBatchResult result{};
    UiDrawBatcher batcher{};
    const std::span<const UiDrawElement> draw_elements(scratch.draw_elements.data(), scratch.draw_element_count);
    const UiDrawBatchStatus status = batcher.Build(draw_elements, batches, &result);
    if (status != UiDrawBatchStatus::Success || !result.Succeeded()) {
        sample_result->failure_stage = "diagnostics_batch";
        return false;
    }

    sample_result->performance_diagnostics.draw_call_count = result.draw_element_count;
    sample_result->performance_diagnostics.batch_count = result.batch_count;
    return true;
}

bool BuildRebuildDiagnostics(
    const UiNodeTree &tree,
    UiComponentSmokeSampleResult *sample_result) {
    if (sample_result == nullptr) {
        return false;
    }

    UiInvalidationModel invalidation_model{};
    UiInvalidationRequest layout_request{};
    layout_request.node_id = NodeId(WINDOW_NODE_ID);
    layout_request.change_type = UiDirtyChangeType::Layout;
    layout_request.scope = UiInvalidationScope::Subtree;

    std::array<UiInvalidatedNode, DIAGNOSTIC_INVALIDATION_CAPACITY> layout_nodes{};
    UiInvalidationResult layout_result{};
    const UiInvalidationStatus layout_status = invalidation_model.Invalidate(
        tree,
        layout_request,
        layout_nodes,
        &layout_result);
    if (layout_status != UiInvalidationStatus::Success || !layout_result.Succeeded()) {
        sample_result->failure_stage = "diagnostics_layout_rebuild";
        return false;
    }

    UiInvalidationRequest paint_request{};
    paint_request.node_id = NodeId(IMAGE_NODE_ID);
    paint_request.change_type = UiDirtyChangeType::PaintOnly;
    paint_request.scope = UiInvalidationScope::Self;

    std::array<UiInvalidatedNode, DIAGNOSTIC_INVALIDATION_CAPACITY> paint_nodes{};
    UiInvalidationResult paint_result{};
    const UiInvalidationStatus paint_status = invalidation_model.Invalidate(
        tree,
        paint_request,
        paint_nodes,
        &paint_result);
    if (paint_status != UiInvalidationStatus::Success || !paint_result.Succeeded()) {
        sample_result->failure_stage = "diagnostics_paint_rebuild";
        return false;
    }

    sample_result->performance_diagnostics.layout_rebuild_count = layout_result.cache_counters.layout_rebuild_count;
    sample_result->performance_diagnostics.paint_rebuild_count = paint_result.cache_counters.paint_rebuild_count;
    return true;
}

bool BuildPerformanceDiagnostics(
    const UiNodeTree &tree,
    const ComponentDiagnosticsScratch &scratch,
    UiComponentSmokeSampleResult *sample_result) {
    if (sample_result == nullptr) {
        return false;
    }

    if (!BuildBatchDiagnostics(scratch, sample_result)) {
        return false;
    }

    if (!BuildRebuildDiagnostics(tree, sample_result)) {
        return false;
    }

    sample_result->performance_diagnostics.atlas_page_count = scratch.atlas_page_count;
    sample_result->performance_diagnostics.list_cell_count = sample_result->grid_pool_cell_count;
    sample_result->performance_diagnostics.reported = true;
    return true;
}

bool BuildTextComponent(
    const UiNodeTree &tree,
    ComponentDiagnosticsScratch *scratch,
    UiComponentSmokeSampleResult *sample_result) {
    if (sample_result == nullptr) {
        return false;
    }

    if (scratch == nullptr) {
        return false;
    }

    const std::array<std::uint32_t, 4U> codepoints{84U, 69U, 88U, 84U};
    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiFontAssetDesc, 1U> fonts{MakeFont()};
    const std::array<UiFontGlyphDesc, 3U> glyphs{
        MakeGlyph(84U, 2000U, 0U),
        MakeGlyph(69U, 2001U, 10U),
        MakeGlyph(88U, 2002U, 20U)};
    const UiFontGlyphAtlasDesc atlas_desc = MakeFontAtlasDesc(pages, fonts, glyphs);
    const UiTextComponentDesc desc = MakeTextDesc(codepoints);
    std::array<UiTextDrawRecord, 4U> records{};
    UiTextComponentResult result{};
    UiTextComponent component{};
    const UiTextComponentStatus status = component.Build(tree, atlas_desc, desc, records, &result);
    if (status != UiTextComponentStatus::Success || !result.Succeeded()) {
        sample_result->failure_stage = "text_component";
        return false;
    }

    const std::uint32_t expected_record_count = static_cast<std::uint32_t>(codepoints.size());
    if (result.draw_record_count != expected_record_count) {
        sample_result->failure_stage = "text_count";
        return false;
    }

    for (std::uint32_t index = 0U; index < result.draw_record_count; ++index) {
        if (!AppendDrawElement(scratch, records[index].draw_element)) {
            sample_result->failure_stage = "diagnostics_draw";
            return false;
        }
    }

    sample_result->text_component_used = true;
    sample_result->text_draw_record_count = result.draw_record_count;
    return true;
}

bool BuildImageComponent(
    const UiNodeTree &tree,
    const UiStaticAtlasMetadataDesc &atlas_desc,
    ComponentDiagnosticsScratch *scratch,
    UiComponentSmokeSampleResult *sample_result) {
    if (sample_result == nullptr) {
        return false;
    }

    if (scratch == nullptr) {
        return false;
    }

    std::array<UiImageDrawRecord, 1U> records{};
    UiImageComponentResult result{};
    UiImageComponent component{};
    const UiImageComponentDesc desc = MakeImageDesc();
    const UiImageComponentStatus status = component.Build(tree, atlas_desc, desc, records, &result);
    if (status != UiImageComponentStatus::Success || !result.Succeeded()) {
        sample_result->failure_stage = "image_component";
        return false;
    }

    if (result.draw_record_count != 1U || records[0U].sprite_key != SPRITE_IMAGE_KEY) {
        sample_result->failure_stage = "image_count";
        return false;
    }

    if (!AppendDrawElement(scratch, records[0U].draw_element)) {
        sample_result->failure_stage = "diagnostics_draw";
        return false;
    }

    sample_result->image_component_used = true;
    sample_result->image_draw_record_count = result.draw_record_count;
    return true;
}

bool BuildButtonComponent(const UiNodeTree &tree, UiComponentSmokeSampleResult *sample_result) {
    if (sample_result == nullptr) {
        return false;
    }

    UiButtonComponentResult result{};
    UiButtonComponent component{};
    const UiButtonComponentDesc desc = MakeButtonDesc();
    const UiButtonComponentStatus status = component.Build(tree, desc, &result);
    if (status != UiButtonComponentStatus::Success || !result.Succeeded()) {
        sample_result->failure_stage = "button_component";
        return false;
    }

    if (!result.activated || result.activation_source != UiButtonActivationSource::Pointer) {
        sample_result->failure_stage = "button_activation";
        return false;
    }

    if (!result.visual_update.has_image_update || !result.visual_update.has_text_update) {
        sample_result->failure_stage = "button_visual";
        return false;
    }

    sample_result->button_component_used = true;
    sample_result->button_activation_event_key = result.activation_event_key;
    return true;
}

bool BuildSliderComponent(const UiNodeTree &tree, UiComponentSmokeSampleResult *sample_result) {
    if (sample_result == nullptr) {
        return false;
    }

    UiSliderComponentResult result{};
    UiSliderComponent component{};
    const UiSliderComponentDesc desc = MakeSliderDesc();
    const UiSliderComponentStatus status = component.Build(tree, desc, &result);
    if (status != UiSliderComponentStatus::Success || !result.Succeeded()) {
        sample_result->failure_stage = "slider_component";
        return false;
    }

    if (result.adjustment_source != UiSliderAdjustmentSource::None) {
        sample_result->failure_stage = "slider_input";
        return false;
    }

    if (!result.visual_update.has_fill_update || !result.visual_update.has_handle_update) {
        sample_result->failure_stage = "slider_visual";
        return false;
    }

    sample_result->slider_component_used = true;
    sample_result->slider_normalized_value = result.normalized_value;
    sample_result->slider_value_changed_event_key = result.value_changed_event_key;
    return true;
}

bool BuildGridViewList(UiComponentSmokeSampleResult *sample_result) {
    if (sample_result == nullptr) {
        return false;
    }

    UiGridViewVirtualizationDesc desc{};
    desc.grid_desc = MakeGridDesc();
    desc.first_visible_group = 10U;
    desc.selected_index = 52U;
    desc.previous_selected_index = 51U;
    desc.updated_item_index = 53U;
    desc.scroll_to_index = INVALID_UI_GRID_INDEX;
    desc.existing_pool_cell_count = 8U;
    desc.auto_scroll_to_selection = false;

    std::array<UiGridViewVirtualCellRecord, GRID_POOL_CELL_COUNT> cells{};
    UiGridViewVirtualizationResult result{};
    UiGridViewVirtualizer virtualizer{};
    const UiGridViewStatus status = virtualizer.Build(desc, cells, &result);
    if (status != UiGridViewStatus::Success || !result.Succeeded()) {
        sample_result->failure_stage = "grid_view";
        return false;
    }

    if (result.pool_cell_count >= desc.grid_desc.item_count) {
        sample_result->failure_stage = "grid_full_pool";
        return false;
    }

    if (result.visible_item_count == 0U || result.dirty_cell_count != 3U) {
        sample_result->failure_stage = "grid_counts";
        return false;
    }

    sample_result->grid_view_used = true;
    sample_result->grid_item_count = desc.grid_desc.item_count;
    sample_result->grid_visible_item_count = result.visible_item_count;
    sample_result->grid_pool_cell_count = result.pool_cell_count;
    sample_result->grid_dirty_cell_count = result.dirty_cell_count;
    return true;
}

bool BuildComponents(UiNodeTree &tree, UiComponentSmokeSampleResult *sample_result) {
    if (sample_result == nullptr) {
        return false;
    }

    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiStaticAtlasSpriteDesc, 8U> sprites{
        MakeSprite(SPRITE_IMAGE_KEY, 0U, 0U, 64U, 64U),
        MakeSprite(SPRITE_BUTTON_NORMAL_KEY, 64U, 0U, 48U, 24U),
        MakeSprite(SPRITE_BUTTON_HOVER_KEY, 112U, 0U, 48U, 24U),
        MakeSprite(SPRITE_BUTTON_PRESSED_KEY, 160U, 0U, 48U, 24U),
        MakeSprite(SPRITE_BUTTON_DISABLED_KEY, 208U, 0U, 48U, 24U),
        MakeSprite(SPRITE_BUTTON_SELECTED_KEY, 256U, 0U, 48U, 24U),
        MakeSprite(SPRITE_SLIDER_FILL_KEY, 304U, 0U, 48U, 12U),
        MakeSprite(SPRITE_SLIDER_HANDLE_KEY, 352U, 0U, 24U, 24U)};
    const UiStaticAtlasMetadataDesc atlas_desc = MakeAtlasDesc(pages, sprites);
    ComponentDiagnosticsScratch diagnostics_scratch{};
    diagnostics_scratch.atlas_page_count = static_cast<std::uint32_t>(pages.size());

    if (!BuildTextComponent(tree, &diagnostics_scratch, sample_result)) {
        return false;
    }

    if (!BuildImageComponent(tree, atlas_desc, &diagnostics_scratch, sample_result)) {
        return false;
    }

    if (!BuildButtonComponent(tree, sample_result)) {
        return false;
    }

    if (!BuildSliderComponent(tree, sample_result)) {
        return false;
    }

    if (!BuildGridViewList(sample_result)) {
        return false;
    }

    return BuildPerformanceDiagnostics(tree, diagnostics_scratch, sample_result);
}
}

bool RunUiComponentSmokeSample(
    const UiComponentSmokeSampleInput &input,
    UiComponentSmokeSampleResult *result) {
    if (result == nullptr) {
        return false;
    }

    *result = {};
    result->failure_stage = "layout_load";

    ComponentLayout layout{};
    if (!LoadComponentLayout(input.layout_text, &layout)) {
        return false;
    }

    result->layout_loaded = true;
    result->layout_node_count = layout.node_count;

    UiNodeTreeDesc tree_desc{};
    tree_desc.node_capacity = 16U;
    tree_desc.viewport_rect = UiRect{0.0F, 0.0F, static_cast<float>(VIEWPORT_WIDTH), static_cast<float>(VIEWPORT_HEIGHT)};
    UiNodeTree tree(tree_desc);
    result->failure_stage = "node_tree";
    if (!BuildNodeTree(layout, &tree)) {
        return false;
    }

    result->node_tree_built = true;
    result->failure_stage = "component_window";
    if (!BuildComponents(tree, result)) {
        return false;
    }

    result->component_window_built = true;
    result->pass_reported = true;
    result->failure_stage = "success";
    return true;
}
}
