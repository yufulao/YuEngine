// 模块: Tests UiCoreSmoke
// 文件: Tests/UiCoreSmoke/UiCoreSmokeSampleTests.cpp

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "UiCoreSmokeSample.h"

#ifndef YU_UI_CORE_SMOKE_LAYOUT_PATH
#define YU_UI_CORE_SMOKE_LAYOUT_PATH "Samples/UiCoreSmoke/Layouts/SimpleWindow.YuUILayout.json"
#endif

namespace {
constexpr const char *TEST_SMOKE_PASS =
    "UiCoreSmokeSample_LoadsLayoutRendersWindowReportsPass";
constexpr const char *TEST_INVALID_LAYOUT =
    "UiCoreSmokeSample_RejectsInvalidLayoutWithoutRenderSubmission";
constexpr const char *TEST_VALIDATION_ROUTE =
    "UiCoreSmokeSample_ValidationRouteDocumentsCommands";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
using TestFunction = int (*)();

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool ReadTextFile(const std::filesystem::path &path, std::string *text) {
    if (text == nullptr) {
        return false;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::istreambuf_iterator<char> begin(file);
    std::istreambuf_iterator<char> end;
    std::string loaded_text(begin, end);
    if (loaded_text.empty()) {
        return false;
    }

    *text = std::move(loaded_text);
    return true;
}

bool ContainsText(std::string_view text, std::string_view expected) {
    return text.find(expected) != std::string_view::npos;
}

int UiCoreSmokeSampleLoadsLayoutRendersWindowReportsPass() {
    std::string layout_text;
    if (!ReadTextFile(YU_UI_CORE_SMOKE_LAYOUT_PATH, &layout_text)) {
        return Fail("layout fixture file was not readable");
    }

    ui_core_smoke_sample::UiCoreSmokeSampleInput input;
    input.layout_text = layout_text;
    ui_core_smoke_sample::UiCoreSmokeSampleResult result;
    if (!ui_core_smoke_sample::RunUiCoreSmokeSample(input, &result)) {
        return Fail(result.failure_stage);
    }

    if (!result.layout_loaded || !result.node_tree_built || !result.layout_passed) {
        return Fail("sample did not load and layout the UI tree");
    }

    if (!result.draw_list_built || !result.render_submitted || !result.pass_reported) {
        return Fail("sample did not render and report pass");
    }

    if (result.layout_node_count != 3U || result.layout_container_count != 2U) {
        return Fail("sample layout counts mismatch");
    }

    if (result.draw_element_count != 2U || result.submitted_entry_count != 2U) {
        return Fail("sample render submission counts mismatch");
    }

    if (result.render_submit_count != 2U) {
        return Fail("sample render backend count mismatch");
    }

    return 0;
}

int UiCoreSmokeSampleRejectsInvalidLayoutWithoutRenderSubmission() {
    ui_core_smoke_sample::UiCoreSmokeSampleInput input;
    input.layout_text = "{\"schema\": \"YuEngine.UI.Layout\", \"layoutId\": \"Bad\"}";
    ui_core_smoke_sample::UiCoreSmokeSampleResult result;
    if (ui_core_smoke_sample::RunUiCoreSmokeSample(input, &result)) {
        return Fail("invalid layout unexpectedly passed");
    }

    if (result.layout_loaded || result.render_submitted || result.pass_reported) {
        return Fail("invalid layout mutated sample pass state");
    }

    if (std::string_view(result.failure_stage) != "layout_load") {
        return Fail("invalid layout did not fail at layout load");
    }

    return 0;
}

int UiCoreSmokeSampleValidationRouteDocumentsCommands() {
    ui_core_smoke_sample::UiCoreSmokeValidationRoute route;
    if (!ui_core_smoke_sample::BuildUiCoreSmokeValidationRoute(&route)) {
        return Fail("validation route build failed");
    }

    if (!route.configure_command_available || !route.build_command_available) {
        return Fail("validation route missing configure or build command");
    }

    if (!route.test_command_available || !route.sample_command_available) {
        return Fail("validation route missing sample command");
    }

    if (!route.validation_doc_available) {
        return Fail("validation doc path was not recorded");
    }

    if (!ContainsText(route.test_command, "UiCoreSmokeSample_")) {
        return Fail("validation test filter mismatch");
    }

    if (!ContainsText(route.sample_command, "YuUiCoreSmokeSample")) {
        return Fail("validation sample command mismatch");
    }

    if (!ContainsText(route.validation_doc_path, "YUENGINE_UI_STAGE1_VALIDATION")) {
        return Fail("validation doc path mismatch");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> tests{
        {TEST_SMOKE_PASS, UiCoreSmokeSampleLoadsLayoutRendersWindowReportsPass},
        {TEST_INVALID_LAYOUT, UiCoreSmokeSampleRejectsInvalidLayoutWithoutRenderSubmission},
        {TEST_VALIDATION_ROUTE, UiCoreSmokeSampleValidationRouteDocumentsCommands}};

    const std::string_view test_name(argv[1]);
    const auto test = tests.find(test_name);
    if (test == tests.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test->second();
}
