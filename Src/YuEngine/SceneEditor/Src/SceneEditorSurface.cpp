// 模块: YuEngine SceneEditor
// 文件: Src/YuEngine/SceneEditor/Src/SceneEditorSurface.cpp

#include "YuEngine/SceneEditor/SceneEditorSurface.h"

#include <array>
#include <cstdint>
#include <vector>

#include "YuEngine/File/MountTable.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeWriter.h"
#include "YuEngine/World/WorldSceneRecordValueStreamBridge.h"

namespace yuengine::sceneeditor {
namespace {
using WorldComponentAttachmentSnapshotRecord =
    yuengine::world::WorldComponentAttachmentSnapshotRecord;
using WorldComponentResourceBindingSnapshotRecord =
    yuengine::world::WorldComponentResourceBindingSnapshotRecord;
using WorldObjectId = yuengine::world::WorldObjectId;
using WorldSceneAuthoringDependencyRecord =
    yuengine::world::WorldSceneAuthoringDependencyRecord;
using WorldSceneAuthoringDocument =
    yuengine::world::WorldSceneAuthoringDocument;
using WorldSceneAuthoringDocumentStatus =
    yuengine::world::WorldSceneAuthoringDocumentStatus;
using WorldSceneAuthoringDocumentValidator =
    yuengine::world::WorldSceneAuthoringDocumentValidator;
using WorldSceneAuthoringRuntimeExport =
    yuengine::world::WorldSceneAuthoringRuntimeExport;
using WorldSceneEditorSidecarKind =
    yuengine::world::WorldSceneEditorSidecarKind;
using WorldSceneEditorSidecarRecord =
    yuengine::world::WorldSceneEditorSidecarRecord;
using WorldSceneObjectTransformRestoreIdentityRecord =
    yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using WorldSceneObjectTransformRestoreTransformRecord =
    yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;
using WorldSceneRecordValueStreamBridge =
    yuengine::world::WorldSceneRecordValueStreamBridge;
using WorldSceneRecordValueStreamResult =
    yuengine::world::WorldSceneRecordValueStreamResult;
using WorldSceneRecordValueStreamStatus =
    yuengine::world::WorldSceneRecordValueStreamStatus;
using ResourceBrowserSurfaceSelectionState =
    yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionState;
using ResourceBrowserSurfacePreviewState =
    yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState;
using PreviewHostViewportSessionResult =
    yuengine::previewhost::PreviewHostViewportSessionResult;
using PreviewHostEditorViewportInteractionResult =
    yuengine::previewhost::PreviewHostEditorViewportInteractionResult;
using PreviewHostStatus = yuengine::previewhost::PreviewHostStatus;
using FileReadRequest = yuengine::file::FileReadRequest;
using FileReadResult = yuengine::file::FileReadResult;
using FileWriteRequest = yuengine::file::FileWriteRequest;
using FileWriteResult = yuengine::file::FileWriteResult;
using SerializeReader = yuengine::serialize::SerializeReader;
using SerializeWriter = yuengine::serialize::SerializeWriter;

template <typename T>
bool IsSpanStorageValidGeneric(std::span<T> rows) {
    if (rows.empty()) {
        return true;
    }

    return rows.data() != nullptr;
}

bool IsSpanStorageValid(std::span<SceneEditorHierarchyRow> rows) {
    if (rows.empty()) {
        return true;
    }

    return rows.data() != nullptr;
}

bool IsSpanStorageValid(std::span<SceneEditorInspectorRow> rows) {
    if (rows.empty()) {
        return true;
    }

    return rows.data() != nullptr;
}

bool IsSpanStorageValid(
    std::span<WorldSceneObjectTransformRestoreTransformRecord> records) {
    if (records.empty()) {
        return true;
    }

    return records.data() != nullptr;
}

bool IsSpanStorageValid(std::span<SceneEditorTransformLedgerRecord> records) {
    if (records.empty()) {
        return true;
    }

    return records.data() != nullptr;
}

bool IsSpanStorageValid(std::span<SceneEditorWorkflowLedgerRecord> records) {
    if (records.empty()) {
        return true;
    }

    return records.data() != nullptr;
}

bool IsSpanStorageValid(std::span<SceneEditorRenderedGizmoRow> records) {
    return IsSpanStorageValidGeneric(records);
}

bool IsSpanStorageValid(std::span<SceneEditorResourcePickerRow> records) {
    return IsSpanStorageValidGeneric(records);
}

bool IsSpanStorageValid(std::span<SceneEditorSaveLoadProofRecord> records) {
    return IsSpanStorageValidGeneric(records);
}

bool IsSpanStorageValid(
    std::span<WorldSceneObjectTransformRestoreIdentityRecord> records) {
    return IsSpanStorageValidGeneric(records);
}

bool IsSpanStorageValid(
    std::span<WorldComponentAttachmentSnapshotRecord> records) {
    return IsSpanStorageValidGeneric(records);
}

bool IsSpanStorageValid(
    std::span<WorldComponentResourceBindingSnapshotRecord> records) {
    return IsSpanStorageValidGeneric(records);
}

bool IsSpanStorageValid(std::span<std::uint8_t> bytes) {
    return IsSpanStorageValidGeneric(bytes);
}

bool IsObjectEqual(WorldObjectId left, WorldObjectId right) {
    return left.value == right.value;
}

bool IsObjectHandleActive(yuengine::object::ObjectHandle handle) {
    return handle.IsValid();
}

SceneEditorSurfaceStatus MapAuthoringStatus(WorldSceneAuthoringDocumentStatus status) {
    if (status == WorldSceneAuthoringDocumentStatus::Success) {
        return SceneEditorSurfaceStatus::Success;
    }

    if (status == WorldSceneAuthoringDocumentStatus::OutputCapacityExceeded) {
        return SceneEditorSurfaceStatus::OutputCapacityExceeded;
    }

    if (status == WorldSceneAuthoringDocumentStatus::InputCountExceeded) {
        return SceneEditorSurfaceStatus::OutputCapacityExceeded;
    }

    return SceneEditorSurfaceStatus::InvalidAuthoringDocument;
}

WorldSceneAuthoringDocumentStatus ValidateAuthoringDocument(
    const WorldSceneAuthoringDocument &document) {
    std::array<
        WorldSceneObjectTransformRestoreIdentityRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> identities{};
    std::array<
        WorldSceneObjectTransformRestoreTransformRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> transforms{};
    std::array<
        WorldComponentAttachmentSnapshotRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> attachments{};
    std::array<
        WorldComponentResourceBindingSnapshotRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> bindings{};
    std::array<
        WorldSceneAuthoringDependencyRecord,
        yuengine::world::MAX_WORLD_SCENE_AUTHORING_DEPENDENCY_COUNT> dependencies{};
    std::uint32_t identity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t attachment_count = 0U;
    std::uint32_t binding_count = 0U;
    std::uint32_t dependency_count = 0U;

    WorldSceneAuthoringRuntimeExport runtime_export{};
    runtime_export.identity_records = identities.data();
    runtime_export.identity_capacity = static_cast<std::uint32_t>(identities.size());
    runtime_export.identity_count = &identity_count;
    runtime_export.transform_records = transforms.data();
    runtime_export.transform_capacity = static_cast<std::uint32_t>(transforms.size());
    runtime_export.transform_count = &transform_count;
    runtime_export.attachment_records = attachments.data();
    runtime_export.attachment_capacity = static_cast<std::uint32_t>(attachments.size());
    runtime_export.attachment_count = &attachment_count;
    runtime_export.binding_records = bindings.data();
    runtime_export.binding_capacity = static_cast<std::uint32_t>(bindings.size());
    runtime_export.binding_count = &binding_count;
    runtime_export.dependency_records = dependencies.data();
    runtime_export.dependency_capacity = static_cast<std::uint32_t>(dependencies.size());
    runtime_export.dependency_count = &dependency_count;

    WorldSceneAuthoringDocumentValidator validator;
    return validator.ValidateAndExport(document, &runtime_export).status;
}

std::uint32_t CountSidecarKind(
    const WorldSceneAuthoringDocument &document,
    WorldSceneEditorSidecarKind kind) {
    std::uint32_t count = 0U;
    std::uint32_t index = 0U;
    while (index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &record = document.sidecar_records[index];
        if (record.kind == kind) {
            ++count;
        }

        ++index;
    }

    return count;
}

std::uint32_t CountAttachmentsForObject(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t count = 0U;
    std::uint32_t index = 0U;
    while (index < document.header.attachment_record_count) {
        const WorldComponentAttachmentSnapshotRecord &record =
            document.attachment_records[index];
        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            ++count;
        }

        ++index;
    }

