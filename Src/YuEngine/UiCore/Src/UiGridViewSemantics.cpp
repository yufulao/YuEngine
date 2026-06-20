// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiGridViewSemantics.cpp

#include "YuEngine/UiCore/UiGridViewSemantics.h"

namespace yuengine::uicore {
namespace {
std::uint32_t MinUint(std::uint32_t left, std::uint32_t right) {
    if (left < right) {
        return left;
    }

    return right;
}

std::uint32_t ClampRangeEnd(std::uint32_t start, std::uint32_t count, std::uint32_t limit) {
    if (start >= limit) {
        return limit;
    }

    const std::uint32_t remaining = limit - start;
    const std::uint32_t clamped_count = MinUint(count, remaining);
    return start + clamped_count;
}

std::uint32_t CountRange(std::uint32_t start, std::uint32_t end) {
    if (end <= start) {
        return 0U;
    }

    return end - start;
}

std::uint32_t SubtractClamped(std::uint32_t value, std::uint32_t amount) {
    if (amount > value) {
        return 0U;
    }

    return value - amount;
}

void SetRangeFailure(UiGridViewRangeResult *out_result, UiGridViewStatus status) {
    out_result->status = status;
}
}

UiGridViewStatus UiGridViewSemantics::BuildVisibleRange(
    const UiGridViewDesc &desc,
    std::uint32_t first_visible_group,
    std::uint32_t selected_index,
    std::span<UiGridViewCellRecord> out_cells,
    UiGridViewRangeResult *out_result) const {
    if (out_result == nullptr) {
        return UiGridViewStatus::InvalidOutputBuffer;
    }

    *out_result = UiGridViewRangeResult{};
    out_result->item_count = desc.item_count;

    UiGridViewStatus status = ValidateDesc(desc);
    if (status != UiGridViewStatus::Success) {
        SetRangeFailure(out_result, status);
        return status;
    }

    if (selected_index != INVALID_UI_GRID_INDEX && selected_index >= desc.item_count) {
        SetRangeFailure(out_result, UiGridViewStatus::InvalidItemIndex);
        return UiGridViewStatus::InvalidItemIndex;
    }

    const std::uint32_t group_count = CountGroups(desc);
    out_result->group_count = group_count;
    status = ValidateVisibleGroup(group_count, first_visible_group);
    if (status != UiGridViewStatus::Success) {
        SetRangeFailure(out_result, status);
        return status;
    }

    if (group_count == 0U) {
        out_result->status = UiGridViewStatus::Success;
        return UiGridViewStatus::Success;
    }

    const std::uint32_t visible_end = ClampRangeEnd(first_visible_group, desc.visible_group_count, group_count);
    const std::uint32_t first_materialized_group = SubtractClamped(first_visible_group, desc.buffer_group_count);
    const std::uint32_t buffered_visible_count = CountRange(first_visible_group, visible_end) + desc.buffer_group_count;
    const std::uint32_t materialized_end = ClampRangeEnd(first_visible_group, buffered_visible_count, group_count);
    const std::uint32_t materialized_group_count = CountRange(first_materialized_group, materialized_end);
    if (desc.pool_group_count < materialized_group_count) {
        SetRangeFailure(out_result, UiGridViewStatus::InvalidPoolGroupCount);
        return UiGridViewStatus::InvalidPoolGroupCount;
    }

    if (IsFullPoolRejected(desc, group_count, materialized_group_count)) {
        SetRangeFailure(out_result, UiGridViewStatus::FullPoolRejected);
        return UiGridViewStatus::FullPoolRejected;
    }

    std::uint32_t materialized_cell_count = 0U;
    std::uint32_t visible_cell_count = 0U;
    for (std::uint32_t group_index = first_materialized_group; group_index < materialized_end; ++group_index) {
        const std::uint32_t group_cell_count = CountCellsInGroup(desc, group_index);
        materialized_cell_count += group_cell_count;
        if (group_index >= first_visible_group && group_index < visible_end) {
            visible_cell_count += group_cell_count;
        }
    }

    if (out_cells.size() < materialized_cell_count) {
        out_result->materialized_cell_count = materialized_cell_count;
        SetRangeFailure(out_result, UiGridViewStatus::OutputCapacityExceeded);
        return UiGridViewStatus::OutputCapacityExceeded;
    }

    if (materialized_cell_count > 0U && out_cells.data() == nullptr) {
        SetRangeFailure(out_result, UiGridViewStatus::InvalidOutputBuffer);
        return UiGridViewStatus::InvalidOutputBuffer;
    }

    std::uint32_t output_index = 0U;
    for (std::uint32_t group_index = first_materialized_group; group_index < materialized_end; ++group_index) {
        const std::uint32_t group_cell_count = CountCellsInGroup(desc, group_index);
        for (std::uint32_t cell_index = 0U; cell_index < group_cell_count; ++cell_index) {
            const std::uint32_t item_index = group_index * desc.axis_cell_count + cell_index;
            UiGridViewCellRecord record;
            record.item_index = item_index;
            record.group_index = group_index;
            record.cell_index = cell_index;
            record.visible = group_index >= first_visible_group && group_index < visible_end;
            record.selected = item_index == selected_index;
            out_cells[output_index] = record;
            ++output_index;
        }
    }

    out_result->status = UiGridViewStatus::Success;
    out_result->first_visible_group = first_visible_group;
    out_result->visible_group_count = CountRange(first_visible_group, visible_end);
    out_result->first_materialized_group = first_materialized_group;
    out_result->materialized_group_count = materialized_group_count;
    out_result->visible_cell_count = visible_cell_count;
    out_result->materialized_cell_count = materialized_cell_count;
    out_result->required_pool_group_count = materialized_group_count;
    return UiGridViewStatus::Success;
}

UiGridViewStatus UiGridViewSemantics::ResolveScrollToIndex(
    const UiGridViewDesc &desc,
    std::uint32_t first_visible_group,
    std::uint32_t item_index,
    UiGridViewIndexResult *out_result) const {
    return FillIndexResult(desc, first_visible_group, item_index, out_result);
}

UiGridViewStatus UiGridViewSemantics::ResolveSelection(
    const UiGridViewDesc &desc,
    const UiGridViewRangeResult &range_result,
    std::uint32_t selected_index,
    UiGridViewIndexResult *out_result) const {
    if (out_result == nullptr) {
        return UiGridViewStatus::InvalidOutputBuffer;
    }

    *out_result = UiGridViewIndexResult{};
    UiGridViewStatus status = ValidateDesc(desc);
    if (status != UiGridViewStatus::Success) {
        out_result->status = status;
        return status;
    }

    if (!range_result.Succeeded()) {
        out_result->status = range_result.status;
        return range_result.status;
    }

    if (selected_index >= desc.item_count) {
        out_result->status = UiGridViewStatus::InvalidItemIndex;
        return UiGridViewStatus::InvalidItemIndex;
    }

    out_result->status = UiGridViewStatus::Success;
    out_result->item_index = selected_index;
    out_result->group_index = selected_index / desc.axis_cell_count;
    out_result->cell_index = selected_index % desc.axis_cell_count;
    out_result->visible = IsGroupVisible(range_result, out_result->group_index);
    out_result->requires_scroll = !out_result->visible;
    if (desc.kind == UiGridViewKind::BtnGridView) {
        out_result->affected_visible_cell_count = range_result.visible_cell_count;
        return UiGridViewStatus::Success;
    }

    if (out_result->visible) {
        out_result->affected_visible_cell_count = 1U;
    }

    return UiGridViewStatus::Success;
}

UiGridViewStatus UiGridViewSemantics::ValidateDesc(const UiGridViewDesc &desc) const {
    if (desc.axis_cell_count == 0U) {
        return UiGridViewStatus::InvalidAxisCellCount;
    }

    if (desc.visible_group_count == 0U) {
        return UiGridViewStatus::InvalidVisibleGroupCount;
    }

    if (desc.pool_group_count == 0U) {
        return UiGridViewStatus::InvalidPoolGroupCount;
    }

    return UiGridViewStatus::Success;
}

std::uint32_t UiGridViewSemantics::CountGroups(const UiGridViewDesc &desc) const {
    if (desc.item_count == 0U) {
        return 0U;
    }

    const std::uint32_t adjusted_count = desc.item_count + desc.axis_cell_count - 1U;
    return adjusted_count / desc.axis_cell_count;
}

std::uint32_t UiGridViewSemantics::CountCellsInGroup(
    const UiGridViewDesc &desc,
    std::uint32_t group_index) const {
    const std::uint32_t first_item_index = group_index * desc.axis_cell_count;
    if (first_item_index >= desc.item_count) {
        return 0U;
    }

    const std::uint32_t remaining = desc.item_count - first_item_index;
    return MinUint(desc.axis_cell_count, remaining);
}

bool UiGridViewSemantics::IsGroupVisible(
    const UiGridViewRangeResult &range_result,
    std::uint32_t group_index) const {
    if (group_index < range_result.first_visible_group) {
        return false;
    }

    const std::uint32_t visible_end = range_result.first_visible_group + range_result.visible_group_count;
    return group_index < visible_end;
}

bool UiGridViewSemantics::IsFullPoolRejected(
    const UiGridViewDesc &desc,
    std::uint32_t group_count,
    std::uint32_t materialized_group_count) const {
    if (group_count <= materialized_group_count) {
        return false;
    }

    return desc.pool_group_count >= group_count;
}

UiGridViewStatus UiGridViewSemantics::ValidateVisibleGroup(
    std::uint32_t group_count,
    std::uint32_t first_visible_group) const {
    if (group_count == 0U) {
        if (first_visible_group == 0U) {
            return UiGridViewStatus::Success;
        }

        return UiGridViewStatus::InvalidVisibleGroupIndex;
    }

    if (first_visible_group >= group_count) {
        return UiGridViewStatus::InvalidVisibleGroupIndex;
    }

    return UiGridViewStatus::Success;
}

UiGridViewStatus UiGridViewSemantics::FillIndexResult(
    const UiGridViewDesc &desc,
    std::uint32_t first_visible_group,
    std::uint32_t item_index,
    UiGridViewIndexResult *out_result) const {
    if (out_result == nullptr) {
        return UiGridViewStatus::InvalidOutputBuffer;
    }

    *out_result = UiGridViewIndexResult{};
    UiGridViewStatus status = ValidateDesc(desc);
    if (status != UiGridViewStatus::Success) {
        out_result->status = status;
        return status;
    }

    const std::uint32_t group_count = CountGroups(desc);
    status = ValidateVisibleGroup(group_count, first_visible_group);
    if (status != UiGridViewStatus::Success) {
        out_result->status = status;
        return status;
    }

    if (item_index >= desc.item_count) {
        out_result->status = UiGridViewStatus::InvalidItemIndex;
        return UiGridViewStatus::InvalidItemIndex;
    }

    out_result->status = UiGridViewStatus::Success;
    out_result->item_index = item_index;
    out_result->group_index = item_index / desc.axis_cell_count;
    out_result->cell_index = item_index % desc.axis_cell_count;
    const std::uint32_t visible_end = ClampRangeEnd(first_visible_group, desc.visible_group_count, group_count);
    out_result->visible = out_result->group_index >= first_visible_group && out_result->group_index < visible_end;
    out_result->requires_scroll = !out_result->visible;
    if (out_result->visible) {
        out_result->affected_visible_cell_count = 1U;
    }

    return UiGridViewStatus::Success;
}
}
