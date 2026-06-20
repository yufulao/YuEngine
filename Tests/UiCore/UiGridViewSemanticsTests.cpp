// 模块: Tests UiCore
// 文件: Tests/UiCore/UiGridViewSemanticsTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiGridViewSemantics.h"

using yuengine::uicore::INVALID_UI_GRID_INDEX;
using UiGridViewCellRecord = yuengine::uicore::UiGridViewCellRecord;
using UiGridViewDesc = yuengine::uicore::UiGridViewDesc;
using UiGridViewIndexResult = yuengine::uicore::UiGridViewIndexResult;
using yuengine::uicore::UiGridViewKind;
using UiGridViewRangeResult = yuengine::uicore::UiGridViewRangeResult;
using UiGridViewSemantics = yuengine::uicore::UiGridViewSemantics;
using yuengine::uicore::UiGridViewStatus;

namespace {
constexpr const char *TEST_GROUPED_RANGE =
    "UiCore_GridViewSemantics_GroupsCellsAndVisibleBufferRange";
constexpr const char *TEST_REJECT_FULL_POOL =
    "UiCore_GridViewSemantics_RejectsFullPoolAsNonVirtualized";
constexpr const char *TEST_SELECTION =
    "UiCore_GridViewSemantics_BtnSelectionTouchesVisibleCellsOnly";
constexpr const char *TEST_SCROLL_TO_INDEX =
    "UiCore_GridViewSemantics_ResolvesScrollToIndexTarget";
constexpr const char *TEST_INVALID =
    "UiCore_GridViewSemantics_RejectsInvalidAndSmallOutput";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_INDEX = 7777U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiGridViewDesc MakeDesc(
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

UiGridViewCellRecord SentinelCell() {
    UiGridViewCellRecord record;
    record.item_index = SENTINEL_INDEX;
    record.group_index = SENTINEL_INDEX;
    record.cell_index = SENTINEL_INDEX;
    return record;
}

bool CellMatchesSentinel(const UiGridViewCellRecord &record) {
    if (record.item_index != SENTINEL_INDEX) {
        return false;
    }

    if (record.group_index != SENTINEL_INDEX) {
        return false;
    }

    return record.cell_index == SENTINEL_INDEX;
}

int UiCoreGridViewSemanticsGroupsCellsAndVisibleBufferRange() {
    const UiGridViewDesc desc = MakeDesc(10U, 4U, 2U, 1U, 3U);
    std::array<UiGridViewCellRecord, 10U> cells{};
    UiGridViewRangeResult result{};
    UiGridViewSemantics semantics{};
    const UiGridViewStatus status = semantics.BuildVisibleRange(desc, 1U, 5U, cells, &result);
    if (status != UiGridViewStatus::Success || !result.Succeeded()) {
        return Fail("grid semantics did not build visible range");
    }

    if (result.group_count != 3U || result.first_materialized_group != 0U) {
        return Fail("grid semantics group range mismatch");
    }

    if (result.materialized_group_count != 3U || result.materialized_cell_count != 10U) {
        return Fail("grid semantics materialized count mismatch");
    }

    if (result.visible_group_count != 2U || result.visible_cell_count != 6U) {
        return Fail("grid semantics visible count mismatch");
    }

    if (cells[0U].item_index != 0U || cells[0U].group_index != 0U || cells[0U].visible) {
        return Fail("grid semantics first buffer cell mismatch");
    }

    if (cells[5U].item_index != 5U || cells[5U].group_index != 1U || cells[5U].cell_index != 1U) {
        return Fail("grid semantics selected cell index mapping mismatch");
    }

    if (!cells[5U].visible || !cells[5U].selected) {
        return Fail("grid semantics selected visible state mismatch");
    }

    if (cells[9U].item_index != 9U || cells[9U].group_index != 2U || cells[9U].cell_index != 1U) {
        return Fail("grid semantics partial tail group mismatch");
    }

    return 0;
}

int UiCoreGridViewSemanticsRejectsFullPoolAsNonVirtualized() {
    const UiGridViewDesc desc = MakeDesc(100U, 5U, 2U, 1U, 20U);
    std::array<UiGridViewCellRecord, 32U> cells{};
    UiGridViewRangeResult result{};
    UiGridViewSemantics semantics{};
    const UiGridViewStatus status = semantics.BuildVisibleRange(desc, 3U, INVALID_UI_GRID_INDEX, cells, &result);
    if (status != UiGridViewStatus::FullPoolRejected) {
        return Fail("grid semantics accepted full item pool");
    }

    if (result.materialized_cell_count != 0U) {
        return Fail("grid semantics wrote counters after full pool rejection");
    }

    return 0;
}

int UiCoreGridViewSemanticsBtnSelectionTouchesVisibleCellsOnly() {
    UiGridViewDesc desc = MakeDesc(10U, 4U, 2U, 1U, 3U);
    desc.kind = UiGridViewKind::BtnGridView;
    std::array<UiGridViewCellRecord, 10U> cells{};
    UiGridViewRangeResult range_result{};
    UiGridViewSemantics semantics{};
    UiGridViewStatus status = semantics.BuildVisibleRange(desc, 1U, INVALID_UI_GRID_INDEX, cells, &range_result);
    if (status != UiGridViewStatus::Success) {
        return Fail("btn grid semantics did not build visible range");
    }

    UiGridViewIndexResult select_result{};
    status = semantics.ResolveSelection(desc, range_result, 8U, &select_result);
    if (status != UiGridViewStatus::Success || !select_result.Succeeded()) {
        return Fail("btn grid semantics did not resolve visible selection");
    }

    if (!select_result.visible || select_result.requires_scroll) {
        return Fail("btn grid semantics visible selection scroll mismatch");
    }

    if (select_result.affected_visible_cell_count != range_result.visible_cell_count) {
        return Fail("btn grid semantics did not refresh only visible cells");
    }

    status = semantics.ResolveSelection(desc, range_result, 0U, &select_result);
    if (status != UiGridViewStatus::Success) {
        return Fail("btn grid semantics did not resolve offscreen selection");
    }

    if (select_result.visible || !select_result.requires_scroll) {
        return Fail("btn grid semantics offscreen selection scroll mismatch");
    }

    if (select_result.affected_visible_cell_count != range_result.visible_cell_count) {
        return Fail("btn grid semantics offscreen selection refresh count mismatch");
    }

    return 0;
}

int UiCoreGridViewSemanticsResolvesScrollToIndexTarget() {
    const UiGridViewDesc desc = MakeDesc(18U, 4U, 2U, 1U, 4U);
    UiGridViewIndexResult result{};
    UiGridViewSemantics semantics{};
    UiGridViewStatus status = semantics.ResolveScrollToIndex(desc, 2U, 10U, &result);
    if (status != UiGridViewStatus::Success || !result.Succeeded()) {
        return Fail("grid semantics did not resolve visible scroll target");
    }

    if (result.group_index != 2U || result.cell_index != 2U || !result.visible) {
        return Fail("grid semantics visible scroll target mismatch");
    }

    status = semantics.ResolveScrollToIndex(desc, 2U, 1U, &result);
    if (status != UiGridViewStatus::Success) {
        return Fail("grid semantics did not resolve offscreen scroll target");
    }

    if (result.group_index != 0U || result.cell_index != 1U || !result.requires_scroll) {
        return Fail("grid semantics offscreen scroll target mismatch");
    }

    return 0;
}

int UiCoreGridViewSemanticsRejectsInvalidAndSmallOutput() {
    UiGridViewDesc desc = MakeDesc(8U, 0U, 1U, 0U, 1U);
    std::array<UiGridViewCellRecord, 1U> cells{SentinelCell()};
    UiGridViewRangeResult result{};
    UiGridViewSemantics semantics{};
    UiGridViewStatus status = semantics.BuildVisibleRange(desc, 0U, INVALID_UI_GRID_INDEX, cells, &result);
    if (status != UiGridViewStatus::InvalidAxisCellCount) {
        return Fail("grid semantics accepted invalid axis count");
    }

    desc = MakeDesc(8U, 4U, 1U, 0U, 1U);
    status = semantics.BuildVisibleRange(desc, 0U, 9U, cells, &result);
    if (status != UiGridViewStatus::InvalidItemIndex) {
        return Fail("grid semantics accepted invalid selected index");
    }

    status = semantics.BuildVisibleRange(desc, 0U, INVALID_UI_GRID_INDEX, cells, &result);
    if (status != UiGridViewStatus::OutputCapacityExceeded) {
        return Fail("grid semantics accepted undersized output buffer");
    }

    if (!CellMatchesSentinel(cells[0U])) {
        return Fail("grid semantics mutated output after small buffer rejection");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_GROUPED_RANGE) {
        return UiCoreGridViewSemanticsGroupsCellsAndVisibleBufferRange();
    }

    if (name == TEST_REJECT_FULL_POOL) {
        return UiCoreGridViewSemanticsRejectsFullPoolAsNonVirtualized();
    }

    if (name == TEST_SELECTION) {
        return UiCoreGridViewSemanticsBtnSelectionTouchesVisibleCellsOnly();
    }

    if (name == TEST_SCROLL_TO_INDEX) {
        return UiCoreGridViewSemanticsResolvesScrollToIndexTarget();
    }

    if (name == TEST_INVALID) {
        return UiCoreGridViewSemanticsRejectsInvalidAndSmallOutput();
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
