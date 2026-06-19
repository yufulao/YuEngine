// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Include/YuEngine/UiEditor/UiEditorShellState.h

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace yuengine::uieditor {
constexpr std::uint32_t UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT = 3U;
constexpr std::uint32_t UI_EDITOR_LAYOUT_MAX_NODE_COUNT = 32U;
constexpr std::uint32_t UI_EDITOR_LAYOUT_ID_CAPACITY = 64U;
constexpr std::uint32_t UI_EDITOR_LAYOUT_NAME_CAPACITY = 64U;
constexpr std::uint32_t UI_EDITOR_LAYOUT_TYPE_CAPACITY = 32U;

enum class UiEditorShellPanelId {
    Invalid = 0,
    Hierarchy,
    Inspector,
    Preview
};

enum class UiEditorShellStatus {
    Success = 0,
    InvalidPanel,
    InvalidOutput,
    OutputCapacityExceeded,
    DearImGuiUnavailable,
    LayoutNotLoaded,
    InvalidLayoutAsset,
    LayoutNodeCapacityExceeded,
    DuplicateNodeId,
    MissingRootNode,
    MissingParentNode,
    NodeNotFound
};

struct UiEditorShellPanelRecord {
    UiEditorShellPanelId panel_id = UiEditorShellPanelId::Invalid;
    const char *panel_name = "";
    bool is_open = false;
    bool is_placeholder = true;
};

struct UiEditorLayoutNodeRecord {
    std::uint32_t node_id = 0U;
    std::uint32_t parent_node_id = 0U;
    std::uint32_t order = 0U;
    char name[UI_EDITOR_LAYOUT_NAME_CAPACITY] = {};
    char type[UI_EDITOR_LAYOUT_TYPE_CAPACITY] = {};
};

struct UiEditorInspectorRecord {
    bool has_selection = false;
    std::uint32_t node_id = 0U;
    std::uint32_t parent_node_id = 0U;
    std::uint32_t order = 0U;
    char layout_id[UI_EDITOR_LAYOUT_ID_CAPACITY] = {};
    char name[UI_EDITOR_LAYOUT_NAME_CAPACITY] = {};
    char type[UI_EDITOR_LAYOUT_TYPE_CAPACITY] = {};
};

struct UiEditorLayoutDocument {
    bool is_loaded = false;
    std::uint32_t version = 0U;
    std::uint32_t root_node_id = 0U;
    std::uint32_t node_count = 0U;
    char layout_id[UI_EDITOR_LAYOUT_ID_CAPACITY] = {};
    std::array<UiEditorLayoutNodeRecord, UI_EDITOR_LAYOUT_MAX_NODE_COUNT> nodes{};
};

struct UiEditorShellSnapshot {
    std::uint32_t panel_count = 0U;
    std::uint32_t open_panel_count = 0U;
    std::uint32_t placeholder_panel_count = 0U;
    std::uint32_t required_placeholder_count = 0U;
    std::uint32_t loaded_node_count = 0U;
    std::uint32_t selected_node_id = 0U;
    bool dear_imgui_backend_ready = false;
    bool layout_loaded = false;
    UiEditorShellStatus last_status = UiEditorShellStatus::Success;
};

class UiEditorShellState final {
public:
    /**
     * @comment 构造 editor shell state registry。
     */
    UiEditorShellState();

    /**
     * @comment 打开 E1 首片要求的 placeholder panels。
     * @return 显式操作状态。
     */
    UiEditorShellStatus OpenDefaultPlaceholderPanels();
    /**
     * @comment 设置指定 panel 的 open 状态。
     * @param panel_id 输入 panel id。
     * @param is_open 输入 open 状态。
     * @return 显式操作状态。
     */
    UiEditorShellStatus SetPanelOpen(UiEditorShellPanelId panel_id, bool is_open);
    /**
     * @comment 导出当前 panel registry。
     * @param output_records 调用方持有的 output buffer。
     * @param output_capacity output buffer capacity。
     * @param out_count 写回需要的 panel 数量。
     * @return 显式操作状态。
     */
    UiEditorShellStatus ExportPanels(
        UiEditorShellPanelRecord *output_records,
        std::uint32_t output_capacity,
        std::uint32_t *out_count);
    /**
     * @comment 加载 editor 可检查的 YuUILayout 文本。
     * @param layout_text 输入 YuUILayout JSON 文本。
     * @return 显式操作状态。
     */
    UiEditorShellStatus LoadLayoutAsset(std::string_view layout_text);
    /**
     * @comment 导出当前 layout hierarchy nodes。
     * @param output_records 调用方持有的 output buffer。
     * @param output_capacity output buffer capacity。
     * @param out_count 写回需要的 node 数量。
     * @return 显式操作状态。
     */
    UiEditorShellStatus ExportHierarchyNodes(
        UiEditorLayoutNodeRecord *output_records,
        std::uint32_t output_capacity,
        std::uint32_t *out_count);
    /**
     * @comment 选中 layout node 并刷新 inspector record。
     * @param node_id 输入 node id。
     * @return 显式操作状态。
     */
    UiEditorShellStatus SelectLayoutNode(std::uint32_t node_id);
    /**
     * @comment 查询当前 inspector record。
     * @param out_record 输出 inspector record。
     * @return 显式操作状态。
     */
    UiEditorShellStatus GetInspectorRecord(UiEditorInspectorRecord *out_record);
    /**
     * @comment 查询 visual backend gate 状态。
     * @return 当前 Dear ImGui backend 状态。
     */
    UiEditorShellStatus GetVisualBackendStatus() const;
    /**
     * @comment 返回当前 shell state 快照。
     * @return 快照值。
     */
    UiEditorShellSnapshot Snapshot() const;

private:
    UiEditorShellStatus RecordStatus(UiEditorShellStatus status);
    void RecountPanels();
    UiEditorShellPanelRecord *FindPanel(UiEditorShellPanelId panel_id);
    const UiEditorLayoutNodeRecord *FindLayoutNode(std::uint32_t node_id) const;
    void ResetLayoutState();
    UiEditorShellStatus LoadInspectorFromNode(const UiEditorLayoutNodeRecord &node);

    std::array<UiEditorShellPanelRecord, UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT> panels_;
    UiEditorLayoutDocument layout_document_;
    UiEditorInspectorRecord inspector_record_;
    UiEditorShellSnapshot snapshot_;
};
}
