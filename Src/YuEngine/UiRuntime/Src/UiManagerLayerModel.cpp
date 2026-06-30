// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Src/UiManagerLayerModel.cpp

#include "YuEngine/UiRuntime/UiManagerLayerModel.h"

#include <cstddef>

namespace yuengine::uiruntime {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

UiManagerLayerModel::UiManagerLayerModel()
    : UiManagerLayerModel(UiManagerLayerModelDesc{}) {
}

UiManagerLayerModel::UiManagerLayerModel(UiManagerLayerModelDesc desc)
    : layer_records_{},
      used_layer_records_{},
      binding_records_{},
      used_binding_records_{},
      snapshot_{ClampCapacity(desc.layer_capacity, MAX_UI_MANAGER_LAYER_COUNT),
                ClampCapacity(desc.binding_capacity, MAX_UI_MANAGER_PANEL_LAYER_BINDING_COUNT),
                0U,
                0U,
                0U,
                0U,
                0U,
                0U,
                0U,
                UiManagerLayerModelStatus::Success} {
}

UiManagerLayerModelResult UiManagerLayerModel::RegisterLayer(const UiManagerLayerRecord &record) {
    UiManagerLayerModelStatus status = ValidateLayerRecord(record);
    if (status != UiManagerLayerModelStatus::Success) {
        return MakeResult(RecordFailure(status), record, UiManagerPanelLayerBinding{}, 0U);
    }

    if (HasLayer(record.layer_id)) {
        ++snapshot_.duplicate_layer_rejected_count;
        return MakeResult(
            RecordFailure(UiManagerLayerModelStatus::DuplicateLayerId),
            record,
            UiManagerPanelLayerBinding{},
            0U);
    }

    if (HasLayerType(record.type)) {
        ++snapshot_.duplicate_layer_rejected_count;
        return MakeResult(
            RecordFailure(UiManagerLayerModelStatus::DuplicateLayerType),
            record,
            UiManagerPanelLayerBinding{},
            0U);
    }

    if (snapshot_.registered_layer_count >= snapshot_.layer_capacity) {
        return MakeResult(
            RecordFailure(UiManagerLayerModelStatus::CapacityExceeded),
            record,
            UiManagerPanelLayerBinding{},
            0U);
    }

    UiManagerLayerModelResult result = InsertLayerRecord(record);
    RecordSuccess();
    result.status = UiManagerLayerModelStatus::Success;
    result.required_layer_count = snapshot_.registered_layer_count;
    result.required_binding_count = snapshot_.panel_binding_count;
    return result;
}

UiManagerLayerModelResult UiManagerLayerModel::RegisterLayerSet(const UiManagerLayerSet &layer_set) {
    const UiManagerLayerModelStatus status = ValidateLayerSet(layer_set);
    if (status != UiManagerLayerModelStatus::Success) {
        if (status == UiManagerLayerModelStatus::DuplicateLayerId ||
            status == UiManagerLayerModelStatus::DuplicateLayerType) {
            ++snapshot_.duplicate_layer_rejected_count;
        }

        return MakeResult(
            RecordFailure(status),
            UiManagerLayerRecord{},
            UiManagerPanelLayerBinding{},
            0U);
    }

    const std::uint32_t layer_count = static_cast<std::uint32_t>(layer_set.records.size());
    if (snapshot_.registered_layer_count + layer_count > snapshot_.layer_capacity) {
        return MakeResult(
            RecordFailure(UiManagerLayerModelStatus::CapacityExceeded),
            UiManagerLayerRecord{},
            UiManagerPanelLayerBinding{},
            0U);
    }

    UiManagerLayerModelResult result{};
    for (const UiManagerLayerRecord &record : layer_set.records) {
        result = InsertLayerRecord(record);
    }

    RecordSuccess();
    result.status = UiManagerLayerModelStatus::Success;
    result.required_layer_count = snapshot_.registered_layer_count;
    result.required_binding_count = snapshot_.panel_binding_count;
    return result;
}

UiManagerLayerModelResult UiManagerLayerModel::BindPanelToLayer(const UiManagerPanelLayerBinding &binding) {
    UiManagerLayerModelStatus status = ValidateBinding(binding);
    if (status != UiManagerLayerModelStatus::Success) {
        return MakeResult(RecordFailure(status), UiManagerLayerRecord{}, binding, 0U);
    }

    if (FindLayer(binding.layer_id) == nullptr) {
        return MakeResult(
            RecordFailure(UiManagerLayerModelStatus::LayerNotFound),
            UiManagerLayerRecord{},
            binding,
            0U);
    }

    if (HasPanelBinding(binding.panel_id)) {
        ++snapshot_.duplicate_panel_rejected_count;
        return MakeResult(
            RecordFailure(UiManagerLayerModelStatus::DuplicatePanelId),
            UiManagerLayerRecord{},
            binding,
            0U);
    }

    if (snapshot_.panel_binding_count >= snapshot_.binding_capacity) {
        return MakeResult(
            RecordFailure(UiManagerLayerModelStatus::CapacityExceeded),
            UiManagerLayerRecord{},
            binding,
            0U);
    }

    UiManagerLayerModelResult result = InsertBindingRecord(binding);
    RecordSuccess();
    result.status = UiManagerLayerModelStatus::Success;
    result.required_layer_count = snapshot_.registered_layer_count;
    result.required_binding_count = snapshot_.panel_binding_count;
    return result;
}

UiManagerLayerModelStatus UiManagerLayerModel::ResolveLayer(
    UiManagerLayerId layer_id,
    UiManagerLayerRecord *out_record) const {
    if (out_record == nullptr) {
        return UiManagerLayerModelStatus::InvalidOutputBuffer;
    }

    if (!layer_id.IsValid()) {
        return UiManagerLayerModelStatus::InvalidLayerId;
    }

    const UiManagerLayerRecord *record = FindLayer(layer_id);
    if (record == nullptr) {
        return UiManagerLayerModelStatus::LayerNotFound;
    }

    *out_record = *record;
    return UiManagerLayerModelStatus::Success;
}

UiManagerLayerModelStatus UiManagerLayerModel::ResolveLayerByType(
    UiManagerLayerType type,
    UiManagerLayerRecord *out_record) const {
    if (out_record == nullptr) {
        return UiManagerLayerModelStatus::InvalidOutputBuffer;
    }

    const UiManagerLayerModelStatus status = ValidateLayerType(type);
    if (status != UiManagerLayerModelStatus::Success) {
        return status;
    }

    const UiManagerLayerRecord *record = FindLayerByType(type);
    if (record == nullptr) {
        return UiManagerLayerModelStatus::LayerNotFound;
    }

    *out_record = *record;
    return UiManagerLayerModelStatus::Success;
}

UiManagerLayerModelStatus UiManagerLayerModel::ResolvePanelLayer(
    UiPanelId panel_id,
    UiManagerLayerRecord *out_record) const {
    if (out_record == nullptr) {
        return UiManagerLayerModelStatus::InvalidOutputBuffer;
    }

    if (!panel_id.IsValid()) {
        return UiManagerLayerModelStatus::InvalidPanelId;
    }

    const UiManagerPanelLayerBinding *binding = FindBinding(panel_id);
    if (binding == nullptr) {
        return UiManagerLayerModelStatus::PanelNotBound;
    }

    const UiManagerLayerRecord *record = FindLayer(binding->layer_id);
    if (record == nullptr) {
        return UiManagerLayerModelStatus::LayerNotFound;
    }

    *out_record = *record;
    return UiManagerLayerModelStatus::Success;
}

UiManagerLayerModelResult UiManagerLayerModel::ExportLayers(
    UiManagerLayerRecord *output_records,
    std::uint32_t output_capacity) const {
    UiManagerLayerModelResult result{};
    result.status = UiManagerLayerModelStatus::Success;
    result.required_layer_count = snapshot_.registered_layer_count;
    result.required_binding_count = snapshot_.panel_binding_count;

    if (snapshot_.registered_layer_count == 0U) {
        return result;
    }

    if (output_records == nullptr) {
        result.status = UiManagerLayerModelStatus::InvalidOutputBuffer;
        return result;
    }

    if (output_capacity < snapshot_.registered_layer_count) {
        result.status = UiManagerLayerModelStatus::InvalidOutputBuffer;
        return result;
    }

    std::array<UiManagerLayerRecord, MAX_UI_MANAGER_LAYER_COUNT> sorted_records{};
    std::uint32_t sorted_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.layer_capacity; ++index) {
        if (!used_layer_records_[index]) {
            continue;
        }

        InsertSortedLayerRecord(layer_records_[index], sorted_records, &sorted_count);
    }