    return count;
}

std::uint32_t CountBindingsForObject(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t count = 0U;
    std::uint32_t index = 0U;
    while (index < document.header.binding_record_count) {
        const WorldComponentResourceBindingSnapshotRecord &record =
            document.binding_records[index];
        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            ++count;
        }

        ++index;
    }

    return count;
}

const WorldSceneObjectTransformRestoreTransformRecord *FindTransform(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t index = 0U;
    while (index < document.header.transform_record_count) {
        const WorldSceneObjectTransformRestoreTransformRecord &record =
            document.transform_records[index];
        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            return &record;
        }

        ++index;
    }

    return nullptr;
}

const WorldSceneEditorSidecarRecord *FindSidecar(
    const WorldSceneAuthoringDocument &document,
    WorldSceneEditorSidecarKind kind,
    WorldObjectId world_object_id) {
    std::uint32_t index = 0U;
    while (index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &record = document.sidecar_records[index];
        if (record.kind != kind) {
            ++index;
            continue;
        }

        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            return &record;
        }

        ++index;
    }

    return nullptr;
}

const WorldComponentResourceBindingSnapshotRecord *FindFirstBinding(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t index = 0U;
    while (index < document.header.binding_record_count) {
        const WorldComponentResourceBindingSnapshotRecord &record =
            document.binding_records[index];
        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            return &record;
        }

        ++index;
    }

    return nullptr;
}

bool HasSidecarKindForObject(
    const WorldSceneAuthoringDocument &document,
    WorldSceneEditorSidecarKind kind,
    WorldObjectId world_object_id) {
    std::uint32_t index = 0U;
    while (index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &record = document.sidecar_records[index];
        if (record.kind != kind) {
            ++index;
            continue;
        }

        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            return true;
        }

        ++index;
    }

    return false;
}

std::uint32_t CountSidecarsForObject(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t count = 0U;
    std::uint32_t index = 0U;
    while (index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &record = document.sidecar_records[index];
        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            ++count;
        }

        ++index;
    }

    return count;
}

bool IsExpanded(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t index = 0U;
    while (index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &record = document.sidecar_records[index];
        if (record.kind != WorldSceneEditorSidecarKind::Foldout) {
            ++index;
            continue;
        }

        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            return record.value != 0U;
        }

        ++index;
    }

    return true;
}

const WorldSceneObjectTransformRestoreIdentityRecord *FindIdentity(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t index = 0U;
    while (index < document.header.identity_record_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &record =
            document.identity_records[index];
        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            return &record;
        }

        ++index;
    }

    return nullptr;
}

SceneEditorHierarchyRow BuildHierarchyRow(
    const WorldSceneAuthoringDocument &document,
    const WorldSceneObjectTransformRestoreIdentityRecord &identity,
    std::uint32_t row_index) {
    SceneEditorHierarchyRow row{};
    row.world_object_id = identity.world_object_id;
    row.object_handle = identity.object_handle;
    row.row_index = row_index;
    row.depth = 0U;
    row.component_count = CountAttachmentsForObject(document, identity.world_object_id);
    row.resource_binding_count = CountBindingsForObject(document, identity.world_object_id);
    row.has_transform = FindTransform(document, identity.world_object_id) != nullptr;
    row.visible = identity.world_object_id.IsValid();
    row.active = IsObjectHandleActive(identity.object_handle);
    row.selected = HasSidecarKindForObject(
        document,
        WorldSceneEditorSidecarKind::Selection,
        identity.world_object_id);
    row.expanded = IsExpanded(document, identity.world_object_id);
    return row;
}

SceneEditorInspectorRow BuildInspectorRow(
    const WorldSceneAuthoringDocument &document,
    const WorldSceneObjectTransformRestoreIdentityRecord &identity) {
    SceneEditorInspectorRow row{};
    row.world_object_id = identity.world_object_id;
    row.object_handle = identity.object_handle;
    const WorldSceneObjectTransformRestoreTransformRecord *transform =
        FindTransform(document, identity.world_object_id);
    row.has_transform = transform != nullptr;
    if (transform != nullptr) {
        row.transform = transform->transform_state;
    }

    row.component_count = CountAttachmentsForObject(document, identity.world_object_id);
    row.resource_binding_count = CountBindingsForObject(document, identity.world_object_id);
    row.runtime_export_field_count = 1U;
    if (row.has_transform) {
        ++row.runtime_export_field_count;
    }

    row.runtime_export_field_count += row.component_count;
    row.runtime_export_field_count += row.resource_binding_count;
    row.editor_only_sidecar_field_count =
        CountSidecarsForObject(document, identity.world_object_id);
    row.has_component_attachments = row.component_count > 0U;
    row.has_resource_bindings = row.resource_binding_count > 0U;
    row.separated_runtime_and_editor_fields = true;
    row.selected = true;
    return row;
}

void FillResultMetadata(
    const WorldSceneAuthoringDocument &document,
    SceneEditorSurfaceResult *result) {
    if (result == nullptr) {
        return;
    }

    result->scene_document_id = document.header.scene_document_id;
    result->deterministic_document_hash = document.header.deterministic_document_hash;
    result->component_attachment_count = document.header.attachment_record_count;
    result->resource_binding_count = document.header.binding_record_count;
    result->validated_sidecar_count = document.header.sidecar_record_count;
    result->selected_object_count =
        CountSidecarKind(document, WorldSceneEditorSidecarKind::Selection);
    result->folded_object_count =
        CountSidecarKind(document, WorldSceneEditorSidecarKind::Foldout);
    result->consumed_authoring_document = true;
    result->consumed_editor_sidecar = document.header.sidecar_record_count > 0U;
}

void FillSurfaceOutputRequirements(
    const WorldSceneAuthoringDocument &document,
    SceneEditorSurfaceResult *result) {
    if (result == nullptr) {
        return;
    }

    result->hierarchy_row_count = document.header.identity_record_count;
    result->inspector_row_count = result->selected_object_count;
}

void FillTransformOutputRequirements(
    const WorldSceneAuthoringDocument &document,
    SceneEditorTransformCommandResult *result) {
    if (result == nullptr) {
        return;
    }

    result->transform_record_count = document.header.transform_record_count;
    result->ledger_record_count = 1U;
}

void FillWorkflowOutputRequirements(
    const WorldSceneAuthoringDocument &document,
    SceneEditorWorkflowResult *result) {
    if (result == nullptr) {
        return;
    }

    FillResultMetadata(document, &result->surface);
    FillSurfaceOutputRequirements(document, &result->surface);
    FillTransformOutputRequirements(document, &result->transform);
    result->workflow_ledger_count = 1U;
}

void FillGizmoResourceOutputRequirements(
    const WorldSceneAuthoringDocument &document,
    SceneEditorGizmoResourceSaveLoadWorkflowResult *result) {
    if (result == nullptr) {
        return;
    }

    result->gizmo_row_count = 1U;
    result->resource_picker_row_count = 1U;
    result->save_load_record_count = 1U;
    result->loaded_identity_count = document.header.identity_record_count;
    result->loaded_transform_count = document.header.transform_record_count;
    result->loaded_attachment_count = document.header.attachment_record_count;
    result->loaded_binding_count = document.header.binding_record_count;
}

void FillGizmoResourceOutputRequirements(
    const WorldSceneAuthoringDocument &document,
    SceneEditorGizmoResourceFilePersistenceWorkflowResult *result) {
    if (result == nullptr) {
        return;
    }

    result->gizmo_row_count = 1U;
    result->resource_picker_row_count = 1U;
    result->save_load_record_count = 1U;
    result->loaded_identity_count = document.header.identity_record_count;
    result->loaded_transform_count = document.header.transform_record_count;
    result->loaded_attachment_count = document.header.attachment_record_count;
    result->loaded_binding_count = document.header.binding_record_count;
}

