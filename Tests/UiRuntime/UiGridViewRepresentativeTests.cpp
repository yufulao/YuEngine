// 模块: Tests UiRuntime
// 文件: Tests/UiRuntime/UiGridViewRepresentativeTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/UiRuntime/UiGridViewRepresentativeController.h"
#include "YuEngine/UiRuntime/UiGridViewRepresentativeSnapshot.h"
#include "YuEngine/UiRuntime/UiGridViewRepresentativeStatus.h"
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
#include "YuEngine/UiRuntime/UiPanelId.h"
#include "YuEngine/UiRuntime/UiPanelManifestRecord.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgs.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgsSnapshot.h"
#include "YuEngine/UiRuntime/UiPanelRegistry.h"
#include "YuEngine/UiRuntime/UiPanelRegistryResult.h"

using yuengine::uiruntime::INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
using yuengine::uiruntime::UiGridViewRepresentativeCellSnapshot;
using yuengine::uiruntime::UiGridViewRepresentativeController;
using yuengine::uiruntime::UiGridViewRepresentativeSnapshot;
using yuengine::uiruntime::UiGridViewRepresentativeStatus;
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
constexpr const char *TEST_OPEN_DISPLAY =
    "UiRuntime_GridViewRepresentative_OpenDisplaysInventoryWindow";
constexpr const char *TEST_SCROLL_SELECT_REFRESH =
    "UiRuntime_GridViewRepresentative_ScrollSelectRefreshWindow";
constexpr const char *TEST_CLEAR_RELEASE =
    "UiRuntime_GridViewRepresentative_ClearAndReleaseResetState";
constexpr const char *TEST_CACHE_FRESH =
    "UiRuntime_GridViewRepresentative_CloseReopenCacheAndFreshLifecycle";
constexpr const char *TEST_STATUS =
    "UiRuntime_GridViewRepresentative_StatusAndDuplicateRemainExplicit";
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

int RegisterGridLayer(UiManagerLayerModel *layer_model) {
    if (layer_model == nullptr) {
        return Fail("missing layer model");
    }

    const UiManagerLayerRecord record = LayerRecord(71U, UiManagerLayerType::Fullscreen, 70, 971U);
    if (!layer_model->RegisterLayer(record).Succeeded()) {
        return Fail("layer register failed");
    }

    return 0;
}

int RegisterGridPanel(
    UiPanelRegistry *registry,
    UiManagerLayerModel *layer_model,
    std::uint32_t panel_id) {
    if (registry == nullptr || layer_model == nullptr) {
        return Fail("missing grid setup model");
    }

    const UiPanelManifestRecord panel_record =
        PanelRecord(panel_id, 100U + panel_id, 200U + panel_id, 300U + panel_id);
    const UiPanelRegistryResult panel_result = registry->RegisterPanel(panel_record);
    if (!panel_result.Succeeded()) {
        return Fail("panel register failed");
    }

    if (!layer_model->BindPanelToLayer(PanelBinding(panel_id, 71U)).Succeeded()) {
        return Fail("panel layer binding failed");
    }

    return 0;
}

