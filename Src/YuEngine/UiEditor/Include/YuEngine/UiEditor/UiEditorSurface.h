// Module: YuEngine UiEditor
// File: Src/YuEngine/UiEditor/Include/YuEngine/UiEditor/UiEditorSurface.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/PreviewHost/PreviewHost.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiNodeTreeStatus.h"
#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiRectTransform.h"

namespace yuengine::uieditor {
constexpr std::size_t MAX_UI_EDITOR_DOCUMENT_NODES = 32U;
constexpr std::size_t UI_EDITOR_INSPECTOR_FIELD_COUNT = 7U;

enum class UiEditorComponentKind {
    Unknown,
    Panel,
    Text,
    Image,
    Button,
    Slider
};

enum class UiEditorSurfaceStatus {
    Success,
    InvalidArgument,
    InvalidDocument,
    InvalidNode,
    MissingNode,
    DuplicateNode,
    OutputCapacityExceeded,
    PreviewFeedbackMissing,
    UiCoreFailed
};

enum class UiEditorSurfaceBlockedLayer {
    None,
    RuntimeUiDocument,
    UiCoreNodeTree,
    PreviewHostFeedback,
    Output
};

enum class UiEditorDesignWorkflowStatus {
    Success,
    InvalidArgument,
    InvalidDocument,
    InvalidNode,
    MissingNode,
    DuplicateNode,
    OutputCapacityExceeded,
    PreviewFeedbackMissing,
    UiCoreFailed,
    CommandFailed
};

enum class UiEditorDesignWorkflowBlockedLayer {
    None,
    RuntimeUiDocument,
    UiCoreNodeTree,
    PreviewHostFeedback,
    InspectorSelection,
    Command,
    Output
};

enum class UiEditorInspectorFieldKind {
    Unknown,
    ComponentKind,
    Visible,
    Enabled,
    HitTestable,
    RuntimeExported,
    Layer,
    SiblingOrder,
    RectTransform
};

enum class UiEditorDesignCommandKind {
    None,
    SetVisible,
    SetEnabled,
    SetHitTestable,
    SetLayer,
    SetRectTransform
};

struct UiEditorRuntimeDocumentHeader final {
    std::uint32_t document_id = 0U;
    std::uint32_t schema_version = 0U;
    std::uint32_t node_count = 0U;
    float viewport_width = 0.0F;
    float viewport_height = 0.0F;
    bool is_valid = false;
};

struct UiEditorRuntimeNodeRecord final {
    yuengine::uicore::UiNodeId node_id{};
    yuengine::uicore::UiNodeId parent_id{};
    yuengine::uicore::UiRectTransform rect_transform{};
    UiEditorComponentKind component_kind = UiEditorComponentKind::Unknown;
    std::uint32_t sibling_order = 0U;
    std::int32_t layer = 0;
    bool visible = true;
    bool enabled = true;
    bool hit_testable = true;
    bool runtime_exported = true;
};

struct UiEditorRuntimeDocument final {
    UiEditorRuntimeDocumentHeader header{};
    std::span<const UiEditorRuntimeNodeRecord> nodes{};
};

struct UiEditorHierarchyRow final {
    std::uint32_t document_id = 0U;
    yuengine::uicore::UiNodeId node_id{};
    yuengine::uicore::UiNodeId parent_id{};
    UiEditorComponentKind component_kind = UiEditorComponentKind::Unknown;
    yuengine::uicore::UiRect world_rect{};
    std::uint32_t sibling_order = 0U;
    std::uint32_t depth = 0U;
    std::int32_t layer = 0;
    bool selected = false;
    bool visible = false;
    bool enabled = false;
    bool runtime_exported = false;
};

struct UiEditorPreviewFeedbackRecord final {
    std::uint32_t document_id = 0U;
    yuengine::uicore::UiNodeId selected_node_id{};
    std::uint32_t frame_id = 0U;
    std::uint16_t viewport_width = 0U;
    std::uint16_t viewport_height = 0U;
    yuengine::previewhost::PreviewHostStatus preview_status =
        yuengine::previewhost::PreviewHostStatus::InvalidArgument;
    yuengine::previewhost::PreviewHostFrameFormat frame_format =
        yuengine::previewhost::PreviewHostFrameFormat::Unknown;
    std::size_t diagnostic_count = 0U;
    std::size_t capture_bytes_written = 0U;
    bool preview_frame_built = false;
    bool headless_output = false;
    bool feedback_from_preview_host = false;
};

struct UiEditorDesignSurfaceRow final {
    std::uint32_t document_id = 0U;
    yuengine::uicore::UiNodeId node_id{};
    yuengine::uicore::UiNodeId parent_id{};
    UiEditorComponentKind component_kind = UiEditorComponentKind::Unknown;
    yuengine::uicore::UiRect world_rect{};
    std::uint32_t sibling_order = 0U;
    std::uint32_t depth = 0U;
    std::int32_t layer = 0;
    std::uint32_t preview_frame_id = 0U;
    yuengine::previewhost::PreviewHostStatus preview_status =
        yuengine::previewhost::PreviewHostStatus::InvalidArgument;
    bool selected = false;
    bool visible = false;
    bool enabled = false;
    bool hit_testable = false;
    bool runtime_exported = false;
    bool preview_feedback_available = false;
};

struct UiEditorInspectorFieldRow final {
    std::uint32_t document_id = 0U;
    yuengine::uicore::UiNodeId node_id{};
    UiEditorInspectorFieldKind field_kind = UiEditorInspectorFieldKind::Unknown;
    UiEditorComponentKind component_kind = UiEditorComponentKind::Unknown;
    yuengine::uicore::UiRectTransform rect_transform{};
    std::int32_t int_value = 0;
    bool bool_value = false;
    bool selected_node = false;
    bool editable = false;
};

struct UiEditorDesignCommand final {
    UiEditorDesignCommandKind kind = UiEditorDesignCommandKind::None;
    yuengine::uicore::UiRectTransform rect_transform{};
    std::uint32_t command_sequence = 0U;
    std::int32_t int_value = 0;
    bool bool_value = false;
};

struct UiEditorDesignCommandLedgerRecord final {
    std::uint32_t document_id = 0U;
    yuengine::uicore::UiNodeId node_id{};
    UiEditorDesignCommandKind command_kind = UiEditorDesignCommandKind::None;
    std::uint32_t command_sequence = 0U;
    UiEditorComponentKind component_kind = UiEditorComponentKind::Unknown;
    yuengine::uicore::UiRectTransform before_rect_transform{};
    yuengine::uicore::UiRectTransform after_rect_transform{};
    std::int32_t before_layer = 0;
    std::int32_t after_layer = 0;
    bool before_visible = false;
    bool after_visible = false;
    bool before_enabled = false;
    bool after_enabled = false;
    bool before_hit_testable = false;
    bool after_hit_testable = false;
    bool staged_document_update = false;
    bool command_applied = false;
};

struct UiEditorRuntimeDocumentSurfaceRequest final {
    const UiEditorRuntimeDocument *document = nullptr;
    yuengine::uicore::UiNodeId selected_node_id{};
    const yuengine::previewhost::PreviewHostFrameResult *preview_frame = nullptr;
    bool require_preview_feedback = false;
    std::span<UiEditorHierarchyRow> hierarchy_output{};
    std::span<UiEditorPreviewFeedbackRecord> preview_feedback_output{};
};

struct UiEditorRuntimeDocumentSurfaceResult final {
    UiEditorSurfaceStatus status = UiEditorSurfaceStatus::InvalidArgument;
    yuengine::uicore::UiNodeTreeStatus ui_core_status =
        yuengine::uicore::UiNodeTreeStatus::Success;
    yuengine::previewhost::PreviewHostStatus preview_status =
        yuengine::previewhost::PreviewHostStatus::InvalidArgument;
    UiEditorSurfaceBlockedLayer blocked_layer =
        UiEditorSurfaceBlockedLayer::RuntimeUiDocument;
    std::uint32_t document_id = 0U;
    std::size_t runtime_node_count = 0U;
    std::size_t hierarchy_row_count = 0U;
    std::size_t preview_feedback_count = 0U;
    bool consumed_runtime_ui_document = false;
    bool built_ui_node_tree = false;
    bool built_hierarchy_rows = false;
    bool consumed_preview_host_feedback = false;
    bool emitted_preview_feedback = false;
    bool mutated_runtime_data = false;
    bool opened_native_window = false;
    bool used_forbidden_preview_path = false;

