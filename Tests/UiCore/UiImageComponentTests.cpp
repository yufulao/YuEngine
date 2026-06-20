// 模块: Tests UiCore
// 文件: Tests/UiCore/UiImageComponentTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiDirtyDomain.h"
#include "YuEngine/UiCore/UiDrawBatch.h"
#include "YuEngine/UiCore/UiDrawBatcher.h"
#include "YuEngine/UiCore/UiDrawBatchResult.h"
#include "YuEngine/UiCore/UiDrawBatchStatus.h"
#include "YuEngine/UiCore/UiDrawElement.h"
#include "YuEngine/UiCore/UiDrawElementType.h"
#include "YuEngine/UiCore/UiImageComponent.h"
#include "YuEngine/UiCore/UiImageComponentDesc.h"
#include "YuEngine/UiCore/UiImageComponentResult.h"
#include "YuEngine/UiCore/UiImageComponentStatus.h"
#include "YuEngine/UiCore/UiImageDrawRecord.h"
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
#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"

using yuengine::uicore::UiDirtyChangeType;
using yuengine::uicore::UiDrawBatch;
using yuengine::uicore::UiDrawBatcher;
using yuengine::uicore::UiDrawBatchResult;
using yuengine::uicore::UiDrawBatchStatus;
using yuengine::uicore::UiDrawElement;
using yuengine::uicore::UiDrawElementType;
using yuengine::uicore::UiImageComponent;
using yuengine::uicore::UiImageComponentDesc;
using yuengine::uicore::UiImageComponentResult;
using yuengine::uicore::UiImageComponentStatus;
using yuengine::uicore::UiImageDrawRecord;
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
using yuengine::uicore::UiStaticAtlasMetadataDesc;
using yuengine::uicore::UiStaticAtlasPageDesc;
using yuengine::uicore::UiStaticAtlasSpriteDesc;
using yuengine::uicore::UI_DIRTY_LAYOUT;
using yuengine::uicore::UI_DIRTY_PAINT;