std::uint32_t NextCommandSequence(
    const SceneEditorTransformLedgerRecord *history_record) {
    if (history_record == nullptr) {
        return 1U;
    }

    return history_record->command_sequence + 1U;
}

WorldObjectId ResolveCommandWorldObjectId(
    const SceneEditorTransformCommandRequest &request) {
    if (request.mode == SceneEditorTransformCommandMode::Apply) {
        return request.selected_world_object_id;
    }

    if (request.history_record != nullptr) {
        return request.history_record->world_object_id;
    }

    return WorldObjectId{};
}

SceneEditorTransformCommandStatus ValidateHistoryRecord(
    const SceneEditorTransformCommandRequest &request) {
    if (request.mode == SceneEditorTransformCommandMode::Apply) {
        return SceneEditorTransformCommandStatus::Success;
    }

    if (request.mode == SceneEditorTransformCommandMode::Undo &&
        request.history_record == nullptr) {
        return SceneEditorTransformCommandStatus::UndoStackEmpty;
    }

    if (request.mode == SceneEditorTransformCommandMode::Redo &&
        request.history_record == nullptr) {
        return SceneEditorTransformCommandStatus::RedoStackEmpty;
    }

    return SceneEditorTransformCommandStatus::Success;
}

SceneEditorTransformLedgerRecord BuildTransformLedgerRecord(
    const SceneEditorTransformCommandRequest &request,
    WorldObjectId world_object_id,
    const yuengine::world::WorldTransformState &before_transform,
    const yuengine::world::WorldTransformState &after_transform) {
    SceneEditorTransformLedgerRecord record{};
    record.world_object_id = world_object_id;
    record.before_transform = before_transform;
    record.after_transform = after_transform;
    record.command_sequence = NextCommandSequence(request.history_record);
    if (request.mode == SceneEditorTransformCommandMode::Apply) {
        record.applied = true;
        record.undo_available = true;
    }

    if (request.mode == SceneEditorTransformCommandMode::Undo) {
        record.undone = true;
        record.redo_available = true;
    }

    if (request.mode == SceneEditorTransformCommandMode::Redo) {
        record.redone = true;
        record.undo_available = true;
    }

    return record;
}

WorldObjectId FirstSelectedWorldObjectId(const WorldSceneAuthoringDocument &document) {
    std::uint32_t index = 0U;
    while (index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &record = document.sidecar_records[index];
        if (record.kind == WorldSceneEditorSidecarKind::Selection &&
            record.world_object_id.IsValid()) {
            return record.world_object_id;
        }

        ++index;
    }

    return WorldObjectId{};
}

std::uint32_t HierarchyIndexForObject(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t index = 0U;
    while (index < document.header.identity_record_count) {
        if (IsObjectEqual(document.identity_records[index].world_object_id, world_object_id)) {
            return index;
        }

        ++index;
    }

    return 0U;
}

bool ResourceBrowserSelectionReady(const ResourceBrowserSurfaceSelectionState *selection) {
    if (selection == nullptr) {
        return false;
    }

    return selection->selected &&
        selection->import_settings_valid &&
        selection->preview_eligible &&
        selection->preview_state == ResourceBrowserSurfacePreviewState::Eligible &&
        selection->resource_asset_mapping_preserved &&
        !selection->used_locator_path_as_type_truth;
}

bool ViewportSessionReady(const PreviewHostViewportSessionResult *session) {
    if (session == nullptr) {
        return false;
    }

    return session->status == PreviewHostStatus::Success &&
        session->built_frame &&
        session->consumed_resource_browser_selection &&
        session->resource_browser_preview_eligible &&
        session->resource_asset_mapping_preserved &&
        session->selected_entity_available &&
        !session->used_locator_path_as_type_truth;
}

bool ViewportInteractionReady(
    const PreviewHostEditorViewportInteractionResult *interaction) {
    if (interaction == nullptr) {
        return false;
    }

    return interaction->status == PreviewHostStatus::Success &&
        interaction->consumed_viewport_session &&
        interaction->consumed_engine_viewport_frame &&
        interaction->processed_selection_command &&
        interaction->emitted_selection_feedback &&
        interaction->emitted_transform_feedback &&
        interaction->selected_world_object_id.IsValid() &&
        !interaction->opened_native_window &&
        !interaction->used_forbidden_preview_path;
}

bool WorkflowRequestStorageValid(const SceneEditorWorkflowRequest &request) {
    return IsSpanStorageValid(request.hierarchy_rows) &&
        IsSpanStorageValid(request.inspector_rows) &&
        IsSpanStorageValid(request.transform_output) &&
        IsSpanStorageValid(request.transform_ledger_output) &&
        IsSpanStorageValid(request.workflow_ledger_output);
}

bool SaveLoadWorkflowRequestStorageValid(
    const SceneEditorGizmoResourceSaveLoadWorkflowRequest &request) {
    return IsSpanStorageValid(request.persistence_buffer) &&
        IsSpanStorageValid(request.gizmo_rows) &&
        IsSpanStorageValid(request.resource_picker_rows) &&
        IsSpanStorageValid(request.save_load_records) &&
        IsSpanStorageValid(request.loaded_identity_output) &&
        IsSpanStorageValid(request.loaded_transform_output) &&
        IsSpanStorageValid(request.loaded_attachment_output) &&
        IsSpanStorageValid(request.loaded_binding_output);
}

bool FilePersistenceWorkflowRequestStorageValid(
    const SceneEditorGizmoResourceFilePersistenceWorkflowRequest &request) {
    return IsSpanStorageValid(request.persistence_buffer) &&
        IsSpanStorageValid(request.gizmo_rows) &&
        IsSpanStorageValid(request.resource_picker_rows) &&
        IsSpanStorageValid(request.save_load_records) &&
        IsSpanStorageValid(request.loaded_identity_output) &&
        IsSpanStorageValid(request.loaded_transform_output) &&
        IsSpanStorageValid(request.loaded_attachment_output) &&
        IsSpanStorageValid(request.loaded_binding_output);
}

bool WorkflowOutputCapacityReady(
    const SceneEditorWorkflowRequest &request,
    const WorldSceneAuthoringDocument &document,
    std::uint32_t selected_object_count) {
    if (request.hierarchy_rows.size() < document.header.identity_record_count) {
        return false;
    }

    if (request.inspector_rows.size() < selected_object_count) {
        return false;
    }

    if (request.transform_output.size() < document.header.transform_record_count) {
        return false;
    }

    if (request.transform_ledger_output.empty()) {
        return false;
    }

    if (request.workflow_ledger_output.empty()) {
        return false;
    }

    return true;
}

bool SaveLoadWorkflowOutputCapacityReady(
    const SceneEditorGizmoResourceSaveLoadWorkflowRequest &request,
    const WorldSceneAuthoringDocument &document) {
    if (request.gizmo_rows.empty()) {
        return false;
    }

    if (request.resource_picker_rows.empty()) {
        return false;
    }

    if (request.save_load_records.empty()) {
        return false;
    }

    if (request.persistence_buffer.empty()) {
        return false;
    }

    if (request.loaded_identity_output.size() < document.header.identity_record_count) {
        return false;
    }

    if (request.loaded_transform_output.size() < document.header.transform_record_count) {
        return false;
    }

    if (request.loaded_attachment_output.size() < document.header.attachment_record_count) {
        return false;
    }

    if (request.loaded_binding_output.size() < document.header.binding_record_count) {
        return false;
    }

    if (request.loaded_identity_output.empty()) {
        return false;
    }

    if (request.loaded_transform_output.empty()) {
        return false;
    }

    if (request.loaded_attachment_output.empty()) {
        return false;
    }

    if (request.loaded_binding_output.empty()) {
        return false;
    }

    return true;
}

bool FilePersistenceWorkflowOutputCapacityReady(
    const SceneEditorGizmoResourceFilePersistenceWorkflowRequest &request,
    const WorldSceneAuthoringDocument &document) {
    if (request.gizmo_rows.empty()) {
        return false;
    }

    if (request.resource_picker_rows.empty()) {
        return false;
    }

    if (request.save_load_records.empty()) {
        return false;
    }

    if (request.persistence_buffer.empty()) {
        return false;
    }

    if (request.loaded_identity_output.size() < document.header.identity_record_count) {
        return false;
    }

    if (request.loaded_transform_output.size() < document.header.transform_record_count) {
        return false;
    }

    if (request.loaded_attachment_output.size() < document.header.attachment_record_count) {
        return false;
    }

    if (request.loaded_binding_output.size() < document.header.binding_record_count) {
        return false;
    }

    if (request.loaded_identity_output.empty()) {
        return false;
    }

    if (request.loaded_transform_output.empty()) {
        return false;
    }

    if (request.loaded_attachment_output.empty()) {
        return false;
    }

    if (request.loaded_binding_output.empty()) {
        return false;
    }

    return true;
}

