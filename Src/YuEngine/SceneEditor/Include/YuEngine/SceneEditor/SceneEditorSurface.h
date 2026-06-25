// 模块: YuEngine SceneEditor
// 文件: Src/YuEngine/SceneEditor/Include/YuEngine/SceneEditor/SceneEditorSurface.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/File/FileStatus.h"
#include "YuEngine/File/MountId.h"
#include "YuEngine/File/VirtualPath.h"
#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/PreviewHost/PreviewHost.h"
#include "YuEngine/ResourceBrowser/ResourceBrowserSurface.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldSceneAuthoringDocument.h"
#include "YuEngine/World/WorldTransformState.h"
#include "YuEngine/World/WorldSceneRecordValueStreamStatus.h"

namespace yuengine::file {
class MountTable;
}

namespace yuengine::sceneeditor {

enum class SceneEditorSurfaceStatus {
    Success,
    InvalidArgument,
    InvalidAuthoringDocument,
    OutputCapacityExceeded,
    SelectionRequired
};

enum class SceneEditorTransformCommandStatus {
    Success,
    InvalidArgument,
    InvalidAuthoringDocument,
    OutputCapacityExceeded,
    SelectionRequired,
    ObjectNotFound,
    TransformNotFound,
    UndoStackEmpty,
    RedoStackEmpty
};

enum class SceneEditorTransformCommandMode {
    Apply,
    Undo,
    Redo
};

enum class SceneEditorTransformCommandBlockedLayer {
    None,
    AuthoringDocument,
    Selection,
    TransformRecord,
    UndoRedoLedger,
    Output
};

enum class SceneEditorWorkflowStatus {
    Success,
    InvalidArgument,
    InvalidAuthoringDocument,
    OutputCapacityExceeded,
    SelectionRequired,
    BlockedResourceBrowserSelection,
    ViewportSessionFailed,
    ViewportInteractionFailed,
    TransformCommandFailed
};

enum class SceneEditorWorkflowBlockedLayer {
    None,
    AuthoringDocument,
    ResourceBrowserSelection,
    ViewportSession,
    ViewportInteraction,
    TransformCommand,
    Output
};

enum class SceneEditorGizmoResourceWorkflowStatus {
    Success,
    InvalidArgument,
    InvalidAuthoringDocument,
    OutputCapacityExceeded,
    SelectionRequired,
    BlockedResourceBrowserSelection,
    ViewportSessionFailed,
    ViewportInteractionFailed,
    GizmoUnavailable,
    ResourceBindingUnavailable,
    SaveLoadFailed
};

enum class SceneEditorGizmoResourceWorkflowBlockedLayer {
    None,
    AuthoringDocument,
    ResourceBrowserSelection,
    ViewportSession,
    ViewportInteraction,
    GizmoSidecar,
    ResourceBinding,
    SaveLoad,
    FilePersistence,
    Output
};

struct SceneEditorHierarchyRow final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};
    std::uint32_t row_index = 0U;
    std::uint32_t depth = 0U;
    std::uint32_t component_count = 0U;
    std::uint32_t resource_binding_count = 0U;
    bool has_transform = false;
    bool visible = false;
    bool active = false;
    bool selected = false;
    bool expanded = true;
};

struct SceneEditorInspectorRow final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};
    yuengine::world::WorldTransformState transform{};
    std::uint32_t component_count = 0U;
    std::uint32_t resource_binding_count = 0U;
    std::uint32_t runtime_export_field_count = 0U;
    std::uint32_t editor_only_sidecar_field_count = 0U;
    bool has_transform = false;
    bool has_component_attachments = false;
    bool has_resource_bindings = false;
    bool separated_runtime_and_editor_fields = false;
    bool selected = false;
};

struct SceneEditorSurfaceRequest final {
    const yuengine::world::WorldSceneAuthoringDocument *document = nullptr;
    std::span<SceneEditorHierarchyRow> hierarchy_rows{};
    std::span<SceneEditorInspectorRow> inspector_rows{};
    bool require_selection = false;
};

