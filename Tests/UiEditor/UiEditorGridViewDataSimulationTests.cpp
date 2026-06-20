// 模块: Tests UiEditor
// 文件: Tests/UiEditor/UiEditorGridViewDataSimulationTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiEditor/UiEditorComponentTemplate.h"
#include "YuEngine/UiEditor/UiEditorGridViewDataSimulation.h"

using yuengine::uieditor::UiEditorComponentTemplateDefaultState;
using yuengine::uieditor::UiEditorComponentTemplateDesc;
using yuengine::uieditor::UiEditorComponentTemplateFactory;
using yuengine::uieditor::UiEditorComponentTemplateKind;
using yuengine::uieditor::UiEditorComponentTemplateRecord;
using yuengine::uieditor::UiEditorComponentTemplateResult;
using yuengine::uieditor::UiEditorComponentTemplateStatus;
using yuengine::uieditor::UiEditorEventBinding;
using yuengine::uieditor::UiEditorGridViewDataCellRecord;
using yuengine::uieditor::UiEditorGridViewDataSimulationDesc;
using yuengine::uieditor::UiEditorGridViewDataSimulationFactory;
using yuengine::uieditor::UiEditorGridViewDataSimulationRecord;
using yuengine::uieditor::UiEditorGridViewDataSimulationResult;
using yuengine::uieditor::UiEditorGridViewDataSimulationStatus;
using yuengine::uieditor::UiEditorResourceReference;

