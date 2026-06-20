// 模块: Tests UiCore
// 文件: Tests/UiCore/UiButtonComponentTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiButtonActivationSource.h"
#include "YuEngine/UiCore/UiButtonComponent.h"
#include "YuEngine/UiCore/UiButtonComponentDesc.h"
#include "YuEngine/UiCore/UiButtonComponentResult.h"
#include "YuEngine/UiCore/UiButtonComponentStatus.h"
#include "YuEngine/UiCore/UiButtonState.h"
#include "YuEngine/UiCore/UiButtonVisualStateDesc.h"
#include "YuEngine/UiCore/UiButtonVisualUpdate.h"
#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiDirtyDomain.h"
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

using yuengine::uicore::UiButtonActivationSource;
using yuengine::uicore::UiButtonComponent;
using yuengine::uicore::UiButtonComponentDesc;
using yuengine::uicore::UiButtonComponentResult;
using yuengine::uicore::UiButtonComponentStatus;
using yuengine::uicore::UiButtonState;
using yuengine::uicore::UiButtonVisualStateDesc;
using yuengine::uicore::UiButtonVisualUpdate;
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
using yuengine::uicore::UI_DIRTY_LAYOUT;
using yuengine::uicore::UI_DIRTY_PAINT;

namespace {
constexpr const char *TEST_STATE_VISUALS =
    "UiCore_ButtonComponent_ResolvesStateVisuals";
constexpr const char *TEST_POINTER_HOOKS =
    "UiCore_ButtonComponent_PointerActivationEmitsHooks";
constexpr const char *TEST_KEYBOARD_GAMEPAD =
    "UiCore_ButtonComponent_KeyboardGamepadActivationPathsAreExplicit";
constexpr const char *TEST_STATE_DIRTY =
    "UiCore_ButtonComponent_StateDirtyDoesNotTriggerLayout";
constexpr const char *TEST_INVALID_DESC =
    "UiCore_ButtonComponent_RejectsInvalidDesc";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t BUTTON_NODE_ID = 1U;
constexpr std::uint32_t IMAGE_NODE_ID = 2U;
constexpr std::uint32_t TEXT_NODE_ID = 3U;
constexpr std::uint32_t OTHER_NODE_ID = 4U;
constexpr std::uint32_t EVENT_KEY = 90U;
constexpr std::uint32_t SOUND_KEY = 91U;

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

bool TintMatches(const UiImageTint &tint, float red, float green, float blue, float alpha) {
    if (!FloatClose(tint.red, red)) {
        return false;
    }

    if (!FloatClose(tint.green, green)) {
        return false;
    }

    if (!FloatClose(tint.blue, blue)) {
        return false;
    }

    return FloatClose(tint.alpha, alpha);
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

UiNodeDesc MakeNodeDesc(std::uint32_t node_id) {
    UiNodeDesc desc{};
    desc.node_id = NodeId(node_id);
    desc.parent_id = UiNodeId{};
    desc.rect_transform = FixedTransform(10.0F, 20.0F, 160.0F, 40.0F);
    desc.sibling_order = node_id;
    desc.layer = 3;
    desc.is_visible = true;
    desc.is_enabled = true;
    desc.is_hit_testable = true;
    return desc;
}

int CreateButtonNode(UiNodeTree &tree) {
    const UiNodeDesc node_desc = MakeNodeDesc(BUTTON_NODE_ID);
    const UiNodeTreeResult result = tree.CreateNode(node_desc);
    if (result.Succeeded()) {
        return 0;
    }

    return Fail("button node create failed");
}

UiImageTint MakeTint(float seed) {
    UiImageTint tint{};
    tint.red = seed;
    tint.green = seed + 0.1F;
    tint.blue = seed + 0.2F;
    tint.alpha = 1.0F;
    return tint;
}

UiButtonVisualStateDesc MakeVisual(std::uint32_t sprite_key, std::uint32_t style_key, float tint_seed) {
    UiButtonVisualStateDesc visual{};
    visual.image_sprite_key = sprite_key;
    visual.image_style_key = style_key;
    visual.image_material_key = style_key + 100U;
    visual.image_tint = MakeTint(tint_seed);
    visual.text_style_key = style_key + 200U;
    visual.text_material_key = style_key + 300U;
    visual.text_tint = MakeTint(tint_seed + 0.05F);
    return visual;
}

UiButtonComponentDesc MakeButtonDesc() {
    UiButtonComponentDesc desc{};
    desc.node_id = NodeId(BUTTON_NODE_ID);
    desc.image_node_id = NodeId(IMAGE_NODE_ID);
    desc.text_node_id = NodeId(TEXT_NODE_ID);
    desc.hooks.activation_event_key = EVENT_KEY;
    desc.hooks.activation_sound_key = SOUND_KEY;
    desc.normal_visual = MakeVisual(10U, 20U, 0.1F);
    desc.hover_visual = MakeVisual(11U, 21U, 0.2F);
    desc.pressed_visual = MakeVisual(12U, 22U, 0.3F);
    desc.disabled_visual = MakeVisual(13U, 23U, 0.4F);
    desc.selected_visual = MakeVisual(14U, 24U, 0.5F);
    return desc;
}

int BuildButton(
    const UiNodeTree &tree,
    const UiButtonComponentDesc &desc,
    UiButtonComponentResult &out_result) {
    UiButtonComponent component{};
    const UiButtonComponentStatus status = component.Build(tree, desc, &out_result);
    if (status == UiButtonComponentStatus::Success && out_result.Succeeded()) {
        return 0;
    }

    return Fail("button component build failed");
}

int VerifyVisualUpdate(
    const UiButtonComponentResult &result,
    const UiButtonVisualStateDesc &visual) {
    const UiButtonVisualUpdate &update = result.visual_update;
    if (!update.has_image_update || !update.has_text_update) {
        return Fail("button visual update flags mismatch");
    }

    if (update.image_desc.node_id.value != IMAGE_NODE_ID || update.text_desc.node_id.value != TEXT_NODE_ID) {
        return Fail("button visual node ids mismatch");
    }

    if (update.image_desc.sprite_key != visual.image_sprite_key ||
        update.image_desc.style_key != visual.image_style_key ||
        update.image_desc.material_key != visual.image_material_key) {
        return Fail("button image visual state mismatch");
    }

    if (update.text_desc.style_key != visual.text_style_key ||
        update.text_desc.material_key != visual.text_material_key) {
        return Fail("button text visual state mismatch");
    }

    if (!TintMatches(
            update.image_desc.tint,
            visual.image_tint.red,
            visual.image_tint.green,
            visual.image_tint.blue,
            visual.image_tint.alpha)) {
        return Fail("button image tint mismatch");
    }

    if (!TintMatches(
            update.text_desc.tint,
            visual.text_tint.red,
            visual.text_tint.green,
            visual.text_tint.blue,
            visual.text_tint.alpha)) {
        return Fail("button text tint mismatch");
    }

    return 0;
}

int VerifyStateVisual(
    const UiNodeTree &tree,
    UiButtonComponentDesc desc,
    UiButtonState expected_state,
    const UiButtonVisualStateDesc &expected_visual) {
    UiButtonComponentResult result{};
    int ret_code = BuildButton(tree, desc, result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (result.state != expected_state || result.activated) {
        return Fail("button state or activation mismatch");
    }

    return VerifyVisualUpdate(result, expected_visual);
}

int VerifyStateDirty(UiNodeTree &tree, UiButtonComponentDesc desc) {
    UiButtonComponentResult component_result{};
    int ret_code = BuildButton(tree, desc, component_result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (component_result.dirty_change_type != UiDirtyChangeType::HoverState) {
        return Fail("button state dirty type mismatch");
    }

    UiInvalidationRequest request{};
    request.node_id = NodeId(BUTTON_NODE_ID);
    request.change_type = component_result.dirty_change_type;
    request.scope = UiInvalidationScope::Self;

    std::array<UiInvalidatedNode, 1U> affected_nodes{};
    UiInvalidationResult invalidation_result{};
    UiInvalidationModel invalidation_model{};
    const UiInvalidationStatus status =
        invalidation_model.Invalidate(tree, request, affected_nodes, &invalidation_result);
    if (status != UiInvalidationStatus::Success || invalidation_result.affected_node_count != 1U) {
        return Fail("button state invalidation failed");
    }

    if (invalidation_result.cache_counters.layout_rebuild_count != 0U) {
        return Fail("button state dirty triggered layout rebuild counter");
    }

    if ((affected_nodes[0U].domains & UI_DIRTY_LAYOUT) != 0U) {
        return Fail("button state dirty marked layout domain");
    }

    if ((affected_nodes[0U].domains & UI_DIRTY_PAINT) == 0U) {
        return Fail("button state dirty did not mark paint domain");
    }

    return 0;
}

int UiCoreButtonComponentResolvesStateVisuals() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateButtonNode(tree);
    if (ret_code != 0) {
        return ret_code;
    }

    UiButtonComponentDesc desc = MakeButtonDesc();
    ret_code = VerifyStateVisual(tree, desc, UiButtonState::Normal, desc.normal_visual);
    if (ret_code != 0) {
        return ret_code;
    }

    desc.interaction.hit_node_id = NodeId(BUTTON_NODE_ID);
    ret_code = VerifyStateVisual(tree, desc, UiButtonState::Hover, desc.hover_visual);
    if (ret_code != 0) {
        return ret_code;
    }

    desc.interaction.pointer_is_down = true;
    ret_code = VerifyStateVisual(tree, desc, UiButtonState::Pressed, desc.pressed_visual);
    if (ret_code != 0) {
        return ret_code;
    }

    desc.interaction.pointer_is_down = false;
    desc.interaction.hit_node_id = NodeId(OTHER_NODE_ID);
    desc.is_selected = true;
    ret_code = VerifyStateVisual(tree, desc, UiButtonState::Selected, desc.selected_visual);
    if (ret_code != 0) {
        return ret_code;
    }

    desc.is_enabled = false;
    desc.is_selected = false;
    ret_code = VerifyStateVisual(tree, desc, UiButtonState::Disabled, desc.disabled_visual);
    if (ret_code != 0) {
        return ret_code;
    }

    return 0;
}

int UiCoreButtonComponentPointerActivationEmitsHooks() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateButtonNode(tree);
    if (ret_code != 0) {
        return ret_code;
    }

    UiButtonComponentDesc desc = MakeButtonDesc();
    desc.interaction.hit_node_id = NodeId(BUTTON_NODE_ID);
    desc.interaction.pointer_released = true;
    desc.interaction.pointer_pressed_on_button = true;

    UiButtonComponentResult result{};
    ret_code = BuildButton(tree, desc, result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (!result.activated || result.activation_source != UiButtonActivationSource::Pointer) {
        return Fail("button pointer activation source mismatch");
    }

    if (result.activation_event_key != EVENT_KEY || result.activation_sound_key != SOUND_KEY) {
        return Fail("button pointer activation hooks mismatch");
    }

    if (result.state != UiButtonState::Hover) {
        return Fail("button pointer release did not keep hover visual state");
    }

    return 0;
}

int UiCoreButtonComponentKeyboardGamepadActivationPathsAreExplicit() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateButtonNode(tree);
    if (ret_code != 0) {
        return ret_code;
    }

    UiButtonComponentDesc desc = MakeButtonDesc();
    desc.interaction.keyboard_activate_requested = true;

    UiButtonComponentResult result{};
    ret_code = BuildButton(tree, desc, result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (!result.activated || result.activation_source != UiButtonActivationSource::Keyboard) {
        return Fail("button keyboard activation source mismatch");
    }

    desc.interaction.keyboard_activate_requested = false;
    desc.interaction.gamepad_activate_requested = true;
    ret_code = BuildButton(tree, desc, result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (!result.activated || result.activation_source != UiButtonActivationSource::Gamepad) {
        return Fail("button gamepad activation source mismatch");
    }

    desc.is_enabled = false;
    ret_code = BuildButton(tree, desc, result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (result.activated || result.activation_source != UiButtonActivationSource::None) {
        return Fail("disabled button accepted activation request");
    }

    return 0;
}

int UiCoreButtonComponentStateDirtyDoesNotTriggerLayout() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateButtonNode(tree);
    if (ret_code != 0) {
        return ret_code;
    }

    UiButtonComponentDesc desc = MakeButtonDesc();
    desc.interaction.hit_node_id = NodeId(BUTTON_NODE_ID);
    ret_code = VerifyStateDirty(tree, desc);
    if (ret_code != 0) {
        return ret_code;
    }

    desc.interaction.pointer_is_down = true;
    ret_code = VerifyStateDirty(tree, desc);
    if (ret_code != 0) {
        return ret_code;
    }

    desc.interaction.pointer_is_down = false;
    desc.interaction.hit_node_id = NodeId(OTHER_NODE_ID);
    desc.is_selected = true;
    ret_code = VerifyStateDirty(tree, desc);
    if (ret_code != 0) {
        return ret_code;
    }

    desc.is_selected = false;
    desc.is_enabled = false;
    ret_code = VerifyStateDirty(tree, desc);
    if (ret_code != 0) {
        return ret_code;
    }

    return 0;
}

int UiCoreButtonComponentRejectsInvalidDesc() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateButtonNode(tree);
    if (ret_code != 0) {
        return ret_code;
    }

    UiButtonComponent component{};
    UiButtonComponentResult result{};
    UiButtonComponentDesc desc = MakeButtonDesc();
    desc.image_node_id = UiNodeId{};
    desc.text_node_id = UiNodeId{};
    UiButtonComponentStatus status = component.Build(tree, desc, &result);
    if (status != UiButtonComponentStatus::InvalidDesc || result.activated) {
        return Fail("button accepted missing visual target");
    }

    desc = MakeButtonDesc();
    desc.hover_visual.image_sprite_key = 0U;
    status = component.Build(tree, desc, &result);
    if (status != UiButtonComponentStatus::InvalidDesc || result.status != UiButtonComponentStatus::InvalidDesc) {
        return Fail("button accepted invalid image visual");
    }

    desc = MakeButtonDesc();
    desc.node_id = NodeId(OTHER_NODE_ID);
    status = component.Build(tree, desc, &result);
    if (status != UiButtonComponentStatus::NodeNotFound || result.status != UiButtonComponentStatus::NodeNotFound) {
        return Fail("button did not report missing node");
    }

    status = component.Build(tree, MakeButtonDesc(), nullptr);
    if (status != UiButtonComponentStatus::InvalidOutputBuffer) {
        return Fail("button accepted null output");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_STATE_VISUALS) {
        return UiCoreButtonComponentResolvesStateVisuals();
    }

    if (name == TEST_POINTER_HOOKS) {
        return UiCoreButtonComponentPointerActivationEmitsHooks();
    }

    if (name == TEST_KEYBOARD_GAMEPAD) {
        return UiCoreButtonComponentKeyboardGamepadActivationPathsAreExplicit();
    }

    if (name == TEST_STATE_DIRTY) {
        return UiCoreButtonComponentStateDirtyDoesNotTriggerLayout();
    }

    if (name == TEST_INVALID_DESC) {
        return UiCoreButtonComponentRejectsInvalidDesc();
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