struct SceneEditorSurfaceResult final {
    SceneEditorSurfaceStatus status = SceneEditorSurfaceStatus::InvalidArgument;
    std::uint64_t scene_document_id = yuengine::world::INVALID_WORLD_SCENE_DOCUMENT_ID;
    std::uint64_t deterministic_document_hash =
        yuengine::world::INVALID_WORLD_SCENE_DOCUMENT_HASH;
    std::uint32_t hierarchy_row_count = 0U;
    std::uint32_t inspector_row_count = 0U;
    std::uint32_t selected_object_count = 0U;
    std::uint32_t folded_object_count = 0U;
    std::uint32_t component_attachment_count = 0U;
    std::uint32_t resource_binding_count = 0U;
    std::uint32_t validated_sidecar_count = 0U;
    yuengine::world::WorldSceneAuthoringDocumentStatus authoring_status =
        yuengine::world::WorldSceneAuthoringDocumentStatus::Success;
    bool consumed_authoring_document = false;
    bool consumed_editor_sidecar = false;
    bool exported_runtime_data = false;
    bool mutated_runtime_data = false;
    bool opened_native_window = false;
    bool used_preview_feedback = false;

    bool Succeeded() const {
        return status == SceneEditorSurfaceStatus::Success;
    }
};

struct SceneEditorTransformLedgerRecord final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldTransformState before_transform{};
    yuengine::world::WorldTransformState after_transform{};
    std::uint32_t command_sequence = 0U;
    bool applied = false;
    bool undone = false;
    bool redone = false;
    bool undo_available = false;
    bool redo_available = false;
};

struct SceneEditorTransformCommandRequest final {
    const yuengine::world::WorldSceneAuthoringDocument *document = nullptr;
    yuengine::world::WorldObjectId selected_world_object_id{};
    yuengine::world::WorldTransformState requested_transform{};
    const SceneEditorTransformLedgerRecord *history_record = nullptr;
    SceneEditorTransformCommandMode mode = SceneEditorTransformCommandMode::Apply;
    std::span<yuengine::world::WorldSceneObjectTransformRestoreTransformRecord>
        transform_output{};
    std::span<SceneEditorTransformLedgerRecord> ledger_output{};
};

struct SceneEditorTransformCommandResult final {
    SceneEditorTransformCommandStatus status =
        SceneEditorTransformCommandStatus::InvalidArgument;
    yuengine::world::WorldSceneAuthoringDocumentStatus authoring_status =
        yuengine::world::WorldSceneAuthoringDocumentStatus::Success;
    SceneEditorTransformCommandBlockedLayer blocked_layer =
        SceneEditorTransformCommandBlockedLayer::AuthoringDocument;
    yuengine::world::WorldObjectId selected_world_object_id{};
    yuengine::world::WorldTransformState previous_transform{};
    yuengine::world::WorldTransformState current_transform{};
    std::uint32_t transform_record_count = 0U;
    std::uint32_t ledger_record_count = 0U;
    bool consumed_authoring_document = false;
    bool consumed_selection = false;
    bool wrote_transform_output = false;
    bool emitted_undo_redo_ledger = false;
    bool undo_available = false;
    bool redo_available = false;
    bool mutated_runtime_data = false;
    bool opened_native_window = false;
    bool used_preview_feedback = false;

    bool Succeeded() const {
        return status == SceneEditorTransformCommandStatus::Success;
    }
};

struct SceneEditorWorkflowLedgerRecord final {
    SceneEditorWorkflowStatus status = SceneEditorWorkflowStatus::InvalidArgument;
    SceneEditorWorkflowBlockedLayer blocked_layer =
        SceneEditorWorkflowBlockedLayer::AuthoringDocument;
    SceneEditorTransformCommandMode transform_mode =
        SceneEditorTransformCommandMode::Apply;
    yuengine::world::WorldObjectId selected_world_object_id{};
    std::uint32_t selected_hierarchy_index = 0U;
    std::uint32_t viewport_selected_entity_index = 0U;
    std::uint32_t transform_command_sequence = 0U;
    bool consumed_resource_browser_selection = false;
    bool consumed_viewport_session = false;
    bool consumed_viewport_interaction = false;
    bool matched_hierarchy_to_viewport_selection = false;
    bool emitted_inspector_rows = false;
    bool applied_transform_command = false;
    bool replayed_undo = false;
    bool replayed_redo = false;
    bool committed_workflow = false;
};

struct SceneEditorWorkflowRequest final {
    const yuengine::world::WorldSceneAuthoringDocument *document = nullptr;
    const yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionState
        *resource_browser_selection = nullptr;
    const yuengine::previewhost::PreviewHostViewportSessionResult
        *viewport_session = nullptr;
    const yuengine::previewhost::PreviewHostEditorViewportInteractionResult
        *viewport_interaction = nullptr;
    yuengine::world::WorldTransformState requested_transform{};
    const SceneEditorTransformLedgerRecord *history_record = nullptr;
    SceneEditorTransformCommandMode transform_mode =
        SceneEditorTransformCommandMode::Apply;
    std::span<SceneEditorHierarchyRow> hierarchy_rows{};
    std::span<SceneEditorInspectorRow> inspector_rows{};
    std::span<yuengine::world::WorldSceneObjectTransformRestoreTransformRecord>
        transform_output{};
    std::span<SceneEditorTransformLedgerRecord> transform_ledger_output{};
    std::span<SceneEditorWorkflowLedgerRecord> workflow_ledger_output{};
};

