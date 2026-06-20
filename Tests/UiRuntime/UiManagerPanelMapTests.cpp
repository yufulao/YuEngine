// 模块: Tests UiRuntime
// 文件: Tests/UiRuntime/UiManagerPanelMapTests.cpp

#include <array>
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
#include "YuEngine/UiRuntime/UiManagerPanelMapDesc.h"
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

using yuengine::uiruntime::BaseUiController;
using yuengine::uiruntime::BaseUiLifecycleSnapshot;
using yuengine::uiruntime::BaseUiLifecycleStatus;
using yuengine::uiruntime::UiManagerLayerId;
using yuengine::uiruntime::UiManagerLayerModel;
using yuengine::uiruntime::UiManagerLayerRecord;
using yuengine::uiruntime::UiManagerLayerRootRef;
using yuengine::uiruntime::UiManagerLayerSet;
using yuengine::uiruntime::UiManagerLayerType;
using yuengine::uiruntime::UiManagerPanelLayerBinding;
using yuengine::uiruntime::UiManagerPanelMap;
using yuengine::uiruntime::UiManagerPanelMapDesc;
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
constexpr const char *TEST_OPEN_CLOSE_REOPEN =
    "UiRuntime_ManagerPanelMap_OpenCloseReopenRetainsLoadedController";
constexpr const char *TEST_REJECTS_MISSING =
    "UiRuntime_ManagerPanelMap_RejectsMissingPanelLayerAndController";
constexpr const char *TEST_DUPLICATE_IDEMPOTENT =
    "UiRuntime_ManagerPanelMap_DuplicateOpenCloseAreIdempotent";
constexpr const char *TEST_RESOLVE_ROOTS =
    "UiRuntime_ManagerPanelMap_ResolvesLoadedAndActiveLayerRoots";
constexpr const char *TEST_RELEASE_CLEARS =
    "UiRuntime_ManagerPanelMap_ReleaseClearsLoadedControllerAndArgs";
constexpr const char *TEST_RELEASE_ACTIVE_REOPEN =
    "UiRuntime_ManagerPanelMap_ReleaseActiveAndReopenUsesNewLifecycle";
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

UiPanelOpenArgs OpenArgs(std::uint32_t request_key, std::span<const std::uint32_t> values) {
    UiPanelOpenArgs args{};
    args.request_key = request_key;
    args.values = values.data();
    args.value_count = static_cast<std::uint32_t>(values.size());
    return args;
}

int RequireStatus(
    UiManagerPanelMapStatus actual,
    UiManagerPanelMapStatus expected,
    std::string_view message) {
    if (actual == expected) {
        return 0;
    }

    return Fail(message);
}

int RegisterPanelAndLayer(
    UiPanelRegistry *registry,
    UiManagerLayerModel *layer_model,
    std::uint32_t panel_id,
    std::uint32_t layer_id,
    std::uint32_t root_key) {
    if (registry == nullptr || layer_model == nullptr) {
        return Fail("missing model");
    }

    const UiPanelRegistryResult panel_result =
        registry->RegisterPanel(PanelRecord(panel_id, 100U, 200U, 300U));
    if (!panel_result.Succeeded()) {
        return Fail("panel register failed");
    }

    std::array<UiManagerLayerRecord, 1U> layers{};
    layers[0U] = LayerRecord(layer_id, UiManagerLayerType::Popup, 30, root_key);
    UiManagerLayerSet layer_set{};
    layer_set.records = std::span<const UiManagerLayerRecord>(layers.data(), layers.size());
    if (!layer_model->RegisterLayerSet(layer_set).Succeeded()) {
        return Fail("layer register failed");
    }

    if (!layer_model->BindPanelToLayer(PanelBinding(panel_id, layer_id)).Succeeded()) {
        return Fail("panel layer binding failed");
    }

    return 0;
}

int RequireSnapshotCounts(
    const UiManagerPanelMapSnapshot &snapshot,
    std::uint32_t loaded_count,
    std::uint32_t active_count,
    std::string_view message) {
    if (snapshot.loaded_panel_count == loaded_count &&
        snapshot.active_panel_count == active_count) {
        return 0;
    }

    return Fail(message);
}

int RequireEmptyOpenArgs(
    const UiPanelOpenArgsSnapshot &snapshot,
    std::string_view message) {
    if (snapshot.request_key != 0U || snapshot.value_count != 0U || snapshot.has_args) {
        return Fail(message);
    }

    return 0;
}

