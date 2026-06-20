// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Src/UiEditorGridViewDataSimulation.cpp

#include "YuEngine/UiEditor/UiEditorGridViewDataSimulation.h"

namespace yuengine::uieditor {
namespace {
std::uint32_t CountRemainingItems(const UiEditorGridViewDataSimulationDesc &desc) {
    if (desc.first_visible_item_index >= desc.sample_count) {
        return 0U;
    }

    return desc.sample_count - desc.first_visible_item_index;
}

std::uint32_t CountRequestedCells(const UiEditorGridViewDataSimulationDesc &desc) {
    if (desc.visible_cell_count > UI_EDITOR_GRID_VIEW_DATA_SIMULATION_MAX_VISIBLE_CELL_COUNT) {
        return UI_EDITOR_GRID_VIEW_DATA_SIMULATION_MAX_CELL_COUNT;
    }

    if (desc.buffer_cell_count > UI_EDITOR_GRID_VIEW_DATA_SIMULATION_MAX_BUFFER_CELL_COUNT) {
        return UI_EDITOR_GRID_VIEW_DATA_SIMULATION_MAX_CELL_COUNT;
    }

    return desc.visible_cell_count + desc.buffer_cell_count;
}

std::uint32_t CountRequiredCells(const UiEditorGridViewDataSimulationDesc &desc) {
    const std::uint32_t requested_cell_count = CountRequestedCells(desc);
    const std::uint32_t remaining_item_count = CountRemainingItems(desc);
    if (remaining_item_count < requested_cell_count) {
        return remaining_item_count;
    }

    return requested_cell_count;
}

std::uint32_t CountVisibleCells(const UiEditorGridViewDataSimulationDesc &desc) {
    const std::uint32_t required_cell_count = CountRequiredCells(desc);
    if (required_cell_count < desc.visible_cell_count) {
        return required_cell_count;
    }

    return desc.visible_cell_count;
}

std::uint32_t CountBufferCells(const UiEditorGridViewDataSimulationDesc &desc) {
    const std::uint32_t required_cell_count = CountRequiredCells(desc);
    const std::uint32_t visible_cell_count = CountVisibleCells(desc);
    if (required_cell_count <= visible_cell_count) {
        return 0U;
    }

    return required_cell_count - visible_cell_count;
}

bool IsOutputStorageValid(std::span<UiEditorGridViewDataCellRecord> out_cells) {
    if (!out_cells.empty() && out_cells.data() == nullptr) {
        return false;
    }

    return true;
}
}

UiEditorGridViewDataSimulationStatus UiEditorGridViewDataSimulationFactory::CreateGridViewDataSimulation(
    const UiEditorGridViewDataSimulationDesc &desc,
    UiEditorGridViewDataSimulationRecord *out_record,
    std::span<UiEditorGridViewDataCellRecord> out_cells,
    UiEditorGridViewDataSimulationResult *out_result) const {
    if (out_result == nullptr) {
        return UiEditorGridViewDataSimulationStatus::InvalidInput;
    }

    *out_result = MakeResult(desc);

    const UiEditorGridViewDataSimulationStatus desc_status = ValidateDesc(desc, out_result);
    if (desc_status != UiEditorGridViewDataSimulationStatus::Success) {
        out_result->status = desc_status;
        return desc_status;
    }

    if (out_record == nullptr) {
        out_result->status = UiEditorGridViewDataSimulationStatus::InvalidOutput;
        return UiEditorGridViewDataSimulationStatus::InvalidOutput;
    }

    const UiEditorGridViewDataSimulationStatus output_status = ValidateOutput(out_cells, *out_result);
    if (output_status != UiEditorGridViewDataSimulationStatus::Success) {
        out_result->status = output_status;
        return output_status;
    }

    UiEditorGridViewDataSimulationRecord record{};
    WriteRecord(desc, &record, *out_result);
    WriteCells(desc, out_cells, out_result);
    *out_record = record;
    out_result->status = UiEditorGridViewDataSimulationStatus::Success;
    return UiEditorGridViewDataSimulationStatus::Success;
}

UiEditorGridViewDataSimulationStatus UiEditorGridViewDataSimulationFactory::ValidateDesc(
    const UiEditorGridViewDataSimulationDesc &desc,
    UiEditorGridViewDataSimulationResult *out_result) const {
    if (out_result == nullptr) {
        return UiEditorGridViewDataSimulationStatus::InvalidInput;
    }

    if (desc.component_template.kind != UiEditorComponentTemplateKind::GridView) {
        return UiEditorGridViewDataSimulationStatus::InvalidTemplate;
    }

    if (desc.component_template.default_state != UiEditorComponentTemplateDefaultState::GridViewReady) {
        return UiEditorGridViewDataSimulationStatus::InvalidTemplate;
    }

    if (desc.component_template.layout_node.node_id == 0U) {
        return UiEditorGridViewDataSimulationStatus::InvalidTemplate;
    }

    if (desc.sample_count == 0U) {
        return UiEditorGridViewDataSimulationStatus::InvalidSampleCount;
    }

    if (desc.first_visible_item_index >= desc.sample_count) {
        return UiEditorGridViewDataSimulationStatus::InvalidSampleCount;
    }

    if (desc.visible_cell_count == 0U) {
        return UiEditorGridViewDataSimulationStatus::InvalidVisibleCellCount;
    }

    if (desc.visible_cell_count > UI_EDITOR_GRID_VIEW_DATA_SIMULATION_MAX_VISIBLE_CELL_COUNT) {
        return UiEditorGridViewDataSimulationStatus::InvalidVisibleCellCount;
    }

    if (desc.buffer_cell_count > UI_EDITOR_GRID_VIEW_DATA_SIMULATION_MAX_BUFFER_CELL_COUNT) {
        return UiEditorGridViewDataSimulationStatus::InvalidBufferCellCount;
    }

    return UiEditorGridViewDataSimulationStatus::Success;
}

UiEditorGridViewDataSimulationStatus UiEditorGridViewDataSimulationFactory::ValidateOutput(
    std::span<UiEditorGridViewDataCellRecord> out_cells,
    const UiEditorGridViewDataSimulationResult &result) const {
    if (!IsOutputStorageValid(out_cells)) {
        return UiEditorGridViewDataSimulationStatus::InvalidOutput;
    }

    if (out_cells.size() < result.required_cell_count) {
        return UiEditorGridViewDataSimulationStatus::OutputCapacityExceeded;
    }

    return UiEditorGridViewDataSimulationStatus::Success;
}

void UiEditorGridViewDataSimulationFactory::WriteRecord(
    const UiEditorGridViewDataSimulationDesc &desc,
    UiEditorGridViewDataSimulationRecord *out_record,
    const UiEditorGridViewDataSimulationResult &result) const {
    if (out_record == nullptr) {
        return;
    }

    UiEditorGridViewDataSimulationRecord record{};
    record.component_kind = desc.component_template.kind;
    record.node_id = desc.component_template.layout_node.node_id;
    record.sample_count = desc.sample_count;
    record.first_visible_item_index = desc.first_visible_item_index;
    record.requested_visible_cell_count = desc.visible_cell_count;
    record.requested_buffer_cell_count = desc.buffer_cell_count;
    const std::uint32_t written_visible_cell_count = CountVisibleCells(desc);
    record.written_cell_count = result.required_cell_count;
    record.written_visible_cell_count = written_visible_cell_count;
    record.written_buffer_cell_count = CountBufferCells(desc);
    if (written_visible_cell_count != 0U) {
        record.last_visible_item_index =
            desc.first_visible_item_index + written_visible_cell_count - 1U;
    }

    *out_record = record;
}

void UiEditorGridViewDataSimulationFactory::WriteCells(
    const UiEditorGridViewDataSimulationDesc &desc,
    std::span<UiEditorGridViewDataCellRecord> out_cells,
    UiEditorGridViewDataSimulationResult *out_result) const {
    if (out_result == nullptr) {
        return;
    }

    const std::uint32_t visible_cell_count = CountVisibleCells(desc);
    const std::uint32_t required_cell_count = CountRequiredCells(desc);
    std::uint32_t cell_index = 0U;
    while (cell_index < required_cell_count) {
        UiEditorGridViewDataCellRecord cell{};
        cell.node_id = desc.component_template.layout_node.node_id;
        cell.item_index = desc.first_visible_item_index + cell_index;
        cell.visual_order = cell_index;
        cell.visible_cell = cell_index < visible_cell_count;
        cell.buffer_cell = !cell.visible_cell;
        out_cells[cell_index] = cell;
        ++cell_index;
    }

    out_result->written_cell_count = required_cell_count;
    out_result->written_visible_cell_count = visible_cell_count;
    out_result->written_buffer_cell_count = CountBufferCells(desc);
}

UiEditorGridViewDataSimulationResult UiEditorGridViewDataSimulationFactory::MakeResult(
    const UiEditorGridViewDataSimulationDesc &desc) const {
    UiEditorGridViewDataSimulationResult result{};
    result.node_id = desc.component_template.layout_node.node_id;
    result.sample_count = desc.sample_count;
    result.required_cell_count = CountRequiredCells(desc);
    return result;
}
}