struct SceneEditorWorkflowResult final {
    SceneEditorWorkflowStatus status = SceneEditorWorkflowStatus::InvalidArgument;
    SceneEditorWorkflowBlockedLayer blocked_layer =
        SceneEditorWorkflowBlockedLayer::AuthoringDocument;
    SceneEditorSurfaceStatus surface_status = SceneEditorSurfaceStatus::InvalidArgument;
    SceneEditorTransformCommandStatus transform_status =
        SceneEditorTransformCommandStatus::InvalidArgument;
    SceneEditorSurfaceResult surface{};
    SceneEditorTransformCommandResult transform{};
    yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState resource_preview_state =
        yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState::Unknown;
    yuengine::previewhost::PreviewHostStatus viewport_status =
        yuengine::previewhost::PreviewHostStatus::InvalidArgument;
    yuengine::previewhost::PreviewHostStatus viewport_interaction_status =
        yuengine::previewhost::PreviewHostStatus::InvalidArgument;
    yuengine::world::WorldObjectId selected_world_object_id{};
    std::uint32_t selected_hierarchy_index = 0U;
    std::uint32_t viewport_selected_entity_index = 0U;
    std::uint32_t workflow_ledger_count = 0U;
    bool consumed_authoring_document = false;
    bool consumed_resource_browser_selection = false;
    bool consumed_viewport_session = false;
    bool consumed_viewport_interaction = false;
    bool hierarchy_selection_matched_viewport = false;
    bool emitted_hierarchy_rows = false;
    bool emitted_inspector_rows = false;
    bool applied_transform_command = false;
    bool emitted_transform_ledger = false;
    bool committed_workflow = false;
    bool mutated_runtime_data = false;
    bool opened_native_window = false;

    bool Succeeded() const {
        return status == SceneEditorWorkflowStatus::Success;
    }
};

struct SceneEditorRenderedGizmoRow final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldTransformState transform{};
    std::uint64_t gizmo_mode = 0U;
    std::uint32_t viewport_selected_entity_index = 0U;
    bool selected = false;
    bool transform_available = false;
    bool consumed_preview_host_feedback = false;
    bool rendered_from_engine_viewport = false;
};

struct SceneEditorResourcePickerRow final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldComponentTypeId component_type_id{};
    yuengine::world::WorldComponentSlotId component_slot_id{};
    yuengine::resource::ResourceHandle current_resource{};
    yuengine::resource::ResourceTypeId expected_resource_type{};
    yuengine::resource::ResourceHandle selected_resource{};
    yuengine::resource::ResourceTypeId selected_resource_type{};
    bool current_binding_available = false;
    bool resource_browser_selection_ready = false;
    bool resource_asset_mapping_preserved = false;
    bool selected_resource_matches_binding_type = false;
};

struct SceneEditorSaveLoadProofRecord final {
    yuengine::world::WorldSceneRecordValueStreamStatus write_status =
        yuengine::world::WorldSceneRecordValueStreamStatus::InvalidWriter;
    yuengine::world::WorldSceneRecordValueStreamStatus read_status =
        yuengine::world::WorldSceneRecordValueStreamStatus::InvalidReader;
    std::uint32_t saved_identity_count = 0U;
    std::uint32_t saved_transform_count = 0U;
    std::uint32_t saved_attachment_count = 0U;
    std::uint32_t saved_binding_count = 0U;
    std::uint32_t loaded_identity_count = 0U;
    std::uint32_t loaded_transform_count = 0U;
    std::uint32_t loaded_attachment_count = 0U;
    std::uint32_t loaded_binding_count = 0U;
    std::uint32_t committed_byte_count = 0U;
    std::uint32_t persisted_file_byte_count = 0U;
    std::uint32_t read_file_byte_count = 0U;
    std::uint32_t skipped_editor_sidecar_count = 0U;
    yuengine::file::FileStatus file_write_status =
        yuengine::file::FileStatus::WriteFailure;
    yuengine::file::FileStatus file_read_status =
        yuengine::file::FileStatus::ReadFailure;
    bool wrote_scene_records = false;
    bool read_scene_records = false;
    bool wrote_file_scene_artifact = false;
    bool read_file_scene_artifact = false;
    bool persisted_through_file_vfs = false;
    bool preserved_runtime_record_counts = false;
    bool kept_editor_sidecars_out_of_runtime_stream = false;
};

