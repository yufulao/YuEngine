// 模块: Tests UiRuntime
// 文件: Tests/UiRuntime/UiFullscreenRepresentativeTests.cpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/UiRuntime/BaseUiLifecycleSnapshot.h"
#include "YuEngine/UiRuntime/UiFullscreenRepresentativeController.h"
#include "YuEngine/UiRuntime/UiFullscreenRepresentativeSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStack.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStackConstants.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStackResult.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStackSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStackStatus.h"
#include "YuEngine/UiRuntime/UiManagerLayerId.h"
#include "YuEngine/UiRuntime/UiManagerLayerModel.h"
#include "YuEngine/UiRuntime/UiManagerLayerRecord.h"
#include "YuEngine/UiRuntime/UiManagerLayerType.h"
#include "YuEngine/UiRuntime/UiManagerPanelLayerBinding.h"
#include "YuEngine/UiRuntime/UiManagerPanelMap.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapRecord.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapStatus.h"
#include "YuEngine/UiRuntime/UiPanelId.h"
#include "YuEngine/UiRuntime/UiPanelManifestRecord.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgs.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgsSnapshot.h"
#include "YuEngine/UiRuntime/UiPanelRegistry.h"
#include "YuEngine/UiRuntime/UiPanelRegistryResult.h"

using yuengine::uiruntime::BaseUiLifecycleSnapshot;
using yuengine::uiruntime::MAX_UI_MANAGER_FULLSCREEN_STACK_COUNT;
using yuengine::uiruntime::UiFullscreenRepresentativeController;
using yuengine::uiruntime::UiFullscreenRepresentativeSnapshot;
using yuengine::uiruntime::UiManagerFullscreenStack;
using yuengine::uiruntime::UiManagerFullscreenStackResult;
using yuengine::uiruntime::UiManagerFullscreenStackSnapshot;
using yuengine::uiruntime::UiManagerFullscreenStackStatus;
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
using yuengine::uiruntime::UiPanelControllerRef;
using yuengine::uiruntime::UiPanelId;
using yuengine::uiruntime::UiPanelLayoutRef;
using yuengine::uiruntime::UiPanelManifestRecord;
using yuengine::uiruntime::UiPanelOpenArgs;
using yuengine::uiruntime::UiPanelOpenArgsSnapshot;
using yuengine::uiruntime::UiPanelRegistry;
using yuengine::uiruntime::UiPanelRegistryResult;
using yuengine::uiruntime::UiPanelResourceRef;

