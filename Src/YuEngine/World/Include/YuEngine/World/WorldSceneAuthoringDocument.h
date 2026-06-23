// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAuthoringDocument.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"

namespace yuengine::world {
inline constexpr std::uint32_t WORLD_SCENE_AUTHORING_DOCUMENT_SCHEMA_MAJOR_VERSION = 1U;
inline constexpr std::uint32_t WORLD_SCENE_AUTHORING_DOCUMENT_SCHEMA_MINOR_VERSION = 0U;
inline constexpr std::uint32_t MAX_WORLD_SCENE_AUTHORING_DEPENDENCY_COUNT = 64U;
inline constexpr std::uint32_t MAX_WORLD_SCENE_EDITOR_SIDECAR_RECORD_COUNT = 128U;
inline constexpr std::uint64_t INVALID_WORLD_SCENE_DOCUMENT_ID = 0U;
inline constexpr std::uint64_t INVALID_WORLD_SCENE_DOCUMENT_HASH = 0U;

enum class WorldSceneEditorSidecarKind {
    Unknown,
    Selection,
    Foldout,
    ViewportCameraBookmark,
    GridSnap,
    GizmoMode,
    PanelLayout,
    DisplayFilter,
    UndoSelectionCursor
};

enum class WorldSceneEditorSidecarExportPolicy {
    EditorOnly,
    RuntimeData
};

enum class WorldSceneAuthoringDocumentStatus {
    Success,
    InvalidValidatorCapacity,
    UnsupportedVersion,
    InvalidDocumentId,
    InvalidDocumentHash,
    InputCountExceeded,
    UnsupportedRuntimeField,
    InvalidIdentityInput,
    InvalidTransformInput,
    InvalidAttachmentInput,
    InvalidBindingInput,
    InvalidDependencyInput,
    InvalidSidecarInput,
    InvalidRuntimeExportOutput,
    InvalidIdentityOutput,
    InvalidTransformOutput,
    InvalidAttachmentOutput,
    InvalidBindingOutput,
    InvalidDependencyOutput,
    InvalidIdentityOutputCount,
    InvalidTransformOutputCount,
    InvalidAttachmentOutputCount,
    InvalidBindingOutputCount,
    InvalidDependencyOutputCount,
    OutputCapacityExceeded,
    InvalidWorldObjectId,
    InvalidObjectHandle,
    InvalidTransformState,
    InvalidComponentTypeId,
    InvalidComponentSlotId,
    InvalidResourceHandle,
    InvalidResourceTypeId,
    InvalidDependencyId,
    DuplicateIdentityWorldObjectId,
    DuplicateIdentityObjectHandle,
    DuplicateTransformWorldObjectId,
    DuplicateAttachment,
    DuplicateBinding,
    DuplicateDependency,
    DuplicateSidecarRecord,
    MissingIdentityForTransform,
    MissingIdentityForAttachment,
    MissingAttachmentForBinding,
    MissingDependencyForBinding,
    InvalidSidecarKind,
    InvalidSidecarWorldObjectId,
    SidecarReferencesMissingObject,
    SidecarMarkedForRuntimeExport
};

struct WorldSceneAuthoringDocumentHeader final {
    std::uint32_t schema_major_version = WORLD_SCENE_AUTHORING_DOCUMENT_SCHEMA_MAJOR_VERSION;
    std::uint32_t schema_minor_version = WORLD_SCENE_AUTHORING_DOCUMENT_SCHEMA_MINOR_VERSION;
    std::uint64_t scene_document_id = INVALID_WORLD_SCENE_DOCUMENT_ID;
    std::uint64_t deterministic_document_hash = INVALID_WORLD_SCENE_DOCUMENT_HASH;
    std::uint32_t identity_record_count = 0U;
    std::uint32_t transform_record_count = 0U;
    std::uint32_t attachment_record_count = 0U;
    std::uint32_t binding_record_count = 0U;
    std::uint32_t dependency_record_count = 0U;
    std::uint32_t sidecar_record_count = 0U;
    std::uint32_t unsupported_runtime_field_count = 0U;
};

struct WorldSceneAuthoringDependencyRecord final {
    std::uint64_t stable_resource_id = 0U;
    yuengine::resource::ResourceHandle resource_handle{};
    yuengine::resource::ResourceTypeId expected_resource_type{};
};

struct WorldSceneEditorSidecarRecord final {
    WorldSceneEditorSidecarKind kind = WorldSceneEditorSidecarKind::Unknown;
    WorldSceneEditorSidecarExportPolicy export_policy =
        WorldSceneEditorSidecarExportPolicy::EditorOnly;
    WorldObjectId world_object_id{};
    std::uint32_t slot = 0U;
    std::uint64_t value = 0U;
};

struct WorldSceneAuthoringDocument final {
    WorldSceneAuthoringDocumentHeader header{};
    const WorldSceneObjectTransformRestoreIdentityRecord *identity_records = nullptr;
    const WorldSceneObjectTransformRestoreTransformRecord *transform_records = nullptr;
    const WorldComponentAttachmentSnapshotRecord *attachment_records = nullptr;
    const WorldComponentResourceBindingSnapshotRecord *binding_records = nullptr;
    const WorldSceneAuthoringDependencyRecord *dependency_records = nullptr;
    const WorldSceneEditorSidecarRecord *sidecar_records = nullptr;
};

struct WorldSceneAuthoringRuntimeExport final {
    WorldSceneObjectTransformRestoreIdentityRecord *identity_records = nullptr;
    std::uint32_t identity_capacity = 0U;
    std::uint32_t *identity_count = nullptr;
    WorldSceneObjectTransformRestoreTransformRecord *transform_records = nullptr;
    std::uint32_t transform_capacity = 0U;
    std::uint32_t *transform_count = nullptr;
    WorldComponentAttachmentSnapshotRecord *attachment_records = nullptr;
    std::uint32_t attachment_capacity = 0U;
    std::uint32_t *attachment_count = nullptr;
    WorldComponentResourceBindingSnapshotRecord *binding_records = nullptr;
    std::uint32_t binding_capacity = 0U;
    std::uint32_t *binding_count = nullptr;
    WorldSceneAuthoringDependencyRecord *dependency_records = nullptr;
    std::uint32_t dependency_capacity = 0U;
    std::uint32_t *dependency_count = nullptr;
};

struct WorldSceneAuthoringDocumentDesc final {
    std::uint32_t identity_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t transform_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t attachment_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t binding_capacity = MAX_WORLD_OBJECT_COUNT;
    std::uint32_t dependency_capacity = MAX_WORLD_SCENE_AUTHORING_DEPENDENCY_COUNT;
    std::uint32_t sidecar_capacity = MAX_WORLD_SCENE_EDITOR_SIDECAR_RECORD_COUNT;
};

struct WorldSceneAuthoringDocumentState final {
    std::uint64_t scene_document_id = INVALID_WORLD_SCENE_DOCUMENT_ID;
    std::uint64_t deterministic_document_hash = INVALID_WORLD_SCENE_DOCUMENT_HASH;
    std::uint32_t exported_identity_count = 0U;
    std::uint32_t exported_transform_count = 0U;
    std::uint32_t exported_attachment_count = 0U;
    std::uint32_t exported_binding_count = 0U;
    std::uint32_t exported_dependency_count = 0U;
    std::uint32_t validated_sidecar_count = 0U;
};

struct WorldSceneAuthoringDocumentResult final {
    WorldSceneAuthoringDocumentStatus status =
        WorldSceneAuthoringDocumentStatus::Success;
    WorldSceneAuthoringDocumentState state{};

