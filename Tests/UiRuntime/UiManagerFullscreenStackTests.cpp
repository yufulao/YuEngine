// 模块: Tests UiRuntime
// 文件: Tests/UiRuntime/UiManagerFullscreenStackTests.cpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/UiRuntime/BaseUiController.h"
#include "YuEngine/UiRuntime/BaseUiLifecycleSnapshot.h"
#include "YuEngine/UiRuntime/BaseUiLifecycleStatus.h"
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
#include "YuEngine/UiRuntime/UiManagerPanelMapResult.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapStatus.h"
#include "YuEngine/UiRuntime/UiPanelId.h"
#include "YuEngine/UiRuntime/UiPanelManifestRecord.h"
#include "YuEngine/UiRuntime/UiPanelRegistry.h"
#include "YuEngine/UiRuntime/UiPanelRegistryResult.h"

using yuengine::uiruntime::BaseUiController;
using yuengine::uiruntime::BaseUiLifecycleSnapshot;
using yuengine::uiruntime::BaseUiLifecycleStatus;
using yuengine::uiruntime::MAX_UI_MANAGER_FULLSCREEN_STACK_COUNT;
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
using yuengine::uiruntime::UiManagerPanelMapResult;
using yuengine::uiruntime::UiManagerPanelMapSnapshot;
using yuengine::uiruntime::UiManagerPanelMapStatus;
using yuengine::uiruntime::UiPanelControllerRef;
using yuengine::uiruntime::UiPanelId;
using yuengine::uiruntime::UiPanelLayoutRef;
using yuengine::uiruntime::UiPanelManifestRecord;
using yuengine::uiruntime::UiPanelRegistry;
using yuengine::uiruntime::UiPanelRegistryResult;
using yuengine::uiruntime::UiPanelResourceRef;

namespace {
constexpr const char *TEST_OPEN_ORDER =
    "UiRuntime_ManagerFullscreenStack_OpenPushesFullscreenOrder";
constexpr const char *TEST_NAVIGATE_BACK =
    "UiRuntime_ManagerFullscreenStack_NavigateBackRestoresPreviousFullscreen";
constexpr const char *TEST_CLOSE_CURRENT_MIDDLE =
    "UiRuntime_ManagerFullscreenStack_CloseCurrentAndMiddleFullscreen";
constexpr const char *TEST_DUPLICATE_OPEN =
    "UiRuntime_ManagerFullscreenStack_DuplicateOpenMovesExistingFullscreen";
constexpr const char *TEST_REJECTS_STATUS =
    "UiRuntime_ManagerFullscreenStack_RejectsMissingNonFullscreenAndBackEmptyStatus";
constexpr const char *TEST_RELEASE_RESTORES =
    "UiRuntime_ManagerFullscreenStack_ReleaseRestoresPreviousFullscreen";
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

int RequireStatus(
    UiManagerFullscreenStackStatus actual,
    UiManagerFullscreenStackStatus expected,
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

int SetupFullscreenLayer(
    UiPanelRegistry *registry,
    UiManagerLayerModel *layer_model,
    std::span<const std::uint32_t> panel_ids) {
    if (registry == nullptr || layer_model == nullptr) {
        return Fail("missing fullscreen setup model");
    }

    if (RegisterLayer(layer_model, 60U, UiManagerLayerType::Fullscreen, 20, 960U) != 0) {
        return 1;
    }

    for (std::uint32_t panel_id : panel_ids) {
        if (RegisterPanelAndBind(registry, layer_model, panel_id, 60U) != 0) {
            return 1;
        }
    }

    return 0;
}

int RequireActiveFullscreen(
    const UiManagerPanelMap &panel_map,
    std::uint32_t panel_id,
    std::uint32_t root_key,
    std::string_view message) {
    UiManagerPanelMapRecord active_record{};
    const UiManagerPanelMapStatus active_status = panel_map.ResolveActivePanel(PanelId(panel_id), &active_record);
    if (active_status != UiManagerPanelMapStatus::Success) {
        return Fail(message);
    }

    if (active_record.layer_record.type != UiManagerLayerType::Fullscreen) {
        return Fail(message);
    }

    if (active_record.layer_record.root_ref.root_key != root_key) {
        return Fail(message);
    }

    return 0;
}

int RequireInactiveLoaded(
    const UiManagerPanelMap &panel_map,
    std::uint32_t panel_id,
    std::string_view message) {
    UiManagerPanelMapRecord loaded_record{};
    const UiManagerPanelMapStatus loaded_status = panel_map.ResolveLoadedPanel(PanelId(panel_id), &loaded_record);
    if (loaded_status != UiManagerPanelMapStatus::Success) {
        return Fail(message);
    }

    if (!loaded_record.loaded || loaded_record.active) {
        return Fail(message);
    }

    return 0;
}

int RunOpenPushesFullscreenOrderTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 3U> panel_ids{1001U, 1002U, 1003U};
    if (SetupFullscreenLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerFullscreenStack fullscreen_stack;
    TestPanelController first_controller;
    TestPanelController second_controller;
    TestPanelController third_controller;

    UiManagerFullscreenStackResult result =
        fullscreen_stack.OpenFullscreenPanel(PanelId(1001U), registry, layer_model, &panel_map, &first_controller);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "first fullscreen open failed") != 0) {
        return 1;
    }

