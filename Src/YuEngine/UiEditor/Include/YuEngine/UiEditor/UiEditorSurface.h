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

UiEditorSurfaceStatus BuildUiEditorRuntimeDocumentSurface(
    const UiEditorRuntimeDocumentSurfaceRequest &request,
    UiEditorRuntimeDocumentSurfaceResult *out_result);
}
