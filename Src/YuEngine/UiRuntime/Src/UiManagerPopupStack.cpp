// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Src/UiManagerPopupStack.cpp

#include "YuEngine/UiRuntime/UiManagerPopupStack.h"

namespace yuengine::uiruntime {
UiManagerPopupStack::UiManagerPopupStack()
    : snapshot_{} {
}

UiManagerPopupStackResult UiManagerPopupStack::OpenPopupPanel(
    UiPanelId panel_id,
    const UiPanelRegistry &registry,
    const UiManagerLayerModel &layer_model,
    UiManagerPanelMap *panel_map,
    BaseUiController *controller) {
    UiPanelOpenArgs open_args{};
    return OpenPopupPanelWithArgs(panel_id, registry, layer_model, panel_map, controller, open_args);
}

UiManagerPopupStackResult UiManagerPopupStack::OpenPopupPanelWithArgs(
    UiPanelId panel_id,
    const UiPanelRegistry &registry,
    const UiManagerLayerModel &layer_model,
    UiManagerPanelMap *panel_map,
    BaseUiController *controller,
    const UiPanelOpenArgs &open_args) {
    if (!panel_id.IsValid()) {
        return MakeResult(
            RecordFailure(UiManagerPopupStackStatus::InvalidPanelId),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    if (panel_map == nullptr) {
        return MakeResult(
            RecordFailure(UiManagerPopupStackStatus::InvalidPanelMap),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    UiManagerLayerRecord layer_record{};
    const UiManagerPopupStackStatus popup_layer_status =
        ValidatePopupLayer(panel_id, registry, layer_model, &layer_record);
    if (popup_layer_status != UiManagerPopupStackStatus::Success) {
        return MakeResult(
            RecordFailure(popup_layer_status),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const std::uint32_t stack_index = FindStackIndex(panel_id);
    if (stack_index >= snapshot_.popup_count && snapshot_.popup_count >= snapshot_.popup_capacity) {
        return MakeResult(
            RecordFailure(UiManagerPopupStackStatus::CapacityExceeded),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const UiManagerPanelMapResult panel_result =
        panel_map->OpenPanelWithArgs(panel_id, registry, layer_model, controller, open_args);
    if (!panel_result.Succeeded()) {
        const UiManagerPopupStackStatus status = TranslatePanelMapStatus(panel_result.status);
        return MakeResult(
            RecordFailure(status),
            panel_result.status,
            panel_result.record,
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    if (stack_index < snapshot_.popup_count) {
        ++snapshot_.idempotent_open_count;
        if (stack_index + 1U == snapshot_.popup_count) {
            RecordSuccess();
            return MakeResult(
                UiManagerPopupStackStatus::Success,
                panel_result.status,
                panel_result.record,
                panel_id,
                false,
                false,
                true,
                true,
                false,
                false);
        }

        MovePopupToTop(stack_index);
        ++snapshot_.bring_to_top_operation_count;
        RecordSuccess();
        return MakeResult(
            UiManagerPopupStackStatus::Success,
            panel_result.status,
            panel_result.record,
            panel_id,
            false,
            true,
            false,
            true,
            false,
            false);
    }

    PushPopup(panel_id);
    ++snapshot_.open_operation_count;
    RecordSuccess();
    return MakeResult(
        UiManagerPopupStackStatus::Success,
        panel_result.status,
        panel_result.record,
        panel_id,
        true,
        false,
        false,
        false,
        false,
        false);
}

UiManagerPopupStackResult UiManagerPopupStack::BringPopupToTop(
    UiPanelId panel_id,
    const UiManagerPanelMap &panel_map) {
    if (!panel_id.IsValid()) {
        return MakeResult(
            RecordFailure(UiManagerPopupStackStatus::InvalidPanelId),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    UiManagerPanelMapRecord record{};
    UiManagerPanelMapStatus panel_map_status = UiManagerPanelMapStatus::Success;
    const UiManagerPopupStackStatus active_status =
        ResolveActivePopupRecord(panel_id, panel_map, &record, &panel_map_status);
    if (active_status != UiManagerPopupStackStatus::Success) {
        return MakeResult(
            RecordFailure(active_status),
            panel_map_status,
            record,
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const std::uint32_t stack_index = FindStackIndex(panel_id);
    if (stack_index >= snapshot_.popup_count) {
        return MakeResult(
            RecordFailure(UiManagerPopupStackStatus::PopupNotInStack),
            panel_map_status,
            record,
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    if (stack_index + 1U == snapshot_.popup_count) {
        ++snapshot_.idempotent_bring_to_top_count;
        RecordSuccess();
        return MakeResult(
            UiManagerPopupStackStatus::Success,
            panel_map_status,
            record,
            panel_id,
            false,
            false,
            true,
            true,
            false,
            false);
    }

    MovePopupToTop(stack_index);
    ++snapshot_.bring_to_top_operation_count;
    RecordSuccess();
    return MakeResult(
        UiManagerPopupStackStatus::Success,
        panel_map_status,
        record,
        panel_id,
        false,
        true,
        false,
        true,
        false,
        false);
}

UiManagerPopupStackResult UiManagerPopupStack::ClosePopupPanel(
    UiPanelId panel_id,
    UiManagerPanelMap *panel_map) {
    if (!panel_id.IsValid()) {
        return MakeResult(
            RecordFailure(UiManagerPopupStackStatus::InvalidPanelId),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    if (panel_map == nullptr) {
        return MakeResult(
            RecordFailure(UiManagerPopupStackStatus::InvalidPanelMap),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    UiManagerPanelMapRecord record{};
    UiManagerPanelMapStatus panel_map_status = UiManagerPanelMapStatus::Success;
    const UiManagerPopupStackStatus loaded_status =
        ResolveLoadedPopupRecord(panel_id, *panel_map, &record, &panel_map_status);
    if (loaded_status != UiManagerPopupStackStatus::Success) {
        return MakeResult(
            RecordFailure(loaded_status),
            panel_map_status,
            record,
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const std::uint32_t stack_index = FindStackIndex(panel_id);
    if (!record.active) {
        const bool already_in_stack = stack_index < snapshot_.popup_count;
        bool removed_from_stack = false;
        if (already_in_stack) {
            RemovePopupAt(stack_index);
            removed_from_stack = true;
        }

        ++snapshot_.idempotent_close_count;
        RecordSuccess();
        return MakeResult(
            UiManagerPopupStackStatus::Success,
            panel_map_status,
            record,
            panel_id,
            false,
            false,
            false,
            already_in_stack,
            true,
            removed_from_stack);
    }

    if (stack_index >= snapshot_.popup_count) {
        return MakeResult(
            RecordFailure(UiManagerPopupStackStatus::PopupNotInStack),
            panel_map_status,
            record,
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const UiManagerPanelMapResult panel_result = panel_map->ClosePanel(panel_id);
    if (!panel_result.Succeeded()) {
        const UiManagerPopupStackStatus status = TranslatePanelMapStatus(panel_result.status);
        return MakeResult(
            RecordFailure(status),
            panel_result.status,
            panel_result.record,
            panel_id,
            false,
            false,
            false,
            true,
            false,
            false);
    }

    RemovePopupAt(stack_index);
    ++snapshot_.close_operation_count;
    RecordSuccess();
    return MakeResult(
        UiManagerPopupStackStatus::Success,
        panel_result.status,
        panel_result.record,
        panel_id,
        false,
        false,
        false,
        true,
        panel_result.already_inactive,
        true);
}

UiManagerPopupStackResult UiManagerPopupStack::ReleasePopupPanel(
    UiPanelId panel_id,
    UiManagerPanelMap *panel_map) {
    if (!panel_id.IsValid()) {
        return MakeResult(
            RecordFailure(UiManagerPopupStackStatus::InvalidPanelId),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    if (panel_map == nullptr) {
        return MakeResult(
            RecordFailure(UiManagerPopupStackStatus::InvalidPanelMap),
            UiManagerPanelMapStatus::Success,
            UiManagerPanelMapRecord{},
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    UiManagerPanelMapRecord record{};
    UiManagerPanelMapStatus panel_map_status = UiManagerPanelMapStatus::Success;
    const UiManagerPopupStackStatus loaded_status =
        ResolveLoadedPopupRecord(panel_id, *panel_map, &record, &panel_map_status);
    if (loaded_status != UiManagerPopupStackStatus::Success) {
        return MakeResult(
            RecordFailure(loaded_status),
            panel_map_status,
            record,
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const std::uint32_t stack_index = FindStackIndex(panel_id);
    if (record.active && stack_index >= snapshot_.popup_count) {
        return MakeResult(
            RecordFailure(UiManagerPopupStackStatus::PopupNotInStack),
            panel_map_status,
            record,
            panel_id,
            false,
            false,
            false,
            false,
            false,
            false);
    }

    const bool already_in_stack = stack_index < snapshot_.popup_count;
    const UiManagerPanelMapResult panel_result = panel_map->ReleasePanel(panel_id);
    if (!panel_result.Succeeded()) {
        const UiManagerPopupStackStatus status = TranslatePanelMapStatus(panel_result.status);
        return MakeResult(
            RecordFailure(status),
            panel_result.status,
            panel_result.record,
            panel_id,
            false,
            false,
            false,
            already_in_stack,
            !record.active,
            false);
    }

    bool removed_from_stack = false;
    if (already_in_stack) {
        RemovePopupAt(stack_index);
        removed_from_stack = true;
    }

    ++snapshot_.release_operation_count;
    RecordSuccess();
    return MakeResult(
        UiManagerPopupStackStatus::Success,
        panel_result.status,
        panel_result.record,
        panel_id,
        false,
        false,
        false,
        already_in_stack,
        panel_result.already_inactive,
        removed_from_stack);
}

UiManagerPopupStackStatus UiManagerPopupStack::ExportPopupOrder(
    UiPanelId *output_panel_ids,
    std::uint32_t output_capacity) const {
    if (snapshot_.popup_count > 0U && output_panel_ids == nullptr) {
        return UiManagerPopupStackStatus::InvalidOutputBuffer;
    }

    if (output_capacity < snapshot_.popup_count) {
        return UiManagerPopupStackStatus::InvalidOutputBuffer;
    }

    for (std::uint32_t index = 0U; index < snapshot_.popup_count; ++index) {
        output_panel_ids[index] = snapshot_.popup_order[index];
    }

    return UiManagerPopupStackStatus::Success;
}

bool UiManagerPopupStack::IsPopupStacked(UiPanelId panel_id) const {
    return FindStackIndex(panel_id) < snapshot_.popup_count;
}

UiManagerPopupStackSnapshot UiManagerPopupStack::Snapshot() const {
    return snapshot_;
}

UiManagerPopupStackStatus UiManagerPopupStack::ValidatePopupLayer(
    UiPanelId panel_id,
    const UiPanelRegistry &registry,
    const UiManagerLayerModel &layer_model,
    UiManagerLayerRecord *out_layer_record) const {
    if (out_layer_record == nullptr) {
        return UiManagerPopupStackStatus::InvalidOutputBuffer;
    }

    UiPanelManifestRecord manifest_record{};
    const UiPanelRegistryStatus registry_status = registry.ResolvePanel(panel_id, &manifest_record);
    const UiManagerPopupStackStatus panel_status = TranslateRegistryStatus(registry_status);
    if (panel_status != UiManagerPopupStackStatus::Success) {
        return panel_status;
    }

    UiManagerLayerRecord layer_record{};
    const UiManagerLayerModelStatus layer_status = layer_model.ResolvePanelLayer(panel_id, &layer_record);
    const UiManagerPopupStackStatus popup_layer_status = TranslateLayerStatus(layer_status);
    if (popup_layer_status != UiManagerPopupStackStatus::Success) {
        return popup_layer_status;
    }

    if (layer_record.type != UiManagerLayerType::Popup) {
        return UiManagerPopupStackStatus::NonPopupPanel;
    }

    *out_layer_record = layer_record;
    return UiManagerPopupStackStatus::Success;
}

UiManagerPopupStackStatus UiManagerPopupStack::ResolveLoadedPopupRecord(
    UiPanelId panel_id,
    const UiManagerPanelMap &panel_map,
    UiManagerPanelMapRecord *out_record,
    UiManagerPanelMapStatus *out_panel_map_status) const {
    if (out_record == nullptr || out_panel_map_status == nullptr) {
        return UiManagerPopupStackStatus::InvalidOutputBuffer;
    }

    UiManagerPanelMapRecord record{};
    const UiManagerPanelMapStatus panel_map_status = panel_map.ResolveLoadedPanel(panel_id, &record);
    *out_panel_map_status = panel_map_status;
    const UiManagerPopupStackStatus status = TranslatePanelMapStatus(panel_map_status);
    if (status != UiManagerPopupStackStatus::Success) {
        return status;
    }

    if (record.layer_record.type != UiManagerLayerType::Popup) {
        *out_record = record;
        return UiManagerPopupStackStatus::NonPopupPanel;
    }

    *out_record = record;
    return UiManagerPopupStackStatus::Success;
}

UiManagerPopupStackStatus UiManagerPopupStack::ResolveActivePopupRecord(
    UiPanelId panel_id,
    const UiManagerPanelMap &panel_map,
    UiManagerPanelMapRecord *out_record,
    UiManagerPanelMapStatus *out_panel_map_status) const {
    if (out_record == nullptr || out_panel_map_status == nullptr) {
        return UiManagerPopupStackStatus::InvalidOutputBuffer;
    }

    UiManagerPanelMapRecord record{};
    const UiManagerPanelMapStatus panel_map_status = panel_map.ResolveActivePanel(panel_id, &record);
    *out_panel_map_status = panel_map_status;
    const UiManagerPopupStackStatus status = TranslatePanelMapStatus(panel_map_status);
    if (status != UiManagerPopupStackStatus::Success) {
        return status;
    }

    if (record.layer_record.type != UiManagerLayerType::Popup) {
        *out_record = record;
        return UiManagerPopupStackStatus::NonPopupPanel;
    }

    *out_record = record;
    return UiManagerPopupStackStatus::Success;
}

UiManagerPopupStackStatus UiManagerPopupStack::TranslateRegistryStatus(UiPanelRegistryStatus status) const {
    if (status == UiPanelRegistryStatus::Success) {
        return UiManagerPopupStackStatus::Success;
    }

    if (status == UiPanelRegistryStatus::InvalidPanelId) {
        return UiManagerPopupStackStatus::InvalidPanelId;
    }

    if (status == UiPanelRegistryStatus::PanelNotFound) {
        return UiManagerPopupStackStatus::PanelNotRegistered;
    }

    if (status == UiPanelRegistryStatus::InvalidOutputBuffer) {
        return UiManagerPopupStackStatus::InvalidOutputBuffer;
    }

    if (status == UiPanelRegistryStatus::CapacityExceeded) {
        return UiManagerPopupStackStatus::CapacityExceeded;
    }

    return UiManagerPopupStackStatus::PanelNotRegistered;
}

UiManagerPopupStackStatus UiManagerPopupStack::TranslateLayerStatus(UiManagerLayerModelStatus status) const {
    if (status == UiManagerLayerModelStatus::Success) {
        return UiManagerPopupStackStatus::Success;
    }

    if (status == UiManagerLayerModelStatus::InvalidPanelId) {
        return UiManagerPopupStackStatus::InvalidPanelId;
    }

    if (status == UiManagerLayerModelStatus::PanelNotBound) {
        return UiManagerPopupStackStatus::PanelLayerNotBound;
    }

    if (status == UiManagerLayerModelStatus::LayerNotFound) {
        return UiManagerPopupStackStatus::LayerNotFound;
    }

    if (status == UiManagerLayerModelStatus::InvalidOutputBuffer) {
        return UiManagerPopupStackStatus::InvalidOutputBuffer;
    }

    if (status == UiManagerLayerModelStatus::CapacityExceeded) {
        return UiManagerPopupStackStatus::CapacityExceeded;
    }

    return UiManagerPopupStackStatus::PanelLayerNotBound;
}

UiManagerPopupStackStatus UiManagerPopupStack::TranslatePanelMapStatus(UiManagerPanelMapStatus status) const {
    if (status == UiManagerPanelMapStatus::Success) {
        return UiManagerPopupStackStatus::Success;
    }

    if (status == UiManagerPanelMapStatus::InvalidPanelId) {
        return UiManagerPopupStackStatus::InvalidPanelId;
    }

    if (status == UiManagerPanelMapStatus::InvalidController) {
        return UiManagerPopupStackStatus::InvalidController;
    }

    if (status == UiManagerPanelMapStatus::InvalidOutputBuffer) {
        return UiManagerPopupStackStatus::InvalidOutputBuffer;
    }

    if (status == UiManagerPanelMapStatus::PanelNotRegistered) {
        return UiManagerPopupStackStatus::PanelNotRegistered;
    }

    if (status == UiManagerPanelMapStatus::PanelNotLoaded) {
        return UiManagerPopupStackStatus::PanelNotLoaded;
    }

    if (status == UiManagerPanelMapStatus::PanelNotActive) {
        return UiManagerPopupStackStatus::PanelNotActive;
    }

    if (status == UiManagerPanelMapStatus::PanelLayerNotBound) {
        return UiManagerPopupStackStatus::PanelLayerNotBound;
    }

    if (status == UiManagerPanelMapStatus::LayerNotFound) {
        return UiManagerPopupStackStatus::LayerNotFound;
    }

    if (status == UiManagerPanelMapStatus::InvalidOpenArgs) {
        return UiManagerPopupStackStatus::InvalidOpenArgs;
    }

    if (status == UiManagerPanelMapStatus::ControllerOpenFailed) {
        return UiManagerPopupStackStatus::ControllerOpenFailed;
    }

    if (status == UiManagerPanelMapStatus::ControllerCloseFailed) {
        return UiManagerPopupStackStatus::ControllerCloseFailed;
    }

    if (status == UiManagerPanelMapStatus::ControllerReleaseFailed) {
        return UiManagerPopupStackStatus::ControllerReleaseFailed;
    }

    if (status == UiManagerPanelMapStatus::CapacityExceeded) {
        return UiManagerPopupStackStatus::CapacityExceeded;
    }

    return UiManagerPopupStackStatus::PanelMapRejected;
}

std::uint32_t UiManagerPopupStack::FindStackIndex(UiPanelId panel_id) const {
    if (!panel_id.IsValid()) {
        return MAX_UI_MANAGER_POPUP_STACK_COUNT;
    }

    for (std::uint32_t index = 0U; index < snapshot_.popup_count; ++index) {
        if (snapshot_.popup_order[index].value == panel_id.value) {
            return index;
        }
    }

    return MAX_UI_MANAGER_POPUP_STACK_COUNT;
}

void UiManagerPopupStack::PushPopup(UiPanelId panel_id) {
    snapshot_.popup_order[snapshot_.popup_count] = panel_id;
    ++snapshot_.popup_count;
    RefreshTopPanel();
}

void UiManagerPopupStack::MovePopupToTop(std::uint32_t stack_index) {
    if (stack_index >= snapshot_.popup_count) {
        return;
    }

    const UiPanelId panel_id = snapshot_.popup_order[stack_index];
    for (std::uint32_t index = stack_index; index + 1U < snapshot_.popup_count; ++index) {
        snapshot_.popup_order[index] = snapshot_.popup_order[index + 1U];
    }

    snapshot_.popup_order[snapshot_.popup_count - 1U] = panel_id;
    RefreshTopPanel();
}

void UiManagerPopupStack::RemovePopupAt(std::uint32_t stack_index) {
    if (stack_index >= snapshot_.popup_count) {
        return;
    }

    for (std::uint32_t index = stack_index; index + 1U < snapshot_.popup_count; ++index) {
        snapshot_.popup_order[index] = snapshot_.popup_order[index + 1U];
    }

    --snapshot_.popup_count;
    snapshot_.popup_order[snapshot_.popup_count] = UiPanelId{};
    RefreshTopPanel();
}

void UiManagerPopupStack::RefreshTopPanel() {
    snapshot_.top_panel_id = UiPanelId{};
    if (snapshot_.popup_count == 0U) {
        return;
    }

    snapshot_.top_panel_id = snapshot_.popup_order[snapshot_.popup_count - 1U];
}

UiManagerPopupStackResult UiManagerPopupStack::MakeResult(
    UiManagerPopupStackStatus status,
    UiManagerPanelMapStatus panel_map_status,
    const UiManagerPanelMapRecord &record,
    UiPanelId panel_id,
    bool pushed,
    bool brought_to_top,
    bool already_top,
    bool already_in_stack,
    bool already_inactive,
    bool removed_from_stack) const {
    UiManagerPopupStackResult result{};
    result.status = status;
    result.panel_map_status = panel_map_status;
    result.record = record;
    result.panel_id = panel_id;
    result.top_panel_id = snapshot_.top_panel_id;
    result.popup_count = snapshot_.popup_count;
    result.pushed = pushed;
    result.brought_to_top = brought_to_top;
    result.already_top = already_top;
    result.already_in_stack = already_in_stack;
    result.already_inactive = already_inactive;
    result.removed_from_stack = removed_from_stack;
    return result;
}

UiManagerPopupStackStatus UiManagerPopupStack::RecordFailure(UiManagerPopupStackStatus status) {
    ++snapshot_.rejected_operation_count;
    snapshot_.last_status = status;
    return status;
}

void UiManagerPopupStack::RecordSuccess() {
    ++snapshot_.accepted_operation_count;
    snapshot_.last_status = UiManagerPopupStackStatus::Success;
}
}
