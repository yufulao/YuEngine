// Module: YuEngine EditorHost
// File: Src/YuEngine/EditorHost/Include/YuEngine/EditorHost/EditorHostSession.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/AnimationEditor/AnimationEditorSurface.h"
#include "YuEngine/EditorPackageRun/EditorPackageRunSmoke.h"
#include "YuEngine/SceneEditor/SceneEditorSurface.h"
#include "YuEngine/UiEditor/UiEditorSurface.h"

namespace yuengine::editorhost {
constexpr std::uint64_t INVALID_EDITOR_HOST_SESSION_ID = 0U;
constexpr std::size_t MAX_EDITOR_HOST_PANEL_COUNT = 6U;

enum class EditorHostSessionStatus {
    Success,
    InvalidArgument,
    MissingSceneWorkflow,
    MissingAnimationWorkflow,
    MissingUiWorkflow,
    MissingPackageRunWorkflow,
    PanelOutputCapacityExceeded,
    PersistenceOutputCapacityExceeded,
    InvalidPersistedSession
};

enum class EditorHostSessionBlockedLayer {
    None,
    SessionDescriptor,
    SceneEditorWorkflow,
    AnimationEditorWorkflow,
    UiEditorWorkflow,
    PackageRunWorkflow,
    PanelOutput,
    PersistenceStore
};

enum class EditorHostPanelKind {
    Unknown,
    SceneEditor,
    AnimationEditor,
    UiEditor,
    PackageRun
};

enum class EditorHostPanelDock {
    Left,
    Center,
    Right,
    Bottom
};

struct EditorHostSessionDescriptor final {
    std::uint64_t session_id = INVALID_EDITOR_HOST_SESSION_ID;
    std::uint32_t layout_revision = 0U;
    std::uint32_t frame_index = 0U;
    std::uint16_t viewport_width = 0U;
    std::uint16_t viewport_height = 0U;
    EditorHostPanelKind focused_panel = EditorHostPanelKind::SceneEditor;
    bool native_shell_requested = true;
};

struct EditorHostPanelStateRow final {
    std::uint64_t session_id = INVALID_EDITOR_HOST_SESSION_ID;
    std::uint32_t layout_revision = 0U;
    EditorHostPanelKind kind = EditorHostPanelKind::Unknown;
    EditorHostPanelDock dock = EditorHostPanelDock::Left;
    std::uint32_t order = 0U;
    std::uint32_t content_row_count = 0U;
    std::uint32_t selected_item_count = 0U;
    std::uint32_t ledger_record_count = 0U;
    std::uint32_t preview_frame_id = 0U;
    bool visible = true;
    bool focused = false;
    bool dirty = false;
    bool persistable = true;
    bool consumed_editor_workflow = false;
    bool preview_feedback_from_preview_host = false;
    bool package_run_ready = false;
    bool owns_runtime_truth = false;
    bool forged_preview_output = false;
    bool requires_runtime_refresh = false;
};

struct EditorHostPersistedPanelRecord final {
    std::uint64_t session_id = INVALID_EDITOR_HOST_SESSION_ID;
    std::uint32_t layout_revision = 0U;
    EditorHostPanelKind kind = EditorHostPanelKind::Unknown;
    EditorHostPanelDock dock = EditorHostPanelDock::Left;
    std::uint32_t order = 0U;
    std::uint32_t content_row_count = 0U;
    std::uint32_t selected_item_count = 0U;
    std::uint32_t ledger_record_count = 0U;
    bool visible = true;
    bool focused = false;
    bool dirty = false;
    bool shell_state_only = true;
    bool owns_runtime_truth = false;
    bool forged_preview_output = false;
    bool requires_runtime_refresh = true;
};

struct EditorHostSessionShellRequest final {
    EditorHostSessionDescriptor descriptor{};
    const yuengine::sceneeditor::SceneEditorWorkflowResult *scene_workflow = nullptr;
    const yuengine::animationeditor::AnimationEditorTimelineWorkflowResult
        *animation_workflow = nullptr;
    const yuengine::uieditor::UiEditorDesignInspectorWorkflowResult *ui_workflow =
        nullptr;
    const yuengine::editorpackagerun::AuthoredEditorPackageRunResult
        *package_run_workflow = nullptr;
    bool require_package_run = false;
    bool persist_shell_state = false;
    std::span<EditorHostPanelStateRow> panel_output{};
    std::span<EditorHostPersistedPanelRecord> persisted_panel_output{};
};

struct EditorHostSessionShellResult final {
    EditorHostSessionStatus status = EditorHostSessionStatus::InvalidArgument;
    EditorHostSessionBlockedLayer blocked_layer =
        EditorHostSessionBlockedLayer::SessionDescriptor;
    std::uint64_t session_id = INVALID_EDITOR_HOST_SESSION_ID;
    std::uint32_t layout_revision = 0U;
    std::size_t panel_count = 0U;
    std::size_t persisted_panel_count = 0U;
    std::size_t restored_panel_count = 0U;
    bool consumed_scene_editor_workflow = false;
    bool consumed_animation_editor_workflow = false;
    bool consumed_ui_editor_workflow = false;
    bool consumed_package_run_workflow = false;
    bool built_native_shell_state = false;
    bool persisted_shell_state = false;
    bool restored_shell_state = false;
    bool native_shell_requested = false;
    bool opened_native_window = false;
    bool owns_runtime_truth = false;
    bool mutated_runtime_data = false;
    bool forged_preview_output = false;
    bool used_forbidden_shell_path = false;
    bool used_forbidden_visual_fallback = false;

    bool Succeeded() const {
        return status == EditorHostSessionStatus::Success;
    }
};

struct EditorHostSessionRestoreRequest final {
    EditorHostSessionDescriptor descriptor{};
    std::span<const EditorHostPersistedPanelRecord> persisted_panels{};
    std::span<EditorHostPanelStateRow> panel_output{};
};

struct EditorHostSessionRestoreResult final {
    EditorHostSessionStatus status = EditorHostSessionStatus::InvalidArgument;
    EditorHostSessionBlockedLayer blocked_layer =
        EditorHostSessionBlockedLayer::SessionDescriptor;
    std::uint64_t session_id = INVALID_EDITOR_HOST_SESSION_ID;
    std::uint32_t layout_revision = 0U;
    std::size_t restored_panel_count = 0U;
    bool restored_shell_state = false;
    bool requires_runtime_refresh = false;
    bool opened_native_window = false;
    bool owns_runtime_truth = false;
    bool mutated_runtime_data = false;
    bool forged_preview_output = false;
    bool used_forbidden_shell_path = false;
    bool used_forbidden_visual_fallback = false;

    bool Succeeded() const {
        return status == EditorHostSessionStatus::Success;
    }
};

EditorHostSessionStatus BuildEditorHostSessionShell(
    const EditorHostSessionShellRequest &request,
    EditorHostSessionShellResult *out_result);

EditorHostSessionStatus RestoreEditorHostSessionShell(
    const EditorHostSessionRestoreRequest &request,
    EditorHostSessionRestoreResult *out_result);

}
