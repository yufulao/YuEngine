// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Src/Main.cpp

#include <cstdio>

#include "YuEngine/UiEditor/UiEditorShellState.h"

namespace {
constexpr char ERROR_SHELL_STATE_FAILED[] = "YuUiEditorShell failed to initialize shell state.\n";
constexpr char ERROR_DEAR_IMGUI_UNAVAILABLE[] = "YuUiEditorShell requires a reviewed Dear ImGui backend before visual docking can run.\n";
}

int main() {
    yuengine::uieditor::UiEditorShellState shell_state;
    const yuengine::uieditor::UiEditorShellStatus state_status = shell_state.OpenDefaultPlaceholderPanels();
    if (state_status != yuengine::uieditor::UiEditorShellStatus::Success) {
        std::fwrite(ERROR_SHELL_STATE_FAILED, sizeof(char), sizeof(ERROR_SHELL_STATE_FAILED) - 1U, stderr);
        return 1;
    }

    const yuengine::uieditor::UiEditorShellStatus backend_status = shell_state.GetVisualBackendStatus();
    if (backend_status == yuengine::uieditor::UiEditorShellStatus::DearImGuiUnavailable) {
        std::fwrite(ERROR_DEAR_IMGUI_UNAVAILABLE, sizeof(char), sizeof(ERROR_DEAR_IMGUI_UNAVAILABLE) - 1U, stderr);
        return 2;
    }

    return 0;
}