int RunOpenCloseReopenRetainsLoadedControllerTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (RegisterPanelAndLayer(&registry, &layer_model, 501U, 11U, 901U) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    TestPanelController controller;
    UiManagerPanelMapResult result =
        panel_map.OpenPanel(PanelId(501U), registry, layer_model, &controller);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "first open failed") != 0) {
        return 1;
    }

    if (!result.record.loaded || !result.record.active || result.record.layer_record.root_ref.root_key != 901U) {
        return Fail("first open record mismatch");
    }

    result = panel_map.ClosePanel(PanelId(501U));
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "close failed") != 0) {
        return 1;
    }

    if (!result.record.loaded || result.record.active) {
        return Fail("close did not retain loaded record");
    }

    result = panel_map.OpenPanel(PanelId(501U), registry, layer_model, nullptr);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "reopen failed") != 0) {
        return 1;
    }

    if (!result.reused_loaded || result.record.controller != &controller) {
        return Fail("reopen did not reuse loaded controller");
    }

    const BaseUiLifecycleSnapshot controller_snapshot = controller.Snapshot();
    if (controller_snapshot.initialize_count != 1U ||
        controller_snapshot.bind_event_count != 1U ||
        controller_snapshot.open_count != 2U ||
        controller_snapshot.close_count != 1U) {
        return Fail("controller lifecycle snapshot mismatch");
    }

    return RequireSnapshotCounts(panel_map.Snapshot(), 1U, 1U, "panel map final counts mismatch");
}

int RunRejectsMissingPanelLayerAndControllerTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    UiManagerPanelMap panel_map;
    TestPanelController controller;
    UiManagerPanelMapResult result =
        panel_map.OpenPanel(PanelId(601U), registry, layer_model, &controller);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::PanelNotRegistered, "missing panel status mismatch") != 0) {
        return 1;
    }

    const UiPanelRegistryResult panel_result =
        registry.RegisterPanel(PanelRecord(602U, 101U, 201U, 301U));
    if (!panel_result.Succeeded()) {
        return Fail("panel register failed");
    }

    result = panel_map.OpenPanel(PanelId(602U), registry, layer_model, &controller);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::PanelLayerNotBound, "missing layer status mismatch") != 0) {
        return 1;
    }

    std::array<UiManagerLayerRecord, 1U> layers{};
    layers[0U] = LayerRecord(12U, UiManagerLayerType::Popup, 30, 902U);
    UiManagerLayerSet layer_set{};
    layer_set.records = std::span<const UiManagerLayerRecord>(layers.data(), layers.size());
    if (!layer_model.RegisterLayerSet(layer_set).Succeeded()) {
        return Fail("layer register failed");
    }

    if (!layer_model.BindPanelToLayer(PanelBinding(602U, 12U)).Succeeded()) {
        return Fail("panel binding failed");
    }

    result = panel_map.OpenPanel(PanelId(602U), registry, layer_model, nullptr);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::InvalidController, "null controller status mismatch") != 0) {
        return 1;
    }

    result = panel_map.OpenPanel(UiPanelId{}, registry, layer_model, &controller);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::InvalidPanelId, "invalid panel status mismatch") != 0) {
        return 1;
    }

    result = panel_map.ClosePanel(PanelId(999U));
    return RequireStatus(result.status, UiManagerPanelMapStatus::PanelNotLoaded, "missing loaded status mismatch");
}