struct SceneEditorGizmoResourceSaveLoadWorkflowRequest final {
    const yuengine::world::WorldSceneAuthoringDocument *document = nullptr;
    const yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionState
        *resource_browser_selection = nullptr;
    const yuengine::previewhost::PreviewHostViewportSessionResult
        *viewport_session = nullptr;
    const yuengine::previewhost::PreviewHostEditorViewportInteractionResult
        *viewport_interaction = nullptr;
    std::span<std::uint8_t> persistence_buffer{};
    std::span<SceneEditorRenderedGizmoRow> gizmo_rows{};
    std::span<SceneEditorResourcePickerRow> resource_picker_rows{};
    std::span<SceneEditorSaveLoadProofRecord> save_load_records{};
    std::span<yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord>
        loaded_identity_output{};
    std::span<yuengine::world::WorldSceneObjectTransformRestoreTransformRecord>
        loaded_transform_output{};
    std::span<yuengine::world::WorldComponentAttachmentSnapshotRecord>
        loaded_attachment_output{};
    std::span<yuengine::world::WorldComponentResourceBindingSnapshotRecord>
        loaded_binding_output{};
};

struct SceneEditorGizmoResourceSaveLoadWorkflowResult final {
    SceneEditorGizmoResourceWorkflowStatus status =
        SceneEditorGizmoResourceWorkflowStatus::InvalidArgument;
    SceneEditorGizmoResourceWorkflowBlockedLayer blocked_layer =
        SceneEditorGizmoResourceWorkflowBlockedLayer::AuthoringDocument;
    yuengine::world::WorldSceneAuthoringDocumentStatus authoring_status =
        yuengine::world::WorldSceneAuthoringDocumentStatus::Success;
    yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState resource_preview_state =
        yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState::Unknown;
    yuengine::previewhost::PreviewHostStatus viewport_status =
        yuengine::previewhost::PreviewHostStatus::InvalidArgument;
    yuengine::previewhost::PreviewHostStatus viewport_interaction_status =
        yuengine::previewhost::PreviewHostStatus::InvalidArgument;
    yuengine::world::WorldSceneRecordValueStreamStatus save_status =
        yuengine::world::WorldSceneRecordValueStreamStatus::InvalidWriter;
    yuengine::world::WorldSceneRecordValueStreamStatus load_status =
        yuengine::world::WorldSceneRecordValueStreamStatus::InvalidReader;
    yuengine::world::WorldObjectId selected_world_object_id{};
    std::uint32_t viewport_selected_entity_index = 0U;
    std::uint32_t gizmo_row_count = 0U;
    std::uint32_t resource_picker_row_count = 0U;
    std::uint32_t save_load_record_count = 0U;
    std::uint32_t loaded_identity_count = 0U;
    std::uint32_t loaded_transform_count = 0U;
    std::uint32_t loaded_attachment_count = 0U;
    std::uint32_t loaded_binding_count = 0U;
    bool consumed_authoring_document = false;
    bool consumed_resource_browser_selection = false;
    bool consumed_viewport_session = false;
    bool consumed_viewport_interaction = false;
    bool emitted_rendered_gizmo = false;
    bool emitted_resource_picker = false;
    bool wrote_scene_record_stream = false;
    bool read_scene_record_stream = false;
    bool skipped_editor_sidecars_for_runtime_stream = false;
    bool committed_workflow = false;
    bool mutated_runtime_data = false;
    bool opened_native_window = false;

    bool Succeeded() const {
        return status == SceneEditorGizmoResourceWorkflowStatus::Success;
    }
};

struct SceneEditorGizmoResourceFilePersistenceWorkflowRequest final {
    const yuengine::world::WorldSceneAuthoringDocument *document = nullptr;
    const yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionState
        *resource_browser_selection = nullptr;
    const yuengine::previewhost::PreviewHostViewportSessionResult
        *viewport_session = nullptr;
    const yuengine::previewhost::PreviewHostEditorViewportInteractionResult
        *viewport_interaction = nullptr;
    yuengine::file::MountTable *scene_persistence_mount_table = nullptr;
    yuengine::file::MountId scene_persistence_mount{};
    yuengine::file::VirtualPath scene_persistence_path{};
    std::span<std::uint8_t> persistence_buffer{};
    std::span<SceneEditorRenderedGizmoRow> gizmo_rows{};
    std::span<SceneEditorResourcePickerRow> resource_picker_rows{};
    std::span<SceneEditorSaveLoadProofRecord> save_load_records{};
    std::span<yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord>
        loaded_identity_output{};
    std::span<yuengine::world::WorldSceneObjectTransformRestoreTransformRecord>
        loaded_transform_output{};
    std::span<yuengine::world::WorldComponentAttachmentSnapshotRecord>
        loaded_attachment_output{};
    std::span<yuengine::world::WorldComponentResourceBindingSnapshotRecord>
        loaded_binding_output{};
};