    result = fullscreen_stack.OpenFullscreenPanel(PanelId(1002U), registry, layer_model, &panel_map, &second_controller);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "second fullscreen open failed") != 0) {
        return 1;
    }

    result = fullscreen_stack.OpenFullscreenPanel(PanelId(1003U), registry, layer_model, &panel_map, &third_controller);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "third fullscreen open failed") != 0) {
        return 1;
    }

    if (!result.pushed || result.top_panel_id.value != 1003U) {
        return Fail("third fullscreen was not pushed to top");
    }

    if (RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size()), "fullscreen open order mismatch") != 0) {
        return 1;
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.loaded_panel_count != 3U || panel_snapshot.active_panel_count != 1U) {
        return Fail("panel map counts mismatch after fullscreen opens");
    }

    if (RequireActiveFullscreen(panel_map, 1003U, 960U, "top fullscreen active resolve failed") != 0) {
        return 1;
    }

    if (RequireInactiveLoaded(panel_map, 1001U, "first fullscreen was not inactive") != 0) {
        return 1;
    }

    if (RequireInactiveLoaded(panel_map, 1002U, "second fullscreen was not inactive") != 0) {
        return 1;
    }

    std::array<UiPanelId, 2U> small_order{};
    const UiManagerFullscreenStackStatus small_export =
        fullscreen_stack.ExportFullscreenOrder(small_order.data(), static_cast<std::uint32_t>(small_order.size()));
    return RequireStatus(small_export, UiManagerFullscreenStackStatus::InvalidOutputBuffer, "small fullscreen export status mismatch");
}

int RunNavigateBackRestoresPreviousFullscreenTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 3U> panel_ids{1011U, 1012U, 1013U};
    if (SetupFullscreenLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerFullscreenStack fullscreen_stack;
    TestPanelController first_controller;
    TestPanelController second_controller;
    TestPanelController third_controller;
    if (!fullscreen_stack.OpenFullscreenPanel(PanelId(1011U), registry, layer_model, &panel_map, &first_controller).Succeeded()) {
        return Fail("first fullscreen open failed");
    }

    if (!fullscreen_stack.OpenFullscreenPanel(PanelId(1012U), registry, layer_model, &panel_map, &second_controller).Succeeded()) {
        return Fail("second fullscreen open failed");
    }

    if (!fullscreen_stack.OpenFullscreenPanel(PanelId(1013U), registry, layer_model, &panel_map, &third_controller).Succeeded()) {
        return Fail("third fullscreen open failed");
    }

    UiManagerFullscreenStackResult result = fullscreen_stack.NavigateBack(registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "first navigate back failed") != 0) {
        return 1;
    }

    if (!result.navigated_back || !result.restored_previous || result.closed_panel_id.value != 1013U ||
        result.restored_panel_id.value != 1012U) {
        return Fail("first navigate back flags mismatch");
    }

    const std::array<std::uint32_t, 2U> first_expected_order{1011U, 1012U};
    if (RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(first_expected_order.data(), first_expected_order.size()), "first back order mismatch") != 0) {
        return 1;
    }

    if (RequireActiveFullscreen(panel_map, 1012U, 960U, "first restored fullscreen inactive") != 0) {
        return 1;
    }

    if (RequireInactiveLoaded(panel_map, 1013U, "closed fullscreen was not inactive") != 0) {
        return 1;
    }

    result = fullscreen_stack.NavigateBack(registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "second navigate back failed") != 0) {
        return 1;
    }

    if (!result.restored_previous || result.closed_panel_id.value != 1012U || result.restored_panel_id.value != 1011U) {
        return Fail("second navigate back flags mismatch");
    }

    const std::array<std::uint32_t, 1U> second_expected_order{1011U};
    if (RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(second_expected_order.data(), second_expected_order.size()), "second back order mismatch") != 0) {
        return 1;
    }

    if (RequireActiveFullscreen(panel_map, 1011U, 960U, "second restored fullscreen inactive") != 0) {
        return 1;
    }

    const UiManagerFullscreenStackSnapshot snapshot = fullscreen_stack.Snapshot();
    if (snapshot.back_navigation_operation_count != 2U || snapshot.restore_operation_count != 2U) {
        return Fail("navigate back counters mismatch");
    }

    return 0;
}

