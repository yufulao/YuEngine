// 模块: Tests UiEditor
// 文件: Tests/UiEditor/UiEditorTests.cpp

#include <array>
#include <cstdio>
#include <string>
#include <string_view>

#include "YuEngine/UiEditor/UiEditorShellState.h"

using yuengine::uieditor::UiEditorInspectorRecord;
using yuengine::uieditor::UiEditorLayoutNodeRecord;
using yuengine::uieditor::UiEditorPreviewSafeArea;
using yuengine::uieditor::UiEditorPreviewVariantDesc;
using yuengine::uieditor::UiEditorPreviewVariantRecord;
using yuengine::uieditor::UiEditorPreviewViewportDesc;
using yuengine::uieditor::UiEditorPreviewViewportRecord;
using yuengine::uieditor::UiEditorShellPanelId;
using yuengine::uieditor::UiEditorShellPanelRecord;
using yuengine::uieditor::UiEditorShellSnapshot;
using yuengine::uieditor::UiEditorShellState;
using yuengine::uieditor::UiEditorShellStatus;
using yuengine::uieditor::UI_EDITOR_LAYOUT_MAX_NODE_COUNT;
using yuengine::uieditor::UI_EDITOR_PREVIEW_VARIANT_CAPACITY;
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
constexpr std::string_view TEST_LAYOUT_LOAD =
    "UiEditor_LayoutAsset_LoadsHierarchyAndInspector";
constexpr std::string_view TEST_LAYOUT_SELECT =
    "UiEditor_LayoutAsset_SelectNodeUpdatesInspector";
constexpr std::string_view TEST_LAYOUT_DUPLICATE =
    "UiEditor_LayoutAsset_RejectsDuplicateNodeId";
constexpr std::string_view TEST_LAYOUT_MISSING_PARENT =
    "UiEditor_LayoutAsset_RejectsMissingParent";
constexpr std::string_view TEST_PREVIEW_VARIANTS =
    "UiEditor_PreviewVariants_ResolutionDpiSafeAreaExport";
constexpr std::string_view TEST_PREVIEW_VARIANT_INVALID =
    "UiEditor_PreviewVariants_RejectInvalidSafeAreaWithoutMutation";
constexpr std::string_view TEST_PREVIEW_RENDER =
    "UiEditor_PreviewViewport_RendersLoadedLayoutThroughRuntimePath";
constexpr std::string_view TEST_PREVIEW_BEFORE_LAYOUT =
    "UiEditor_PreviewViewport_RejectsBeforeLayoutLoad";
constexpr std::string_view TEST_PREVIEW_INVALID_VIEWPORT =
    "UiEditor_PreviewViewport_RejectsInvalidViewport";
constexpr std::string_view TEST_PREVIEW_WITHOUT_DEAR_IMGUI =
    "UiEditor_PreviewViewport_WorksWithoutDearImGuiBackend";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected exactly one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::string_view SAMPLE_LAYOUT_TEXT = R"({
  "schema": "YuEngine.UI.Layout",
  "version": 1,
  "layoutId": "UiCoreSmoke.SimpleWindow",
  "rootNodeId": 1,
  "nodes": [
    {
      "nodeId": 1,
      "name": "Root",
      "type": "Root",
      "parentNodeId": 0,
      "order": 0
    },
    {
      "nodeId": 2,
      "name": "MainWindow",
      "type": "Window",
      "parentNodeId": 1,
      "order": 0
    },
    {
      "nodeId": 3,
      "name": "TitleBar",
      "type": "TitleBar",
      "parentNodeId": 2,
      "order": 0
    }
  ],
  "resources": []
})";
constexpr std::string_view DUPLICATE_NODE_LAYOUT_TEXT = R"({
  "schema": "YuEngine.UI.Layout",
  "version": 1,
  "layoutId": "UiEditor.Duplicate",
  "rootNodeId": 1,
  "nodes": [
    {
      "nodeId": 1,
      "name": "Root",
      "type": "Root",
      "parentNodeId": 0,
      "order": 0
    },
    {
      "nodeId": 1,
      "name": "Duplicate",
      "type": "Window",
      "parentNodeId": 1,
      "order": 0
    }
  ],
  "resources": []
})";
constexpr std::string_view MISSING_PARENT_LAYOUT_TEXT = R"({
  "schema": "YuEngine.UI.Layout",
  "version": 1,
  "layoutId": "UiEditor.MissingParent",
  "rootNodeId": 1,
  "nodes": [
    {
      "nodeId": 1,
      "name": "Root",
      "type": "Root",
      "parentNodeId": 0,
      "order": 0
    },
    {
      "nodeId": 2,
      "name": "Child",
      "type": "Window",
      "parentNodeId": 99,
      "order": 0
    }
  ],
  "resources": []
})";

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

