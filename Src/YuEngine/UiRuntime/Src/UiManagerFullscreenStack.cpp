// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Src/UiManagerFullscreenStack.cpp

#include "YuEngine/UiRuntime/UiManagerFullscreenStack.h"

namespace yuengine::uiruntime {
UiManagerFullscreenStack::UiManagerFullscreenStack()
    : snapshot_{} {
}

UiManagerFullscreenStackResult UiManagerFullscreenStack::OpenFullscreenPanel(
    UiPanelId panel_id,
    const UiPanelRegistry &registry,
    const UiManagerLayerModel &layer_model,
    UiManagerPanelMap *panel_map,
    BaseUiController *controller) {
    UiPanelOpenArgs open_args{};
    return OpenFullscreenPanelWithArgs(panel_id, registry, layer_model, panel_map, controller, open_args);
}

UiManagerFullscreenStackResult UiManagerFullscreenStack::OpenFullscreenPanelWithArgs(
    UiPanelId panel_id,
    const UiPanelRegistry &registry,
    const UiManagerLayerModel &layer_model,
    UiManagerPanelMap *panel_map,
    BaseUiController *controller,
    const UiPanelOpenArgs &open_args) {
    if (!panel_id.IsValid()) {
        return MakeResult(
            RecordFailure(UiManagerFullscreenStackStatus::InvalidPanelId),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            UiPanelId{},
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    if (panel_map == nullptr) {
        return MakeResult(
            RecordFailure(UiManagerFullscreenStackStatus::InvalidPanelMap),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            UiPanelId{},
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    UiManagerLayerRecord layer_record{};
    const UiManagerFullscreenStackStatus layer_status =
        ValidateFullscreenLayer(panel_id, registry, layer_model, &layer_record);
    if (layer_status != UiManagerFullscreenStackStatus::Success) {
        return MakeResult(
            RecordFailure(layer_status),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            UiPanelId{},
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    if (!panel_map->IsLoaded(panel_id) && controller == nullptr) {
        return MakeResult(
            RecordFailure(UiManagerFullscreenStackStatus::InvalidController),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            UiPanelId{},
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const std::uint32_t stack_index = FindStackIndex(panel_id);
    if (stack_index >= snapshot_.fullscreen_count &&
        snapshot_.fullscreen_count >= snapshot_.fullscreen_capacity) {
        return MakeResult(
            RecordFailure(UiManagerFullscreenStackStatus::CapacityExceeded),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            UiPanelId{},
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const bool already_top =
        stack_index < snapshot_.fullscreen_count && stack_index + 1U == snapshot_.fullscreen_count;
    if (already_top && panel_map->IsActive(panel_id)) {
        UiManagerPanelMapRecord active_record{};
        const UiManagerPanelMapStatus active_status = panel_map->ResolveActivePanel(panel_id, &active_record);
        ++snapshot_.idempotent_open_count;
        RecordSuccess();
        return MakeResult(
            UiManagerFullscreenStackStatus::Success,
            active_status,
            active_record,
            panel_id,
            UiPanelId{},
            panel_id,
            false,
            false,
            false,
            false,
            true,
            true,
            false,
            false,
            false,
            false);
    }

    UiPanelId closed_panel_id{};
    if (snapshot_.top_panel_id.IsValid() && snapshot_.top_panel_id.value != panel_id.value) {
        closed_panel_id = snapshot_.top_panel_id;
        const UiManagerPanelMapResult close_result = panel_map->ClosePanel(closed_panel_id);
        if (!close_result.Succeeded()) {
            const UiManagerFullscreenStackStatus status = TranslatePanelMapStatus(close_result.status);
            return MakeResult(
                RecordFailure(status),
                close_result.status,
                close_result.record,
                panel_id,
                closed_panel_id,
                UiPanelId{},
                false,
                false,
                false,
                false,
                false,
                stack_index < snapshot_.fullscreen_count,
                false,
                false,
                false,
                false);
        }
    }

    const UiManagerPanelMapResult panel_result =
        panel_map->OpenPanelWithArgs(panel_id, registry, layer_model, controller, open_args);
    if (!panel_result.Succeeded()) {
        const UiManagerFullscreenStackStatus status = TranslatePanelMapStatus(panel_result.status);
        return MakeResult(
            RecordFailure(status),
            panel_result.status,
            panel_result.record,
            panel_id,
            closed_panel_id,
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            stack_index < snapshot_.fullscreen_count,
            false,
            false,
            false,
            false);
    }

    if (stack_index < snapshot_.fullscreen_count) {
        ++snapshot_.idempotent_open_count;
        if (stack_index + 1U == snapshot_.fullscreen_count) {
            RecordSuccess();
            return MakeResult(
                UiManagerFullscreenStackStatus::Success,
                panel_result.status,
                panel_result.record,
                panel_id,
                closed_panel_id,
                panel_id,
                false,
                false,
                false,
                false,
                true,
                true,
                false,
                false,
                false,
                false);
        }

        MoveFullscreenToTop(stack_index);
        RecordSuccess();
        return MakeResult(
            UiManagerFullscreenStackStatus::Success,
            panel_result.status,
            panel_result.record,
            panel_id,
            closed_panel_id,
            panel_id,
            false,
            true,
            false,
            false,
            false,
            true,
            false,
            false,
            false,
            false);
    }

    PushFullscreen(panel_id);
    ++snapshot_.open_operation_count;
    RecordSuccess();
    return MakeResult(
        UiManagerFullscreenStackStatus::Success,
        panel_result.status,
        panel_result.record,
        panel_id,
        closed_panel_id,
        panel_id,
        true,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false);
}

UiManagerFullscreenStackResult UiManagerFullscreenStack::NavigateBack(
    const UiPanelRegistry &registry,
    const UiManagerLayerModel &layer_model,
    UiManagerPanelMap *panel_map) {
    if (panel_map == nullptr) {
        return MakeResult(
            RecordFailure(UiManagerFullscreenStackStatus::InvalidPanelMap),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            UiPanelId{},
            UiPanelId{},
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    if (snapshot_.fullscreen_count == 0U) {
        return MakeResult(
            RecordFailure(UiManagerFullscreenStackStatus::BackStackEmpty),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            UiPanelId{},
            UiPanelId{},
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const UiPanelId closed_panel_id = snapshot_.top_panel_id;
    const UiManagerPanelMapResult close_result = panel_map->ClosePanel(closed_panel_id);
    if (!close_result.Succeeded()) {
        const UiManagerFullscreenStackStatus status = TranslatePanelMapStatus(close_result.status);
        return MakeResult(
            RecordFailure(status),
            close_result.status,
            close_result.record,
            closed_panel_id,
            closed_panel_id,
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            true,
            false,
            false,
            true,
            false);
    }

    RemoveFullscreenAt(snapshot_.fullscreen_count - 1U);
    ++snapshot_.back_navigation_operation_count;
    return RestoreTopFullscreen(
        registry,
        layer_model,
        panel_map,
        closed_panel_id,
        closed_panel_id,
        close_result.record,
        true,
        close_result.already_inactive,
        true,
        false);
}

UiManagerFullscreenStackResult UiManagerFullscreenStack::CloseFullscreenPanel(
    UiPanelId panel_id,
    const UiPanelRegistry &registry,
    const UiManagerLayerModel &layer_model,
    UiManagerPanelMap *panel_map) {
    if (!panel_id.IsValid()) {
        return MakeResult(
            RecordFailure(UiManagerFullscreenStackStatus::InvalidPanelId),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            UiPanelId{},
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    if (panel_map == nullptr) {
        return MakeResult(
            RecordFailure(UiManagerFullscreenStackStatus::InvalidPanelMap),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            UiPanelId{},
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    UiManagerPanelMapRecord record{};
    UiManagerPanelMapStatus panel_map_status = UiManagerPanelMapStatus::Success;
    const UiManagerFullscreenStackStatus loaded_status =
        ResolveLoadedFullscreenRecord(panel_id, *panel_map, &record, &panel_map_status);
    if (loaded_status != UiManagerFullscreenStackStatus::Success) {
        return MakeResult(
            RecordFailure(loaded_status),
            panel_map_status,
            record,
            panel_id,
            UiPanelId{},
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const std::uint32_t stack_index = FindStackIndex(panel_id);
    if (stack_index >= snapshot_.fullscreen_count) {
        if (!record.active) {
            ++snapshot_.idempotent_close_count;
            RecordSuccess();
            return MakeResult(
                UiManagerFullscreenStackStatus::Success,
                panel_map_status,
                record,
                panel_id,
                panel_id,
                UiPanelId{},
                false,
                false,
                false,
                false,
                false,
                false,
                true,
                false,
                false,
                false);
        }

        return MakeResult(
            RecordFailure(UiManagerFullscreenStackStatus::FullscreenNotInStack),
            panel_map_status,
            record,
            panel_id,
            UiPanelId{},
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const bool closing_current = stack_index + 1U == snapshot_.fullscreen_count;
    if (!record.active) {
        RemoveFullscreenAt(stack_index);
        ++snapshot_.idempotent_close_count;
        RecordSuccess();
        return MakeResult(
            UiManagerFullscreenStackStatus::Success,
            panel_map_status,
            record,
            panel_id,
            panel_id,
            snapshot_.top_panel_id,
            false,
            false,
            false,
            false,
            false,
            true,
            true,
            true,
            false,
            !closing_current);
    }

    const UiManagerPanelMapResult close_result = panel_map->ClosePanel(panel_id);
    if (!close_result.Succeeded()) {
        const UiManagerFullscreenStackStatus status = TranslatePanelMapStatus(close_result.status);
        return MakeResult(
            RecordFailure(status),
            close_result.status,
            close_result.record,
            panel_id,
            panel_id,
            UiPanelId{},
            false,
            false,
            false,
            false,
            false,
            true,
            false,
            false,
            closing_current,
            !closing_current);
    }

    RemoveFullscreenAt(stack_index);
    ++snapshot_.close_operation_count;
    if (closing_current) {
        return RestoreTopFullscreen(
            registry,
            layer_model,
            panel_map,
            panel_id,
            panel_id,
            close_result.record,
            false,
            close_result.already_inactive,
            true,
            false);
    }

    RecordSuccess();
    return MakeResult(
        UiManagerFullscreenStackStatus::Success,
        close_result.status,
        close_result.record,
        panel_id,
        panel_id,
        snapshot_.top_panel_id,
        false,
        false,
        false,
        false,
        false,
        true,
        close_result.already_inactive,
        true,
        false,
        true);
}

UiManagerFullscreenStackStatus UiManagerFullscreenStack::ExportFullscreenOrder(
    UiPanelId *output_panel_ids,
    std::uint32_t output_capacity) const {
    if (snapshot_.fullscreen_count > 0U && output_panel_ids == nullptr) {
        return UiManagerFullscreenStackStatus::InvalidOutputBuffer;
    }

    if (output_capacity < snapshot_.fullscreen_count) {
        return UiManagerFullscreenStackStatus::InvalidOutputBuffer;
    }

    for (std::uint32_t index = 0U; index < snapshot_.fullscreen_count; ++index) {
        output_panel_ids[index] = snapshot_.fullscreen_order[index];
    }

    return UiManagerFullscreenStackStatus::Success;
}

bool UiManagerFullscreenStack::IsFullscreenStacked(UiPanelId panel_id) const {
    return FindStackIndex(panel_id) < snapshot_.fullscreen_count;
}

UiManagerFullscreenStackSnapshot UiManagerFullscreenStack::Snapshot() const {
    return snapshot_;
}

UiManagerFullscreenStackStatus UiManagerFullscreenStack::ValidateFullscreenLayer(
    UiPanelId panel_id,
    const UiPanelRegistry &registry,
    const UiManagerLayerModel &layer_model,
    UiManagerLayerRecord *out_layer_record) const {
    if (out_layer_record == nullptr) {
        return UiManagerFullscreenStackStatus::InvalidOutputBuffer;
    }

    UiPanelManifestRecord manifest_record{};
    const UiPanelRegistryStatus registry_status = registry.ResolvePanel(panel_id, &manifest_record);
    const UiManagerFullscreenStackStatus panel_status = TranslateRegistryStatus(registry_status);
    if (panel_status != UiManagerFullscreenStackStatus::Success) {
        return panel_status;
    }

    UiManagerLayerRecord layer_record{};
    const UiManagerLayerModelStatus layer_status = layer_model.ResolvePanelLayer(panel_id, &layer_record);
    const UiManagerFullscreenStackStatus fullscreen_layer_status = TranslateLayerStatus(layer_status);
    if (fullscreen_layer_status != UiManagerFullscreenStackStatus::Success) {
        return fullscreen_layer_status;
    }

    if (layer_record.type != UiManagerLayerType::Fullscreen) {
        return UiManagerFullscreenStackStatus::NonFullscreenPanel;
    }

    *out_layer_record = layer_record;
    return UiManagerFullscreenStackStatus::Success;
}

UiManagerFullscreenStackStatus UiManagerFullscreenStack::ResolveLoadedFullscreenRecord(
    UiPanelId panel_id,
    const UiManagerPanelMap &panel_map,
    UiManagerPanelMapRecord *out_record,
    UiManagerPanelMapStatus *out_panel_map_status) const {
    if (out_record == nullptr || out_panel_map_status == nullptr) {
        return UiManagerFullscreenStackStatus::InvalidOutputBuffer;
    }

    UiManagerPanelMapRecord record{};
    const UiManagerPanelMapStatus panel_map_status = panel_map.ResolveLoadedPanel(panel_id, &record);
    *out_panel_map_status = panel_map_status;
    const UiManagerFullscreenStackStatus status = TranslatePanelMapStatus(panel_map_status);
    if (status != UiManagerFullscreenStackStatus::Success) {
        return status;
    }

    if (record.layer_record.type != UiManagerLayerType::Fullscreen) {
        *out_record = record;
        return UiManagerFullscreenStackStatus::NonFullscreenPanel;
    }

    *out_record = record;
    return UiManagerFullscreenStackStatus::Success;
}

UiManagerFullscreenStackStatus UiManagerFullscreenStack::TranslateRegistryStatus(UiPanelRegistryStatus status) const {
    if (status == UiPanelRegistryStatus::Success) {
        return UiManagerFullscreenStackStatus::Success;
    }

    if (status == UiPanelRegistryStatus::InvalidPanelId) {
        return UiManagerFullscreenStackStatus::InvalidPanelId;
    }

    if (status == UiPanelRegistryStatus::PanelNotFound) {
        return UiManagerFullscreenStackStatus::PanelNotRegistered;
    }

    if (status == UiPanelRegistryStatus::InvalidOutputBuffer) {
        return UiManagerFullscreenStackStatus::InvalidOutputBuffer;
    }

    if (status == UiPanelRegistryStatus::CapacityExceeded) {
        return UiManagerFullscreenStackStatus::CapacityExceeded;
    }

    return UiManagerFullscreenStackStatus::PanelNotRegistered;
}

UiManagerFullscreenStackStatus UiManagerFullscreenStack::TranslateLayerStatus(UiManagerLayerModelStatus status) const {
    if (status == UiManagerLayerModelStatus::Success) {
        return UiManagerFullscreenStackStatus::Success;
    }

    if (status == UiManagerLayerModelStatus::InvalidPanelId) {
        return UiManagerFullscreenStackStatus::InvalidPanelId;
    }

    if (status == UiManagerLayerModelStatus::PanelNotBound) {
        return UiManagerFullscreenStackStatus::PanelLayerNotBound;
    }

    if (status == UiManagerLayerModelStatus::LayerNotFound) {
        return UiManagerFullscreenStackStatus::LayerNotFound;
    }

    if (status == UiManagerLayerModelStatus::InvalidOutputBuffer) {
        return UiManagerFullscreenStackStatus::InvalidOutputBuffer;
    }

    if (status == UiManagerLayerModelStatus::CapacityExceeded) {
        return UiManagerFullscreenStackStatus::CapacityExceeded;
    }

    return UiManagerFullscreenStackStatus::PanelLayerNotBound;
}

UiManagerFullscreenStackStatus UiManagerFullscreenStack::TranslatePanelMapStatus(UiManagerPanelMapStatus status) const {
    if (status == UiManagerPanelMapStatus::Success) {
        return UiManagerFullscreenStackStatus::Success;
    }

    if (status == UiManagerPanelMapStatus::InvalidPanelId) {
        return UiManagerFullscreenStackStatus::InvalidPanelId;
    }

    if (status == UiManagerPanelMapStatus::InvalidController) {
        return UiManagerFullscreenStackStatus::InvalidController;
    }

    if (status == UiManagerPanelMapStatus::InvalidOutputBuffer) {
        return UiManagerFullscreenStackStatus::InvalidOutputBuffer;
    }

    if (status == UiManagerPanelMapStatus::PanelNotRegistered) {
        return UiManagerFullscreenStackStatus::PanelNotRegistered;
    }

    if (status == UiManagerPanelMapStatus::PanelNotLoaded) {
        return UiManagerFullscreenStackStatus::PanelNotLoaded;
    }

    if (status == UiManagerPanelMapStatus::PanelNotActive) {
        return UiManagerFullscreenStackStatus::PanelNotActive;
    }

    if (status == UiManagerPanelMapStatus::PanelLayerNotBound) {
        return UiManagerFullscreenStackStatus::PanelLayerNotBound;
    }

    if (status == UiManagerPanelMapStatus::LayerNotFound) {
        return UiManagerFullscreenStackStatus::LayerNotFound;
    }

    if (status == UiManagerPanelMapStatus::InvalidOpenArgs) {
        return UiManagerFullscreenStackStatus::InvalidOpenArgs;
    }

    if (status == UiManagerPanelMapStatus::ControllerOpenFailed) {
        return UiManagerFullscreenStackStatus::ControllerOpenFailed;
    }

    if (status == UiManagerPanelMapStatus::ControllerCloseFailed) {
        return UiManagerFullscreenStackStatus::ControllerCloseFailed;
    }

    if (status == UiManagerPanelMapStatus::CapacityExceeded) {
        return UiManagerFullscreenStackStatus::CapacityExceeded;
    }

    return UiManagerFullscreenStackStatus::PanelMapRejected;
}

UiManagerFullscreenStackResult UiManagerFullscreenStack::RestoreTopFullscreen(
    const UiPanelRegistry &registry,
    const UiManagerLayerModel &layer_model,
    UiManagerPanelMap *panel_map,
    UiPanelId panel_id,
    UiPanelId closed_panel_id,
    const UiManagerPanelMapRecord &record,
    bool navigated_back,
    bool already_inactive,
    bool closed_current,
    bool closed_middle) {
    const UiPanelId restored_panel_id = snapshot_.top_panel_id;
    if (!restored_panel_id.IsValid()) {
        RecordSuccess();
        return MakeResult(
            UiManagerFullscreenStackStatus::Success,
            UiManagerPanelMapStatus::Success,
            record,
            panel_id,
            closed_panel_id,
            UiPanelId{},
            false,
            false,
            navigated_back,
            false,
            false,
            true,
            already_inactive,
            true,
            closed_current,
            closed_middle);
    }

    UiManagerPanelMapRecord restored_record{};
    const UiManagerPanelMapStatus restored_record_status =
        panel_map->ResolveLoadedPanel(restored_panel_id, &restored_record);
    if (restored_record_status != UiManagerPanelMapStatus::Success) {
        const UiManagerFullscreenStackStatus status = TranslatePanelMapStatus(restored_record_status);
        return MakeResult(
            RecordFailure(status),
            restored_record_status,
            restored_record,
            panel_id,
            closed_panel_id,
            restored_panel_id,
            false,
            false,
            navigated_back,
            false,
            false,
            true,
            already_inactive,
            true,
            closed_current,
            closed_middle);
    }

    const UiPanelOpenArgs restored_open_args = restored_record.open_args.ToArgs();
    const UiManagerPanelMapResult open_result =
        panel_map->OpenPanelWithArgs(restored_panel_id, registry, layer_model, nullptr, restored_open_args);
    if (!open_result.Succeeded()) {
        const UiManagerFullscreenStackStatus status = TranslatePanelMapStatus(open_result.status);
        return MakeResult(
            RecordFailure(status),
            open_result.status,
            open_result.record,
            panel_id,
            closed_panel_id,
            restored_panel_id,
            false,
            false,
            navigated_back,
            false,
            false,
            true,
            already_inactive,
            true,
            closed_current,
            closed_middle);
    }

    ++snapshot_.restore_operation_count;
    RecordSuccess();
    return MakeResult(
        UiManagerFullscreenStackStatus::Success,
        open_result.status,
        open_result.record,
        panel_id,
        closed_panel_id,
        restored_panel_id,
        false,
        false,
        navigated_back,
        true,
        false,
        true,
        already_inactive,
        true,
        closed_current,
        closed_middle);
}

std::uint32_t UiManagerFullscreenStack::FindStackIndex(UiPanelId panel_id) const {
    if (!panel_id.IsValid()) {
        return MAX_UI_MANAGER_FULLSCREEN_STACK_COUNT;
    }

    for (std::uint32_t index = 0U; index < snapshot_.fullscreen_count; ++index) {
        if (snapshot_.fullscreen_order[index].value == panel_id.value) {
            return index;
        }
    }

    return MAX_UI_MANAGER_FULLSCREEN_STACK_COUNT;
}

void UiManagerFullscreenStack::PushFullscreen(UiPanelId panel_id) {
    snapshot_.fullscreen_order[snapshot_.fullscreen_count] = panel_id;
    ++snapshot_.fullscreen_count;
    RefreshTopPanel();
}

void UiManagerFullscreenStack::MoveFullscreenToTop(std::uint32_t stack_index) {
    if (stack_index >= snapshot_.fullscreen_count) {
        return;
    }

    const UiPanelId panel_id = snapshot_.fullscreen_order[stack_index];
    for (std::uint32_t index = stack_index; index + 1U < snapshot_.fullscreen_count; ++index) {
        snapshot_.fullscreen_order[index] = snapshot_.fullscreen_order[index + 1U];
    }

    snapshot_.fullscreen_order[snapshot_.fullscreen_count - 1U] = panel_id;
    RefreshTopPanel();
}

void UiManagerFullscreenStack::RemoveFullscreenAt(std::uint32_t stack_index) {
    if (stack_index >= snapshot_.fullscreen_count) {
        return;
    }

    for (std::uint32_t index = stack_index; index + 1U < snapshot_.fullscreen_count; ++index) {
        snapshot_.fullscreen_order[index] = snapshot_.fullscreen_order[index + 1U];
    }

    --snapshot_.fullscreen_count;
    snapshot_.fullscreen_order[snapshot_.fullscreen_count] = UiPanelId{};
    RefreshTopPanel();
}

void UiManagerFullscreenStack::RefreshTopPanel() {
    snapshot_.top_panel_id = UiPanelId{};
    if (snapshot_.fullscreen_count == 0U) {
        return;
    }

    snapshot_.top_panel_id = snapshot_.fullscreen_order[snapshot_.fullscreen_count - 1U];
}

UiManagerFullscreenStackResult UiManagerFullscreenStack::MakeResult(
    UiManagerFullscreenStackStatus status,
    UiManagerPanelMapStatus panel_map_status,
    const UiManagerPanelMapRecord &record,
    UiPanelId panel_id,
    UiPanelId closed_panel_id,
    UiPanelId restored_panel_id,
    bool pushed,
    bool moved_to_top,
    bool navigated_back,
    bool restored_previous,
    bool already_top,
    bool already_in_stack,
    bool already_inactive,
    bool removed_from_stack,
    bool closed_current,
    bool closed_middle) const {
    UiManagerFullscreenStackResult result{};
    result.status = status;
    result.panel_map_status = panel_map_status;
    result.record = record;
    result.panel_id = panel_id;
    result.closed_panel_id = closed_panel_id;
    result.restored_panel_id = restored_panel_id;
    result.top_panel_id = snapshot_.top_panel_id;
    result.fullscreen_count = snapshot_.fullscreen_count;
    result.pushed = pushed;
    result.moved_to_top = moved_to_top;
    result.navigated_back = navigated_back;
    result.restored_previous = restored_previous;
    result.already_top = already_top;
    result.already_in_stack = already_in_stack;
    result.already_inactive = already_inactive;
    result.removed_from_stack = removed_from_stack;
    result.closed_current = closed_current;
    result.closed_middle = closed_middle;
    return result;
}

UiManagerFullscreenStackStatus UiManagerFullscreenStack::RecordFailure(UiManagerFullscreenStackStatus status) {
    ++snapshot_.rejected_operation_count;
    snapshot_.last_status = status;
    return status;
}

void UiManagerFullscreenStack::RecordSuccess() {
    ++snapshot_.accepted_operation_count;
    snapshot_.last_status = UiManagerFullscreenStackStatus::Success;
}
}