int RunCloseCurrentAndMiddleFullscreenTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 3U> panel_ids{1021U, 1022U, 1023U};
    if (SetupFullscreenLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerFullscreenStack fullscreen_stack;
    TestPanelController first_controller;
    TestPanelController second_controller;
    TestPanelController third_controller;
    if (!fullscreen_stack.OpenFullscreenPanel(PanelId(1021U), registry, layer_model, &panel_map, &first_controller).Succeeded()) {
        return Fail("first fullscreen open failed");
    }

    if (!fullscreen_stack.OpenFullscreenPanel(PanelId(1022U), registry, layer_model, &panel_map, &second_controller).Succeeded()) {
        return Fail("second fullscreen open failed");
    }

    if (!fullscreen_stack.OpenFullscreenPanel(PanelId(1023U), registry, layer_model, &panel_map, &third_controller).Succeeded()) {
        return Fail("third fullscreen open failed");
    }

    UiManagerFullscreenStackResult result =
        fullscreen_stack.CloseFullscreenPanel(PanelId(1022U), registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "middle fullscreen close failed") != 0) {
        return 1;
    }

    if (!result.closed_middle || !result.already_inactive || !result.removed_from_stack) {
        return Fail("middle close flags mismatch");
    }

    const std::array<std::uint32_t, 2U> middle_expected_order{1021U, 1023U};
    if (RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(middle_expected_order.data(), middle_expected_order.size()), "middle close order mismatch") != 0) {
        return 1;
    }

    result = fullscreen_stack.CloseFullscreenPanel(PanelId(1023U), registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "current fullscreen close failed") != 0) {
        return 1;
    }

    if (!result.closed_current || !result.restored_previous || result.restored_panel_id.value != 1021U) {
        return Fail("current close flags mismatch");
    }

    const std::array<std::uint32_t, 1U> current_expected_order{1021U};
    if (RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(current_expected_order.data(), current_expected_order.size()), "current close order mismatch") != 0) {
        return 1;
    }

    if (RequireActiveFullscreen(panel_map, 1021U, 960U, "current close did not restore previous") != 0) {
        return 1;
    }

    result = fullscreen_stack.CloseFullscreenPanel(PanelId(1022U), registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "duplicate middle close failed") != 0) {
        return 1;
    }

    if (!result.already_inactive || result.removed_from_stack) {
        return Fail("duplicate close flags mismatch");
    }

    const UiManagerFullscreenStackSnapshot snapshot = fullscreen_stack.Snapshot();
    if (snapshot.close_operation_count != 1U ||
        snapshot.idempotent_close_count != 2U ||
        snapshot.restore_operation_count != 1U) {
        return Fail("close counters mismatch");
    }

    return 0;
}