bool ResourceTypeMatches(
    yuengine::resource::ResourceTypeId left,
    yuengine::resource::ResourceTypeId right) {
    return left.value == right.value;
}

SceneEditorRenderedGizmoRow BuildRenderedGizmoRow(
    const WorldSceneAuthoringDocument &document,
    const PreviewHostEditorViewportInteractionResult &interaction,
    const WorldSceneEditorSidecarRecord &gizmo_sidecar) {
    SceneEditorRenderedGizmoRow row{};
    row.world_object_id = interaction.selected_world_object_id;
    const WorldSceneObjectTransformRestoreTransformRecord *transform =
        FindTransform(document, interaction.selected_world_object_id);
    if (transform != nullptr) {
        row.transform = transform->transform_state;
        row.transform_available = true;
    }

    row.gizmo_mode = gizmo_sidecar.value;
    row.viewport_selected_entity_index = interaction.selected_entity_index;
    row.selected = true;
    row.consumed_preview_host_feedback = interaction.emitted_transform_feedback;
    row.rendered_from_engine_viewport = interaction.consumed_engine_viewport_frame;
    return row;
}

SceneEditorResourcePickerRow BuildResourcePickerRow(
    const ResourceBrowserSurfaceSelectionState &selection,
    const WorldComponentResourceBindingSnapshotRecord &binding) {
    SceneEditorResourcePickerRow row{};
    row.world_object_id = binding.world_object_id;
    row.component_type_id = binding.component_type_id;
    row.component_slot_id = binding.component_slot_id;
    row.current_resource = binding.resource_handle;
    row.expected_resource_type = binding.expected_resource_type;
    row.selected_resource = selection.resource;
    row.selected_resource_type = selection.import_settings.resource_type;
    row.current_binding_available = true;
    row.resource_browser_selection_ready = true;
    row.resource_asset_mapping_preserved = selection.resource_asset_mapping_preserved;
    row.selected_resource_matches_binding_type =
        ResourceTypeMatches(row.selected_resource_type, row.expected_resource_type);
    return row;
}

SceneEditorSaveLoadProofRecord BuildSaveLoadProofRecord(
    const WorldSceneAuthoringDocument &document,
    const WorldSceneRecordValueStreamResult &write_result,
    const WorldSceneRecordValueStreamResult &read_result,
    std::uint32_t loaded_identity_count,
    std::uint32_t loaded_transform_count,
    std::uint32_t loaded_attachment_count,
    std::uint32_t loaded_binding_count) {
    SceneEditorSaveLoadProofRecord record{};
    record.write_status = write_result.status;
    record.read_status = read_result.status;
    record.saved_identity_count = document.header.identity_record_count;
    record.saved_transform_count = document.header.transform_record_count;
    record.saved_attachment_count = document.header.attachment_record_count;
    record.saved_binding_count = document.header.binding_record_count;
    record.loaded_identity_count = loaded_identity_count;
    record.loaded_transform_count = loaded_transform_count;
    record.loaded_attachment_count = loaded_attachment_count;
    record.loaded_binding_count = loaded_binding_count;
    record.committed_byte_count = write_result.state.committed_byte_count;
    record.skipped_editor_sidecar_count = document.header.sidecar_record_count;
    record.wrote_scene_records = write_result.Succeeded();
    record.read_scene_records = read_result.Succeeded();
    record.preserved_runtime_record_counts =
        loaded_identity_count == document.header.identity_record_count &&
        loaded_transform_count == document.header.transform_record_count &&
        loaded_attachment_count == document.header.attachment_record_count &&
        loaded_binding_count == document.header.binding_record_count;
    record.kept_editor_sidecars_out_of_runtime_stream =
        document.header.sidecar_record_count > 0U;
    return record;
}

SceneEditorWorkflowLedgerRecord BuildWorkflowLedger(
    const SceneEditorWorkflowRequest &request,
    const SceneEditorWorkflowResult &result) {
    SceneEditorWorkflowLedgerRecord record{};
    record.status = result.status;
    record.blocked_layer = result.blocked_layer;
    record.transform_mode = request.transform_mode;
    record.selected_world_object_id = result.selected_world_object_id;
    record.selected_hierarchy_index = result.selected_hierarchy_index;
    record.viewport_selected_entity_index = result.viewport_selected_entity_index;
    record.transform_command_sequence = result.transform.ledger_record_count > 0U
        ? request.transform_ledger_output[0U].command_sequence
        : 0U;
    record.consumed_resource_browser_selection =
        result.consumed_resource_browser_selection;
    record.consumed_viewport_session = result.consumed_viewport_session;
    record.consumed_viewport_interaction = result.consumed_viewport_interaction;
    record.matched_hierarchy_to_viewport_selection =
        result.hierarchy_selection_matched_viewport;
    record.emitted_inspector_rows = result.emitted_inspector_rows;
    record.applied_transform_command = result.applied_transform_command;
    record.replayed_undo =
        request.transform_mode == SceneEditorTransformCommandMode::Undo &&
        result.applied_transform_command;
    record.replayed_redo =
        request.transform_mode == SceneEditorTransformCommandMode::Redo &&
        result.applied_transform_command;
    record.committed_workflow = result.committed_workflow;
    return record;
}

}

SceneEditorSurfaceStatus BuildSceneEditorNativeSurface(
    const SceneEditorSurfaceRequest &request,
    SceneEditorSurfaceResult *out_result) {
    if (out_result == nullptr) {
        return SceneEditorSurfaceStatus::InvalidArgument;
    }

    SceneEditorSurfaceResult result{};
    if (request.document == nullptr ||
        !IsSpanStorageValid(request.hierarchy_rows) ||
        !IsSpanStorageValid(request.inspector_rows)) {
        *out_result = result;
        return result.status;
    }

    const WorldSceneAuthoringDocument &document = *request.document;
    const WorldSceneAuthoringDocumentStatus authoring_status =
        ValidateAuthoringDocument(document);
    result.authoring_status = authoring_status;
    if (authoring_status != WorldSceneAuthoringDocumentStatus::Success) {
        result.status = MapAuthoringStatus(authoring_status);
        *out_result = result;
        return result.status;
    }

    FillResultMetadata(document, &result);
    if (request.hierarchy_rows.size() < document.header.identity_record_count) {
        result.status = SceneEditorSurfaceStatus::OutputCapacityExceeded;
        FillSurfaceOutputRequirements(document, &result);
        *out_result = result;
        return result.status;
    }

    if (request.inspector_rows.size() < result.selected_object_count) {
        result.status = SceneEditorSurfaceStatus::OutputCapacityExceeded;
        FillSurfaceOutputRequirements(document, &result);
        *out_result = result;
        return result.status;
    }

    if (request.require_selection && result.selected_object_count == 0U) {
        result.status = SceneEditorSurfaceStatus::SelectionRequired;
        *out_result = result;
        return result.status;
    }

    std::uint32_t identity_index = 0U;
    while (identity_index < document.header.identity_record_count) {
        request.hierarchy_rows[identity_index] = BuildHierarchyRow(
            document,
            document.identity_records[identity_index],
            identity_index);
        ++result.hierarchy_row_count;
        ++identity_index;
    }

    std::uint32_t sidecar_index = 0U;
    while (sidecar_index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &sidecar =
            document.sidecar_records[sidecar_index];
        if (sidecar.kind != WorldSceneEditorSidecarKind::Selection) {
            ++sidecar_index;
            continue;
        }

        const WorldSceneObjectTransformRestoreIdentityRecord *identity =
            FindIdentity(document, sidecar.world_object_id);
        if (identity != nullptr) {
            request.inspector_rows[result.inspector_row_count] =
                BuildInspectorRow(document, *identity);
            ++result.inspector_row_count;
        }

        ++sidecar_index;
    }

    result.status = SceneEditorSurfaceStatus::Success;
    *out_result = result;
    return result.status;
}