int LoadSampleLayout(UiEditorShellState &shell_state) {
    UiEditorShellStatus status = shell_state.OpenDefaultPlaceholderPanels();
    int ret_code = ExpectStatus(status, UiEditorShellStatus::Success, "open placeholders failed");
    if (ret_code != 0) {
        return ret_code;
    }

    status = shell_state.LoadLayoutAsset(SAMPLE_LAYOUT_TEXT);
    return ExpectStatus(status, UiEditorShellStatus::Success, "load layout asset failed");
}

int ExportSampleHierarchy(
    UiEditorShellState &shell_state,
    std::array<UiEditorLayoutNodeRecord, UI_EDITOR_LAYOUT_MAX_NODE_COUNT> &nodes,
    std::uint32_t *out_node_count) {
    const UiEditorShellStatus status = shell_state.ExportHierarchyNodes(
        nodes.data(),
        static_cast<std::uint32_t>(nodes.size()),
        out_node_count);
    return ExpectStatus(status, UiEditorShellStatus::Success, "export hierarchy failed");
}

int UiEditorLayoutAssetLoadsHierarchyAndInspector() {
    UiEditorShellState shell_state;
    int ret_code = LoadSampleLayout(shell_state);
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (!snapshot.layout_loaded || snapshot.loaded_node_count != 3U) {
        return Fail("layout snapshot did not report loaded nodes");
    }

    if (snapshot.placeholder_panel_count != 1U) {
        return Fail("hierarchy and inspector did not switch away from placeholders");
    }

    std::array<UiEditorLayoutNodeRecord, UI_EDITOR_LAYOUT_MAX_NODE_COUNT> nodes{};
    std::uint32_t node_count = 0U;
    ret_code = ExportSampleHierarchy(shell_state, nodes, &node_count);
    if (ret_code != 0) {
        return ret_code;
    }

    if (node_count != 3U) {
        return Fail("hierarchy node count mismatch");
    }

    if (nodes[0U].node_id != 1U || nodes[0U].parent_node_id != 0U) {
        return Fail("root hierarchy node mismatch");
    }

    if (std::string_view(nodes[1U].name) != "MainWindow" || std::string_view(nodes[1U].type) != "Window") {
        return Fail("window hierarchy record mismatch");
    }

    if (nodes[2U].parent_node_id != 2U || std::string_view(nodes[2U].name) != "TitleBar") {
        return Fail("title hierarchy record mismatch");
    }

    UiEditorInspectorRecord inspector;
    const UiEditorShellStatus inspector_status = shell_state.GetInspectorRecord(&inspector);
    ret_code = ExpectStatus(inspector_status, UiEditorShellStatus::Success, "inspector query failed");
    if (ret_code != 0) {
        return ret_code;
    }

    if (!inspector.has_selection || inspector.node_id != 1U) {
        return Fail("inspector did not select root node after load");
    }

    if (std::string_view(inspector.layout_id) != "UiCoreSmoke.SimpleWindow") {
        return Fail("inspector layout id mismatch");
    }

    return 0;
}