int RunDuplicateOpenMovesExistingFullscreenTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 2U> panel_ids{1031U, 1032U};
    if (SetupFullscreenLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerFullscreenStack fullscreen_stack;
    TestPanelController first_controller;
    TestPanelController second_controller;
    TestPanelController unused_controller;
    TestPanelController second_unused_controller;
    if (!fullscreen_stack.OpenFullscreenPanel(PanelId(1031U), registry, layer_model, &panel_map, &first_controller).Succeeded()) {
        return Fail("first fullscreen open failed");
    }

    UiManagerFullscreenStackResult result =
        fullscreen_stack.OpenFullscreenPanel(PanelId(1031U), registry, layer_model, &panel_map, &unused_controller);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "duplicate top fullscreen open failed") != 0) {
        return 1;
    }

    if (!result.already_top || !result.already_in_stack || result.pushed) {
        return Fail("duplicate top open flags mismatch");
    }

    if (!fullscreen_stack.OpenFullscreenPanel(PanelId(1032U), registry, layer_model, &panel_map, &second_controller).Succeeded()) {
        return Fail("second fullscreen open failed");
    }

    result = fullscreen_stack.OpenFullscreenPanel(PanelId(1031U), registry, layer_model, &panel_map, &second_unused_controller);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "duplicate middle fullscreen open failed") != 0) {
        return 1;
    }

    if (!result.already_in_stack || !result.moved_to_top || result.pushed || result.closed_panel_id.value != 1032U) {
        return Fail("duplicate middle open flags mismatch");
    }

    const std::array<std::uint32_t, 2U> expected_order{1032U, 1031U};
    if (RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(expected_order.data(), expected_order.size()), "duplicate open order mismatch") != 0) {
        return 1;
    }

    if (RequireActiveFullscreen(panel_map, 1031U, 960U, "duplicate open did not reactivate target") != 0) {
        return 1;
    }

    if (RequireInactiveLoaded(panel_map, 1032U, "duplicate open did not close previous top") != 0) {
        return 1;
    }

    const BaseUiLifecycleSnapshot unused_snapshot = unused_controller.Snapshot();
    if (unused_snapshot.open_count != 0U) {
        return Fail("duplicate top open touched unused controller");
    }

    const BaseUiLifecycleSnapshot second_unused_snapshot = second_unused_controller.Snapshot();
    if (second_unused_snapshot.open_count != 0U) {
        return Fail("duplicate middle open touched unused controller");
    }

    const UiManagerFullscreenStackSnapshot snapshot = fullscreen_stack.Snapshot();
    if (snapshot.idempotent_open_count != 2U) {
        return Fail("duplicate open counter mismatch");
    }

    return 0;
}

