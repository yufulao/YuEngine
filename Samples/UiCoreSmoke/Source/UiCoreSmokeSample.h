// 模块: UiCoreSmokeSample
// 文件: Samples/UiCoreSmoke/Source/UiCoreSmokeSample.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ui_core_smoke_sample {
/**
 * @comment 描述 UI Core smoke sample 的输入。
 */
struct UiCoreSmokeSampleInput final {
    std::string_view layout_text{};
};

/**
 * @comment 描述 UI Core smoke sample 的运行结果。
 */
struct UiCoreSmokeSampleResult final {
    bool layout_loaded = false;
    bool node_tree_built = false;
    bool layout_passed = false;
    bool draw_list_built = false;
    bool render_submitted = false;
    bool pass_reported = false;
    const char *failure_stage = "not_started";
    std::uint32_t layout_node_count = 0U;
    std::uint32_t layout_container_count = 0U;
    std::size_t draw_element_count = 0U;
    std::size_t submitted_entry_count = 0U;
    std::uint64_t render_submit_count = 0U;
};

/**
 * @comment 描述 UI Core smoke sample 的验证命令记录。
 */
struct UiCoreSmokeValidationRoute final {
    const char *configure_command = "";
    const char *build_command = "";
    const char *test_command = "";
    const char *sample_command = "";
    const char *validation_doc_path = "";
    bool configure_command_available = false;
    bool build_command_available = false;
    bool test_command_available = false;
    bool sample_command_available = false;
    bool validation_doc_available = false;
};

/**
 * @comment 运行 UI Core smoke sample。
 * @param input 输入 sample 数据。
 * @param result 输出 sample 结果。
 * @return sample 通过返回 true。
 */
bool RunUiCoreSmokeSample(const UiCoreSmokeSampleInput &input, UiCoreSmokeSampleResult *result);

/**
 * @comment 构建 UI Core smoke sample 的验证命令记录。
 * @param route 输出验证命令记录。
 * @return 构建成功返回 true。
 */
bool BuildUiCoreSmokeValidationRoute(UiCoreSmokeValidationRoute *route);
}
