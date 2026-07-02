// Module: YuEngine UiEditor
// File: Src/YuEngine/UiEditor/Src/UiEditorSurface.cpp

#include "YuEngine/UiEditor/UiEditorSurface.h"

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/File/FileReadRequest.h"
#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/FileWriteRequest.h"
#include "YuEngine/File/FileWriteResult.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeRecordId.h"
#include "YuEngine/Serialize/SerializeWriter.h"
#include "YuEngine/UiCore/UiNodeDesc.h"
#include "YuEngine/UiCore/UiNodeRecord.h"
#include "YuEngine/UiCore/UiNodeTree.h"
#include "YuEngine/UiCore/UiNodeTreeDesc.h"
#include "YuEngine/UiCore/UiNodeTreeResult.h"

namespace yuengine::uieditor {
namespace {
using PreviewHostFrameResult = yuengine::previewhost::PreviewHostFrameResult;
using PreviewHostStatus = yuengine::previewhost::PreviewHostStatus;
using FileReadRequest = yuengine::file::FileReadRequest;
using FileReadResult = yuengine::file::FileReadResult;
using FileStatus = yuengine::file::FileStatus;
using FileWriteRequest = yuengine::file::FileWriteRequest;
using FileWriteResult = yuengine::file::FileWriteResult;
using MountTable = yuengine::file::MountTable;
using SerializeFieldId = yuengine::serialize::SerializeFieldId;
using SerializeReader = yuengine::serialize::SerializeReader;
using SerializeRecordId = yuengine::serialize::SerializeRecordId;
using SerializeStatus = yuengine::serialize::SerializeStatus;
using SerializeWriter = yuengine::serialize::SerializeWriter;
using UiNodeDesc = yuengine::uicore::UiNodeDesc;
using UiNodeId = yuengine::uicore::UiNodeId;
using UiNodeRecord = yuengine::uicore::UiNodeRecord;
using UiNodeTree = yuengine::uicore::UiNodeTree;
using UiNodeTreeDesc = yuengine::uicore::UiNodeTreeDesc;
using UiNodeTreeResult = yuengine::uicore::UiNodeTreeResult;
using UiNodeTreeStatus = yuengine::uicore::UiNodeTreeStatus;
using UiRect = yuengine::uicore::UiRect;

constexpr std::uint32_t UI_EDITOR_SERIALIZATION_BUFFER_SIZE = 512U;
constexpr SerializeRecordId UI_EDITOR_SERIALIZATION_RECORD{1U};
constexpr SerializeFieldId FIELD_DOCUMENT_ID{1U};
constexpr SerializeFieldId FIELD_NODE_ID{2U};
constexpr SerializeFieldId FIELD_COMPONENT_KIND{3U};
constexpr SerializeFieldId FIELD_STYLE_KEY{4U};
constexpr SerializeFieldId FIELD_THEME_KEY{5U};
constexpr SerializeFieldId FIELD_TEMPLATE_KEY{6U};
constexpr SerializeFieldId FIELD_STATE_REVISION{7U};
constexpr SerializeFieldId FIELD_HOVERED{8U};
constexpr SerializeFieldId FIELD_FOCUSED{9U};
constexpr SerializeFieldId FIELD_PRESSED{10U};
constexpr SerializeFieldId FIELD_DISABLED{11U};
constexpr SerializeFieldId FIELD_RUNTIME_STATE_VALID{12U};
constexpr SerializeFieldId FIELD_STYLE_RESOLVED{13U};
constexpr SerializeFieldId FIELD_TEMPLATE_INSTANCED{14U};
constexpr SerializeFieldId FIELD_PREVIEW_FRAME_ID{15U};

struct SerializedStyleThemeTemplateValues final {
    std::uint32_t document_id = 0U;
    UiNodeId node_id{};
    UiEditorComponentKind component_kind = UiEditorComponentKind::Unknown;
    std::uint32_t style_key = 0U;
    std::uint32_t theme_key = 0U;
    std::uint32_t template_key = 0U;
    std::uint32_t state_revision = 0U;
    std::uint32_t preview_frame_id = 0U;
    bool hovered = false;
    bool focused = false;
    bool pressed = false;
    bool disabled = false;
    bool runtime_state_valid = false;
    bool style_resolved = false;
    bool template_instanced = false;
};

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

bool IsPreviewComponentKind(UiEditorComponentKind kind) {
    switch (kind) {
        case UiEditorComponentKind::Text:
        case UiEditorComponentKind::Image:
        case UiEditorComponentKind::Button:
        case UiEditorComponentKind::Slider:
            return true;
        case UiEditorComponentKind::Panel:
        case UiEditorComponentKind::Unknown:
            break;
    }

    return false;
}

void AddPreviewComponentCount(
    UiEditorPreviewFeedbackRecord *record,
    UiEditorComponentKind kind) {
    if (record == nullptr) {
        return;
    }

    if (kind == UiEditorComponentKind::Text) {
        ++record->text_component_count;
    }

    if (kind == UiEditorComponentKind::Image) {
        ++record->image_component_count;
    }

    if (kind == UiEditorComponentKind::Button) {
        ++record->button_component_count;
    }

    if (kind == UiEditorComponentKind::Slider) {
        ++record->slider_component_count;
    }
}

std::size_t CountPreviewComponentKinds(
    const UiEditorPreviewFeedbackRecord &record) {
    std::size_t count = 0U;
    if (record.text_component_count > 0U) {
        ++count;
    }

    if (record.image_component_count > 0U) {
        ++count;
    }

    if (record.button_component_count > 0U) {
        ++count;
    }

    if (record.slider_component_count > 0U) {
        ++count;
    }

    return count;
}

void FillPreviewComponentCounts(
    std::span<const UiEditorRuntimeNodeRecord> nodes,
    std::uint32_t node_count,
    UiEditorPreviewFeedbackRecord *record) {
    if (record == nullptr) {
        return;
    }

    for (std::uint32_t index = 0U; index < node_count; ++index) {
        const UiEditorRuntimeNodeRecord &node = nodes[index];
        if (!node.visible || !node.runtime_exported) {
            continue;
        }

        if (!IsPreviewComponentKind(node.component_kind)) {
            continue;
        }

        ++record->preview_component_count;
        AddPreviewComponentCount(record, node.component_kind);
    }

    record->preview_component_kind_count = CountPreviewComponentKinds(*record);
    record->multi_component_preview_layout =
        record->preview_component_kind_count >= 2U &&
        record->preview_frame_built;
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
    std::span<const UiEditorRuntimeNodeRecord> nodes,
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
    FillPreviewComponentCounts(nodes, header.node_count, &record);
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

UiEditorDesignWorkflowStatus MapDesignWorkflowStatus(UiEditorSurfaceStatus status) {
    if (status == UiEditorSurfaceStatus::Success) {
        return UiEditorDesignWorkflowStatus::Success;
    }

    if (status == UiEditorSurfaceStatus::InvalidDocument) {
        return UiEditorDesignWorkflowStatus::InvalidDocument;
    }

    if (status == UiEditorSurfaceStatus::InvalidNode) {
        return UiEditorDesignWorkflowStatus::InvalidNode;
    }

    if (status == UiEditorSurfaceStatus::MissingNode) {
        return UiEditorDesignWorkflowStatus::MissingNode;
    }

    if (status == UiEditorSurfaceStatus::DuplicateNode) {
        return UiEditorDesignWorkflowStatus::DuplicateNode;
    }

    if (status == UiEditorSurfaceStatus::OutputCapacityExceeded) {
        return UiEditorDesignWorkflowStatus::OutputCapacityExceeded;
    }

    if (status == UiEditorSurfaceStatus::PreviewFeedbackMissing) {
        return UiEditorDesignWorkflowStatus::PreviewFeedbackMissing;
    }

    if (status == UiEditorSurfaceStatus::UiCoreFailed) {
        return UiEditorDesignWorkflowStatus::UiCoreFailed;
    }

    return UiEditorDesignWorkflowStatus::InvalidArgument;
}

UiEditorDesignWorkflowBlockedLayer MapDesignWorkflowLayer(
    UiEditorSurfaceBlockedLayer layer) {
    if (layer == UiEditorSurfaceBlockedLayer::None) {
        return UiEditorDesignWorkflowBlockedLayer::None;
    }

    if (layer == UiEditorSurfaceBlockedLayer::RuntimeUiDocument) {
        return UiEditorDesignWorkflowBlockedLayer::RuntimeUiDocument;
    }

    if (layer == UiEditorSurfaceBlockedLayer::UiCoreNodeTree) {
        return UiEditorDesignWorkflowBlockedLayer::UiCoreNodeTree;
    }

    if (layer == UiEditorSurfaceBlockedLayer::PreviewHostFeedback) {
        return UiEditorDesignWorkflowBlockedLayer::PreviewHostFeedback;
    }

    return UiEditorDesignWorkflowBlockedLayer::Output;
}

bool DesignWorkflowStorageValid(
    const UiEditorDesignInspectorWorkflowRequest &request) {
    return IsSpanStorageValid(request.hierarchy_output) &&
        IsSpanStorageValid(request.design_surface_output) &&
        IsSpanStorageValid(request.inspector_output) &&
        IsSpanStorageValid(request.preview_feedback_output) &&
        IsSpanStorageValid(request.staged_document_output) &&
        IsSpanStorageValid(request.command_ledger_output);
}

bool DesignWorkflowOutputCapacityReady(
    const UiEditorDesignInspectorWorkflowRequest &request,
    std::size_t node_count) {
    if (request.hierarchy_output.size() < node_count) {
        return false;
    }

    if (request.design_surface_output.size() < node_count) {
        return false;
    }

    if (request.inspector_output.size() < UI_EDITOR_INSPECTOR_FIELD_COUNT) {
        return false;
    }

    if (request.preview_feedback_output.empty()) {
        return false;
    }

    if (request.staged_document_output.size() < node_count) {
        return false;
    }

    if (request.command_ledger_output.empty()) {
        return false;
    }

    return true;
}

bool PreviewFeedbackReady(const PreviewHostFrameResult *preview_frame) {
    if (preview_frame == nullptr) {
        return false;
    }

    return preview_frame->status == PreviewHostStatus::Success &&
        preview_frame->submitted_render_scene_frame;
}

std::uint32_t FindDocumentNodeIndex(
    std::span<const UiEditorRuntimeNodeRecord> nodes,
    UiNodeId node_id,
    std::uint32_t node_count) {
    std::uint32_t index = 0U;
    while (index < node_count) {
        if (nodes[index].node_id.value == node_id.value) {
            return index;
        }

        ++index;
    }

    return node_count;
}

UiEditorDesignSurfaceRow BuildDesignSurfaceRow(
    const UiEditorHierarchyRow &hierarchy_row,
    const UiEditorRuntimeNodeRecord &node,
    const UiEditorPreviewFeedbackRecord &preview_feedback) {
    UiEditorDesignSurfaceRow row{};
    row.document_id = hierarchy_row.document_id;
    row.node_id = hierarchy_row.node_id;
    row.parent_id = hierarchy_row.parent_id;
    row.component_kind = hierarchy_row.component_kind;
    row.world_rect = hierarchy_row.world_rect;
    row.sibling_order = hierarchy_row.sibling_order;
    row.depth = hierarchy_row.depth;
    row.layer = hierarchy_row.layer;
    row.preview_frame_id = preview_feedback.frame_id;
    row.preview_status = preview_feedback.preview_status;
    row.selected = hierarchy_row.selected;
    row.visible = hierarchy_row.visible;
    row.enabled = hierarchy_row.enabled;
    row.hit_testable = node.hit_testable;
    row.runtime_exported = hierarchy_row.runtime_exported;
    row.preview_feedback_available = preview_feedback.feedback_from_preview_host;
    return row;
}

UiEditorInspectorFieldRow InspectorFieldBase(
    const UiEditorRuntimeDocumentHeader &header,
    const UiEditorRuntimeNodeRecord &node,
    UiEditorInspectorFieldKind kind) {
    UiEditorInspectorFieldRow row{};
    row.document_id = header.document_id;
    row.node_id = node.node_id;
    row.field_kind = kind;
    row.component_kind = node.component_kind;
    row.selected_node = true;
    row.editable = true;
    return row;
}

std::size_t BuildInspectorFields(
    const UiEditorRuntimeDocumentHeader &header,
    const UiEditorRuntimeNodeRecord &node,
    std::span<UiEditorInspectorFieldRow> output) {
    output[0U] =
        InspectorFieldBase(header, node, UiEditorInspectorFieldKind::ComponentKind);
    output[0U].int_value = static_cast<std::int32_t>(node.component_kind);
    output[1U] = InspectorFieldBase(header, node, UiEditorInspectorFieldKind::Visible);
    output[1U].bool_value = node.visible;
    output[2U] = InspectorFieldBase(header, node, UiEditorInspectorFieldKind::Enabled);
    output[2U].bool_value = node.enabled;
    output[3U] =
        InspectorFieldBase(header, node, UiEditorInspectorFieldKind::HitTestable);
    output[3U].bool_value = node.hit_testable;
    output[4U] =
        InspectorFieldBase(header, node, UiEditorInspectorFieldKind::RuntimeExported);
    output[4U].bool_value = node.runtime_exported;
    output[5U] = InspectorFieldBase(header, node, UiEditorInspectorFieldKind::Layer);
    output[5U].int_value = node.layer;
    output[6U] =
        InspectorFieldBase(header, node, UiEditorInspectorFieldKind::RectTransform);
    output[6U].rect_transform = node.rect_transform;
    return UI_EDITOR_INSPECTOR_FIELD_COUNT;
}

bool IsDesignCommandSupported(UiEditorDesignCommandKind kind) {
    switch (kind) {
        case UiEditorDesignCommandKind::None:
        case UiEditorDesignCommandKind::SetVisible:
        case UiEditorDesignCommandKind::SetEnabled:
        case UiEditorDesignCommandKind::SetHitTestable:
        case UiEditorDesignCommandKind::SetLayer:
        case UiEditorDesignCommandKind::SetRectTransform:
            return true;
    }

    return false;
}

UiEditorDesignCommandLedgerRecord BuildCommandLedger(
    const UiEditorRuntimeDocumentHeader &header,
    const UiEditorRuntimeNodeRecord &before,
    const UiEditorRuntimeNodeRecord &after,
    const UiEditorDesignCommand &command) {
    UiEditorDesignCommandLedgerRecord record{};
    record.document_id = header.document_id;
    record.node_id = before.node_id;
    record.command_kind = command.kind;
    record.command_sequence = command.command_sequence;
    record.component_kind = before.component_kind;
    record.before_rect_transform = before.rect_transform;
    record.after_rect_transform = after.rect_transform;
    record.before_layer = before.layer;
    record.after_layer = after.layer;
    record.before_visible = before.visible;
    record.after_visible = after.visible;
    record.before_enabled = before.enabled;
    record.after_enabled = after.enabled;
    record.before_hit_testable = before.hit_testable;
    record.after_hit_testable = after.hit_testable;
    record.staged_document_update =
        command.kind != UiEditorDesignCommandKind::None;
    record.command_applied = record.staged_document_update;
    return record;
}

UiEditorRuntimeNodeRecord ApplyDesignCommand(
    const UiEditorRuntimeNodeRecord &node,
    const UiEditorDesignCommand &command) {
    UiEditorRuntimeNodeRecord updated = node;
    if (command.kind == UiEditorDesignCommandKind::SetVisible) {
        updated.visible = command.bool_value;
    }

    if (command.kind == UiEditorDesignCommandKind::SetEnabled) {
        updated.enabled = command.bool_value;
    }

    if (command.kind == UiEditorDesignCommandKind::SetHitTestable) {
        updated.hit_testable = command.bool_value;
    }

    if (command.kind == UiEditorDesignCommandKind::SetLayer) {
        updated.layer = command.int_value;
    }

    if (command.kind == UiEditorDesignCommandKind::SetRectTransform) {
        updated.rect_transform = command.rect_transform;
    }

    return updated;
}

UiEditorRuntimePreviewWorkflowStatus MapRuntimePreviewWorkflowStatus(
    UiEditorDesignWorkflowStatus status) {
    if (status == UiEditorDesignWorkflowStatus::Success) {
        return UiEditorRuntimePreviewWorkflowStatus::Success;
    }

    if (status == UiEditorDesignWorkflowStatus::InvalidDocument) {
        return UiEditorRuntimePreviewWorkflowStatus::InvalidDocument;
    }

    if (status == UiEditorDesignWorkflowStatus::InvalidNode) {
        return UiEditorRuntimePreviewWorkflowStatus::InvalidNode;
    }

    if (status == UiEditorDesignWorkflowStatus::MissingNode) {
        return UiEditorRuntimePreviewWorkflowStatus::MissingNode;
    }

    if (status == UiEditorDesignWorkflowStatus::DuplicateNode) {
        return UiEditorRuntimePreviewWorkflowStatus::DuplicateNode;
    }

    if (status == UiEditorDesignWorkflowStatus::OutputCapacityExceeded) {
        return UiEditorRuntimePreviewWorkflowStatus::OutputCapacityExceeded;
    }

    if (status == UiEditorDesignWorkflowStatus::PreviewFeedbackMissing) {
        return UiEditorRuntimePreviewWorkflowStatus::PreviewFeedbackMissing;
    }

    if (status == UiEditorDesignWorkflowStatus::UiCoreFailed) {
        return UiEditorRuntimePreviewWorkflowStatus::UiCoreFailed;
    }

    if (status == UiEditorDesignWorkflowStatus::CommandFailed) {
        return UiEditorRuntimePreviewWorkflowStatus::DesignCommandFailed;
    }

    return UiEditorRuntimePreviewWorkflowStatus::InvalidArgument;
}

UiEditorRuntimePreviewWorkflowBlockedLayer MapRuntimePreviewWorkflowLayer(
    UiEditorDesignWorkflowBlockedLayer layer) {
    if (layer == UiEditorDesignWorkflowBlockedLayer::None) {
        return UiEditorRuntimePreviewWorkflowBlockedLayer::None;
    }

    if (layer == UiEditorDesignWorkflowBlockedLayer::RuntimeUiDocument) {
        return UiEditorRuntimePreviewWorkflowBlockedLayer::RuntimeUiDocument;
    }

    if (layer == UiEditorDesignWorkflowBlockedLayer::UiCoreNodeTree) {
        return UiEditorRuntimePreviewWorkflowBlockedLayer::UiCoreNodeTree;
    }

    if (layer == UiEditorDesignWorkflowBlockedLayer::PreviewHostFeedback) {
        return UiEditorRuntimePreviewWorkflowBlockedLayer::PreviewHostFeedback;
    }

    if (layer == UiEditorDesignWorkflowBlockedLayer::Output) {
        return UiEditorRuntimePreviewWorkflowBlockedLayer::Output;
    }

    return UiEditorRuntimePreviewWorkflowBlockedLayer::DesignWorkflow;
}

bool RuntimePreviewWorkflowStorageValid(
    const UiEditorRuntimePreviewWorkflowRequest &request) {
    return IsConstSpanStorageValid(request.style_template_state_records) &&
        IsSpanStorageValid(request.hierarchy_output) &&
        IsSpanStorageValid(request.design_surface_output) &&
        IsSpanStorageValid(request.inspector_output) &&
        IsSpanStorageValid(request.preview_feedback_output) &&
        IsSpanStorageValid(request.staged_document_output) &&
        IsSpanStorageValid(request.command_ledger_output) &&
        IsSpanStorageValid(request.runtime_preview_output) &&
        IsSpanStorageValid(request.style_template_state_ledger_output);
}

bool RuntimePreviewWorkflowOutputCapacityReady(
    const UiEditorRuntimePreviewWorkflowRequest &request,
    std::size_t node_count) {
    if (request.hierarchy_output.size() < node_count) {
        return false;
    }

    if (request.design_surface_output.size() < node_count) {
        return false;
    }

    if (request.inspector_output.size() < UI_EDITOR_INSPECTOR_FIELD_COUNT) {
        return false;
    }

    if (request.preview_feedback_output.empty()) {
        return false;
    }

    if (request.staged_document_output.size() < node_count) {
        return false;
    }

    if (request.command_ledger_output.empty()) {
        return false;
    }

    if (request.runtime_preview_output.empty()) {
        return false;
    }

    if (request.style_template_state_ledger_output.empty()) {
        return false;
    }

    return true;
}

const UiEditorStyleTemplateStateRecord *FindStyleTemplateStateRecord(
    std::span<const UiEditorStyleTemplateStateRecord> records,
    UiNodeId node_id) {
    if (!node_id.IsValid()) {
        return nullptr;
    }

    for (const UiEditorStyleTemplateStateRecord &record : records) {
        if (record.node_id.value == node_id.value) {
            return &record;
        }
    }

    return nullptr;
}

bool IsStyleTemplateStateRecordValid(
    const UiEditorStyleTemplateStateRecord &record,
    UiEditorComponentKind component_kind) {
    if (!record.node_id.IsValid()) {
        return false;
    }

    if (!IsComponentKindValid(record.component_kind)) {
        return false;
    }

    if (record.component_kind != component_kind) {
        return false;
    }

    if (record.style_key == 0U || record.theme_key == 0U ||
        record.template_key == 0U || record.state_revision == 0U) {
        return false;
    }

    if (!record.runtime_state_valid || !record.style_resolved ||
        !record.template_instanced) {
        return false;
    }

    return true;
}

bool IsStyleTemplateStateCommandSupported(
    const UiEditorStyleTemplateStateCommand &command) {
    if (command.kind == UiEditorStyleTemplateStateCommandKind::None) {
        return true;
    }

    if (command.kind == UiEditorStyleTemplateStateCommandKind::SetStyleKey) {
        return command.style_key != 0U;
    }

    if (command.kind == UiEditorStyleTemplateStateCommandKind::SetThemeKey) {
        return command.theme_key != 0U;
    }

    if (command.kind == UiEditorStyleTemplateStateCommandKind::SetTemplateKey) {
        return command.template_key != 0U;
    }

    if (command.kind ==
        UiEditorStyleTemplateStateCommandKind::SetInteractionState) {
        return command.state_revision != 0U;
    }

    return false;
}

UiEditorStyleTemplateStateRecord ApplyStyleTemplateStateCommand(
    const UiEditorStyleTemplateStateRecord &record,
    const UiEditorStyleTemplateStateCommand &command) {
    UiEditorStyleTemplateStateRecord updated = record;
    if (command.kind == UiEditorStyleTemplateStateCommandKind::SetStyleKey) {
        updated.style_key = command.style_key;
    }

    if (command.kind == UiEditorStyleTemplateStateCommandKind::SetThemeKey) {
        updated.theme_key = command.theme_key;
    }

    if (command.kind == UiEditorStyleTemplateStateCommandKind::SetTemplateKey) {
        updated.template_key = command.template_key;
    }

    if (command.kind ==
        UiEditorStyleTemplateStateCommandKind::SetInteractionState) {
        updated.state_revision = command.state_revision;
        updated.hovered = command.hovered;
        updated.focused = command.focused;
        updated.pressed = command.pressed;
        updated.disabled = command.disabled;
    }

    return updated;
}

UiEditorRuntimePreviewStyleTemplateStateRow BuildRuntimePreviewStyleRow(
    const UiEditorRuntimeDocumentHeader &header,
    const UiEditorDesignSurfaceRow &design_row,
    const UiEditorStyleTemplateStateRecord &record,
    const UiEditorPreviewFeedbackRecord &preview_feedback) {
    UiEditorRuntimePreviewStyleTemplateStateRow row{};
    row.document_id = header.document_id;
    row.node_id = record.node_id;
    row.component_kind = record.component_kind;
    row.world_rect = design_row.world_rect;
    row.style_key = record.style_key;
    row.theme_key = record.theme_key;
    row.template_key = record.template_key;
    row.state_revision = record.state_revision;
    row.preview_frame_id = preview_feedback.frame_id;
    row.preview_status = preview_feedback.preview_status;
    row.selected = design_row.selected;
    row.hovered = record.hovered;
    row.focused = record.focused;
    row.pressed = record.pressed;
    row.disabled = record.disabled;
    row.runtime_state_valid = record.runtime_state_valid;
    row.style_resolved = record.style_resolved;
    row.template_instanced = record.template_instanced;
    row.engine_runtime_preview =
        preview_feedback.feedback_from_preview_host &&
        preview_feedback.preview_frame_built;
    row.preview_feedback_available = preview_feedback.feedback_from_preview_host;
    row.editable = true;
    return row;
}

UiEditorStyleTemplateStateLedgerRecord BuildStyleTemplateStateLedger(
    const UiEditorRuntimeDocumentHeader &header,
    const UiEditorStyleTemplateStateRecord &before,
    const UiEditorStyleTemplateStateRecord &after,
    const UiEditorStyleTemplateStateCommand &command) {
    UiEditorStyleTemplateStateLedgerRecord record{};
    record.document_id = header.document_id;
    record.node_id = before.node_id;
    record.command_kind = command.kind;
    record.command_sequence = command.command_sequence;
    record.before_style_key = before.style_key;
    record.after_style_key = after.style_key;
    record.before_theme_key = before.theme_key;
    record.after_theme_key = after.theme_key;
    record.before_template_key = before.template_key;
    record.after_template_key = after.template_key;
    record.before_state_revision = before.state_revision;
    record.after_state_revision = after.state_revision;
    record.before_hovered = before.hovered;
    record.after_hovered = after.hovered;
    record.before_focused = before.focused;
    record.after_focused = after.focused;
    record.before_pressed = before.pressed;
    record.after_pressed = after.pressed;
    record.before_disabled = before.disabled;
    record.after_disabled = after.disabled;
    record.staged_style_template_state_update =
        command.kind != UiEditorStyleTemplateStateCommandKind::None;
    record.command_applied = record.staged_style_template_state_update;
    return record;
}

UiEditorStyleThemeTemplateSerializationBlockedLayer MapSerializationWorkflowLayer(
    UiEditorRuntimePreviewWorkflowBlockedLayer layer) {
    if (layer == UiEditorRuntimePreviewWorkflowBlockedLayer::None) {
        return UiEditorStyleThemeTemplateSerializationBlockedLayer::None;
    }

    if (layer == UiEditorRuntimePreviewWorkflowBlockedLayer::RuntimeUiDocument) {
        return UiEditorStyleThemeTemplateSerializationBlockedLayer::RuntimeUiDocument;
    }

    if (layer == UiEditorRuntimePreviewWorkflowBlockedLayer::UiCoreNodeTree) {
        return UiEditorStyleThemeTemplateSerializationBlockedLayer::UiCoreNodeTree;
    }

    if (layer == UiEditorRuntimePreviewWorkflowBlockedLayer::PreviewHostFeedback) {
        return UiEditorStyleThemeTemplateSerializationBlockedLayer::PreviewHostFeedback;
    }

    if (layer == UiEditorRuntimePreviewWorkflowBlockedLayer::StyleTemplateState) {
        return UiEditorStyleThemeTemplateSerializationBlockedLayer::StyleThemeTemplate;
    }

    if (layer == UiEditorRuntimePreviewWorkflowBlockedLayer::Output) {
        return UiEditorStyleThemeTemplateSerializationBlockedLayer::Output;
    }

    return UiEditorStyleThemeTemplateSerializationBlockedLayer::DesignWorkflow;
}

bool SerializationWorkflowStorageValid(
    const UiEditorStyleThemeTemplateSerializationWorkflowRequest &request) {
    return IsConstSpanStorageValid(request.style_template_state_records) &&
        IsSpanStorageValid(request.hierarchy_output) &&
        IsSpanStorageValid(request.design_surface_output) &&
        IsSpanStorageValid(request.inspector_output) &&
        IsSpanStorageValid(request.preview_feedback_output) &&
        IsSpanStorageValid(request.staged_document_output) &&
        IsSpanStorageValid(request.command_ledger_output) &&
        IsSpanStorageValid(request.runtime_preview_output) &&
        IsSpanStorageValid(request.style_template_state_ledger_output) &&
        IsSpanStorageValid(request.serialization_output);
}

bool SerializationWorkflowOutputCapacityReady(
    const UiEditorStyleThemeTemplateSerializationWorkflowRequest &request,
    std::size_t node_count) {
    if (request.hierarchy_output.size() < node_count) {
        return false;
    }

    if (request.design_surface_output.size() < node_count) {
        return false;
    }

    if (request.inspector_output.size() < UI_EDITOR_INSPECTOR_FIELD_COUNT) {
        return false;
    }

    if (request.preview_feedback_output.empty()) {
        return false;
    }

    if (request.staged_document_output.size() < node_count) {
        return false;
    }

    if (request.command_ledger_output.empty()) {
        return false;
    }

    if (request.runtime_preview_output.empty()) {
        return false;
    }

    if (request.style_template_state_ledger_output.empty()) {
        return false;
    }

    if (request.serialization_output.empty()) {
        return false;
    }

    return true;
}

void SetSurfaceRequiredCounts(
    UiEditorRuntimeDocumentSurfaceResult *result,
    std::size_t node_count,
    bool preview_feedback_requested) {
    if (result == nullptr) {
        return;
    }

    result->hierarchy_row_count = node_count;
    if (preview_feedback_requested) {
        result->preview_feedback_count = 1U;
    }
}

void SetDesignWorkflowRequiredCounts(
    UiEditorDesignInspectorWorkflowResult *result,
    std::size_t node_count) {
    if (result == nullptr) {
        return;
    }

    result->hierarchy_row_count = node_count;
    result->design_surface_row_count = node_count;
    result->inspector_field_count = UI_EDITOR_INSPECTOR_FIELD_COUNT;
    result->preview_feedback_count = 1U;
    result->staged_node_count = node_count;
    result->command_ledger_count = 1U;
}

void SetRuntimePreviewWorkflowRequiredCounts(
    UiEditorRuntimePreviewWorkflowResult *result,
    const UiEditorDesignInspectorWorkflowResult &design_result) {
    if (result == nullptr) {
        return;
    }

    result->hierarchy_row_count = design_result.hierarchy_row_count;
    result->design_surface_row_count = design_result.design_surface_row_count;
    result->inspector_field_count = design_result.inspector_field_count;
    result->preview_feedback_count = design_result.preview_feedback_count;
    result->staged_node_count = design_result.staged_node_count;
    result->command_ledger_count = design_result.command_ledger_count;
    result->runtime_preview_row_count = 1U;
    result->style_template_state_ledger_count = 1U;
}

void SetSerializationWorkflowRequiredCounts(
    UiEditorStyleThemeTemplateSerializationWorkflowResult *result,
    const UiEditorRuntimePreviewWorkflowResult &runtime_preview_result) {
    if (result == nullptr) {
        return;
    }

    result->hierarchy_row_count = runtime_preview_result.hierarchy_row_count;
    result->design_surface_row_count =
        runtime_preview_result.design_surface_row_count;
    result->inspector_field_count = runtime_preview_result.inspector_field_count;
    result->preview_feedback_count = runtime_preview_result.preview_feedback_count;
    result->staged_node_count = runtime_preview_result.staged_node_count;
    result->command_ledger_count = runtime_preview_result.command_ledger_count;
    result->runtime_preview_row_count =
        runtime_preview_result.runtime_preview_row_count;
    result->style_template_state_ledger_count =
        runtime_preview_result.style_template_state_ledger_count;
    result->serialization_row_count = 1U;
}

bool FileVfsRequestAvailable(
    const UiEditorStyleThemeTemplateSerializationWorkflowRequest &request) {
    if (request.file_mount_table == nullptr) {
        return false;
    }

    if (!request.file_mount.IsValid()) {
        return false;
    }

    return request.file_path.ByteLength() > 0U;
}

std::uint32_t BoolFieldValue(bool value) {
    if (value) {
        return 1U;
    }

    return 0U;
}

bool BoolFromFieldValue(std::uint32_t value) {
    return value != 0U;
}

SerializedStyleThemeTemplateValues ValuesFromRuntimePreviewRow(
    const UiEditorRuntimePreviewStyleTemplateStateRow &row) {
    SerializedStyleThemeTemplateValues values{};
    values.document_id = row.document_id;
    values.node_id = row.node_id;
    values.component_kind = row.component_kind;
    values.style_key = row.style_key;
    values.theme_key = row.theme_key;
    values.template_key = row.template_key;
    values.state_revision = row.state_revision;
    values.preview_frame_id = row.preview_frame_id;
    values.hovered = row.hovered;
    values.focused = row.focused;
    values.pressed = row.pressed;
    values.disabled = row.disabled;
    values.runtime_state_valid = row.runtime_state_valid;
    values.style_resolved = row.style_resolved;
    values.template_instanced = row.template_instanced;
    return values;
}

SerializeStatus WriteStyleThemeTemplateStream(
    const SerializedStyleThemeTemplateValues &values,
    std::span<std::uint8_t> output_bytes,
    std::uint32_t *out_byte_count) {
    if (out_byte_count == nullptr) {
        return SerializeStatus::InvalidArgument;
    }

    *out_byte_count = 0U;
    SerializeWriter writer(output_bytes.data(), static_cast<std::uint32_t>(output_bytes.size()));
    SerializeStatus status = writer.BeginStream();
    if (status == SerializeStatus::Success) {
        status = writer.BeginRecord(UI_EDITOR_SERIALIZATION_RECORD);
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(FIELD_DOCUMENT_ID, values.document_id);
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(FIELD_NODE_ID, values.node_id.value);
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(
            FIELD_COMPONENT_KIND,
            static_cast<std::uint32_t>(values.component_kind));
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(FIELD_STYLE_KEY, values.style_key);
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(FIELD_THEME_KEY, values.theme_key);
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(FIELD_TEMPLATE_KEY, values.template_key);
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(FIELD_STATE_REVISION, values.state_revision);
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(FIELD_HOVERED, BoolFieldValue(values.hovered));
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(FIELD_FOCUSED, BoolFieldValue(values.focused));
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(FIELD_PRESSED, BoolFieldValue(values.pressed));
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(FIELD_DISABLED, BoolFieldValue(values.disabled));
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(
            FIELD_RUNTIME_STATE_VALID,
            BoolFieldValue(values.runtime_state_valid));
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(
            FIELD_STYLE_RESOLVED,
            BoolFieldValue(values.style_resolved));
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(
            FIELD_TEMPLATE_INSTANCED,
            BoolFieldValue(values.template_instanced));
    }

    if (status == SerializeStatus::Success) {
        status = writer.WriteUInt32(FIELD_PREVIEW_FRAME_ID, values.preview_frame_id);
    }

    if (status == SerializeStatus::Success) {
        *out_byte_count = writer.Snapshot().committed_byte_count;
    }

    return status;
}

SerializeStatus ReadUInt32Field(
    SerializeReader &reader,
    SerializeFieldId field,
    std::uint32_t *out_value) {
    if (out_value == nullptr) {
        return SerializeStatus::InvalidArgument;
    }

    return reader.ReadUInt32(UI_EDITOR_SERIALIZATION_RECORD, field, *out_value);
}

SerializeStatus ReadStyleThemeTemplateStream(
    std::span<const std::uint8_t> bytes,
    SerializedStyleThemeTemplateValues *out_values) {
    if (out_values == nullptr) {
        return SerializeStatus::InvalidArgument;
    }

    SerializedStyleThemeTemplateValues values{};
    SerializeReader reader(bytes.data(), static_cast<std::uint32_t>(bytes.size()));
    SerializeStatus status = reader.OpenStream();
    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_DOCUMENT_ID, &values.document_id);
    }

    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_NODE_ID, &values.node_id.value);
    }

    std::uint32_t component_kind = 0U;
    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_COMPONENT_KIND, &component_kind);
    }

    if (status == SerializeStatus::Success) {
        values.component_kind = static_cast<UiEditorComponentKind>(component_kind);
        status = ReadUInt32Field(reader, FIELD_STYLE_KEY, &values.style_key);
    }

    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_THEME_KEY, &values.theme_key);
    }

    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_TEMPLATE_KEY, &values.template_key);
    }

    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_STATE_REVISION, &values.state_revision);
    }

    std::uint32_t bool_value = 0U;
    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_HOVERED, &bool_value);
        values.hovered = BoolFromFieldValue(bool_value);
    }

    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_FOCUSED, &bool_value);
        values.focused = BoolFromFieldValue(bool_value);
    }

    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_PRESSED, &bool_value);
        values.pressed = BoolFromFieldValue(bool_value);
    }

    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_DISABLED, &bool_value);
        values.disabled = BoolFromFieldValue(bool_value);
    }

    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_RUNTIME_STATE_VALID, &bool_value);
        values.runtime_state_valid = BoolFromFieldValue(bool_value);
    }

    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_STYLE_RESOLVED, &bool_value);
        values.style_resolved = BoolFromFieldValue(bool_value);
    }

    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_TEMPLATE_INSTANCED, &bool_value);
        values.template_instanced = BoolFromFieldValue(bool_value);
    }

    if (status == SerializeStatus::Success) {
        status = ReadUInt32Field(reader, FIELD_PREVIEW_FRAME_ID, &values.preview_frame_id);
    }

    if (status == SerializeStatus::Success) {
        *out_values = values;
    }

    return status;
}

