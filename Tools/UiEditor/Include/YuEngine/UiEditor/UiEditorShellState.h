// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Include/YuEngine/UiEditor/UiEditorShellState.h

#pragma once

#include <array>
#include <cstdint>

namespace yuengine::uieditor {
constexpr std::uint32_t UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT = 3U;

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
    DearImGuiUnavailable
};

struct UiEditorShellPanelRecord {
    UiEditorShellPanelId panel_id = UiEditorShellPanelId::Invalid;
    const char *panel_name = "";
    bool is_open = false;
    bool is_placeholder = true;
};

struct UiEditorShellSnapshot {
    std::uint32_t panel_count = 0U;
    std::uint32_t open_panel_count = 0U;
    std::uint32_t placeholder_panel_count = 0U;
    std::uint32_t required_placeholder_count = 0U;
    bool dear_imgui_backend_ready = false;
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

    std::array<UiEditorShellPanelRecord, UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT> panels_;
    UiEditorShellSnapshot snapshot_;
};
}
