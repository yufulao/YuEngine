// 模块: Tools UiWebEditorShell
// 文件: Tools/UiWebEditorShell/Src/UiWebEditorShell.cpp

#include "YuEngine/UiWebEditorShell/UiWebEditorShell.h"

namespace yuengine::ui_web_editor_shell {
UiWebEditorShellStatus UiWebEditorShell::BuildSnapshot(
    const UiWebEditorShellRequest &request,
    std::span<UiWebEditorHierarchyItemRecord> out_hierarchy,
    UiWebEditorInspectorRecord *out_inspector,
    std::span<UiWebEditorCanvasItemRecord> out_canvas,
    std::span<UiWebEditorResourceItemRecord> out_resources,
    UiWebEditorShellSnapshot *out_snapshot) const {
    if (out_snapshot == nullptr) {
        return UiWebEditorShellStatus::InvalidInput;
    }

    *out_snapshot = UiWebEditorShellSnapshot{};
    out_snapshot->document_id = request.document.document_id;
    out_snapshot->layout_id = request.schema.header.layout_id;
    out_snapshot->document_loaded = request.document.loaded;
    out_snapshot->has_selection = request.has_selection;
    out_snapshot->selected_node_id = request.selected_node_id;
    if (out_inspector == nullptr) {
        out_snapshot->status = UiWebEditorShellStatus::InvalidInput;
        return UiWebEditorShellStatus::InvalidInput;
    }

    if (request.document.document_id == 0U || !request.document.loaded) {
        out_snapshot->status = UiWebEditorShellStatus::InvalidDocument;
        return UiWebEditorShellStatus::InvalidDocument;
    }

    if (!ValidateDocumentCounts(request)) {
        out_snapshot->status = UiWebEditorShellStatus::SchemaCountMismatch;
        return UiWebEditorShellStatus::SchemaCountMismatch;
    }

    std::size_t inspector_node_index = 0U;
    if (!ResolveInspectorNodeIndex(request, &inspector_node_index)) {
        out_snapshot->status = UiWebEditorShellStatus::MissingSelectedNode;
        return UiWebEditorShellStatus::MissingSelectedNode;
    }

    if (!HasOutputCapacity(request, out_hierarchy, out_canvas, out_resources)) {
        out_snapshot->status = UiWebEditorShellStatus::OutputCapacityExceeded;
        return UiWebEditorShellStatus::OutputCapacityExceeded;
    }

    WriteHierarchy(request, out_hierarchy);
    WriteInspector(request, inspector_node_index, out_inspector);
    WriteCanvas(request, out_canvas);
    WriteResources(request, out_resources);
    WriteSnapshot(request, inspector_node_index, out_snapshot);
    return UiWebEditorShellStatus::Success;
}

bool UiWebEditorShell::ValidateDocumentCounts(const UiWebEditorShellRequest &request) const {
    if (request.document.schema_version != request.schema.header.schema_version) {
        return false;
    }

    if (request.document.layout_id != request.schema.header.layout_id) {
        return false;
    }

    const std::uint32_t node_count = static_cast<std::uint32_t>(request.schema.nodes.size());
    if (request.document.node_count != node_count) {
        return false;
    }

    const std::uint32_t layout_count = static_cast<std::uint32_t>(request.schema.layouts.size());
    if (request.document.layout_count != layout_count) {
        return false;
    }

    const std::uint32_t style_ref_count = static_cast<std::uint32_t>(request.schema.style_refs.size());
    if (request.document.style_ref_count != style_ref_count) {
        return false;
    }

    const std::uint32_t resource_ref_count = static_cast<std::uint32_t>(request.schema.resource_refs.size());
    if (request.document.resource_ref_count != resource_ref_count) {
        return false;
    }

    const std::uint32_t event_binding_count = static_cast<std::uint32_t>(request.schema.event_bindings.size());
    return request.document.event_binding_count == event_binding_count;
}

bool UiWebEditorShell::HasOutputCapacity(
    const UiWebEditorShellRequest &request,
    std::span<UiWebEditorHierarchyItemRecord> out_hierarchy,
    std::span<UiWebEditorCanvasItemRecord> out_canvas,
    std::span<UiWebEditorResourceItemRecord> out_resources) const {
    if (out_hierarchy.size() < request.schema.nodes.size()) {
        return false;
    }

    if (out_canvas.size() < request.schema.nodes.size()) {
        return false;
    }

    return out_resources.size() >= request.schema.resource_refs.size();
}

bool UiWebEditorShell::FindNodeIndex(
    std::span<const yuengine::uicore::UiFileNodeRecord> nodes,
    yuengine::uicore::UiNodeId node_id,
    std::size_t *out_index) const {
    if (out_index == nullptr) {
        return false;
    }

    for (std::size_t index = 0U; index < nodes.size(); ++index) {
        const yuengine::uicore::UiFileNodeRecord &record = nodes[index];
        if (record.node_id.value == node_id.value) {
            *out_index = index;
            return true;
        }
    }

    return false;
}

bool UiWebEditorShell::ResolveInspectorNodeIndex(
    const UiWebEditorShellRequest &request,
    std::size_t *out_index) const {
    if (out_index == nullptr) {
        return false;
    }

    if (request.has_selection) {
        return FindNodeIndex(request.schema.nodes, request.selected_node_id, out_index);
    }

    return FindNodeIndex(request.schema.nodes, request.schema.header.root_node_id, out_index);
}

std::uint32_t UiWebEditorShell::CountLayoutContainers(
    const UiWebEditorShellRequest &request,
    yuengine::uicore::UiNodeId node_id) const {
    std::uint32_t count = 0U;
    for (std::size_t index = 0U; index < request.schema.layouts.size(); ++index) {
        const yuengine::uicore::UiFileLayoutRecord &record = request.schema.layouts[index];
        if (record.container.container_id.value == node_id.value) {
            ++count;
        }
    }

    return count;
}

std::uint32_t UiWebEditorShell::CountStyleRefs(
    const UiWebEditorShellRequest &request,
    yuengine::uicore::UiNodeId node_id) const {
    std::uint32_t count = 0U;
    for (std::size_t index = 0U; index < request.schema.style_refs.size(); ++index) {
        const yuengine::uicore::UiFileStyleRef &record = request.schema.style_refs[index];
        if (record.node_id.value == node_id.value) {
            ++count;
        }
    }

    return count;
}

std::uint32_t UiWebEditorShell::CountResourceRefs(
    const UiWebEditorShellRequest &request,
    yuengine::uicore::UiNodeId node_id) const {
    std::uint32_t count = 0U;
    for (std::size_t index = 0U; index < request.schema.resource_refs.size(); ++index) {
        const yuengine::uicore::UiFileResourceRef &record = request.schema.resource_refs[index];
        if (record.node_id.value == node_id.value) {
            ++count;
        }
    }

    return count;
}

std::uint32_t UiWebEditorShell::CountEventBindings(
    const UiWebEditorShellRequest &request,
    yuengine::uicore::UiNodeId node_id) const {
    std::uint32_t count = 0U;
    for (std::size_t index = 0U; index < request.schema.event_bindings.size(); ++index) {
        const yuengine::uicore::UiFileEventBinding &record = request.schema.event_bindings[index];
        if (record.node_id.value == node_id.value) {
            ++count;
        }
    }

    return count;
}

void UiWebEditorShell::WriteHierarchy(
    const UiWebEditorShellRequest &request,
    std::span<UiWebEditorHierarchyItemRecord> out_hierarchy) const {
    for (std::size_t index = 0U; index < request.schema.nodes.size(); ++index) {
        const yuengine::uicore::UiFileNodeRecord &node = request.schema.nodes[index];
        UiWebEditorHierarchyItemRecord record{};
        record.node_id = node.node_id;
        record.parent_id = node.parent_id;
        record.sibling_order = node.sibling_order;
        record.layer = node.layer;
        record.is_root = node.node_id.value == request.schema.header.root_node_id.value;
        record.is_selected = request.has_selection && node.node_id.value == request.selected_node_id.value;
        record.is_visible = node.is_visible;
        record.is_enabled = node.is_enabled;
        out_hierarchy[index] = record;
    }
}

void UiWebEditorShell::WriteInspector(
    const UiWebEditorShellRequest &request,
    std::size_t node_index,
    UiWebEditorInspectorRecord *out_inspector) const {
    if (out_inspector == nullptr) {
        return;
    }

    const yuengine::uicore::UiFileNodeRecord &node = request.schema.nodes[node_index];
    UiWebEditorInspectorRecord record{};
    record.node_id = node.node_id;
    record.layout_container_count = CountLayoutContainers(request, node.node_id);
    record.style_ref_count = CountStyleRefs(request, node.node_id);
    record.resource_ref_count = CountResourceRefs(request, node.node_id);
    record.event_binding_count = CountEventBindings(request, node.node_id);
    record.is_root = node.node_id.value == request.schema.header.root_node_id.value;
    record.is_visible = node.is_visible;
    record.is_enabled = node.is_enabled;
    record.is_hit_testable = node.is_hit_testable;
    *out_inspector = record;
}

void UiWebEditorShell::WriteCanvas(
    const UiWebEditorShellRequest &request,
    std::span<UiWebEditorCanvasItemRecord> out_canvas) const {
    for (std::size_t index = 0U; index < request.schema.nodes.size(); ++index) {
        const yuengine::uicore::UiFileNodeRecord &node = request.schema.nodes[index];
        UiWebEditorCanvasItemRecord record{};
        record.node_id = node.node_id;
        record.rect_transform = node.rect_transform;
        record.sibling_order = node.sibling_order;
        record.layer = node.layer;
        record.is_selected = request.has_selection && node.node_id.value == request.selected_node_id.value;
        record.is_visible = node.is_visible;
        record.is_enabled = node.is_enabled;
        record.is_hit_testable = node.is_hit_testable;
        out_canvas[index] = record;
    }
}

void UiWebEditorShell::WriteResources(
    const UiWebEditorShellRequest &request,
    std::span<UiWebEditorResourceItemRecord> out_resources) const {
    for (std::size_t index = 0U; index < request.schema.resource_refs.size(); ++index) {
        const yuengine::uicore::UiFileResourceRef &resource = request.schema.resource_refs[index];
        UiWebEditorResourceItemRecord record{};
        record.node_id = resource.node_id;
        record.resource_kind = resource.kind;
        record.resource_key = resource.resource_key;
        record.is_selected_node = request.has_selection && resource.node_id.value == request.selected_node_id.value;
        out_resources[index] = record;
    }
}

void UiWebEditorShell::WriteSnapshot(
    const UiWebEditorShellRequest &request,
    std::size_t inspector_node_index,
    UiWebEditorShellSnapshot *out_snapshot) const {
    if (out_snapshot == nullptr) {
        return;
    }

    out_snapshot->status = UiWebEditorShellStatus::Success;
    out_snapshot->shell_version = UI_WEB_EDITOR_SHELL_VERSION;
    out_snapshot->document_id = request.document.document_id;
    out_snapshot->layout_id = request.schema.header.layout_id;
    out_snapshot->hierarchy_item_count = static_cast<std::uint32_t>(request.schema.nodes.size());
    out_snapshot->canvas_item_count = static_cast<std::uint32_t>(request.schema.nodes.size());
    out_snapshot->resource_item_count = static_cast<std::uint32_t>(request.schema.resource_refs.size());
    out_snapshot->selected_node_id = request.selected_node_id;
    out_snapshot->inspector_node_id = request.schema.nodes[inspector_node_index].node_id;
    out_snapshot->document_loaded = request.document.loaded;
    out_snapshot->has_selection = request.has_selection;
    out_snapshot->schema_count_matched = true;
    out_snapshot->hierarchy_ready = true;
    out_snapshot->inspector_ready = true;
    out_snapshot->canvas_ready = true;
    out_snapshot->resource_panel_ready = true;
}
}
