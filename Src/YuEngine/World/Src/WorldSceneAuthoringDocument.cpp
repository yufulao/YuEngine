// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldSceneAuthoringDocument.cpp

#include "YuEngine/World/WorldSceneAuthoringDocument.h"

#include <cmath>

namespace yuengine::world {
namespace {
std::uint32_t ClampObjectRecordCapacity(std::uint32_t requested_capacity) {
    if (requested_capacity > MAX_WORLD_OBJECT_COUNT) {
        return MAX_WORLD_OBJECT_COUNT;
    }

    return requested_capacity;
}

std::uint32_t ClampDependencyCapacity(std::uint32_t requested_capacity) {
    if (requested_capacity > MAX_WORLD_SCENE_AUTHORING_DEPENDENCY_COUNT) {
        return MAX_WORLD_SCENE_AUTHORING_DEPENDENCY_COUNT;
    }

    return requested_capacity;
}

std::uint32_t ClampSidecarCapacity(std::uint32_t requested_capacity) {
    if (requested_capacity > MAX_WORLD_SCENE_EDITOR_SIDECAR_RECORD_COUNT) {
        return MAX_WORLD_SCENE_EDITOR_SIDECAR_RECORD_COUNT;
    }

    return requested_capacity;
}

bool IsFinite(float value) {
    return std::isfinite(value);
}

bool IsValidTransformState(const WorldTransformState &transform_state) {
    if (!IsFinite(transform_state.translation_x)) {
        return false;
    }

    if (!IsFinite(transform_state.translation_y)) {
        return false;
    }

    if (!IsFinite(transform_state.translation_z)) {
        return false;
    }

    if (!IsFinite(transform_state.rotation_x)) {
        return false;
    }

    if (!IsFinite(transform_state.rotation_y)) {
        return false;
    }

    if (!IsFinite(transform_state.rotation_z)) {
        return false;
    }

    if (!IsFinite(transform_state.rotation_w)) {
        return false;
    }

    if (!IsFinite(transform_state.scale_x)) {
        return false;
    }

    if (!IsFinite(transform_state.scale_y)) {
        return false;
    }

    if (!IsFinite(transform_state.scale_z)) {
        return false;
    }

    if (transform_state.scale_x == 0.0F) {
        return false;
    }

    if (transform_state.scale_y == 0.0F) {
        return false;
    }

    return transform_state.scale_z != 0.0F;
}

bool IsValidSidecarKind(WorldSceneEditorSidecarKind kind) {
    switch (kind) {
        case WorldSceneEditorSidecarKind::Selection:
        case WorldSceneEditorSidecarKind::Foldout:
        case WorldSceneEditorSidecarKind::ViewportCameraBookmark:
        case WorldSceneEditorSidecarKind::GridSnap:
        case WorldSceneEditorSidecarKind::GizmoMode:
        case WorldSceneEditorSidecarKind::PanelLayout:
        case WorldSceneEditorSidecarKind::DisplayFilter:
        case WorldSceneEditorSidecarKind::UndoSelectionCursor:
            return true;
        default:
            break;
    }

    return false;
}

bool IsObjectBoundSidecarKind(WorldSceneEditorSidecarKind kind) {
    switch (kind) {
        case WorldSceneEditorSidecarKind::Selection:
        case WorldSceneEditorSidecarKind::Foldout:
        case WorldSceneEditorSidecarKind::UndoSelectionCursor:
            return true;
        default:
            break;
    }

    return false;
}

bool IsSidecarFailure(WorldSceneAuthoringDocumentStatus status) {
    if (status == WorldSceneAuthoringDocumentStatus::InvalidSidecarInput) {
        return true;
    }

    if (status == WorldSceneAuthoringDocumentStatus::InvalidSidecarKind) {
        return true;
    }

    if (status == WorldSceneAuthoringDocumentStatus::InvalidSidecarWorldObjectId) {
        return true;
    }

    if (status == WorldSceneAuthoringDocumentStatus::DuplicateSidecarRecord) {
        return true;
    }

    if (status == WorldSceneAuthoringDocumentStatus::SidecarReferencesMissingObject) {
        return true;
    }

    return status == WorldSceneAuthoringDocumentStatus::SidecarMarkedForRuntimeExport;
}
}

WorldSceneAuthoringDocumentValidator::WorldSceneAuthoringDocumentValidator(
    WorldSceneAuthoringDocumentDesc desc)
    : identity_capacity_(ClampObjectRecordCapacity(desc.identity_capacity)),
      transform_capacity_(ClampObjectRecordCapacity(desc.transform_capacity)),
      attachment_capacity_(ClampObjectRecordCapacity(desc.attachment_capacity)),
      binding_capacity_(ClampObjectRecordCapacity(desc.binding_capacity)),
      dependency_capacity_(ClampDependencyCapacity(desc.dependency_capacity)),
      sidecar_capacity_(ClampSidecarCapacity(desc.sidecar_capacity)) {
    snapshot_.identity_capacity = identity_capacity_;
    snapshot_.transform_capacity = transform_capacity_;
    snapshot_.attachment_capacity = attachment_capacity_;
    snapshot_.binding_capacity = binding_capacity_;
    snapshot_.dependency_capacity = dependency_capacity_;
    snapshot_.sidecar_capacity = sidecar_capacity_;
}

WorldSceneAuthoringDocumentResult WorldSceneAuthoringDocumentValidator::ValidateAndExport(
    const WorldSceneAuthoringDocument &document,
    WorldSceneAuthoringRuntimeExport *runtime_output) {
    const WorldSceneAuthoringDocumentStatus capacity_status = ValidateValidatorCapacity();
    if (capacity_status != WorldSceneAuthoringDocumentStatus::Success) {
        return RecordFailure(capacity_status);
    }

    const WorldSceneAuthoringDocumentStatus header_status = ValidateHeader(document.header);
    if (header_status != WorldSceneAuthoringDocumentStatus::Success) {
        return RecordFailure(header_status);
    }

    const WorldSceneAuthoringDocumentStatus input_status = ValidateDocumentInputs(document);
    if (input_status != WorldSceneAuthoringDocumentStatus::Success) {
        return RecordFailure(input_status);
    }

    const WorldSceneAuthoringDocumentStatus output_status = ValidateRuntimeOutput(
        document,
        runtime_output);
    if (output_status != WorldSceneAuthoringDocumentStatus::Success) {
        return RecordFailure(output_status);
    }

    const WorldSceneAuthoringDocumentStatus identity_status = ValidateIdentityRecords(
        document.identity_records,
        document.header.identity_record_count);
    if (identity_status != WorldSceneAuthoringDocumentStatus::Success) {
        return RecordFailure(identity_status);
    }

    const WorldSceneAuthoringDocumentStatus transform_status = ValidateTransformRecords(
        document.identity_records,
        document.header.identity_record_count,
        document.transform_records,
        document.header.transform_record_count);
    if (transform_status != WorldSceneAuthoringDocumentStatus::Success) {
        return RecordFailure(transform_status);
    }

    const WorldSceneAuthoringDocumentStatus attachment_status = ValidateAttachmentRecords(
        document.identity_records,
        document.header.identity_record_count,
        document.attachment_records,
        document.header.attachment_record_count);
    if (attachment_status != WorldSceneAuthoringDocumentStatus::Success) {
        return RecordFailure(attachment_status);
    }

    const WorldSceneAuthoringDocumentStatus dependency_status = ValidateDependencyRecords(
        document.dependency_records,
        document.header.dependency_record_count);
    if (dependency_status != WorldSceneAuthoringDocumentStatus::Success) {
        return RecordFailure(dependency_status);
    }

    const WorldSceneAuthoringDocumentStatus binding_status = ValidateBindingRecords(
        document.attachment_records,
        document.header.attachment_record_count,
        document.dependency_records,
        document.header.dependency_record_count,
        document.binding_records,
        document.header.binding_record_count);
    if (binding_status != WorldSceneAuthoringDocumentStatus::Success) {
        return RecordFailure(binding_status);
    }

    const WorldSceneAuthoringDocumentStatus sidecar_status = ValidateSidecarRecords(
        document.identity_records,
        document.header.identity_record_count,
        document.sidecar_records,
        document.header.sidecar_record_count);
    if (sidecar_status != WorldSceneAuthoringDocumentStatus::Success) {
        return RecordFailure(sidecar_status);
    }

    CopyRuntimeOutputs(document, runtime_output);

    WorldSceneAuthoringDocumentState state{};
    state.scene_document_id = document.header.scene_document_id;
    state.deterministic_document_hash = document.header.deterministic_document_hash;
    state.exported_identity_count = document.header.identity_record_count;
    state.exported_transform_count = document.header.transform_record_count;
    state.exported_attachment_count = document.header.attachment_record_count;
    state.exported_binding_count = document.header.binding_record_count;
    state.exported_dependency_count = document.header.dependency_record_count;
    state.validated_sidecar_count = document.header.sidecar_record_count;
    return RecordSuccess(state);
}

WorldSceneAuthoringDocumentSnapshot WorldSceneAuthoringDocumentValidator::Snapshot() const {
    return snapshot_;
}

WorldSceneAuthoringDocumentResult WorldSceneAuthoringDocumentValidator::RecordFailure(
    WorldSceneAuthoringDocumentStatus status) {
    ++snapshot_.failed_operation_count;
    ++snapshot_.rejected_document_count;
    if (IsSidecarFailure(status)) {
        ++snapshot_.rejected_sidecar_count;
    }

    snapshot_.last_status = status;
    return WorldSceneAuthoringDocumentResult::Failure(status);
}

WorldSceneAuthoringDocumentResult WorldSceneAuthoringDocumentValidator::RecordSuccess(
    const WorldSceneAuthoringDocumentState &state) {
    ++snapshot_.validate_count;
    ++snapshot_.runtime_export_count;
    snapshot_.exported_identity_count += state.exported_identity_count;
    snapshot_.exported_transform_count += state.exported_transform_count;
    snapshot_.exported_attachment_count += state.exported_attachment_count;
    snapshot_.exported_binding_count += state.exported_binding_count;
    snapshot_.exported_dependency_count += state.exported_dependency_count;
    snapshot_.validated_sidecar_count += state.validated_sidecar_count;
    snapshot_.last_status = WorldSceneAuthoringDocumentStatus::Success;
    return WorldSceneAuthoringDocumentResult::Success(state);
}

WorldSceneAuthoringDocumentStatus WorldSceneAuthoringDocumentValidator::ValidateValidatorCapacity() const {
    if (identity_capacity_ == 0U) {
        return WorldSceneAuthoringDocumentStatus::InvalidValidatorCapacity;
    }

    if (transform_capacity_ == 0U) {
        return WorldSceneAuthoringDocumentStatus::InvalidValidatorCapacity;
    }

    if (attachment_capacity_ == 0U) {
        return WorldSceneAuthoringDocumentStatus::InvalidValidatorCapacity;
    }

    if (binding_capacity_ == 0U) {
        return WorldSceneAuthoringDocumentStatus::InvalidValidatorCapacity;
    }

    if (dependency_capacity_ == 0U) {
        return WorldSceneAuthoringDocumentStatus::InvalidValidatorCapacity;
    }

    if (sidecar_capacity_ == 0U) {
        return WorldSceneAuthoringDocumentStatus::InvalidValidatorCapacity;
    }

    return WorldSceneAuthoringDocumentStatus::Success;
}

WorldSceneAuthoringDocumentStatus WorldSceneAuthoringDocumentValidator::ValidateHeader(
    const WorldSceneAuthoringDocumentHeader &header) const {
    if (header.schema_major_version != WORLD_SCENE_AUTHORING_DOCUMENT_SCHEMA_MAJOR_VERSION) {
        return WorldSceneAuthoringDocumentStatus::UnsupportedVersion;
    }

    if (header.schema_minor_version > WORLD_SCENE_AUTHORING_DOCUMENT_SCHEMA_MINOR_VERSION) {
        return WorldSceneAuthoringDocumentStatus::UnsupportedVersion;
    }

    if (header.scene_document_id == INVALID_WORLD_SCENE_DOCUMENT_ID) {
        return WorldSceneAuthoringDocumentStatus::InvalidDocumentId;
    }

    if (header.deterministic_document_hash == INVALID_WORLD_SCENE_DOCUMENT_HASH) {
        return WorldSceneAuthoringDocumentStatus::InvalidDocumentHash;
    }

    if (header.identity_record_count > identity_capacity_) {
        return WorldSceneAuthoringDocumentStatus::InputCountExceeded;
    }

    if (header.transform_record_count > transform_capacity_) {
        return WorldSceneAuthoringDocumentStatus::InputCountExceeded;
    }

    if (header.attachment_record_count > attachment_capacity_) {
        return WorldSceneAuthoringDocumentStatus::InputCountExceeded;
    }

    if (header.binding_record_count > binding_capacity_) {
        return WorldSceneAuthoringDocumentStatus::InputCountExceeded;
    }

    if (header.dependency_record_count > dependency_capacity_) {
        return WorldSceneAuthoringDocumentStatus::InputCountExceeded;
    }

    if (header.sidecar_record_count > sidecar_capacity_) {
        return WorldSceneAuthoringDocumentStatus::InputCountExceeded;
    }

    if (header.unsupported_runtime_field_count > 0U) {
        return WorldSceneAuthoringDocumentStatus::UnsupportedRuntimeField;
    }

    return WorldSceneAuthoringDocumentStatus::Success;
}

WorldSceneAuthoringDocumentStatus WorldSceneAuthoringDocumentValidator::ValidateDocumentInputs(
    const WorldSceneAuthoringDocument &document) const {
    if (document.header.identity_record_count > 0U && document.identity_records == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidIdentityInput;
    }

    if (document.header.transform_record_count > 0U && document.transform_records == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidTransformInput;
    }

    if (document.header.attachment_record_count > 0U && document.attachment_records == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidAttachmentInput;
    }

    if (document.header.binding_record_count > 0U && document.binding_records == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidBindingInput;
    }

    if (document.header.dependency_record_count > 0U && document.dependency_records == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidDependencyInput;
    }

    if (document.header.sidecar_record_count > 0U && document.sidecar_records == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidSidecarInput;
    }

    return WorldSceneAuthoringDocumentStatus::Success;
}

WorldSceneAuthoringDocumentStatus WorldSceneAuthoringDocumentValidator::ValidateRuntimeOutput(
    const WorldSceneAuthoringDocument &document,
    const WorldSceneAuthoringRuntimeExport *runtime_output) const {
    if (runtime_output == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidRuntimeExportOutput;
    }

    if (runtime_output->identity_count == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidIdentityOutputCount;
    }

    if (runtime_output->transform_count == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidTransformOutputCount;
    }

    if (runtime_output->attachment_count == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidAttachmentOutputCount;
    }

    if (runtime_output->binding_count == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidBindingOutputCount;
    }

    if (runtime_output->dependency_count == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidDependencyOutputCount;
    }

    if (document.header.identity_record_count > runtime_output->identity_capacity) {
        return WorldSceneAuthoringDocumentStatus::OutputCapacityExceeded;
    }

    if (document.header.transform_record_count > runtime_output->transform_capacity) {
        return WorldSceneAuthoringDocumentStatus::OutputCapacityExceeded;
    }

    if (document.header.attachment_record_count > runtime_output->attachment_capacity) {
        return WorldSceneAuthoringDocumentStatus::OutputCapacityExceeded;
    }

    if (document.header.binding_record_count > runtime_output->binding_capacity) {
        return WorldSceneAuthoringDocumentStatus::OutputCapacityExceeded;
    }

    if (document.header.dependency_record_count > runtime_output->dependency_capacity) {
        return WorldSceneAuthoringDocumentStatus::OutputCapacityExceeded;
    }

    if (document.header.identity_record_count > 0U && runtime_output->identity_records == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidIdentityOutput;
    }

    if (document.header.transform_record_count > 0U && runtime_output->transform_records == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidTransformOutput;
    }

    if (document.header.attachment_record_count > 0U && runtime_output->attachment_records == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidAttachmentOutput;
    }

    if (document.header.binding_record_count > 0U && runtime_output->binding_records == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidBindingOutput;
    }

    if (document.header.dependency_record_count > 0U && runtime_output->dependency_records == nullptr) {
        return WorldSceneAuthoringDocumentStatus::InvalidDependencyOutput;
    }

    return WorldSceneAuthoringDocumentStatus::Success;
}

WorldSceneAuthoringDocumentStatus WorldSceneAuthoringDocumentValidator::ValidateIdentityRecords(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < record_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &record = records[record_index];
        if (!record.world_object_id.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidWorldObjectId;
        }

        if (!record.object_handle.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidObjectHandle;
        }

        if (HasDuplicateIdentityWorldObjectId(records, record_index)) {
            return WorldSceneAuthoringDocumentStatus::DuplicateIdentityWorldObjectId;
        }

        if (HasDuplicateIdentityObjectHandle(records, record_index)) {
            return WorldSceneAuthoringDocumentStatus::DuplicateIdentityObjectHandle;
        }

        ++record_index;
    }