int RunDuplicateOpenCloseAreIdempotentTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (RegisterPanelAndLayer(&registry, &layer_model, 701U, 13U, 903U) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    TestPanelController controller;
    TestPanelController unused_controller;
    UiManagerPanelMapResult result =
        panel_map.OpenPanel(PanelId(701U), registry, layer_model, &controller);
    if (!result.Succeeded()) {
        return Fail("first open failed");
    }

    result = panel_map.OpenPanel(PanelId(701U), registry, layer_model, &unused_controller);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "duplicate open failed") != 0) {
        return 1;
    }

    if (!result.already_active || result.record.controller != &controller) {
        return Fail("duplicate open did not keep active controller");
    }

    if (unused_controller.Snapshot().open_count != 0U) {
        return Fail("duplicate open touched replacement controller");
    }

    result = panel_map.ClosePanel(PanelId(701U));
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "first close failed") != 0) {
        return 1;
    }

    result = panel_map.ClosePanel(PanelId(701U));
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "duplicate close failed") != 0) {
        return 1;
    }

    if (!result.already_inactive || result.record.active) {
        return Fail("duplicate close did not keep loaded inactive record");
    }

    const BaseUiLifecycleSnapshot controller_snapshot = controller.Snapshot();
    if (controller_snapshot.open_count != 1U || controller_snapshot.close_count != 1U) {
        return Fail("idempotent path lifecycle mismatch");
    }

    const UiManagerPanelMapSnapshot snapshot = panel_map.Snapshot();
    if (snapshot.idempotent_open_count != 1U || snapshot.idempotent_close_count != 1U) {
        return Fail("idempotent snapshot mismatch");
    }

    return RequireSnapshotCounts(snapshot, 1U, 0U, "idempotent final counts mismatch");
}

int RunResolvesLoadedAndActiveLayerRootsTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (RegisterPanelAndLayer(&registry, &layer_model, 801U, 14U, 904U) != 0) {
        return 1;
    }

    UiManagerPanelMapDesc desc{};
    desc.panel_capacity = 4U;
    UiManagerPanelMap panel_map(desc);
    TestPanelController controller;
    if (!panel_map.OpenPanel(PanelId(801U), registry, layer_model, &controller).Succeeded()) {
        return Fail("open failed");
    }

    UiManagerPanelMapRecord loaded_record{};
    UiManagerPanelMapStatus status = panel_map.ResolveLoadedPanel(PanelId(801U), &loaded_record);
    if (RequireStatus(status, UiManagerPanelMapStatus::Success, "loaded resolve failed") != 0) {
        return 1;
    }

    if (loaded_record.layer_record.root_ref.root_key != 904U ||
        loaded_record.manifest_record.layout_ref.layout_asset_key != 100U ||
        loaded_record.manifest_record.controller_ref.controller_type_key != 200U) {
        return Fail("loaded resolve record mismatch");
    }

    UiManagerPanelMapRecord active_record{};
    status = panel_map.ResolveActivePanel(PanelId(801U), &active_record);
    if (RequireStatus(status, UiManagerPanelMapStatus::Success, "active resolve failed") != 0) {
        return 1;
    }

    if (!active_record.active || active_record.layer_record.root_ref.root_key != 904U) {
        return Fail("active resolve record mismatch");
    }

    if (!panel_map.ClosePanel(PanelId(801U)).Succeeded()) {
        return Fail("close failed");
    }

    status = panel_map.ResolveLoadedPanel(PanelId(801U), &loaded_record);
    if (RequireStatus(status, UiManagerPanelMapStatus::Success, "loaded resolve after close failed") != 0) {
        return 1;
    }

    status = panel_map.ResolveActivePanel(PanelId(801U), &active_record);
    return RequireStatus(status, UiManagerPanelMapStatus::PanelNotActive, "inactive active resolve status mismatch");
}

int RunReleaseClearsLoadedControllerAndArgsTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (RegisterPanelAndLayer(&registry, &layer_model, 811U, 15U, 905U) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    TestPanelController controller;
    std::array<std::uint32_t, 2U> values{31U, 37U};
    const std::span<const std::uint32_t> value_span(values.data(), values.size());
    const UiPanelOpenArgs open_args = OpenArgs(9101U, value_span);
    UiManagerPanelMapResult result =
        panel_map.OpenPanelWithArgs(PanelId(811U), registry, layer_model, &controller, open_args);
    if (!result.Succeeded()) {
        return Fail("open with args failed");
    }

    result = panel_map.ClosePanel(PanelId(811U));
    if (!result.Succeeded()) {
        return Fail("close before release failed");
    }

    result = panel_map.ReleasePanel(PanelId(811U));
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "release failed") != 0) {
        return 1;
    }

    if (!result.released_loaded || result.released_active || !result.already_inactive) {
        return Fail("release result flags mismatch");
    }

    if (result.record.loaded || result.record.active || result.record.controller != nullptr) {
        return Fail("release result did not clear record");
    }

    if (RequireEmptyOpenArgs(result.open_args, "release result args not cleared") != 0) {
        return 1;
    }

    UiManagerPanelMapRecord loaded_record{};
    const UiManagerPanelMapStatus loaded_status =
        panel_map.ResolveLoadedPanel(PanelId(811U), &loaded_record);
    if (RequireStatus(loaded_status, UiManagerPanelMapStatus::PanelNotLoaded, "released panel still loaded") != 0) {
        return 1;
    }

    const BaseUiLifecycleSnapshot controller_snapshot = controller.Snapshot();
    if (controller_snapshot.open_count != 1U ||
        controller_snapshot.close_count != 1U ||
        controller_snapshot.clear_count != 1U ||
        !controller_snapshot.destroyed) {
        return Fail("release lifecycle snapshot mismatch");
    }

    const UiManagerPanelMapSnapshot snapshot = panel_map.Snapshot();
    if (snapshot.release_operation_count != 1U ||
        snapshot.release_active_operation_count != 0U ||
        snapshot.loaded_panel_count != 0U ||
        snapshot.active_panel_count != 0U) {
        return Fail("release snapshot counters mismatch");
    }

    return 0;
}

