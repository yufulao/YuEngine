// 模块: Tests UiRuntime
// 文件: Tests/UiRuntime/UiManagerLayerModelTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/UiRuntime/UiManagerLayerId.h"
#include "YuEngine/UiRuntime/UiManagerLayerModel.h"
#include "YuEngine/UiRuntime/UiManagerLayerModelDesc.h"
#include "YuEngine/UiRuntime/UiManagerLayerModelResult.h"
#include "YuEngine/UiRuntime/UiManagerLayerModelSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerLayerModelStatus.h"
#include "YuEngine/UiRuntime/UiManagerLayerRecord.h"
#include "YuEngine/UiRuntime/UiManagerLayerType.h"
#include "YuEngine/UiRuntime/UiManagerPanelLayerBinding.h"
#include "YuEngine/UiRuntime/UiPanelId.h"

using yuengine::uiruntime::UiManagerLayerId;
using yuengine::uiruntime::UiManagerLayerModel;
using yuengine::uiruntime::UiManagerLayerModelDesc;
using yuengine::uiruntime::UiManagerLayerModelResult;
using yuengine::uiruntime::UiManagerLayerModelSnapshot;
using yuengine::uiruntime::UiManagerLayerModelStatus;
using yuengine::uiruntime::UiManagerLayerRecord;
using yuengine::uiruntime::UiManagerLayerRootRef;
using yuengine::uiruntime::UiManagerLayerSet;
using yuengine::uiruntime::UiManagerLayerType;
using yuengine::uiruntime::UiManagerPanelLayerBinding;
using yuengine::uiruntime::UiPanelId;

