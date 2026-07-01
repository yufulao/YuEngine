// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Src/UiPanelRegistry.cpp

#include "YuEngine/UiRuntime/UiPanelRegistry.h"

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

UiPanelRegistry::UiPanelRegistry()
    : UiPanelRegistry(UiPanelRegistryDesc{}) {
}

UiPanelRegistry::UiPanelRegistry(UiPanelRegistryDesc desc)
    : records_{},
      active_records_{},
      snapshot_{ClampCapacity(desc.panel_capacity, MAX_UI_PANEL_REGISTRY_RECORD_COUNT),
                0U,
                0U,
                0U,
                0U,
                0U,
                UiPanelRegistryStatus::Success,
                0U,
                UiPanelId{},
                0U} {
}

UiPanelRegistryResult UiPanelRegistry::RegisterPanel(const UiPanelManifestRecord &record) {
    const UiPanelRegistryStatus status = ValidateRecord(record);
    if (status != UiPanelRegistryStatus::Success) {
        return MakeResult(RecordFailure(status), record, 0U);
    }

    if (HasPanel(record.panel_id)) {
        ++snapshot_.duplicate_panel_rejected_count;
        return MakeResult(RecordFailure(UiPanelRegistryStatus::DuplicatePanelId), record, 0U);
    }

    if (snapshot_.registered_panel_count >= snapshot_.panel_capacity) {
        const std::uint32_t required_record_count = snapshot_.registered_panel_count + 1U;
        const std::uint32_t failed_record_index = snapshot_.registered_panel_count;
        return MakeCapacityResult(record, failed_record_index, required_record_count);
    }

    UiPanelRegistryResult result = InsertRecord(record);
    RecordSuccess();
    result.status = UiPanelRegistryStatus::Success;
    result.required_record_count = snapshot_.registered_panel_count;
    return result;
}

UiPanelRegistryResult UiPanelRegistry::RegisterManifest(const UiPanelTestManifest &manifest) {
    UiPanelRegistryStatus status = ValidateManifest(manifest);
    if (status != UiPanelRegistryStatus::Success) {
        if (status == UiPanelRegistryStatus::DuplicatePanelId) {
            ++snapshot_.duplicate_panel_rejected_count;
        }

        if (status == UiPanelRegistryStatus::CapacityExceeded) {
            const auto failed_record_index = static_cast<std::uint32_t>(MAX_UI_PANEL_REGISTRY_RECORD_COUNT);
            const auto required_record_count = static_cast<std::uint32_t>(manifest.records.size());
            const UiPanelManifestRecord &failed_record = manifest.records[failed_record_index];
            return MakeCapacityResult(failed_record, failed_record_index, required_record_count);
        }

        return MakeResult(RecordFailure(status), UiPanelManifestRecord{}, 0U);
    }

    const std::uint32_t manifest_count = static_cast<std::uint32_t>(manifest.records.size());
    const std::uint32_t required_record_count = snapshot_.registered_panel_count + manifest_count;
    if (required_record_count > snapshot_.panel_capacity) {
        const std::uint32_t failed_record_index = snapshot_.panel_capacity - snapshot_.registered_panel_count;
        const UiPanelManifestRecord &failed_record = manifest.records[failed_record_index];
        return MakeCapacityResult(failed_record, failed_record_index, required_record_count);
    }

    UiPanelRegistryResult result{};
    for (const UiPanelManifestRecord &record : manifest.records) {
        result = InsertRecord(record);
    }

    RecordSuccess();
    result.status = UiPanelRegistryStatus::Success;
    result.required_record_count = snapshot_.registered_panel_count;
    return result;
}

UiPanelRegistryStatus UiPanelRegistry::ResolvePanel(UiPanelId panel_id, UiPanelManifestRecord *out_record) const {
    if (out_record == nullptr) {
        return UiPanelRegistryStatus::InvalidOutputBuffer;
    }

    if (!panel_id.IsValid()) {
        return UiPanelRegistryStatus::InvalidPanelId;
    }

    const UiPanelManifestRecord *record = FindRecord(panel_id);
    if (record == nullptr) {
        return UiPanelRegistryStatus::PanelNotFound;
    }

    *out_record = *record;
    return UiPanelRegistryStatus::Success;
}

UiPanelRegistryResult UiPanelRegistry::ExportManifest(
    UiPanelManifestRecord *output_records,
    std::uint32_t output_capacity) const {
    UiPanelRegistryResult result{};
    result.status = UiPanelRegistryStatus::Success;
    result.required_record_count = snapshot_.registered_panel_count;

    if (snapshot_.registered_panel_count == 0U) {
        return result;
    }

    if (output_records == nullptr) {
        result.status = UiPanelRegistryStatus::InvalidOutputBuffer;
        return result;
    }

    if (output_capacity < snapshot_.registered_panel_count) {
        result.status = UiPanelRegistryStatus::InvalidOutputBuffer;
        return result;
    }

    std::uint32_t output_index = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.panel_capacity; ++index) {
        if (!active_records_[index]) {
            continue;
        }

        output_records[output_index] = records_[index];
        ++output_index;
    }

    return result;
}

UiPanelRegistrySnapshot UiPanelRegistry::Snapshot() const {
    return snapshot_;
}

UiPanelRegistryStatus UiPanelRegistry::ValidateRecord(const UiPanelManifestRecord &record) const {
    if (!record.panel_id.IsValid()) {
        return UiPanelRegistryStatus::InvalidPanelId;
    }

    if (!record.IsValid()) {
        return UiPanelRegistryStatus::InvalidDesc;
    }

    return ValidateResourceRefs(record);
}

