// 模块: Tests UiComponentSmoke
// 文件: Tests/UiComponentSmoke/UiComponentSmokeSampleTests.cpp

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "UiComponentSmokeSample.h"

#ifndef YU_UI_COMPONENT_SMOKE_LAYOUT_PATH
#define YU_UI_COMPONENT_SMOKE_LAYOUT_PATH "Samples/UiComponentSmoke/Layouts/ComponentWindow.YuUILayout.json"
#endif

namespace {
constexpr const char *TEST_COMPONENT_WINDOW =
    "UiComponentSmokeSample_UsesStage2ComponentsInOneWindow";
constexpr const char *TEST_INVALID_LAYOUT =
    "UiComponentSmokeSample_RejectsInvalidLayoutBeforeComponents";
constexpr const char *TEST_GRID_VIRTUALIZATION =
    "UiComponentSmokeSample_KeepsGridViewListVirtualized";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
using TestFunction = int (*)();

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool FloatClose(float left, float right) {
    float diff = left - right;
    if (diff < 0.0F) {
        diff = -diff;
    }

    return diff < 0.0001F;
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

int RunSample(ui_component_smoke_sample::UiComponentSmokeSampleResult *result) {
    if (result == nullptr) {
        return Fail("missing sample result");
    }

    std::string layout_text;
    if (!ReadTextFile(YU_UI_COMPONENT_SMOKE_LAYOUT_PATH, &layout_text)) {
        return Fail("layout fixture file was not readable");
    }

    ui_component_smoke_sample::UiComponentSmokeSampleInput input;
    input.layout_text = layout_text;
    if (!ui_component_smoke_sample::RunUiComponentSmokeSample(input, result)) {
        return Fail(result->failure_stage);
    }

    return 0;
}

int UiComponentSmokeSampleUsesStage2ComponentsInOneWindow() {
    ui_component_smoke_sample::UiComponentSmokeSampleResult result;
    int ret_code = RunSample(&result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (!result.layout_loaded || !result.node_tree_built || !result.component_window_built) {
        return Fail("sample did not load one component window");
    }

    if (!result.text_component_used || !result.image_component_used || !result.button_component_used) {
        return Fail("sample did not use text image and button components");
    }

    if (!result.slider_component_used || !result.grid_view_used || !result.pass_reported) {
        return Fail("sample did not use slider and grid components");
    }

    if (result.layout_node_count != 11U) {
        return Fail("component window node count mismatch");
    }

    if (result.text_draw_record_count != 4U || result.image_draw_record_count != 1U) {
        return Fail("component draw record count mismatch");
    }

    if (result.button_activation_event_key != 900U) {
        return Fail("button activation route mismatch");
    }

    if (!FloatClose(result.slider_normalized_value, 0.25F)) {
        return Fail("slider normalized value mismatch");
    }

    return 0;
}

int UiComponentSmokeSampleRejectsInvalidLayoutBeforeComponents() {
    ui_component_smoke_sample::UiComponentSmokeSampleInput input;
    input.layout_text = "{\"schema\": \"YuEngine.UI.Layout\", \"layoutId\": \"Bad\"}";
    ui_component_smoke_sample::UiComponentSmokeSampleResult result;
    if (ui_component_smoke_sample::RunUiComponentSmokeSample(input, &result)) {
        return Fail("invalid layout unexpectedly passed");
    }

    if (result.layout_loaded || result.text_component_used || result.grid_view_used || result.pass_reported) {
        return Fail("invalid layout mutated component sample state");
    }

    if (std::string_view(result.failure_stage) != "layout_load") {
        return Fail("invalid layout did not fail at layout load");
    }

    return 0;
}

int UiComponentSmokeSampleKeepsGridViewListVirtualized() {
    ui_component_smoke_sample::UiComponentSmokeSampleResult result;
    int ret_code = RunSample(&result);
    if (ret_code != 0) {
        return ret_code;
    }

    if (!result.grid_view_used) {
        return Fail("grid view was not used");
    }

    if (result.grid_item_count != 1000U || result.grid_visible_item_count != 10U) {
        return Fail("grid item or visible count mismatch");
    }

    if (result.grid_pool_cell_count != 20U || result.grid_pool_cell_count >= result.grid_item_count) {
        return Fail("grid view did not stay virtualized");
    }

    if (result.grid_dirty_cell_count != 3U) {
        return Fail("grid view dirty cell count mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_COMPONENT_WINDOW) {
        return UiComponentSmokeSampleUsesStage2ComponentsInOneWindow();
    }

    if (name == TEST_INVALID_LAYOUT) {
        return UiComponentSmokeSampleRejectsInvalidLayoutBeforeComponents();
    }

    if (name == TEST_GRID_VIRTUALIZATION) {
        return UiComponentSmokeSampleKeepsGridViewListVirtualized();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