    return WorldSceneAuthoringDocumentStatus::Success;
}

WorldSceneAuthoringDocumentStatus WorldSceneAuthoringDocumentValidator::ValidateTransformRecords(
    const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
    std::uint32_t identity_record_count,
    const WorldSceneObjectTransformRestoreTransformRecord *transform_records,
    std::uint32_t transform_record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < transform_record_count) {
        const WorldSceneObjectTransformRestoreTransformRecord &record = transform_records[record_index];
        if (!record.world_object_id.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidWorldObjectId;
        }

        if (!IsValidTransformState(record.transform_state)) {
            return WorldSceneAuthoringDocumentStatus::InvalidTransformState;
        }

        if (HasDuplicateTransformWorldObjectId(transform_records, record_index)) {
            return WorldSceneAuthoringDocumentStatus::DuplicateTransformWorldObjectId;
        }

        if (!HasIdentityRecord(identity_records, identity_record_count, record.world_object_id)) {
            return WorldSceneAuthoringDocumentStatus::MissingIdentityForTransform;
        }

        ++record_index;
    }

    return WorldSceneAuthoringDocumentStatus::Success;
}

WorldSceneAuthoringDocumentStatus WorldSceneAuthoringDocumentValidator::ValidateAttachmentRecords(
    const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
    std::uint32_t identity_record_count,
    const WorldComponentAttachmentSnapshotRecord *attachment_records,
    std::uint32_t attachment_record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < attachment_record_count) {
        const WorldComponentAttachmentSnapshotRecord &record = attachment_records[record_index];
        if (!record.world_object_id.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidWorldObjectId;
        }

        if (!record.component_type_id.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidComponentTypeId;
        }

        if (!record.component_slot_id.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidComponentSlotId;
        }

        if (!HasIdentityRecord(identity_records, identity_record_count, record.world_object_id)) {
            return WorldSceneAuthoringDocumentStatus::MissingIdentityForAttachment;
        }

        if (HasDuplicateAttachment(attachment_records, record_index)) {
            return WorldSceneAuthoringDocumentStatus::DuplicateAttachment;
        }

        ++record_index;
    }

    return WorldSceneAuthoringDocumentStatus::Success;
}

