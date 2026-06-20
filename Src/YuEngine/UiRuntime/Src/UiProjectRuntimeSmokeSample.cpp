// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Src/UiProjectRuntimeSmokeSample.cpp

#include "YuEngine/UiRuntime/UiProjectRuntimeSmokeSample.h"

#include <array>
#include <cstdint>
#include <span>

#include "YuEngine/UiRuntime/UiFullscreenRepresentativeController.h"
#include "YuEngine/UiRuntime/UiFullscreenRepresentativeSnapshot.h"
#include "YuEngine/UiRuntime/UiGridViewRepresentativeController.h"
#include "YuEngine/UiRuntime/UiGridViewRepresentativeSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStack.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStackResult.h"
#include "YuEngine/UiRuntime/UiManagerLayerId.h"
#include "YuEngine/UiRuntime/UiManagerLayerModel.h"
#include "YuEngine/UiRuntime/UiManagerLayerRecord.h"
#include "YuEngine/UiRuntime/UiManagerLayerType.h"
#include "YuEngine/UiRuntime/UiManagerPanelLayerBinding.h"
#include "YuEngine/UiRuntime/UiManagerPanelMap.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapResult.h"
#include "YuEngine/UiRuntime/UiManagerPopupStack.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackResult.h"
#include "YuEngine/UiRuntime/UiPanelId.h"
#include "YuEngine/UiRuntime/UiPanelManifestRecord.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgs.h"
#include "YuEngine/UiRuntime/UiPanelRegistry.h"
#include "YuEngine/UiRuntime/UiSimplePopupToastRepresentativeController.h"
#include "YuEngine/UiRuntime/UiSimplePopupToastRepresentativeSnapshot.h"

