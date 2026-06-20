// 模块: Tests UiRuntime
// 文件: Tests/UiRuntime/UiPanelParameterTests.cpp

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
#include "YuEngine/UiRuntime/UiManagerFullscreenStackResult.h"
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
#include "YuEngine/UiRuntime/UiManagerPopupStack.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackResult.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackStatus.h"
#include "YuEngine/UiRuntime/UiPanelId.h"
#include "YuEngine/UiRuntime/UiPanelManifestRecord.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgs.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgsSnapshot.h"
#include "YuEngine/UiRuntime/UiPanelRegistry.h"
#include "YuEngine/UiRuntime/UiPanelRegistryResult.h"

using yuengine::uiruntime::BaseUiController;
using yuengine::uiruntime::BaseUiLifecycleSnapshot;
using yuengine::uiruntime::BaseUiLifecycleStatus;
using yuengine::uiruntime::UiManagerFullscreenStack;
using yuengine::uiruntime::UiManagerFullscreenStackResult;
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
using yuengine::uiruntime::UiManagerPopupStack;
using yuengine::uiruntime::UiManagerPopupStackResult;
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

namespace {
constexpr const char *TEST_OPEN_ARGS_REACH_CONTROLLER =
    "UiRuntime_PanelParameter_OpenArgsReachController";
constexpr const char *TEST_EMPTY_AND_INVALID_ARGS =
    "UiRuntime_PanelParameter_EmptyArgsAndInvalidArgsAreStable";
constexpr const char *TEST_REOPEN_UPDATES_ARGS =
    "UiRuntime_PanelParameter_ReopenLoadedUpdatesArgs";
constexpr const char *TEST_STACK_PASS_THROUGH =
    "UiRuntime_PanelParameter_PopupAndFullscreenPassArgs";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiPanelOpenArgsSnapshot CopyOpenArgs(const UiPanelOpenArgs &open_args) {
    UiPanelOpenArgsSnapshot snapshot{};
    snapshot.request_key = open_args.request_key;
    snapshot.value_count = open_args.value_count;
    snapshot.has_args = open_args.HasArgs();
    for (std::uint32_t index = 0U; index < open_args.value_count; ++index) {
        snapshot.values[index] = open_args.values[index];
    }

    return snapshot;
}

class CapturingPanelController final : public BaseUiController {
public:
    const UiPanelOpenArgsSnapshot &LastOpenArgs() const {
        return last_open_args_;
    }

    std::uint32_t OpenWithArgsCount() const {
        return open_with_args_count_;
    }

protected:
    BaseUiLifecycleStatus OnInitEvent() override {
        return BaseUiLifecycleStatus::Success;
    }

    BaseUiLifecycleStatus OnBindEvent() override {
        return BaseUiLifecycleStatus::Success;
    }

    BaseUiLifecycleStatus OnOpenWithArgsEvent(const UiPanelOpenArgs &open_args) override {
        last_open_args_ = CopyOpenArgs(open_args);
        ++open_with_args_count_;
        return BaseUiLifecycleStatus::Success;
    }

    BaseUiLifecycleStatus OnCloseEvent() override {
        return BaseUiLifecycleStatus::Success;
    }