WorldSceneAuthoringDocumentStatus WorldSceneAuthoringDocumentValidator::ValidateBindingRecords(
    const WorldComponentAttachmentSnapshotRecord *attachment_records,
    std::uint32_t attachment_record_count,
    const WorldSceneAuthoringDependencyRecord *dependency_records,
    std::uint32_t dependency_record_count,
    const WorldComponentResourceBindingSnapshotRecord *binding_records,
    std::uint32_t binding_record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < binding_record_count) {
        const WorldComponentResourceBindingSnapshotRecord &record = binding_records[record_index];
        if (!record.world_object_id.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidWorldObjectId;
        }

        if (!record.component_type_id.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidComponentTypeId;
        }

        if (!record.component_slot_id.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidComponentSlotId;
        }

        if (!record.resource_handle.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidResourceHandle;
        }

        if (!record.expected_resource_type.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidResourceTypeId;
        }

        if (!HasAttachmentTuple(attachment_records, attachment_record_count, record)) {
            return WorldSceneAuthoringDocumentStatus::MissingAttachmentForBinding;
        }

        if (!HasDependencyForBinding(dependency_records, dependency_record_count, record)) {
            return WorldSceneAuthoringDocumentStatus::MissingDependencyForBinding;
        }

        if (HasDuplicateBinding(binding_records, record_index)) {
            return WorldSceneAuthoringDocumentStatus::DuplicateBinding;
        }

        ++record_index;
    }

    return WorldSceneAuthoringDocumentStatus::Success;
}