SceneEditorTransformCommandStatus ApplySceneEditorTransformCommand(
    const SceneEditorTransformCommandRequest &request,
    SceneEditorTransformCommandResult *out_result) {
    if (out_result == nullptr) {
        return SceneEditorTransformCommandStatus::InvalidArgument;
    }

    SceneEditorTransformCommandResult result{};
    if (request.document == nullptr ||
        !IsSpanStorageValid(request.transform_output) ||
        !IsSpanStorageValid(request.ledger_output)) {
        *out_result = result;
        return result.status;
    }

    const WorldSceneAuthoringDocument &document = *request.document;
    result.consumed_authoring_document = true;
    const WorldSceneAuthoringDocumentStatus authoring_status =
        ValidateAuthoringDocument(document);
    result.authoring_status = authoring_status;
    if (authoring_status != WorldSceneAuthoringDocumentStatus::Success) {
        result.status = SceneEditorTransformCommandStatus::InvalidAuthoringDocument;
        result.blocked_layer = SceneEditorTransformCommandBlockedLayer::AuthoringDocument;
        *out_result = result;
        return result.status;
    }

    const SceneEditorTransformCommandStatus history_status =
        ValidateHistoryRecord(request);
    if (history_status != SceneEditorTransformCommandStatus::Success) {
        result.status = history_status;
        result.blocked_layer = SceneEditorTransformCommandBlockedLayer::UndoRedoLedger;
        *out_result = result;
        return result.status;
    }

    const WorldObjectId world_object_id = ResolveCommandWorldObjectId(request);
    result.selected_world_object_id = world_object_id;
    if (!world_object_id.IsValid()) {
        result.status = SceneEditorTransformCommandStatus::SelectionRequired;
        result.blocked_layer = SceneEditorTransformCommandBlockedLayer::Selection;
        *out_result = result;
        return result.status;
    }

    result.consumed_selection = true;
    if (FindIdentity(document, world_object_id) == nullptr) {
        result.status = SceneEditorTransformCommandStatus::ObjectNotFound;
        result.blocked_layer = SceneEditorTransformCommandBlockedLayer::Selection;
        *out_result = result;
        return result.status;
    }

    const WorldSceneObjectTransformRestoreTransformRecord *selected_transform =
        FindTransform(document, world_object_id);
    if (selected_transform == nullptr) {
        result.status = SceneEditorTransformCommandStatus::TransformNotFound;
        result.blocked_layer = SceneEditorTransformCommandBlockedLayer::TransformRecord;
        *out_result = result;
        return result.status;
    }

    if (request.transform_output.size() < document.header.transform_record_count ||
        request.ledger_output.empty()) {
        result.status = SceneEditorTransformCommandStatus::OutputCapacityExceeded;
        result.blocked_layer = SceneEditorTransformCommandBlockedLayer::Output;
        *out_result = result;
        return result.status;
    }

    result.previous_transform = selected_transform->transform_state;
    result.current_transform = request.requested_transform;
    if (request.mode == SceneEditorTransformCommandMode::Undo &&
        request.history_record != nullptr) {
        result.previous_transform = request.history_record->after_transform;
        result.current_transform = request.history_record->before_transform;
    }

    if (request.mode == SceneEditorTransformCommandMode::Redo &&
        request.history_record != nullptr) {
        result.previous_transform = request.history_record->before_transform;
        result.current_transform = request.history_record->after_transform;
    }

    std::array<
        WorldSceneObjectTransformRestoreTransformRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> staged_transforms{};
    std::uint32_t transform_index = 0U;
    while (transform_index < document.header.transform_record_count) {
        staged_transforms[transform_index] = document.transform_records[transform_index];
        if (IsObjectEqual(
                staged_transforms[transform_index].world_object_id,
                world_object_id)) {
            staged_transforms[transform_index].transform_state = result.current_transform;
        }

        ++transform_index;
    }

    SceneEditorTransformLedgerRecord ledger_record = BuildTransformLedgerRecord(
        request,
        world_object_id,
        result.previous_transform,
        result.current_transform);
    transform_index = 0U;
    while (transform_index < document.header.transform_record_count) {
        request.transform_output[transform_index] = staged_transforms[transform_index];
        ++result.transform_record_count;
        ++transform_index;
    }

    request.ledger_output[0U] = ledger_record;
    result.ledger_record_count = 1U;
    result.undo_available = ledger_record.undo_available;
    result.redo_available = ledger_record.redo_available;
    result.wrote_transform_output = true;
    result.emitted_undo_redo_ledger = true;
    result.blocked_layer = SceneEditorTransformCommandBlockedLayer::None;
    result.status = SceneEditorTransformCommandStatus::Success;
    *out_result = result;
    return result.status;
}