namespace yuengine::uiruntime {
namespace {
constexpr std::uint32_t POPUP_PANEL_ID = 7201U;
constexpr std::uint32_t FULLSCREEN_FIRST_PANEL_ID = 7202U;
constexpr std::uint32_t FULLSCREEN_SECOND_PANEL_ID = 7203U;
constexpr std::uint32_t GRID_PANEL_ID = 7204U;
constexpr std::uint32_t POPUP_LAYER_ID = 7301U;
constexpr std::uint32_t FULLSCREEN_LAYER_ID = 7302U;
constexpr std::uint32_t POPUP_ROOT_KEY = 7401U;
constexpr std::uint32_t FULLSCREEN_ROOT_KEY = 7402U;

struct UiProjectRuntimeSmokeContext final {
    UiPanelRegistry registry;
    UiManagerLayerModel layer_model;
    UiManagerPanelMap panel_map;
    UiManagerPopupStack popup_stack;
    UiManagerFullscreenStack fullscreen_stack;
    UiSimplePopupToastRepresentativeController popup_controller;
    UiFullscreenRepresentativeController fullscreen_first_controller;
    UiFullscreenRepresentativeController fullscreen_second_controller;
    UiGridViewRepresentativeController grid_controller;
};

UiPanelId MakePanelId(std::uint32_t value) {
    return UiPanelId{value};
}

UiManagerLayerId MakeLayerId(std::uint32_t value) {
    return UiManagerLayerId{value};
}

UiPanelLayoutRef MakeLayoutRef(std::uint32_t asset_key, std::uint32_t variant_key) {
    UiPanelLayoutRef ref{};
    ref.layout_asset_key = asset_key;
    ref.layout_variant_key = variant_key;
    return ref;
}

UiPanelControllerRef MakeControllerRef(std::uint32_t type_key) {
    UiPanelControllerRef ref{};
    ref.controller_type_key = type_key;
    return ref;
}

UiPanelResourceRef MakeResourceRef(std::uint32_t resource_key, std::uint32_t type_key) {
    UiPanelResourceRef ref{};
    ref.resource_key = resource_key;
    ref.resource_type_key = type_key;
    return ref;
}

UiManagerLayerRootRef MakeRootRef(std::uint32_t root_key) {
    UiManagerLayerRootRef ref{};
    ref.root_key = root_key;
    return ref;
}

UiPanelManifestRecord MakePanelRecord(
    std::uint32_t panel_id,
    std::uint32_t layout_key,
    std::uint32_t controller_key,
    std::uint32_t resource_key) {
    const std::uint32_t layout_variant_key = layout_key + 10U;
    UiPanelManifestRecord record{};
    record.panel_id = MakePanelId(panel_id);
    record.layout_ref = MakeLayoutRef(layout_key, layout_variant_key);
    record.controller_ref = MakeControllerRef(controller_key);
    record.resource_refs[0U] = MakeResourceRef(resource_key, 970U);
    record.resource_ref_count = 1U;
    return record;
}

UiManagerLayerRecord MakeLayerRecord(
    std::uint32_t layer_id,
    UiManagerLayerType type,
    std::int32_t order,
    std::uint32_t root_key) {
    UiManagerLayerRecord record{};
    record.layer_id = MakeLayerId(layer_id);
    record.type = type;
    record.order = order;
    record.root_ref = MakeRootRef(root_key);
    return record;
}

UiManagerPanelLayerBinding MakePanelBinding(std::uint32_t panel_id, std::uint32_t layer_id) {
    UiManagerPanelLayerBinding binding{};
    binding.panel_id = MakePanelId(panel_id);
    binding.layer_id = MakeLayerId(layer_id);
    return binding;
}

UiPanelOpenArgs MakeOpenArgs(std::uint32_t request_key, std::span<const std::uint32_t> values) {
    UiPanelOpenArgs args{};
    args.request_key = request_key;
    args.values = values.data();
    args.value_count = static_cast<std::uint32_t>(values.size());
    return args;
}

bool RegisterLayer(
    UiManagerLayerModel *layer_model,
    std::uint32_t layer_id,
    UiManagerLayerType type,
    std::int32_t order,
    std::uint32_t root_key) {
    if (layer_model == nullptr) {
        return false;
    }

    const UiManagerLayerRecord record = MakeLayerRecord(layer_id, type, order, root_key);
    return layer_model->RegisterLayer(record).Succeeded();
}

bool RegisterPanelAndBind(
    UiPanelRegistry *registry,
    UiManagerLayerModel *layer_model,
    std::uint32_t panel_id,
    std::uint32_t layer_id) {
    if (registry == nullptr || layer_model == nullptr) {
        return false;
    }

    const std::uint32_t layout_key = 100U + panel_id;
    const std::uint32_t controller_key = 200U + panel_id;
    const std::uint32_t resource_key = 300U + panel_id;
    const UiPanelManifestRecord record = MakePanelRecord(panel_id, layout_key, controller_key, resource_key);
    if (!registry->RegisterPanel(record).Succeeded()) {
        return false;
    }

    const UiManagerPanelLayerBinding binding = MakePanelBinding(panel_id, layer_id);
    return layer_model->BindPanelToLayer(binding).Succeeded();
}

bool SetupSmokeManifest(UiProjectRuntimeSmokeContext *context) {
    if (context == nullptr) {
        return false;
    }

    if (!RegisterLayer(
        &context->layer_model,
        POPUP_LAYER_ID,
        UiManagerLayerType::Popup,
        50,
        POPUP_ROOT_KEY)) {
        return false;
    }

    if (!RegisterLayer(
        &context->layer_model,
        FULLSCREEN_LAYER_ID,
        UiManagerLayerType::Fullscreen,
        20,
        FULLSCREEN_ROOT_KEY)) {
        return false;
    }

    if (!RegisterPanelAndBind(&context->registry, &context->layer_model, POPUP_PANEL_ID, POPUP_LAYER_ID)) {
        return false;
    }

    if (!RegisterPanelAndBind(
        &context->registry,
        &context->layer_model,
        FULLSCREEN_FIRST_PANEL_ID,
        FULLSCREEN_LAYER_ID)) {
        return false;
    }

    if (!RegisterPanelAndBind(
        &context->registry,
        &context->layer_model,
        FULLSCREEN_SECOND_PANEL_ID,
        FULLSCREEN_LAYER_ID)) {
        return false;
    }

    return RegisterPanelAndBind(&context->registry, &context->layer_model, GRID_PANEL_ID, FULLSCREEN_LAYER_ID);
}

bool PopupSnapshotDisplayed(const UiSimplePopupToastRepresentativeSnapshot &snapshot) {
    if (!snapshot.visible || !snapshot.displayed || snapshot.cleared) {
        return false;
    }

    if (snapshot.message_key == 0U || snapshot.style_key == 0U) {
        return false;
    }

    return snapshot.open_display_count == 1U;
}

bool PopupSnapshotClosed(const UiSimplePopupToastRepresentativeSnapshot &snapshot) {
    if (snapshot.visible || snapshot.displayed || snapshot.cleared) {
        return false;
    }

    return snapshot.close_display_count == 1U;
}

bool FullscreenSnapshotDisplayed(const UiFullscreenRepresentativeSnapshot &snapshot) {
    if (!snapshot.visible || !snapshot.displayed || !snapshot.top_active || snapshot.cleared) {
        return false;
    }

    return snapshot.screen_key != 0U;
}

bool FullscreenSnapshotClosed(const UiFullscreenRepresentativeSnapshot &snapshot) {
    if (snapshot.visible || snapshot.displayed || snapshot.top_active || snapshot.cleared) {
        return false;
    }

    return snapshot.close_display_count > 0U;
}

bool GridSnapshotDisplayed(const UiGridViewRepresentativeSnapshot &snapshot) {
    if (!snapshot.visible || !snapshot.displayed || !snapshot.grid_displayed || snapshot.cleared) {
        return false;
    }

    if (snapshot.visible_item_count == 0U || snapshot.cell_count == 0U) {
        return false;
    }

    for (std::uint32_t index = 0U; index < snapshot.cell_count; ++index) {
        const UiGridViewRepresentativeCellSnapshot &cell = snapshot.cells[index];
        if (cell.has_item && cell.visible && cell.item_key != 0U) {
            return true;
        }
    }

    return false;
}

void CaptureSnapshots(
    const UiProjectRuntimeSmokeContext &context,
    UiProjectRuntimeSmokeSampleResult *result) {
    if (result == nullptr) {
        return;
    }

    result->panel_map_snapshot = context.panel_map.Snapshot();
    result->popup_stack_snapshot = context.popup_stack.Snapshot();
    result->fullscreen_stack_snapshot = context.fullscreen_stack.Snapshot();
}

bool ValidateCleanup(
    const UiProjectRuntimeSmokeContext &context,
    UiProjectRuntimeSmokeSampleResult *result) {
    if (result == nullptr) {
        return false;
    }

    CaptureSnapshots(context, result);
    if (result->panel_map_snapshot.loaded_panel_count != 0U ||
        result->panel_map_snapshot.active_panel_count != 0U) {
        return false;
    }

    if (result->popup_stack_snapshot.popup_count != 0U ||
        result->popup_stack_snapshot.top_panel_id.IsValid()) {
        return false;
    }

    if (result->fullscreen_stack_snapshot.fullscreen_count != 0U ||
        result->fullscreen_stack_snapshot.top_panel_id.IsValid()) {
        return false;
    }

    return true;
}

UiProjectRuntimeSmokeSampleStatus RunPopupSmoke(
    UiProjectRuntimeSmokeContext *context,
    UiProjectRuntimeSmokeSampleResult *result) {
    if (context == nullptr || result == nullptr) {
        return UiProjectRuntimeSmokeSampleStatus::InvalidOutputBuffer;
    }

    std::array<std::uint32_t, 2U> values{1101U, 1102U};
    const std::span<const std::uint32_t> value_span(values.data(), values.size());
    const UiPanelOpenArgs open_args = MakeOpenArgs(8101U, value_span);
    const UiManagerPopupStackResult open_result =
        context->popup_stack.OpenPopupPanelWithArgs(
            MakePanelId(POPUP_PANEL_ID),
            context->registry,
            context->layer_model,
            &context->panel_map,
            &context->popup_controller,
            open_args);
    if (!open_result.Succeeded()) {
        return UiProjectRuntimeSmokeSampleStatus::PopupOpenFailed;
    }

    const UiSimplePopupToastRepresentativeSnapshot open_snapshot =
        context->popup_controller.GetRepresentativeSnapshot();
    if (!PopupSnapshotDisplayed(open_snapshot)) {
        return UiProjectRuntimeSmokeSampleStatus::PopupDisplayMismatch;
    }

    result->popup_opened = true;
    result->popup_displayed = true;
    result->popup_open_display_count = open_snapshot.open_display_count;
    ++result->smoke_step_count;

    const UiManagerPopupStackResult close_result =
        context->popup_stack.ClosePopupPanel(MakePanelId(POPUP_PANEL_ID), &context->panel_map);
    if (!close_result.Succeeded()) {
        return UiProjectRuntimeSmokeSampleStatus::PopupCloseFailed;
    }

    const UiSimplePopupToastRepresentativeSnapshot close_snapshot =
        context->popup_controller.GetRepresentativeSnapshot();
    if (!PopupSnapshotClosed(close_snapshot)) {
        return UiProjectRuntimeSmokeSampleStatus::PopupCloseFailed;
    }

    result->popup_closed = true;

    const UiManagerPopupStackResult release_result =
        context->popup_stack.ReleasePopupPanel(MakePanelId(POPUP_PANEL_ID), &context->panel_map);
    if (!release_result.Succeeded()) {
        return UiProjectRuntimeSmokeSampleStatus::PopupReleaseFailed;
    }

    result->popup_released = true;
    ++result->released_panel_count;
    return UiProjectRuntimeSmokeSampleStatus::Success;
}

UiProjectRuntimeSmokeSampleStatus RunFullscreenSmoke(
    UiProjectRuntimeSmokeContext *context,
    UiProjectRuntimeSmokeSampleResult *result) {
    if (context == nullptr || result == nullptr) {
        return UiProjectRuntimeSmokeSampleStatus::InvalidOutputBuffer;
    }

    std::array<std::uint32_t, 2U> first_values{1201U, 1202U};
    const std::span<const std::uint32_t> first_span(first_values.data(), first_values.size());
    const UiPanelOpenArgs first_args = MakeOpenArgs(8201U, first_span);
    const UiManagerFullscreenStackResult first_result =
        context->fullscreen_stack.OpenFullscreenPanelWithArgs(
            MakePanelId(FULLSCREEN_FIRST_PANEL_ID),
            context->registry,
            context->layer_model,
            &context->panel_map,
            &context->fullscreen_first_controller,
            first_args);
    if (!first_result.Succeeded()) {
        return UiProjectRuntimeSmokeSampleStatus::FullscreenFirstOpenFailed;
    }

    std::array<std::uint32_t, 2U> second_values{1301U, 1302U};
    const std::span<const std::uint32_t> second_span(second_values.data(), second_values.size());
    const UiPanelOpenArgs second_args = MakeOpenArgs(8202U, second_span);
    const UiManagerFullscreenStackResult second_result =
        context->fullscreen_stack.OpenFullscreenPanelWithArgs(
            MakePanelId(FULLSCREEN_SECOND_PANEL_ID),
            context->registry,
            context->layer_model,
            &context->panel_map,
            &context->fullscreen_second_controller,
            second_args);
    if (!second_result.Succeeded()) {
        return UiProjectRuntimeSmokeSampleStatus::FullscreenSecondOpenFailed;
    }

    const UiFullscreenRepresentativeSnapshot first_closed_snapshot =
        context->fullscreen_first_controller.GetRepresentativeSnapshot();
    const UiFullscreenRepresentativeSnapshot second_open_snapshot =
        context->fullscreen_second_controller.GetRepresentativeSnapshot();
    if (!FullscreenSnapshotClosed(first_closed_snapshot) ||
        !FullscreenSnapshotDisplayed(second_open_snapshot)) {
        return UiProjectRuntimeSmokeSampleStatus::FullscreenDisplayMismatch;
    }

    const UiManagerFullscreenStackResult back_result =
        context->fullscreen_stack.NavigateBack(context->registry, context->layer_model, &context->panel_map);
    if (!back_result.Succeeded() || !back_result.restored_previous) {
        return UiProjectRuntimeSmokeSampleStatus::FullscreenBackFailed;
    }

    const UiFullscreenRepresentativeSnapshot restored_snapshot =
        context->fullscreen_first_controller.GetRepresentativeSnapshot();
    const UiFullscreenRepresentativeSnapshot second_closed_snapshot =
        context->fullscreen_second_controller.GetRepresentativeSnapshot();
    if (!FullscreenSnapshotDisplayed(restored_snapshot) ||
        !FullscreenSnapshotClosed(second_closed_snapshot)) {
        return UiProjectRuntimeSmokeSampleStatus::FullscreenDisplayMismatch;
    }

    result->fullscreen_opened = true;
    result->fullscreen_top_displayed = true;
    result->fullscreen_back_restored = true;
    result->fullscreen_open_display_count =
        restored_snapshot.open_display_count + second_closed_snapshot.open_display_count;
    ++result->smoke_step_count;

    const UiManagerFullscreenStackResult release_first_result =
        context->fullscreen_stack.ReleaseFullscreenPanel(
            MakePanelId(FULLSCREEN_FIRST_PANEL_ID),
            context->registry,
            context->layer_model,
            &context->panel_map);
    if (!release_first_result.Succeeded()) {
        return UiProjectRuntimeSmokeSampleStatus::FullscreenReleaseFailed;
    }

    const UiManagerPanelMapResult release_second_result =
        context->panel_map.ReleasePanel(MakePanelId(FULLSCREEN_SECOND_PANEL_ID));
    if (!release_second_result.Succeeded()) {
        return UiProjectRuntimeSmokeSampleStatus::FullscreenReleaseFailed;
    }

    result->fullscreen_released = true;
    result->released_panel_count += 2U;
    return UiProjectRuntimeSmokeSampleStatus::Success;
}

UiProjectRuntimeSmokeSampleStatus RunGridSmoke(
    UiProjectRuntimeSmokeContext *context,
    UiProjectRuntimeSmokeSampleResult *result) {
    if (context == nullptr || result == nullptr) {
        return UiProjectRuntimeSmokeSampleStatus::InvalidOutputBuffer;
    }

    std::array<std::uint32_t, 2U> values{2U, 9U};
    const std::span<const std::uint32_t> value_span(values.data(), values.size());
    const UiPanelOpenArgs open_args = MakeOpenArgs(8301U, value_span);
    const UiManagerPanelMapResult open_result =
        context->panel_map.OpenPanelWithArgs(
            MakePanelId(GRID_PANEL_ID),
            context->registry,
            context->layer_model,
            &context->grid_controller,
            open_args);
    if (!open_result.Succeeded()) {
        return UiProjectRuntimeSmokeSampleStatus::GridOpenFailed;
    }

    const UiGridViewRepresentativeSnapshot grid_snapshot =
        context->grid_controller.GetRepresentativeSnapshot();
    if (!GridSnapshotDisplayed(grid_snapshot)) {
        return UiProjectRuntimeSmokeSampleStatus::GridDisplayMismatch;
    }

    result->grid_opened = true;
    result->grid_displayed = true;
    result->grid_visible_data_read = true;
    result->grid_open_display_count = grid_snapshot.open_display_count;
    result->grid_visible_item_count = grid_snapshot.visible_item_count;
    ++result->smoke_step_count;

    const UiManagerPanelMapResult release_result =
        context->panel_map.ReleasePanel(MakePanelId(GRID_PANEL_ID));
    if (!release_result.Succeeded()) {
        return UiProjectRuntimeSmokeSampleStatus::GridReleaseFailed;
    }

    result->grid_released = true;
    ++result->released_panel_count;
    return UiProjectRuntimeSmokeSampleStatus::Success;
}
}

UiProjectRuntimeSmokeSampleStatus UiProjectRuntimeSmokeSample::Run(
    UiProjectRuntimeSmokeSampleResult *out_result) {
    if (out_result == nullptr) {
        return UiProjectRuntimeSmokeSampleStatus::InvalidOutputBuffer;
    }

    UiProjectRuntimeSmokeSampleResult result{};
    UiProjectRuntimeSmokeContext context{};
    if (!SetupSmokeManifest(&context)) {
        result.status = UiProjectRuntimeSmokeSampleStatus::SetupFailed;
        CaptureSnapshots(context, &result);
        *out_result = result;
        return result.status;
    }

    result.status = RunPopupSmoke(&context, &result);
    if (result.status != UiProjectRuntimeSmokeSampleStatus::Success) {
        CaptureSnapshots(context, &result);
        *out_result = result;
        return result.status;
    }

    result.status = RunFullscreenSmoke(&context, &result);
    if (result.status != UiProjectRuntimeSmokeSampleStatus::Success) {
        CaptureSnapshots(context, &result);
        *out_result = result;
        return result.status;
    }

    result.status = RunGridSmoke(&context, &result);
    if (result.status != UiProjectRuntimeSmokeSampleStatus::Success) {
        CaptureSnapshots(context, &result);
        *out_result = result;
        return result.status;
    }

    if (!ValidateCleanup(context, &result)) {
        result.status = UiProjectRuntimeSmokeSampleStatus::CleanupMismatch;
        *out_result = result;
        return result.status;
    }

    result.cleanup_passed = true;
    result.passed = true;
    result.status = UiProjectRuntimeSmokeSampleStatus::Success;
    *out_result = result;
    return result.status;
}
}