UiPanelRegistryStatus UiPanelRegistry::ValidateManifest(const UiPanelTestManifest &manifest) const {
    if (manifest.records.empty()) {
        return UiPanelRegistryStatus::InvalidDesc;
    }

    if (manifest.records.size() > static_cast<std::size_t>(MAX_UI_PANEL_REGISTRY_RECORD_COUNT)) {
        return UiPanelRegistryStatus::CapacityExceeded;
    }

    for (const UiPanelManifestRecord &record : manifest.records) {
        const UiPanelRegistryStatus status = ValidateRecord(record);
        if (status != UiPanelRegistryStatus::Success) {
            return status;
        }

        if (HasPanel(record.panel_id)) {
            return UiPanelRegistryStatus::DuplicatePanelId;
        }
    }

    return ValidateNoManifestDuplicate(manifest);
}

UiPanelRegistryStatus UiPanelRegistry::ValidateResourceRefs(const UiPanelManifestRecord &record) const {
    if (record.resource_ref_count > MAX_UI_PANEL_RESOURCE_REF_COUNT) {
        return UiPanelRegistryStatus::InvalidDesc;
    }

    if (record.resource_ref_count == 0U) {
        return UiPanelRegistryStatus::InvalidDesc;
    }

    for (std::uint32_t index = 0U; index < record.resource_ref_count; ++index) {
        if (!record.resource_refs[index].IsValid()) {
            return UiPanelRegistryStatus::InvalidDesc;
        }
    }

    return UiPanelRegistryStatus::Success;
}

UiPanelRegistryStatus UiPanelRegistry::ValidateNoManifestDuplicate(const UiPanelTestManifest &manifest) const {
    for (std::size_t left_index = 0U; left_index < manifest.records.size(); ++left_index) {
        const UiPanelId left_panel_id = manifest.records[left_index].panel_id;
        for (std::size_t right_index = left_index + 1U; right_index < manifest.records.size(); ++right_index) {
            const UiPanelId right_panel_id = manifest.records[right_index].panel_id;
            if (left_panel_id.value == right_panel_id.value) {
                return UiPanelRegistryStatus::DuplicatePanelId;
            }
        }
    }

    return UiPanelRegistryStatus::Success;
}

bool UiPanelRegistry::HasPanel(UiPanelId panel_id) const {
    return FindRecord(panel_id) != nullptr;
}

const UiPanelManifestRecord *UiPanelRegistry::FindRecord(UiPanelId panel_id) const {
    if (!panel_id.IsValid()) {
        return nullptr;
    }

    for (std::uint32_t index = 0U; index < snapshot_.panel_capacity; ++index) {
        if (!active_records_[index]) {
            continue;
        }

        if (records_[index].panel_id.value == panel_id.value) {
            return &records_[index];
        }
    }

    return nullptr;
}

std::uint32_t UiPanelRegistry::FindFreeRecordIndex() const {
    for (std::uint32_t index = 0U; index < snapshot_.panel_capacity; ++index) {
        if (!active_records_[index]) {
            return index;
        }
    }

    return MAX_UI_PANEL_REGISTRY_RECORD_COUNT;
}

UiPanelRegistryResult UiPanelRegistry::InsertRecord(const UiPanelManifestRecord &record) {
    const std::uint32_t record_index = FindFreeRecordIndex();
    if (record_index >= snapshot_.panel_capacity) {
        return MakeResult(UiPanelRegistryStatus::CapacityExceeded, record, record_index);
    }

    records_[record_index] = record;
    active_records_[record_index] = true;
    ++snapshot_.registered_panel_count;
    return MakeResult(UiPanelRegistryStatus::Success, record, record_index);
}

UiPanelRegistryResult UiPanelRegistry::MakeResult(
    UiPanelRegistryStatus status,
    const UiPanelManifestRecord &record,
    std::uint32_t record_index) const {
    UiPanelRegistryResult result{};
    result.status = status;
    result.record = record;
    result.record_index = record_index;
    result.required_record_count = snapshot_.registered_panel_count;
    return result;
}

UiPanelRegistryResult UiPanelRegistry::MakeCapacityResult(
    const UiPanelManifestRecord &record,
    std::uint32_t failed_record_index,
    std::uint32_t required_record_count) {
    RecordCapacityEntry(record, failed_record_index, required_record_count);
    UiPanelRegistryResult result = MakeResult(RecordFailure(UiPanelRegistryStatus::CapacityExceeded), record, failed_record_index);
    result.required_record_count = required_record_count;
    result.failed_panel_id = record.panel_id;
    result.failed_record_index = failed_record_index;
    return result;
}

UiPanelRegistryStatus UiPanelRegistry::RecordFailure(UiPanelRegistryStatus status) {
    if (status != UiPanelRegistryStatus::CapacityExceeded) {
        ClearCapacityEntry();
    }

    ++snapshot_.rejected_operation_count;
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return status;
}

void UiPanelRegistry::RecordSuccess() {
    ClearCapacityEntry();
    ++snapshot_.accepted_operation_count;
    snapshot_.last_status = UiPanelRegistryStatus::Success;
}

void UiPanelRegistry::ClearCapacityEntry() {
    snapshot_.required_record_count = 0U;
    snapshot_.failed_panel_id = UiPanelId{};
    snapshot_.failed_record_index = 0U;
}

void UiPanelRegistry::RecordCapacityEntry(
    const UiPanelManifestRecord &record,
    std::uint32_t failed_record_index,
    std::uint32_t required_record_count) {
    snapshot_.required_record_count = required_record_count;
    snapshot_.failed_panel_id = record.panel_id;
    snapshot_.failed_record_index = failed_record_index;
}
}