SceneEditorWorkflowStatus BuildSceneEditorUsableWorkflowSurface(
    const SceneEditorWorkflowRequest &request,
    SceneEditorWorkflowResult *out_result) {
    if (out_result == nullptr) {
        return SceneEditorWorkflowStatus::InvalidArgument;
    }

    SceneEditorWorkflowResult result{};
    if (request.document == nullptr || !WorkflowRequestStorageValid(request)) {
        *out_result = result;
        return result.status;
    }

    const WorldSceneAuthoringDocument &document = *request.document;
    result.consumed_authoring_document = true;
    const WorldSceneAuthoringDocumentStatus authoring_status =
        ValidateAuthoringDocument(document);
    result.surface.authoring_status = authoring_status;
    if (authoring_status != WorldSceneAuthoringDocumentStatus::Success) {
        result.status = SceneEditorWorkflowStatus::InvalidAuthoringDocument;
        result.blocked_layer = SceneEditorWorkflowBlockedLayer::AuthoringDocument;
        result.surface_status = MapAuthoringStatus(authoring_status);
        *out_result = result;
        return result.status;
    }

    const std::uint32_t selected_object_count =
        CountSidecarKind(document, WorldSceneEditorSidecarKind::Selection);
    if (!WorkflowOutputCapacityReady(request, document, selected_object_count)) {
        result.status = SceneEditorWorkflowStatus::OutputCapacityExceeded;
        result.blocked_layer = SceneEditorWorkflowBlockedLayer::Output;
        result.surface_status = SceneEditorSurfaceStatus::OutputCapacityExceeded;
        result.transform_status = SceneEditorTransformCommandStatus::OutputCapacityExceeded;
        result.surface.status = SceneEditorSurfaceStatus::OutputCapacityExceeded;
        result.transform.status = SceneEditorTransformCommandStatus::OutputCapacityExceeded;
        result.transform.blocked_layer = SceneEditorTransformCommandBlockedLayer::Output;
        FillWorkflowOutputRequirements(document, &result);
        *out_result = result;
        return result.status;
    }

    const WorldObjectId selected_world_object_id = FirstSelectedWorldObjectId(document);
    result.selected_world_object_id = selected_world_object_id;
    result.selected_hierarchy_index = HierarchyIndexForObject(document, selected_world_object_id);
    if (!selected_world_object_id.IsValid()) {
        result.status = SceneEditorWorkflowStatus::SelectionRequired;
        result.blocked_layer = SceneEditorWorkflowBlockedLayer::AuthoringDocument;
        result.surface_status = SceneEditorSurfaceStatus::SelectionRequired;
        *out_result = result;
        return result.status;
    }

    result.resource_preview_state = request.resource_browser_selection != nullptr
        ? request.resource_browser_selection->preview_state
        : ResourceBrowserSurfacePreviewState::Unknown;
    if (!ResourceBrowserSelectionReady(request.resource_browser_selection)) {
        result.status = SceneEditorWorkflowStatus::BlockedResourceBrowserSelection;
        result.blocked_layer = SceneEditorWorkflowBlockedLayer::ResourceBrowserSelection;
        *out_result = result;
        return result.status;
    }

    result.consumed_resource_browser_selection = true;
    result.viewport_status = request.viewport_session != nullptr
        ? request.viewport_session->status
        : PreviewHostStatus::InvalidArgument;
    if (!ViewportSessionReady(request.viewport_session)) {
        result.status = SceneEditorWorkflowStatus::ViewportSessionFailed;
        result.blocked_layer = SceneEditorWorkflowBlockedLayer::ViewportSession;
        *out_result = result;
        return result.status;
    }

    result.consumed_viewport_session = true;
    result.viewport_interaction_status = request.viewport_interaction != nullptr
        ? request.viewport_interaction->status
        : PreviewHostStatus::InvalidArgument;
    if (!ViewportInteractionReady(request.viewport_interaction)) {
        result.status = SceneEditorWorkflowStatus::ViewportInteractionFailed;
        result.blocked_layer = SceneEditorWorkflowBlockedLayer::ViewportInteraction;
        *out_result = result;
        return result.status;
    }

    result.consumed_viewport_interaction = true;
    result.viewport_selected_entity_index =
        request.viewport_interaction->selected_entity_index;
    if (!IsObjectEqual(
            selected_world_object_id,
            request.viewport_interaction->selected_world_object_id)) {
        result.status = SceneEditorWorkflowStatus::ViewportInteractionFailed;
        result.blocked_layer = SceneEditorWorkflowBlockedLayer::ViewportInteraction;
        *out_result = result;
        return result.status;
    }

    result.hierarchy_selection_matched_viewport = true;

    std::array<
        WorldSceneObjectTransformRestoreTransformRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> staged_transform_output{};
    std::array<SceneEditorTransformLedgerRecord, 1U> staged_transform_ledger{};
    SceneEditorTransformCommandRequest transform_request{};
    transform_request.document = &document;
    transform_request.selected_world_object_id = selected_world_object_id;
    transform_request.requested_transform = request.requested_transform;
    transform_request.history_record = request.history_record;
    transform_request.mode = request.transform_mode;
    transform_request.transform_output =
        std::span<WorldSceneObjectTransformRestoreTransformRecord>(
            staged_transform_output.data(),
            staged_transform_output.size());
    transform_request.ledger_output =
        std::span<SceneEditorTransformLedgerRecord>(
            staged_transform_ledger.data(),
            staged_transform_ledger.size());
    ApplySceneEditorTransformCommand(transform_request, &result.transform);
    result.transform_status = result.transform.status;
    if (!result.transform.Succeeded()) {
        result.status = SceneEditorWorkflowStatus::TransformCommandFailed;
        result.blocked_layer = SceneEditorWorkflowBlockedLayer::TransformCommand;
        *out_result = result;
        return result.status;
    }

    SceneEditorSurfaceRequest surface_request{};
    surface_request.document = &document;
    surface_request.hierarchy_rows = request.hierarchy_rows;
    surface_request.inspector_rows = request.inspector_rows;
    surface_request.require_selection = true;
    BuildSceneEditorNativeSurface(surface_request, &result.surface);
    result.surface_status = result.surface.status;
    if (!result.surface.Succeeded()) {
        result.status = SceneEditorWorkflowStatus::InvalidAuthoringDocument;
        result.blocked_layer = SceneEditorWorkflowBlockedLayer::AuthoringDocument;
        *out_result = result;
        return result.status;
    }

    std::uint32_t transform_index = 0U;
    while (transform_index < result.transform.transform_record_count) {
        request.transform_output[transform_index] =
            staged_transform_output[transform_index];
        ++transform_index;
    }

    request.transform_ledger_output[0U] = staged_transform_ledger[0U];
    result.status = SceneEditorWorkflowStatus::Success;
    result.blocked_layer = SceneEditorWorkflowBlockedLayer::None;
    result.emitted_hierarchy_rows = result.surface.hierarchy_row_count > 0U;
    result.emitted_inspector_rows = result.surface.inspector_row_count > 0U;
    result.applied_transform_command = true;
    result.emitted_transform_ledger = true;
    result.committed_workflow = true;
    result.workflow_ledger_count = 1U;
    request.workflow_ledger_output[0U] = BuildWorkflowLedger(request, result);
    *out_result = result;
    return result.status;
}

SceneEditorGizmoResourceWorkflowStatus BuildSceneEditorGizmoResourceSaveLoadWorkflow(
    const SceneEditorGizmoResourceSaveLoadWorkflowRequest &request,
    SceneEditorGizmoResourceSaveLoadWorkflowResult *out_result) {
    if (out_result == nullptr) {
        return SceneEditorGizmoResourceWorkflowStatus::InvalidArgument;
    }

    SceneEditorGizmoResourceSaveLoadWorkflowResult result{};
    if (request.document == nullptr ||
        !SaveLoadWorkflowRequestStorageValid(request)) {
        *out_result = result;
        return result.status;
    }

    const WorldSceneAuthoringDocument &document = *request.document;
    result.consumed_authoring_document = true;
    const WorldSceneAuthoringDocumentStatus authoring_status =
        ValidateAuthoringDocument(document);
    result.authoring_status = authoring_status;
    if (authoring_status != WorldSceneAuthoringDocumentStatus::Success) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::InvalidAuthoringDocument;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::AuthoringDocument;
        *out_result = result;
        return result.status;
    }

    if (!SaveLoadWorkflowOutputCapacityReady(request, document)) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::OutputCapacityExceeded;
        result.blocked_layer = SceneEditorGizmoResourceWorkflowBlockedLayer::Output;
        result.save_status = WorldSceneRecordValueStreamStatus::OutputCapacityExceeded;
        result.load_status = WorldSceneRecordValueStreamStatus::OutputCapacityExceeded;
        FillGizmoResourceOutputRequirements(document, &result);
        *out_result = result;
        return result.status;
    }

    const WorldObjectId selected_world_object_id = FirstSelectedWorldObjectId(document);
    result.selected_world_object_id = selected_world_object_id;
    if (!selected_world_object_id.IsValid()) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SelectionRequired;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::AuthoringDocument;
        *out_result = result;
        return result.status;
    }

    result.resource_preview_state = request.resource_browser_selection != nullptr
        ? request.resource_browser_selection->preview_state
        : ResourceBrowserSurfacePreviewState::Unknown;
    if (!ResourceBrowserSelectionReady(request.resource_browser_selection)) {
        result.status =
            SceneEditorGizmoResourceWorkflowStatus::BlockedResourceBrowserSelection;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ResourceBrowserSelection;
        *out_result = result;
        return result.status;
    }

    result.consumed_resource_browser_selection = true;
    result.viewport_status = request.viewport_session != nullptr
        ? request.viewport_session->status
        : PreviewHostStatus::InvalidArgument;
    if (!ViewportSessionReady(request.viewport_session)) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::ViewportSessionFailed;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ViewportSession;
        *out_result = result;
        return result.status;
    }

    result.consumed_viewport_session = true;
    result.viewport_interaction_status = request.viewport_interaction != nullptr
        ? request.viewport_interaction->status
        : PreviewHostStatus::InvalidArgument;
    if (!ViewportInteractionReady(request.viewport_interaction)) {
        result.status =
            SceneEditorGizmoResourceWorkflowStatus::ViewportInteractionFailed;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ViewportInteraction;
        *out_result = result;
        return result.status;
    }

    result.consumed_viewport_interaction = true;
    result.viewport_selected_entity_index =
        request.viewport_interaction->selected_entity_index;
    if (!IsObjectEqual(
            selected_world_object_id,
            request.viewport_interaction->selected_world_object_id)) {
        result.status =
            SceneEditorGizmoResourceWorkflowStatus::ViewportInteractionFailed;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ViewportInteraction;
        *out_result = result;
        return result.status;
    }

    const WorldSceneEditorSidecarRecord *gizmo_sidecar = FindSidecar(
        document,
        WorldSceneEditorSidecarKind::GizmoMode,
        selected_world_object_id);
    if (gizmo_sidecar == nullptr || gizmo_sidecar->value == 0U) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::GizmoUnavailable;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::GizmoSidecar;
        *out_result = result;
        return result.status;
    }

    const WorldComponentResourceBindingSnapshotRecord *binding =
        FindFirstBinding(document, selected_world_object_id);
    if (binding == nullptr) {
        result.status =
            SceneEditorGizmoResourceWorkflowStatus::ResourceBindingUnavailable;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ResourceBinding;
        *out_result = result;
        return result.status;
    }

    SceneEditorRenderedGizmoRow staged_gizmo = BuildRenderedGizmoRow(
        document,
        *request.viewport_interaction,
        *gizmo_sidecar);
    SceneEditorResourcePickerRow staged_picker = BuildResourcePickerRow(
        *request.resource_browser_selection,
        *binding);
    if (!staged_gizmo.transform_available ||
        !staged_picker.selected_resource.IsValid() ||
        !staged_picker.selected_resource_matches_binding_type) {
        result.status =
            SceneEditorGizmoResourceWorkflowStatus::ResourceBindingUnavailable;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ResourceBinding;
        *out_result = result;
        return result.status;
    }

    WorldSceneRecordValueStreamBridge stream_bridge;
    SerializeWriter writer(
        request.persistence_buffer.data(),
        static_cast<std::uint32_t>(request.persistence_buffer.size()));
    const WorldSceneRecordValueStreamResult write_result =
        stream_bridge.WriteSceneRecords(
            &writer,
            document.identity_records,
            document.header.identity_record_count,
            document.transform_records,
            document.header.transform_record_count,
            document.attachment_records,
            document.header.attachment_record_count,
            document.binding_records,
            document.header.binding_record_count);
    result.save_status = write_result.status;
    if (!write_result.Succeeded()) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SaveLoadFailed;
        result.blocked_layer = SceneEditorGizmoResourceWorkflowBlockedLayer::SaveLoad;
        *out_result = result;
        return result.status;
    }

    result.wrote_scene_record_stream = true;
    std::uint32_t loaded_identity_count = 0U;
    std::uint32_t loaded_transform_count = 0U;
    std::uint32_t loaded_attachment_count = 0U;
    std::uint32_t loaded_binding_count = 0U;
    SerializeReader reader(
        request.persistence_buffer.data(),
        write_result.state.committed_byte_count);
    const WorldSceneRecordValueStreamResult read_result =
        stream_bridge.ReadSceneRecords(
            &reader,
            request.loaded_identity_output.data(),
            static_cast<std::uint32_t>(request.loaded_identity_output.size()),
            &loaded_identity_count,
            request.loaded_transform_output.data(),
            static_cast<std::uint32_t>(request.loaded_transform_output.size()),
            &loaded_transform_count,
            request.loaded_attachment_output.data(),
            static_cast<std::uint32_t>(request.loaded_attachment_output.size()),
            &loaded_attachment_count,
            request.loaded_binding_output.data(),
            static_cast<std::uint32_t>(request.loaded_binding_output.size()),
            &loaded_binding_count);
    result.load_status = read_result.status;
    if (!read_result.Succeeded()) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SaveLoadFailed;
        result.blocked_layer = SceneEditorGizmoResourceWorkflowBlockedLayer::SaveLoad;
        *out_result = result;
        return result.status;
    }

    SceneEditorSaveLoadProofRecord staged_save_load = BuildSaveLoadProofRecord(
        document,
        write_result,
        read_result,
        loaded_identity_count,
        loaded_transform_count,
        loaded_attachment_count,
        loaded_binding_count);
    if (!staged_save_load.preserved_runtime_record_counts) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SaveLoadFailed;
        result.blocked_layer = SceneEditorGizmoResourceWorkflowBlockedLayer::SaveLoad;
        *out_result = result;
        return result.status;
    }

    request.gizmo_rows[0U] = staged_gizmo;
    request.resource_picker_rows[0U] = staged_picker;
    request.save_load_records[0U] = staged_save_load;
    result.loaded_identity_count = loaded_identity_count;
    result.loaded_transform_count = loaded_transform_count;
    result.loaded_attachment_count = loaded_attachment_count;
    result.loaded_binding_count = loaded_binding_count;
    result.gizmo_row_count = 1U;
    result.resource_picker_row_count = 1U;
    result.save_load_record_count = 1U;
    result.read_scene_record_stream = true;
    result.skipped_editor_sidecars_for_runtime_stream =
        staged_save_load.kept_editor_sidecars_out_of_runtime_stream;
    result.emitted_rendered_gizmo = true;
    result.emitted_resource_picker = true;
    result.committed_workflow = true;
    result.status = SceneEditorGizmoResourceWorkflowStatus::Success;
    result.blocked_layer = SceneEditorGizmoResourceWorkflowBlockedLayer::None;
    *out_result = result;
    return result.status;
}

