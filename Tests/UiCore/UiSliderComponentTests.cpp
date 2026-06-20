// 模块: Tests UiCore
// 文件: Tests/UiCore/UiSliderComponentTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiDirtyDomain.h"
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
#include "YuEngine/UiCore/UiSliderVisualUpdate.h"

using yuengine::uicore::UiDirtyChangeType;
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
using yuengine::uicore::UiSliderAxis;
using yuengine::uicore::UiSliderComponent;
using yuengine::uicore::UiSliderComponentDesc;
using yuengine::uicore::UiSliderComponentResult;
using yuengine::uicore::UiSliderComponentStatus;
using yuengine::uicore::UiSliderVisualDesc;
using yuengine::uicore::UiSliderVisualUpdate;
using yuengine::uicore::UI_DIRTY_LAYOUT;
using yuengine::uicore::UI_DIRTY_PAINT;

namespace {
constexpr const char *TEST_VALUE_MAPPING =
    "UiCore_SliderComponent_MapsValueToFillAndHandle";
constexpr const char *TEST_POINTER_CAPTURE =
    "UiCore_SliderComponent_PointerCaptureUpdatesValueAndHook";
constexpr const char *TEST_KEYBOARD_GAMEPAD =
    "UiCore_SliderComponent_KeyboardGamepadAdjustmentPathsAreExplicit";
constexpr const char *TEST_VALUE_DIRTY =
    "UiCore_SliderComponent_ValueDirtyDoesNotTriggerLayout";
constexpr const char *TEST_INVALID_DESC =
    "UiCore_SliderComponent_RejectsInvalidDesc";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t TRACK_NODE_ID = 1U;
constexpr std::uint32_t FILL_NODE_ID = 2U;
constexpr std::uint32_t HANDLE_NODE_ID = 3U;
constexpr std::uint32_t OTHER_NODE_ID = 4U;
constexpr std::uint32_t VALUE_CHANGED_EVENT_KEY = 120U;

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

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

UiNodeTreeDesc MakeTreeDesc() {
    UiNodeTreeDesc desc{};
    desc.node_capacity = 8U;
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

UiNodeDesc MakeNodeDesc(std::uint32_t node_id, float x, float y, float width, float height) {
    UiNodeDesc desc{};
    desc.node_id = NodeId(node_id);
    desc.parent_id = UiNodeId{};
    desc.rect_transform = FixedTransform(x, y, width, height);
    desc.sibling_order = node_id;
    desc.layer = 3;
    desc.is_visible = true;
    desc.is_enabled = true;
    desc.is_hit_testable = true;
    return desc;
}

int CreateNode(UiNodeTree &tree, const UiNodeDesc &desc) {
    const UiNodeTreeResult result = tree.CreateNode(desc);
    if (result.Succeeded()) {
        return 0;
    }

    return Fail("slider node create failed");
}

int CreateSliderNodes(UiNodeTree &tree) {
    int ret_code = CreateNode(tree, MakeNodeDesc(TRACK_NODE_ID, 100.0F, 20.0F, 200.0F, 20.0F));
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = CreateNode(tree, MakeNodeDesc(FILL_NODE_ID, 100.0F, 20.0F, 200.0F, 20.0F));
    if (ret_code != 0) {
        return ret_code;
    }

    return CreateNode(tree, MakeNodeDesc(HANDLE_NODE_ID, 90.0F, 10.0F, 20.0F, 40.0F));
}

UiImageTint MakeTint(float seed) {
    UiImageTint tint{};
    tint.red = seed;
    tint.green = seed + 0.1F;
    tint.blue = seed + 0.2F;
    tint.alpha = 1.0F;
    return tint;
}

UiSliderVisualDesc MakeVisual(std::uint32_t sprite_key, std::uint32_t style_key, float seed) {
    UiSliderVisualDesc visual{};
    visual.sprite_key = sprite_key;
    visual.style_key = style_key;
    visual.material_key = style_key + 100U;
    visual.tint = MakeTint(seed);
    return visual;
}

UiSliderComponentDesc MakeSliderDesc() {
    UiSliderComponentDesc desc{};
    desc.node_id = NodeId(TRACK_NODE_ID);
    desc.fill_node_id = NodeId(FILL_NODE_ID);
    desc.handle_node_id = NodeId(HANDLE_NODE_ID);
    desc.hooks.value_changed_event_key = VALUE_CHANGED_EVENT_KEY;
    desc.fill_visual = MakeVisual(50U, 60U, 0.1F);
    desc.handle_visual = MakeVisual(51U, 61U, 0.2F);
    desc.min_value = 0.0F;
    desc.max_value = 100.0F;
    desc.value = 25.0F;
    desc.step_size = 5.0F;
    return desc;
}

int BuildSlider(
    const UiNodeTree &tree,
    const UiSliderComponentDesc &desc,
    UiSliderComponentResult &out_result) {
    UiSliderComponent component{};
    const UiSliderComponentStatus status = component.Build(tree, desc, &out_result);
    if (status == UiSliderComponentStatus::Success && out_result.Succeeded()) {
        return 0;
    }

    return Fail("slider component build failed");
}

int VerifyImageDesc(const UiSliderVisualUpdate &update) {
    if (!update.has_fill_update || !update.has_handle_update) {
        return Fail("slider visual update flags mismatch");
    }

    if (update.fill_image_desc.node_id.value != FILL_NODE_ID) {
        return Fail("slider fill image node mismatch");
    }

    if (update.handle_image_desc.node_id.value != HANDLE_NODE_ID) {
        return Fail("slider handle image node mismatch");
    }

    if (update.fill_image_desc.sprite_key != 50U || update.handle_image_desc.sprite_key != 51U) {
        return Fail("slider visual sprite mismatch");
    }

    return 0;
}

int UiCoreSliderComponentMapsValueToFillAndHandle() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateSliderNodes(tree);
    if (ret_code != 0) {
        return ret_code;
    }

    UiSliderComponentDesc desc = MakeSliderDesc();
    UiSliderComponentResult result{};
    ret_code = BuildSlider(tree, desc, result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (!FloatClose(result.resolved_value, 25.0F) || !FloatClose(result.normalized_value, 0.25F)) {
        return Fail("slider value normalization mismatch");
    }

    if (!FloatClose(result.visual_update.fill_rect.x, 100.0F) ||
        !FloatClose(result.visual_update.fill_rect.width, 50.0F)) {
        return Fail("slider fill rect mismatch");
    }

    if (!FloatClose(result.visual_update.handle_center.x, 150.0F) ||
        !FloatClose(result.visual_update.handle_rect.x, 140.0F)) {
        return Fail("slider handle rect mismatch");
    }

    return VerifyImageDesc(result.visual_update);
}

int UiCoreSliderComponentPointerCaptureUpdatesValueAndHook() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateSliderNodes(tree);
    if (ret_code != 0) {
        return ret_code;
    }

    UiSliderComponentDesc desc = MakeSliderDesc();
    desc.interaction.hit_node_id = NodeId(TRACK_NODE_ID);
    desc.interaction.pointer_is_down = true;
    desc.interaction.pointer_pressed_on_slider = true;
    desc.interaction.pointer_position = {260.0F, 30.0F};

    UiSliderComponentResult result{};
    ret_code = BuildSlider(tree, desc, result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (!result.pointer_capture_active || result.captured_node_id.value != TRACK_NODE_ID) {
        return Fail("slider pointer capture mismatch");
    }

    if (result.adjustment_source != UiSliderAdjustmentSource::Pointer) {
        return Fail("slider pointer adjustment source mismatch");
    }

    if (!result.value_changed || result.value_changed_event_key != VALUE_CHANGED_EVENT_KEY) {
        return Fail("slider value changed hook mismatch");
    }

    if (!FloatClose(result.resolved_value, 80.0F) || !FloatClose(result.visual_update.fill_rect.width, 160.0F)) {
        return Fail("slider pointer value mapping mismatch");
    }

    return 0;
}

int UiCoreSliderComponentKeyboardGamepadAdjustmentPathsAreExplicit() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateSliderNodes(tree);
    if (ret_code != 0) {
        return ret_code;
    }

    UiSliderComponentDesc desc = MakeSliderDesc();
    desc.interaction.keyboard_adjustment_delta = 12.0F;

    UiSliderComponentResult result{};
    ret_code = BuildSlider(tree, desc, result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (result.adjustment_source != UiSliderAdjustmentSource::Keyboard ||
        !result.keyboard_adjustment_path ||
        !FloatClose(result.resolved_value, 35.0F)) {
        return Fail("slider keyboard adjustment mismatch");
    }

    desc = MakeSliderDesc();
    desc.interaction.gamepad_adjustment_delta = -12.0F;
    ret_code = BuildSlider(tree, desc, result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (result.adjustment_source != UiSliderAdjustmentSource::Gamepad ||
        !result.gamepad_adjustment_path ||
        !FloatClose(result.resolved_value, 15.0F)) {
        return Fail("slider gamepad adjustment mismatch");
    }

    desc.is_enabled = false;
    ret_code = BuildSlider(tree, desc, result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (result.adjustment_source != UiSliderAdjustmentSource::None ||
        result.value_changed ||
        !FloatClose(result.resolved_value, 25.0F)) {
        return Fail("disabled slider accepted adjustment");
    }

    return 0;
}

int UiCoreSliderComponentValueDirtyDoesNotTriggerLayout() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateSliderNodes(tree);
    if (ret_code != 0) {
        return ret_code;
    }

    UiSliderComponentDesc desc = MakeSliderDesc();
    desc.interaction.keyboard_adjustment_delta = 5.0F;

    UiSliderComponentResult component_result{};
    ret_code = BuildSlider(tree, desc, component_result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (component_result.dirty_change_type != UiDirtyChangeType::PaintOnly) {
        return Fail("slider value dirty type mismatch");
    }

    UiInvalidationRequest request{};
    request.node_id = NodeId(TRACK_NODE_ID);
    request.change_type = component_result.dirty_change_type;
    request.scope = UiInvalidationScope::Self;

    std::array<UiInvalidatedNode, 1U> affected_nodes{};
    UiInvalidationResult invalidation_result{};
    UiInvalidationModel invalidation_model{};
    const UiInvalidationStatus status =
        invalidation_model.Invalidate(tree, request, affected_nodes, &invalidation_result);
    if (status != UiInvalidationStatus::Success || invalidation_result.affected_node_count != 1U) {
        return Fail("slider value invalidation failed");
    }

    if (invalidation_result.cache_counters.layout_rebuild_count != 0U) {
        return Fail("slider value dirty triggered layout rebuild counter");
    }

    if ((affected_nodes[0U].domains & UI_DIRTY_LAYOUT) != 0U) {
        return Fail("slider value dirty marked layout domain");
    }

    if ((affected_nodes[0U].domains & UI_DIRTY_PAINT) == 0U) {
        return Fail("slider value dirty did not mark paint domain");
    }

    return 0;
}

int UiCoreSliderComponentRejectsInvalidDesc() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateSliderNodes(tree);
    if (ret_code != 0) {
        return ret_code;
    }

    UiSliderComponent component{};
    UiSliderComponentResult result{};
    UiSliderComponentDesc desc = MakeSliderDesc();
    desc.max_value = desc.min_value;
    UiSliderComponentStatus status = component.Build(tree, desc, &result);
    if (status != UiSliderComponentStatus::InvalidDesc ||
        result.status != UiSliderComponentStatus::InvalidDesc) {
        return Fail("slider accepted invalid range");
    }

    desc = MakeSliderDesc();
    desc.fill_visual.sprite_key = 0U;
    status = component.Build(tree, desc, &result);
    if (status != UiSliderComponentStatus::InvalidDesc ||
        result.status != UiSliderComponentStatus::InvalidDesc) {
        return Fail("slider accepted invalid fill visual");
    }

    desc = MakeSliderDesc();
    desc.handle_node_id = NodeId(OTHER_NODE_ID);
    status = component.Build(tree, desc, &result);
    if (status != UiSliderComponentStatus::NodeNotFound ||
        result.status != UiSliderComponentStatus::NodeNotFound) {
        return Fail("slider did not report missing handle");
    }

    desc = MakeSliderDesc();
    UiRectTransform invalid_track = FixedTransform(100.0F, 20.0F, 0.0F, 20.0F);
    const UiNodeTreeResult rect_result = tree.SetNodeRect(NodeId(TRACK_NODE_ID), invalid_track);
    if (!rect_result.Succeeded()) {
        return Fail("slider test could not set invalid track");
    }

    status = component.Build(tree, desc, &result);
    if (status != UiSliderComponentStatus::InvalidDesc ||
        result.status != UiSliderComponentStatus::InvalidDesc) {
        return Fail("slider accepted invalid track geometry");
    }

    status = component.Build(tree, MakeSliderDesc(), nullptr);
    if (status != UiSliderComponentStatus::InvalidOutputBuffer) {
        return Fail("slider accepted null output");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_VALUE_MAPPING) {
        return UiCoreSliderComponentMapsValueToFillAndHandle();
    }

    if (name == TEST_POINTER_CAPTURE) {
        return UiCoreSliderComponentPointerCaptureUpdatesValueAndHook();
    }

    if (name == TEST_KEYBOARD_GAMEPAD) {
        return UiCoreSliderComponentKeyboardGamepadAdjustmentPathsAreExplicit();
    }

    if (name == TEST_VALUE_DIRTY) {
        return UiCoreSliderComponentValueDirtyDoesNotTriggerLayout();
    }

    if (name == TEST_INVALID_DESC) {
        return UiCoreSliderComponentRejectsInvalidDesc();
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
