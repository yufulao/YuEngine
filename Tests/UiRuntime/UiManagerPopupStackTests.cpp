// 模块: Tests UiRuntime
// 文件: Tests/UiRuntime/UiManagerPopupStackTests.cpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/UiRuntime/BaseUiController.h"
#include "YuEngine/UiRuntime/BaseUiLifecycleSnapshot.h"
#include "YuEngine/UiRuntime/BaseUiLifecycleStatus.h"
#include "YuEngine/UiRuntime/UiManagerLayerId.h"
#include "YuEngine/UiRuntime/UiManagerLayerModel.h"
#include "YuEngine/UiRuntime/UiManagerLayerRecord.h"
#include "YuEngine/UiRuntime/UiManagerLayerType.h"
#include "YuEngine/UiRuntime/UiManagerPanelLayerBinding.h"
#include "YuEngine/UiRuntime/UiManagerPanelMap.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapRecord.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapResult.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapStatus.h"
#include "YuEngine/UiRuntime/UiManagerPopupStack.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackConstants.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackResult.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackStatus.h"
#include "YuEngine/UiRuntime/UiPanelId.h"
#include "YuEngine/UiRuntime/UiPanelManifestRecord.h"
#include "YuEngine/UiRuntime/UiPanelRegistry.h"
#include "YuEngine/UiRuntime/UiPanelRegistryResult.h"

using yuengine::uiruntime::BaseUiController;
using yuengine::uiruntime::BaseUiLifecycleSnapshot;
using yuengine::uiruntime::BaseUiLifecycleStatus;
using yuengine::uiruntime::MAX_UI_MANAGER_POPUP_STACK_COUNT;
using yuengine::uiruntime::UiManagerLayerId;
using yuengine::uiruntime::UiManagerLayerModel;
using yuengine::uiruntime::UiManagerLayerRecord;
using yuengine::uiruntime::UiManagerLayerRootRef;
using yuengine::uiruntime::UiManagerLayerType;
using yuengine::uiruntime::UiManagerPanelLayerBinding;
using yuengine::uiruntime::UiManagerPanelMap;
using yuengine::uiruntime::UiManagerPanelMapRecord;
using yuengine::uiruntime::UiManagerPanelMapResult;
using yuengine::uiruntime::UiManagerPanelMapSnapshot;
using yuengine::uiruntime::UiManagerPanelMapStatus;
using yuengine::uiruntime::UiManagerPopupStack;
using yuengine::uiruntime::UiManagerPopupStackResult;
using yuengine::uiruntime::UiManagerPopupStackSnapshot;
using yuengine::uiruntime::UiManagerPopupStackStatus;
using yuengine::uiruntime::UiPanelControllerRef;
using yuengine::uiruntime::UiPanelId;
using yuengine::uiruntime::UiPanelLayoutRef;
using yuengine::uiruntime::UiPanelManifestRecord;
using yuengine::uiruntime::UiPanelRegistry;
using yuengine::uiruntime::UiPanelRegistryResult;
using yuengine::uiruntime::UiPanelResourceRef;

namespace {
constexpr const char *TEST_OPEN_ORDER =
    "UiRuntime_ManagerPopupStack_OpenPushesPopupOrder";
constexpr const char *TEST_BRING_TO_TOP =
    "UiRuntime_ManagerPopupStack_BringExistingPopupToTop";
constexpr const char *TEST_CLOSE_REMOVES =
    "UiRuntime_ManagerPopupStack_CloseRemovesPopupFromStack";
constexpr const char *TEST_DUPLICATE_OPEN =
    "UiRuntime_ManagerPopupStack_DuplicateOpenMovesExistingPopup";
constexpr const char *TEST_REJECTS_STATUS =
    "UiRuntime_ManagerPopupStack_RejectsMissingNonPopupAndUntrackedStatus";
constexpr const char *TEST_RELEASE_REMOVES =
    "UiRuntime_ManagerPopupStack_ReleaseRemovesPopupAndPanelCache";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

class TestPanelController final : public BaseUiController {
protected:
    BaseUiLifecycleStatus OnInitEvent() override {
        return BaseUiLifecycleStatus::Success;
    }

