// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Src/UiEditorShellState.cpp

#include "YuEngine/UiEditor/UiEditorShellState.h"

namespace yuengine::uieditor {
namespace {
constexpr const char *PANEL_HIERARCHY_NAME = "Hierarchy";
constexpr const char *PANEL_INSPECTOR_NAME = "Inspector";
constexpr const char *PANEL_PREVIEW_NAME = "Preview";

UiEditorShellPanelRecord MakePanelRecord(UiEditorShellPanelId panel_id, const char *panel_name) {
    UiEditorShellPanelRecord record;
    record.panel_id = panel_id;
    record.panel_name = panel_name;
    record.is_open = false;
    record.is_placeholder = true;
    return record;
}
}

UiEditorShellState::UiEditorShellState()
    : panels_{
          MakePanelRecord(UiEditorShellPanelId::Hierarchy, PANEL_HIERARCHY_NAME),
          MakePanelRecord(UiEditorShellPanelId::Inspector, PANEL_INSPECTOR_NAME),
          MakePanelRecord(UiEditorShellPanelId::Preview, PANEL_PREVIEW_NAME)},
      snapshot_{
          UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT,
          0U,
          UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT,
          UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT,
          false,
          UiEditorShellStatus::Success} {
}

UiEditorShellStatus UiEditorShellState::OpenDefaultPlaceholderPanels() {
    for (UiEditorShellPanelRecord &panel : panels_) {
        panel.is_open = true;
    }

    RecountPanels();
    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::SetPanelOpen(UiEditorShellPanelId panel_id, bool is_open) {
    UiEditorShellPanelRecord *panel = FindPanel(panel_id);
    if (panel == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidPanel);
    }

    panel->is_open = is_open;
    RecountPanels();
    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::ExportPanels(
    UiEditorShellPanelRecord *output_records,
    std::uint32_t output_capacity,
    std::uint32_t *out_count) {
    if (out_count == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidOutput);
    }

    *out_count = snapshot_.panel_count;
    if (output_capacity < snapshot_.panel_count) {
        return RecordStatus(UiEditorShellStatus::OutputCapacityExceeded);
    }

    if (output_records == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidOutput);
    }

    for (std::uint32_t index = 0U; index < snapshot_.panel_count; ++index) {
        output_records[index] = panels_[index];
    }

    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::GetVisualBackendStatus() const {
    return UiEditorShellStatus::DearImGuiUnavailable;
}

UiEditorShellSnapshot UiEditorShellState::Snapshot() const {
    return snapshot_;
}

UiEditorShellStatus UiEditorShellState::RecordStatus(UiEditorShellStatus status) {
    snapshot_.last_status = status;
    return status;
}

void UiEditorShellState::RecountPanels() {
    std::uint32_t open_panel_count = 0U;
    std::uint32_t placeholder_panel_count = 0U;

    for (const UiEditorShellPanelRecord &panel : panels_) {
        if (panel.is_open) {
            ++open_panel_count;
        }

        if (panel.is_placeholder) {
            ++placeholder_panel_count;
        }
    }

    snapshot_.open_panel_count = open_panel_count;
    snapshot_.placeholder_panel_count = placeholder_panel_count;
}

UiEditorShellPanelRecord *UiEditorShellState::FindPanel(UiEditorShellPanelId panel_id) {
    if (panel_id == UiEditorShellPanelId::Invalid) {
        return nullptr;
    }

    for (UiEditorShellPanelRecord &panel : panels_) {
        if (panel.panel_id == panel_id) {
            return &panel;
        }
    }

    return nullptr;
}
}