int UiEditorLayoutAssetSelectNodeUpdatesInspector() {
    UiEditorShellState shell_state;
    int ret_code = LoadSampleLayout(shell_state);
    if (ret_code != 0) {
        return ret_code;
    }

    UiEditorShellStatus status = shell_state.SelectLayoutNode(2U);
    ret_code = ExpectStatus(status, UiEditorShellStatus::Success, "select layout node failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiEditorInspectorRecord inspector;
    status = shell_state.GetInspectorRecord(&inspector);
    ret_code = ExpectStatus(status, UiEditorShellStatus::Success, "inspector query failed");
    if (ret_code != 0) {
        return ret_code;
    }

    if (inspector.node_id != 2U || inspector.parent_node_id != 1U || inspector.order != 0U) {
        return Fail("inspector node identity mismatch");
    }

    if (std::string_view(inspector.name) != "MainWindow") {
        return Fail("inspector node name mismatch");
    }

    if (std::string_view(inspector.type) != "Window") {
        return Fail("inspector node type mismatch");
    }

    return 0;
}

int UiEditorLayoutAssetRejectsDuplicateNodeId() {
    UiEditorShellState shell_state;
    const UiEditorShellStatus status = shell_state.LoadLayoutAsset(DUPLICATE_NODE_LAYOUT_TEXT);
    int ret_code = ExpectStatus(status, UiEditorShellStatus::DuplicateNodeId, "duplicate node id was not rejected");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (snapshot.layout_loaded || snapshot.loaded_node_count != 0U) {
        return Fail("duplicate layout mutated loaded state");
    }

    return 0;
}

int UiEditorLayoutAssetRejectsMissingParent() {
    UiEditorShellState shell_state;
    const UiEditorShellStatus status = shell_state.LoadLayoutAsset(MISSING_PARENT_LAYOUT_TEXT);
    int ret_code = ExpectStatus(status, UiEditorShellStatus::MissingParentNode, "missing parent was not rejected");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (snapshot.last_status != UiEditorShellStatus::MissingParentNode) {
        return Fail("missing parent status was not recorded");
    }

    return 0;
}

bool FloatClose(float left, float right) {
    float diff = left - right;
    if (diff < 0.0F) {
        diff = -diff;
    }

    return diff < 0.001F;
}

int UiEditorPreviewVariantsResolutionDpiSafeAreaExport() {
    UiEditorShellState shell_state;
    int ret_code = LoadSampleLayout(shell_state);
    if (ret_code != 0) {
        return ret_code;
    }

    UiEditorPreviewVariantDesc desktop_desc;
    desktop_desc.variant_id = 1U;
    desktop_desc.target_width = 1920U;
    desktop_desc.target_height = 1080U;
    desktop_desc.dpi_scale_percent = 100U;
    UiEditorShellStatus status = shell_state.RegisterPreviewVariant(desktop_desc);
    ret_code = ExpectStatus(status, UiEditorShellStatus::Success, "desktop preview variant failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiEditorPreviewVariantDesc phone_desc;
    phone_desc.variant_id = 2U;
    phone_desc.target_width = 390U;
    phone_desc.target_height = 844U;
    phone_desc.dpi_scale_percent = 200U;
    phone_desc.safe_area = UiEditorPreviewSafeArea{0U, 47U, 0U, 34U};
    status = shell_state.RegisterPreviewVariant(phone_desc);
    ret_code = ExpectStatus(status, UiEditorShellStatus::Success, "phone preview variant failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (snapshot.preview_variant_count != 2U) {
        return Fail("preview variant count mismatch");
    }

    std::array<UiEditorPreviewVariantRecord, UI_EDITOR_PREVIEW_VARIANT_CAPACITY> variants{};
    std::uint32_t variant_count = 0U;
    status = shell_state.ExportPreviewVariants(
        variants.data(),
        static_cast<std::uint32_t>(variants.size()),
        &variant_count);
    ret_code = ExpectStatus(status, UiEditorShellStatus::Success, "export preview variants failed");
    if (ret_code != 0) {
        return ret_code;
    }

    if (variant_count != 2U) {
        return Fail("exported preview variant count mismatch");
    }

    if (variants[0U].target_width != 1920U || variants[0U].previewed_node_count != 3U) {
        return Fail("desktop preview variant did not preserve layout data");
    }

    if (!FloatClose(variants[0U].logical_width, 1920.0F) || !FloatClose(variants[0U].logical_height, 1080.0F)) {
        return Fail("desktop preview logical rect mismatch");
    }

    if (!FloatClose(variants[1U].logical_y, 23.5F)) {
        return Fail("phone preview safe-area y mismatch");
    }

    if (!FloatClose(variants[1U].logical_width, 195.0F) || !FloatClose(variants[1U].logical_height, 381.5F)) {
        return Fail("phone preview logical rect mismatch");
    }

    return 0;
}

int UiEditorPreviewVariantsRejectInvalidSafeAreaWithoutMutation() {
    UiEditorShellState shell_state;
    int ret_code = LoadSampleLayout(shell_state);
    if (ret_code != 0) {
        return ret_code;
    }

    UiEditorPreviewVariantDesc invalid_desc;
    invalid_desc.variant_id = 3U;
    invalid_desc.target_width = 100U;
    invalid_desc.target_height = 100U;
    invalid_desc.safe_area = UiEditorPreviewSafeArea{60U, 0U, 60U, 0U};
    const UiEditorShellStatus status = shell_state.RegisterPreviewVariant(invalid_desc);
    ret_code = ExpectStatus(status, UiEditorShellStatus::InvalidPreviewVariant, "invalid safe area was accepted");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (snapshot.preview_variant_count != 0U) {
        return Fail("invalid preview variant mutated registry");
    }

    return 0;
}

int BuildDefaultPreviewViewport(
    UiEditorShellState &shell_state,
    UiEditorPreviewViewportRecord *record) {
    UiEditorPreviewViewportDesc desc;
    desc.viewport_width = 800U;
    desc.viewport_height = 600U;

    const UiEditorShellStatus status = shell_state.BuildRuntimePreviewViewport(desc, record);
    return ExpectStatus(status, UiEditorShellStatus::Success, "preview viewport build failed");
}

int UiEditorPreviewViewportRendersLoadedLayoutThroughRuntimePath() {
    UiEditorShellState shell_state;
    int ret_code = LoadSampleLayout(shell_state);
    if (ret_code != 0) {
        return ret_code;
    }

    UiEditorPreviewViewportRecord record;
    ret_code = BuildDefaultPreviewViewport(shell_state, &record);
    if (ret_code != 0) {
        return ret_code;
    }

    if (!record.is_ready || !record.uses_headless_rendercore_path) {
        return Fail("preview route did not report ready headless RenderCore path");
    }

    if (record.viewport_width != 800U || record.viewport_height != 600U) {
        return Fail("preview viewport dimensions mismatch");
    }

    if (record.layout_node_count != 3U || record.layout_container_count != 2U) {
        return Fail("preview layout counts mismatch");
    }

    if (record.draw_element_count != 2U) {
        return Fail("preview draw element count mismatch");
    }

    if (record.submitted_entry_count != static_cast<std::size_t>(2U)) {
        return Fail("preview submitted entry count mismatch");
    }

    if (record.render_submit_count != static_cast<std::uint64_t>(2U)) {
        return Fail("preview render submit count mismatch");
    }

    if (std::string_view(record.layout_id) != "UiCoreSmoke.SimpleWindow") {
        return Fail("preview layout id mismatch");
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (!snapshot.preview_ready || snapshot.preview_draw_element_count != 2U) {
        return Fail("preview snapshot did not record ready path");
    }

    if (snapshot.placeholder_panel_count != 0U) {
        return Fail("preview panel stayed placeholder after render path");
    }

    std::array<UiEditorShellPanelRecord, UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT> panels{};
    ret_code = ExportDefaultPanels(shell_state, panels);
    if (ret_code != 0) {
        return ret_code;
    }

    if (panels[2U].panel_id != UiEditorShellPanelId::Preview) {
        return Fail("preview panel index mismatch");
    }

    if (panels[2U].is_placeholder) {
        return Fail("preview panel did not switch away from placeholder");
    }

    return 0;
}

int UiEditorPreviewViewportRejectsBeforeLayoutLoad() {
    UiEditorShellState shell_state;
    UiEditorPreviewViewportDesc desc;
    desc.viewport_width = 800U;
    desc.viewport_height = 600U;
    UiEditorPreviewViewportRecord record;

    const UiEditorShellStatus status = shell_state.BuildRuntimePreviewViewport(desc, &record);
    int ret_code = ExpectStatus(
        status,
        UiEditorShellStatus::LayoutNotLoaded,
        "preview before layout did not fail explicitly");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (snapshot.preview_ready || record.is_ready) {
        return Fail("preview before layout mutated ready state");
    }

    return 0;
}

int UiEditorPreviewViewportRejectsInvalidViewport() {
    UiEditorShellState shell_state;
    int ret_code = LoadSampleLayout(shell_state);
    if (ret_code != 0) {
        return ret_code;
    }

    UiEditorPreviewViewportDesc desc;
    desc.viewport_width = 0U;
    desc.viewport_height = 600U;
    UiEditorPreviewViewportRecord record;
    const UiEditorShellStatus status = shell_state.BuildRuntimePreviewViewport(desc, &record);
    ret_code = ExpectStatus(
        status,
        UiEditorShellStatus::InvalidPreviewViewport,
        "invalid preview viewport was not rejected");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorShellSnapshot snapshot = shell_state.Snapshot();
    if (!snapshot.layout_loaded || snapshot.preview_ready || record.is_ready) {
        return Fail("invalid preview viewport mutated layout or preview state");
    }

    return 0;
}

int UiEditorPreviewViewportWorksWithoutDearImGuiBackend() {
    UiEditorShellState shell_state;
    int ret_code = LoadSampleLayout(shell_state);
    if (ret_code != 0) {
        return ret_code;
    }

    UiEditorShellStatus status = shell_state.GetVisualBackendStatus();
    ret_code = ExpectStatus(
        status,
        UiEditorShellStatus::DearImGuiUnavailable,
        "Dear ImGui gate was not unavailable before preview");
    if (ret_code != 0) {
        return ret_code;
    }

    UiEditorPreviewViewportRecord record;
    ret_code = BuildDefaultPreviewViewport(shell_state, &record);
    if (ret_code != 0) {
        return ret_code;
    }

    status = shell_state.GetVisualBackendStatus();
    ret_code = ExpectStatus(
        status,
        UiEditorShellStatus::DearImGuiUnavailable,
        "headless preview changed Dear ImGui backend gate");
    if (ret_code != 0) {
        return ret_code;
    }

    if (!record.uses_headless_rendercore_path || record.render_submit_count == 0U) {
        return Fail("headless preview did not submit through RenderCore");
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

    if (test_name == TEST_LAYOUT_LOAD) {
        return UiEditorLayoutAssetLoadsHierarchyAndInspector();
    }

    if (test_name == TEST_LAYOUT_SELECT) {
        return UiEditorLayoutAssetSelectNodeUpdatesInspector();
    }

    if (test_name == TEST_LAYOUT_DUPLICATE) {
        return UiEditorLayoutAssetRejectsDuplicateNodeId();
    }

    if (test_name == TEST_LAYOUT_MISSING_PARENT) {
        return UiEditorLayoutAssetRejectsMissingParent();
    }

    if (test_name == TEST_PREVIEW_VARIANTS) {
        return UiEditorPreviewVariantsResolutionDpiSafeAreaExport();
    }

    if (test_name == TEST_PREVIEW_VARIANT_INVALID) {
        return UiEditorPreviewVariantsRejectInvalidSafeAreaWithoutMutation();
    }

    if (test_name == TEST_PREVIEW_RENDER) {
        return UiEditorPreviewViewportRendersLoadedLayoutThroughRuntimePath();
    }

    if (test_name == TEST_PREVIEW_BEFORE_LAYOUT) {
        return UiEditorPreviewViewportRejectsBeforeLayoutLoad();
    }

    if (test_name == TEST_PREVIEW_INVALID_VIEWPORT) {
        return UiEditorPreviewViewportRejectsInvalidViewport();
    }

    if (test_name == TEST_PREVIEW_WITHOUT_DEAR_IMGUI) {
        return UiEditorPreviewViewportWorksWithoutDearImGuiBackend();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
