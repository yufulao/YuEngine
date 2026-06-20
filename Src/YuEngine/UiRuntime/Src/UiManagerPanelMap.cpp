// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Src/UiManagerPanelMap.cpp

#include "YuEngine/UiRuntime/UiManagerPanelMap.h"

#include "YuEngine/UiRuntime/BaseUiLifecycleStatus.h"

namespace yuengine::uiruntime {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

UiManagerPanelMap::UiManagerPanelMap()
    : UiManagerPanelMap(UiManagerPanelMapDesc{}) {
}

UiManagerPanelMap::UiManagerPanelMap(UiManagerPanelMapDesc desc)
    : records_{},
      snapshot_{ClampCapacity(desc.panel_capacity, MAX_UI_MANAGER_PANEL_MAP_RECORD_COUNT),
                0U,
                0U,
                0U,
                0U,
                0U,
                0U,
                0U,
                0U,
                0U,
                UiManagerPanelMapStatus::Success} {
}

UiManagerPanelMapResult UiManagerPanelMap::OpenPanel(
    UiPanelId panel_id,
    const UiPanelRegistry &registry,
    const UiManagerLayerModel &layer_model,
    BaseUiController *controller) {
    if (!panel_id.IsValid()) {
        return MakeResult(RecordFailure(UiManagerPanelMapStatus::InvalidPanelId), UiManagerPanelMapRecord{}, false, false, false);
    }

    UiManagerPanelMapRecord *loaded_record = FindLoadedRecord(panel_id);
    if (loaded_record != nullptr) {
        if (loaded_record->active) {
            ++snapshot_.idempotent_open_count;
            RecordSuccess();
            return MakeResult(UiManagerPanelMapStatus::Success, *loaded_record, true, true, false);
        }

        if (loaded_record->controller == nullptr) {
            return MakeResult(RecordFailure(UiManagerPanelMapStatus::InvalidController), *loaded_record, true, false, false);
        }

        const BaseUiLifecycleStatus lifecycle_status = loaded_record->controller->Open();
        if (lifecycle_status != BaseUiLifecycleStatus::Success) {
            return MakeResult(RecordFailure(UiManagerPanelMapStatus::ControllerOpenFailed), *loaded_record, true, false, false);
        }

        loaded_record->active = true;
        ++snapshot_.active_panel_count;
        ++snapshot_.open_operation_count;
        ++snapshot_.reused_loaded_count;
        RecordSuccess();
        return MakeResult(UiManagerPanelMapStatus::Success, *loaded_record, true, false, false);
    }

    UiPanelManifestRecord manifest_record{};
    UiManagerLayerRecord layer_record{};
    const UiManagerPanelMapStatus resolve_status =
        ResolveManifestAndLayer(panel_id, registry, layer_model, &manifest_record, &layer_record);
    if (resolve_status != UiManagerPanelMapStatus::Success) {
        return MakeResult(RecordFailure(resolve_status), UiManagerPanelMapRecord{}, false, false, false);
    }

    if (controller == nullptr) {
        return MakeResult(RecordFailure(UiManagerPanelMapStatus::InvalidController), UiManagerPanelMapRecord{}, false, false, false);
    }

    if (snapshot_.loaded_panel_count >= snapshot_.panel_capacity) {
        return MakeResult(RecordFailure(UiManagerPanelMapStatus::CapacityExceeded), UiManagerPanelMapRecord{}, false, false, false);
    }

    const std::uint32_t record_index = FindFreeRecordIndex();
    if (record_index >= snapshot_.panel_capacity) {
        return MakeResult(RecordFailure(UiManagerPanelMapStatus::CapacityExceeded), UiManagerPanelMapRecord{}, false, false, false);
    }

    const BaseUiLifecycleStatus lifecycle_status = controller->Open();
    if (lifecycle_status != BaseUiLifecycleStatus::Success) {
        return MakeResult(RecordFailure(UiManagerPanelMapStatus::ControllerOpenFailed), UiManagerPanelMapRecord{}, false, false, false);
    }

    UiManagerPanelMapRecord record{};
    record.panel_id = panel_id;
    record.manifest_record = manifest_record;
    record.layer_record = layer_record;
    record.controller = controller;
    record.loaded = true;
    record.active = true;
    records_[record_index] = record;
    ++snapshot_.loaded_panel_count;
    ++snapshot_.active_panel_count;
    ++snapshot_.open_operation_count;
    RecordSuccess();
    return MakeResult(UiManagerPanelMapStatus::Success, records_[record_index], false, false, false);
}

UiManagerPanelMapResult UiManagerPanelMap::ClosePanel(UiPanelId panel_id) {
    if (!panel_id.IsValid()) {
        return MakeResult(RecordFailure(UiManagerPanelMapStatus::InvalidPanelId), UiManagerPanelMapRecord{}, false, false, false);
    }

    UiManagerPanelMapRecord *loaded_record = FindLoadedRecord(panel_id);
    if (loaded_record == nullptr) {
        return MakeResult(RecordFailure(UiManagerPanelMapStatus::PanelNotLoaded), UiManagerPanelMapRecord{}, false, false, false);
    }

    if (!loaded_record->active) {
        ++snapshot_.idempotent_close_count;
        RecordSuccess();
        return MakeResult(UiManagerPanelMapStatus::Success, *loaded_record, true, false, true);
    }

    if (loaded_record->controller == nullptr) {
        return MakeResult(RecordFailure(UiManagerPanelMapStatus::InvalidController), *loaded_record, true, false, false);
    }

    const BaseUiLifecycleStatus lifecycle_status = loaded_record->controller->Close();
    if (lifecycle_status != BaseUiLifecycleStatus::Success) {
        return MakeResult(RecordFailure(UiManagerPanelMapStatus::ControllerCloseFailed), *loaded_record, true, false, false);
    }

    loaded_record->active = false;
    if (snapshot_.active_panel_count > 0U) {
        --snapshot_.active_panel_count;
    }

    ++snapshot_.close_operation_count;
    RecordSuccess();
    return MakeResult(UiManagerPanelMapStatus::Success, *loaded_record, true, false, false);
}

UiManagerPanelMapStatus UiManagerPanelMap::ResolveLoadedPanel(
    UiPanelId panel_id,
    UiManagerPanelMapRecord *out_record) const {
    if (out_record == nullptr) {
        return UiManagerPanelMapStatus::InvalidOutputBuffer;
    }

    if (!panel_id.IsValid()) {
        return UiManagerPanelMapStatus::InvalidPanelId;
    }

    const UiManagerPanelMapRecord *record = FindLoadedRecord(panel_id);
    if (record == nullptr) {
        return UiManagerPanelMapStatus::PanelNotLoaded;
    }

    *out_record = *record;
    return UiManagerPanelMapStatus::Success;
}

UiManagerPanelMapStatus UiManagerPanelMap::ResolveActivePanel(
    UiPanelId panel_id,
    UiManagerPanelMapRecord *out_record) const {
    if (out_record == nullptr) {
        return UiManagerPanelMapStatus::InvalidOutputBuffer;
    }

    if (!panel_id.IsValid()) {
        return UiManagerPanelMapStatus::InvalidPanelId;
    }

    const UiManagerPanelMapRecord *record = FindLoadedRecord(panel_id);
    if (record == nullptr) {
        return UiManagerPanelMapStatus::PanelNotLoaded;
    }

    if (!record->active) {
        return UiManagerPanelMapStatus::PanelNotActive;
    }

    *out_record = *record;
    return UiManagerPanelMapStatus::Success;
}

bool UiManagerPanelMap::IsLoaded(UiPanelId panel_id) const {
    return FindLoadedRecord(panel_id) != nullptr;
}

bool UiManagerPanelMap::IsActive(UiPanelId panel_id) const {
    const UiManagerPanelMapRecord *record = FindLoadedRecord(panel_id);
    if (record == nullptr) {
        return false;
    }

    return record->active;
}

UiManagerPanelMapSnapshot UiManagerPanelMap::Snapshot() const {
    return snapshot_;
}

UiManagerPanelMapStatus UiManagerPanelMap::ResolveManifestAndLayer(
    UiPanelId panel_id,
    const UiPanelRegistry &registry,
    const UiManagerLayerModel &layer_model,
    UiPanelManifestRecord *out_manifest_record,
    UiManagerLayerRecord *out_layer_record) const {
    if (out_manifest_record == nullptr || out_layer_record == nullptr) {
        return UiManagerPanelMapStatus::InvalidOutputBuffer;
    }

    UiPanelManifestRecord manifest_record{};
    const UiPanelRegistryStatus registry_status = registry.ResolvePanel(panel_id, &manifest_record);
    const UiManagerPanelMapStatus panel_status = TranslateRegistryStatus(registry_status);
    if (panel_status != UiManagerPanelMapStatus::Success) {
        return panel_status;
    }

    UiManagerLayerRecord layer_record{};
    const UiManagerLayerModelStatus layer_status = layer_model.ResolvePanelLayer(panel_id, &layer_record);
    const UiManagerPanelMapStatus panel_layer_status = TranslateLayerStatus(layer_status);
    if (panel_layer_status != UiManagerPanelMapStatus::Success) {
        return panel_layer_status;
    }

    *out_manifest_record = manifest_record;
    *out_layer_record = layer_record;
    return UiManagerPanelMapStatus::Success;
}

UiManagerPanelMapStatus UiManagerPanelMap::TranslateRegistryStatus(UiPanelRegistryStatus status) const {
    if (status == UiPanelRegistryStatus::Success) {
        return UiManagerPanelMapStatus::Success;
    }

    if (status == UiPanelRegistryStatus::InvalidPanelId) {
        return UiManagerPanelMapStatus::InvalidPanelId;
    }

    if (status == UiPanelRegistryStatus::PanelNotFound) {
        return UiManagerPanelMapStatus::PanelNotRegistered;
    }

    if (status == UiPanelRegistryStatus::InvalidOutputBuffer) {
        return UiManagerPanelMapStatus::InvalidOutputBuffer;
    }

    return UiManagerPanelMapStatus::PanelNotRegistered;
}

UiManagerPanelMapStatus UiManagerPanelMap::TranslateLayerStatus(UiManagerLayerModelStatus status) const {
    if (status == UiManagerLayerModelStatus::Success) {
        return UiManagerPanelMapStatus::Success;
    }

    if (status == UiManagerLayerModelStatus::InvalidPanelId) {
        return UiManagerPanelMapStatus::InvalidPanelId;
    }

    if (status == UiManagerLayerModelStatus::PanelNotBound) {
        return UiManagerPanelMapStatus::PanelLayerNotBound;
    }

    if (status == UiManagerLayerModelStatus::LayerNotFound) {
        return UiManagerPanelMapStatus::LayerNotFound;
    }

    if (status == UiManagerLayerModelStatus::InvalidOutputBuffer) {
        return UiManagerPanelMapStatus::InvalidOutputBuffer;
    }

    return UiManagerPanelMapStatus::PanelLayerNotBound;
}

UiManagerPanelMapRecord *UiManagerPanelMap::FindLoadedRecord(UiPanelId panel_id) {
    if (!panel_id.IsValid()) {
        return nullptr;
    }

    for (std::uint32_t index = 0U; index < snapshot_.panel_capacity; ++index) {
        if (!records_[index].loaded) {
            continue;
        }

        if (records_[index].panel_id.value == panel_id.value) {
            return &records_[index];
        }
    }

    return nullptr;
}

const UiManagerPanelMapRecord *UiManagerPanelMap::FindLoadedRecord(UiPanelId panel_id) const {
    if (!panel_id.IsValid()) {
        return nullptr;
    }

    for (std::uint32_t index = 0U; index < snapshot_.panel_capacity; ++index) {
        if (!records_[index].loaded) {
            continue;
        }

        if (records_[index].panel_id.value == panel_id.value) {
            return &records_[index];
        }
    }

    return nullptr;
}

std::uint32_t UiManagerPanelMap::FindFreeRecordIndex() const {
    for (std::uint32_t index = 0U; index < snapshot_.panel_capacity; ++index) {
        if (!records_[index].loaded) {
            return index;
        }
    }

    return MAX_UI_MANAGER_PANEL_MAP_RECORD_COUNT;
}

UiManagerPanelMapResult UiManagerPanelMap::MakeResult(
    UiManagerPanelMapStatus status,
    const UiManagerPanelMapRecord &record,
    bool reused_loaded,
    bool already_active,
    bool already_inactive) const {
    UiManagerPanelMapResult result{};
    result.status = status;
    result.record = record;
    result.loaded_panel_count = snapshot_.loaded_panel_count;
    result.active_panel_count = snapshot_.active_panel_count;
    result.reused_loaded = reused_loaded;
    result.already_active = already_active;
    result.already_inactive = already_inactive;
    return result;
}

UiManagerPanelMapStatus UiManagerPanelMap::RecordFailure(UiManagerPanelMapStatus status) {
    ++snapshot_.rejected_operation_count;
    snapshot_.last_status = status;
    return status;
}

void UiManagerPanelMap::RecordSuccess() {
    ++snapshot_.accepted_operation_count;
    snapshot_.last_status = UiManagerPanelMapStatus::Success;
}
}