    for (std::uint32_t index = 0U; index < sorted_count; ++index) {
        output_records[index] = sorted_records[index];
    }

    return result;
}

UiManagerLayerModelSnapshot UiManagerLayerModel::Snapshot() const {
    return snapshot_;
}

UiManagerLayerModelStatus UiManagerLayerModel::ValidateLayerType(UiManagerLayerType type) const {
    if (type == UiManagerLayerType::Game) {
        return UiManagerLayerModelStatus::Success;
    }

    if (type == UiManagerLayerType::Fullscreen) {
        return UiManagerLayerModelStatus::Success;
    }

    if (type == UiManagerLayerType::Popup) {
        return UiManagerLayerModelStatus::Success;
    }

    if (type == UiManagerLayerType::Loading) {
        return UiManagerLayerModelStatus::Success;
    }

    if (type == UiManagerLayerType::Debug) {
        return UiManagerLayerModelStatus::Success;
    }

    return UiManagerLayerModelStatus::InvalidLayerType;
}

UiManagerLayerModelStatus UiManagerLayerModel::ValidateLayerRecord(const UiManagerLayerRecord &record) const {
    if (!record.layer_id.IsValid()) {
        return UiManagerLayerModelStatus::InvalidLayerId;
    }

    const UiManagerLayerModelStatus type_status = ValidateLayerType(record.type);
    if (type_status != UiManagerLayerModelStatus::Success) {
        return type_status;
    }

    if (!record.root_ref.IsValid()) {
        return UiManagerLayerModelStatus::InvalidLayerRoot;
    }

    return UiManagerLayerModelStatus::Success;
}

