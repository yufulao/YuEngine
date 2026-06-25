// Module: Tests EditorHost
// File: Tests/EditorHost/EditorHostSessionTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/EditorHost/EditorHostSession.h"
#include "YuEngine/World/WorldObjectId.h"

namespace {
using yuengine::animationeditor::AnimationEditorSurfaceStatus;
using yuengine::animationeditor::AnimationEditorTimelineWorkflowResult;
using yuengine::editorhost::BuildEditorHostSessionShell;
using yuengine::editorhost::EditorHostPanelDock;
using yuengine::editorhost::EditorHostPanelKind;
using yuengine::editorhost::EditorHostPanelStateRow;
using yuengine::editorhost::EditorHostPersistedPanelRecord;
using yuengine::editorhost::EditorHostSessionBlockedLayer;
using yuengine::editorhost::EditorHostSessionDescriptor;
using yuengine::editorhost::EditorHostSessionRestoreRequest;
using yuengine::editorhost::EditorHostSessionRestoreResult;
using yuengine::editorhost::EditorHostSessionShellRequest;
using yuengine::editorhost::EditorHostSessionShellResult;
using yuengine::editorhost::EditorHostSessionStatus;
using yuengine::editorhost::RestoreEditorHostSessionShell;
using yuengine::editorpackagerun::AuthoredEditorPackageRunBlockedLayer;
using yuengine::editorpackagerun::AuthoredEditorPackageRunResult;
using yuengine::editorpackagerun::AuthoredEditorPackageRunStatus;
using yuengine::runtimeasset::RuntimeAssetDataStatus;
using yuengine::runtimeasset::RuntimeAssetPackageArtifactProductRunMissingLayer;
using yuengine::sceneeditor::SceneEditorSurfaceStatus;
using yuengine::sceneeditor::SceneEditorWorkflowBlockedLayer;
using yuengine::sceneeditor::SceneEditorWorkflowResult;
using yuengine::sceneeditor::SceneEditorWorkflowStatus;
using yuengine::uicore::UiNodeId;
using yuengine::uieditor::UiEditorDesignInspectorWorkflowResult;
using yuengine::uieditor::UiEditorDesignWorkflowBlockedLayer;
using yuengine::uieditor::UiEditorDesignWorkflowStatus;
using yuengine::uieditor::UiEditorSurfaceStatus;
using yuengine::world::WorldObjectId;

constexpr const char *TEST_BUILDS_SHELL =
    "EditorHostSession_BuildsNativeShellPanelStateFromRav6Workflows";
constexpr const char *TEST_PERSIST_RESTORE =
    "EditorHostSession_PersistsAndRestoresPanelStateWithoutRuntimeTruth";
constexpr const char *TEST_MISSING_SCENE =
    "EditorHostSession_RejectsMissingSceneWorkflowWithoutMutation";
constexpr const char *TEST_SMALL_PERSISTENCE =
    "EditorHostSession_RejectsSmallPersistenceOutputWithoutMutation";
constexpr const char *TEST_INVALID_PERSISTED =
    "EditorHostSession_RejectsInvalidPersistedSessionWithoutMutation";

int Fail(const char *message) {
    std::fprintf(stderr, "%s\n", message);
    return 1;
}

WorldObjectId ObjectId(std::uint32_t value) {
    return WorldObjectId{value};
}

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

EditorHostSessionDescriptor Descriptor() {
    EditorHostSessionDescriptor descriptor{};
    descriptor.session_id = 0xA7001U;
    descriptor.layout_revision = 7U;
    descriptor.frame_index = 42U;
    descriptor.viewport_width = 1280U;
    descriptor.viewport_height = 720U;
    descriptor.focused_panel = EditorHostPanelKind::SceneEditor;
    descriptor.native_shell_requested = true;
    return descriptor;
}

SceneEditorWorkflowResult ReadySceneWorkflow() {
    SceneEditorWorkflowResult result{};
    result.status = SceneEditorWorkflowStatus::Success;
    result.blocked_layer = SceneEditorWorkflowBlockedLayer::None;
    result.surface_status = SceneEditorSurfaceStatus::Success;
    result.surface.status = SceneEditorSurfaceStatus::Success;
    result.surface.hierarchy_row_count = 3U;
    result.surface.inspector_row_count = 1U;
    result.surface.selected_object_count = 1U;
    result.surface.consumed_authoring_document = true;
    result.selected_world_object_id = ObjectId(11U);
    result.selected_hierarchy_index = 1U;
    result.workflow_ledger_count = 1U;
    result.consumed_authoring_document = true;
    result.consumed_resource_browser_selection = true;
    result.consumed_viewport_session = true;
    result.consumed_viewport_interaction = true;
    result.hierarchy_selection_matched_viewport = true;
    result.emitted_hierarchy_rows = true;
    result.emitted_inspector_rows = true;
    result.applied_transform_command = true;
    result.committed_workflow = true;
    return result;
}

AnimationEditorTimelineWorkflowResult ReadyAnimationWorkflow() {
    AnimationEditorTimelineWorkflowResult result{};
    result.status = AnimationEditorSurfaceStatus::Success;
    result.blocked_layer =
        yuengine::animationeditor::AnimationEditorSurfaceBlockedLayer::None;
    result.clip_id = 31U;
    result.track_row_count = 2U;
    result.keyframe_marker_count = 4U;
    result.preview_feedback_count = 1U;
    result.selection_feedback_count = 1U;
    result.consumed_workflow_command = true;
    result.scrub_applied = true;
    result.updated_sample_time = true;
    result.consumed_runtime_records = true;
    result.built_timeline_rows = true;
    result.sampled_runtime_values = true;
    result.consumed_preview_host_feedback = true;
    result.emitted_preview_feedback = true;
    result.emitted_selected_track_feedback = true;
    result.emitted_selected_key_feedback = true;
    return result;
}

UiEditorDesignInspectorWorkflowResult ReadyUiWorkflow() {
    UiEditorDesignInspectorWorkflowResult result{};
    result.status = UiEditorDesignWorkflowStatus::Success;
    result.surface_status = UiEditorSurfaceStatus::Success;
    result.blocked_layer = UiEditorDesignWorkflowBlockedLayer::None;
    result.selected_node_id = NodeId(41U);
    result.document_id = 51U;
    result.hierarchy_row_count = 2U;
    result.design_surface_row_count = 2U;
    result.inspector_field_count = 7U;
    result.preview_feedback_count = 1U;
    result.command_ledger_count = 1U;
    result.consumed_runtime_ui_document = true;
    result.consumed_preview_host_feedback = true;
    result.built_design_surface = true;
    result.emitted_hierarchy_rows = true;
    result.emitted_inspector_fields = true;
    result.staged_document_update = true;
    result.emitted_command_ledger = true;
    result.command_applied = true;
    return result;
}

AuthoredEditorPackageRunResult ReadyPackageRun() {
    AuthoredEditorPackageRunResult result{};
    result.status = AuthoredEditorPackageRunStatus::Success;
    result.blocked_layer = AuthoredEditorPackageRunBlockedLayer::None;
    result.runtime_status = RuntimeAssetDataStatus::Success;
    result.product_run_layer = RuntimeAssetPackageArtifactProductRunMissingLayer::None;
    result.cooked_source_file_count = 4U;
    result.cooked_runtime_file_count = 4U;
    result.package_load_plan_record_count = 5U;
    result.consumed_scene_editor_workflow = true;
    result.consumed_animation_editor_workflow = true;
    result.consumed_ui_editor_workflow = true;
    result.consumed_runtime_asset_import_cook = true;
    result.import_cook_wrote_source_and_cooked_files = true;
    result.package_artifact_read = true;
    result.package_registry_rebuilt = true;
    result.package_load_plan_resolved = true;
    result.packaged_runtime_entrypoint_executed = true;
    result.runtime_app_frame_loop_success = true;
    return result;
}

EditorHostSessionShellRequest MakeRequest(
    const SceneEditorWorkflowResult *scene,
    const AnimationEditorTimelineWorkflowResult *animation,
    const UiEditorDesignInspectorWorkflowResult *ui,
    const AuthoredEditorPackageRunResult *package_run,
    bool persist,
    std::span<EditorHostPanelStateRow> panel_output,
    std::span<EditorHostPersistedPanelRecord> persisted_output) {
    EditorHostSessionShellRequest request{};
    request.descriptor = Descriptor();
    request.scene_workflow = scene;
    request.animation_workflow = animation;
    request.ui_workflow = ui;
    request.package_run_workflow = package_run;
    request.require_package_run = package_run != nullptr;
    request.persist_shell_state = persist;
    request.panel_output = panel_output;
    request.persisted_panel_output = persisted_output;
    return request;
}

bool SentinelPanelUnchanged(const EditorHostPanelStateRow &row) {
    return row.session_id == 91U &&
        row.kind == EditorHostPanelKind::PackageRun &&
        row.content_row_count == 92U &&
        row.forged_preview_output;
}

bool SentinelPersistedUnchanged(const EditorHostPersistedPanelRecord &record) {
    return record.session_id == 93U &&
        record.kind == EditorHostPanelKind::UiEditor &&
        record.content_row_count == 94U &&
        record.forged_preview_output;
}

int EditorHostSessionBuildsNativeShellPanelStateFromRav6Workflows() {
    SceneEditorWorkflowResult scene = ReadySceneWorkflow();
    AnimationEditorTimelineWorkflowResult animation = ReadyAnimationWorkflow();
    UiEditorDesignInspectorWorkflowResult ui = ReadyUiWorkflow();
    AuthoredEditorPackageRunResult package_run = ReadyPackageRun();
    std::array<EditorHostPanelStateRow, 4U> panels{};
    std::array<EditorHostPersistedPanelRecord, 4U> persisted{};
    EditorHostSessionShellResult result{};
    const EditorHostSessionStatus status = BuildEditorHostSessionShell(
        MakeRequest(
            &scene,
            &animation,
            &ui,
            &package_run,
            true,
            panels,
            persisted),
        &result);

    if (status != EditorHostSessionStatus::Success ||
        !result.Succeeded() ||
        result.blocked_layer != EditorHostSessionBlockedLayer::None ||
        result.panel_count != 4U ||
        result.persisted_panel_count != 4U ||
        !result.consumed_scene_editor_workflow ||
        !result.consumed_animation_editor_workflow ||
        !result.consumed_ui_editor_workflow ||
        !result.consumed_package_run_workflow ||
        !result.built_native_shell_state ||
        !result.persisted_shell_state ||
        result.opened_native_window ||
        result.owns_runtime_truth ||
        result.mutated_runtime_data ||
        result.forged_preview_output ||
        result.used_forbidden_shell_path ||
        result.used_forbidden_visual_fallback) {
        return Fail("editor host session shell result mismatch");
    }

    if (panels[0U].kind != EditorHostPanelKind::SceneEditor ||
        panels[0U].dock != EditorHostPanelDock::Center ||
        panels[0U].content_row_count != 3U ||
        panels[0U].selected_item_count != 1U ||
        panels[0U].ledger_record_count != 1U ||
        !panels[0U].focused ||
        !panels[0U].preview_feedback_from_preview_host ||
        !panels[0U].dirty ||
        panels[0U].owns_runtime_truth ||
        panels[0U].forged_preview_output ||
        panels[0U].requires_runtime_refresh) {
        return Fail("scene editor host panel row mismatch");
    }

    if (panels[1U].kind != EditorHostPanelKind::AnimationEditor ||
        panels[1U].content_row_count != 6U ||
        panels[1U].selected_item_count != 1U ||
        !panels[1U].preview_feedback_from_preview_host ||
        !panels[1U].dirty) {
        return Fail("animation editor host panel row mismatch");
    }

    if (panels[2U].kind != EditorHostPanelKind::UiEditor ||
        panels[2U].content_row_count != 9U ||
        panels[2U].selected_item_count != 1U ||
        panels[2U].ledger_record_count != 1U ||
        !panels[2U].preview_feedback_from_preview_host ||
        !panels[2U].dirty) {
        return Fail("ui editor host panel row mismatch");
    }

    if (panels[3U].kind != EditorHostPanelKind::PackageRun ||
        panels[3U].content_row_count != 5U ||
        panels[3U].ledger_record_count != 8U ||
        !panels[3U].package_run_ready ||
        panels[3U].preview_feedback_from_preview_host) {
        return Fail("package run host panel row mismatch");
    }

    if (!persisted[0U].shell_state_only ||
        !persisted[0U].requires_runtime_refresh ||
        persisted[0U].owns_runtime_truth ||
        persisted[0U].forged_preview_output ||
        persisted[0U].kind != EditorHostPanelKind::SceneEditor ||
        persisted[3U].kind != EditorHostPanelKind::PackageRun) {
        return Fail("editor host persisted panel rows did not stay shell-only");
    }

    return 0;
}

int EditorHostSessionPersistsAndRestoresPanelStateWithoutRuntimeTruth() {
    SceneEditorWorkflowResult scene = ReadySceneWorkflow();
    AnimationEditorTimelineWorkflowResult animation = ReadyAnimationWorkflow();
    UiEditorDesignInspectorWorkflowResult ui = ReadyUiWorkflow();
    std::array<EditorHostPanelStateRow, 3U> panels{};
    std::array<EditorHostPersistedPanelRecord, 3U> persisted{};
    EditorHostSessionShellResult build_result{};
    const EditorHostSessionStatus build_status = BuildEditorHostSessionShell(
        MakeRequest(
            &scene,
            &animation,
            &ui,
            nullptr,
            true,
            panels,
            persisted),
        &build_result);
    if (build_status != EditorHostSessionStatus::Success ||
        build_result.persisted_panel_count != 3U) {
        return Fail("editor host persistence setup failed");
    }

    std::array<EditorHostPanelStateRow, 3U> restored{};
    EditorHostSessionRestoreRequest restore_request{};
    restore_request.descriptor = Descriptor();
    restore_request.persisted_panels = persisted;
    restore_request.panel_output = restored;
    EditorHostSessionRestoreResult restore_result{};
    const EditorHostSessionStatus restore_status =
        RestoreEditorHostSessionShell(restore_request, &restore_result);

    if (restore_status != EditorHostSessionStatus::Success ||
        !restore_result.Succeeded() ||
        restore_result.blocked_layer != EditorHostSessionBlockedLayer::None ||
        restore_result.restored_panel_count != 3U ||
        !restore_result.restored_shell_state ||
        !restore_result.requires_runtime_refresh ||
        restore_result.opened_native_window ||
        restore_result.owns_runtime_truth ||
        restore_result.mutated_runtime_data ||
        restore_result.forged_preview_output ||
        restore_result.used_forbidden_shell_path ||
        restore_result.used_forbidden_visual_fallback) {
        return Fail("editor host restore result mismatch");
    }

    if (restored[0U].kind != EditorHostPanelKind::SceneEditor ||
        restored[0U].content_row_count != 3U ||
        restored[0U].selected_item_count != 1U ||
        !restored[0U].requires_runtime_refresh ||
        restored[0U].consumed_editor_workflow ||
        restored[0U].preview_feedback_from_preview_host ||
        restored[0U].owns_runtime_truth ||
        restored[0U].forged_preview_output) {
        return Fail("restored scene panel did not require runtime refresh");
    }

    if (restored[1U].kind != EditorHostPanelKind::AnimationEditor ||
        restored[2U].kind != EditorHostPanelKind::UiEditor) {
        return Fail("restored panel order mismatch");
    }

    return 0;
}

int EditorHostSessionRejectsMissingSceneWorkflowWithoutMutation() {
    AnimationEditorTimelineWorkflowResult animation = ReadyAnimationWorkflow();
    UiEditorDesignInspectorWorkflowResult ui = ReadyUiWorkflow();
    std::array<EditorHostPanelStateRow, 1U> panels{};
    panels[0U].session_id = 91U;
    panels[0U].kind = EditorHostPanelKind::PackageRun;
    panels[0U].content_row_count = 92U;
    panels[0U].forged_preview_output = true;
    std::array<EditorHostPersistedPanelRecord, 1U> persisted{};
    persisted[0U].session_id = 93U;
    persisted[0U].kind = EditorHostPanelKind::UiEditor;
    persisted[0U].content_row_count = 94U;
    persisted[0U].forged_preview_output = true;

    EditorHostSessionShellResult result{};
    const EditorHostSessionStatus status = BuildEditorHostSessionShell(
        MakeRequest(
            nullptr,
            &animation,
            &ui,
            nullptr,
            true,
            panels,
            persisted),
        &result);

    if (status != EditorHostSessionStatus::MissingSceneWorkflow ||
        result.blocked_layer != EditorHostSessionBlockedLayer::SceneEditorWorkflow ||
        result.consumed_scene_editor_workflow ||
        result.consumed_animation_editor_workflow ||
        result.built_native_shell_state) {
        return Fail("missing scene workflow status mismatch");
    }

    if (!SentinelPanelUnchanged(panels[0U]) ||
        !SentinelPersistedUnchanged(persisted[0U])) {
        return Fail("missing scene workflow mutated caller outputs");
    }

    return 0;
}

int EditorHostSessionRejectsSmallPersistenceOutputWithoutMutation() {
    SceneEditorWorkflowResult scene = ReadySceneWorkflow();
    AnimationEditorTimelineWorkflowResult animation = ReadyAnimationWorkflow();
    UiEditorDesignInspectorWorkflowResult ui = ReadyUiWorkflow();
    std::array<EditorHostPanelStateRow, 3U> panels{};
    panels[0U].session_id = 91U;
    panels[0U].kind = EditorHostPanelKind::PackageRun;
    panels[0U].content_row_count = 92U;
    panels[0U].forged_preview_output = true;
    std::array<EditorHostPersistedPanelRecord, 2U> persisted{};
    persisted[0U].session_id = 93U;
    persisted[0U].kind = EditorHostPanelKind::UiEditor;
    persisted[0U].content_row_count = 94U;
    persisted[0U].forged_preview_output = true;

    EditorHostSessionShellResult result{};
    const EditorHostSessionStatus status = BuildEditorHostSessionShell(
        MakeRequest(
            &scene,
            &animation,
            &ui,
            nullptr,
            true,
            panels,
            persisted),
        &result);

    if (status != EditorHostSessionStatus::PersistenceOutputCapacityExceeded ||
        result.blocked_layer != EditorHostSessionBlockedLayer::PersistenceStore ||
        !result.consumed_scene_editor_workflow ||
        !result.consumed_animation_editor_workflow ||
        !result.consumed_ui_editor_workflow ||
        result.built_native_shell_state ||
        result.persisted_shell_state) {
        return Fail("small persistence output status mismatch");
    }

    if (!SentinelPanelUnchanged(panels[0U]) ||
        !SentinelPersistedUnchanged(persisted[0U])) {
        return Fail("small persistence output mutated caller outputs");
    }

    return 0;
}

int EditorHostSessionRejectsInvalidPersistedSessionWithoutMutation() {
    std::array<EditorHostPersistedPanelRecord, 1U> persisted{};
    persisted[0U].session_id = 0xBADU;
    persisted[0U].kind = EditorHostPanelKind::SceneEditor;
    persisted[0U].shell_state_only = true;
    persisted[0U].requires_runtime_refresh = true;
    std::array<EditorHostPanelStateRow, 1U> restored{};
    restored[0U].session_id = 91U;
    restored[0U].kind = EditorHostPanelKind::PackageRun;
    restored[0U].content_row_count = 92U;
    restored[0U].forged_preview_output = true;

    EditorHostSessionRestoreRequest request{};
    request.descriptor = Descriptor();
    request.persisted_panels = persisted;
    request.panel_output = restored;
    EditorHostSessionRestoreResult result{};
    const EditorHostSessionStatus status =
        RestoreEditorHostSessionShell(request, &result);

    if (status != EditorHostSessionStatus::InvalidPersistedSession ||
        result.blocked_layer != EditorHostSessionBlockedLayer::PersistenceStore ||
        result.restored_shell_state) {
        return Fail("invalid persisted session status mismatch");
    }

    if (!SentinelPanelUnchanged(restored[0U])) {
        return Fail("invalid persisted session mutated restored output");
    }

    return 0;
}

}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail("expected one test name");
    }

    const std::string_view test_name(argv[1]);
    if (test_name == TEST_BUILDS_SHELL) {
        return EditorHostSessionBuildsNativeShellPanelStateFromRav6Workflows();
    }

    if (test_name == TEST_PERSIST_RESTORE) {
        return EditorHostSessionPersistsAndRestoresPanelStateWithoutRuntimeTruth();
    }

    if (test_name == TEST_MISSING_SCENE) {
        return EditorHostSessionRejectsMissingSceneWorkflowWithoutMutation();
    }

    if (test_name == TEST_SMALL_PERSISTENCE) {
        return EditorHostSessionRejectsSmallPersistenceOutputWithoutMutation();
    }

    if (test_name == TEST_INVALID_PERSISTED) {
        return EditorHostSessionRejectsInvalidPersistedSessionWithoutMutation();
    }

    return Fail("unknown test name");
}