struct SceneEditorGizmoResourceFilePersistenceWorkflowResult final {
    SceneEditorGizmoResourceWorkflowStatus status =
        SceneEditorGizmoResourceWorkflowStatus::InvalidArgument;
    SceneEditorGizmoResourceWorkflowBlockedLayer blocked_layer =
        SceneEditorGizmoResourceWorkflowBlockedLayer::AuthoringDocument;
    yuengine::world::WorldSceneAuthoringDocumentStatus authoring_status =
        yuengine::world::WorldSceneAuthoringDocumentStatus::Success;
    yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState resource_preview_state =
        yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState::Unknown;
    yuengine::previewhost::PreviewHostStatus viewport_status =
        yuengine::previewhost::PreviewHostStatus::InvalidArgument;
    yuengine::previewhost::PreviewHostStatus viewport_interaction_status =
        yuengine::previewhost::PreviewHostStatus::InvalidArgument;
    yuengine::world::WorldSceneRecordValueStreamStatus save_status =
        yuengine::world::WorldSceneRecordValueStreamStatus::InvalidWriter;
    yuengine::world::WorldSceneRecordValueStreamStatus load_status =
        yuengine::world::WorldSceneRecordValueStreamStatus::InvalidReader;
    yuengine::file::FileStatus file_write_status =
        yuengine::file::FileStatus::WriteFailure;
    yuengine::file::FileStatus file_read_status =
        yuengine::file::FileStatus::ReadFailure;
    yuengine::world::WorldObjectId selected_world_object_id{};
    std::uint32_t viewport_selected_entity_index = 0U;
    std::uint32_t gizmo_row_count = 0U;
    std::uint32_t resource_picker_row_count = 0U;
    std::uint32_t save_load_record_count = 0U;
    std::uint32_t loaded_identity_count = 0U;
    std::uint32_t loaded_transform_count = 0U;
    std::uint32_t loaded_attachment_count = 0U;
    std::uint32_t loaded_binding_count = 0U;
    std::uint32_t persisted_file_byte_count = 0U;
    std::uint32_t read_file_byte_count = 0U;
    bool consumed_authoring_document = false;
    bool consumed_resource_browser_selection = false;
    bool consumed_viewport_session = false;
    bool consumed_viewport_interaction = false;
    bool emitted_rendered_gizmo = false;
    bool emitted_resource_picker = false;
    bool wrote_scene_record_stream = false;
    bool wrote_file_scene_artifact = false;
    bool read_file_scene_artifact = false;
    bool read_scene_record_stream = false;
    bool skipped_editor_sidecars_for_runtime_stream = false;
    bool committed_workflow = false;
    bool mutated_runtime_data = false;
    bool opened_native_window = false;

    bool Succeeded() const {
        return status == SceneEditorGizmoResourceWorkflowStatus::Success;
    }
};

SceneEditorSurfaceStatus BuildSceneEditorNativeSurface(
    const SceneEditorSurfaceRequest &request,
    SceneEditorSurfaceResult *out_result);

SceneEditorTransformCommandStatus ApplySceneEditorTransformCommand(
    const SceneEditorTransformCommandRequest &request,
    SceneEditorTransformCommandResult *out_result);

SceneEditorWorkflowStatus BuildSceneEditorUsableWorkflowSurface(
    const SceneEditorWorkflowRequest &request,
    SceneEditorWorkflowResult *out_result);

SceneEditorGizmoResourceWorkflowStatus BuildSceneEditorGizmoResourceSaveLoadWorkflow(
    const SceneEditorGizmoResourceSaveLoadWorkflowRequest &request,
    SceneEditorGizmoResourceSaveLoadWorkflowResult *out_result);

SceneEditorGizmoResourceWorkflowStatus BuildSceneEditorGizmoResourceFilePersistenceWorkflow(
    const SceneEditorGizmoResourceFilePersistenceWorkflowRequest &request,
    SceneEditorGizmoResourceFilePersistenceWorkflowResult *out_result);

}