    static WorldSceneAuthoringDocumentResult Success(
        const WorldSceneAuthoringDocumentState &state) {
        return WorldSceneAuthoringDocumentResult{
            WorldSceneAuthoringDocumentStatus::Success,
            state};
    }

    static WorldSceneAuthoringDocumentResult Failure(
        WorldSceneAuthoringDocumentStatus status) {
        return WorldSceneAuthoringDocumentResult{
            status,
            WorldSceneAuthoringDocumentState{}};
    }

    bool Succeeded() const {
        return status == WorldSceneAuthoringDocumentStatus::Success;
    }
};

struct WorldSceneAuthoringDocumentSnapshot final {
    std::uint32_t identity_capacity = 0U;
    std::uint32_t transform_capacity = 0U;
    std::uint32_t attachment_capacity = 0U;
    std::uint32_t binding_capacity = 0U;
    std::uint32_t dependency_capacity = 0U;
    std::uint32_t sidecar_capacity = 0U;
    std::uint64_t validate_count = 0U;
    std::uint64_t runtime_export_count = 0U;
    std::uint64_t exported_identity_count = 0U;
    std::uint64_t exported_transform_count = 0U;
    std::uint64_t exported_attachment_count = 0U;
    std::uint64_t exported_binding_count = 0U;
    std::uint64_t exported_dependency_count = 0U;
    std::uint64_t validated_sidecar_count = 0U;
    std::uint32_t rejected_document_count = 0U;
    std::uint32_t rejected_sidecar_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    WorldSceneAuthoringDocumentStatus last_status =
        WorldSceneAuthoringDocumentStatus::Success;
};

class WorldSceneAuthoringDocumentValidator final {
public:
    explicit WorldSceneAuthoringDocumentValidator(
        WorldSceneAuthoringDocumentDesc desc=WorldSceneAuthoringDocumentDesc{});

