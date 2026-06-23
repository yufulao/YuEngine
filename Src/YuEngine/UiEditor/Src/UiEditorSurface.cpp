// Module: YuEngine UiEditor
// File: Src/YuEngine/UiEditor/Src/UiEditorSurface.cpp

#include "YuEngine/UiEditor/UiEditorSurface.h"

#include <array>
#include <cstddef>

#include "YuEngine/UiCore/UiNodeDesc.h"
#include "YuEngine/UiCore/UiNodeRecord.h"
#include "YuEngine/UiCore/UiNodeTree.h"
#include "YuEngine/UiCore/UiNodeTreeDesc.h"
#include "YuEngine/UiCore/UiNodeTreeResult.h"

namespace yuengine::uieditor {
namespace {
using PreviewHostFrameResult = yuengine::previewhost::PreviewHostFrameResult;
using UiNodeDesc = yuengine::uicore::UiNodeDesc;
using UiNodeId = yuengine::uicore::UiNodeId;
using UiNodeRecord = yuengine::uicore::UiNodeRecord;
using UiNodeTree = yuengine::uicore::UiNodeTree;
using UiNodeTreeDesc = yuengine::uicore::UiNodeTreeDesc;
using UiNodeTreeResult = yuengine::uicore::UiNodeTreeResult;
using UiNodeTreeStatus = yuengine::uicore::UiNodeTreeStatus;
using UiRect = yuengine::uicore::UiRect;

template <typename T>
bool IsSpanStorageValid(std::span<T> values) {
    if (values.empty()) {
        return true;
    }

    return values.data() != nullptr;
}

template <typename T>
bool IsConstSpanStorageValid(std::span<const T> values) {
    if (values.empty()) {
        return true;
    }

    return values.data() != nullptr;
}

UiEditorSurfaceStatus MapUiCoreStatus(UiNodeTreeStatus status) {
    if (status == UiNodeTreeStatus::Success) {
        return UiEditorSurfaceStatus::Success;
    }

    if (status == UiNodeTreeStatus::DuplicateNodeId) {
        return UiEditorSurfaceStatus::DuplicateNode;
    }

    if (status == UiNodeTreeStatus::NodeNotFound ||
        status == UiNodeTreeStatus::ParentNotFound) {
        return UiEditorSurfaceStatus::MissingNode;
    }

    if (status == UiNodeTreeStatus::CapacityExceeded) {
        return UiEditorSurfaceStatus::OutputCapacityExceeded;
    }

    if (status == UiNodeTreeStatus::InvalidNodeId ||
        status == UiNodeTreeStatus::InvalidRect ||
        status == UiNodeTreeStatus::SelfParent ||
        status == UiNodeTreeStatus::CycleRejected) {
        return UiEditorSurfaceStatus::InvalidNode;
    }

    return UiEditorSurfaceStatus::UiCoreFailed;
}

bool IsComponentKindValid(UiEditorComponentKind kind) {
    switch (kind) {
        case UiEditorComponentKind::Panel:
        case UiEditorComponentKind::Text:
        case UiEditorComponentKind::Image:
        case UiEditorComponentKind::Button:
        case UiEditorComponentKind::Slider:
            return true;
        case UiEditorComponentKind::Unknown:
            break;
    }

    return false;
}

bool IsHeaderValid(const UiEditorRuntimeDocumentHeader &header) {
    if (!header.is_valid) {
        return false;
    }

    if (header.document_id == 0U || header.schema_version == 0U) {
        return false;
    }

    if (header.node_count == 0U ||
        header.node_count > MAX_UI_EDITOR_DOCUMENT_NODES) {
        return false;
    }

    if (header.viewport_width <= 0.0F || header.viewport_height <= 0.0F) {
        return false;
    }

    return true;
}

const UiEditorRuntimeNodeRecord *FindDocumentNode(
    std::span<const UiEditorRuntimeNodeRecord> nodes,
    UiNodeId node_id,
    std::uint32_t node_count) {
    if (!node_id.IsValid()) {
        return nullptr;
    }

    for (std::uint32_t index = 0U; index < node_count; ++index) {
        const UiEditorRuntimeNodeRecord &record = nodes[index];
        if (record.node_id.value != node_id.value) {
            continue;
        }

        return &record;
    }

    return nullptr;
}

std::uint32_t CalculateDepth(
    std::span<const UiEditorRuntimeNodeRecord> nodes,
    const UiEditorRuntimeNodeRecord &record,
    std::uint32_t node_count) {
    std::uint32_t depth = 0U;
    UiNodeId parent_id = record.parent_id;
    while (parent_id.IsValid() && depth < node_count) {
        const UiEditorRuntimeNodeRecord *parent =
            FindDocumentNode(nodes, parent_id, node_count);
        if (parent == nullptr) {
            return depth;
        }

        parent_id = parent->parent_id;
        ++depth;
    }

    return depth;
}

UiNodeDesc BuildNodeDesc(const UiEditorRuntimeNodeRecord &record) {
    UiNodeDesc desc{};
    desc.node_id = record.node_id;
    desc.parent_id = record.parent_id;
    desc.rect_transform = record.rect_transform;
    desc.sibling_order = record.sibling_order;
    desc.layer = record.layer;
    desc.is_visible = record.visible;
    desc.is_enabled = record.enabled;
    desc.is_hit_testable = record.hit_testable;
    return desc;
}

UiEditorHierarchyRow BuildHierarchyRow(
    const UiEditorRuntimeDocumentHeader &header,
    std::span<const UiEditorRuntimeNodeRecord> nodes,
    const UiEditorRuntimeNodeRecord &source,
    const UiNodeRecord &resolved,
    UiNodeId selected_node_id) {
    UiEditorHierarchyRow row{};
    row.document_id = header.document_id;
    row.node_id = source.node_id;
    row.parent_id = source.parent_id;
    row.component_kind = source.component_kind;
    row.world_rect = resolved.world_rect;
    row.sibling_order = source.sibling_order;
    row.depth = CalculateDepth(nodes, source, header.node_count);
    row.layer = source.layer;
    row.selected =
        selected_node_id.IsValid() && selected_node_id.value == source.node_id.value;
    row.visible = source.visible;
    row.enabled = source.enabled;
    row.runtime_exported = source.runtime_exported;
    return row;
}

UiEditorPreviewFeedbackRecord BuildPreviewFeedback(
    const UiEditorRuntimeDocumentHeader &header,
    UiNodeId selected_node_id,
    const PreviewHostFrameResult &preview_frame) {
    UiEditorPreviewFeedbackRecord record{};
    record.document_id = header.document_id;
    record.selected_node_id = selected_node_id;
    record.frame_id = preview_frame.frame.frame_id;
    record.viewport_width = preview_frame.frame.width;
    record.viewport_height = preview_frame.frame.height;
    record.preview_status = preview_frame.status;
    record.frame_format = preview_frame.frame.format;
    record.diagnostic_count = preview_frame.diagnostic_count;
    record.capture_bytes_written = preview_frame.capture_bytes_written;
    record.preview_frame_built = preview_frame.submitted_render_scene_frame;
    record.headless_output = preview_frame.headless_output;
    record.feedback_from_preview_host = true;
    return record;
}

UiEditorSurfaceStatus ValidateDocument(
    const UiEditorRuntimeDocumentSurfaceRequest &request,
    UiEditorRuntimeDocumentSurfaceResult *result) {
    if (request.document == nullptr || result == nullptr) {
        return UiEditorSurfaceStatus::InvalidArgument;
    }

    const UiEditorRuntimeDocument &document = *request.document;
    result->document_id = document.header.document_id;
    result->runtime_node_count = document.header.node_count;
    if (!IsHeaderValid(document.header)) {
        return UiEditorSurfaceStatus::InvalidDocument;
    }

    if (!IsConstSpanStorageValid(document.nodes) ||
        document.nodes.size() < document.header.node_count) {
        return UiEditorSurfaceStatus::InvalidDocument;
    }

    for (std::uint32_t index = 0U; index < document.header.node_count; ++index) {
        const UiEditorRuntimeNodeRecord &record = document.nodes[index];
        if (!record.runtime_exported || !record.node_id.IsValid()) {
            return UiEditorSurfaceStatus::InvalidNode;
        }

        if (!IsComponentKindValid(record.component_kind)) {
            return UiEditorSurfaceStatus::InvalidNode;
        }
    }

    return UiEditorSurfaceStatus::Success;
}
}

UiEditorSurfaceStatus BuildUiEditorRuntimeDocumentSurface(
    const UiEditorRuntimeDocumentSurfaceRequest &request,
    UiEditorRuntimeDocumentSurfaceResult *out_result) {
    UiEditorRuntimeDocumentSurfaceResult result{};

    if (out_result == nullptr) {
        return UiEditorSurfaceStatus::InvalidArgument;
    }

    if (!IsSpanStorageValid(request.hierarchy_output) ||
        !IsSpanStorageValid(request.preview_feedback_output)) {
        result.status = UiEditorSurfaceStatus::InvalidArgument;
        result.blocked_layer = UiEditorSurfaceBlockedLayer::Output;
        *out_result = result;
        return result.status;
    }

    UiEditorSurfaceStatus status = ValidateDocument(request, &result);
    if (status != UiEditorSurfaceStatus::Success) {
        result.status = status;
        result.blocked_layer = UiEditorSurfaceBlockedLayer::RuntimeUiDocument;
        *out_result = result;
        return status;
    }

    const UiEditorRuntimeDocument &document = *request.document;
    result.consumed_runtime_ui_document = true;

    if (request.hierarchy_output.size() < document.header.node_count) {
        result.status = UiEditorSurfaceStatus::OutputCapacityExceeded;
        result.blocked_layer = UiEditorSurfaceBlockedLayer::Output;
        *out_result = result;
        return result.status;
    }

    const bool preview_feedback_requested =
        request.require_preview_feedback || !request.preview_feedback_output.empty();
    if (preview_feedback_requested && request.preview_feedback_output.empty()) {
        result.status = UiEditorSurfaceStatus::OutputCapacityExceeded;
        result.blocked_layer = UiEditorSurfaceBlockedLayer::Output;
        *out_result = result;
        return result.status;
    }

    if (request.selected_node_id.IsValid() &&
        FindDocumentNode(document.nodes, request.selected_node_id, document.header.node_count) ==
            nullptr) {
        result.status = UiEditorSurfaceStatus::MissingNode;
        result.blocked_layer = UiEditorSurfaceBlockedLayer::RuntimeUiDocument;
        *out_result = result;
        return result.status;
    }

    UiNodeTreeDesc tree_desc{};
    tree_desc.node_capacity = document.header.node_count;
    tree_desc.viewport_rect =
        UiRect{0.0F, 0.0F, document.header.viewport_width, document.header.viewport_height};
    UiNodeTree tree(tree_desc);

    for (std::uint32_t index = 0U; index < document.header.node_count; ++index) {
        const UiEditorRuntimeNodeRecord &record = document.nodes[index];
        const UiNodeTreeResult create_result = tree.CreateNode(BuildNodeDesc(record));
        if (!create_result.Succeeded()) {
            result.ui_core_status = create_result.status;
            result.status = MapUiCoreStatus(create_result.status);
            result.blocked_layer = UiEditorSurfaceBlockedLayer::UiCoreNodeTree;
            *out_result = result;
            return result.status;
        }
    }

    std::array<UiEditorHierarchyRow, MAX_UI_EDITOR_DOCUMENT_NODES> staged_rows{};
    for (std::uint32_t index = 0U; index < document.header.node_count; ++index) {
        const UiEditorRuntimeNodeRecord &record = document.nodes[index];
        const UiNodeTreeResult query_result = tree.QueryNode(record.node_id);
        if (!query_result.Succeeded()) {
            result.ui_core_status = query_result.status;
            result.status = MapUiCoreStatus(query_result.status);
            result.blocked_layer = UiEditorSurfaceBlockedLayer::UiCoreNodeTree;
            *out_result = result;
            return result.status;
        }

        staged_rows[index] =
            BuildHierarchyRow(
                document.header,
                document.nodes,
                record,
                query_result.record,
                request.selected_node_id);
    }

    UiEditorPreviewFeedbackRecord staged_preview{};
    std::size_t preview_feedback_count = 0U;
    if (preview_feedback_requested) {
        if (request.preview_frame == nullptr) {
            result.status = UiEditorSurfaceStatus::PreviewFeedbackMissing;
            result.blocked_layer = UiEditorSurfaceBlockedLayer::PreviewHostFeedback;
            *out_result = result;
            return result.status;
        }

        staged_preview =
            BuildPreviewFeedback(document.header, request.selected_node_id, *request.preview_frame);
        result.preview_status = request.preview_frame->status;
        result.consumed_preview_host_feedback = true;
        preview_feedback_count = 1U;
    }

    for (std::uint32_t index = 0U; index < document.header.node_count; ++index) {
        request.hierarchy_output[index] = staged_rows[index];
    }

    if (preview_feedback_count > 0U) {
        request.preview_feedback_output[0U] = staged_preview;
    }

    result.status = UiEditorSurfaceStatus::Success;
    result.blocked_layer = UiEditorSurfaceBlockedLayer::None;
    result.built_ui_node_tree = true;
    result.built_hierarchy_rows = true;
    result.hierarchy_row_count = document.header.node_count;
    result.preview_feedback_count = preview_feedback_count;
    result.emitted_preview_feedback = preview_feedback_count > 0U;
    *out_result = result;
    return result.status;
}
}
