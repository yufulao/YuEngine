// 模块: Tests UiCore
// 文件: Tests/UiCore/UiGridViewVirtualizerTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiGridViewSemantics.h"
#include "YuEngine/UiCore/UiGridViewVirtualizer.h"

using yuengine::uicore::INVALID_UI_GRID_INDEX;
using yuengine::uicore::UiGridViewKind;
using UiGridViewDesc = yuengine::uicore::UiGridViewDesc;
using UiGridViewStatus = yuengine::uicore::UiGridViewStatus;
using UiGridViewVirtualCellRecord = yuengine::uicore::UiGridViewVirtualCellRecord;
using UiGridViewVirtualizationDesc = yuengine::uicore::UiGridViewVirtualizationDesc;
using UiGridViewVirtualizationResult = yuengine::uicore::UiGridViewVirtualizationResult;
using UiGridViewVirtualizer = yuengine::uicore::UiGridViewVirtualizer;

namespace {
constexpr const char *TEST_LARGE_LIST =
    "UiCore_GridViewVirtualizer_LargeListUsesVisibleBufferPool";
constexpr const char *TEST_TAIL_SLOT =
    "UiCore_GridViewVirtualizer_TailGroupKeepsEmptyPoolSlotHidden";
constexpr const char *TEST_REJECT_FULL_POOL =
    "UiCore_GridViewVirtualizer_RejectsFullPoolAsNonVirtualized";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiCore_GridViewVirtualizer_RejectsSmallOutputWithoutMutation";
constexpr const char *TEST_SCROLL_DIRTY =
    "UiCore_GridViewVirtualizer_ResolvesAutoScrollAndAffectedDirtyCells";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_INDEX = 4444U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiGridViewDesc MakeGridDesc(
    std::uint32_t item_count,
    std::uint32_t axis_cell_count,
    std::uint32_t visible_group_count,
    std::uint32_t buffer_group_count,
    std::uint32_t pool_group_count) {
    UiGridViewDesc desc;
    desc.item_count = item_count;
    desc.axis_cell_count = axis_cell_count;
    desc.visible_group_count = visible_group_count;
    desc.buffer_group_count = buffer_group_count;
    desc.pool_group_count = pool_group_count;
    return desc;
}

UiGridViewVirtualCellRecord SentinelCell() {
    UiGridViewVirtualCellRecord record;
    record.pool_cell_index = SENTINEL_INDEX;
    record.item_index = SENTINEL_INDEX;
    record.group_index = SENTINEL_INDEX;
    record.cell_index = SENTINEL_INDEX;
    return record;
}

bool CellMatchesSentinel(const UiGridViewVirtualCellRecord &record) {
    if (record.pool_cell_index != SENTINEL_INDEX) {
        return false;
    }

    if (record.item_index != SENTINEL_INDEX) {
        return false;
    }

    if (record.group_index != SENTINEL_INDEX) {
        return false;
    }

    return record.cell_index == SENTINEL_INDEX;
}

int UiCoreGridViewVirtualizerLargeListUsesVisibleBufferPool() {
    UiGridViewVirtualizationDesc desc{};
    desc.grid_desc = MakeGridDesc(1000U, 5U, 2U, 1U, 4U);
    desc.first_visible_group = 10U;
    desc.selected_index = 52U;
    desc.previous_selected_index = 51U;
    desc.updated_item_index = 53U;
    desc.existing_pool_cell_count = 8U;

    std::array<UiGridViewVirtualCellRecord, 20U> cells{};
    UiGridViewVirtualizationResult result{};
    UiGridViewVirtualizer virtualizer{};
    const UiGridViewStatus status = virtualizer.Build(desc, cells, &result);
    if (status != UiGridViewStatus::Success || !result.Succeeded()) {
        return Fail("virtualizer rejected large list fixture");
    }

    if (result.group_count != 200U || result.first_materialized_group != 9U) {
        return Fail("virtualizer group range mismatch");
    }

    if (result.materialized_group_count != 4U || result.required_pool_cell_count != 20U) {
        return Fail("virtualizer pool count mismatch");
    }

    if (result.pool_cell_count >= desc.grid_desc.item_count) {
        return Fail("virtualizer created full item list");
    }

    if (result.visible_item_count != 10U || result.materialized_item_count != 20U) {
        return Fail("virtualizer visible materialized item count mismatch");
    }

    if (result.reused_cell_count != 8U || result.created_cell_count != 12U) {
        return Fail("virtualizer reused created count mismatch");
    }

    if (result.dirty_cell_count != 3U) {
        return Fail("virtualizer did not dirty only affected cells");
    }

    const UiGridViewVirtualCellRecord &first_cell = cells[0U];
    if (first_cell.item_index != 45U || first_cell.group_index != 9U || first_cell.visible) {
        return Fail("virtualizer first buffer cell mismatch");
    }

    if (!first_cell.reused || first_cell.init_callback_required || !first_cell.update_callback_required) {
        return Fail("virtualizer reused callback state mismatch");
    }

    const UiGridViewVirtualCellRecord &selected_cell = cells[7U];
    if (selected_cell.item_index != 52U || !selected_cell.visible || !selected_cell.selected) {
        return Fail("virtualizer selected cell mapping mismatch");
    }

    if (!selected_cell.dirty || selected_cell.init_callback_required) {
        return Fail("virtualizer selected dirty or init state mismatch");
    }

    if (cells[8U].reused || !cells[8U].init_callback_required || !cells[8U].dirty) {
        return Fail("virtualizer created cell callback state mismatch");
    }

    return 0;
}

int UiCoreGridViewVirtualizerTailGroupKeepsEmptyPoolSlotHidden() {
    UiGridViewVirtualizationDesc desc{};
    desc.grid_desc = MakeGridDesc(7U, 4U, 2U, 0U, 2U);
    desc.first_visible_group = 0U;

    std::array<UiGridViewVirtualCellRecord, 8U> cells{};
    UiGridViewVirtualizationResult result{};
    UiGridViewVirtualizer virtualizer{};
    const UiGridViewStatus status = virtualizer.Build(desc, cells, &result);
    if (status != UiGridViewStatus::Success) {
        return Fail("virtualizer rejected tail group fixture");
    }

    if (result.pool_cell_count != 8U || result.materialized_item_count != 7U) {
        return Fail("virtualizer tail pool count mismatch");
    }

    if (result.dirty_cell_count != 0U) {
        return Fail("virtualizer tail fixture dirtied cells without selection");
    }

    const UiGridViewVirtualCellRecord &tail_slot = cells[7U];
    if (tail_slot.has_item || tail_slot.visible || tail_slot.update_callback_required) {
        return Fail("virtualizer tail empty slot state mismatch");
    }

    if (tail_slot.item_index != INVALID_UI_GRID_INDEX || tail_slot.group_index != 1U || tail_slot.cell_index != 3U) {
        return Fail("virtualizer tail empty slot index mismatch");
    }

    return 0;
}

int UiCoreGridViewVirtualizerRejectsFullPoolAsNonVirtualized() {
    UiGridViewVirtualizationDesc desc{};
    desc.grid_desc = MakeGridDesc(100U, 5U, 2U, 1U, 20U);
    desc.first_visible_group = 3U;

    std::array<UiGridViewVirtualCellRecord, 32U> cells{SentinelCell()};
    UiGridViewVirtualizationResult result{};
    UiGridViewVirtualizer virtualizer{};
    const UiGridViewStatus status = virtualizer.Build(desc, cells, &result);
    if (status != UiGridViewStatus::FullPoolRejected) {
        return Fail("virtualizer accepted full pool");
    }

    if (!CellMatchesSentinel(cells[0U]) || result.pool_cell_count != 0U) {
        return Fail("virtualizer mutated output after full pool rejection");
    }

    return 0;
}

int UiCoreGridViewVirtualizerRejectsSmallOutputWithoutMutation() {
    UiGridViewVirtualizationDesc desc{};
    desc.grid_desc = MakeGridDesc(20U, 5U, 2U, 1U, 3U);
    desc.first_visible_group = 0U;

    std::array<UiGridViewVirtualCellRecord, 1U> cells{SentinelCell()};
    UiGridViewVirtualizationResult result{};
    UiGridViewVirtualizer virtualizer{};
    const UiGridViewStatus status = virtualizer.Build(desc, cells, &result);
    if (status != UiGridViewStatus::OutputCapacityExceeded) {
        return Fail("virtualizer accepted small output");
    }

    if (result.required_pool_cell_count != 15U || !CellMatchesSentinel(cells[0U])) {
        return Fail("virtualizer small output mutation or count mismatch");
    }

    return 0;
}

int UiCoreGridViewVirtualizerResolvesAutoScrollAndAffectedDirtyCells() {
    UiGridViewVirtualizationDesc desc{};
    desc.grid_desc = MakeGridDesc(40U, 4U, 2U, 1U, 4U);
    desc.first_visible_group = 3U;
    desc.selected_index = 1U;
    desc.previous_selected_index = 9U;
    desc.auto_scroll_to_selection = true;
    desc.existing_pool_cell_count = 20U;

    std::array<UiGridViewVirtualCellRecord, 16U> cells{};
    UiGridViewVirtualizationResult result{};
    UiGridViewVirtualizer virtualizer{};
    const UiGridViewStatus status = virtualizer.Build(desc, cells, &result);
    if (status != UiGridViewStatus::Success) {
        return Fail("virtualizer rejected auto scroll fixture");
    }

    if (result.scroll_target_index != 1U || result.scroll_group_index != 0U || result.scroll_cell_index != 1U) {
        return Fail("virtualizer auto scroll target mismatch");
    }

    if (!result.scroll_required) {
        return Fail("virtualizer did not request offscreen auto scroll");
    }

    if (result.dirty_cell_count != 1U) {
        return Fail("virtualizer dirty affected cell count mismatch");
    }

    if (cells[1U].item_index != 9U || !cells[1U].dirty || !cells[1U].reused) {
        return Fail("virtualizer previous selection dirty cell mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_LARGE_LIST) {
        return UiCoreGridViewVirtualizerLargeListUsesVisibleBufferPool();
    }

    if (name == TEST_TAIL_SLOT) {
        return UiCoreGridViewVirtualizerTailGroupKeepsEmptyPoolSlotHidden();
    }

    if (name == TEST_REJECT_FULL_POOL) {
        return UiCoreGridViewVirtualizerRejectsFullPoolAsNonVirtualized();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiCoreGridViewVirtualizerRejectsSmallOutputWithoutMutation();
    }

    if (name == TEST_SCROLL_DIRTY) {
        return UiCoreGridViewVirtualizerResolvesAutoScrollAndAffectedDirtyCells();
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