namespace {
constexpr const char *TEST_SPRITE =
    "UiCore_ImageComponent_BuildsSpriteDrawRecord";
constexpr const char *TEST_NINE_SLICE =
    "UiCore_ImageComponent_BuildsNineSliceRecordsAndBatches";
constexpr const char *TEST_MISSING_SPRITE =
    "UiCore_ImageComponent_ReportsMissingSpriteWithoutMutation";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiCore_ImageComponent_RejectsSmallOutputWithoutMutation";
constexpr const char *TEST_PAINT_INVALIDATION =
    "UiCore_ImageComponent_PaintInvalidationDoesNotTriggerLayout";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_SPRITE_KEY = 9999U;
constexpr std::uint32_t PAGE_KEY = 7U;
constexpr std::uint32_t TEXTURE_KEY = 77U;
constexpr std::uint32_t SPRITE_KEY = 11U;

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

UiNodeDesc MakeNodeDesc(std::uint32_t node_id, UiRect rect) {
    UiNodeDesc desc{};
    desc.node_id = NodeId(node_id);
    desc.parent_id = UiNodeId{};
    desc.rect_transform = FixedTransform(rect.x, rect.y, rect.width, rect.height);
    desc.sibling_order = node_id;
    desc.layer = 3;
    desc.is_visible = true;
    desc.is_enabled = true;
    desc.is_hit_testable = true;
    return desc;
}

int CreateNode(UiNodeTree &tree, UiRect rect) {
    const UiNodeDesc node_desc = MakeNodeDesc(1U, rect);
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

UiStaticAtlasSpriteDesc MakeSprite() {
    UiStaticAtlasSpriteDesc desc{};
    desc.sprite_key = SPRITE_KEY;
    desc.page_key = PAGE_KEY;
    desc.x = 64U;
    desc.y = 32U;
    desc.width = 64U;
    desc.height = 32U;
    return desc;
}

UiStaticAtlasSpriteDesc MakeNineSliceSprite() {
    UiStaticAtlasSpriteDesc desc = MakeSprite();
    desc.nine_slice_enabled = true;
    desc.border_left = 4U;
    desc.border_top = 6U;
    desc.border_right = 8U;
    desc.border_bottom = 10U;
    return desc;
}

UiImageComponentDesc MakeImageDesc() {
    UiImageComponentDesc desc{};
    desc.node_id = NodeId(1U);
    desc.sprite_key = SPRITE_KEY;
    desc.style_key = 9U;
    desc.material_key = 13U;
    desc.tint.red = 0.25F;
    desc.tint.green = 0.5F;
    desc.tint.blue = 0.75F;
    desc.tint.alpha = 0.875F;
    desc.scissor_enabled = true;
    return desc;
}

UiImageDrawRecord SentinelRecord() {
    UiImageDrawRecord record{};
    record.sprite_key = SENTINEL_SPRITE_KEY;
    record.slice_index = SENTINEL_SPRITE_KEY;
    record.draw_element.texture_key = SENTINEL_SPRITE_KEY;
    return record;
}

bool RecordMatchesSentinel(const UiImageDrawRecord &record) {
    if (record.sprite_key != SENTINEL_SPRITE_KEY) {
        return false;
    }

    if (record.slice_index != SENTINEL_SPRITE_KEY) {
        return false;
    }

    return record.draw_element.texture_key == SENTINEL_SPRITE_KEY;
}

int UiCoreImageComponentBuildsSpriteDrawRecord() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateNode(tree, UiRect{10.0F, 20.0F, 100.0F, 50.0F});
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{MakeSprite()};
    const UiStaticAtlasMetadataDesc atlas_desc{pages, sprites};
    std::array<UiImageDrawRecord, 1U> records{};
    UiImageComponentResult result{};
    UiImageComponent component{};
    const UiImageComponentDesc desc = MakeImageDesc();
    const UiImageComponentStatus status = component.Build(tree, atlas_desc, desc, records, &result);
    if (status != UiImageComponentStatus::Success || !result.Succeeded()) {
        return Fail("image component rejected sprite fixture");
    }

    if (result.draw_record_count != 1U || result.required_draw_record_count != 1U) {
        return Fail("sprite draw record count mismatch");
    }

    const UiImageDrawRecord &record = records[0U];
    if (record.draw_element.type != UiDrawElementType::TexturedQuad || record.draw_element.texture_key != TEXTURE_KEY) {
        return Fail("sprite draw element state mismatch");
    }

    if (!RectClose(record.draw_element.rect, 10.0F, 20.0F, 100.0F, 50.0F)) {
        return Fail("sprite draw rect mismatch");
    }

    if (record.draw_element.style_key != 9U || record.draw_element.material_key != 13U) {
        return Fail("sprite draw style material mismatch");
    }

    if (!record.draw_element.scissor_enabled || record.sprite_key != SPRITE_KEY) {
        return Fail("sprite draw flags mismatch");
    }

    if (!FloatClose(record.uv_rect.u_min, 0.25F) || !FloatClose(record.uv_rect.v_min, 0.25F)) {
        return Fail("sprite uv min mismatch");
    }

    if (!FloatClose(record.uv_rect.u_max, 0.5F) || !FloatClose(record.uv_rect.v_max, 0.5F)) {
        return Fail("sprite uv max mismatch");
    }

    if (!FloatClose(record.tint.blue, 0.75F) || !FloatClose(record.tint.alpha, 0.875F)) {
        return Fail("sprite tint mismatch");
    }

    return 0;
}

int UiCoreImageComponentBuildsNineSliceRecordsAndBatches() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateNode(tree, UiRect{10.0F, 20.0F, 100.0F, 80.0F});
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{MakeNineSliceSprite()};
    const UiStaticAtlasMetadataDesc atlas_desc{pages, sprites};
    std::array<UiImageDrawRecord, 9U> records{};
    UiImageComponentResult result{};
    UiImageComponent component{};
    const UiImageComponentDesc desc = MakeImageDesc();
    const UiImageComponentStatus status = component.Build(tree, atlas_desc, desc, records, &result);
    if (status != UiImageComponentStatus::Success || result.draw_record_count != 9U) {
        return Fail("image component did not emit nine-slice records");
    }

    if (!RectClose(records[0U].draw_element.rect, 10.0F, 20.0F, 4.0F, 10.0F)) {
        return Fail("nine-slice first rect mismatch");
    }

    if (!RectClose(records[8U].draw_element.rect, 102.0F, 94.0F, 8.0F, 6.0F)) {
        return Fail("nine-slice last rect mismatch");
    }

    if (records[0U].slice_index != 0U || records[8U].slice_index != 8U) {
        return Fail("nine-slice index mismatch");
    }

    std::array<UiDrawElement, 9U> elements{};
    for (std::uint32_t index = 0U; index < 9U; ++index) {
        elements[index] = records[index].draw_element;
    }

    std::array<UiDrawBatch, 9U> batches{};
    UiDrawBatchResult batch_result{};
    UiDrawBatcher batcher{};
    const UiDrawBatchStatus batch_status = batcher.Build(elements, batches, &batch_result);
    if (batch_status != UiDrawBatchStatus::Success || batch_result.batch_count != 1U) {
        return Fail("nine-slice records did not batch together");
    }