namespace {
constexpr const char *TEST_OPEN_TOP =
    "UiRuntime_FullscreenRepresentative_OpenDisplaysAndTracksTopActive";
constexpr const char *TEST_BACK_RESTORE =
    "UiRuntime_FullscreenRepresentative_OpenSecondAndNavigateBackRestoresPrevious";
constexpr const char *TEST_RELEASE_CLEAR =
    "UiRuntime_FullscreenRepresentative_CloseAndReleaseClearSnapshots";
constexpr const char *TEST_CACHE_FRESH =
    "UiRuntime_FullscreenRepresentative_ReopenCacheAndReleaseFreshLifecycle";
constexpr const char *TEST_STATUS_DUPLICATE =
    "UiRuntime_FullscreenRepresentative_StatusAndDuplicateRemainExplicit";
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
    record.resource_refs[0U] = ResourceRef(resource_key, 910U);
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

int SetupFullscreenLayer(
    UiPanelRegistry *registry,
    UiManagerLayerModel *layer_model,
    std::span<const std::uint32_t> panel_ids) {
    if (registry == nullptr || layer_model == nullptr) {
        return Fail("missing fullscreen setup model");
    }

    if (RegisterLayer(layer_model, 81U, UiManagerLayerType::Fullscreen, 20, 981U) != 0) {
        return 1;
    }

    for (std::uint32_t panel_id : panel_ids) {
        if (RegisterPanelAndBind(registry, layer_model, panel_id, 81U) != 0) {
            return 1;
        }
    }

    return 0;
}

int RequireFullscreenStatus(
    UiManagerFullscreenStackStatus actual,
    UiManagerFullscreenStackStatus expected,
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

int RequireFullscreenOrder(
    const UiManagerFullscreenStack &fullscreen_stack,
    std::span<const std::uint32_t> expected_order,
    std::string_view message) {
    std::array<UiPanelId, MAX_UI_MANAGER_FULLSCREEN_STACK_COUNT> output_order{};
    const std::uint32_t expected_count = static_cast<std::uint32_t>(expected_order.size());
    const UiManagerFullscreenStackStatus export_status =
        fullscreen_stack.ExportFullscreenOrder(output_order.data(), expected_count);
    if (export_status != UiManagerFullscreenStackStatus::Success) {
        return Fail(message);
    }

    const UiManagerFullscreenStackSnapshot snapshot = fullscreen_stack.Snapshot();
    if (snapshot.fullscreen_count != expected_count) {
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
    const UiFullscreenRepresentativeSnapshot &snapshot,
    std::uint32_t request_key,
    std::span<const std::uint32_t> values,
    std::uint32_t expected_open_count,
    std::string_view message) {
    if (!snapshot.visible || !snapshot.displayed || !snapshot.top_active || snapshot.cleared) {
        return Fail(message);
    }

    if (snapshot.open_display_count != expected_open_count) {
        return Fail(message);
    }

    if (snapshot.screen_key != request_key) {
        return Fail(message);
    }

    const std::uint32_t expected_display_mode_key = values.empty() ? 0U : values[0U];
    if (snapshot.display_mode_key != expected_display_mode_key) {
        return Fail(message);
    }

    const std::uint32_t expected_focus_key = values.size() < 2U ? 0U : values[1U];
    if (snapshot.focus_key != expected_focus_key) {
        return Fail(message);
    }

    return RequireArgsSnapshot(snapshot.open_args, request_key, values, message);
}

int RequireClosedSnapshot(
    const UiFullscreenRepresentativeSnapshot &snapshot,
    std::uint32_t request_key,
    std::span<const std::uint32_t> values,
    std::uint32_t expected_open_count,
    std::uint32_t expected_close_count,
    std::string_view message) {
    if (snapshot.visible || snapshot.displayed || snapshot.top_active || snapshot.cleared) {
        return Fail(message);
    }

    if (snapshot.open_display_count != expected_open_count ||
        snapshot.close_display_count != expected_close_count) {
        return Fail(message);
    }

    return RequireArgsSnapshot(snapshot.open_args, request_key, values, message);
}

int RequireClearedRepresentative(
    const UiFullscreenRepresentativeController &controller,
    std::uint32_t expected_open_count,
    std::uint32_t expected_close_count,
    std::uint32_t expected_clear_count,
    std::string_view message) {
    const UiFullscreenRepresentativeSnapshot snapshot = controller.GetRepresentativeSnapshot();
    if (snapshot.visible || snapshot.displayed || snapshot.top_active || !snapshot.cleared) {
        return Fail(message);
    }

    if (snapshot.screen_key != 0U ||
        snapshot.display_mode_key != 0U ||
        snapshot.focus_key != 0U) {
        return Fail(message);
    }

    if (snapshot.open_display_count != expected_open_count ||
        snapshot.close_display_count != expected_close_count ||
        snapshot.clear_display_count != expected_clear_count) {
        return Fail(message);
    }

    return RequireEmptyArgsSnapshot(snapshot.open_args, message);
}

int RequireActiveFullscreen(
    const UiManagerPanelMap &panel_map,
    std::uint32_t panel_id,
    std::string_view message) {
    UiManagerPanelMapRecord active_record{};
    const UiManagerPanelMapStatus active_status = panel_map.ResolveActivePanel(PanelId(panel_id), &active_record);
    if (active_status != UiManagerPanelMapStatus::Success) {
        return Fail(message);
    }

    if (active_record.layer_record.type != UiManagerLayerType::Fullscreen ||
        active_record.layer_record.root_ref.root_key != 981U) {
        return Fail(message);
    }

    return 0;
}

int RunOpenDisplaysAndTracksTopActiveTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 1U> panel_ids{3101U};
    if (SetupFullscreenLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerFullscreenStack fullscreen_stack;
    UiFullscreenRepresentativeController controller;
    std::array<std::uint32_t, 2U> values{11U, 12U};
    const std::span<const std::uint32_t> value_span(values.data(), values.size());
    const UiPanelOpenArgs open_args = OpenArgs(9101U, value_span);
    const UiManagerFullscreenStackResult open_result =
        fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3101U), registry, layer_model, &panel_map, &controller, open_args);
    if (RequireFullscreenStatus(open_result.status, UiManagerFullscreenStackStatus::Success, "fullscreen open failed") != 0) {
        return 1;
    }

    if (!open_result.pushed || open_result.top_panel_id.value != 3101U) {
        return Fail("fullscreen open flags mismatch");
    }

    if (RequireDisplayedSnapshot(controller.GetRepresentativeSnapshot(), 9101U, value_span, 1U, "fullscreen display mismatch") != 0) {
        return 1;
    }

    if (RequireActiveFullscreen(panel_map, 3101U, "fullscreen active resolve failed") != 0) {
        return 1;
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.loaded_panel_count != 1U ||
        panel_snapshot.active_panel_count != 1U ||
        panel_snapshot.parameter_open_count != 1U) {
        return Fail("fullscreen panel map counters mismatch");
    }

    return RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size()), "fullscreen order mismatch");
}

