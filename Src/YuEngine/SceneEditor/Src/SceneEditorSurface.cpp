// 模块: YuEngine SceneEditor
// 文件: Src/YuEngine/SceneEditor/Src/SceneEditorSurface.cpp

#include "YuEngine/SceneEditor/SceneEditorSurface.h"

#include <array>
#include <cstdint>

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
using ResourceBrowserSurfaceSelectionState =
    yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionState;
using ResourceBrowserSurfacePreviewState =
    yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState;
using PreviewHostViewportSessionResult =
    yuengine::previewhost::PreviewHostViewportSessionResult;
using PreviewHostEditorViewportInteractionResult =
    yuengine::previewhost::PreviewHostEditorViewportInteractionResult;
using PreviewHostStatus = yuengine::previewhost::PreviewHostStatus;

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
        *out_result = result;
        return result.status;
    }

    if (request.inspector_rows.size() < result.selected_object_count) {
        result.status = SceneEditorSurfaceStatus::OutputCapacityExceeded;
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

}