bool SerializedValuesMatch(
    const SerializedStyleThemeTemplateValues &left,
    const SerializedStyleThemeTemplateValues &right) {
    if (left.document_id != right.document_id) {
        return false;
    }

    if (left.node_id.value != right.node_id.value) {
        return false;
    }

    if (left.component_kind != right.component_kind) {
        return false;
    }

    if (left.style_key != right.style_key || left.theme_key != right.theme_key ||
        left.template_key != right.template_key ||
        left.state_revision != right.state_revision) {
        return false;
    }

    if (left.hovered != right.hovered || left.focused != right.focused ||
        left.pressed != right.pressed || left.disabled != right.disabled) {
        return false;
    }

    if (left.runtime_state_valid != right.runtime_state_valid ||
        left.style_resolved != right.style_resolved ||
        left.template_instanced != right.template_instanced) {
        return false;
    }

    return left.preview_frame_id == right.preview_frame_id;
}

UiEditorStyleThemeTemplateSerializationRow BuildSerializationBoundaryRow(
    const SerializedStyleThemeTemplateValues &source,
    const SerializedStyleThemeTemplateValues &loaded,
    std::size_t serialized_byte_count,
    std::size_t saved_byte_count,
    std::size_t loaded_byte_count) {
    UiEditorStyleThemeTemplateSerializationRow row{};
    row.document_id = source.document_id;
    row.node_id = source.node_id;
    row.component_kind = source.component_kind;
    row.source_style_key = source.style_key;
    row.source_theme_key = source.theme_key;
    row.source_template_key = source.template_key;
    row.source_state_revision = source.state_revision;
    row.runtime_asset_style_key = loaded.style_key;
    row.runtime_asset_theme_key = loaded.theme_key;
    row.runtime_asset_template_key = loaded.template_key;
    row.runtime_asset_state_revision = loaded.state_revision;
    row.ui_runtime_style_key = loaded.style_key;
    row.ui_runtime_theme_key = loaded.theme_key;
    row.ui_runtime_template_key = loaded.template_key;
    row.ui_runtime_state_revision = loaded.state_revision;
    row.serialized_byte_count = serialized_byte_count;
    row.saved_byte_count = saved_byte_count;
    row.loaded_byte_count = loaded_byte_count;
    row.saved_through_file_vfs = true;
    row.loaded_through_file_vfs = true;
    row.serialized_runtime_ui_data = true;
    row.deserialized_runtime_ui_data = true;
    row.runtime_asset_boundary_preserved = SerializedValuesMatch(source, loaded);
    row.ui_runtime_boundary_preserved = row.runtime_asset_boundary_preserved;
    row.component_template_state_provenance_preserved =
        source.component_kind == loaded.component_kind &&
        source.template_key == loaded.template_key &&
        source.state_revision == loaded.state_revision;
    return row;
}