int RunOpenSecondAndNavigateBackRestoresPreviousTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 2U> panel_ids{3201U, 3202U};
    if (SetupFullscreenLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerFullscreenStack fullscreen_stack;
    UiFullscreenRepresentativeController first_controller;
    UiFullscreenRepresentativeController second_controller;
    std::array<std::uint32_t, 2U> first_values{21U, 22U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = OpenArgs(9201U, first_span);
    if (!fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3201U), registry, layer_model, &panel_map, &first_controller, first_args).Succeeded()) {
        return Fail("first fullscreen open failed");
    }

    std::array<std::uint32_t, 2U> second_values{23U, 24U};
    const std::span<const std::uint32_t> second_span(second_values.data(), second_values.size());
    const UiPanelOpenArgs second_args = OpenArgs(9202U, second_span);
    if (!fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3202U), registry, layer_model, &panel_map, &second_controller, second_args).Succeeded()) {
        return Fail("second fullscreen open failed");
    }

    if (RequireClosedSnapshot(first_controller.GetRepresentativeSnapshot(), 9201U, first_span, 1U, 1U, "first close on second open mismatch") != 0) {
        return 1;
    }

    if (RequireDisplayedSnapshot(second_controller.GetRepresentativeSnapshot(), 9202U, second_span, 1U, "second fullscreen display mismatch") != 0) {
        return 1;
    }

    const std::array<std::uint32_t, 2U> expected_order{3201U, 3202U};
    if (RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(expected_order.data(), expected_order.size()), "second fullscreen order mismatch") != 0) {
        return 1;
    }

    UiManagerFullscreenStackResult back_result = fullscreen_stack.NavigateBack(registry, layer_model, &panel_map);
    if (RequireFullscreenStatus(back_result.status, UiManagerFullscreenStackStatus::Success, "navigate back failed") != 0) {
        return 1;
    }

    if (!back_result.navigated_back ||
        !back_result.restored_previous ||
        back_result.closed_panel_id.value != 3202U ||
        back_result.restored_panel_id.value != 3201U) {
        return Fail("navigate back flags mismatch");
    }

    if (RequireDisplayedSnapshot(first_controller.GetRepresentativeSnapshot(), 9201U, first_span, 2U, "restored fullscreen display mismatch") != 0) {
        return 1;
    }

    if (RequireClosedSnapshot(second_controller.GetRepresentativeSnapshot(), 9202U, second_span, 1U, 1U, "closed second fullscreen mismatch") != 0) {
        return 1;
    }

    const std::array<std::uint32_t, 1U> restored_order{3201U};
    if (RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(restored_order.data(), restored_order.size()), "restored fullscreen order mismatch") != 0) {
        return 1;
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.loaded_panel_count != 2U ||
        panel_snapshot.active_panel_count != 1U ||
        panel_snapshot.reused_loaded_count != 1U) {
        return Fail("navigate back panel counters mismatch");
    }

    return RequireActiveFullscreen(panel_map, 3201U, "navigate back active resolve mismatch");
}

int RunCloseAndReleaseClearSnapshotsTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 2U> panel_ids{3301U, 3302U};
    if (SetupFullscreenLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerFullscreenStack fullscreen_stack;
    UiFullscreenRepresentativeController first_controller;
    UiFullscreenRepresentativeController second_controller;
    std::array<std::uint32_t, 1U> first_values{31U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = OpenArgs(9301U, first_span);
    if (!fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3301U), registry, layer_model, &panel_map, &first_controller, first_args).Succeeded()) {
        return Fail("first fullscreen open failed");
    }

    std::array<std::uint32_t, 1U> second_values{32U};
    const std::span<const std::uint32_t> second_span(second_values.data(), second_values.size());
    const UiPanelOpenArgs second_args = OpenArgs(9302U, second_span);
    if (!fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3302U), registry, layer_model, &panel_map, &second_controller, second_args).Succeeded()) {
        return Fail("second fullscreen open failed");
    }

    UiManagerFullscreenStackResult close_result =
        fullscreen_stack.CloseFullscreenPanel(PanelId(3302U), registry, layer_model, &panel_map);
    if (RequireFullscreenStatus(close_result.status, UiManagerFullscreenStackStatus::Success, "fullscreen close failed") != 0) {
        return 1;
    }

    if (!close_result.closed_current || !close_result.restored_previous || close_result.restored_panel_id.value != 3301U) {
        return Fail("close restore flags mismatch");
    }

    if (RequireDisplayedSnapshot(first_controller.GetRepresentativeSnapshot(), 9301U, first_span, 2U, "close restore display mismatch") != 0) {
        return 1;
    }

    if (RequireClosedSnapshot(second_controller.GetRepresentativeSnapshot(), 9302U, second_span, 1U, 1U, "closed fullscreen display mismatch") != 0) {
        return 1;
    }

    UiManagerFullscreenStackResult release_result =
        fullscreen_stack.ReleaseFullscreenPanel(PanelId(3301U), registry, layer_model, &panel_map);
    if (RequireFullscreenStatus(release_result.status, UiManagerFullscreenStackStatus::Success, "fullscreen release failed") != 0) {
        return 1;
    }

    if (!release_result.closed_current || !release_result.removed_from_stack || release_result.fullscreen_count != 0U) {
        return Fail("release stack flags mismatch");
    }

    if (RequireClearedRepresentative(first_controller, 2U, 2U, 1U, "released fullscreen snapshot mismatch") != 0) {
        return 1;
    }

    if (release_result.record.loaded ||
        release_result.record.active ||
        release_result.record.controller != nullptr) {
        return Fail("release result did not clear record");
    }

    if (RequireEmptyArgsSnapshot(release_result.record.open_args, "release result args snapshot mismatch") != 0) {
        return 1;
    }

    UiManagerPanelMapRecord loaded_record{};
    const UiManagerPanelMapStatus loaded_status = panel_map.ResolveLoadedPanel(PanelId(3301U), &loaded_record);
    if (RequirePanelStatus(loaded_status, UiManagerPanelMapStatus::PanelNotLoaded, "release did not clear loaded map") != 0) {
        return 1;
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.loaded_panel_count != 1U ||
        panel_snapshot.active_panel_count != 0U ||
        panel_snapshot.release_operation_count != 1U) {
        return Fail("release panel counters mismatch");
    }

    return RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(), "release fullscreen order mismatch");
}