namespace {
constexpr const char *TEST_LAYER_ROOTS =
    "UiRuntime_ManagerLayerModel_RegistersFiveLayerRoots";
constexpr const char *TEST_DUPLICATE_MISSING =
    "UiRuntime_ManagerLayerModel_RejectsDuplicateAndMissingLayers";
constexpr const char *TEST_STABLE_ORDER =
    "UiRuntime_ManagerLayerModel_ExportsLayersInStableOrder";
constexpr const char *TEST_PANEL_RESOLVE =
    "UiRuntime_ManagerLayerModel_ResolvesPanelIdsToLayerRoots";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiManagerLayerId LayerId(std::uint32_t value) {
    return UiManagerLayerId{value};
}

UiPanelId PanelId(std::uint32_t value) {
    return UiPanelId{value};
}

UiManagerLayerRootRef RootRef(std::uint32_t root_key) {
    UiManagerLayerRootRef ref{};
    ref.root_key = root_key;
    return ref;
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

std::array<UiManagerLayerRecord, 5U> MakeDefaultLayers() {
    std::array<UiManagerLayerRecord, 5U> layers{};
    layers[0U] = LayerRecord(1U, UiManagerLayerType::Game, 10, 100U);
    layers[1U] = LayerRecord(2U, UiManagerLayerType::Fullscreen, 20, 200U);
    layers[2U] = LayerRecord(3U, UiManagerLayerType::Popup, 30, 300U);
    layers[3U] = LayerRecord(4U, UiManagerLayerType::Loading, 40, 400U);
    layers[4U] = LayerRecord(5U, UiManagerLayerType::Debug, 50, 500U);
    return layers;
}

int RequireStatus(
    UiManagerLayerModelStatus actual,
    UiManagerLayerModelStatus expected,
    std::string_view message) {
    if (actual == expected) {
        return 0;
    }

    return Fail(message);
}

int RegisterDefaultLayers(UiManagerLayerModel *model) {
    if (model == nullptr) {
        return Fail("missing layer model");
    }

    const std::array<UiManagerLayerRecord, 5U> layers = MakeDefaultLayers();
    UiManagerLayerSet layer_set{};
    layer_set.records = std::span<const UiManagerLayerRecord>(layers.data(), layers.size());
    const UiManagerLayerModelResult result = model->RegisterLayerSet(layer_set);
    if (!result.Succeeded()) {
        return Fail("default layer set register failed");
    }

    return 0;
}

int RequireLayerByType(
    const UiManagerLayerModel &model,
    UiManagerLayerType type,
    std::uint32_t expected_layer_id,
    std::int32_t expected_order,
    std::uint32_t expected_root_key) {
    UiManagerLayerRecord record{};
    const UiManagerLayerModelStatus status = model.ResolveLayerByType(type, &record);
    if (RequireStatus(status, UiManagerLayerModelStatus::Success, "layer type resolve failed") != 0) {
        return 1;
    }

    if (record.layer_id.value != expected_layer_id) {
        return Fail("layer id mismatch");
    }

    if (record.order != expected_order) {
        return Fail("layer order mismatch");
    }

    if (record.root_ref.root_key != expected_root_key) {
        return Fail("layer root ref mismatch");
    }

    return 0;
}

int RequireLayerType(const UiManagerLayerRecord &record, UiManagerLayerType expected_type) {
    if (record.type == expected_type) {
        return 0;
    }

    return Fail("layer type mismatch");
}

int UiRuntimeManagerLayerModelRegistersFiveLayerRoots() {
    UiManagerLayerModel model;
    int ret_code = RegisterDefaultLayers(&model);
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = RequireLayerByType(model, UiManagerLayerType::Game, 1U, 10, 100U);
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = RequireLayerByType(model, UiManagerLayerType::Fullscreen, 2U, 20, 200U);
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = RequireLayerByType(model, UiManagerLayerType::Popup, 3U, 30, 300U);
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = RequireLayerByType(model, UiManagerLayerType::Loading, 4U, 40, 400U);
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = RequireLayerByType(model, UiManagerLayerType::Debug, 5U, 50, 500U);
    if (ret_code != 0) {
        return ret_code;
    }

    const UiManagerLayerModelSnapshot snapshot = model.Snapshot();
    if (snapshot.registered_layer_count != 5U || snapshot.accepted_operation_count != 1U) {
        return Fail("layer model snapshot mismatch");
    }

    return 0;
}

int UiRuntimeManagerLayerModelRejectsDuplicateAndMissingLayers() {
    UiManagerLayerModel model;
    UiManagerLayerModelResult result =
        model.RegisterLayer(LayerRecord(1U, UiManagerLayerType::Game, 10, 100U));
    if (!result.Succeeded()) {
        return Fail("first layer register failed");
    }

    result = model.RegisterLayer(LayerRecord(1U, UiManagerLayerType::Fullscreen, 20, 200U));
    if (result.status != UiManagerLayerModelStatus::DuplicateLayerId) {
        return Fail("duplicate layer id was not rejected");
    }

    result = model.RegisterLayer(LayerRecord(2U, UiManagerLayerType::Game, 20, 201U));
    if (result.status != UiManagerLayerModelStatus::DuplicateLayerType) {
        return Fail("duplicate layer type was not rejected");
    }

    result = model.RegisterLayer(LayerRecord(3U, UiManagerLayerType::Popup, 30, 0U));
    if (result.status != UiManagerLayerModelStatus::InvalidLayerRoot) {
        return Fail("invalid layer root was not rejected");
    }

    UiManagerLayerRecord resolved{};
    UiManagerLayerModelStatus status = model.ResolveLayerByType(UiManagerLayerType::Popup, &resolved);
    if (RequireStatus(status, UiManagerLayerModelStatus::LayerNotFound, "missing layer status mismatch") != 0) {
        return 1;
    }

    status = model.ResolveLayer(UiManagerLayerId{}, &resolved);
    if (RequireStatus(status, UiManagerLayerModelStatus::InvalidLayerId, "invalid layer id status mismatch") != 0) {
        return 1;
    }

    status = model.ResolveLayer(LayerId(1U), nullptr);
    if (RequireStatus(status, UiManagerLayerModelStatus::InvalidOutputBuffer, "null layer output status mismatch") != 0) {
        return 1;
    }

    result = model.RegisterLayer(LayerRecord(3U, UiManagerLayerType::Popup, 30, 300U));
    if (!result.Succeeded()) {
        return Fail("valid layer register after failure failed");
    }

    const UiManagerLayerModelSnapshot snapshot = model.Snapshot();
    if (snapshot.registered_layer_count != 2U ||
        snapshot.accepted_operation_count != 2U ||
        snapshot.failed_operation_count != 3U ||
        snapshot.duplicate_layer_rejected_count != 2U ||
        snapshot.rejected_operation_count != 3U ||
        snapshot.last_status != UiManagerLayerModelStatus::Success) {
        return Fail("duplicate layer snapshot mismatch");
    }

    return 0;
}

int UiRuntimeManagerLayerModelExportsLayersInStableOrder() {
    UiManagerLayerModel model;
    if (!model.RegisterLayer(LayerRecord(3U, UiManagerLayerType::Popup, 30, 300U)).Succeeded()) {
        return Fail("popup layer register failed");
    }

    if (!model.RegisterLayer(LayerRecord(1U, UiManagerLayerType::Game, 10, 100U)).Succeeded()) {
        return Fail("game layer register failed");
    }

    if (!model.RegisterLayer(LayerRecord(5U, UiManagerLayerType::Debug, 50, 500U)).Succeeded()) {
        return Fail("debug layer register failed");
    }

    if (!model.RegisterLayer(LayerRecord(2U, UiManagerLayerType::Fullscreen, 20, 200U)).Succeeded()) {
        return Fail("fullscreen layer register failed");
    }

    if (!model.RegisterLayer(LayerRecord(4U, UiManagerLayerType::Loading, 40, 400U)).Succeeded()) {
        return Fail("loading layer register failed");
    }

    std::array<UiManagerLayerRecord, 5U> output_records{};
    UiManagerLayerModelResult result = model.ExportLayers(output_records.data(), 4U);
    if (result.status != UiManagerLayerModelStatus::InvalidOutputBuffer ||
        result.required_layer_count != 5U) {
        return Fail("small layer export status mismatch");
    }

    result = model.ExportLayers(output_records.data(), static_cast<std::uint32_t>(output_records.size()));
    if (!result.Succeeded() || result.required_layer_count != 5U) {
        return Fail("layer export failed");
    }

    if (RequireLayerType(output_records[0U], UiManagerLayerType::Game) != 0) {
        return 1;
    }

    if (RequireLayerType(output_records[1U], UiManagerLayerType::Fullscreen) != 0) {
        return 1;
    }

    if (RequireLayerType(output_records[2U], UiManagerLayerType::Popup) != 0) {
        return 1;
    }

    if (RequireLayerType(output_records[3U], UiManagerLayerType::Loading) != 0) {
        return 1;
    }

    if (RequireLayerType(output_records[4U], UiManagerLayerType::Debug) != 0) {
        return 1;
    }

    output_records[0U].root_ref.root_key = 9999U;
    UiManagerLayerRecord resolved{};
    const UiManagerLayerModelStatus status = model.ResolveLayerByType(UiManagerLayerType::Game, &resolved);
    if (RequireStatus(status, UiManagerLayerModelStatus::Success, "readonly layer resolve failed") != 0) {
        return 1;
    }

    if (resolved.root_ref.root_key != 100U) {
        return Fail("exported layer mutated model storage");
    }

    return 0;
}

int UiRuntimeManagerLayerModelResolvesPanelIdsToLayerRoots() {
    UiManagerLayerModel model;
    int ret_code = RegisterDefaultLayers(&model);
    if (ret_code != 0) {
        return ret_code;
    }

    UiManagerLayerModelResult result = model.BindPanelToLayer(PanelBinding(501U, 3U));
    if (!result.Succeeded()) {
        return Fail("popup panel binding failed");
    }

    result = model.BindPanelToLayer(PanelBinding(502U, 4U));
    if (!result.Succeeded()) {
        return Fail("loading panel binding failed");
    }

    UiManagerLayerRecord resolved{};
    UiManagerLayerModelStatus status = model.ResolvePanelLayer(PanelId(501U), &resolved);
    if (RequireStatus(status, UiManagerLayerModelStatus::Success, "popup panel resolve failed") != 0) {
        return 1;
    }

    if (resolved.type != UiManagerLayerType::Popup || resolved.root_ref.root_key != 300U) {
        return Fail("popup panel layer root mismatch");
    }

    status = model.ResolvePanelLayer(PanelId(502U), &resolved);
    if (RequireStatus(status, UiManagerLayerModelStatus::Success, "loading panel resolve failed") != 0) {
        return 1;
    }

    if (resolved.type != UiManagerLayerType::Loading || resolved.root_ref.root_key != 400U) {
        return Fail("loading panel layer root mismatch");
    }

    result = model.BindPanelToLayer(PanelBinding(501U, 5U));
    if (result.status != UiManagerLayerModelStatus::DuplicatePanelId) {
        return Fail("duplicate panel binding was not rejected");
    }

    result = model.BindPanelToLayer(PanelBinding(503U, 99U));
    if (result.status != UiManagerLayerModelStatus::LayerNotFound) {
        return Fail("missing binding layer was not rejected");
    }

    result = model.BindPanelToLayer(PanelBinding(0U, 5U));
    if (result.status != UiManagerLayerModelStatus::InvalidPanelId) {
        return Fail("invalid binding panel was not rejected");
    }

    status = model.ResolvePanelLayer(PanelId(999U), &resolved);
    if (RequireStatus(status, UiManagerLayerModelStatus::PanelNotBound, "unbound panel status mismatch") != 0) {
        return 1;
    }

    status = model.ResolvePanelLayer(PanelId(501U), nullptr);
    if (RequireStatus(status, UiManagerLayerModelStatus::InvalidOutputBuffer, "null panel output status mismatch") != 0) {
        return 1;
    }

    const UiManagerLayerModelSnapshot snapshot = model.Snapshot();
    if (snapshot.panel_binding_count != 2U ||
        snapshot.duplicate_panel_rejected_count != 1U ||
        snapshot.rejected_operation_count != 3U) {
        return Fail("panel binding snapshot mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view test_name) {
    if (test_name == TEST_LAYER_ROOTS) {
        return UiRuntimeManagerLayerModelRegistersFiveLayerRoots();
    }

    if (test_name == TEST_DUPLICATE_MISSING) {
        return UiRuntimeManagerLayerModelRejectsDuplicateAndMissingLayers();
    }

    if (test_name == TEST_STABLE_ORDER) {
        return UiRuntimeManagerLayerModelExportsLayersInStableOrder();
    }

    if (test_name == TEST_PANEL_RESOLVE) {
        return UiRuntimeManagerLayerModelResolvesPanelIdsToLayerRoots();
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