    BaseUiLifecycleStatus OnClearEvent() override {
        return BaseUiLifecycleStatus::Success;
    }

private:
    UiPanelOpenArgsSnapshot last_open_args_;
    std::uint32_t open_with_args_count_ = 0U;
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
    record.resource_refs[0U] = ResourceRef(resource_key, 700U);
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

int SetupSingleLayer(
    UiPanelRegistry *registry,
    UiManagerLayerModel *layer_model,
    std::uint32_t layer_id,
    UiManagerLayerType layer_type,
    std::span<const std::uint32_t> panel_ids) {
    const std::int32_t layer_order = 30;
    const std::uint32_t root_key = 800U + layer_id;
    if (RegisterLayer(layer_model, layer_id, layer_type, layer_order, root_key) != 0) {
        return 1;
    }

    for (std::uint32_t panel_id : panel_ids) {
        if (RegisterPanelAndBind(registry, layer_model, panel_id, layer_id) != 0) {
            return 1;
        }
    }

    return 0;
}

int RunOpenArgsReachControllerTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 1U> panel_ids{1101U};
    const std::span<const std::uint32_t> panel_span(panel_ids.data(), panel_ids.size());
    if (SetupSingleLayer(&registry, &layer_model, 31U, UiManagerLayerType::Popup, panel_span) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    CapturingPanelController controller;
    std::array<std::uint32_t, 3U> input_values{7U, 11U, 13U};
    const std::span<const std::uint32_t> input_span(input_values.data(), input_values.size());
    const UiPanelOpenArgs open_args = OpenArgs(4101U, input_span);
    UiManagerPanelMapResult result =
        panel_map.OpenPanelWithArgs(PanelId(1101U), registry, layer_model, &controller, open_args);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "open with args failed") != 0) {
        return 1;
    }

    input_values[0U] = 99U;
    const std::array<std::uint32_t, 3U> expected_values{7U, 11U, 13U};
    const std::span<const std::uint32_t> expected_span(expected_values.data(), expected_values.size());
    if (RequireArgsSnapshot(controller.LastOpenArgs(), 4101U, expected_span, "controller args mismatch") != 0) {
        return 1;
    }

    if (RequireArgsSnapshot(result.record.open_args, 4101U, expected_span, "record args mismatch") != 0) {
        return 1;
    }

    if (RequireArgsSnapshot(result.open_args, 4101U, expected_span, "result args mismatch") != 0) {
        return 1;
    }

    const UiManagerPanelMapSnapshot snapshot = panel_map.Snapshot();
    if (snapshot.parameter_open_count != 1U ||
        snapshot.empty_open_args_count != 0U ||
        snapshot.reopen_open_args_update_count != 0U) {
        return Fail("parameter counters mismatch");
    }

    return RequireArgsSnapshot(snapshot.last_open_args, 4101U, expected_span, "last args snapshot mismatch");
}

int RunEmptyArgsAndInvalidArgsAreStableTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 2U> panel_ids{1201U, 1202U};
    const std::span<const std::uint32_t> panel_span(panel_ids.data(), panel_ids.size());
    if (SetupSingleLayer(&registry, &layer_model, 32U, UiManagerLayerType::Popup, panel_span) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    CapturingPanelController controller;
    UiManagerPanelMapResult result =
        panel_map.OpenPanel(PanelId(1201U), registry, layer_model, &controller);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "empty args open failed") != 0) {
        return 1;
    }

    if (RequireEmptyArgsSnapshot(controller.LastOpenArgs(), "controller empty args mismatch") != 0) {
        return 1;
    }

    if (RequireEmptyArgsSnapshot(result.open_args, "result empty args mismatch") != 0) {
        return 1;
    }

    CapturingPanelController invalid_controller;
    UiPanelOpenArgs invalid_args{};
    invalid_args.request_key = 5101U;
    invalid_args.value_count = 1U;
    result = panel_map.OpenPanelWithArgs(PanelId(1202U), registry, layer_model, &invalid_controller, invalid_args);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::InvalidOpenArgs, "invalid args status mismatch") != 0) {
        return 1;
    }

    if (invalid_controller.OpenWithArgsCount() != 0U) {
        return Fail("invalid args touched controller");
    }

    std::array<std::uint32_t, 1U> missing_values{17U};
    const std::span<const std::uint32_t> missing_span(missing_values.data(), missing_values.size());
    const UiPanelOpenArgs missing_args = OpenArgs(5102U, missing_span);
    CapturingPanelController missing_controller;
    result = panel_map.OpenPanelWithArgs(PanelId(1299U), registry, layer_model, &missing_controller, missing_args);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::PanelNotRegistered, "missing panel status mismatch") != 0) {
        return 1;
    }

    if (missing_controller.OpenWithArgsCount() != 0U) {
        return Fail("missing panel touched controller");
    }

    const UiManagerPanelMapSnapshot snapshot = panel_map.Snapshot();
    if (snapshot.loaded_panel_count != 1U ||
        snapshot.active_panel_count != 1U ||
        snapshot.empty_open_args_count != 1U ||
        snapshot.rejected_operation_count != 2U) {
        return Fail("empty invalid snapshot mismatch");
    }

    return RequireEmptyArgsSnapshot(snapshot.last_open_args, "last empty args mismatch");
}

int RunReopenLoadedUpdatesArgsTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 1U> panel_ids{1301U};
    const std::span<const std::uint32_t> panel_span(panel_ids.data(), panel_ids.size());
    if (SetupSingleLayer(&registry, &layer_model, 33U, UiManagerLayerType::Popup, panel_span) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    CapturingPanelController controller;
    std::array<std::uint32_t, 1U> first_values{21U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = OpenArgs(6101U, first_span);
    UiManagerPanelMapResult result =
        panel_map.OpenPanelWithArgs(PanelId(1301U), registry, layer_model, &controller, first_args);
    if (!result.Succeeded()) {
        return Fail("first args open failed");
    }

    result = panel_map.ClosePanel(PanelId(1301U));
    if (!result.Succeeded()) {
        return Fail("close before reopen failed");
    }

    std::array<std::uint32_t, 2U> second_values{34U, 55U};
    const std::span<const std::uint32_t> second_span(second_values.data(), second_values.size());
    const UiPanelOpenArgs second_args = OpenArgs(6102U, second_span);
    result = panel_map.OpenPanelWithArgs(PanelId(1301U), registry, layer_model, nullptr, second_args);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "reopen with args failed") != 0) {
        return 1;
    }

    if (!result.reused_loaded || result.already_active) {
        return Fail("reopen flags mismatch");
    }

    if (controller.OpenWithArgsCount() != 2U) {
        return Fail("reopen did not call loaded controller");
    }

    if (RequireArgsSnapshot(controller.LastOpenArgs(), 6102U, second_span, "reopen controller args mismatch") != 0) {
        return 1;
    }

    CapturingPanelController unused_controller;
    std::array<std::uint32_t, 1U> third_values{89U};
    const std::span<const std::uint32_t> third_span(third_values.data(), third_values.size());
    const UiPanelOpenArgs third_args = OpenArgs(6103U, third_span);
    result = panel_map.OpenPanelWithArgs(PanelId(1301U), registry, layer_model, &unused_controller, third_args);
    if (RequireStatus(result.status, UiManagerPanelMapStatus::Success, "duplicate active open failed") != 0) {
        return 1;
    }

    if (!result.already_active || unused_controller.OpenWithArgsCount() != 0U) {
        return Fail("duplicate active open touched replacement controller");
    }

    if (RequireArgsSnapshot(result.record.open_args, 6102U, second_span, "duplicate active args changed") != 0) {
        return 1;
    }

    const UiManagerPanelMapSnapshot snapshot = panel_map.Snapshot();
    if (snapshot.parameter_open_count != 2U ||
        snapshot.reopen_open_args_update_count != 1U ||
        snapshot.idempotent_open_count != 1U) {
        return Fail("reopen counters mismatch");
    }

    return RequireArgsSnapshot(snapshot.last_open_args, 6102U, second_span, "reopen last args mismatch");
}

