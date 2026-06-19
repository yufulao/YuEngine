// 模块: Tests UiEditor
// 文件: Tests/UiEditor/UiEditorTests.cpp

#include <array>
#include <cstdio>
#include <string>
#include <string_view>

#include "YuEngine/UiEditor/UiEditorShellState.h"

using yuengine::uieditor::UiEditorShellPanelId;
using yuengine::uieditor::UiEditorShellPanelRecord;
using yuengine::uieditor::UiEditorShellSnapshot;
using yuengine::uieditor::UiEditorShellState;
using yuengine::uieditor::UiEditorShellStatus;
using yuengine::uieditor::UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT;

namespace {
constexpr std::string_view TEST_DEFAULT_PLACEHOLDERS =
    "UiEditor_ShellState_DefaultPlaceholdersOpen";
constexpr std::string_view TEST_CLOSE_PANEL =
    "UiEditor_ShellState_ClosePanelUpdatesSnapshot";
constexpr std::string_view TEST_SMALL_OUTPUT =
    "UiEditor_ShellState_RejectsSmallPanelOutput";
constexpr std::string_view TEST_BACKEND_GATE =
    "UiEditor_ShellState_DearImGuiBackendGateIsExplicit";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected exactly one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";

int Fail(const std::string &message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int ExpectStatus(UiEditorShellStatus status, UiEditorShellStatus expected_status, std::string_view message) {
    if (status == expected_status) {
        return 0;
    }

    return Fail(std::string(message));
}

int ExportDefaultPanels(
    UiEditorShellState &shell_state,
    std::array<UiEditorShellPanelRecord, UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT> &records) {
    std::uint32_t panel_count = 0U;
    const UiEditorShellStatus status = shell_state.ExportPanels(
        records.data(),
        static_cast<std::uint32_t>(records.size()),
        &panel_count);
    int ret_code = ExpectStatus(status, UiEditorShellStatus::Success, "export panels failed");
    if (ret_code != 0) {
        return ret_code;
    }

    if (panel_count != UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT) {
        return Fail("panel count mismatch");
    }

    return 0;
}

int UiEditorShellDefaultPlaceholdersOpen() {
    UiEditorShellState shell_state;
    UiEditorShellStatus status = shell_state.OpenDefaultPlaceholderPanels();
    int ret_code = ExpectStatus(status, UiEditorShellStatus::Success, "open placeholders failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (snapshot.panel_count != UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT) {
        return Fail("snapshot panel count mismatch");
    }

    if (snapshot.open_panel_count != UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT) {
        return Fail("default placeholders were not open");
    }

    if (snapshot.placeholder_panel_count != UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT) {
        return Fail("placeholder panel count mismatch");
    }

    std::array<UiEditorShellPanelRecord, UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT> records{};
    ret_code = ExportDefaultPanels(shell_state, records);
    if (ret_code != 0) {
        return ret_code;
    }

    if (records[0U].panel_id != UiEditorShellPanelId::Hierarchy) {
        return Fail("hierarchy panel was not first");
    }

    if (records[1U].panel_id != UiEditorShellPanelId::Inspector) {
        return Fail("inspector panel was not second");
    }

    if (records[2U].panel_id != UiEditorShellPanelId::Preview) {
        return Fail("preview panel was not third");
    }

    if (std::string_view(records[0U].panel_name) != "Hierarchy") {
        return Fail("hierarchy panel name mismatch");
    }

    if (std::string_view(records[1U].panel_name) != "Inspector") {
        return Fail("inspector panel name mismatch");
    }

    if (std::string_view(records[2U].panel_name) != "Preview") {
        return Fail("preview panel name mismatch");
    }

    return 0;
}

int UiEditorShellClosePanelUpdatesSnapshot() {
    UiEditorShellState shell_state;
    UiEditorShellStatus status = shell_state.OpenDefaultPlaceholderPanels();
    int ret_code = ExpectStatus(status, UiEditorShellStatus::Success, "open placeholders failed");
    if (ret_code != 0) {
        return ret_code;
    }

    status = shell_state.SetPanelOpen(UiEditorShellPanelId::Preview, false);
    ret_code = ExpectStatus(status, UiEditorShellStatus::Success, "close preview failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (snapshot.open_panel_count != 2U) {
        return Fail("closed preview did not update open count");
    }

    std::array<UiEditorShellPanelRecord, UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT> records{};
    ret_code = ExportDefaultPanels(shell_state, records);
    if (ret_code != 0) {
        return ret_code;
    }

    if (records[2U].is_open) {
        return Fail("preview panel still reported open");
    }

    return 0;
}

int UiEditorShellRejectsSmallPanelOutput() {
    UiEditorShellState shell_state;
    std::array<UiEditorShellPanelRecord, 2U> records{};
    std::uint32_t panel_count = 0U;

    const UiEditorShellStatus status = shell_state.ExportPanels(
        records.data(),
        static_cast<std::uint32_t>(records.size()),
        &panel_count);
    int ret_code = ExpectStatus(status, UiEditorShellStatus::OutputCapacityExceeded, "small output did not fail explicitly");
    if (ret_code != 0) {
        return ret_code;
    }

    if (panel_count != UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT) {
        return Fail("small output did not report required panel count");
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (snapshot.last_status != UiEditorShellStatus::OutputCapacityExceeded) {
        return Fail("small output status was not recorded");
    }

    return 0;
}

int UiEditorShellDearImGuiBackendGateIsExplicit() {
    UiEditorShellState shell_state;
    const UiEditorShellStatus backend_status = shell_state.GetVisualBackendStatus();
    int ret_code = ExpectStatus(
        backend_status,
        UiEditorShellStatus::DearImGuiUnavailable,
        "dear imgui backend gate was not explicit");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (snapshot.dear_imgui_backend_ready) {
        return Fail("dear imgui backend was reported ready without provenance");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::string_view test_name(argv[1]);
    if (test_name == TEST_DEFAULT_PLACEHOLDERS) {
        return UiEditorShellDefaultPlaceholdersOpen();
    }

    if (test_name == TEST_CLOSE_PANEL) {
        return UiEditorShellClosePanelUpdatesSnapshot();
    }

    if (test_name == TEST_SMALL_OUTPUT) {
        return UiEditorShellRejectsSmallPanelOutput();
    }

    if (test_name == TEST_BACKEND_GATE) {
        return UiEditorShellDearImGuiBackendGateIsExplicit();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