UiManagerLayerModelStatus UiManagerLayerModel::ValidateLayerSet(const UiManagerLayerSet &layer_set) const {
    if (layer_set.records.empty()) {
        return UiManagerLayerModelStatus::InvalidDesc;
    }

    if (layer_set.records.size() > static_cast<std::size_t>(MAX_UI_MANAGER_LAYER_COUNT)) {
        return UiManagerLayerModelStatus::CapacityExceeded;
    }

    for (std::size_t left_index = 0U; left_index < layer_set.records.size(); ++left_index) {
        const UiManagerLayerRecord &left_record = layer_set.records[left_index];
        const UiManagerLayerModelStatus status = ValidateLayerRecord(left_record);
        if (status != UiManagerLayerModelStatus::Success) {
            return status;
        }

        if (HasLayer(left_record.layer_id)) {
            return UiManagerLayerModelStatus::DuplicateLayerId;
        }

        if (HasLayerType(left_record.type)) {
            return UiManagerLayerModelStatus::DuplicateLayerType;
        }

        for (std::size_t right_index = left_index + 1U; right_index < layer_set.records.size(); ++right_index) {
            const UiManagerLayerRecord &right_record = layer_set.records[right_index];
            if (left_record.layer_id.value == right_record.layer_id.value) {
                return UiManagerLayerModelStatus::DuplicateLayerId;
            }

            if (left_record.type == right_record.type) {
                return UiManagerLayerModelStatus::DuplicateLayerType;
            }
        }
    }

    return UiManagerLayerModelStatus::Success;
}

UiManagerLayerModelStatus UiManagerLayerModel::ValidateBinding(
    const UiManagerPanelLayerBinding &binding) const {
    if (!binding.panel_id.IsValid()) {
        return UiManagerLayerModelStatus::InvalidPanelId;
    }

    if (!binding.layer_id.IsValid()) {
        return UiManagerLayerModelStatus::InvalidLayerId;
    }

    return UiManagerLayerModelStatus::Success;
}

bool UiManagerLayerModel::HasLayer(UiManagerLayerId layer_id) const {
    return FindLayer(layer_id) != nullptr;
}

bool UiManagerLayerModel::HasLayerType(UiManagerLayerType type) const {
    return FindLayerByType(type) != nullptr;
}

bool UiManagerLayerModel::HasPanelBinding(UiPanelId panel_id) const {
    return FindBinding(panel_id) != nullptr;
}

const UiManagerLayerRecord *UiManagerLayerModel::FindLayer(UiManagerLayerId layer_id) const {
    if (!layer_id.IsValid()) {
        return nullptr;
    }

    for (std::uint32_t index = 0U; index < snapshot_.layer_capacity; ++index) {
        if (!used_layer_records_[index]) {
            continue;
        }

        if (layer_records_[index].layer_id.value == layer_id.value) {
            return &layer_records_[index];
        }
    }

    return nullptr;
}