    bool Succeeded() const {
        return status == UiEditorSurfaceStatus::Success;
    }
};

struct UiEditorDesignInspectorWorkflowRequest final {
    const UiEditorRuntimeDocument *document = nullptr;
    yuengine::uicore::UiNodeId selected_node_id{};
    const yuengine::previewhost::PreviewHostFrameResult *preview_frame = nullptr;
    UiEditorDesignCommand command{};
    std::span<UiEditorHierarchyRow> hierarchy_output{};
    std::span<UiEditorDesignSurfaceRow> design_surface_output{};
    std::span<UiEditorInspectorFieldRow> inspector_output{};
    std::span<UiEditorPreviewFeedbackRecord> preview_feedback_output{};
    std::span<UiEditorRuntimeNodeRecord> staged_document_output{};
    std::span<UiEditorDesignCommandLedgerRecord> command_ledger_output{};
};

struct UiEditorDesignInspectorWorkflowResult final {
    UiEditorDesignWorkflowStatus status =
        UiEditorDesignWorkflowStatus::InvalidArgument;
    UiEditorSurfaceStatus surface_status = UiEditorSurfaceStatus::InvalidArgument;
    UiEditorDesignWorkflowBlockedLayer blocked_layer =
        UiEditorDesignWorkflowBlockedLayer::RuntimeUiDocument;
    UiEditorRuntimeDocumentSurfaceResult surface{};
    yuengine::uicore::UiNodeId selected_node_id{};
    std::uint32_t document_id = 0U;
    std::size_t hierarchy_row_count = 0U;
    std::size_t design_surface_row_count = 0U;
    std::size_t inspector_field_count = 0U;
    std::size_t preview_feedback_count = 0U;
    std::size_t staged_node_count = 0U;
    std::size_t command_ledger_count = 0U;
    bool consumed_runtime_ui_document = false;
    bool consumed_preview_host_feedback = false;
    bool built_design_surface = false;
    bool emitted_hierarchy_rows = false;
    bool emitted_inspector_fields = false;
    bool staged_document_update = false;
    bool emitted_command_ledger = false;
    bool command_applied = false;
    bool mutated_runtime_data = false;
    bool opened_native_window = false;
    bool used_forbidden_preview_path = false;

    bool Succeeded() const {
        return status == UiEditorDesignWorkflowStatus::Success;
    }
};

UiEditorSurfaceStatus BuildUiEditorRuntimeDocumentSurface(
    const UiEditorRuntimeDocumentSurfaceRequest &request,
    UiEditorRuntimeDocumentSurfaceResult *out_result);

UiEditorDesignWorkflowStatus BuildUiEditorDesignInspectorWorkflowSurface(
    const UiEditorDesignInspectorWorkflowRequest &request,
    UiEditorDesignInspectorWorkflowResult *out_result);
}