SceneEditorGizmoResourceWorkflowStatus BuildSceneEditorGizmoResourceFilePersistenceWorkflow(
    const SceneEditorGizmoResourceFilePersistenceWorkflowRequest &request,
    SceneEditorGizmoResourceFilePersistenceWorkflowResult *out_result) {
    if (out_result == nullptr) {
        return SceneEditorGizmoResourceWorkflowStatus::InvalidArgument;
    }

    SceneEditorGizmoResourceFilePersistenceWorkflowResult result{};
    if (request.document == nullptr ||
        !FilePersistenceWorkflowRequestStorageValid(request)) {
        *out_result = result;
        return result.status;
    }

    const WorldSceneAuthoringDocument &document = *request.document;
    result.consumed_authoring_document = true;
    const WorldSceneAuthoringDocumentStatus authoring_status =
        ValidateAuthoringDocument(document);
    result.authoring_status = authoring_status;
    if (authoring_status != WorldSceneAuthoringDocumentStatus::Success) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::InvalidAuthoringDocument;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::AuthoringDocument;
        *out_result = result;
        return result.status;
    }

    if (!FilePersistenceWorkflowOutputCapacityReady(request, document)) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::OutputCapacityExceeded;
        result.blocked_layer = SceneEditorGizmoResourceWorkflowBlockedLayer::Output;
        result.save_status = WorldSceneRecordValueStreamStatus::OutputCapacityExceeded;
        result.load_status = WorldSceneRecordValueStreamStatus::OutputCapacityExceeded;
        FillGizmoResourceOutputRequirements(document, &result);
        *out_result = result;
        return result.status;
    }

    if (request.scene_persistence_mount_table == nullptr ||
        !request.scene_persistence_mount.IsValid() ||
        request.scene_persistence_path.ByteLength() == 0U) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SaveLoadFailed;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::FilePersistence;
        *out_result = result;
        return result.status;
    }

    const WorldObjectId selected_world_object_id = FirstSelectedWorldObjectId(document);
    result.selected_world_object_id = selected_world_object_id;
    if (!selected_world_object_id.IsValid()) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SelectionRequired;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::AuthoringDocument;
        *out_result = result;
        return result.status;
    }

    result.resource_preview_state = request.resource_browser_selection != nullptr
        ? request.resource_browser_selection->preview_state
        : ResourceBrowserSurfacePreviewState::Unknown;
    if (!ResourceBrowserSelectionReady(request.resource_browser_selection)) {
        result.status =
            SceneEditorGizmoResourceWorkflowStatus::BlockedResourceBrowserSelection;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ResourceBrowserSelection;
        *out_result = result;
        return result.status;
    }

    result.consumed_resource_browser_selection = true;
    result.viewport_status = request.viewport_session != nullptr
        ? request.viewport_session->status
        : PreviewHostStatus::InvalidArgument;
    if (!ViewportSessionReady(request.viewport_session)) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::ViewportSessionFailed;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ViewportSession;
        *out_result = result;
        return result.status;
    }

    result.consumed_viewport_session = true;
    result.viewport_interaction_status = request.viewport_interaction != nullptr
        ? request.viewport_interaction->status
        : PreviewHostStatus::InvalidArgument;
    if (!ViewportInteractionReady(request.viewport_interaction)) {
        result.status =
            SceneEditorGizmoResourceWorkflowStatus::ViewportInteractionFailed;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ViewportInteraction;
        *out_result = result;
        return result.status;
    }

    result.consumed_viewport_interaction = true;
    result.viewport_selected_entity_index =
        request.viewport_interaction->selected_entity_index;
    if (!IsObjectEqual(
            selected_world_object_id,
            request.viewport_interaction->selected_world_object_id)) {
        result.status =
            SceneEditorGizmoResourceWorkflowStatus::ViewportInteractionFailed;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ViewportInteraction;
        *out_result = result;
        return result.status;
    }

    const WorldSceneEditorSidecarRecord *gizmo_sidecar = FindSidecar(
        document,
        WorldSceneEditorSidecarKind::GizmoMode,
        selected_world_object_id);
    if (gizmo_sidecar == nullptr || gizmo_sidecar->value == 0U) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::GizmoUnavailable;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::GizmoSidecar;
        *out_result = result;
        return result.status;
    }

    const WorldComponentResourceBindingSnapshotRecord *binding =
        FindFirstBinding(document, selected_world_object_id);
    if (binding == nullptr) {
        result.status =
            SceneEditorGizmoResourceWorkflowStatus::ResourceBindingUnavailable;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ResourceBinding;
        *out_result = result;
        return result.status;
    }

    SceneEditorRenderedGizmoRow staged_gizmo = BuildRenderedGizmoRow(
        document,
        *request.viewport_interaction,
        *gizmo_sidecar);
    SceneEditorResourcePickerRow staged_picker = BuildResourcePickerRow(
        *request.resource_browser_selection,
        *binding);
    if (!staged_gizmo.transform_available ||
        !staged_picker.selected_resource.IsValid() ||
        !staged_picker.selected_resource_matches_binding_type) {
        result.status =
            SceneEditorGizmoResourceWorkflowStatus::ResourceBindingUnavailable;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::ResourceBinding;
        *out_result = result;
        return result.status;
    }

    WorldSceneRecordValueStreamBridge stream_bridge;
    std::vector<std::uint8_t> staged_persistence(request.persistence_buffer.size());
    SerializeWriter writer(
        staged_persistence.data(),
        static_cast<std::uint32_t>(staged_persistence.size()));
    const WorldSceneRecordValueStreamResult write_result =
        stream_bridge.WriteSceneRecords(
            &writer,
            document.identity_records,
            document.header.identity_record_count,
            document.transform_records,
            document.header.transform_record_count,
            document.attachment_records,
            document.header.attachment_record_count,
            document.binding_records,
            document.header.binding_record_count);
    result.save_status = write_result.status;
    if (!write_result.Succeeded()) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SaveLoadFailed;
        result.blocked_layer = SceneEditorGizmoResourceWorkflowBlockedLayer::SaveLoad;
        *out_result = result;
        return result.status;
    }

    result.wrote_scene_record_stream = true;
    FileWriteRequest file_write_request{};
    file_write_request.mount = request.scene_persistence_mount;
    file_write_request.path = request.scene_persistence_path;
    file_write_request.bytes = staged_persistence.data();
    file_write_request.byte_count = write_result.state.committed_byte_count;
    const FileWriteResult file_write_result =
        request.scene_persistence_mount_table->Write(file_write_request);
    result.file_write_status = file_write_result.status;
    if (!file_write_result.Succeeded()) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SaveLoadFailed;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::FilePersistence;
        *out_result = result;
        return result.status;
    }

    result.wrote_file_scene_artifact = true;
    result.persisted_file_byte_count =
        static_cast<std::uint32_t>(file_write_result.byte_count);
    FileReadRequest file_read_request{};
    file_read_request.mount = request.scene_persistence_mount;
    file_read_request.path = request.scene_persistence_path;
    const FileReadResult file_read_result =
        request.scene_persistence_mount_table->Read(file_read_request);
    result.file_read_status = file_read_result.status;
    if (!file_read_result.Succeeded()) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SaveLoadFailed;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::FilePersistence;
        *out_result = result;
        return result.status;
    }

    result.read_file_scene_artifact = true;
    result.read_file_byte_count =
        static_cast<std::uint32_t>(file_read_result.bytes.size());
    if (file_read_result.bytes.size() != write_result.state.committed_byte_count) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SaveLoadFailed;
        result.blocked_layer =
            SceneEditorGizmoResourceWorkflowBlockedLayer::FilePersistence;
        *out_result = result;
        return result.status;
    }

    std::array<
        WorldSceneObjectTransformRestoreIdentityRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> staged_loaded_identities{};
    std::array<
        WorldSceneObjectTransformRestoreTransformRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> staged_loaded_transforms{};
    std::array<
        WorldComponentAttachmentSnapshotRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> staged_loaded_attachments{};
    std::array<
        WorldComponentResourceBindingSnapshotRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> staged_loaded_bindings{};
    std::uint32_t loaded_identity_count = 0U;
    std::uint32_t loaded_transform_count = 0U;
    std::uint32_t loaded_attachment_count = 0U;
    std::uint32_t loaded_binding_count = 0U;
    SerializeReader reader(
        file_read_result.bytes.data(),
        static_cast<std::uint32_t>(file_read_result.bytes.size()));
    const WorldSceneRecordValueStreamResult read_result =
        stream_bridge.ReadSceneRecords(
            &reader,
            staged_loaded_identities.data(),
            static_cast<std::uint32_t>(staged_loaded_identities.size()),
            &loaded_identity_count,
            staged_loaded_transforms.data(),
            static_cast<std::uint32_t>(staged_loaded_transforms.size()),
            &loaded_transform_count,
            staged_loaded_attachments.data(),
            static_cast<std::uint32_t>(staged_loaded_attachments.size()),
            &loaded_attachment_count,
            staged_loaded_bindings.data(),
            static_cast<std::uint32_t>(staged_loaded_bindings.size()),
            &loaded_binding_count);
    result.load_status = read_result.status;
    if (!read_result.Succeeded()) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SaveLoadFailed;
        result.blocked_layer = SceneEditorGizmoResourceWorkflowBlockedLayer::SaveLoad;
        *out_result = result;
        return result.status;
    }

    SceneEditorSaveLoadProofRecord staged_save_load = BuildSaveLoadProofRecord(
        document,
        write_result,
        read_result,
        loaded_identity_count,
        loaded_transform_count,
        loaded_attachment_count,
        loaded_binding_count);
    staged_save_load.file_write_status = file_write_result.status;
    staged_save_load.file_read_status = file_read_result.status;
    staged_save_load.persisted_file_byte_count = result.persisted_file_byte_count;
    staged_save_load.read_file_byte_count = result.read_file_byte_count;
    staged_save_load.wrote_file_scene_artifact = true;
    staged_save_load.read_file_scene_artifact = true;
    staged_save_load.persisted_through_file_vfs = true;
    if (!staged_save_load.preserved_runtime_record_counts) {
        result.status = SceneEditorGizmoResourceWorkflowStatus::SaveLoadFailed;
        result.blocked_layer = SceneEditorGizmoResourceWorkflowBlockedLayer::SaveLoad;
        *out_result = result;
        return result.status;
    }

    std::uint32_t persisted_index = 0U;
    while (persisted_index < write_result.state.committed_byte_count) {
        request.persistence_buffer[persisted_index] = staged_persistence[persisted_index];
        ++persisted_index;
    }

    std::uint32_t loaded_index = 0U;
    while (loaded_index < loaded_identity_count) {
        request.loaded_identity_output[loaded_index] =
            staged_loaded_identities[loaded_index];
        ++loaded_index;
    }

    loaded_index = 0U;
    while (loaded_index < loaded_transform_count) {
        request.loaded_transform_output[loaded_index] =
            staged_loaded_transforms[loaded_index];
        ++loaded_index;
    }

    loaded_index = 0U;
    while (loaded_index < loaded_attachment_count) {
        request.loaded_attachment_output[loaded_index] =
            staged_loaded_attachments[loaded_index];
        ++loaded_index;
    }

    loaded_index = 0U;
    while (loaded_index < loaded_binding_count) {
        request.loaded_binding_output[loaded_index] =
            staged_loaded_bindings[loaded_index];
        ++loaded_index;
    }

    request.gizmo_rows[0U] = staged_gizmo;
    request.resource_picker_rows[0U] = staged_picker;
    request.save_load_records[0U] = staged_save_load;
    result.loaded_identity_count = loaded_identity_count;
    result.loaded_transform_count = loaded_transform_count;
    result.loaded_attachment_count = loaded_attachment_count;
    result.loaded_binding_count = loaded_binding_count;
    result.gizmo_row_count = 1U;
    result.resource_picker_row_count = 1U;
    result.save_load_record_count = 1U;
    result.read_scene_record_stream = true;
    result.skipped_editor_sidecars_for_runtime_stream =
        staged_save_load.kept_editor_sidecars_out_of_runtime_stream;
    result.emitted_rendered_gizmo = true;
    result.emitted_resource_picker = true;
    result.committed_workflow = true;
    result.status = SceneEditorGizmoResourceWorkflowStatus::Success;
    result.blocked_layer = SceneEditorGizmoResourceWorkflowBlockedLayer::None;
    *out_result = result;
    return result.status;
}

}