int RunPopupAndFullscreenPassArgsTest() {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    const std::array<std::uint32_t, 1U> popup_ids{1401U};
    const std::span<const std::uint32_t> popup_span(popup_ids.data(), popup_ids.size());
    if (SetupSingleLayer(&registry, &layer_model, 34U, UiManagerLayerType::Popup, popup_span) != 0) {
        return 1;
    }

    const std::array<std::uint32_t, 2U> fullscreen_ids{1501U, 1502U};
    const std::span<const std::uint32_t> fullscreen_span(fullscreen_ids.data(), fullscreen_ids.size());
    if (SetupSingleLayer(&registry, &layer_model, 35U, UiManagerLayerType::Fullscreen, fullscreen_span) != 0) {
        return 1;
    }

    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    CapturingPanelController popup_controller;
    std::array<std::uint32_t, 2U> popup_values{3U, 5U};
    const std::span<const std::uint32_t> popup_value_span(popup_values.data(), popup_values.size());
    const UiPanelOpenArgs popup_args = OpenArgs(7101U, popup_value_span);
    const UiManagerPopupStackResult popup_result =
        popup_stack.OpenPopupPanelWithArgs(PanelId(1401U), registry, layer_model, &panel_map, &popup_controller, popup_args);
    if (popup_result.status != UiManagerPopupStackStatus::Success) {
        return Fail("popup args open failed");
    }

    if (RequireArgsSnapshot(popup_result.record.open_args, 7101U, popup_value_span, "popup result args mismatch") != 0) {
        return 1;
    }

    if (RequireArgsSnapshot(popup_controller.LastOpenArgs(), 7101U, popup_value_span, "popup controller args mismatch") != 0) {
        return 1;
    }

    UiManagerFullscreenStack fullscreen_stack;
    CapturingPanelController first_fullscreen_controller;
    CapturingPanelController second_fullscreen_controller;
    std::array<std::uint32_t, 1U> first_values{8U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = OpenArgs(7201U, first_span);
    UiManagerFullscreenStackResult fullscreen_result =
        fullscreen_stack.OpenFullscreenPanelWithArgs(
            PanelId(1501U),
            registry,
            layer_model,
            &panel_map,
            &first_fullscreen_controller,
            first_args);
    if (fullscreen_result.status != UiManagerFullscreenStackStatus::Success) {
        return Fail("first fullscreen args open failed");
    }

    std::array<std::uint32_t, 2U> second_values{13U, 21U};
    const std::span<const std::uint32_t> second_span(second_values.data(), second_values.size());
    const UiPanelOpenArgs second_args = OpenArgs(7202U, second_span);
    fullscreen_result =
        fullscreen_stack.OpenFullscreenPanelWithArgs(
            PanelId(1502U),
            registry,
            layer_model,
            &panel_map,
            &second_fullscreen_controller,
            second_args);
    if (fullscreen_result.status != UiManagerFullscreenStackStatus::Success) {
        return Fail("second fullscreen args open failed");
    }

    fullscreen_result = fullscreen_stack.NavigateBack(registry, layer_model, &panel_map);
    if (fullscreen_result.status != UiManagerFullscreenStackStatus::Success) {
        return Fail("fullscreen navigate back failed");
    }

    if (!fullscreen_result.restored_previous || fullscreen_result.restored_panel_id.value != 1501U) {
        return Fail("fullscreen restore flags mismatch");
    }

    const BaseUiLifecycleSnapshot first_snapshot = first_fullscreen_controller.Snapshot();
    if (first_snapshot.open_count != 2U || first_fullscreen_controller.OpenWithArgsCount() != 2U) {
        return Fail("fullscreen restore did not reopen first controller");
    }

    if (RequireArgsSnapshot(first_fullscreen_controller.LastOpenArgs(), 7201U, first_span, "fullscreen restore args mismatch") != 0) {
        return 1;
    }

    if (RequireArgsSnapshot(fullscreen_result.record.open_args, 7201U, first_span, "fullscreen result args mismatch") != 0) {
        return 1;
    }

    return 0;
}

int RunNamedTest(std::string_view test_name) {
    if (test_name == TEST_OPEN_ARGS_REACH_CONTROLLER) {
        return RunOpenArgsReachControllerTest();
    }

    if (test_name == TEST_EMPTY_AND_INVALID_ARGS) {
        return RunEmptyArgsAndInvalidArgsAreStableTest();
    }

    if (test_name == TEST_REOPEN_UPDATES_ARGS) {
        return RunReopenLoadedUpdatesArgsTest();
    }

    if (test_name == TEST_STACK_PASS_THROUGH) {
        return RunPopupAndFullscreenPassArgsTest();
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