WorldSceneAuthoringDocumentStatus WorldSceneAuthoringDocumentValidator::ValidateDependencyRecords(
    const WorldSceneAuthoringDependencyRecord *dependency_records,
    std::uint32_t dependency_record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < dependency_record_count) {
        const WorldSceneAuthoringDependencyRecord &record = dependency_records[record_index];
        if (record.stable_resource_id == 0U) {
            return WorldSceneAuthoringDocumentStatus::InvalidDependencyId;
        }

        if (!record.resource_handle.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidResourceHandle;
        }

        if (!record.expected_resource_type.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidResourceTypeId;
        }

        if (HasDuplicateDependency(dependency_records, record_index)) {
            return WorldSceneAuthoringDocumentStatus::DuplicateDependency;
        }

        ++record_index;
    }

    return WorldSceneAuthoringDocumentStatus::Success;
}

WorldSceneAuthoringDocumentStatus WorldSceneAuthoringDocumentValidator::ValidateSidecarRecords(
    const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
    std::uint32_t identity_record_count,
    const WorldSceneEditorSidecarRecord *sidecar_records,
    std::uint32_t sidecar_record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &record = sidecar_records[record_index];
        if (!IsValidSidecarKind(record.kind)) {
            return WorldSceneAuthoringDocumentStatus::InvalidSidecarKind;
        }

        if (record.export_policy != WorldSceneEditorSidecarExportPolicy::EditorOnly) {
            return WorldSceneAuthoringDocumentStatus::SidecarMarkedForRuntimeExport;
        }

        if (IsObjectBoundSidecarKind(record.kind) && !record.world_object_id.IsValid()) {
            return WorldSceneAuthoringDocumentStatus::InvalidSidecarWorldObjectId;
        }

        if (record.world_object_id.IsValid() &&
            !HasIdentityRecord(identity_records, identity_record_count, record.world_object_id)) {
            return WorldSceneAuthoringDocumentStatus::SidecarReferencesMissingObject;
        }

        if (HasDuplicateSidecarRecord(sidecar_records, record_index)) {
            return WorldSceneAuthoringDocumentStatus::DuplicateSidecarRecord;
        }

        ++record_index;
    }

    return WorldSceneAuthoringDocumentStatus::Success;
}

