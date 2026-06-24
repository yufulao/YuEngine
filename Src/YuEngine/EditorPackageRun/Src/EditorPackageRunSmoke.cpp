// Module: YuEngine EditorPackageRun
// File: Src/YuEngine/EditorPackageRun/Src/EditorPackageRunSmoke.cpp

#include "YuEngine/EditorPackageRun/EditorPackageRunSmoke.h"

namespace yuengine::editorpackagerun {
namespace {
using yuengine::animationeditor::AnimationEditorSurfaceStatus;
using yuengine::runtimeasset::RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT;
using yuengine::runtimeasset::RuntimeAssetDataStatus;
using yuengine::runtimeasset::RuntimeAssetImportCookMissingLayer;
using yuengine::runtimeasset::RuntimeAssetPackageArtifactProductRunMissingLayer;
using yuengine::runtimeasset::RuntimeAssetPackagedRunBlockedLayer;
using yuengine::runtimeasset::RunRuntimeAssetPackageArtifactProductCommand;
using yuengine::sceneeditor::SceneEditorWorkflowBlockedLayer;
using yuengine::sceneeditor::SceneEditorWorkflowStatus;
using yuengine::uieditor::UiEditorDesignWorkflowStatus;

void FailResult(
    AuthoredEditorPackageRunResult *result,
    AuthoredEditorPackageRunStatus status,
    AuthoredEditorPackageRunBlockedLayer layer,
    RuntimeAssetDataStatus runtime_status) {
    result->status = status;
    result->blocked_layer = layer;
    result->runtime_status = runtime_status;
}

bool ValidateSceneWorkflow(
    const AuthoredEditorPackageRunRequest &request,
    AuthoredEditorPackageRunResult *result) {
    if (request.scene_workflow == nullptr) {
        FailResult(
            result,
            AuthoredEditorPackageRunStatus::MissingSceneWorkflowOutput,
            AuthoredEditorPackageRunBlockedLayer::SceneEditorWorkflow,
            RuntimeAssetDataStatus::InvalidArgument);
        return false;
    }

    const sceneeditor::SceneEditorWorkflowResult &workflow = *request.scene_workflow;
    result->scene_hierarchy_row_count = workflow.surface.hierarchy_row_count;
    result->scene_transform_record_count = workflow.transform.transform_record_count;
    const bool workflow_valid =
        workflow.Succeeded() &&
        workflow.status == SceneEditorWorkflowStatus::Success &&
        workflow.blocked_layer == SceneEditorWorkflowBlockedLayer::None &&
        workflow.consumed_authoring_document &&
        workflow.consumed_resource_browser_selection &&
        workflow.consumed_viewport_session &&
        workflow.consumed_viewport_interaction &&
        workflow.committed_workflow &&
        workflow.emitted_hierarchy_rows &&
        workflow.emitted_inspector_rows &&
        workflow.emitted_transform_ledger &&
        !workflow.mutated_runtime_data &&
        !workflow.opened_native_window;
    const bool output_valid =
        !request.scene_workflow_ledger.empty() &&
        request.scene_workflow_ledger[0U].committed_workflow &&
        request.scene_transform_output.size() >= workflow.transform.transform_record_count &&
        workflow.transform.transform_record_count > 0U;
    if (!workflow_valid || !output_valid) {
        FailResult(
            result,
            AuthoredEditorPackageRunStatus::MissingSceneWorkflowOutput,
            AuthoredEditorPackageRunBlockedLayer::SceneEditorWorkflow,
            RuntimeAssetDataStatus::InvalidArgument);
        return false;
    }

    result->consumed_scene_editor_workflow = true;
    result->scene_document_output_available = true;
    return true;
}

bool ValidateAnimationWorkflow(
    const AuthoredEditorPackageRunRequest &request,
    AuthoredEditorPackageRunResult *result) {
    if (request.animation_workflow == nullptr) {
        FailResult(
            result,
            AuthoredEditorPackageRunStatus::MissingAnimationWorkflowOutput,
            AuthoredEditorPackageRunBlockedLayer::AnimationEditorWorkflow,
            RuntimeAssetDataStatus::InvalidArgument);
        return false;
    }

    const animationeditor::AnimationEditorTimelineWorkflowResult &workflow =
        *request.animation_workflow;
    result->animation_track_row_count = workflow.track_row_count;
    result->animation_selection_feedback_count = workflow.selection_feedback_count;
    const bool workflow_valid =
        workflow.Succeeded() &&
        workflow.status == AnimationEditorSurfaceStatus::Success &&
        workflow.consumed_workflow_command &&
        workflow.updated_sample_time &&
        workflow.consumed_runtime_records &&
        workflow.built_timeline_rows &&
        workflow.sampled_runtime_values &&
        workflow.consumed_preview_host_feedback &&
        workflow.emitted_preview_feedback &&
        workflow.emitted_selected_track_feedback &&
        workflow.emitted_selected_key_feedback &&
        !workflow.mutated_runtime_data &&
        !workflow.opened_native_window;
    const bool output_valid =
        request.animation_track_rows.size() >= workflow.track_row_count &&
        request.animation_selection_feedback.size() >= workflow.selection_feedback_count &&
        workflow.track_row_count > 0U &&
        workflow.selection_feedback_count > 0U &&
        request.animation_selection_feedback[0U].selected_track &&
        request.animation_selection_feedback[0U].matched_preview_host_feedback;
    if (!workflow_valid || !output_valid) {
        FailResult(
            result,
            AuthoredEditorPackageRunStatus::MissingAnimationWorkflowOutput,
            AuthoredEditorPackageRunBlockedLayer::AnimationEditorWorkflow,
            RuntimeAssetDataStatus::InvalidArgument);
        return false;
    }

    result->consumed_animation_editor_workflow = true;
    result->animation_clip_binding_output_available = true;
    return true;
}

bool ValidateUiWorkflow(
    const AuthoredEditorPackageRunRequest &request,
    AuthoredEditorPackageRunResult *result) {
    if (request.ui_workflow == nullptr || request.ui_runtime_document == nullptr) {
        FailResult(
            result,
            AuthoredEditorPackageRunStatus::MissingUiWorkflowOutput,
            AuthoredEditorPackageRunBlockedLayer::UiEditorWorkflow,
            RuntimeAssetDataStatus::InvalidArgument);
        return false;
    }

    const uieditor::UiEditorDesignInspectorWorkflowResult &workflow =
        *request.ui_workflow;
    result->ui_runtime_node_count = request.ui_runtime_document->nodes.size();
    result->ui_staged_node_count = workflow.staged_node_count;
    result->ui_command_ledger_count = workflow.command_ledger_count;
    const bool document_valid =
        request.ui_runtime_document->header.is_valid &&
        request.ui_runtime_document->header.node_count > 0U &&
        request.ui_runtime_document->nodes.size() >=
            request.ui_runtime_document->header.node_count;
    const bool workflow_valid =
        workflow.Succeeded() &&
        workflow.status == UiEditorDesignWorkflowStatus::Success &&
        workflow.consumed_runtime_ui_document &&
        workflow.consumed_preview_host_feedback &&
        workflow.built_design_surface &&
        workflow.emitted_hierarchy_rows &&
        workflow.emitted_inspector_fields &&
        workflow.staged_document_update &&
        workflow.emitted_command_ledger &&
        workflow.command_applied &&
        !workflow.mutated_runtime_data &&
        !workflow.opened_native_window &&
        !workflow.used_forbidden_preview_path;
    const bool output_valid =
        request.ui_staged_document.size() >= workflow.staged_node_count &&
        request.ui_command_ledger.size() >= workflow.command_ledger_count &&
        workflow.staged_node_count > 0U &&
        workflow.command_ledger_count > 0U &&
        request.ui_command_ledger[0U].staged_document_update &&
        request.ui_command_ledger[0U].command_applied;
    if (!document_valid || !workflow_valid || !output_valid) {
        FailResult(
            result,
            AuthoredEditorPackageRunStatus::MissingUiWorkflowOutput,
            AuthoredEditorPackageRunBlockedLayer::UiEditorWorkflow,
            RuntimeAssetDataStatus::InvalidArgument);
        return false;
    }

    result->consumed_ui_editor_workflow = true;
    result->ui_runtime_document_output_available = true;
    return true;
}

bool ValidateRuntimeAssetCook(
    const AuthoredEditorPackageRunRequest &request,
    AuthoredEditorPackageRunResult *result) {
    if (request.import_cook_result == nullptr) {
        FailResult(
            result,
            AuthoredEditorPackageRunStatus::RuntimeAssetCookMissing,
            AuthoredEditorPackageRunBlockedLayer::RuntimeAssetImportCook,
            RuntimeAssetDataStatus::InvalidArgument);
        result->import_cook_layer = RuntimeAssetImportCookMissingLayer::Command;
        return false;
    }

    const runtimeasset::RuntimeAssetImportCookCommandResult &cook =
        *request.import_cook_result;
    result->runtime_status = cook.status;
    result->import_cook_layer = cook.missing_layer;
    result->cooked_source_file_count = cook.fixture.source_file_count;
    result->cooked_runtime_file_count = cook.fixture.cooked_file_count;
    const bool cook_valid =
        cook.status == RuntimeAssetDataStatus::Success &&
        cook.fixture.status == RuntimeAssetDataStatus::Success &&
        cook.fixture.wrote_to_disk &&
        cook.fixture.validated_source_files &&
        cook.fixture.validated_cooked_files &&
        cook.fixture.source_file_count == RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT &&
        cook.fixture.cooked_file_count == RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT;
    if (!cook_valid) {
        FailResult(
            result,
            AuthoredEditorPackageRunStatus::RuntimeAssetCookMissing,
            AuthoredEditorPackageRunBlockedLayer::RuntimeAssetImportCook,
            cook.status);
        return false;
    }

    result->consumed_runtime_asset_import_cook = true;
    result->import_cook_wrote_source_and_cooked_files = true;
    return true;
}

bool ValidateProductRunRequest(
    const AuthoredEditorPackageRunRequest &request,
    AuthoredEditorPackageRunResult *result) {
    if (request.product_run_request == nullptr) {
        FailResult(
            result,
            AuthoredEditorPackageRunStatus::ProductRunFailed,
            AuthoredEditorPackageRunBlockedLayer::RuntimeAssetFilePackageRunCommand,
            RuntimeAssetDataStatus::InvalidArgument);
        result->product_run_layer =
            RuntimeAssetPackageArtifactProductRunMissingLayer::Command;
        return false;
    }

    return true;
}

void CopyProductRunLedger(AuthoredEditorPackageRunResult *result) {
    result->product_run_layer = result->product_run.missing_layer;
    result->package_load_plan_record_count =
        result->product_run.package_load_plan_record_count;
    result->package_artifact_read = result->product_run.package_artifact_read;
    result->package_registry_rebuilt = result->product_run.package_registry_rebuilt;
    result->package_load_plan_resolved = result->product_run.package_load_plan_resolved;
    result->packaged_runtime_entrypoint_executed =
        result->product_run.packaged_run_executed &&
        result->product_run.packaged_run.packaged_runtime_entrypoint_available;
    result->runtime_app_frame_loop_success =
        result->product_run.packaged_run.runtime_app_frame_loop_success;
}
}

AuthoredEditorPackageRunStatus RunAuthoredEditorPackageRunSmoke(
    const AuthoredEditorPackageRunRequest &request,
    AuthoredEditorPackageRunResult *out_result) {
    if (out_result == nullptr) {
        return AuthoredEditorPackageRunStatus::InvalidArgument;
    }

    AuthoredEditorPackageRunResult result{};
    if (!ValidateSceneWorkflow(request, &result)) {
        *out_result = result;
        return result.status;
    }

    if (!ValidateAnimationWorkflow(request, &result)) {
        *out_result = result;
        return result.status;
    }

    if (!ValidateUiWorkflow(request, &result)) {
        *out_result = result;
        return result.status;
    }

    if (!ValidateRuntimeAssetCook(request, &result)) {
        *out_result = result;
        return result.status;
    }

    if (!ValidateProductRunRequest(request, &result)) {
        *out_result = result;
        return result.status;
    }

    result.blocked_layer =
        AuthoredEditorPackageRunBlockedLayer::RuntimeAssetFilePackageRunCommand;
    result.runtime_status = RunRuntimeAssetPackageArtifactProductCommand(
        *request.product_run_request,
        &result.product_run);
    CopyProductRunLedger(&result);
    if (result.runtime_status != RuntimeAssetDataStatus::Success ||
        result.product_run.status != RuntimeAssetDataStatus::Success ||
        result.product_run.missing_layer !=
            RuntimeAssetPackageArtifactProductRunMissingLayer::None ||
        result.product_run.packaged_run.blocked_layer !=
            RuntimeAssetPackagedRunBlockedLayer::None) {
        result.status = AuthoredEditorPackageRunStatus::ProductRunFailed;
        *out_result = result;
        return result.status;
    }

    result.status = AuthoredEditorPackageRunStatus::Success;
    result.blocked_layer = AuthoredEditorPackageRunBlockedLayer::None;
    result.product_run_layer = RuntimeAssetPackageArtifactProductRunMissingLayer::None;
    *out_result = result;
    return result.status;
}

}
