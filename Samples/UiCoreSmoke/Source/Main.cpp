// 模块: UiCoreSmokeSample
// 文件: Samples/UiCoreSmoke/Source/Main.cpp

#include "UiCoreSmokeSample.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>

#ifndef YU_UI_CORE_SMOKE_DEFAULT_LAYOUT
#define YU_UI_CORE_SMOKE_DEFAULT_LAYOUT "Samples/UiCoreSmoke/Layouts/SimpleWindow.YuUILayout.json"
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

    *layout_path = YU_UI_CORE_SMOKE_DEFAULT_LAYOUT;
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
        std::fprintf(stderr, "Usage: YuUiCoreSmokeSample [--layout <path>]\n");
        return 2;
    }

    std::string layout_text;
    if (!ReadTextFile(layout_path, &layout_text)) {
        std::fprintf(stderr, "YuUiCoreSmokeSample FAIL stage=layout_file\n");
        return 3;
    }

    ui_core_smoke_sample::UiCoreSmokeSampleInput input;
    input.layout_text = layout_text;
    ui_core_smoke_sample::UiCoreSmokeSampleResult result;
    if (!ui_core_smoke_sample::RunUiCoreSmokeSample(input, &result)) {
        std::fprintf(stderr, "YuUiCoreSmokeSample FAIL stage=%s\n", result.failure_stage);
        return 4;
    }

    std::printf(
        "YuUiCoreSmokeSample PASS nodes=%u containers=%u draws=%zu submitted=%zu renders=%llu\n",
        result.layout_node_count,
        result.layout_container_count,
        result.draw_element_count,
        result.submitted_entry_count,
        static_cast<unsigned long long>(result.render_submit_count));
    return 0;
}