bool WorldSceneAuthoringDocumentValidator::HasDuplicateIdentityWorldObjectId(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreIdentityRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldSceneObjectTransformRestoreIdentityRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value == record.world_object_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneAuthoringDocumentValidator::HasDuplicateIdentityObjectHandle(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreIdentityRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldSceneObjectTransformRestoreIdentityRecord &compare_record = records[compare_index];
        if (compare_record.object_handle.slot != record.object_handle.slot) {
            ++compare_index;
            continue;
        }

        if (compare_record.object_handle.generation == record.object_handle.generation) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneAuthoringDocumentValidator::HasDuplicateTransformWorldObjectId(
    const WorldSceneObjectTransformRestoreTransformRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreTransformRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldSceneObjectTransformRestoreTransformRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value == record.world_object_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneAuthoringDocumentValidator::HasDuplicateAttachment(
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t record_index) const {
    const WorldComponentAttachmentSnapshotRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldComponentAttachmentSnapshotRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value != record.world_object_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_record.component_type_id.value == record.component_type_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneAuthoringDocumentValidator::HasDuplicateBinding(
    const WorldComponentResourceBindingSnapshotRecord *records,
    std::uint32_t record_index) const {
    const WorldComponentResourceBindingSnapshotRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldComponentResourceBindingSnapshotRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value != record.world_object_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_record.component_type_id.value != record.component_type_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_record.component_slot_id.value == record.component_slot_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneAuthoringDocumentValidator::HasDuplicateDependency(
    const WorldSceneAuthoringDependencyRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneAuthoringDependencyRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldSceneAuthoringDependencyRecord &compare_record = records[compare_index];
        if (compare_record.stable_resource_id == record.stable_resource_id) {
            return true;
        }

        if (compare_record.resource_handle.slot != record.resource_handle.slot) {
            ++compare_index;
            continue;
        }

        if (compare_record.resource_handle.generation == record.resource_handle.generation) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneAuthoringDocumentValidator::HasDuplicateSidecarRecord(
    const WorldSceneEditorSidecarRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneEditorSidecarRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldSceneEditorSidecarRecord &compare_record = records[compare_index];
        if (compare_record.kind != record.kind) {
            ++compare_index;
            continue;
        }

        if (compare_record.world_object_id.value != record.world_object_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_record.slot == record.slot) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneAuthoringDocumentValidator::HasIdentityRecord(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_count,
    WorldObjectId world_object_id) const {
    std::uint32_t record_index = 0U;
    while (record_index < record_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &record = records[record_index];
        if (record.world_object_id.value == world_object_id.value) {
            return true;
        }

        ++record_index;
    }

    return false;
}

bool WorldSceneAuthoringDocumentValidator::HasAttachmentTuple(
    const WorldComponentAttachmentSnapshotRecord *attachment_records,
    std::uint32_t attachment_record_count,
    const WorldComponentResourceBindingSnapshotRecord &binding_record) const {
    std::uint32_t attachment_index = 0U;
    while (attachment_index < attachment_record_count) {
        const WorldComponentAttachmentSnapshotRecord &attachment = attachment_records[attachment_index];
        if (attachment.world_object_id.value != binding_record.world_object_id.value) {
            ++attachment_index;
            continue;
        }

        if (attachment.component_type_id.value != binding_record.component_type_id.value) {
            ++attachment_index;
            continue;
        }

        if (attachment.component_slot_id.value == binding_record.component_slot_id.value) {
            return true;
        }

        ++attachment_index;
    }

    return false;
}

bool WorldSceneAuthoringDocumentValidator::HasDependencyForBinding(
    const WorldSceneAuthoringDependencyRecord *dependency_records,
    std::uint32_t dependency_record_count,
    const WorldComponentResourceBindingSnapshotRecord &binding_record) const {
    std::uint32_t dependency_index = 0U;
    while (dependency_index < dependency_record_count) {
        const WorldSceneAuthoringDependencyRecord &dependency = dependency_records[dependency_index];
        if (dependency.resource_handle.slot != binding_record.resource_handle.slot) {
            ++dependency_index;
            continue;
        }

        if (dependency.resource_handle.generation != binding_record.resource_handle.generation) {
            ++dependency_index;
            continue;
        }

        if (dependency.expected_resource_type.value == binding_record.expected_resource_type.value) {
            return true;
        }

        ++dependency_index;
    }

    return false;
}

void WorldSceneAuthoringDocumentValidator::CopyRuntimeOutputs(
    const WorldSceneAuthoringDocument &document,
    WorldSceneAuthoringRuntimeExport *runtime_output) const {
    std::uint32_t identity_index = 0U;
    while (identity_index < document.header.identity_record_count) {
        runtime_output->identity_records[identity_index] = document.identity_records[identity_index];
        ++identity_index;
    }

    std::uint32_t transform_index = 0U;
    while (transform_index < document.header.transform_record_count) {
        runtime_output->transform_records[transform_index] = document.transform_records[transform_index];
        ++transform_index;
    }

    std::uint32_t attachment_index = 0U;
    while (attachment_index < document.header.attachment_record_count) {
        runtime_output->attachment_records[attachment_index] = document.attachment_records[attachment_index];
        ++attachment_index;
    }

    std::uint32_t binding_index = 0U;
    while (binding_index < document.header.binding_record_count) {
        runtime_output->binding_records[binding_index] = document.binding_records[binding_index];
        ++binding_index;
    }

    std::uint32_t dependency_index = 0U;
    while (dependency_index < document.header.dependency_record_count) {
        runtime_output->dependency_records[dependency_index] = document.dependency_records[dependency_index];
        ++dependency_index;
    }

    *runtime_output->identity_count = document.header.identity_record_count;
    *runtime_output->transform_count = document.header.transform_record_count;
    *runtime_output->attachment_count = document.header.attachment_record_count;
    *runtime_output->binding_count = document.header.binding_record_count;
    *runtime_output->dependency_count = document.header.dependency_record_count;
}
}