namespace {
constexpr const char *TEST_EXPORTS_VISIBLE_AND_BUFFER_CELLS =
    "UiEditor_GridViewDataSimulation_ExportsVisibleAndBufferCells";
constexpr const char *TEST_CLAMPS_TO_SAMPLE_COUNT =
    "UiEditor_GridViewDataSimulation_ClampsToSampleCount";
constexpr const char *TEST_REJECTS_INVALID_TEMPLATE =
    "UiEditor_GridViewDataSimulation_RejectsInvalidTemplateWithoutMutation";
constexpr const char *TEST_REJECTS_INVALID_COUNTS =
    "UiEditor_GridViewDataSimulation_RejectsInvalidCountsWithoutMutation";
constexpr const char *TEST_REJECTS_SMALL_OUTPUT =
    "UiEditor_GridViewDataSimulation_RejectsSmallOutputWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t ROOT_NODE_ID = 1U;
constexpr std::uint32_t GRID_VIEW_NODE_ID = 20U;
constexpr std::uint32_t SENTINEL_NODE_ID = 777U;
constexpr std::uint32_t SENTINEL_ITEM_INDEX = 888U;
constexpr std::uint32_t VISIBLE_AND_BUFFER_CELL_COUNT = 7U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiEditorComponentTemplateDesc MakeGridViewTemplateDesc() {
    UiEditorComponentTemplateDesc desc{};
    desc.kind = UiEditorComponentTemplateKind::GridView;
    desc.node_id = GRID_VIEW_NODE_ID;
    desc.parent_node_id = ROOT_NODE_ID;
    desc.order = 0U;
    return desc;
}

int CreateGridViewTemplate(UiEditorComponentTemplateRecord *out_record) {
    if (out_record == nullptr) {
        return Fail("grid view template output was null");
    }

    std::array<UiEditorResourceReference, 2U> resources{};
    std::array<UiEditorEventBinding, 1U> events{};
    UiEditorComponentTemplateResult result{};
    const UiEditorComponentTemplateFactory factory{};
    const UiEditorComponentTemplateDesc desc = MakeGridViewTemplateDesc();
    const UiEditorComponentTemplateStatus status = factory.Create(desc, out_record, resources, events, &result);
    if (status != UiEditorComponentTemplateStatus::Success || !result.Succeeded()) {
        return Fail("grid view component template setup failed");
    }

    return 0;
}

UiEditorGridViewDataCellRecord SentinelCellRecord() {
    UiEditorGridViewDataCellRecord record{};
    record.node_id = SENTINEL_NODE_ID;
    record.item_index = SENTINEL_ITEM_INDEX;
    record.visual_order = 9U;
    record.visible_cell = false;
    record.buffer_cell = true;
    return record;
}

UiEditorGridViewDataSimulationRecord SentinelSimulationRecord() {
    UiEditorGridViewDataSimulationRecord record{};
    record.component_kind = UiEditorComponentTemplateKind::Button;
    record.node_id = SENTINEL_NODE_ID;
    record.sample_count = 123U;
    record.first_visible_item_index = 4U;
    record.requested_visible_cell_count = 5U;
    record.requested_buffer_cell_count = 6U;
    record.written_cell_count = 7U;
    record.written_visible_cell_count = 8U;
    record.written_buffer_cell_count = 9U;
    record.last_visible_item_index = 10U;
    return record;
}

bool CellRecordMatchesSentinel(const UiEditorGridViewDataCellRecord &record) {
    if (record.node_id != SENTINEL_NODE_ID) {
        return false;
    }

    if (record.item_index != SENTINEL_ITEM_INDEX) {
        return false;
    }

    if (record.visual_order != 9U) {
        return false;
    }

    if (record.visible_cell) {
        return false;
    }

    return record.buffer_cell;
}

bool SimulationRecordMatchesSentinel(const UiEditorGridViewDataSimulationRecord &record) {
    if (record.component_kind != UiEditorComponentTemplateKind::Button) {
        return false;
    }

    if (record.node_id != SENTINEL_NODE_ID || record.sample_count != 123U) {
        return false;
    }

    if (record.first_visible_item_index != 4U || record.requested_visible_cell_count != 5U) {
        return false;
    }

    if (record.requested_buffer_cell_count != 6U || record.written_cell_count != 7U) {
        return false;
    }

    if (record.written_visible_cell_count != 8U || record.written_buffer_cell_count != 9U) {
        return false;
    }

    return record.last_visible_item_index == 10U;
}

UiEditorGridViewDataSimulationDesc MakeSimulationDesc(
    const UiEditorComponentTemplateRecord &grid_view_template,
    std::uint32_t sample_count,
    std::uint32_t first_visible_item_index,
    std::uint32_t visible_cell_count,
    std::uint32_t buffer_cell_count) {
    UiEditorGridViewDataSimulationDesc desc{};
    desc.component_template = grid_view_template;
    desc.sample_count = sample_count;
    desc.first_visible_item_index = first_visible_item_index;
    desc.visible_cell_count = visible_cell_count;
    desc.buffer_cell_count = buffer_cell_count;
    return desc;
}

int UiEditorGridViewDataSimulationExportsVisibleAndBufferCells() {
    UiEditorComponentTemplateRecord grid_view_template{};
    int ret_code = CreateGridViewTemplate(&grid_view_template);
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorGridViewDataSimulationDesc desc =
        MakeSimulationDesc(grid_view_template, 12U, 3U, 4U, 3U);

    std::array<UiEditorGridViewDataCellRecord, VISIBLE_AND_BUFFER_CELL_COUNT> cells{};
    UiEditorGridViewDataSimulationRecord record{};
    UiEditorGridViewDataSimulationResult result{};
    const UiEditorGridViewDataSimulationFactory factory{};
    const UiEditorGridViewDataSimulationStatus status =
        factory.CreateGridViewDataSimulation(desc, &record, cells, &result);
    if (status != UiEditorGridViewDataSimulationStatus::Success || !result.Succeeded()) {
        return Fail("grid view data simulation creation failed");
    }

    if (record.node_id != GRID_VIEW_NODE_ID || record.sample_count != 12U) {
        return Fail("grid view data simulation identity mismatch");
    }

    if (record.written_cell_count != 7U || record.written_visible_cell_count != 4U) {
        return Fail("grid view data simulation visible count mismatch");
    }

    if (record.written_buffer_cell_count != 3U || record.last_visible_item_index != 6U) {
        return Fail("grid view data simulation buffer count mismatch");
    }

    std::uint32_t index = 0U;
    while (index < VISIBLE_AND_BUFFER_CELL_COUNT) {
        const std::uint32_t expected_item_index = 3U + index;
        if (cells[index].node_id != GRID_VIEW_NODE_ID || cells[index].item_index != expected_item_index) {
            return Fail("grid view data simulation cell identity mismatch");
        }

        if (cells[index].visual_order != index) {
            return Fail("grid view data simulation visual order mismatch");
        }

        const bool expected_visible = index < 4U;
        if (cells[index].visible_cell != expected_visible) {
            return Fail("grid view data simulation visible flag mismatch");
        }

        if (cells[index].buffer_cell == expected_visible) {
            return Fail("grid view data simulation buffer flag mismatch");
        }

        ++index;
    }

    return 0;
}

int UiEditorGridViewDataSimulationClampsToSampleCount() {
    UiEditorComponentTemplateRecord grid_view_template{};
    int ret_code = CreateGridViewTemplate(&grid_view_template);
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorGridViewDataSimulationDesc desc =
        MakeSimulationDesc(grid_view_template, 6U, 4U, 4U, 4U);

    std::array<UiEditorGridViewDataCellRecord, 8U> cells{};
    UiEditorGridViewDataSimulationRecord record{};
    UiEditorGridViewDataSimulationResult result{};
    const UiEditorGridViewDataSimulationFactory factory{};
    const UiEditorGridViewDataSimulationStatus status =
        factory.CreateGridViewDataSimulation(desc, &record, cells, &result);
    if (status != UiEditorGridViewDataSimulationStatus::Success || !result.Succeeded()) {
        return Fail("grid view data simulation clamp creation failed");
    }

    if (result.required_cell_count != 2U || result.written_cell_count != 2U) {
        return Fail("grid view data simulation clamp count mismatch");
    }

    if (record.written_visible_cell_count != 2U || record.written_buffer_cell_count != 0U) {
        return Fail("grid view data simulation clamp visible count mismatch");
    }

    if (record.last_visible_item_index != 5U) {
        return Fail("grid view data simulation clamp last visible mismatch");
    }

    if (cells[0U].item_index != 4U || cells[1U].item_index != 5U) {
        return Fail("grid view data simulation clamp item mismatch");
    }

    if (!cells[0U].visible_cell || !cells[1U].visible_cell) {
        return Fail("grid view data simulation clamp visible flag mismatch");
    }

    if (cells[0U].buffer_cell || cells[1U].buffer_cell) {
        return Fail("grid view data simulation clamp buffer flag mismatch");
    }

    return 0;
}

int UiEditorGridViewDataSimulationRejectsInvalidTemplateWithoutMutation() {
    UiEditorGridViewDataSimulationDesc desc{};
    desc.component_template.kind = UiEditorComponentTemplateKind::Button;
    desc.component_template.default_state = UiEditorComponentTemplateDefaultState::ButtonNormal;
    desc.component_template.layout_node.node_id = GRID_VIEW_NODE_ID;
    desc.sample_count = 10U;
    desc.first_visible_item_index = 0U;
    desc.visible_cell_count = 4U;
    desc.buffer_cell_count = 2U;

    std::array<UiEditorGridViewDataCellRecord, 1U> cells = {SentinelCellRecord()};
    UiEditorGridViewDataSimulationRecord record = SentinelSimulationRecord();
    UiEditorGridViewDataSimulationResult result{};
    const UiEditorGridViewDataSimulationFactory factory{};
    const UiEditorGridViewDataSimulationStatus status =
        factory.CreateGridViewDataSimulation(desc, &record, cells, &result);
    if (status != UiEditorGridViewDataSimulationStatus::InvalidTemplate) {
        return Fail("invalid grid view template was not rejected");
    }

    if (!CellRecordMatchesSentinel(cells[0U])) {
        return Fail("invalid grid view template mutated cells");
    }

    if (!SimulationRecordMatchesSentinel(record)) {
        return Fail("invalid grid view template mutated record");
    }

    return 0;
}

int UiEditorGridViewDataSimulationRejectsInvalidCountsWithoutMutation() {
    UiEditorComponentTemplateRecord grid_view_template{};
    int ret_code = CreateGridViewTemplate(&grid_view_template);
    if (ret_code != 0) {
        return ret_code;
    }

    UiEditorGridViewDataSimulationDesc desc =
        MakeSimulationDesc(grid_view_template, 0U, 0U, 4U, 2U);

    std::array<UiEditorGridViewDataCellRecord, 1U> cells = {SentinelCellRecord()};
    UiEditorGridViewDataSimulationRecord record = SentinelSimulationRecord();
    UiEditorGridViewDataSimulationResult result{};
    const UiEditorGridViewDataSimulationFactory factory{};
    UiEditorGridViewDataSimulationStatus status =
        factory.CreateGridViewDataSimulation(desc, &record, cells, &result);
    if (status != UiEditorGridViewDataSimulationStatus::InvalidSampleCount) {
        return Fail("invalid sample count was not rejected");
    }

    if (!CellRecordMatchesSentinel(cells[0U]) || !SimulationRecordMatchesSentinel(record)) {
        return Fail("invalid sample count mutated output");
    }

    desc = MakeSimulationDesc(grid_view_template, 8U, 0U, 0U, 2U);
    status = factory.CreateGridViewDataSimulation(desc, &record, cells, &result);
    if (status != UiEditorGridViewDataSimulationStatus::InvalidVisibleCellCount) {
        return Fail("invalid visible count was not rejected");
    }

    if (!CellRecordMatchesSentinel(cells[0U]) || !SimulationRecordMatchesSentinel(record)) {
        return Fail("invalid visible count mutated output");
    }

    desc = MakeSimulationDesc(grid_view_template, 8U, 0U, 4U, 65U);
    status = factory.CreateGridViewDataSimulation(desc, &record, cells, &result);
    if (status != UiEditorGridViewDataSimulationStatus::InvalidBufferCellCount) {
        return Fail("invalid buffer count was not rejected");
    }

    if (!CellRecordMatchesSentinel(cells[0U]) || !SimulationRecordMatchesSentinel(record)) {
        return Fail("invalid buffer count mutated output");
    }

    return 0;
}

int UiEditorGridViewDataSimulationRejectsSmallOutputWithoutMutation() {
    UiEditorComponentTemplateRecord grid_view_template{};
    int ret_code = CreateGridViewTemplate(&grid_view_template);
    if (ret_code != 0) {
        return ret_code;
    }

    const UiEditorGridViewDataSimulationDesc desc =
        MakeSimulationDesc(grid_view_template, 10U, 0U, 4U, 4U);

    std::array<UiEditorGridViewDataCellRecord, 1U> cells = {SentinelCellRecord()};
    UiEditorGridViewDataSimulationRecord record = SentinelSimulationRecord();
    UiEditorGridViewDataSimulationResult result{};
    const UiEditorGridViewDataSimulationFactory factory{};
    const UiEditorGridViewDataSimulationStatus status =
        factory.CreateGridViewDataSimulation(desc, &record, cells, &result);
    if (status != UiEditorGridViewDataSimulationStatus::OutputCapacityExceeded) {
        return Fail("small output was not rejected");
    }

    if (result.required_cell_count != 8U) {
        return Fail("small output required count mismatch");
    }

    if (!CellRecordMatchesSentinel(cells[0U])) {
        return Fail("small output mutated cells");
    }

    if (!SimulationRecordMatchesSentinel(record)) {
        return Fail("small output mutated record");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_EXPORTS_VISIBLE_AND_BUFFER_CELLS) {
        return UiEditorGridViewDataSimulationExportsVisibleAndBufferCells();
    }

    if (name == TEST_CLAMPS_TO_SAMPLE_COUNT) {
        return UiEditorGridViewDataSimulationClampsToSampleCount();
    }

    if (name == TEST_REJECTS_INVALID_TEMPLATE) {
        return UiEditorGridViewDataSimulationRejectsInvalidTemplateWithoutMutation();
    }

    if (name == TEST_REJECTS_INVALID_COUNTS) {
        return UiEditorGridViewDataSimulationRejectsInvalidCountsWithoutMutation();
    }

    if (name == TEST_REJECTS_SMALL_OUTPUT) {
        return UiEditorGridViewDataSimulationRejectsSmallOutputWithoutMutation();
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