int RunReleaseActiveAndReopenUsesNewLifecycleTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    if (RegisterPanelAndLayer(&registry, &layer_model, 812U, 16U, 906U) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    TestPanelController first_controller;
    UiManagerPanelMapResult result =
        panel_map.OpenPanel(PanelId(812U), registry, layer_model, &first_controller);
    if (!result.Succeeded()) {
        return Fail("first open failed");
    }

    result = panel_map.ReleasePanel(PanelId(812U));
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "active release failed") != 0) {
        return 1;
    }

    if (!result.released_loaded || !result.released_active || result.already_inactive) {
        return Fail("active release flags mismatch");
    }

    BaseUiLifecycleSnapshot controller_snapshot = first_controller.Snapshot();
    if (controller_snapshot.open_count != 1U ||
        controller_snapshot.close_count != 1U ||
        controller_snapshot.clear_count != 1U ||
        !controller_snapshot.destroyed) {
        return Fail("active release lifecycle mismatch");
    }

    result = panel_map.ReleasePanel(PanelId(812U));
    if (RequireStatus(result.status, UiManagerPanelMapStatus::PanelNotLoaded, "duplicate release status mismatch") != 0) {
        return 1;
    }

    TestPanelController second_controller;
    result = panel_map.OpenPanel(PanelId(812U), registry, layer_model, &second_controller);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "reopen after release failed") != 0) {
        return 1;
    }

    if (result.reused_loaded || result.record.controller != &second_controller) {
        return Fail("reopen after release reused old controller");
    }

    controller_snapshot = first_controller.Snapshot();
    if (controller_snapshot.open_count != 1U || controller_snapshot.clear_count != 1U) {
        return Fail("old controller lifecycle changed after reopen");
    }

    const BaseUiLifecycleSnapshot second_snapshot = second_controller.Snapshot();
    if (second_snapshot.open_count != 1U ||
        second_snapshot.clear_count != 0U ||
        !second_snapshot.open) {
        return Fail("new controller lifecycle mismatch");
    }

    const UiManagerPanelMapSnapshot snapshot = panel_map.Snapshot();
    if (snapshot.release_operation_count != 1U ||
        snapshot.release_active_operation_count != 1U ||
        snapshot.loaded_panel_count != 1U ||
        snapshot.active_panel_count != 1U ||
        snapshot.rejected_operation_count != 1U) {
        return Fail("active release counters mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view test_name) {
    if (test_name == TEST_OPEN_CLOSE_REOPEN) {
        return RunOpenCloseReopenRetainsLoadedControllerTest();
    }

    if (test_name == TEST_REJECTS_MISSING) {
        return RunRejectsMissingPanelLayerAndControllerTest();
    }

    if (test_name == TEST_DUPLICATE_IDEMPOTENT) {
        return RunDuplicateOpenCloseAreIdempotentTest();
    }

    if (test_name == TEST_RESOLVE_ROOTS) {
        return RunResolvesLoadedAndActiveLayerRootsTest();
    }

    if (test_name == TEST_RELEASE_CLEARS) {
        return RunReleaseClearsLoadedControllerAndArgsTest();
    }

    if (test_name == TEST_RELEASE_ACTIVE_REOPEN) {
        return RunReleaseActiveAndReopenUsesNewLifecycleTest();
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