int SetupGridPanel(
    UiPanelRegistry *registry,
    UiManagerLayerModel *layer_model,
    std::uint32_t panel_id) {
    if (RegisterGridLayer(layer_model) != 0) {
        return 1;
    }

    return RegisterGridPanel(registry, layer_model, panel_id);
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

int RequireRepresentativeStatus(
    UiGridViewRepresentativeStatus actual,
    UiGridViewRepresentativeStatus expected,
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

int RequireDisplayedSnapshot(
    const UiGridViewRepresentativeSnapshot &snapshot,
    std::uint32_t request_key,
    std::span<const std::uint32_t> values,
    std::uint32_t expected_open_count,
    std::string_view message) {
    if (!snapshot.visible || !snapshot.displayed || !snapshot.grid_displayed || snapshot.cleared) {
        return Fail(message);
    }

    if (snapshot.open_display_count != expected_open_count) {
        return Fail(message);
    }

    if (snapshot.store_key != request_key) {
        return Fail(message);
    }

    if (snapshot.item_count <= snapshot.pool_cell_count) {
        return Fail(message);
    }

    if (snapshot.visible_item_count == 0U || snapshot.cell_count == 0U) {
        return Fail(message);
    }

    return RequireArgsSnapshot(snapshot.open_args, request_key, values, message);
}

int RequireCell(
    const UiGridViewRepresentativeCellSnapshot &cell,
    std::uint32_t item_index,
    bool visible,
    std::string_view message) {
    if (!cell.has_item || cell.item_index != item_index || cell.visible != visible) {
        return Fail(message);
    }

    if (cell.item_key != 70000U + item_index) {
        return Fail(message);
    }

    if (cell.stack_count == 0U || cell.price <= item_index) {
        return Fail(message);
    }

    return 0;
}

int RequireClearedRepresentative(
    const UiGridViewRepresentativeController &controller,
    std::uint32_t expected_open_count,
    std::uint32_t expected_close_count,
    std::uint32_t expected_clear_count,
    std::string_view message) {
    const UiGridViewRepresentativeSnapshot snapshot = controller.GetRepresentativeSnapshot();
    if (snapshot.visible || snapshot.displayed || snapshot.grid_displayed || !snapshot.cleared) {
        return Fail(message);
    }

    if (snapshot.item_count != 0U || snapshot.cell_count != 0U || snapshot.pool_cell_count != 0U) {
        return Fail(message);
    }

    if (snapshot.open_display_count != expected_open_count ||
        snapshot.close_display_count != expected_close_count ||
        snapshot.clear_display_count != expected_clear_count) {
        return Fail(message);
    }

    return RequireEmptyArgsSnapshot(snapshot.open_args, message);
}

int RunOpenDisplaysInventoryWindowTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (SetupGridPanel(&registry, &layer_model, 6101U) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiGridViewRepresentativeController controller;
    std::array<std::uint32_t, 2U> values{0U, 5U};
    const std::span<const std::uint32_t> value_span(values.data(), values.size());
    const UiPanelOpenArgs open_args = OpenArgs(10101U, value_span);
    const UiManagerPanelMapResult open_result =
        panel_map.OpenPanelWithArgs(PanelId(6101U), registry, layer_model, &controller, open_args);
    if (RequirePanelStatus(open_result.status, UiManagerPanelMapStatus::Success, "grid open failed") != 0) {
        return 1;
    }

    if (!open_result.record.loaded || !open_result.record.active || open_result.record.controller != &controller) {
        return Fail("grid open record mismatch");
    }

    const UiGridViewRepresentativeSnapshot snapshot = controller.GetRepresentativeSnapshot();
    if (RequireDisplayedSnapshot(snapshot, 10101U, value_span, 1U, "grid display snapshot mismatch") != 0) {
        return 1;
    }

    if (snapshot.group_count != 12U ||
        snapshot.first_visible_group != 0U ||
        snapshot.first_materialized_group != 0U ||
        snapshot.materialized_group_count != 3U ||
        snapshot.visible_item_count != 8U ||
        snapshot.cell_count != 12U ||
        snapshot.created_cell_count != 12U ||
        snapshot.reused_cell_count != 0U) {
        return Fail("grid virtualization counts mismatch");
    }

    if (RequireCell(snapshot.cells[0U], 0U, true, "first inventory item mismatch") != 0) {
        return 1;
    }

    if (RequireCell(snapshot.cells[5U], 5U, true, "selected inventory item mismatch") != 0) {
        return 1;
    }

    if (!snapshot.cells[5U].selected || !snapshot.cells[5U].dirty) {
        return Fail("selected inventory item state mismatch");
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.loaded_panel_count != 1U ||
        panel_snapshot.active_panel_count != 1U ||
        panel_snapshot.parameter_open_count != 1U) {
        return Fail("grid panel map counters mismatch");
    }

    return 0;
}

int RunScrollSelectRefreshWindowTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (SetupGridPanel(&registry, &layer_model, 6201U) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiGridViewRepresentativeController controller;
    std::array<std::uint32_t, 2U> values{0U, 2U};
    const std::span<const std::uint32_t> value_span(values.data(), values.size());
    const UiPanelOpenArgs open_args = OpenArgs(10201U, value_span);
    if (!panel_map.OpenPanelWithArgs(PanelId(6201U), registry, layer_model, &controller, open_args).Succeeded()) {
        return Fail("grid open before scroll failed");
    }

    if (RequireRepresentativeStatus(
        controller.ScrollToItem(24U),
        UiGridViewRepresentativeStatus::Success,
        "scroll to item failed") != 0) {
        return 1;
    }

    UiGridViewRepresentativeSnapshot snapshot = controller.GetRepresentativeSnapshot();
    if (snapshot.first_visible_group != 6U ||
        snapshot.first_materialized_group != 5U ||
        snapshot.materialized_group_count != 4U ||
        snapshot.cell_count != 16U ||
        snapshot.reused_cell_count != 12U ||
        snapshot.created_cell_count != 4U ||
        snapshot.scroll_count != 1U) {
        return Fail("scroll window counters mismatch");
    }

    if (RequireCell(snapshot.cells[0U], 20U, false, "scroll buffer item mismatch") != 0) {
        return 1;
    }

    if (RequireCell(snapshot.cells[4U], 24U, true, "scroll visible item mismatch") != 0) {
        return 1;
    }

    if (RequireRepresentativeStatus(
        controller.SelectItem(25U),
        UiGridViewRepresentativeStatus::Success,
        "select item failed") != 0) {
        return 1;
    }

    snapshot = controller.GetRepresentativeSnapshot();
    if (snapshot.selected_index != 25U ||
        snapshot.previous_selected_index != 2U ||
        snapshot.selection_change_count != 1U ||
        snapshot.dirty_cell_count != 8U) {
        return Fail("selection counters mismatch");
    }

    if (RequireCell(snapshot.cells[5U], 25U, true, "selected scrolled item mismatch") != 0) {
        return 1;
    }

    if (!snapshot.cells[5U].selected || !snapshot.cells[5U].dirty) {
        return Fail("selected scrolled item state mismatch");
    }

    if (RequireRepresentativeStatus(
        controller.RefreshItem(26U),
        UiGridViewRepresentativeStatus::Success,
        "refresh item failed") != 0) {
        return 1;
    }

    snapshot = controller.GetRepresentativeSnapshot();
    if (snapshot.updated_item_index != 26U || snapshot.refresh_count != 1U) {
        return Fail("refresh counters mismatch");
    }

    if (RequireCell(snapshot.cells[6U], 26U, true, "refreshed scrolled item mismatch") != 0) {
        return 1;
    }

    if (!snapshot.cells[6U].dirty) {
        return Fail("refreshed item was not dirty");
    }

    if (RequireRepresentativeStatus(
        controller.ScrollToGroup(2U),
        UiGridViewRepresentativeStatus::Success,
        "scroll to group failed") != 0) {
        return 1;
    }

    snapshot = controller.GetRepresentativeSnapshot();
    if (snapshot.first_visible_group != 2U || snapshot.scroll_count != 2U) {
        return Fail("scroll to group counters mismatch");
    }

    return RequireCell(snapshot.cells[4U], 8U, true, "scroll to group visible item mismatch");
}

int RunClearAndReleaseResetStateTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (SetupGridPanel(&registry, &layer_model, 6301U) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiGridViewRepresentativeController controller;
    std::array<std::uint32_t, 2U> values{1U, 7U};
    const std::span<const std::uint32_t> value_span(values.data(), values.size());
    const UiPanelOpenArgs open_args = OpenArgs(10301U, value_span);
    if (!panel_map.OpenPanelWithArgs(PanelId(6301U), registry, layer_model, &controller, open_args).Succeeded()) {
        return Fail("grid open before clear failed");
    }

    if (RequireRepresentativeStatus(
        controller.ClearGrid(),
        UiGridViewRepresentativeStatus::Success,
        "clear grid failed") != 0) {
        return 1;
    }

    UiGridViewRepresentativeSnapshot snapshot = controller.GetRepresentativeSnapshot();
    if (!snapshot.visible ||
        snapshot.displayed ||
        snapshot.grid_displayed ||
        !snapshot.grid_cleared ||
        snapshot.item_count != 0U ||
        snapshot.cell_count != 0U ||
        snapshot.clear_grid_count != 1U) {
        return Fail("clear grid state mismatch");
    }

    if (RequireArgsSnapshot(snapshot.open_args, 10301U, value_span, "clear grid args mismatch") != 0) {
        return 1;
    }

    const UiManagerPanelMapResult release_result = panel_map.ReleasePanel(PanelId(6301U));
    if (RequirePanelStatus(release_result.status, UiManagerPanelMapStatus::Success, "release after clear failed") != 0) {
        return 1;
    }

    if (!release_result.released_loaded || !release_result.released_active) {
        return Fail("release after clear flags mismatch");
    }

    if (RequireClearedRepresentative(controller, 1U, 1U, 1U, "released cleared grid snapshot mismatch") != 0) {
        return 1;
    }

    if (release_result.record.loaded ||
        release_result.record.active ||
        release_result.record.controller != nullptr) {
        return Fail("release result did not clear record");
    }

    UiManagerPanelMapRecord loaded_record{};
    const UiManagerPanelMapStatus loaded_status = panel_map.ResolveLoadedPanel(PanelId(6301U), &loaded_record);
    if (RequirePanelStatus(loaded_status, UiManagerPanelMapStatus::PanelNotLoaded, "release did not clear loaded map") != 0) {
        return 1;
    }

    return RequireEmptyArgsSnapshot(release_result.record.open_args, "release args snapshot mismatch");
}

int RunCloseReopenCacheAndFreshLifecycleTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (SetupGridPanel(&registry, &layer_model, 6401U) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiGridViewRepresentativeController first_controller;
    std::array<std::uint32_t, 2U> first_values{0U, 3U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = OpenArgs(10401U, first_span);
    if (!panel_map.OpenPanelWithArgs(PanelId(6401U), registry, layer_model, &first_controller, first_args).Succeeded()) {
        return Fail("first grid open failed");
    }

    if (!panel_map.ClosePanel(PanelId(6401U)).Succeeded()) {
        return Fail("grid close before cached reopen failed");
    }

    std::array<std::uint32_t, 2U> second_values{3U, 13U};
    const std::span<const std::uint32_t> second_span(second_values.data(), second_values.size());
    const UiPanelOpenArgs second_args = OpenArgs(10402U, second_span);
    const UiManagerPanelMapResult reopen_result =
        panel_map.OpenPanelWithArgs(PanelId(6401U), registry, layer_model, nullptr, second_args);
    if (RequirePanelStatus(reopen_result.status, UiManagerPanelMapStatus::Success, "cached grid reopen failed") != 0) {
        return 1;
    }

    if (!reopen_result.reused_loaded || reopen_result.record.controller != &first_controller) {
        return Fail("cached grid reopen flags mismatch");
    }

    UiGridViewRepresentativeSnapshot snapshot = first_controller.GetRepresentativeSnapshot();
    if (RequireDisplayedSnapshot(snapshot, 10402U, second_span, 2U, "cached grid display mismatch") != 0) {
        return 1;
    }

    if (snapshot.close_display_count != 1U || snapshot.first_visible_group != 3U) {
        return Fail("cached grid state mismatch");
    }

    const UiManagerPanelMapSnapshot cached_snapshot = panel_map.Snapshot();
    if (cached_snapshot.reused_loaded_count != 1U ||
        cached_snapshot.reopen_open_args_update_count != 1U ||
        cached_snapshot.loaded_panel_count != 1U ||
        cached_snapshot.active_panel_count != 1U) {
        return Fail("cached grid panel counters mismatch");
    }

    if (!panel_map.ReleasePanel(PanelId(6401U)).Succeeded()) {
        return Fail("release before fresh grid reopen failed");
    }

    UiGridViewRepresentativeController second_controller;
    std::array<std::uint32_t, 2U> third_values{5U, 21U};
    const std::span<const std::uint32_t> third_span(third_values.data(), third_values.size());
    const UiPanelOpenArgs third_args = OpenArgs(10403U, third_span);
    const UiManagerPanelMapResult fresh_result =
        panel_map.OpenPanelWithArgs(PanelId(6401U), registry, layer_model, &second_controller, third_args);
    if (RequirePanelStatus(fresh_result.status, UiManagerPanelMapStatus::Success, "fresh grid reopen failed") != 0) {
        return 1;
    }

    if (fresh_result.record.controller != &second_controller || fresh_result.reused_loaded) {
        return Fail("fresh grid reopen flags mismatch");
    }

    if (RequireClearedRepresentative(first_controller, 2U, 2U, 1U, "released cached grid not cleared") != 0) {
        return 1;
    }

    snapshot = second_controller.GetRepresentativeSnapshot();
    if (RequireDisplayedSnapshot(snapshot, 10403U, third_span, 1U, "fresh grid display mismatch") != 0) {
        return 1;
    }

    if (snapshot.first_visible_group != 5U) {
        return Fail("fresh grid first visible group mismatch");
    }

    return 0;
}

int RunStatusAndDuplicateRemainExplicitTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (SetupGridPanel(&registry, &layer_model, 6501U) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiGridViewRepresentativeController controller;
    std::array<std::uint32_t, 2U> first_values{0U, 1U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = OpenArgs(10501U, first_span);
    if (!panel_map.OpenPanelWithArgs(PanelId(6501U), registry, layer_model, &controller, first_args).Succeeded()) {
        return Fail("status grid open failed");
    }

    UiGridViewRepresentativeController unused_controller;
    std::array<std::uint32_t, 2U> duplicate_values{2U, 9U};
    const std::span<const std::uint32_t> duplicate_span(duplicate_values.data(), duplicate_values.size());
    const UiPanelOpenArgs duplicate_args = OpenArgs(10502U, duplicate_span);
    const UiManagerPanelMapResult duplicate_result =
        panel_map.OpenPanelWithArgs(PanelId(6501U), registry, layer_model, &unused_controller, duplicate_args);
    if (RequirePanelStatus(duplicate_result.status, UiManagerPanelMapStatus::Success, "duplicate grid status mismatch") != 0) {
        return 1;
    }

    if (!duplicate_result.reused_loaded ||
        !duplicate_result.already_active ||
        duplicate_result.record.controller != &controller) {
        return Fail("duplicate grid flags mismatch");
    }

    if (unused_controller.GetRepresentativeSnapshot().open_display_count != 0U) {
        return Fail("duplicate grid touched unused controller");
    }

    if (RequireDisplayedSnapshot(controller.GetRepresentativeSnapshot(), 10501U, first_span, 1U, "duplicate grid mutated display") != 0) {
        return 1;
    }

    UiGridViewRepresentativeController missing_controller;
    const UiManagerPanelMapResult missing_result =
        panel_map.OpenPanelWithArgs(PanelId(6599U), registry, layer_model, &missing_controller, first_args);
    if (RequirePanelStatus(missing_result.status, UiManagerPanelMapStatus::PanelNotRegistered, "missing grid status mismatch") != 0) {
        return 1;
    }

    if (missing_controller.GetRepresentativeSnapshot().open_display_count != 0U) {
        return Fail("missing grid touched controller");
    }

    if (RequireRepresentativeStatus(
        controller.ScrollToItem(999U),
        UiGridViewRepresentativeStatus::InvalidItemIndex,
        "invalid scroll item status mismatch") != 0) {
        return 1;
    }

    if (RequireRepresentativeStatus(
        controller.SelectItem(999U),
        UiGridViewRepresentativeStatus::InvalidItemIndex,
        "invalid select item status mismatch") != 0) {
        return 1;
    }

    if (RequireRepresentativeStatus(
        controller.RefreshItem(999U),
        UiGridViewRepresentativeStatus::InvalidItemIndex,
        "invalid refresh item status mismatch") != 0) {
        return 1;
    }

    if (RequireRepresentativeStatus(
        controller.ScrollToGroup(999U),
        UiGridViewRepresentativeStatus::InvalidGroupIndex,
        "invalid group status mismatch") != 0) {
        return 1;
    }

    const UiGridViewRepresentativeSnapshot snapshot = controller.GetRepresentativeSnapshot();
    if (snapshot.last_status != UiGridViewRepresentativeStatus::InvalidGroupIndex ||
        snapshot.store_key != 10501U ||
        snapshot.open_display_count != 1U) {
        return Fail("invalid status mutated grid display");
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.idempotent_open_count != 1U ||
        panel_snapshot.rejected_operation_count != 1U ||
        panel_snapshot.accepted_operation_count != 2U) {
        return Fail("status panel counters mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view test_name) {
    if (test_name == TEST_OPEN_DISPLAY) {
        return RunOpenDisplaysInventoryWindowTest();
    }

    if (test_name == TEST_SCROLL_SELECT_REFRESH) {
        return RunScrollSelectRefreshWindowTest();
    }

    if (test_name == TEST_CLEAR_RELEASE) {
        return RunClearAndReleaseResetStateTest();
    }

    if (test_name == TEST_CACHE_FRESH) {
        return RunCloseReopenCacheAndFreshLifecycleTest();
    }

    if (test_name == TEST_STATUS) {
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
