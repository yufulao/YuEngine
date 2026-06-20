// 模块: Tests UiRuntime
// 文件: Tests/UiRuntime/UiSimplePopupToastRepresentativeTests.cpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/UiRuntime/BaseUiLifecycleSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerLayerId.h"
#include "YuEngine/UiRuntime/UiManagerLayerModel.h"
#include "YuEngine/UiRuntime/UiManagerLayerRecord.h"
#include "YuEngine/UiRuntime/UiManagerLayerType.h"
#include "YuEngine/UiRuntime/UiManagerPanelLayerBinding.h"
#include "YuEngine/UiRuntime/UiManagerPanelMap.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapRecord.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapStatus.h"
#include "YuEngine/UiRuntime/UiManagerPopupStack.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackConstants.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackResult.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackStatus.h"
#include "YuEngine/UiRuntime/UiPanelId.h"
#include "YuEngine/UiRuntime/UiPanelManifestRecord.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgs.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgsSnapshot.h"
#include "YuEngine/UiRuntime/UiPanelRegistry.h"
#include "YuEngine/UiRuntime/UiPanelRegistryResult.h"
#include "YuEngine/UiRuntime/UiSimplePopupToastRepresentativeController.h"
#include "YuEngine/UiRuntime/UiSimplePopupToastRepresentativeSnapshot.h"

using yuengine::uiruntime::BaseUiLifecycleSnapshot;
using yuengine::uiruntime::MAX_UI_MANAGER_POPUP_STACK_COUNT;
using yuengine::uiruntime::UiManagerLayerId;
using yuengine::uiruntime::UiManagerLayerModel;
using yuengine::uiruntime::UiManagerLayerRecord;
using yuengine::uiruntime::UiManagerLayerRootRef;
using yuengine::uiruntime::UiManagerLayerType;
using yuengine::uiruntime::UiManagerPanelLayerBinding;
using yuengine::uiruntime::UiManagerPanelMap;
using yuengine::uiruntime::UiManagerPanelMapRecord;
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
using yuengine::uiruntime::UiPanelOpenArgs;
using yuengine::uiruntime::UiPanelOpenArgsSnapshot;
using yuengine::uiruntime::UiPanelRegistry;
using yuengine::uiruntime::UiPanelRegistryResult;
using yuengine::uiruntime::UiPanelResourceRef;
using yuengine::uiruntime::UiSimplePopupToastRepresentativeController;
using yuengine::uiruntime::UiSimplePopupToastRepresentativeSnapshot;