const UiManagerLayerRecord *UiManagerLayerModel::FindLayerByType(UiManagerLayerType type) const {
    if (ValidateLayerType(type) != UiManagerLayerModelStatus::Success) {
        return nullptr;
    }

    for (std::uint32_t index = 0U; index < snapshot_.layer_capacity; ++index) {
        if (!used_layer_records_[index]) {
            continue;
        }

        if (layer_records_[index].type == type) {
            return &layer_records_[index];
        }
    }

    return nullptr;
}

const UiManagerPanelLayerBinding *UiManagerLayerModel::FindBinding(UiPanelId panel_id) const {
    if (!panel_id.IsValid()) {
        return nullptr;
    }

    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        if (!used_binding_records_[index]) {
            continue;
        }

        if (binding_records_[index].panel_id.value == panel_id.value) {
            return &binding_records_[index];
        }
    }

    return nullptr;
}

std::uint32_t UiManagerLayerModel::FindFreeLayerRecordIndex() const {
    for (std::uint32_t index = 0U; index < snapshot_.layer_capacity; ++index) {
        if (!used_layer_records_[index]) {
            return index;
        }
    }

    return MAX_UI_MANAGER_LAYER_COUNT;
}

std::uint32_t UiManagerLayerModel::FindFreeBindingRecordIndex() const {
    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        if (!used_binding_records_[index]) {
            return index;
        }
    }

    return MAX_UI_MANAGER_PANEL_LAYER_BINDING_COUNT;
}

UiManagerLayerModelResult UiManagerLayerModel::InsertLayerRecord(const UiManagerLayerRecord &record) {
    const std::uint32_t record_index = FindFreeLayerRecordIndex();
    if (record_index >= snapshot_.layer_capacity) {
        return MakeResult(
            UiManagerLayerModelStatus::CapacityExceeded,
            record,
            UiManagerPanelLayerBinding{},
            record_index);
    }

    layer_records_[record_index] = record;
    used_layer_records_[record_index] = true;
    ++snapshot_.registered_layer_count;
    return MakeResult(
        UiManagerLayerModelStatus::Success,
        record,
        UiManagerPanelLayerBinding{},
        record_index);
}

UiManagerLayerModelResult UiManagerLayerModel::InsertBindingRecord(const UiManagerPanelLayerBinding &binding) {
    const std::uint32_t record_index = FindFreeBindingRecordIndex();
    if (record_index >= snapshot_.binding_capacity) {
        return MakeResult(
            UiManagerLayerModelStatus::CapacityExceeded,
            UiManagerLayerRecord{},
            binding,
            record_index);
    }

    binding_records_[record_index] = binding;
    used_binding_records_[record_index] = true;
    ++snapshot_.panel_binding_count;
    return MakeResult(
        UiManagerLayerModelStatus::Success,
        UiManagerLayerRecord{},
        binding,
        record_index);
}

void UiManagerLayerModel::InsertSortedLayerRecord(
    const UiManagerLayerRecord &record,
    std::array<UiManagerLayerRecord, MAX_UI_MANAGER_LAYER_COUNT> &sorted_records,
    std::uint32_t *sorted_count) const {
    if (sorted_count == nullptr) {
        return;
    }

    std::uint32_t write_index = *sorted_count;
    while (write_index > 0U) {
        const std::uint32_t previous_index = write_index - 1U;
        if (sorted_records[previous_index].order <= record.order) {
            break;
        }

        sorted_records[write_index] = sorted_records[previous_index];
        write_index = previous_index;
    }

    sorted_records[write_index] = record;
    ++(*sorted_count);
}

UiManagerLayerModelResult UiManagerLayerModel::MakeResult(
    UiManagerLayerModelStatus status,
    const UiManagerLayerRecord &layer_record,
    const UiManagerPanelLayerBinding &binding_record,
    std::uint32_t record_index) const {
    UiManagerLayerModelResult result{};
    result.status = status;
    result.layer_record = layer_record;
    result.binding_record = binding_record;
    result.record_index = record_index;
    result.required_layer_count = snapshot_.registered_layer_count;
    result.required_binding_count = snapshot_.panel_binding_count;
    return result;
}

UiManagerLayerModelStatus UiManagerLayerModel::RecordFailure(UiManagerLayerModelStatus status) {
    ++snapshot_.failed_operation_count;
    ++snapshot_.rejected_operation_count;
    snapshot_.last_status = status;
    return status;
}

void UiManagerLayerModel::RecordSuccess() {
    ++snapshot_.accepted_operation_count;
    snapshot_.last_status = UiManagerLayerModelStatus::Success;
}
}