    BaseUiLifecycleStatus OnBindEvent() override {
        return BaseUiLifecycleStatus::Success;
    }

    BaseUiLifecycleStatus OnOpenEvent() override {
        return BaseUiLifecycleStatus::Success;
    }

    BaseUiLifecycleStatus OnCloseEvent() override {
        return BaseUiLifecycleStatus::Success;
    }

    BaseUiLifecycleStatus OnClearEvent() override {
        return BaseUiLifecycleStatus::Success;
    }
};

UiPanelId PanelId(std::uint32_t value) {
    return UiPanelId{value};
}

UiManagerLayerId LayerId(std::uint32_t value) {
    return UiManagerLayerId{value};
}

UiPanelLayoutRef LayoutRef(std::uint32_t asset_key, std::uint32_t variant_key) {
    UiPanelLayoutRef ref{};
    ref.layout_asset_key = asset_key;
    ref.layout_variant_key = variant_key;
    return ref;
}

UiPanelControllerRef ControllerRef(std::uint32_t type_key) {
    UiPanelControllerRef ref{};
    ref.controller_type_key = type_key;
    return ref;
}

UiPanelResourceRef ResourceRef(std::uint32_t resource_key, std::uint32_t type_key) {
    UiPanelResourceRef ref{};
    ref.resource_key = resource_key;
    ref.resource_type_key = type_key;
    return ref;
}

UiManagerLayerRootRef RootRef(std::uint32_t root_key) {
    UiManagerLayerRootRef ref{};
    ref.root_key = root_key;
    return ref;
}

UiPanelManifestRecord PanelRecord(
    std::uint32_t panel_id,
    std::uint32_t layout_key,
    std::uint32_t controller_key,
    std::uint32_t resource_key) {
    UiPanelManifestRecord record{};
    record.panel_id = PanelId(panel_id);
    record.layout_ref = LayoutRef(layout_key, layout_key + 10U);
    record.controller_ref = ControllerRef(controller_key);
    record.resource_refs[0U] = ResourceRef(resource_key, 800U);
    record.resource_ref_count = 1U;
    return record;
}

UiManagerLayerRecord LayerRecord(
    std::uint32_t layer_id,
    UiManagerLayerType type,
    std::int32_t order,
    std::uint32_t root_key) {
    UiManagerLayerRecord record{};
    record.layer_id = LayerId(layer_id);
    record.type = type;
    record.order = order;
    record.root_ref = RootRef(root_key);
    return record;
}

UiManagerPanelLayerBinding PanelBinding(std::uint32_t panel_id, std::uint32_t layer_id) {
    UiManagerPanelLayerBinding binding{};
    binding.panel_id = PanelId(panel_id);
    binding.layer_id = LayerId(layer_id);
    return binding;
}

int RequireStatus(
    UiManagerPopupStackStatus actual,
    UiManagerPopupStackStatus expected,
    std::string_view message) {
    if (actual == expected) {
        return 0;
    }

    return Fail(message);
}

int RegisterLayer(
    UiManagerLayerModel *layer_model,
    std::uint32_t layer_id,
    UiManagerLayerType type,
    std::int32_t order,
    std::uint32_t root_key) {
    if (layer_model == nullptr) {
        return Fail("missing layer model");
    }

    const UiManagerLayerRecord record = LayerRecord(layer_id, type, order, root_key);
    if (!layer_model->RegisterLayer(record).Succeeded()) {
        return Fail("layer register failed");
    }

    return 0;
}

int RegisterPanelAndBind(
    UiPanelRegistry *registry,
    UiManagerLayerModel *layer_model,
    std::uint32_t panel_id,
    std::uint32_t layer_id) {
    if (registry == nullptr || layer_model == nullptr) {
        return Fail("missing panel setup model");
    }

    const UiPanelRegistryResult panel_result =
        registry->RegisterPanel(PanelRecord(panel_id, 100U + panel_id, 200U + panel_id, 300U + panel_id));
    if (!panel_result.Succeeded()) {
        return Fail("panel register failed");
    }

    if (!layer_model->BindPanelToLayer(PanelBinding(panel_id, layer_id)).Succeeded()) {
        return Fail("panel layer binding failed");
    }

    return 0;
}

int RequirePopupOrder(
    const UiManagerPopupStack &popup_stack,
    std::span<const std::uint32_t> expected_order,
    std::string_view message) {
    std::array<UiPanelId, MAX_UI_MANAGER_POPUP_STACK_COUNT> output_order{};
    const std::uint32_t expected_count = static_cast<std::uint32_t>(expected_order.size());
    const UiManagerPopupStackStatus export_status =
        popup_stack.ExportPopupOrder(output_order.data(), expected_count);
    if (export_status != UiManagerPopupStackStatus::Success) {
        return Fail(message);
    }

    const UiManagerPopupStackSnapshot snapshot = popup_stack.Snapshot();
    if (snapshot.popup_count != expected_count) {
        return Fail(message);
    }

    for (std::uint32_t index = 0U; index < expected_count; ++index) {
        const std::uint32_t expected_panel_id = expected_order[static_cast<std::size_t>(index)];
        if (output_order[index].value != expected_panel_id) {
            return Fail(message);
        }
    }

    if (expected_count == 0U && snapshot.top_panel_id.IsValid()) {
        return Fail(message);
    }

    if (expected_count == 0U) {
        return 0;
    }

    const std::uint32_t top_index = expected_count - 1U;
    if (snapshot.top_panel_id.value != expected_order[static_cast<std::size_t>(top_index)]) {
        return Fail(message);
    }

    return 0;
}

int SetupPopupLayer(
    UiPanelRegistry *registry,
    UiManagerLayerModel *layer_model,
    std::span<const std::uint32_t> panel_ids) {
    if (registry == nullptr || layer_model == nullptr) {
        return Fail("missing popup setup model");
    }

    if (RegisterLayer(layer_model, 30U, UiManagerLayerType::Popup, 30, 930U) != 0) {
        return 1;
    }

    for (std::uint32_t panel_id : panel_ids) {
        if (RegisterPanelAndBind(registry, layer_model, panel_id, 30U) != 0) {
            return 1;
        }
    }

    return 0;
}

int RunOpenPushesPopupOrderTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 3U> panel_ids{901U, 902U, 903U};
    if (SetupPopupLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    TestPanelController first_controller;
    TestPanelController second_controller;
    TestPanelController third_controller;

    UiManagerPopupStackResult result =
        popup_stack.OpenPopupPanel(PanelId(901U), registry, layer_model, &panel_map, &first_controller);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::Success, "first popup open failed") != 0) {
        return 1;
    }

    result = popup_stack.OpenPopupPanel(PanelId(902U), registry, layer_model, &panel_map, &second_controller);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::Success, "second popup open failed") != 0) {
        return 1;
    }

    result = popup_stack.OpenPopupPanel(PanelId(903U), registry, layer_model, &panel_map, &third_controller);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::Success, "third popup open failed") != 0) {
        return 1;
    }

    if (!result.pushed || result.top_panel_id.value != 903U) {
        return Fail("third popup was not pushed to top");
    }

    if (RequirePopupOrder(popup_stack, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size()), "popup open order mismatch") != 0) {
        return 1;
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.loaded_panel_count != 3U || panel_snapshot.active_panel_count != 3U) {
        return Fail("panel map counts mismatch after popup opens");
    }

    UiManagerPanelMapRecord active_record{};
    const UiManagerPanelMapStatus active_status = panel_map.ResolveActivePanel(PanelId(903U), &active_record);
    if (active_status != UiManagerPanelMapStatus::Success) {
        return Fail("top popup active resolve failed");
    }

    if (active_record.layer_record.type != UiManagerLayerType::Popup ||
        active_record.layer_record.root_ref.root_key != 930U) {
        return Fail("top popup layer root mismatch");
    }

    std::array<UiPanelId, 2U> small_order{};
    const UiManagerPopupStackStatus small_export =
        popup_stack.ExportPopupOrder(small_order.data(), static_cast<std::uint32_t>(small_order.size()));
    return RequireStatus(small_export, UiManagerPopupStackStatus::InvalidOutputBuffer, "small popup export status mismatch");
}

int RunBringExistingPopupToTopTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 3U> panel_ids{911U, 912U, 913U};
    if (SetupPopupLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    TestPanelController first_controller;
    TestPanelController second_controller;
    TestPanelController third_controller;
    if (!popup_stack.OpenPopupPanel(PanelId(911U), registry, layer_model, &panel_map, &first_controller).Succeeded()) {
        return Fail("first popup open failed");
    }

    if (!popup_stack.OpenPopupPanel(PanelId(912U), registry, layer_model, &panel_map, &second_controller).Succeeded()) {
        return Fail("second popup open failed");
    }

    if (!popup_stack.OpenPopupPanel(PanelId(913U), registry, layer_model, &panel_map, &third_controller).Succeeded()) {
        return Fail("third popup open failed");
    }

    UiManagerPopupStackResult result = popup_stack.BringPopupToTop(PanelId(911U), panel_map);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::Success, "bring popup failed") != 0) {
        return 1;
    }

    if (!result.brought_to_top || result.top_panel_id.value != 911U) {
        return Fail("bring result did not report top change");
    }

    const std::array<std::uint32_t, 3U> expected_order{912U, 913U, 911U};
    if (RequirePopupOrder(popup_stack, std::span<const std::uint32_t>(expected_order.data(), expected_order.size()), "bring order mismatch") != 0) {
        return 1;
    }

    result = popup_stack.BringPopupToTop(PanelId(911U), panel_map);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::Success, "idempotent bring failed") != 0) {
        return 1;
    }

    if (!result.already_top || result.brought_to_top) {
        return Fail("idempotent bring flags mismatch");
    }

    const UiManagerPopupStackSnapshot snapshot = popup_stack.Snapshot();
    if (snapshot.bring_to_top_operation_count != 1U ||
        snapshot.idempotent_bring_to_top_count != 1U) {
        return Fail("bring counters mismatch");
    }

    return 0;
}

int RunCloseRemovesPopupFromStackTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 3U> panel_ids{921U, 922U, 923U};
    if (SetupPopupLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    TestPanelController first_controller;
    TestPanelController second_controller;
    TestPanelController third_controller;
    if (!popup_stack.OpenPopupPanel(PanelId(921U), registry, layer_model, &panel_map, &first_controller).Succeeded()) {
        return Fail("first popup open failed");
    }

    if (!popup_stack.OpenPopupPanel(PanelId(922U), registry, layer_model, &panel_map, &second_controller).Succeeded()) {
        return Fail("second popup open failed");
    }

    if (!popup_stack.OpenPopupPanel(PanelId(923U), registry, layer_model, &panel_map, &third_controller).Succeeded()) {
        return Fail("third popup open failed");
    }

    UiManagerPopupStackResult result = popup_stack.ClosePopupPanel(PanelId(922U), &panel_map);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::Success, "popup close failed") != 0) {
        return 1;
    }

    if (!result.removed_from_stack || result.popup_count != 2U) {
        return Fail("close result did not remove stack item");
    }

    const std::array<std::uint32_t, 2U> expected_order{921U, 923U};
    if (RequirePopupOrder(popup_stack, std::span<const std::uint32_t>(expected_order.data(), expected_order.size()), "close order mismatch") != 0) {
        return 1;
    }

    UiManagerPanelMapRecord loaded_record{};
    UiManagerPanelMapStatus map_status = panel_map.ResolveLoadedPanel(PanelId(922U), &loaded_record);
    if (map_status != UiManagerPanelMapStatus::Success) {
        return Fail("closed popup loaded resolve failed");
    }

    if (!loaded_record.loaded || loaded_record.active) {
        return Fail("closed popup did not remain loaded inactive");
    }

    map_status = panel_map.ResolveActivePanel(PanelId(922U), &loaded_record);
    if (map_status != UiManagerPanelMapStatus::PanelNotActive) {
        return Fail("closed popup active status mismatch");
    }

    result = popup_stack.ClosePopupPanel(PanelId(922U), &panel_map);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::Success, "duplicate popup close failed") != 0) {
        return 1;
    }

    if (!result.already_inactive || result.removed_from_stack) {
        return Fail("duplicate close flags mismatch");
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.loaded_panel_count != 3U || panel_snapshot.active_panel_count != 2U) {
        return Fail("panel map counts mismatch after popup close");
    }

    return 0;
}

int RunDuplicateOpenMovesExistingPopupTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 2U> panel_ids{931U, 932U};
    if (SetupPopupLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    TestPanelController first_controller;
    TestPanelController second_controller;
    TestPanelController unused_controller;
    if (!popup_stack.OpenPopupPanel(PanelId(931U), registry, layer_model, &panel_map, &first_controller).Succeeded()) {
        return Fail("first popup open failed");
    }

    if (!popup_stack.OpenPopupPanel(PanelId(932U), registry, layer_model, &panel_map, &second_controller).Succeeded()) {
        return Fail("second popup open failed");
    }

    UiManagerPopupStackResult result =
        popup_stack.OpenPopupPanel(PanelId(931U), registry, layer_model, &panel_map, &unused_controller);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::Success, "duplicate popup open failed") != 0) {
        return 1;
    }

    if (!result.already_in_stack || !result.brought_to_top || result.pushed) {
        return Fail("duplicate popup open flags mismatch");
    }

    const std::array<std::uint32_t, 2U> expected_order{932U, 931U};
    if (RequirePopupOrder(popup_stack, std::span<const std::uint32_t>(expected_order.data(), expected_order.size()), "duplicate open order mismatch") != 0) {
        return 1;
    }

    const BaseUiLifecycleSnapshot unused_snapshot = unused_controller.Snapshot();
    if (unused_snapshot.open_count != 0U) {
        return Fail("duplicate open touched unused controller");
    }

    UiManagerPanelMapRecord active_record{};
    const UiManagerPanelMapStatus active_status = panel_map.ResolveActivePanel(PanelId(931U), &active_record);
    if (active_status != UiManagerPanelMapStatus::Success) {
        return Fail("duplicate popup active resolve failed");
    }

    if (active_record.controller != &first_controller) {
        return Fail("duplicate open replaced controller");
    }

    const UiManagerPopupStackSnapshot snapshot = popup_stack.Snapshot();
    if (snapshot.idempotent_open_count != 1U || snapshot.bring_to_top_operation_count != 1U) {
        return Fail("duplicate open counters mismatch");
    }

    return 0;
}

int RunRejectsMissingNonPopupAndUntrackedStatusTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (RegisterLayer(&layer_model, 40U, UiManagerLayerType::Popup, 30, 940U) != 0) {
        return 1;
    }

    if (RegisterLayer(&layer_model, 41U, UiManagerLayerType::Fullscreen, 20, 941U) != 0) {
        return 1;
    }

    if (RegisterPanelAndBind(&registry, &layer_model, 941U, 40U) != 0) {
        return 1;
    }

    if (RegisterPanelAndBind(&registry, &layer_model, 942U, 41U) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    TestPanelController popup_controller;
    TestPanelController fullscreen_controller;
    UiManagerPopupStackResult result =
        popup_stack.OpenPopupPanel(PanelId(999U), registry, layer_model, &panel_map, &popup_controller);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::PanelNotRegistered, "missing popup status mismatch") != 0) {
        return 1;
    }

    result = popup_stack.OpenPopupPanel(PanelId(942U), registry, layer_model, &panel_map, &fullscreen_controller);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::NonPopupPanel, "non-popup open status mismatch") != 0) {
        return 1;
    }

    if (panel_map.IsLoaded(PanelId(942U))) {
        return Fail("non-popup open mutated panel map");
    }

    const UiManagerPanelMapResult direct_open =
        panel_map.OpenPanel(PanelId(941U), registry, layer_model, &popup_controller);
    if (!direct_open.Succeeded()) {
        return Fail("direct popup open failed");
    }

    result = popup_stack.BringPopupToTop(PanelId(941U), panel_map);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::PopupNotInStack, "untracked bring status mismatch") != 0) {
        return 1;
    }

    result = popup_stack.ClosePopupPanel(PanelId(941U), &panel_map);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::PopupNotInStack, "untracked close status mismatch") != 0) {
        return 1;
    }

    if (!panel_map.ClosePanel(PanelId(941U)).Succeeded()) {
        return Fail("direct popup close failed");
    }

    result = popup_stack.ClosePopupPanel(PanelId(941U), &panel_map);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::Success, "duplicate inactive close status mismatch") != 0) {
        return 1;
    }

    if (!result.already_inactive || result.removed_from_stack) {
        return Fail("inactive untracked close flags mismatch");
    }

    result = popup_stack.ClosePopupPanel(PanelId(999U), &panel_map);
    return RequireStatus(result.status, UiManagerPopupStackStatus::PanelNotLoaded, "missing close status mismatch");
}