void CopySerializationWorkflowOutputs(
    const UiEditorStyleThemeTemplateSerializationWorkflowRequest &request,
    std::span<const UiEditorHierarchyRow> staged_hierarchy,
    std::span<const UiEditorDesignSurfaceRow> staged_design,
    std::span<const UiEditorInspectorFieldRow> staged_inspector,
    std::span<const UiEditorRuntimeNodeRecord> staged_nodes,
    const UiEditorPreviewFeedbackRecord &staged_preview,
    const UiEditorDesignCommandLedgerRecord &staged_design_ledger,
    const UiEditorRuntimePreviewStyleTemplateStateRow &runtime_preview_row,
    const UiEditorStyleTemplateStateLedgerRecord &style_ledger,
    const UiEditorStyleThemeTemplateSerializationRow &serialization_row,
    std::size_t node_count,
    std::size_t inspector_count) {
    std::size_t node_index = 0U;
    while (node_index < node_count) {
        request.hierarchy_output[node_index] = staged_hierarchy[node_index];
        request.design_surface_output[node_index] = staged_design[node_index];
        request.staged_document_output[node_index] = staged_nodes[node_index];
        ++node_index;
    }

    std::size_t field_index = 0U;
    while (field_index < inspector_count) {
        request.inspector_output[field_index] = staged_inspector[field_index];
        ++field_index;
    }

    request.preview_feedback_output[0U] = staged_preview;
    request.command_ledger_output[0U] = staged_design_ledger;
    request.runtime_preview_output[0U] = runtime_preview_row;
    request.style_template_state_ledger_output[0U] = style_ledger;
    request.serialization_output[0U] = serialization_row;
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

    const bool preview_feedback_requested =
        request.require_preview_feedback || !request.preview_feedback_output.empty();
    if (request.hierarchy_output.size() < document.header.node_count) {
        result.status = UiEditorSurfaceStatus::OutputCapacityExceeded;
        result.blocked_layer = UiEditorSurfaceBlockedLayer::Output;
        SetSurfaceRequiredCounts(
            &result,
            document.header.node_count,
            preview_feedback_requested);
        *out_result = result;
        return result.status;
    }

    if (preview_feedback_requested && request.preview_feedback_output.empty()) {
        result.status = UiEditorSurfaceStatus::OutputCapacityExceeded;
        result.blocked_layer = UiEditorSurfaceBlockedLayer::Output;
        SetSurfaceRequiredCounts(
            &result,
            document.header.node_count,
            preview_feedback_requested);
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
            BuildPreviewFeedback(
                document.header,
                document.nodes,
                request.selected_node_id,
                *request.preview_frame);
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

UiEditorDesignWorkflowStatus BuildUiEditorDesignInspectorWorkflowSurface(
    const UiEditorDesignInspectorWorkflowRequest &request,
    UiEditorDesignInspectorWorkflowResult *out_result) {
    UiEditorDesignInspectorWorkflowResult result{};

    if (out_result == nullptr) {
        return UiEditorDesignWorkflowStatus::InvalidArgument;
    }

    if (request.document == nullptr || !DesignWorkflowStorageValid(request)) {
        *out_result = result;
        return result.status;
    }

    std::array<UiEditorHierarchyRow, MAX_UI_EDITOR_DOCUMENT_NODES> staged_hierarchy{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> staged_preview{};
    UiEditorRuntimeDocumentSurfaceRequest surface_request{};
    surface_request.document = request.document;
    surface_request.selected_node_id = request.selected_node_id;
    surface_request.preview_frame = request.preview_frame;
    surface_request.require_preview_feedback = true;
    surface_request.hierarchy_output =
        std::span<UiEditorHierarchyRow>(
            staged_hierarchy.data(),
            staged_hierarchy.size());
    surface_request.preview_feedback_output =
        std::span<UiEditorPreviewFeedbackRecord>(
            staged_preview.data(),
            staged_preview.size());
    UiEditorRuntimeDocumentSurfaceResult surface_result{};
    const UiEditorSurfaceStatus surface_status =
        BuildUiEditorRuntimeDocumentSurface(surface_request, &surface_result);
    result.surface = surface_result;
    result.surface_status = surface_status;
    result.document_id = surface_result.document_id;
    result.selected_node_id = request.selected_node_id;
    result.consumed_runtime_ui_document =
        surface_result.consumed_runtime_ui_document;
    result.consumed_preview_host_feedback =
        surface_result.consumed_preview_host_feedback;
    if (surface_status != UiEditorSurfaceStatus::Success) {
        result.status = MapDesignWorkflowStatus(surface_status);
        result.blocked_layer = MapDesignWorkflowLayer(surface_result.blocked_layer);
        *out_result = result;
        return result.status;
    }

    if (!PreviewFeedbackReady(request.preview_frame)) {
        result.status = UiEditorDesignWorkflowStatus::PreviewFeedbackMissing;
        result.blocked_layer = UiEditorDesignWorkflowBlockedLayer::PreviewHostFeedback;
        *out_result = result;
        return result.status;
    }

    const UiEditorRuntimeDocument &document = *request.document;
    const std::uint32_t node_count = document.header.node_count;
    if (!DesignWorkflowOutputCapacityReady(request, node_count)) {
        result.status = UiEditorDesignWorkflowStatus::OutputCapacityExceeded;
        result.blocked_layer = UiEditorDesignWorkflowBlockedLayer::Output;
        SetDesignWorkflowRequiredCounts(&result, node_count);
        *out_result = result;
        return result.status;
    }

    if (!request.selected_node_id.IsValid()) {
        result.status = UiEditorDesignWorkflowStatus::MissingNode;
        result.blocked_layer = UiEditorDesignWorkflowBlockedLayer::InspectorSelection;
        *out_result = result;
        return result.status;
    }

    const std::uint32_t selected_index = FindDocumentNodeIndex(
        document.nodes,
        request.selected_node_id,
        node_count);
    if (selected_index == node_count) {
        result.status = UiEditorDesignWorkflowStatus::MissingNode;
        result.blocked_layer = UiEditorDesignWorkflowBlockedLayer::InspectorSelection;
        *out_result = result;
        return result.status;
    }

    if (!IsDesignCommandSupported(request.command.kind)) {
        result.status = UiEditorDesignWorkflowStatus::CommandFailed;
        result.blocked_layer = UiEditorDesignWorkflowBlockedLayer::Command;
        *out_result = result;
        return result.status;
    }

    std::array<UiEditorDesignSurfaceRow, MAX_UI_EDITOR_DOCUMENT_NODES>
        staged_design_rows{};
    std::array<UiEditorInspectorFieldRow, UI_EDITOR_INSPECTOR_FIELD_COUNT>
        staged_inspector_rows{};
    std::array<UiEditorRuntimeNodeRecord, MAX_UI_EDITOR_DOCUMENT_NODES>
        staged_nodes{};

    std::uint32_t node_index = 0U;
    while (node_index < node_count) {
        staged_nodes[node_index] = document.nodes[node_index];
        staged_design_rows[node_index] = BuildDesignSurfaceRow(
            staged_hierarchy[node_index],
            document.nodes[node_index],
            staged_preview[0U]);
        ++node_index;
    }

    const UiEditorRuntimeNodeRecord before_node = staged_nodes[selected_index];
    staged_nodes[selected_index] =
        ApplyDesignCommand(staged_nodes[selected_index], request.command);
    const UiEditorRuntimeNodeRecord after_node = staged_nodes[selected_index];
    const std::size_t inspector_count = BuildInspectorFields(
        document.header,
        after_node,
        std::span<UiEditorInspectorFieldRow>(
            staged_inspector_rows.data(),
            staged_inspector_rows.size()));
    const UiEditorDesignCommandLedgerRecord ledger =
        BuildCommandLedger(document.header, before_node, after_node, request.command);

    node_index = 0U;
    while (node_index < node_count) {
        request.hierarchy_output[node_index] = staged_hierarchy[node_index];
        request.design_surface_output[node_index] = staged_design_rows[node_index];
        request.staged_document_output[node_index] = staged_nodes[node_index];
        ++node_index;
    }

    std::size_t field_index = 0U;
    while (field_index < inspector_count) {
        request.inspector_output[field_index] = staged_inspector_rows[field_index];
        ++field_index;
    }

    request.preview_feedback_output[0U] = staged_preview[0U];
    request.command_ledger_output[0U] = ledger;

    result.status = UiEditorDesignWorkflowStatus::Success;
    result.blocked_layer = UiEditorDesignWorkflowBlockedLayer::None;
    result.hierarchy_row_count = node_count;
    result.design_surface_row_count = node_count;
    result.inspector_field_count = inspector_count;
    result.preview_feedback_count = 1U;
    result.staged_node_count = node_count;
    result.command_ledger_count = 1U;
    result.built_design_surface = true;
    result.emitted_hierarchy_rows = true;
    result.emitted_inspector_fields = true;
    result.staged_document_update = ledger.staged_document_update;
    result.emitted_command_ledger = true;
    result.command_applied = ledger.command_applied;
    *out_result = result;
    return result.status;
}

UiEditorRuntimePreviewWorkflowStatus
BuildUiEditorRuntimePreviewStyleTemplateStateWorkflow(
    const UiEditorRuntimePreviewWorkflowRequest &request,
    UiEditorRuntimePreviewWorkflowResult *out_result) {
    UiEditorRuntimePreviewWorkflowResult result{};

    if (out_result == nullptr) {
        return UiEditorRuntimePreviewWorkflowStatus::InvalidArgument;
    }

    if (request.document == nullptr ||
        !RuntimePreviewWorkflowStorageValid(request)) {
        *out_result = result;
        return result.status;
    }

    result.selected_node_id = request.selected_node_id;
    result.document_id = request.document->header.document_id;
    result.style_template_state_record_count =
        request.style_template_state_records.size();

    std::array<UiEditorHierarchyRow, MAX_UI_EDITOR_DOCUMENT_NODES>
        staged_hierarchy{};
    std::array<UiEditorDesignSurfaceRow, MAX_UI_EDITOR_DOCUMENT_NODES>
        staged_design{};
    std::array<UiEditorInspectorFieldRow, UI_EDITOR_INSPECTOR_FIELD_COUNT>
        staged_inspector{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> staged_preview{};
    std::array<UiEditorRuntimeNodeRecord, MAX_UI_EDITOR_DOCUMENT_NODES>
        staged_nodes{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> staged_design_ledger{};

    UiEditorDesignInspectorWorkflowRequest design_request{};
    design_request.document = request.document;
    design_request.selected_node_id = request.selected_node_id;
    design_request.preview_frame = request.preview_frame;
    design_request.command = request.design_command;
    design_request.hierarchy_output =
        std::span<UiEditorHierarchyRow>(
            staged_hierarchy.data(),
            staged_hierarchy.size());
    design_request.design_surface_output =
        std::span<UiEditorDesignSurfaceRow>(
            staged_design.data(),
            staged_design.size());
    design_request.inspector_output =
        std::span<UiEditorInspectorFieldRow>(
            staged_inspector.data(),
            staged_inspector.size());
    design_request.preview_feedback_output =
        std::span<UiEditorPreviewFeedbackRecord>(
            staged_preview.data(),
            staged_preview.size());
    design_request.staged_document_output =
        std::span<UiEditorRuntimeNodeRecord>(
            staged_nodes.data(),
            staged_nodes.size());
    design_request.command_ledger_output =
        std::span<UiEditorDesignCommandLedgerRecord>(
            staged_design_ledger.data(),
            staged_design_ledger.size());

    UiEditorDesignInspectorWorkflowResult design_result{};
    const UiEditorDesignWorkflowStatus design_status =
        BuildUiEditorDesignInspectorWorkflowSurface(design_request, &design_result);
    result.design_workflow = design_result;
    result.document_id = design_result.document_id;
    result.consumed_runtime_ui_document =
        design_result.consumed_runtime_ui_document;
    result.consumed_preview_host_feedback =
        design_result.consumed_preview_host_feedback;
    result.built_design_surface = design_result.built_design_surface;
    if (design_status != UiEditorDesignWorkflowStatus::Success) {
        result.status = MapRuntimePreviewWorkflowStatus(design_status);
        result.blocked_layer =
            MapRuntimePreviewWorkflowLayer(design_result.blocked_layer);
        *out_result = result;
        return result.status;
    }

    const UiEditorRuntimeDocument &document = *request.document;
    const std::uint32_t node_count = document.header.node_count;
    if (!RuntimePreviewWorkflowOutputCapacityReady(request, node_count)) {
        result.status =
            UiEditorRuntimePreviewWorkflowStatus::OutputCapacityExceeded;
        result.blocked_layer = UiEditorRuntimePreviewWorkflowBlockedLayer::Output;
        SetRuntimePreviewWorkflowRequiredCounts(&result, design_result);
        *out_result = result;
        return result.status;
    }

    const std::uint32_t selected_index = FindDocumentNodeIndex(
        document.nodes,
        request.selected_node_id,
        node_count);
    if (selected_index == node_count) {
        result.status = UiEditorRuntimePreviewWorkflowStatus::MissingNode;
        result.blocked_layer =
            UiEditorRuntimePreviewWorkflowBlockedLayer::DesignWorkflow;
        *out_result = result;
        return result.status;
    }

    const UiEditorRuntimeNodeRecord &selected_node = staged_nodes[selected_index];
    const UiEditorStyleTemplateStateRecord *style_state =
        FindStyleTemplateStateRecord(
            request.style_template_state_records,
            request.selected_node_id);
    if (style_state == nullptr) {
        result.status =
            UiEditorRuntimePreviewWorkflowStatus::MissingStyleTemplateState;
        result.blocked_layer =
            UiEditorRuntimePreviewWorkflowBlockedLayer::StyleTemplateState;
        *out_result = result;
        return result.status;
    }

    if (!IsStyleTemplateStateRecordValid(*style_state, selected_node.component_kind)) {
        result.status =
            UiEditorRuntimePreviewWorkflowStatus::InvalidStyleTemplateState;
        result.blocked_layer =
            UiEditorRuntimePreviewWorkflowBlockedLayer::StyleTemplateState;
        *out_result = result;
        return result.status;
    }

    if (!IsStyleTemplateStateCommandSupported(
        request.style_template_state_command)) {
        result.status =
            UiEditorRuntimePreviewWorkflowStatus::StyleTemplateStateCommandFailed;
        result.blocked_layer =
            UiEditorRuntimePreviewWorkflowBlockedLayer::StyleTemplateState;
        *out_result = result;
        return result.status;
    }

    const UiEditorStyleTemplateStateRecord before_style_state = *style_state;
    const UiEditorStyleTemplateStateRecord after_style_state =
        ApplyStyleTemplateStateCommand(
            before_style_state,
            request.style_template_state_command);
    if (!IsStyleTemplateStateRecordValid(
        after_style_state,
        selected_node.component_kind)) {
        result.status =
            UiEditorRuntimePreviewWorkflowStatus::InvalidStyleTemplateState;
        result.blocked_layer =
            UiEditorRuntimePreviewWorkflowBlockedLayer::StyleTemplateState;
        *out_result = result;
        return result.status;
    }

    const UiEditorRuntimePreviewStyleTemplateStateRow runtime_preview_row =
        BuildRuntimePreviewStyleRow(
            document.header,
            staged_design[selected_index],
            after_style_state,
            staged_preview[0U]);
    const UiEditorStyleTemplateStateLedgerRecord style_ledger =
        BuildStyleTemplateStateLedger(
            document.header,
            before_style_state,
            after_style_state,
            request.style_template_state_command);

    std::uint32_t node_index = 0U;
    while (node_index < node_count) {
        request.hierarchy_output[node_index] = staged_hierarchy[node_index];
        request.design_surface_output[node_index] = staged_design[node_index];
        request.staged_document_output[node_index] = staged_nodes[node_index];
        ++node_index;
    }

    std::size_t field_index = 0U;
    while (field_index < design_result.inspector_field_count) {
        request.inspector_output[field_index] = staged_inspector[field_index];
        ++field_index;
    }

    request.preview_feedback_output[0U] = staged_preview[0U];
    request.command_ledger_output[0U] = staged_design_ledger[0U];
    request.runtime_preview_output[0U] = runtime_preview_row;
    request.style_template_state_ledger_output[0U] = style_ledger;

    result.status = UiEditorRuntimePreviewWorkflowStatus::Success;
    result.blocked_layer = UiEditorRuntimePreviewWorkflowBlockedLayer::None;
    result.hierarchy_row_count = design_result.hierarchy_row_count;
    result.design_surface_row_count = design_result.design_surface_row_count;
    result.inspector_field_count = design_result.inspector_field_count;
    result.preview_feedback_count = design_result.preview_feedback_count;
    result.staged_node_count = design_result.staged_node_count;
    result.command_ledger_count = design_result.command_ledger_count;
    result.runtime_preview_row_count = 1U;
    result.style_template_state_ledger_count = 1U;
    result.consumed_style_template_state = true;
    result.built_engine_runtime_preview = true;
    result.emitted_hierarchy_rows = true;
    result.emitted_inspector_fields = true;
    result.emitted_runtime_preview_row = true;
    result.staged_document_update = design_result.staged_document_update;
    result.emitted_command_ledger = true;
    result.emitted_style_template_state_ledger = true;
    result.design_command_applied = design_result.command_applied;
    result.style_template_state_command_applied = style_ledger.command_applied;
    *out_result = result;
    return result.status;
}

UiEditorStyleThemeTemplateSerializationStatus
BuildUiEditorStyleThemeTemplateSerializationWorkflow(
    const UiEditorStyleThemeTemplateSerializationWorkflowRequest &request,
    UiEditorStyleThemeTemplateSerializationWorkflowResult *out_result) {
    UiEditorStyleThemeTemplateSerializationWorkflowResult result{};

    if (out_result == nullptr) {
        return UiEditorStyleThemeTemplateSerializationStatus::InvalidArgument;
    }

    if (request.document == nullptr ||
        !SerializationWorkflowStorageValid(request)) {
        *out_result = result;
        return result.status;
    }

    result.selected_node_id = request.selected_node_id;
    result.document_id = request.document->header.document_id;

    std::array<UiEditorHierarchyRow, MAX_UI_EDITOR_DOCUMENT_NODES>
        staged_hierarchy{};
    std::array<UiEditorDesignSurfaceRow, MAX_UI_EDITOR_DOCUMENT_NODES>
        staged_design{};
    std::array<UiEditorInspectorFieldRow, UI_EDITOR_INSPECTOR_FIELD_COUNT>
        staged_inspector{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> staged_preview{};
    std::array<UiEditorRuntimeNodeRecord, MAX_UI_EDITOR_DOCUMENT_NODES>
        staged_nodes{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> staged_design_ledger{};
    std::array<UiEditorRuntimePreviewStyleTemplateStateRow, 1U>
        staged_runtime_preview{};
    std::array<UiEditorStyleTemplateStateLedgerRecord, 1U>
        staged_style_ledger{};

    UiEditorRuntimePreviewWorkflowRequest runtime_preview_request{};
    runtime_preview_request.document = request.document;
    runtime_preview_request.selected_node_id = request.selected_node_id;
    runtime_preview_request.preview_frame = request.preview_frame;
    runtime_preview_request.design_command = request.design_command;
    runtime_preview_request.style_template_state_command =
        request.style_template_state_command;
    runtime_preview_request.style_template_state_records =
        request.style_template_state_records;
    runtime_preview_request.hierarchy_output =
        std::span<UiEditorHierarchyRow>(
            staged_hierarchy.data(),
            staged_hierarchy.size());
    runtime_preview_request.design_surface_output =
        std::span<UiEditorDesignSurfaceRow>(
            staged_design.data(),
            staged_design.size());
    runtime_preview_request.inspector_output =
        std::span<UiEditorInspectorFieldRow>(
            staged_inspector.data(),
            staged_inspector.size());
    runtime_preview_request.preview_feedback_output =
        std::span<UiEditorPreviewFeedbackRecord>(
            staged_preview.data(),
            staged_preview.size());
    runtime_preview_request.staged_document_output =
        std::span<UiEditorRuntimeNodeRecord>(
            staged_nodes.data(),
            staged_nodes.size());
    runtime_preview_request.command_ledger_output =
        std::span<UiEditorDesignCommandLedgerRecord>(
            staged_design_ledger.data(),
            staged_design_ledger.size());
    runtime_preview_request.runtime_preview_output =
        std::span<UiEditorRuntimePreviewStyleTemplateStateRow>(
            staged_runtime_preview.data(),
            staged_runtime_preview.size());
    runtime_preview_request.style_template_state_ledger_output =
        std::span<UiEditorStyleTemplateStateLedgerRecord>(
            staged_style_ledger.data(),
            staged_style_ledger.size());

    UiEditorRuntimePreviewWorkflowResult runtime_preview_result{};
    const UiEditorRuntimePreviewWorkflowStatus runtime_preview_status =
        BuildUiEditorRuntimePreviewStyleTemplateStateWorkflow(
            runtime_preview_request,
            &runtime_preview_result);
    result.runtime_preview_workflow = runtime_preview_result;
    result.document_id = runtime_preview_result.document_id;
    result.consumed_runtime_ui_document =
        runtime_preview_result.consumed_runtime_ui_document;
    result.consumed_preview_host_feedback =
        runtime_preview_result.consumed_preview_host_feedback;
    result.consumed_style_template_state =
        runtime_preview_result.consumed_style_template_state;
    result.built_engine_runtime_preview =
        runtime_preview_result.built_engine_runtime_preview;
    if (runtime_preview_status != UiEditorRuntimePreviewWorkflowStatus::Success) {
        result.status =
            UiEditorStyleThemeTemplateSerializationStatus::RuntimePreviewWorkflowFailed;
        result.blocked_layer =
            MapSerializationWorkflowLayer(runtime_preview_result.blocked_layer);
        *out_result = result;
        return result.status;
    }

    const std::size_t node_count = runtime_preview_result.staged_node_count;
    if (!SerializationWorkflowOutputCapacityReady(request, node_count)) {
        result.status =
            UiEditorStyleThemeTemplateSerializationStatus::OutputCapacityExceeded;
        result.blocked_layer =
            UiEditorStyleThemeTemplateSerializationBlockedLayer::Output;
        SetSerializationWorkflowRequiredCounts(&result, runtime_preview_result);
        *out_result = result;
        return result.status;
    }

    if (!FileVfsRequestAvailable(request)) {
        result.status =
            UiEditorStyleThemeTemplateSerializationStatus::FileVfsUnavailable;
        result.blocked_layer =
            UiEditorStyleThemeTemplateSerializationBlockedLayer::FileVfs;
        *out_result = result;
        return result.status;
    }

    std::array<std::uint8_t, UI_EDITOR_SERIALIZATION_BUFFER_SIZE>
        serialized_bytes{};
    std::uint32_t serialized_byte_count = 0U;
    const SerializedStyleThemeTemplateValues source_values =
        ValuesFromRuntimePreviewRow(staged_runtime_preview[0U]);
    const SerializeStatus write_status =
        WriteStyleThemeTemplateStream(
            source_values,
            std::span<std::uint8_t>(
                serialized_bytes.data(),
                serialized_bytes.size()),
            &serialized_byte_count);
    result.serialize_write_status = write_status;
    result.serialized_byte_count = serialized_byte_count;
    if (write_status != SerializeStatus::Success) {
        result.status =
            UiEditorStyleThemeTemplateSerializationStatus::SerializeFailed;
        result.blocked_layer =
            UiEditorStyleThemeTemplateSerializationBlockedLayer::Serialize;
        *out_result = result;
        return result.status;
    }

    MountTable &mount_table = *request.file_mount_table;
    FileWriteRequest write_request{};
    write_request.mount = request.file_mount;
    write_request.path = request.file_path;
    write_request.bytes = serialized_bytes.data();
    write_request.byte_count = serialized_byte_count;
    const FileWriteResult write_result = mount_table.Write(write_request);
    result.file_write_status = write_result.status;
    result.saved_byte_count = write_result.byte_count;
    if (!write_result.Succeeded()) {
        result.status =
            UiEditorStyleThemeTemplateSerializationStatus::FileVfsFailed;
        result.blocked_layer =
            UiEditorStyleThemeTemplateSerializationBlockedLayer::FileVfs;
        *out_result = result;
        return result.status;
    }

    FileReadRequest read_request{};
    read_request.mount = request.file_mount;
    read_request.path = request.file_path;
    const FileReadResult read_result = mount_table.Read(read_request);
    result.file_read_status = read_result.status;
    result.loaded_byte_count = read_result.bytes.size();
    if (!read_result.Succeeded()) {
        result.status =
            UiEditorStyleThemeTemplateSerializationStatus::FileVfsFailed;
        result.blocked_layer =
            UiEditorStyleThemeTemplateSerializationBlockedLayer::FileVfs;
        *out_result = result;
        return result.status;
    }

    SerializedStyleThemeTemplateValues loaded_values{};
    const SerializeStatus read_status =
        ReadStyleThemeTemplateStream(
            std::span<const std::uint8_t>(
                read_result.bytes.data(),
                read_result.bytes.size()),
            &loaded_values);
    result.serialize_read_status = read_status;
    if (read_status != SerializeStatus::Success) {
        result.status =
            UiEditorStyleThemeTemplateSerializationStatus::DeserializeFailed;
        result.blocked_layer =
            UiEditorStyleThemeTemplateSerializationBlockedLayer::Serialize;
        *out_result = result;
        return result.status;
    }

    const UiEditorStyleThemeTemplateSerializationRow serialization_row =
        BuildSerializationBoundaryRow(
            source_values,
            loaded_values,
            serialized_byte_count,
            write_result.byte_count,
            read_result.bytes.size());
    if (!serialization_row.runtime_asset_boundary_preserved ||
        !serialization_row.ui_runtime_boundary_preserved ||
        !serialization_row.component_template_state_provenance_preserved) {
        result.status =
            UiEditorStyleThemeTemplateSerializationStatus::ProvenanceMismatch;
        result.blocked_layer =
            UiEditorStyleThemeTemplateSerializationBlockedLayer::RuntimeAssetBoundary;
        *out_result = result;
        return result.status;
    }

    CopySerializationWorkflowOutputs(
        request,
        std::span<const UiEditorHierarchyRow>(
            staged_hierarchy.data(),
            staged_hierarchy.size()),
        std::span<const UiEditorDesignSurfaceRow>(
            staged_design.data(),
            staged_design.size()),
        std::span<const UiEditorInspectorFieldRow>(
            staged_inspector.data(),
            staged_inspector.size()),
        std::span<const UiEditorRuntimeNodeRecord>(
            staged_nodes.data(),
            staged_nodes.size()),
        staged_preview[0U],
        staged_design_ledger[0U],
        staged_runtime_preview[0U],
        staged_style_ledger[0U],
        serialization_row,
        node_count,
        runtime_preview_result.inspector_field_count);

    result.status = UiEditorStyleThemeTemplateSerializationStatus::Success;
    result.blocked_layer =
        UiEditorStyleThemeTemplateSerializationBlockedLayer::None;
    result.hierarchy_row_count = runtime_preview_result.hierarchy_row_count;
    result.design_surface_row_count =
        runtime_preview_result.design_surface_row_count;
    result.inspector_field_count = runtime_preview_result.inspector_field_count;
    result.preview_feedback_count = runtime_preview_result.preview_feedback_count;
    result.staged_node_count = runtime_preview_result.staged_node_count;
    result.command_ledger_count = runtime_preview_result.command_ledger_count;
    result.runtime_preview_row_count =
        runtime_preview_result.runtime_preview_row_count;
    result.style_template_state_ledger_count =
        runtime_preview_result.style_template_state_ledger_count;
    result.serialization_row_count = 1U;
    result.saved_through_file_vfs = true;
    result.loaded_through_file_vfs = true;
    result.serialized_runtime_ui_data = true;
    result.deserialized_runtime_ui_data = true;
    result.runtime_asset_boundary_preserved =
        serialization_row.runtime_asset_boundary_preserved;
    result.ui_runtime_boundary_preserved =
        serialization_row.ui_runtime_boundary_preserved;
    result.component_template_state_provenance_preserved =
        serialization_row.component_template_state_provenance_preserved;
    result.emitted_serialization_row = true;
    *out_result = result;
    return result.status;
}
}
