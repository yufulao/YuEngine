// Module: YuEngine EditorHost
// File: Src/YuEngine/EditorHost/Src/EditorHostSession.cpp

#include "YuEngine/EditorHost/EditorHostSession.h"

#include <array>

namespace yuengine::editorhost {
namespace {
using AnimationEditorTimelineWorkflowResult =
    yuengine::animationeditor::AnimationEditorTimelineWorkflowResult;
using AuthoredEditorPackageRunResult =
    yuengine::editorpackagerun::AuthoredEditorPackageRunResult;
using AuthoredEditorPackageRunStatus =
    yuengine::editorpackagerun::AuthoredEditorPackageRunStatus;
using PreviewHostStatus = yuengine::previewhost::PreviewHostStatus;
using PreviewHostViewportSessionResult =
    yuengine::previewhost::PreviewHostViewportSessionResult;
using ResourceBrowserDepthWorkflowResult =
    yuengine::resourcebrowser::ResourceBrowserDepthWorkflowResult;
using SceneEditorWorkflowResult = yuengine::sceneeditor::SceneEditorWorkflowResult;
using SceneEditorGizmoResourceSaveLoadWorkflowResult =
    yuengine::sceneeditor::SceneEditorGizmoResourceSaveLoadWorkflowResult;
using AnimationEditorStateEventPlaybackWorkflowResult =
    yuengine::animationeditor::AnimationEditorStateEventPlaybackWorkflowResult;
using UiEditorDesignInspectorWorkflowResult =
    yuengine::uieditor::UiEditorDesignInspectorWorkflowResult;
using UiEditorRuntimePreviewWorkflowResult =
    yuengine::uieditor::UiEditorRuntimePreviewWorkflowResult;

bool IsSpanStorageValid(std::span<EditorHostPanelStateRow> rows) {
    if (rows.empty()) {
        return true;
    }

    return rows.data() != nullptr;
}

bool IsSpanStorageValid(std::span<EditorHostPersistedPanelRecord> rows) {
    if (rows.empty()) {
        return true;
    }

    return rows.data() != nullptr;
}

bool IsSpanStorageValid(std::span<const EditorHostPersistedPanelRecord> rows) {
    if (rows.empty()) {
        return true;
    }

    return rows.data() != nullptr;
}

bool IsSpanStorageValid(std::span<EditorHostApplicationIntegrationRow> rows) {
    if (rows.empty()) {
        return true;
    }

    return rows.data() != nullptr;
}

bool IsDescriptorValid(const EditorHostSessionDescriptor &descriptor) {
    return descriptor.session_id != INVALID_EDITOR_HOST_SESSION_ID &&
        descriptor.viewport_width > 0U &&
        descriptor.viewport_height > 0U;
}

bool SceneWorkflowReady(const SceneEditorWorkflowResult *workflow) {
    if (workflow == nullptr) {
        return false;
    }

    return workflow->Succeeded() &&
        workflow->committed_workflow &&
        workflow->emitted_hierarchy_rows &&
        workflow->emitted_inspector_rows &&
        !workflow->mutated_runtime_data &&
        !workflow->opened_native_window;
}

bool AnimationWorkflowReady(const AnimationEditorTimelineWorkflowResult *workflow) {
    if (workflow == nullptr) {
        return false;
    }

    return workflow->Succeeded() &&
        workflow->built_timeline_rows &&
        workflow->emitted_preview_feedback &&
        !workflow->mutated_runtime_data &&
        !workflow->opened_native_window;
}

bool UiWorkflowReady(const UiEditorDesignInspectorWorkflowResult *workflow) {
    if (workflow == nullptr) {
        return false;
    }

    return workflow->Succeeded() &&
        workflow->built_design_surface &&
        workflow->emitted_hierarchy_rows &&
        workflow->emitted_inspector_fields &&
        !workflow->mutated_runtime_data &&
        !workflow->opened_native_window &&
        !workflow->used_forbidden_preview_path;
}

bool PackageRunReady(const AuthoredEditorPackageRunResult *workflow) {
    if (workflow == nullptr) {
        return false;
    }

    return workflow->Succeeded() &&
        workflow->status == AuthoredEditorPackageRunStatus::Success &&
        workflow->packaged_runtime_entrypoint_executed &&
        workflow->runtime_app_frame_loop_success &&
        !workflow->wrote_fake_artifact &&
        !workflow->opened_native_window &&
        !workflow->used_forbidden_preview_path;
}

bool ResourceBrowserIntegrationReady(const ResourceBrowserDepthWorkflowResult *workflow) {
    if (workflow == nullptr) {
        return false;
    }

    return workflow->Succeeded() &&
        workflow->emitted_catalog_rows &&
        workflow->emitted_importer_rows &&
        workflow->emitted_asset_gap_rows &&
        workflow->emitted_selection_ledger &&
        !workflow->mutated_runtime_state &&
        !workflow->opened_native_window;
}

bool PreviewHostIntegrationReady(const PreviewHostViewportSessionResult *viewport) {
    if (viewport == nullptr) {
        return false;
    }

    return viewport->status == PreviewHostStatus::Success &&
        viewport->built_frame &&
        viewport->consumed_viewport_controls &&
        viewport->consumed_resource_browser_selection &&
        viewport->resource_browser_preview_eligible &&
        viewport->resource_asset_mapping_preserved;
}

bool SceneIntegrationReady(
    const SceneEditorGizmoResourceSaveLoadWorkflowResult *workflow) {
    if (workflow == nullptr) {
        return false;
    }

    return workflow->Succeeded() &&
        workflow->committed_workflow &&
        workflow->emitted_rendered_gizmo &&
        workflow->emitted_resource_picker &&
        workflow->wrote_scene_record_stream &&
        workflow->read_scene_record_stream &&
        workflow->skipped_editor_sidecars_for_runtime_stream &&
        !workflow->mutated_runtime_data &&
        !workflow->opened_native_window;
}

bool AnimationIntegrationReady(
    const AnimationEditorStateEventPlaybackWorkflowResult *workflow) {
    if (workflow == nullptr) {
        return false;
    }

    return workflow->Succeeded() &&
        workflow->built_visible_playback_feedback &&
        workflow->consumed_timeline_workflow &&
        workflow->consumed_preview_host_feedback &&
        workflow->emitted_preview_feedback &&
        !workflow->mutated_runtime_data &&
        !workflow->opened_native_window &&
        !workflow->used_gameplay_fsm;
}

bool UiIntegrationReady(const UiEditorRuntimePreviewWorkflowResult *workflow) {
    if (workflow == nullptr) {
        return false;
    }

    return workflow->Succeeded() &&
        workflow->built_design_surface &&
        workflow->built_engine_runtime_preview &&
        workflow->emitted_runtime_preview_row &&
        workflow->consumed_preview_host_feedback &&
        !workflow->mutated_runtime_data &&
        !workflow->opened_native_window &&
        !workflow->used_forbidden_preview_path;
}

std::uint32_t BoolCount(bool value) {
    if (value) {
        return 1U;
    }

    return 0U;
}

EditorHostPanelDock DockForPanel(EditorHostPanelKind kind) {
    if (kind == EditorHostPanelKind::SceneEditor) {
        return EditorHostPanelDock::Center;
    }

    if (kind == EditorHostPanelKind::AnimationEditor) {
        return EditorHostPanelDock::Bottom;
    }

    if (kind == EditorHostPanelKind::UiEditor) {
        return EditorHostPanelDock::Right;
    }

    if (kind == EditorHostPanelKind::PackageRun) {
        return EditorHostPanelDock::Bottom;
    }

    return EditorHostPanelDock::Left;
}

EditorHostPanelStateRow BasePanelRow(
    const EditorHostSessionDescriptor &descriptor,
    EditorHostPanelKind kind,
    std::uint32_t order) {
    EditorHostPanelStateRow row{};
    row.session_id = descriptor.session_id;
    row.layout_revision = descriptor.layout_revision;
    row.kind = kind;
    row.dock = DockForPanel(kind);
    row.order = order;
    row.navigation_index = order;
    row.visible = true;
    row.focused = descriptor.focused_panel == kind;
    row.persistable = true;
    row.consumed_editor_workflow = true;
    return row;
}

EditorHostPanelStateRow BuildScenePanelRow(
    const EditorHostSessionDescriptor &descriptor,
    const SceneEditorWorkflowResult &workflow,
    std::uint32_t order) {
    EditorHostPanelStateRow row =
        BasePanelRow(descriptor, EditorHostPanelKind::SceneEditor, order);
    row.content_row_count =
        static_cast<std::uint32_t>(workflow.surface.hierarchy_row_count);
    row.selected_item_count =
        BoolCount(workflow.selected_world_object_id.IsValid());
    row.ledger_record_count =
        static_cast<std::uint32_t>(workflow.workflow_ledger_count);
    row.preview_feedback_from_preview_host = workflow.consumed_viewport_session &&
        workflow.consumed_viewport_interaction &&
        workflow.hierarchy_selection_matched_viewport;
    row.dirty = workflow.applied_transform_command;
    return row;
}

EditorHostPanelStateRow BuildAnimationPanelRow(
    const EditorHostSessionDescriptor &descriptor,
    const AnimationEditorTimelineWorkflowResult &workflow,
    std::uint32_t order) {
    EditorHostPanelStateRow row =
        BasePanelRow(descriptor, EditorHostPanelKind::AnimationEditor, order);
    row.content_row_count =
        static_cast<std::uint32_t>(
            workflow.track_row_count + workflow.keyframe_marker_count);
    row.selected_item_count =
        static_cast<std::uint32_t>(workflow.selection_feedback_count);
    row.preview_feedback_from_preview_host =
        workflow.consumed_preview_host_feedback &&
        workflow.emitted_preview_feedback;
    row.dirty = workflow.updated_sample_time;
    return row;
}

EditorHostPanelStateRow BuildUiPanelRow(
    const EditorHostSessionDescriptor &descriptor,
    const UiEditorDesignInspectorWorkflowResult &workflow,
    std::uint32_t order) {
    EditorHostPanelStateRow row =
        BasePanelRow(descriptor, EditorHostPanelKind::UiEditor, order);
    row.content_row_count =
        static_cast<std::uint32_t>(
            workflow.design_surface_row_count + workflow.inspector_field_count);
    row.selected_item_count = BoolCount(workflow.selected_node_id.IsValid());
    row.ledger_record_count =
        static_cast<std::uint32_t>(workflow.command_ledger_count);
    row.preview_feedback_from_preview_host =
        workflow.consumed_preview_host_feedback &&
        workflow.preview_feedback_count > 0U;
    row.dirty = workflow.command_applied;
    return row;
}

EditorHostPanelStateRow BuildPackageRunPanelRow(
    const EditorHostSessionDescriptor &descriptor,
    const AuthoredEditorPackageRunResult &workflow,
    std::uint32_t order) {
    EditorHostPanelStateRow row =
        BasePanelRow(descriptor, EditorHostPanelKind::PackageRun, order);
    row.content_row_count =
        static_cast<std::uint32_t>(workflow.package_load_plan_record_count);
    row.ledger_record_count = static_cast<std::uint32_t>(
        workflow.cooked_runtime_file_count + workflow.cooked_source_file_count);
    row.preview_feedback_from_preview_host = false;
    row.package_run_ready = true;
    return row;
}

EditorHostPersistedPanelRecord BuildPersistedRecord(
    const EditorHostPanelStateRow &row) {
    EditorHostPersistedPanelRecord record{};
    record.session_id = row.session_id;
    record.layout_revision = row.layout_revision;
    record.kind = row.kind;
    record.dock = row.dock;
    record.order = row.order;
    record.content_row_count = row.content_row_count;
    record.selected_item_count = row.selected_item_count;
    record.ledger_record_count = row.ledger_record_count;
    record.navigation_index = row.navigation_index;
    record.visible = row.visible;
    record.focused = row.focused;
    record.dirty = row.dirty;
    record.shell_state_only = true;
    record.owns_runtime_truth = false;
    record.forged_preview_output = false;
    record.requires_runtime_refresh = true;
    return record;
}

EditorHostPanelStateRow BuildRestoredPanelRow(
    const EditorHostSessionDescriptor &descriptor,
    const EditorHostPersistedPanelRecord &record) {
    EditorHostPanelStateRow row{};
    row.session_id = descriptor.session_id;
    row.layout_revision = descriptor.layout_revision;
    row.kind = record.kind;
    row.dock = record.dock;
    row.order = record.order;
    row.content_row_count = record.content_row_count;
    row.selected_item_count = record.selected_item_count;
    row.ledger_record_count = record.ledger_record_count;
    row.navigation_index = record.navigation_index;
    row.visible = record.visible;
    row.focused = record.focused;
    row.dirty = record.dirty;
    row.persistable = true;
    row.consumed_editor_workflow = false;
    row.preview_feedback_from_preview_host = false;
    row.package_run_ready = record.kind == EditorHostPanelKind::PackageRun;
    row.owns_runtime_truth = false;
    row.forged_preview_output = false;
    row.requires_runtime_refresh = true;
    return row;
}

bool IsPersistedRecordValid(
    const EditorHostSessionDescriptor &descriptor,
    const EditorHostPersistedPanelRecord &record) {
    return record.session_id == descriptor.session_id &&
        record.kind != EditorHostPanelKind::Unknown &&
        record.shell_state_only &&
        !record.owns_runtime_truth &&
        !record.forged_preview_output;
}

EditorHostApplicationIntegrationRow BaseIntegrationRow(
    const EditorHostSessionDescriptor &descriptor,
    EditorHostApplicationLifecyclePhase phase,
    EditorHostApplicationIntegrationKind kind,
    std::uint32_t order) {
    EditorHostApplicationIntegrationRow row{};
    row.session_id = descriptor.session_id;
    row.phase = phase;
    row.kind = kind;
    row.order = order;
    row.navigation_index = order;
    row.visible = true;
    row.consumed_integration = true;
    row.shell_state_only = true;
    return row;
}

EditorHostApplicationIntegrationRow BuildResourceBrowserIntegrationRow(
    const EditorHostSessionDescriptor &descriptor,
    EditorHostApplicationLifecyclePhase phase,
    const ResourceBrowserDepthWorkflowResult &workflow) {
    EditorHostApplicationIntegrationRow row = BaseIntegrationRow(
        descriptor,
        phase,
        EditorHostApplicationIntegrationKind::ResourceBrowser,
        0U);
    row.content_row_count =
        workflow.catalog_row_count + workflow.importer_row_count +
        workflow.asset_gap_row_count;
    row.selected_item_count = BoolCount(workflow.selection_committed);
    row.ledger_record_count = workflow.selection_ledger_count;
    row.runtime_truth_refresh_available = workflow.preview_request_ready_count > 0U;
    return row;
}

EditorHostApplicationIntegrationRow BuildPreviewHostIntegrationRow(
    const EditorHostSessionDescriptor &descriptor,
    EditorHostApplicationLifecyclePhase phase,
    const PreviewHostViewportSessionResult &viewport) {
    EditorHostApplicationIntegrationRow row = BaseIntegrationRow(
        descriptor,
        phase,
        EditorHostApplicationIntegrationKind::PreviewHost,
        1U);
    row.content_row_count = viewport.frame.submitted_entity_count;
    row.selected_item_count = BoolCount(viewport.selected_entity_available);
    row.ledger_record_count = static_cast<std::uint32_t>(
        viewport.frame.hit_record_count +
        viewport.frame.selection_record_count +
        viewport.frame.transform_feedback_count);
    row.preview_frame_id = viewport.frame.frame.frame_id;
    row.runtime_truth_refresh_available = viewport.built_frame;
    return row;
}

EditorHostApplicationIntegrationRow BuildSceneIntegrationRow(
    const EditorHostSessionDescriptor &descriptor,
    EditorHostApplicationLifecyclePhase phase,
    const SceneEditorGizmoResourceSaveLoadWorkflowResult &workflow) {
    EditorHostApplicationIntegrationRow row = BaseIntegrationRow(
        descriptor,
        phase,
        EditorHostApplicationIntegrationKind::SceneEditor,
        2U);
    row.content_row_count = workflow.gizmo_row_count +
        workflow.resource_picker_row_count +
        workflow.save_load_record_count;
    row.selected_item_count = BoolCount(workflow.selected_world_object_id.IsValid());
    row.ledger_record_count = workflow.save_load_record_count;
    row.runtime_truth_refresh_available =
        workflow.wrote_scene_record_stream &&
        workflow.read_scene_record_stream;
    return row;
}

EditorHostApplicationIntegrationRow BuildAnimationIntegrationRow(
    const EditorHostSessionDescriptor &descriptor,
    EditorHostApplicationLifecyclePhase phase,
    const AnimationEditorStateEventPlaybackWorkflowResult &workflow) {
    EditorHostApplicationIntegrationRow row = BaseIntegrationRow(
        descriptor,
        phase,
        EditorHostApplicationIntegrationKind::AnimationEditor,
        3U);
    row.content_row_count = static_cast<std::uint32_t>(
        workflow.state_row_count +
        workflow.event_row_count +
        workflow.track_row_count +
        workflow.keyframe_marker_count);
    row.selected_item_count =
        static_cast<std::uint32_t>(workflow.selection_feedback_count);
    row.ledger_record_count =
        static_cast<std::uint32_t>(workflow.emitted_event_count);
    row.runtime_truth_refresh_available = workflow.built_visible_playback_feedback;
    return row;
}

EditorHostApplicationIntegrationRow BuildUiIntegrationRow(
    const EditorHostSessionDescriptor &descriptor,
    EditorHostApplicationLifecyclePhase phase,
    const UiEditorRuntimePreviewWorkflowResult &workflow) {
    EditorHostApplicationIntegrationRow row = BaseIntegrationRow(
        descriptor,
        phase,
        EditorHostApplicationIntegrationKind::UiEditor,
        4U);
    row.content_row_count = static_cast<std::uint32_t>(
        workflow.design_surface_row_count +
        workflow.inspector_field_count +
        workflow.runtime_preview_row_count);
    row.selected_item_count = BoolCount(workflow.selected_node_id.IsValid());
    row.ledger_record_count = static_cast<std::uint32_t>(
        workflow.command_ledger_count +
        workflow.style_template_state_ledger_count);
    row.preview_frame_id = workflow.design_workflow.surface.preview_status ==
            yuengine::previewhost::PreviewHostStatus::Success
        ? static_cast<std::uint32_t>(workflow.preview_feedback_count)
        : 0U;
    row.runtime_truth_refresh_available = workflow.built_engine_runtime_preview;
    return row;
}

EditorHostApplicationIntegrationRow BuildRestoredIntegrationRow(
    const EditorHostSessionDescriptor &descriptor,
    EditorHostApplicationIntegrationKind kind,
    std::uint32_t order) {
    EditorHostApplicationIntegrationRow row = BaseIntegrationRow(
        descriptor,
        EditorHostApplicationLifecyclePhase::Restore,
        kind,
        order);
    row.consumed_integration = false;
    row.runtime_truth_refresh_available = false;
    row.requires_runtime_refresh = true;
    return row;
}

bool LifecycleRequiresRuntimeIntegrations(EditorHostApplicationLifecyclePhase phase) {
    if (phase == EditorHostApplicationLifecyclePhase::Open) {
        return true;
    }

    return phase == EditorHostApplicationLifecyclePhase::RefreshRuntimeTruth;
}

bool IsLifecyclePhaseValid(EditorHostApplicationLifecyclePhase phase) {
    if (phase == EditorHostApplicationLifecyclePhase::Open) {
        return true;
    }

    if (phase == EditorHostApplicationLifecyclePhase::Restore) {
        return true;
    }

    if (phase == EditorHostApplicationLifecyclePhase::RefreshRuntimeTruth) {
        return true;
    }

    return phase == EditorHostApplicationLifecyclePhase::Close;
}

}

namespace {
constexpr std::size_t EDITOR_HOST_APPLICATION_INTEGRATION_COUNT = 5U;

EditorHostSessionStatus ValidateRuntimeIntegrations(
    const EditorHostApplicationLifecycleRequest &request,
    EditorHostApplicationLifecycleResult *result) {
    if (!ResourceBrowserIntegrationReady(request.resource_browser_workflow)) {
        if (result != nullptr) {
            result->status = EditorHostSessionStatus::MissingResourceBrowserIntegration;
            result->blocked_layer =
                EditorHostSessionBlockedLayer::ResourceBrowserIntegration;
        }

        return EditorHostSessionStatus::MissingResourceBrowserIntegration;
    }

    if (!PreviewHostIntegrationReady(request.preview_host_viewport)) {
        if (result != nullptr) {
            result->status = EditorHostSessionStatus::MissingPreviewHostIntegration;
            result->blocked_layer =
                EditorHostSessionBlockedLayer::PreviewHostIntegration;
        }

        return EditorHostSessionStatus::MissingPreviewHostIntegration;
    }

    if (!SceneIntegrationReady(request.scene_workflow)) {
        if (result != nullptr) {
            result->status = EditorHostSessionStatus::MissingSceneIntegration;
            result->blocked_layer = EditorHostSessionBlockedLayer::SceneEditorIntegration;
        }

        return EditorHostSessionStatus::MissingSceneIntegration;
    }

    if (!AnimationIntegrationReady(request.animation_workflow)) {
        if (result != nullptr) {
            result->status = EditorHostSessionStatus::MissingAnimationIntegration;
            result->blocked_layer =
                EditorHostSessionBlockedLayer::AnimationEditorIntegration;
        }

        return EditorHostSessionStatus::MissingAnimationIntegration;
    }

    if (!UiIntegrationReady(request.ui_workflow)) {
        if (result != nullptr) {
            result->status = EditorHostSessionStatus::MissingUiIntegration;
            result->blocked_layer = EditorHostSessionBlockedLayer::UiEditorIntegration;
        }

        return EditorHostSessionStatus::MissingUiIntegration;
    }

    return EditorHostSessionStatus::Success;
}

void StageRuntimeIntegrationRows(
    const EditorHostApplicationLifecycleRequest &request,
    std::span<EditorHostApplicationIntegrationRow> staged_rows) {
    staged_rows[0U] = BuildResourceBrowserIntegrationRow(
        request.shell_request.descriptor,
        request.phase,
        *request.resource_browser_workflow);
    staged_rows[1U] = BuildPreviewHostIntegrationRow(
        request.shell_request.descriptor,
        request.phase,
        *request.preview_host_viewport);
    staged_rows[2U] = BuildSceneIntegrationRow(
        request.shell_request.descriptor,
        request.phase,
        *request.scene_workflow);
    staged_rows[3U] = BuildAnimationIntegrationRow(
        request.shell_request.descriptor,
        request.phase,
        *request.animation_workflow);
    staged_rows[4U] = BuildUiIntegrationRow(
        request.shell_request.descriptor,
        request.phase,
        *request.ui_workflow);
}

void StageRestoredIntegrationRows(
    const EditorHostSessionDescriptor &descriptor,
    std::span<EditorHostApplicationIntegrationRow> staged_rows) {
    staged_rows[0U] = BuildRestoredIntegrationRow(
        descriptor,
        EditorHostApplicationIntegrationKind::ResourceBrowser,
        0U);
    staged_rows[1U] = BuildRestoredIntegrationRow(
        descriptor,
        EditorHostApplicationIntegrationKind::PreviewHost,
        1U);
    staged_rows[2U] = BuildRestoredIntegrationRow(
        descriptor,
        EditorHostApplicationIntegrationKind::SceneEditor,
        2U);
    staged_rows[3U] = BuildRestoredIntegrationRow(
        descriptor,
        EditorHostApplicationIntegrationKind::AnimationEditor,
        3U);
    staged_rows[4U] = BuildRestoredIntegrationRow(
        descriptor,
        EditorHostApplicationIntegrationKind::UiEditor,
        4U);
}

void CommitIntegrationRows(
    std::span<const EditorHostApplicationIntegrationRow> staged_rows,
    std::span<EditorHostApplicationIntegrationRow> output_rows) {
    std::size_t index = 0U;
    while (index < staged_rows.size()) {
        output_rows[index] = staged_rows[index];
        ++index;
    }
}

}

EditorHostSessionStatus BuildEditorHostSessionShell(
    const EditorHostSessionShellRequest &request,
    EditorHostSessionShellResult *out_result) {
    if (out_result == nullptr) {
        return EditorHostSessionStatus::InvalidArgument;
    }

    EditorHostSessionShellResult result{};
    result.session_id = request.descriptor.session_id;
    result.layout_revision = request.descriptor.layout_revision;
    result.native_shell_requested = request.descriptor.native_shell_requested;
    if (!IsDescriptorValid(request.descriptor)) {
        *out_result = result;
        return result.status;
    }

    if (!IsSpanStorageValid(request.panel_output) ||
        !IsSpanStorageValid(request.persisted_panel_output)) {
        *out_result = result;
        return result.status;
    }

    if (!SceneWorkflowReady(request.scene_workflow)) {
        result.status = EditorHostSessionStatus::MissingSceneWorkflow;
        result.blocked_layer = EditorHostSessionBlockedLayer::SceneEditorWorkflow;
        *out_result = result;
        return result.status;
    }

    result.consumed_scene_editor_workflow = true;
    if (!AnimationWorkflowReady(request.animation_workflow)) {
        result.status = EditorHostSessionStatus::MissingAnimationWorkflow;
        result.blocked_layer = EditorHostSessionBlockedLayer::AnimationEditorWorkflow;
        *out_result = result;
        return result.status;
    }

    result.consumed_animation_editor_workflow = true;
    if (!UiWorkflowReady(request.ui_workflow)) {
        result.status = EditorHostSessionStatus::MissingUiWorkflow;
        result.blocked_layer = EditorHostSessionBlockedLayer::UiEditorWorkflow;
        *out_result = result;
        return result.status;
    }

    result.consumed_ui_editor_workflow = true;
    const bool package_run_ready = PackageRunReady(request.package_run_workflow);
    if (request.require_package_run && !package_run_ready) {
        result.status = EditorHostSessionStatus::MissingPackageRunWorkflow;
        result.blocked_layer = EditorHostSessionBlockedLayer::PackageRunWorkflow;
        *out_result = result;
        return result.status;
    }

    result.consumed_package_run_workflow = package_run_ready;
    const std::size_t panel_count = package_run_ready ? 4U : 3U;
    if (request.panel_output.size() < panel_count) {
        result.panel_count = panel_count;
        result.status = EditorHostSessionStatus::PanelOutputCapacityExceeded;
        result.blocked_layer = EditorHostSessionBlockedLayer::PanelOutput;
        *out_result = result;
        return result.status;
    }

    if (request.persist_shell_state &&
        request.persisted_panel_output.size() < panel_count) {
        result.panel_count = panel_count;
        result.persisted_panel_count = panel_count;
        result.status = EditorHostSessionStatus::PersistenceOutputCapacityExceeded;
        result.blocked_layer = EditorHostSessionBlockedLayer::PersistenceStore;
        *out_result = result;
        return result.status;
    }

    std::array<EditorHostPanelStateRow, MAX_EDITOR_HOST_PANEL_COUNT> staged_panels{};
    staged_panels[0U] =
        BuildScenePanelRow(request.descriptor, *request.scene_workflow, 0U);
    staged_panels[1U] =
        BuildAnimationPanelRow(request.descriptor, *request.animation_workflow, 1U);
    staged_panels[2U] = BuildUiPanelRow(request.descriptor, *request.ui_workflow, 2U);
    if (package_run_ready) {
        staged_panels[3U] =
            BuildPackageRunPanelRow(request.descriptor, *request.package_run_workflow, 3U);
    }

    std::size_t index = 0U;
    while (index < panel_count) {
        request.panel_output[index] = staged_panels[index];
        ++index;
    }

    if (request.persist_shell_state) {
        index = 0U;
        while (index < panel_count) {
            request.persisted_panel_output[index] =
                BuildPersistedRecord(staged_panels[index]);
            ++index;
        }

        result.persisted_panel_count = panel_count;
        result.persisted_shell_state = true;
    }

    result.panel_count = panel_count;
    result.built_native_shell_state = true;
    result.status = EditorHostSessionStatus::Success;
    result.blocked_layer = EditorHostSessionBlockedLayer::None;
    *out_result = result;
    return result.status;
}

EditorHostSessionStatus RestoreEditorHostSessionShell(
    const EditorHostSessionRestoreRequest &request,
    EditorHostSessionRestoreResult *out_result) {
    if (out_result == nullptr) {
        return EditorHostSessionStatus::InvalidArgument;
    }

    EditorHostSessionRestoreResult result{};
    result.session_id = request.descriptor.session_id;
    result.layout_revision = request.descriptor.layout_revision;
    if (!IsDescriptorValid(request.descriptor) ||
        !IsSpanStorageValid(request.persisted_panels) ||
        !IsSpanStorageValid(request.panel_output) ||
        request.persisted_panels.empty()) {
        *out_result = result;
        return result.status;
    }

    if (request.persisted_panels.size() > MAX_EDITOR_HOST_PANEL_COUNT) {
        result.status = EditorHostSessionStatus::InvalidPersistedSession;
        result.blocked_layer = EditorHostSessionBlockedLayer::PersistenceStore;
        *out_result = result;
        return result.status;
    }

    if (request.panel_output.size() < request.persisted_panels.size()) {
        result.restored_panel_count = request.persisted_panels.size();
        result.status = EditorHostSessionStatus::PanelOutputCapacityExceeded;
        result.blocked_layer = EditorHostSessionBlockedLayer::PanelOutput;
        *out_result = result;
        return result.status;
    }

    std::array<EditorHostPanelStateRow, MAX_EDITOR_HOST_PANEL_COUNT> staged_panels{};
    std::size_t index = 0U;
    while (index < request.persisted_panels.size()) {
        if (!IsPersistedRecordValid(request.descriptor, request.persisted_panels[index])) {
            result.status = EditorHostSessionStatus::InvalidPersistedSession;
            result.blocked_layer = EditorHostSessionBlockedLayer::PersistenceStore;
            *out_result = result;
            return result.status;
        }

        staged_panels[index] =
            BuildRestoredPanelRow(request.descriptor, request.persisted_panels[index]);
        ++index;
    }

    index = 0U;
    while (index < request.persisted_panels.size()) {
        request.panel_output[index] = staged_panels[index];
        ++index;
    }

    result.restored_panel_count = request.persisted_panels.size();
    result.restored_shell_state = true;
    result.requires_runtime_refresh = true;
    result.status = EditorHostSessionStatus::Success;
    result.blocked_layer = EditorHostSessionBlockedLayer::None;
    *out_result = result;
    return result.status;
}

EditorHostSessionStatus BuildEditorHostApplicationLifecycle(
    const EditorHostApplicationLifecycleRequest &request,
    EditorHostApplicationLifecycleResult *out_result) {
    if (out_result == nullptr) {
        return EditorHostSessionStatus::InvalidArgument;
    }

    EditorHostApplicationLifecycleResult result{};
    result.phase = request.phase;
    result.session_id = request.shell_request.descriptor.session_id;
    if (!IsLifecyclePhaseValid(request.phase) ||
        !IsDescriptorValid(request.shell_request.descriptor) ||
        !IsSpanStorageValid(request.integration_output)) {
        *out_result = result;
        return result.status;
    }

    if (request.request_native_window_launch) {
        result.status = EditorHostSessionStatus::NativeWindowBlocked;
        result.blocked_layer = EditorHostSessionBlockedLayer::NativeWindow;
        result.native_window_blocked = true;
        *out_result = result;
        return result.status;
    }

    if (request.phase == EditorHostApplicationLifecyclePhase::Close) {
        result.status = EditorHostSessionStatus::Success;
        result.blocked_layer = EditorHostSessionBlockedLayer::None;
        result.closed_session = true;
        *out_result = result;
        return result.status;
    }

    if (request.integration_output.size() < EDITOR_HOST_APPLICATION_INTEGRATION_COUNT) {
        result.integration_row_count = EDITOR_HOST_APPLICATION_INTEGRATION_COUNT;
        result.status = EditorHostSessionStatus::IntegrationOutputCapacityExceeded;
        result.blocked_layer = EditorHostSessionBlockedLayer::IntegrationOutput;
        *out_result = result;
        return result.status;
    }

    std::array<
        EditorHostApplicationIntegrationRow,
        EDITOR_HOST_APPLICATION_INTEGRATION_COUNT> staged_integrations{};
    if (request.phase == EditorHostApplicationLifecyclePhase::Restore) {
        EditorHostSessionRestoreRequest restore_request{};
        restore_request.descriptor = request.shell_request.descriptor;
        restore_request.persisted_panels = request.restore_persisted_panels;
        restore_request.panel_output = request.shell_request.panel_output;
        const EditorHostSessionStatus restore_status =
            RestoreEditorHostSessionShell(restore_request, &result.restore);
        if (restore_status != EditorHostSessionStatus::Success) {
            result.status = EditorHostSessionStatus::RestoreLifecycleFailed;
            result.blocked_layer = result.restore.blocked_layer;
            *out_result = result;
            return result.status;
        }

        StageRestoredIntegrationRows(
            request.shell_request.descriptor,
            staged_integrations);
        CommitIntegrationRows(staged_integrations, request.integration_output);
        result.status = EditorHostSessionStatus::Success;
        result.blocked_layer = EditorHostSessionBlockedLayer::None;
        result.restored_session = true;
        result.restored_panel_count = result.restore.restored_panel_count;
        result.integration_row_count = EDITOR_HOST_APPLICATION_INTEGRATION_COUNT;
        result.emitted_integration_rows = true;
        result.requires_runtime_refresh = true;
        *out_result = result;
        return result.status;
    }

    if (LifecycleRequiresRuntimeIntegrations(request.phase)) {
        const EditorHostSessionStatus integration_status =
            ValidateRuntimeIntegrations(request, &result);
        if (integration_status != EditorHostSessionStatus::Success) {
            *out_result = result;
            return result.status;
        }
    }

    const EditorHostSessionStatus shell_status =
        BuildEditorHostSessionShell(request.shell_request, &result.shell);
    if (shell_status != EditorHostSessionStatus::Success) {
        result.status = EditorHostSessionStatus::ShellLifecycleFailed;
        result.blocked_layer = result.shell.blocked_layer;
        *out_result = result;
        return result.status;
    }

    StageRuntimeIntegrationRows(request, staged_integrations);
    CommitIntegrationRows(staged_integrations, request.integration_output);
    result.status = EditorHostSessionStatus::Success;
    result.blocked_layer = EditorHostSessionBlockedLayer::None;
    result.opened_session = request.phase == EditorHostApplicationLifecyclePhase::Open;
    result.refreshed_runtime_truth =
        request.phase == EditorHostApplicationLifecyclePhase::RefreshRuntimeTruth;
    result.persisted_shell_state = result.shell.persisted_shell_state;
    result.persisted_panel_count = result.shell.persisted_panel_count;
    result.integration_row_count = EDITOR_HOST_APPLICATION_INTEGRATION_COUNT;
    result.emitted_integration_rows = true;
    *out_result = result;
    return result.status;
}

}