int RunReopenCacheAndReleaseFreshLifecycleTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 1U> panel_ids{3401U};
    if (SetupFullscreenLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerFullscreenStack fullscreen_stack;
    UiFullscreenRepresentativeController first_controller;
    std::array<std::uint32_t, 1U> first_values{41U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = OpenArgs(9401U, first_span);
    if (!fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3401U), registry, layer_model, &panel_map, &first_controller, first_args).Succeeded()) {
        return Fail("first fullscreen open failed");
    }

    if (!fullscreen_stack.CloseFullscreenPanel(PanelId(3401U), registry, layer_model, &panel_map).Succeeded()) {
        return Fail("close before cached fullscreen reopen failed");
    }

    std::array<std::uint32_t, 2U> second_values{42U, 43U};
    const std::span<const std::uint32_t> second_span(second_values.data(), second_values.size());
    const UiPanelOpenArgs second_args = OpenArgs(9402U, second_span);
    UiManagerFullscreenStackResult reopen_result =
        fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3401U), registry, layer_model, &panel_map, nullptr, second_args);
    if (RequireFullscreenStatus(reopen_result.status, UiManagerFullscreenStackStatus::Success, "cached fullscreen reopen failed") != 0) {
        return 1;
    }

    if (!reopen_result.pushed || reopen_result.record.controller != &first_controller) {
        return Fail("cached fullscreen reopen flags mismatch");
    }

    if (RequireDisplayedSnapshot(first_controller.GetRepresentativeSnapshot(), 9402U, second_span, 2U, "cached fullscreen display mismatch") != 0) {
        return 1;
    }

    const UiManagerPanelMapSnapshot cached_snapshot = panel_map.Snapshot();
    if (cached_snapshot.reused_loaded_count != 1U ||
        cached_snapshot.reopen_open_args_update_count != 1U ||
        cached_snapshot.loaded_panel_count != 1U ||
        cached_snapshot.active_panel_count != 1U) {
        return Fail("cached fullscreen panel counters mismatch");
    }

    if (!fullscreen_stack.ReleaseFullscreenPanel(PanelId(3401U), registry, layer_model, &panel_map).Succeeded()) {
        return Fail("release before fresh fullscreen reopen failed");
    }

    UiFullscreenRepresentativeController second_controller;
    std::array<std::uint32_t, 1U> third_values{44U};
    const std::span<const std::uint32_t> third_span(third_values.data(), third_values.size());
    const UiPanelOpenArgs third_args = OpenArgs(9403U, third_span);
    reopen_result =
        fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3401U), registry, layer_model, &panel_map, &second_controller, third_args);
    if (RequireFullscreenStatus(reopen_result.status, UiManagerFullscreenStackStatus::Success, "fresh fullscreen reopen failed") != 0) {
        return 1;
    }

    if (reopen_result.record.controller != &second_controller || reopen_result.already_in_stack) {
        return Fail("fresh fullscreen reopen flags mismatch");
    }

    if (RequireClearedRepresentative(first_controller, 2U, 2U, 1U, "released cached fullscreen not cleared") != 0) {
        return 1;
    }

    if (RequireDisplayedSnapshot(second_controller.GetRepresentativeSnapshot(), 9403U, third_span, 1U, "fresh fullscreen display mismatch") != 0) {
        return 1;
    }

    const UiManagerPanelMapSnapshot fresh_snapshot = panel_map.Snapshot();
    if (fresh_snapshot.loaded_panel_count != 1U ||
        fresh_snapshot.active_panel_count != 1U ||
        fresh_snapshot.release_operation_count != 1U ||
        fresh_snapshot.reused_loaded_count != 1U) {
        return Fail("fresh fullscreen panel counters mismatch");
    }

    return RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size()), "fresh fullscreen order mismatch");
}

int RunStatusAndDuplicateRemainExplicitTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 2U> panel_ids{3501U, 3502U};
    if (SetupFullscreenLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerFullscreenStack fullscreen_stack;
    UiFullscreenRepresentativeController first_controller;
    UiFullscreenRepresentativeController second_controller;
    std::array<std::uint32_t, 1U> first_values{51U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = OpenArgs(9501U, first_span);
    if (!fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3501U), registry, layer_model, &panel_map, &first_controller, first_args).Succeeded()) {
        return Fail("first status fullscreen open failed");
    }

    UiFullscreenRepresentativeController unused_controller;
    std::array<std::uint32_t, 1U> unused_values{52U};
    const std::span<const std::uint32_t> unused_span(unused_values.data(), unused_values.size());
    const UiPanelOpenArgs unused_args = OpenArgs(9502U, unused_span);
    UiManagerFullscreenStackResult duplicate_result =
        fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3501U), registry, layer_model, &panel_map, &unused_controller, unused_args);
    if (RequireFullscreenStatus(duplicate_result.status, UiManagerFullscreenStackStatus::Success, "duplicate top status mismatch") != 0) {
        return 1;
    }

    if (!duplicate_result.already_top || duplicate_result.pushed) {
        return Fail("duplicate top flags mismatch");
    }

    if (unused_controller.GetRepresentativeSnapshot().open_display_count != 0U) {
        return Fail("duplicate top touched unused controller");
    }

    if (RequireDisplayedSnapshot(first_controller.GetRepresentativeSnapshot(), 9501U, first_span, 1U, "duplicate top mutated display") != 0) {
        return 1;
    }

    std::array<std::uint32_t, 1U> second_values{53U};
    const std::span<const std::uint32_t> second_span(second_values.data(), second_values.size());
    const UiPanelOpenArgs second_args = OpenArgs(9503U, second_span);
    if (!fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3502U), registry, layer_model, &panel_map, &second_controller, second_args).Succeeded()) {
        return Fail("second status fullscreen open failed");
    }

    std::array<std::uint32_t, 1U> duplicate_values{54U};
    const std::span<const std::uint32_t> duplicate_span(duplicate_values.data(), duplicate_values.size());
    const UiPanelOpenArgs duplicate_args = OpenArgs(9504U, duplicate_span);
    duplicate_result =
        fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3501U), registry, layer_model, &panel_map, &unused_controller, duplicate_args);
    if (RequireFullscreenStatus(duplicate_result.status, UiManagerFullscreenStackStatus::Success, "duplicate middle status mismatch") != 0) {
        return 1;
    }

    if (!duplicate_result.already_in_stack ||
        !duplicate_result.moved_to_top ||
        duplicate_result.pushed ||
        duplicate_result.closed_panel_id.value != 3502U) {
        return Fail("duplicate middle flags mismatch");
    }

    const std::array<std::uint32_t, 2U> duplicate_order{3502U, 3501U};
    if (RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(duplicate_order.data(), duplicate_order.size()), "duplicate fullscreen order mismatch") != 0) {
        return 1;
    }

    if (RequireDisplayedSnapshot(first_controller.GetRepresentativeSnapshot(), 9504U, duplicate_span, 2U, "duplicate middle display mismatch") != 0) {
        return 1;
    }

    if (RequireClosedSnapshot(second_controller.GetRepresentativeSnapshot(), 9503U, second_span, 1U, 1U, "duplicate middle did not close previous top") != 0) {
        return 1;
    }

    UiFullscreenRepresentativeController missing_controller;
    duplicate_result =
        fullscreen_stack.OpenFullscreenPanelWithArgs(PanelId(3599U), registry, layer_model, &panel_map, &missing_controller, first_args);
    if (RequireFullscreenStatus(duplicate_result.status, UiManagerFullscreenStackStatus::PanelNotRegistered, "missing fullscreen status mismatch") != 0) {
        return 1;
    }

    if (missing_controller.GetRepresentativeSnapshot().open_display_count != 0U) {
        return Fail("missing fullscreen touched controller");
    }

    const UiManagerFullscreenStackSnapshot snapshot = fullscreen_stack.Snapshot();
    if (snapshot.fullscreen_count != 2U ||
        snapshot.rejected_operation_count != 1U ||
        snapshot.idempotent_open_count != 2U) {
        return Fail("fullscreen status counters mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view test_name) {
    if (test_name == TEST_OPEN_TOP) {
        return RunOpenDisplaysAndTracksTopActiveTest();
    }

    if (test_name == TEST_BACK_RESTORE) {
        return RunOpenSecondAndNavigateBackRestoresPreviousTest();
    }

    if (test_name == TEST_RELEASE_CLEAR) {
        return RunCloseAndReleaseClearSnapshotsTest();
    }

    if (test_name == TEST_CACHE_FRESH) {
        return RunReopenCacheAndReleaseFreshLifecycleTest();
    }

    if (test_name == TEST_STATUS_DUPLICATE) {
        return RunStatusAndDuplicateRemainExplicitTest();
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
