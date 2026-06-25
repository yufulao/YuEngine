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
using SceneEditorWorkflowResult = yuengine::sceneeditor::SceneEditorWorkflowResult;
using UiEditorDesignInspectorWorkflowResult =
    yuengine::uieditor::UiEditorDesignInspectorWorkflowResult;

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
        !workflow->opened_native_window &&
        !workflow->used_web_timeline;
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
        result.status = EditorHostSessionStatus::PanelOutputCapacityExceeded;
        result.blocked_layer = EditorHostSessionBlockedLayer::PanelOutput;
        *out_result = result;
        return result.status;
    }

    if (request.persist_shell_state &&
        request.persisted_panel_output.size() < panel_count) {
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

}