namespace {
constexpr const char *TEST_OPEN_CLOSE =
    "UiRuntime_SimplePopupToastRepresentative_OpenDisplaysAndCloseClears";
constexpr const char *TEST_REOPEN_CACHE =
    "UiRuntime_SimplePopupToastRepresentative_ReopenBeforeReleaseUsesCache";
constexpr const char *TEST_RELEASE_CLEAR =
    "UiRuntime_SimplePopupToastRepresentative_ReleaseClearsLoadedCache";
constexpr const char *TEST_FRESH_REOPEN =
    "UiRuntime_SimplePopupToastRepresentative_ReleaseThenReopenUsesFreshLifecycle";
constexpr const char *TEST_STATUS_ORDER =
    "UiRuntime_SimplePopupToastRepresentative_StatusAndOrderRemainExplicit";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

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
    record.resource_refs[0U] = ResourceRef(resource_key, 900U);
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

UiPanelOpenArgs OpenArgs(std::uint32_t request_key, std::span<const std::uint32_t> values) {
    UiPanelOpenArgs args{};
    args.request_key = request_key;
    args.values = values.data();
    args.value_count = static_cast<std::uint32_t>(values.size());
    return args;
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

    const std::uint32_t layout_key = 100U + panel_id;
    const std::uint32_t controller_key = 200U + panel_id;
    const std::uint32_t resource_key = 300U + panel_id;
    const UiPanelManifestRecord panel_record = PanelRecord(panel_id, layout_key, controller_key, resource_key);
    const UiPanelRegistryResult panel_result = registry->RegisterPanel(panel_record);
    if (!panel_result.Succeeded()) {
        return Fail("panel register failed");
    }

    if (!layer_model->BindPanelToLayer(PanelBinding(panel_id, layer_id)).Succeeded()) {
        return Fail("panel layer binding failed");
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

    if (RegisterLayer(layer_model, 51U, UiManagerLayerType::Popup, 50, 951U) != 0) {
        return 1;
    }

    for (std::uint32_t panel_id : panel_ids) {
        if (RegisterPanelAndBind(registry, layer_model, panel_id, 51U) != 0) {
            return 1;
        }
    }

    return 0;
}

int RequirePopupStatus(
    UiManagerPopupStackStatus actual,
    UiManagerPopupStackStatus expected,
    std::string_view message) {
    if (actual == expected) {
        return 0;
    }

    return Fail(message);
}

int RequirePanelStatus(
    UiManagerPanelMapStatus actual,
    UiManagerPanelMapStatus expected,
    std::string_view message) {
    if (actual == expected) {
        return 0;
    }

    return Fail(message);
}

int RequireArgsSnapshot(
    const UiPanelOpenArgsSnapshot &snapshot,
    std::uint32_t request_key,
    std::span<const std::uint32_t> values,
    std::string_view message) {
    const std::uint32_t expected_value_count = static_cast<std::uint32_t>(values.size());
    const bool expected_has_args = request_key != 0U || expected_value_count > 0U;
    if (snapshot.request_key != request_key ||
        snapshot.value_count != expected_value_count ||
        snapshot.has_args != expected_has_args) {
        return Fail(message);
    }

    for (std::uint32_t index = 0U; index < expected_value_count; ++index) {
        const std::uint32_t expected_value = values[static_cast<std::size_t>(index)];
        if (snapshot.values[index] != expected_value) {
            return Fail(message);
        }
    }

    return 0;
}

int RequireEmptyArgsSnapshot(const UiPanelOpenArgsSnapshot &snapshot, std::string_view message) {
    if (snapshot.request_key != 0U || snapshot.value_count != 0U || snapshot.has_args) {
        return Fail(message);
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

int RequireDisplayedSnapshot(
    const UiSimplePopupToastRepresentativeSnapshot &snapshot,
    std::uint32_t request_key,
    std::span<const std::uint32_t> values,
    std::uint32_t expected_open_count,
    std::string_view message) {
    if (!snapshot.visible || !snapshot.displayed || snapshot.cleared) {
        return Fail(message);
    }

    if (snapshot.open_display_count != expected_open_count) {
        return Fail(message);
    }

    if (snapshot.message_key != request_key) {
        return Fail(message);
    }

    const std::uint32_t expected_style_key = values.empty() ? 0U : values[0U];
    if (snapshot.style_key != expected_style_key) {
        return Fail(message);
    }

    return RequireArgsSnapshot(snapshot.open_args, request_key, values, message);
}

int RequireClearedRepresentative(
    const UiSimplePopupToastRepresentativeController &controller,
    std::uint32_t expected_open_count,
    std::uint32_t expected_close_count,
    std::uint32_t expected_clear_count,
    std::string_view message) {
    const UiSimplePopupToastRepresentativeSnapshot snapshot = controller.GetRepresentativeSnapshot();
    if (snapshot.visible || snapshot.displayed || !snapshot.cleared) {
        return Fail(message);
    }

    if (snapshot.message_key != 0U || snapshot.style_key != 0U) {
        return Fail(message);
    }

    if (snapshot.open_display_count != expected_open_count ||
        snapshot.close_display_count != expected_close_count ||
        snapshot.clear_display_count != expected_clear_count) {
        return Fail(message);
    }

    return RequireEmptyArgsSnapshot(snapshot.open_args, message);
}

int RunOpenDisplaysAndCloseClearsTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 1U> panel_ids{2101U};
    if (SetupPopupLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    UiSimplePopupToastRepresentativeController controller;
    std::array<std::uint32_t, 2U> values{17U, 23U};
    const std::span<const std::uint32_t> value_span(values.data(), values.size());
    const UiPanelOpenArgs open_args = OpenArgs(8101U, value_span);
    const UiManagerPopupStackResult open_result =
        popup_stack.OpenPopupPanelWithArgs(PanelId(2101U), registry, layer_model, &panel_map, &controller, open_args);
    if (RequirePopupStatus(open_result.status, UiManagerPopupStackStatus::Success, "popup open failed") != 0) {
        return 1;
    }

    if (!open_result.pushed || open_result.top_panel_id.value != 2101U) {
        return Fail("popup open flags mismatch");
    }

    if (RequireDisplayedSnapshot(controller.GetRepresentativeSnapshot(), 8101U, value_span, 1U, "display snapshot mismatch") != 0) {
        return 1;
    }

    UiManagerPanelMapRecord active_record{};
    const UiManagerPanelMapStatus active_status = panel_map.ResolveActivePanel(PanelId(2101U), &active_record);
    if (RequirePanelStatus(active_status, UiManagerPanelMapStatus::Success, "active resolve failed") != 0) {
        return 1;
    }

    if (active_record.controller != &controller) {
        return Fail("active controller mismatch");
    }

    UiManagerPopupStackResult close_result = popup_stack.ClosePopupPanel(PanelId(2101U), &panel_map);
    if (RequirePopupStatus(close_result.status, UiManagerPopupStackStatus::Success, "popup close failed") != 0) {
        return 1;
    }

    if (!close_result.removed_from_stack || close_result.popup_count != 0U) {
        return Fail("popup close stack flags mismatch");
    }

    const UiSimplePopupToastRepresentativeSnapshot close_snapshot = controller.GetRepresentativeSnapshot();
    if (close_snapshot.visible || close_snapshot.displayed || close_snapshot.cleared) {
        return Fail("close did not clear display state");
    }

    if (close_snapshot.open_display_count != 1U ||
        close_snapshot.close_display_count != 1U ||
        close_snapshot.clear_display_count != 0U) {
        return Fail("close display counters mismatch");
    }

    if (RequireArgsSnapshot(close_snapshot.open_args, 8101U, value_span, "close retained args mismatch") != 0) {
        return 1;
    }

    UiManagerPanelMapRecord loaded_record{};
    const UiManagerPanelMapStatus loaded_status = panel_map.ResolveLoadedPanel(PanelId(2101U), &loaded_record);
    if (RequirePanelStatus(loaded_status, UiManagerPanelMapStatus::Success, "loaded resolve after close failed") != 0) {
        return 1;
    }

    if (!loaded_record.loaded || loaded_record.active) {
        return Fail("closed popup cache state mismatch");
    }

    return RequirePopupOrder(popup_stack, std::span<const std::uint32_t>(), "close popup order mismatch");
}

int RunReopenBeforeReleaseUsesCacheTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 1U> panel_ids{2201U};
    if (SetupPopupLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    UiSimplePopupToastRepresentativeController controller;
    std::array<std::uint32_t, 1U> first_values{31U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = OpenArgs(8201U, first_span);
    if (!popup_stack.OpenPopupPanelWithArgs(PanelId(2201U), registry, layer_model, &panel_map, &controller, first_args).Succeeded()) {
        return Fail("first popup open failed");
    }

    if (!popup_stack.ClosePopupPanel(PanelId(2201U), &panel_map).Succeeded()) {
        return Fail("close before cached reopen failed");
    }

    std::array<std::uint32_t, 2U> second_values{37U, 41U};
    const std::span<const std::uint32_t> second_span(second_values.data(), second_values.size());
    const UiPanelOpenArgs second_args = OpenArgs(8202U, second_span);
    UiManagerPopupStackResult reopen_result =
        popup_stack.OpenPopupPanelWithArgs(PanelId(2201U), registry, layer_model, &panel_map, nullptr, second_args);
    if (RequirePopupStatus(reopen_result.status, UiManagerPopupStackStatus::Success, "cached reopen failed") != 0) {
        return 1;
    }

    if (!reopen_result.pushed || reopen_result.record.controller != &controller) {
        return Fail("cached reopen did not reuse controller");
    }

    if (RequireDisplayedSnapshot(controller.GetRepresentativeSnapshot(), 8202U, second_span, 2U, "cached reopen display mismatch") != 0) {
        return 1;
    }

    if (controller.GetRepresentativeSnapshot().close_display_count != 1U) {
        return Fail("cached reopen close count mismatch");
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.reused_loaded_count != 1U ||
        panel_snapshot.reopen_open_args_update_count != 1U ||
        panel_snapshot.loaded_panel_count != 1U ||
        panel_snapshot.active_panel_count != 1U) {
        return Fail("cached reopen panel counters mismatch");
    }

    return RequirePopupOrder(popup_stack, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size()), "cached reopen order mismatch");
}

int RunReleaseClearsLoadedCacheTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 1U> panel_ids{2301U};
    if (SetupPopupLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    UiSimplePopupToastRepresentativeController controller;
    std::array<std::uint32_t, 1U> values{43U};
    const std::span<const std::uint32_t> value_span(values.data(), values.size());
    const UiPanelOpenArgs open_args = OpenArgs(8301U, value_span);
    if (!popup_stack.OpenPopupPanelWithArgs(PanelId(2301U), registry, layer_model, &panel_map, &controller, open_args).Succeeded()) {
        return Fail("popup open before release failed");
    }

    const UiManagerPopupStackResult release_result = popup_stack.ReleasePopupPanel(PanelId(2301U), &panel_map);
    if (RequirePopupStatus(release_result.status, UiManagerPopupStackStatus::Success, "popup release failed") != 0) {
        return 1;
    }

    if (!release_result.already_in_stack || !release_result.removed_from_stack || release_result.popup_count != 0U) {
        return Fail("release stack flags mismatch");
    }

    if (release_result.record.loaded ||
        release_result.record.active ||
        release_result.record.controller != nullptr) {
        return Fail("release result did not clear record");
    }

    if (RequireEmptyArgsSnapshot(release_result.record.open_args, "release result args snapshot mismatch") != 0) {
        return 1;
    }

    if (RequireClearedRepresentative(controller, 1U, 1U, 1U, "released controller snapshot mismatch") != 0) {
        return 1;
    }

    const BaseUiLifecycleSnapshot lifecycle_snapshot = controller.Snapshot();
    if (!lifecycle_snapshot.destroyed ||
        lifecycle_snapshot.close_count != 1U ||
        lifecycle_snapshot.clear_count != 1U) {
        return Fail("released controller lifecycle mismatch");
    }

    UiManagerPanelMapRecord loaded_record{};
    const UiManagerPanelMapStatus loaded_status = panel_map.ResolveLoadedPanel(PanelId(2301U), &loaded_record);
    if (RequirePanelStatus(loaded_status, UiManagerPanelMapStatus::PanelNotLoaded, "release did not clear panel map cache") != 0) {
        return 1;
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.loaded_panel_count != 0U ||
        panel_snapshot.active_panel_count != 0U ||
        panel_snapshot.release_operation_count != 1U) {
        return Fail("release panel counters mismatch");
    }

    const UiManagerPopupStackResult duplicate_release = popup_stack.ReleasePopupPanel(PanelId(2301U), &panel_map);
    return RequirePopupStatus(duplicate_release.status, UiManagerPopupStackStatus::PanelNotLoaded, "duplicate release status mismatch");
}

int RunReleaseThenReopenUsesFreshLifecycleTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 1U> panel_ids{2401U};
    if (SetupPopupLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    UiSimplePopupToastRepresentativeController first_controller;
    std::array<std::uint32_t, 1U> first_values{47U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = OpenArgs(8401U, first_span);
    if (!popup_stack.OpenPopupPanelWithArgs(PanelId(2401U), registry, layer_model, &panel_map, &first_controller, first_args).Succeeded()) {
        return Fail("first popup open failed");
    }

    if (!popup_stack.ReleasePopupPanel(PanelId(2401U), &panel_map).Succeeded()) {
        return Fail("first popup release failed");
    }

    UiSimplePopupToastRepresentativeController second_controller;
    std::array<std::uint32_t, 2U> second_values{53U, 59U};
    const std::span<const std::uint32_t> second_span(second_values.data(), second_values.size());
    const UiPanelOpenArgs second_args = OpenArgs(8402U, second_span);
    const UiManagerPopupStackResult reopen_result =
        popup_stack.OpenPopupPanelWithArgs(PanelId(2401U), registry, layer_model, &panel_map, &second_controller, second_args);
    if (RequirePopupStatus(reopen_result.status, UiManagerPopupStackStatus::Success, "fresh reopen failed") != 0) {
        return 1;
    }

    if (reopen_result.record.controller != &second_controller || reopen_result.already_in_stack) {
        return Fail("fresh reopen controller flags mismatch");
    }

    if (RequireClearedRepresentative(first_controller, 1U, 1U, 1U, "first controller not cleared after release") != 0) {
        return 1;
    }

    if (RequireDisplayedSnapshot(second_controller.GetRepresentativeSnapshot(), 8402U, second_span, 1U, "fresh controller display mismatch") != 0) {
        return 1;
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.loaded_panel_count != 1U ||
        panel_snapshot.active_panel_count != 1U ||
        panel_snapshot.release_operation_count != 1U ||
        panel_snapshot.reused_loaded_count != 0U) {
        return Fail("fresh reopen panel counters mismatch");
    }

    return RequirePopupOrder(popup_stack, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size()), "fresh reopen order mismatch");
}

int RunStatusAndOrderRemainExplicitTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 2U> panel_ids{2501U, 2502U};
    if (SetupPopupLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    UiSimplePopupToastRepresentativeController first_controller;
    UiSimplePopupToastRepresentativeController second_controller;
    std::array<std::uint32_t, 1U> first_values{61U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = OpenArgs(8501U, first_span);
    if (!popup_stack.OpenPopupPanelWithArgs(PanelId(2501U), registry, layer_model, &panel_map, &first_controller, first_args).Succeeded()) {
        return Fail("first order popup open failed");
    }

    std::array<std::uint32_t, 1U> second_values{67U};
    const std::span<const std::uint32_t> second_span(second_values.data(), second_values.size());
    const UiPanelOpenArgs second_args = OpenArgs(8502U, second_span);
    if (!popup_stack.OpenPopupPanelWithArgs(PanelId(2502U), registry, layer_model, &panel_map, &second_controller, second_args).Succeeded()) {
        return Fail("second order popup open failed");
    }

    const std::array<std::uint32_t, 2U> initial_order{2501U, 2502U};
    if (RequirePopupOrder(popup_stack, std::span<const std::uint32_t>(initial_order.data(), initial_order.size()), "initial popup order mismatch") != 0) {
        return 1;
    }

    UiSimplePopupToastRepresentativeController unused_controller;
    std::array<std::uint32_t, 1U> duplicate_values{71U};
    const std::span<const std::uint32_t> duplicate_span(duplicate_values.data(), duplicate_values.size());
    const UiPanelOpenArgs duplicate_args = OpenArgs(8503U, duplicate_span);
    UiManagerPopupStackResult duplicate_result =
        popup_stack.OpenPopupPanelWithArgs(PanelId(2501U), registry, layer_model, &panel_map, &unused_controller, duplicate_args);
    if (RequirePopupStatus(duplicate_result.status, UiManagerPopupStackStatus::Success, "duplicate popup status mismatch") != 0) {
        return 1;
    }

    if (!duplicate_result.already_in_stack || !duplicate_result.brought_to_top || duplicate_result.pushed) {
        return Fail("duplicate popup flags mismatch");
    }

    const std::array<std::uint32_t, 2U> duplicate_order{2502U, 2501U};
    if (RequirePopupOrder(popup_stack, std::span<const std::uint32_t>(duplicate_order.data(), duplicate_order.size()), "duplicate popup order mismatch") != 0) {
        return 1;
    }

    if (unused_controller.GetRepresentativeSnapshot().open_display_count != 0U) {
        return Fail("duplicate popup touched unused controller");
    }

    if (RequireDisplayedSnapshot(first_controller.GetRepresentativeSnapshot(), 8501U, first_span, 1U, "duplicate popup mutated cached display") != 0) {
        return 1;
    }

    UiSimplePopupToastRepresentativeController missing_controller;
    duplicate_result =
        popup_stack.OpenPopupPanelWithArgs(PanelId(2599U), registry, layer_model, &panel_map, &missing_controller, first_args);
    if (RequirePopupStatus(duplicate_result.status, UiManagerPopupStackStatus::PanelNotRegistered, "missing popup status mismatch") != 0) {
        return 1;
    }

    if (missing_controller.GetRepresentativeSnapshot().open_display_count != 0U) {
        return Fail("missing popup touched controller");
    }

    const UiManagerPopupStackSnapshot snapshot = popup_stack.Snapshot();
    if (snapshot.popup_count != 2U ||
        snapshot.rejected_operation_count != 1U ||
        snapshot.idempotent_open_count != 1U ||
        snapshot.bring_to_top_operation_count != 1U) {
        return Fail("status order popup counters mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view test_name) {
    if (test_name == TEST_OPEN_CLOSE) {
        return RunOpenDisplaysAndCloseClearsTest();
    }

    if (test_name == TEST_REOPEN_CACHE) {
        return RunReopenBeforeReleaseUsesCacheTest();
    }

    if (test_name == TEST_RELEASE_CLEAR) {
        return RunReleaseClearsLoadedCacheTest();
    }

    if (test_name == TEST_FRESH_REOPEN) {
        return RunReleaseThenReopenUsesFreshLifecycleTest();
    }

    if (test_name == TEST_STATUS_ORDER) {
        return RunStatusAndOrderRemainExplicitTest();
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