    if (batches[0U].element_count != 9U || batches[0U].key.texture_key != TEXTURE_KEY) {
        return Fail("nine-slice batch state mismatch");
    }

    return 0;
}

int UiCoreImageComponentReportsMissingSpriteWithoutMutation() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateNode(tree, UiRect{10.0F, 20.0F, 100.0F, 50.0F});
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    UiStaticAtlasSpriteDesc sprite = MakeSprite();
    sprite.sprite_key = 12U;
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{sprite};
    const UiStaticAtlasMetadataDesc atlas_desc{pages, sprites};
    std::array<UiImageDrawRecord, 1U> records{SentinelRecord()};
    UiImageComponentResult result{};
    UiImageComponent component{};
    const UiImageComponentDesc desc = MakeImageDesc();
    const UiImageComponentStatus status = component.Build(tree, atlas_desc, desc, records, &result);
    if (status != UiImageComponentStatus::SpriteNotFound) {
        return Fail("missing sprite did not report explicit status");
    }

    if (!RecordMatchesSentinel(records[0U]) || result.draw_record_count != 0U) {
        return Fail("missing sprite mutated output");
    }

    return 0;
}

int UiCoreImageComponentRejectsSmallOutputWithoutMutation() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateNode(tree, UiRect{10.0F, 20.0F, 100.0F, 80.0F});
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{MakeNineSliceSprite()};
    const UiStaticAtlasMetadataDesc atlas_desc{pages, sprites};
    std::array<UiImageDrawRecord, 1U> records{SentinelRecord()};
    UiImageComponentResult result{};
    UiImageComponent component{};
    const UiImageComponentDesc desc = MakeImageDesc();
    const UiImageComponentStatus status = component.Build(tree, atlas_desc, desc, records, &result);
    if (status != UiImageComponentStatus::OutputCapacityExceeded) {
        return Fail("small output was not rejected");
    }

    if (result.required_draw_record_count != 9U || !RecordMatchesSentinel(records[0U])) {
        return Fail("small output mutation or count mismatch");
    }

    return 0;
}

int UiCoreImageComponentPaintInvalidationDoesNotTriggerLayout() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateNode(tree, UiRect{10.0F, 20.0F, 100.0F, 50.0F});
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakePage()};
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{MakeSprite()};
    const UiStaticAtlasMetadataDesc atlas_desc{pages, sprites};
    std::array<UiImageDrawRecord, 1U> records{};
    UiImageComponentResult component_result{};
    UiImageComponent component{};
    const UiImageComponentDesc desc = MakeImageDesc();
    const UiImageComponentStatus component_status = component.Build(tree, atlas_desc, desc, records, &component_result);
    if (component_status != UiImageComponentStatus::Success) {
        return Fail("image component build failed before invalidation");
    }

    if (component_result.dirty_change_type != UiDirtyChangeType::PaintOnly) {
        return Fail("image component did not expose paint-only dirty type");
    }

    UiInvalidationRequest request{};
    request.node_id = NodeId(1U);
    request.change_type = component_result.dirty_change_type;
    request.scope = UiInvalidationScope::Self;
    std::array<UiInvalidatedNode, 1U> affected_nodes{};
    UiInvalidationResult invalidation_result{};
    UiInvalidationModel invalidation_model{};
    const UiInvalidationStatus invalidation_status =
        invalidation_model.Invalidate(tree, request, affected_nodes, &invalidation_result);
    if (invalidation_status != UiInvalidationStatus::Success || invalidation_result.affected_node_count != 1U) {
        return Fail("image paint invalidation failed");
    }

    if (invalidation_result.cache_counters.layout_rebuild_count != 0U) {
        return Fail("image paint invalidation triggered layout counter");
    }

    if ((affected_nodes[0U].domains & UI_DIRTY_LAYOUT) != 0U) {
        return Fail("image paint invalidation marked layout domain");
    }

    if ((affected_nodes[0U].domains & UI_DIRTY_PAINT) == 0U) {
        return Fail("image paint invalidation did not mark paint domain");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_SPRITE) {
        return UiCoreImageComponentBuildsSpriteDrawRecord();
    }

    if (name == TEST_NINE_SLICE) {
        return UiCoreImageComponentBuildsNineSliceRecordsAndBatches();
    }

    if (name == TEST_MISSING_SPRITE) {
        return UiCoreImageComponentReportsMissingSpriteWithoutMutation();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiCoreImageComponentRejectsSmallOutputWithoutMutation();
    }

    if (name == TEST_PAINT_INVALIDATION) {
        return UiCoreImageComponentPaintInvalidationDoesNotTriggerLayout();
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
