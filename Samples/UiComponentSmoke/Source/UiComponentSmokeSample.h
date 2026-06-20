// 模块: UiComponentSmokeSample
// 文件: Samples/UiComponentSmoke/Source/UiComponentSmokeSample.h

#pragma once

#include <cstdint>
#include <string_view>

namespace ui_component_smoke_sample {
/**
 * @comment 描述 Stage 2 component smoke sample 的输入。
 */
struct UiComponentSmokeSampleInput final {
    std::string_view layout_text{};
};

/**
 * @comment 描述 Stage 2 component smoke sample 的性能诊断输出。
 */
struct UiComponentPerformanceDiagnostics final {
    bool reported = false;
    std::uint32_t draw_call_count = 0U;
    std::uint32_t batch_count = 0U;
    std::uint32_t atlas_page_count = 0U;
    std::uint32_t layout_rebuild_count = 0U;
    std::uint32_t paint_rebuild_count = 0U;
    std::uint32_t list_cell_count = 0U;
};

/**
 * @comment 描述 Stage 2 component smoke sample 的运行结果。
 */
struct UiComponentSmokeSampleResult final {
    bool layout_loaded = false;
    bool node_tree_built = false;
    bool component_window_built = false;
    bool text_component_used = false;
    bool image_component_used = false;
    bool button_component_used = false;
    bool slider_component_used = false;
    bool grid_view_used = false;
    bool pass_reported = false;
    const char *failure_stage = "not_started";
    std::uint32_t layout_node_count = 0U;
    std::uint32_t text_draw_record_count = 0U;
    std::uint32_t image_draw_record_count = 0U;
    std::uint32_t button_activation_event_key = 0U;
    std::uint32_t slider_value_changed_event_key = 0U;
    std::uint32_t grid_item_count = 0U;
    std::uint32_t grid_visible_item_count = 0U;
    std::uint32_t grid_pool_cell_count = 0U;
    std::uint32_t grid_dirty_cell_count = 0U;
    float slider_normalized_value = 0.0F;
    UiComponentPerformanceDiagnostics performance_diagnostics;
};

/**
 * @comment 运行 Stage 2 component smoke sample。
 * @param input 输入 sample 数据。
 * @param result 输出 sample 结果。
 * @return sample 通过返回 true。
 */
bool RunUiComponentSmokeSample(
    const UiComponentSmokeSampleInput &input,
    UiComponentSmokeSampleResult *result);
}
