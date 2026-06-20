// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Include/YuEngine/UiEditor/UiEditorGridViewDataSimulation.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiEditor/UiEditorComponentTemplate.h"

namespace yuengine::uieditor {
constexpr std::uint32_t UI_EDITOR_GRID_VIEW_DATA_SIMULATION_MAX_VISIBLE_CELL_COUNT = 64U;
constexpr std::uint32_t UI_EDITOR_GRID_VIEW_DATA_SIMULATION_MAX_BUFFER_CELL_COUNT = 64U;
constexpr std::uint32_t UI_EDITOR_GRID_VIEW_DATA_SIMULATION_MAX_CELL_COUNT =
    UI_EDITOR_GRID_VIEW_DATA_SIMULATION_MAX_VISIBLE_CELL_COUNT +
    UI_EDITOR_GRID_VIEW_DATA_SIMULATION_MAX_BUFFER_CELL_COUNT;

enum class UiEditorGridViewDataSimulationStatus {
    Success = 0,
    InvalidInput,
    InvalidTemplate,
    InvalidSampleCount,
    InvalidVisibleCellCount,
    InvalidBufferCellCount,
    InvalidOutput,
    OutputCapacityExceeded
};

struct UiEditorGridViewDataSimulationDesc final {
    UiEditorComponentTemplateRecord component_template;
    std::uint32_t sample_count = 0U;
    std::uint32_t first_visible_item_index = 0U;
    std::uint32_t visible_cell_count = 0U;
    std::uint32_t buffer_cell_count = 0U;
};

struct UiEditorGridViewDataCellRecord final {
    std::uint32_t node_id = 0U;
    std::uint32_t item_index = 0U;
    std::uint32_t visual_order = 0U;
    bool visible_cell = false;
    bool buffer_cell = false;
};

struct UiEditorGridViewDataSimulationRecord final {
    UiEditorComponentTemplateKind component_kind = UiEditorComponentTemplateKind::Invalid;
    std::uint32_t node_id = 0U;
    std::uint32_t sample_count = 0U;
    std::uint32_t first_visible_item_index = 0U;
    std::uint32_t requested_visible_cell_count = 0U;
    std::uint32_t requested_buffer_cell_count = 0U;
    std::uint32_t written_cell_count = 0U;
    std::uint32_t written_visible_cell_count = 0U;
    std::uint32_t written_buffer_cell_count = 0U;
    std::uint32_t last_visible_item_index = 0U;
};

struct UiEditorGridViewDataSimulationResult final {
    UiEditorGridViewDataSimulationStatus status = UiEditorGridViewDataSimulationStatus::Success;
    std::uint32_t node_id = 0U;
    std::uint32_t sample_count = 0U;
    std::uint32_t required_cell_count = 0U;
    std::uint32_t written_cell_count = 0U;
    std::uint32_t written_visible_cell_count = 0U;
    std::uint32_t written_buffer_cell_count = 0U;

    /**
     * @comment 检查 GridView data simulation 是否成功生成 visible/buffer cell 记录。
     * @return 成功生成完整 cell 记录时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiEditorGridViewDataSimulationStatus::Success;
    }
};

class UiEditorGridViewDataSimulationFactory final {
public:
    /**
     * @comment 创建 editor-only GridView data simulation 记录。
     * @param desc 输入 GridView sample 和 visible/buffer cell 描述。
     * @param out_record 输出 simulation summary。
     * @param out_cells 调用方持有的 cell preview buffer。
     * @param out_result 输出显式状态和计数。
     * @return 显式创建状态。
     */
    UiEditorGridViewDataSimulationStatus CreateGridViewDataSimulation(
        const UiEditorGridViewDataSimulationDesc &desc,
        UiEditorGridViewDataSimulationRecord *out_record,
        std::span<UiEditorGridViewDataCellRecord> out_cells,
        UiEditorGridViewDataSimulationResult *out_result) const;

private:
    UiEditorGridViewDataSimulationStatus ValidateDesc(
        const UiEditorGridViewDataSimulationDesc &desc,
        UiEditorGridViewDataSimulationResult *out_result) const;
    UiEditorGridViewDataSimulationStatus ValidateOutput(
        std::span<UiEditorGridViewDataCellRecord> out_cells,
        const UiEditorGridViewDataSimulationResult &result) const;
    void WriteRecord(
        const UiEditorGridViewDataSimulationDesc &desc,
        UiEditorGridViewDataSimulationRecord *out_record,
        const UiEditorGridViewDataSimulationResult &result) const;
    void WriteCells(
        const UiEditorGridViewDataSimulationDesc &desc,
        std::span<UiEditorGridViewDataCellRecord> out_cells,
        UiEditorGridViewDataSimulationResult *out_result) const;
    UiEditorGridViewDataSimulationResult MakeResult(
        const UiEditorGridViewDataSimulationDesc &desc) const;
};
}