int RunRejectsMissingNonFullscreenAndBackEmptyStatusTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (RegisterLayer(&layer_model, 70U, UiManagerLayerType::Fullscreen, 20, 970U) != 0) {
        return 1;
    }

    if (RegisterLayer(&layer_model, 71U, UiManagerLayerType::Popup, 30, 971U) != 0) {
        return 1;
    }

    if (RegisterPanelAndBind(&registry, &layer_model, 1041U, 70U) != 0) {
        return 1;
    }

    if (RegisterPanelAndBind(&registry, &layer_model, 1042U, 71U) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerFullscreenStack fullscreen_stack;
    TestPanelController fullscreen_controller;
    TestPanelController popup_controller;
    const UiManagerFullscreenStackSnapshot initial_snapshot = fullscreen_stack.Snapshot();
    if (initial_snapshot.failed_operation_count != 0U ||
        initial_snapshot.rejected_operation_count != 0U ||
        initial_snapshot.accepted_operation_count != 0U) {
        return Fail("fullscreen initial operation counters changed");
    }

    UiManagerFullscreenStackResult result = fullscreen_stack.NavigateBack(registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::BackStackEmpty, "empty back status mismatch") != 0) {
        return 1;
    }

    result = fullscreen_stack.OpenFullscreenPanel(PanelId(9999U), registry, layer_model, &panel_map, &fullscreen_controller);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::PanelNotRegistered, "missing fullscreen open status mismatch") != 0) {
        return 1;
    }

    result = fullscreen_stack.OpenFullscreenPanel(PanelId(1042U), registry, layer_model, &panel_map, &popup_controller);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::NonFullscreenPanel, "non-fullscreen open status mismatch") != 0) {
        return 1;
    }

    if (panel_map.IsLoaded(PanelId(1042U))) {
        return Fail("non-fullscreen stack open mutated panel map");
    }

    const UiManagerPanelMapResult popup_open =
        panel_map.OpenPanel(PanelId(1042U), registry, layer_model, &popup_controller);
    if (!popup_open.Succeeded()) {
        return Fail("direct popup open failed");
    }

    result = fullscreen_stack.CloseFullscreenPanel(PanelId(1042U), registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::NonFullscreenPanel, "non-fullscreen close status mismatch") != 0) {
        return 1;
    }

    const UiManagerPanelMapResult direct_open =
        panel_map.OpenPanel(PanelId(1041U), registry, layer_model, &fullscreen_controller);
    if (!direct_open.Succeeded()) {
        return Fail("direct fullscreen open failed");
    }

    result = fullscreen_stack.CloseFullscreenPanel(PanelId(1041U), registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::FullscreenNotInStack, "untracked fullscreen close status mismatch") != 0) {
        return 1;
    }

    const UiManagerFullscreenStackSnapshot before_recovery_snapshot = fullscreen_stack.Snapshot();
    if (before_recovery_snapshot.failed_operation_count != 5U ||
        before_recovery_snapshot.rejected_operation_count != 5U ||
        before_recovery_snapshot.accepted_operation_count != 0U) {
        return Fail("fullscreen failure counters before recovery mismatch");
    }

    if (before_recovery_snapshot.last_status != UiManagerFullscreenStackStatus::FullscreenNotInStack) {
        return Fail("fullscreen failure last status before recovery mismatch");
    }

    if (!panel_map.ClosePanel(PanelId(1041U)).Succeeded()) {
        return Fail("direct fullscreen close failed");
    }

    result = fullscreen_stack.CloseFullscreenPanel(PanelId(1041U), registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "inactive untracked fullscreen close failed") != 0) {
        return 1;
    }

    if (!result.already_inactive || result.removed_from_stack) {
        return Fail("inactive untracked close flags mismatch");
    }

    const UiManagerFullscreenStackSnapshot after_recovery_snapshot = fullscreen_stack.Snapshot();
    if (after_recovery_snapshot.failed_operation_count != before_recovery_snapshot.failed_operation_count ||
        after_recovery_snapshot.rejected_operation_count != before_recovery_snapshot.rejected_operation_count ||
        after_recovery_snapshot.accepted_operation_count != 1U) {
        return Fail("fullscreen success-after-failure counters mismatch");
    }

    if (after_recovery_snapshot.last_status != UiManagerFullscreenStackStatus::Success) {
        return Fail("fullscreen success-after-failure did not clear last status");
    }

    result = fullscreen_stack.CloseFullscreenPanel(PanelId(9999U), registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::PanelNotLoaded, "missing close status mismatch") != 0) {
        return 1;
    }

    const UiManagerFullscreenStackSnapshot final_snapshot = fullscreen_stack.Snapshot();
    if (final_snapshot.failed_operation_count != 6U ||
        final_snapshot.rejected_operation_count != 6U ||
        final_snapshot.accepted_operation_count != 1U) {
        return Fail("fullscreen final failure counters mismatch");
    }

    if (final_snapshot.last_status != UiManagerFullscreenStackStatus::PanelNotLoaded) {
        return Fail("fullscreen final failure last status mismatch");
    }

    return 0;
}

int RunReleaseRestoresPreviousFullscreenTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 3U> panel_ids{1051U, 1052U, 1053U};
    if (SetupFullscreenLayer(&registry, &layer_model, std::span<const std::uint32_t>(panel_ids.data(), panel_ids.size())) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerFullscreenStack fullscreen_stack;
    TestPanelController first_controller;
    TestPanelController second_controller;
    TestPanelController third_controller;
    if (!fullscreen_stack.OpenFullscreenPanel(PanelId(1051U), registry, layer_model, &panel_map, &first_controller).Succeeded()) {
        return Fail("first fullscreen open failed");
    }

    if (!fullscreen_stack.OpenFullscreenPanel(PanelId(1052U), registry, layer_model, &panel_map, &second_controller).Succeeded()) {
        return Fail("second fullscreen open failed");
    }

    if (!fullscreen_stack.OpenFullscreenPanel(PanelId(1053U), registry, layer_model, &panel_map, &third_controller).Succeeded()) {
        return Fail("third fullscreen open failed");
    }

    UiManagerFullscreenStackResult result =
        fullscreen_stack.ReleaseFullscreenPanel(PanelId(1052U), registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "middle fullscreen release failed") != 0) {
        return 1;
    }

    if (!result.closed_middle || !result.already_inactive || !result.removed_from_stack) {
        return Fail("middle release flags mismatch");
    }

    const std::array<std::uint32_t, 2U> middle_expected_order{1051U, 1053U};
    if (RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(middle_expected_order.data(), middle_expected_order.size()), "middle release order mismatch") != 0) {
        return 1;
    }

    UiManagerPanelMapRecord loaded_record{};
    UiManagerPanelMapStatus map_status = panel_map.ResolveLoadedPanel(PanelId(1052U), &loaded_record);
    if (map_status != UiManagerPanelMapStatus::PanelNotLoaded) {
        return Fail("middle release left loaded record");
    }

    result = fullscreen_stack.ReleaseFullscreenPanel(PanelId(1053U), registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::Success, "current fullscreen release failed") != 0) {
        return 1;
    }

    if (!result.closed_current || !result.restored_previous || result.restored_panel_id.value != 1051U) {
        return Fail("current release restore flags mismatch");
    }

    const std::array<std::uint32_t, 1U> current_expected_order{1051U};
    if (RequireFullscreenOrder(fullscreen_stack, std::span<const std::uint32_t>(current_expected_order.data(), current_expected_order.size()), "current release order mismatch") != 0) {
        return 1;
    }

    if (RequireActiveFullscreen(panel_map, 1051U, 960U, "release did not restore previous fullscreen") != 0) {
        return 1;
    }

    map_status = panel_map.ResolveLoadedPanel(PanelId(1053U), &loaded_record);
    if (map_status != UiManagerPanelMapStatus::PanelNotLoaded) {
        return Fail("current release left loaded record");
    }

    const BaseUiLifecycleSnapshot second_snapshot = second_controller.Snapshot();
    if (second_snapshot.clear_count != 1U || !second_snapshot.destroyed) {
        return Fail("middle release lifecycle mismatch");
    }

    const BaseUiLifecycleSnapshot third_snapshot = third_controller.Snapshot();
    if (third_snapshot.close_count != 1U ||
        third_snapshot.clear_count != 1U ||
        !third_snapshot.destroyed) {
        return Fail("current release lifecycle mismatch");
    }

    const BaseUiLifecycleSnapshot first_snapshot = first_controller.Snapshot();
    if (first_snapshot.open_count != 2U || first_snapshot.clear_count != 0U) {
        return Fail("restored fullscreen lifecycle mismatch");
    }

    result = fullscreen_stack.ReleaseFullscreenPanel(PanelId(1052U), registry, layer_model, &panel_map);
    if (RequireStatus(result.status, UiManagerFullscreenStackStatus::PanelNotLoaded, "duplicate fullscreen release status mismatch") != 0) {
        return 1;
    }

    const UiManagerFullscreenStackSnapshot snapshot = fullscreen_stack.Snapshot();
    if (snapshot.release_operation_count != 2U ||
        snapshot.restore_operation_count != 1U ||
        snapshot.rejected_operation_count != 1U) {
        return Fail("fullscreen release counters mismatch");
    }

    const UiManagerPanelMapSnapshot panel_snapshot = panel_map.Snapshot();
    if (panel_snapshot.loaded_panel_count != 1U ||
        panel_snapshot.active_panel_count != 1U ||
        panel_snapshot.release_operation_count != 2U ||
        panel_snapshot.release_active_operation_count != 1U) {
        return Fail("fullscreen release panel map counters mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view test_name) {
    if (test_name == TEST_OPEN_ORDER) {
        return RunOpenPushesFullscreenOrderTest();
    }

    if (test_name == TEST_NAVIGATE_BACK) {
        return RunNavigateBackRestoresPreviousFullscreenTest();
    }

    if (test_name == TEST_CLOSE_CURRENT_MIDDLE) {
        return RunCloseCurrentAndMiddleFullscreenTest();
    }

    if (test_name == TEST_DUPLICATE_OPEN) {
        return RunDuplicateOpenMovesExistingFullscreenTest();
    }

    if (test_name == TEST_REJECTS_STATUS) {
        return RunRejectsMissingNonFullscreenAndBackEmptyStatusTest();
    }

    if (test_name == TEST_RELEASE_RESTORES) {
        return RunReleaseRestoresPreviousFullscreenTest();
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
