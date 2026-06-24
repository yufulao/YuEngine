// Module: YuEngine EditorPackageRun
// File: Src/YuEngine/EditorPackageRun/Include/YuEngine/EditorPackageRun/EditorPackageRunSmoke.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/AnimationEditor/AnimationEditorSurface.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"
#include "YuEngine/SceneEditor/SceneEditorSurface.h"
#include "YuEngine/UiEditor/UiEditorSurface.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"

namespace yuengine::editorpackagerun {

enum class AuthoredEditorPackageRunStatus {
    Success,
    InvalidArgument,
    MissingSceneWorkflowOutput,
    MissingAnimationWorkflowOutput,
    MissingUiWorkflowOutput,
    RuntimeAssetCookMissing,
    ProductRunFailed
};

enum class AuthoredEditorPackageRunBlockedLayer {
    None,
    SceneEditorWorkflow,
    AnimationEditorWorkflow,
    UiEditorWorkflow,
    RuntimeAssetImportCook,
    RuntimeAssetFilePackageRunCommand
};

struct AuthoredEditorPackageRunRequest final {
    const yuengine::sceneeditor::SceneEditorWorkflowResult *scene_workflow = nullptr;
    std::span<const yuengine::sceneeditor::SceneEditorWorkflowLedgerRecord>
        scene_workflow_ledger{};
    std::span<const yuengine::world::WorldSceneObjectTransformRestoreTransformRecord>
        scene_transform_output{};
    const yuengine::animationeditor::AnimationEditorTimelineWorkflowResult
        *animation_workflow = nullptr;
    std::span<const yuengine::animationeditor::AnimationEditorTimelineTrackRow>
        animation_track_rows{};
    std::span<const yuengine::animationeditor::AnimationEditorTimelineSelectionFeedbackRecord>
        animation_selection_feedback{};
    const yuengine::uieditor::UiEditorDesignInspectorWorkflowResult *ui_workflow =
        nullptr;
    const yuengine::uieditor::UiEditorRuntimeDocument *ui_runtime_document = nullptr;
    std::span<const yuengine::uieditor::UiEditorRuntimeNodeRecord>
        ui_staged_document{};
    std::span<const yuengine::uieditor::UiEditorDesignCommandLedgerRecord>
        ui_command_ledger{};
    const yuengine::runtimeasset::RuntimeAssetImportCookCommandResult
        *import_cook_result = nullptr;
    const yuengine::runtimeasset::RuntimeAssetPackageArtifactProductRunRequest
        *product_run_request = nullptr;
};

struct AuthoredEditorPackageRunResult final {
    AuthoredEditorPackageRunStatus status =
        AuthoredEditorPackageRunStatus::InvalidArgument;
    AuthoredEditorPackageRunBlockedLayer blocked_layer =
        AuthoredEditorPackageRunBlockedLayer::SceneEditorWorkflow;
    yuengine::runtimeasset::RuntimeAssetDataStatus runtime_status =
        yuengine::runtimeasset::RuntimeAssetDataStatus::InvalidArgument;
    yuengine::runtimeasset::RuntimeAssetImportCookMissingLayer import_cook_layer =
        yuengine::runtimeasset::RuntimeAssetImportCookMissingLayer::None;
    yuengine::runtimeasset::RuntimeAssetPackageArtifactProductRunMissingLayer
        product_run_layer =
            yuengine::runtimeasset::RuntimeAssetPackageArtifactProductRunMissingLayer::Command;
    yuengine::runtimeasset::RuntimeAssetPackageArtifactProductRunResult
        product_run{};
    std::size_t scene_hierarchy_row_count = 0U;
    std::size_t scene_transform_record_count = 0U;
    std::size_t animation_track_row_count = 0U;
    std::size_t animation_selection_feedback_count = 0U;
    std::size_t ui_runtime_node_count = 0U;
    std::size_t ui_staged_node_count = 0U;
    std::size_t ui_command_ledger_count = 0U;
    std::uint32_t cooked_source_file_count = 0U;
    std::uint32_t cooked_runtime_file_count = 0U;
    std::uint32_t package_load_plan_record_count = 0U;
    bool consumed_scene_editor_workflow = false;
    bool consumed_animation_editor_workflow = false;
    bool consumed_ui_editor_workflow = false;
    bool consumed_runtime_asset_import_cook = false;
    bool scene_document_output_available = false;
    bool animation_clip_binding_output_available = false;
    bool ui_runtime_document_output_available = false;
    bool import_cook_wrote_source_and_cooked_files = false;
    bool package_artifact_read = false;
    bool package_registry_rebuilt = false;
    bool package_load_plan_resolved = false;
    bool packaged_runtime_entrypoint_executed = false;
    bool runtime_app_frame_loop_success = false;
    bool wrote_fake_artifact = false;
    bool opened_native_window = false;
    bool used_forbidden_preview_path = false;

    bool Succeeded() const {
        return status == AuthoredEditorPackageRunStatus::Success;
    }
};

AuthoredEditorPackageRunStatus RunAuthoredEditorPackageRunSmoke(
    const AuthoredEditorPackageRunRequest &request,
    AuthoredEditorPackageRunResult *out_result);

}
