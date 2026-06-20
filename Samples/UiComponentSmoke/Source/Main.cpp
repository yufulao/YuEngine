// 模块: UiComponentSmokeSample
// 文件: Samples/UiComponentSmoke/Source/Main.cpp

#include "UiComponentSmokeSample.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>

#ifndef YU_UI_COMPONENT_SMOKE_DEFAULT_LAYOUT
#define YU_UI_COMPONENT_SMOKE_DEFAULT_LAYOUT "Samples/UiComponentSmoke/Layouts/ComponentWindow.YuUILayout.json"
#endif

namespace {
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

bool ParseLayoutArgument(int argc, char **argv, std::filesystem::path *layout_path) {
    if (layout_path == nullptr) {
        return false;
    }

    *layout_path = YU_UI_COMPONENT_SMOKE_DEFAULT_LAYOUT;
    for (int index = 1; index < argc; ++index) {
        const std::string argument(argv[index]);
        if (argument != "--layout") {
            return false;
        }

        if (index + 1 >= argc) {
            return false;
        }

        ++index;
        *layout_path = argv[index];
    }

    return true;
}
}

int main(int argc, char **argv) {
    std::filesystem::path layout_path;
    if (!ParseLayoutArgument(argc, argv, &layout_path)) {
        std::fprintf(stderr, "Usage: YuUiComponentSmokeSample [--layout <path>]\n");
        return 2;
    }

    std::string layout_text;
    if (!ReadTextFile(layout_path, &layout_text)) {
        std::fprintf(stderr, "YuUiComponentSmokeSample FAIL stage=layout_file\n");
        return 3;
    }

    ui_component_smoke_sample::UiComponentSmokeSampleInput input;
    input.layout_text = layout_text;
    ui_component_smoke_sample::UiComponentSmokeSampleResult result;
    if (!ui_component_smoke_sample::RunUiComponentSmokeSample(input, &result)) {
        std::fprintf(stderr, "YuUiComponentSmokeSample FAIL stage=%s\n", result.failure_stage);
        return 4;
    }

    std::printf(
        "YuUiComponentSmokeSample PASS nodes=%u text=%u image=%u button=%u slider=%.2f gridVisible=%u gridPool=%u gridDirty=%u diagDraw=%u diagBatches=%u diagAtlasPages=%u diagLayoutRebuild=%u diagPaintRebuild=%u diagListCells=%u\n",
        result.layout_node_count,
        result.text_draw_record_count,
        result.image_draw_record_count,
        result.button_activation_event_key,
        static_cast<double>(result.slider_normalized_value),
        result.grid_visible_item_count,
        result.grid_pool_cell_count,
        result.grid_dirty_cell_count,
        result.performance_diagnostics.draw_call_count,
        result.performance_diagnostics.batch_count,
        result.performance_diagnostics.atlas_page_count,
        result.performance_diagnostics.layout_rebuild_count,
        result.performance_diagnostics.paint_rebuild_count,
        result.performance_diagnostics.list_cell_count);
    return 0;
}