int RunReleaseRemovesPopupAndPanelCacheTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 2U> panel_ids{951U, 952U};
    if (SetupPopupLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    TestPanelController first_controller;
    TestPanelController second_controller;
    if (!popup_stack.OpenPopupPanel(PanelId(951U), registry, layer_model, &panel_map, &first_controller).Succeeded()) {
        return Fail("first popup open failed");
    }

    if (!popup_stack.OpenPopupPanel(PanelId(952U), registry, layer_model, &panel_map, &second_controller).Succeeded()) {
        return Fail("second popup open failed");
    }

    UiManagerPopupStackResult result = popup_stack.ReleasePopupPanel(PanelId(951U), &panel_map);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::Success, "first popup release failed") != 0) {
        return 1;
    }

    if (!result.already_in_stack || !result.removed_from_stack || result.popup_count != 1U) {
        return Fail("first popup release flags mismatch");
    }

    const std::array<std::uint32_t, 1U> expected_order{952U};
    if (RequirePopupOrder(popup_stack, std::span<const std::uint32_t>(expected_order.data(), expected_order.size()), "release popup order mismatch") != 0) {
        return 1;
    }

    UiManagerPanelMapRecord loaded_record{};
    UiManagerPanelMapStatus map_status = panel_map.ResolveLoadedPanel(PanelId(951U), &loaded_record);
    if (map_status != UiManagerPanelMapStatus::PanelNotLoaded) {
        return Fail("released popup remained loaded");
    }

    if (!panel_map.IsActive(PanelId(952U)) || !popup_stack.IsPopupStacked(PanelId(952U))) {
        return Fail("release changed remaining popup");
    }

    const BaseUiLifecycleSnapshot first_snapshot = first_controller.Snapshot();
    if (first_snapshot.close_count != 1U ||
        first_snapshot.clear_count != 1U ||
        !first_snapshot.destroyed) {
        return Fail("released popup lifecycle mismatch");
    }

    result = popup_stack.ReleasePopupPanel(PanelId(951U), &panel_map);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::PanelNotLoaded, "duplicate popup release status mismatch") != 0) {
        return 1;
    }

    result = popup_stack.ReleasePopupPanel(PanelId(952U), &panel_map);
    if (RequireStatus(result.status, UiManagerPopupStackStatus::Success, "second popup release failed") != 0) {
        return 1;
    }

    if (result.popup_count != 0U || popup_stack.Snapshot().top_panel_id.IsValid()) {
        return Fail("second popup release left stack state");
    }

    const UiManagerPopupStackSnapshot snapshot = popup_stack.Snapshot();
    if (snapshot.release_operation_count != 2U ||
        snapshot.rejected_operation_count != 1U) {
        return Fail("popup release counters mismatch");
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.loaded_panel_count != 0U ||
        panel_snapshot.active_panel_count != 0U ||
        panel_snapshot.release_operation_count != 2U) {
        return Fail("popup release panel map counters mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view test_name) {
    if (test_name == TEST_OPEN_ORDER) {
        return RunOpenPushesPopupOrderTest();
    }

    if (test_name == TEST_BRING_TO_TOP) {
        return RunBringExistingPopupToTopTest();
    }

    if (test_name == TEST_CLOSE_REMOVES) {
        return RunCloseRemovesPopupFromStackTest();
    }

    if (test_name == TEST_DUPLICATE_OPEN) {
        return RunDuplicateOpenMovesExistingPopupTest();
    }

    if (test_name == TEST_REJECTS_STATUS) {
        return RunRejectsMissingNonPopupAndUntrackedStatusTest();
    }

    if (test_name == TEST_RELEASE_REMOVES) {
        return RunReleaseRemovesPopupAndPanelCacheTest();
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