    WorldSceneAuthoringDocumentResult ValidateAndExport(
        const WorldSceneAuthoringDocument &document,
        WorldSceneAuthoringRuntimeExport *runtime_output);

    WorldSceneAuthoringDocumentSnapshot Snapshot() const;

private:
    WorldSceneAuthoringDocumentResult RecordFailure(
        WorldSceneAuthoringDocumentStatus status);
    WorldSceneAuthoringDocumentResult RecordSuccess(
        const WorldSceneAuthoringDocumentState &state);
    WorldSceneAuthoringDocumentStatus ValidateValidatorCapacity() const;
    WorldSceneAuthoringDocumentStatus ValidateHeader(
        const WorldSceneAuthoringDocumentHeader &header) const;
    WorldSceneAuthoringDocumentStatus ValidateDocumentInputs(
        const WorldSceneAuthoringDocument &document) const;
    WorldSceneAuthoringDocumentStatus ValidateRuntimeOutput(
        const WorldSceneAuthoringDocument &document,
        const WorldSceneAuthoringRuntimeExport *runtime_output) const;
    WorldSceneAuthoringDocumentStatus ValidateIdentityRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_count) const;
    WorldSceneAuthoringDocumentStatus ValidateTransformRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
        std::uint32_t identity_record_count,
        const WorldSceneObjectTransformRestoreTransformRecord *transform_records,
        std::uint32_t transform_record_count) const;
    WorldSceneAuthoringDocumentStatus ValidateAttachmentRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
        std::uint32_t identity_record_count,
        const WorldComponentAttachmentSnapshotRecord *attachment_records,
        std::uint32_t attachment_record_count) const;
    WorldSceneAuthoringDocumentStatus ValidateBindingRecords(
        const WorldComponentAttachmentSnapshotRecord *attachment_records,
        std::uint32_t attachment_record_count,
        const WorldSceneAuthoringDependencyRecord *dependency_records,
        std::uint32_t dependency_record_count,
        const WorldComponentResourceBindingSnapshotRecord *binding_records,
        std::uint32_t binding_record_count) const;
    WorldSceneAuthoringDocumentStatus ValidateDependencyRecords(
        const WorldSceneAuthoringDependencyRecord *dependency_records,
        std::uint32_t dependency_record_count) const;
    WorldSceneAuthoringDocumentStatus ValidateSidecarRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
        std::uint32_t identity_record_count,
        const WorldSceneEditorSidecarRecord *sidecar_records,
        std::uint32_t sidecar_record_count) const;
    bool HasDuplicateIdentityWorldObjectId(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateIdentityObjectHandle(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateTransformWorldObjectId(
        const WorldSceneObjectTransformRestoreTransformRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateAttachment(
        const WorldComponentAttachmentSnapshotRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateBinding(
        const WorldComponentResourceBindingSnapshotRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateDependency(
        const WorldSceneAuthoringDependencyRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateSidecarRecord(
        const WorldSceneEditorSidecarRecord *records,
        std::uint32_t record_index) const;
    bool HasIdentityRecord(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_count,
        WorldObjectId world_object_id) const;
    bool HasAttachmentTuple(
        const WorldComponentAttachmentSnapshotRecord *attachment_records,
        std::uint32_t attachment_record_count,
        const WorldComponentResourceBindingSnapshotRecord &binding_record) const;
    bool HasDependencyForBinding(
        const WorldSceneAuthoringDependencyRecord *dependency_records,
        std::uint32_t dependency_record_count,
        const WorldComponentResourceBindingSnapshotRecord &binding_record) const;
    void CopyRuntimeOutputs(
        const WorldSceneAuthoringDocument &document,
        WorldSceneAuthoringRuntimeExport *runtime_output) const;

    std::uint32_t identity_capacity_;
    std::uint32_t transform_capacity_;
    std::uint32_t attachment_capacity_;
    std::uint32_t binding_capacity_;
    std::uint32_t dependency_capacity_;
    std::uint32_t sidecar_capacity_;
    WorldSceneAuthoringDocumentSnapshot snapshot_;
};
}
