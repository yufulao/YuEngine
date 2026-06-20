// 模块: Tools UiWebEditorShell
// 文件: Tools/UiWebEditorShell/Include/YuEngine/UiWebEditorShell/UiWebEditorShell.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"
#include "YuEngine/UiWebEditorService/UiWebEditorLocalService.h"

namespace yuengine::ui_web_editor_shell {
constexpr std::uint32_t UI_WEB_EDITOR_SHELL_VERSION = 1U;

enum class UiWebEditorShellStatus {
    Success = 0,
    InvalidInput,
    InvalidDocument,
    SchemaCountMismatch,
    MissingSelectedNode,
    OutputCapacityExceeded
};

struct UiWebEditorShellRequest final {
    yuengine::ui_web_editor_service::UiWebEditorLocalDocumentRecord document;
    yuengine::uicore::UiFileSchemaDesc schema;
    yuengine::uicore::UiNodeId selected_node_id;
    bool has_selection = false;
};

struct UiWebEditorHierarchyItemRecord final {
    yuengine::uicore::UiNodeId node_id;
    yuengine::uicore::UiNodeId parent_id;
    std::uint32_t sibling_order = 0U;
    std::int32_t layer = 0;
    bool is_root = false;
    bool is_selected = false;
    bool is_visible = false;
    bool is_enabled = false;
};

struct UiWebEditorInspectorRecord final {
    yuengine::uicore::UiNodeId node_id;
    std::uint32_t layout_container_count = 0U;
    std::uint32_t style_ref_count = 0U;
    std::uint32_t resource_ref_count = 0U;
    std::uint32_t event_binding_count = 0U;
    bool is_root = false;
    bool is_visible = false;
    bool is_enabled = false;
    bool is_hit_testable = false;
};

struct UiWebEditorCanvasItemRecord final {
    yuengine::uicore::UiNodeId node_id;
    yuengine::uicore::UiRectTransform rect_transform;
    std::uint32_t sibling_order = 0U;
    std::int32_t layer = 0;
    bool is_selected = false;
    bool is_visible = false;
    bool is_enabled = false;
    bool is_hit_testable = false;
};

struct UiWebEditorResourceItemRecord final {
    yuengine::uicore::UiNodeId node_id;
    yuengine::uicore::UiFileResourceKind resource_kind = yuengine::uicore::UiFileResourceKind::Invalid;
    std::uint32_t resource_key = 0U;
    bool is_selected_node = false;
};

struct UiWebEditorShellSnapshot final {
    UiWebEditorShellStatus status = UiWebEditorShellStatus::Success;
    std::uint32_t shell_version = UI_WEB_EDITOR_SHELL_VERSION;
    std::uint32_t document_id = 0U;
    std::uint32_t layout_id = 0U;
    std::uint32_t hierarchy_item_count = 0U;
    std::uint32_t canvas_item_count = 0U;
    std::uint32_t resource_item_count = 0U;
    yuengine::uicore::UiNodeId selected_node_id;
    yuengine::uicore::UiNodeId inspector_node_id;
    bool document_loaded = false;
    bool has_selection = false;
    bool schema_count_matched = false;
    bool hierarchy_ready = false;
    bool inspector_ready = false;
    bool canvas_ready = false;
    bool resource_panel_ready = false;

    /**
     * @comment 检查 Web editor shell snapshot 是否构建成功。
     * @return shell snapshot 构建成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiWebEditorShellStatus::Success;
    }
};

class UiWebEditorShell final {
public:
    /**
     * @comment 基于通用 UI file schema 和 local service document 构建 Web editor shell panel snapshot。
     * @param request shell 构建输入。
     * @param out_hierarchy 调用方持有的 hierarchy panel 输出。
     * @param out_inspector 调用方持有的 inspector panel 输出。
     * @param out_canvas 调用方持有的 canvas panel 输出。
     * @param out_resources 调用方持有的 resource panel 输出。
     * @param out_snapshot 输出 shell snapshot。
     * @return 显式 shell 构建状态。
     */
    UiWebEditorShellStatus BuildSnapshot(
        const UiWebEditorShellRequest &request,
        std::span<UiWebEditorHierarchyItemRecord> out_hierarchy,
        UiWebEditorInspectorRecord *out_inspector,
        std::span<UiWebEditorCanvasItemRecord> out_canvas,
        std::span<UiWebEditorResourceItemRecord> out_resources,
        UiWebEditorShellSnapshot *out_snapshot) const;

private:
    bool ValidateDocumentCounts(const UiWebEditorShellRequest &request) const;
    bool HasOutputCapacity(
        const UiWebEditorShellRequest &request,
        std::span<UiWebEditorHierarchyItemRecord> out_hierarchy,
        std::span<UiWebEditorCanvasItemRecord> out_canvas,
        std::span<UiWebEditorResourceItemRecord> out_resources) const;
    bool FindNodeIndex(
        std::span<const yuengine::uicore::UiFileNodeRecord> nodes,
        yuengine::uicore::UiNodeId node_id,
        std::size_t *out_index) const;
    bool ResolveInspectorNodeIndex(const UiWebEditorShellRequest &request, std::size_t *out_index) const;
    std::uint32_t CountLayoutContainers(
        const UiWebEditorShellRequest &request,
        yuengine::uicore::UiNodeId node_id) const;
    std::uint32_t CountStyleRefs(
        const UiWebEditorShellRequest &request,
        yuengine::uicore::UiNodeId node_id) const;
    std::uint32_t CountResourceRefs(
        const UiWebEditorShellRequest &request,
        yuengine::uicore::UiNodeId node_id) const;
    std::uint32_t CountEventBindings(
        const UiWebEditorShellRequest &request,
        yuengine::uicore::UiNodeId node_id) const;
    void WriteHierarchy(
        const UiWebEditorShellRequest &request,
        std::span<UiWebEditorHierarchyItemRecord> out_hierarchy) const;
    void WriteInspector(
        const UiWebEditorShellRequest &request,
        std::size_t node_index,
        UiWebEditorInspectorRecord *out_inspector) const;
    void WriteCanvas(
        const UiWebEditorShellRequest &request,
        std::span<UiWebEditorCanvasItemRecord> out_canvas) const;
    void WriteResources(
        const UiWebEditorShellRequest &request,
        std::span<UiWebEditorResourceItemRecord> out_resources) const;
    void WriteSnapshot(
        const UiWebEditorShellRequest &request,
        std::size_t inspector_node_index,
        UiWebEditorShellSnapshot *out_snapshot) const;
};
}
