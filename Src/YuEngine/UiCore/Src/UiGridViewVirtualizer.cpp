// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiGridViewVirtualizer.cpp

#include "YuEngine/UiCore/UiGridViewVirtualizer.h"

#include <cstddef>
#include <cstdint>
#include <limits>

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

bool TryMultiplyUint(std::uint32_t left, std::uint32_t right, std::uint32_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    const std::uint64_t product = static_cast<std::uint64_t>(left) * static_cast<std::uint64_t>(right);
    if (product > std::numeric_limits<std::uint32_t>::max()) {
        return false;
    }

    *out_value = static_cast<std::uint32_t>(product);
    return true;
}

bool SpanTooSmall(std::size_t capacity, std::uint32_t required_count) {
    const std::size_t required_size = static_cast<std::size_t>(required_count);
    return capacity < required_size;
}

bool IsOptionalIndexValid(std::uint32_t item_count, std::uint32_t item_index) {
    if (item_index == INVALID_UI_GRID_INDEX) {
        return true;
    }

    return item_index < item_count;
}

void SetFailure(UiGridViewVirtualizationResult *out_result, UiGridViewStatus status) {
    out_result->status = status;
}
}

UiGridViewStatus UiGridViewVirtualizer::Build(
    const UiGridViewVirtualizationDesc &desc,
    std::span<UiGridViewVirtualCellRecord> out_cells,
    UiGridViewVirtualizationResult *out_result) const {
    if (out_result == nullptr) {
        return UiGridViewStatus::InvalidOutputBuffer;
    }

    *out_result = UiGridViewVirtualizationResult{};
    out_result->first_visible_group = desc.first_visible_group;

    UiGridViewStatus status = ValidateDesc(desc);
    if (status != UiGridViewStatus::Success) {
        SetFailure(out_result, status);
        return status;
    }

    std::uint32_t group_count = 0U;
    if (!TryCountGroups(desc.grid_desc, &group_count)) {
        SetFailure(out_result, UiGridViewStatus::OutputCapacityExceeded);
        return UiGridViewStatus::OutputCapacityExceeded;
    }

    out_result->group_count = group_count;
    status = ValidateVisibleGroup(group_count, desc.first_visible_group);
    if (status != UiGridViewStatus::Success) {
        SetFailure(out_result, status);
        return status;
    }

    if (group_count == 0U) {
        out_result->status = UiGridViewStatus::Success;
        return UiGridViewStatus::Success;
    }

    const std::uint32_t visible_end =
        ClampRangeEnd(desc.first_visible_group, desc.grid_desc.visible_group_count, group_count);
    const std::uint32_t first_materialized_group =
        SubtractClamped(desc.first_visible_group, desc.grid_desc.buffer_group_count);
    const std::uint32_t buffered_visible_count =
        CountRange(desc.first_visible_group, visible_end) + desc.grid_desc.buffer_group_count;
    const std::uint32_t materialized_end =
        ClampRangeEnd(desc.first_visible_group, buffered_visible_count, group_count);
    const std::uint32_t materialized_group_count = CountRange(first_materialized_group, materialized_end);
    if (desc.grid_desc.pool_group_count < materialized_group_count) {
        SetFailure(out_result, UiGridViewStatus::InvalidPoolGroupCount);
        return UiGridViewStatus::InvalidPoolGroupCount;
    }

    if (IsFullPoolRejected(desc.grid_desc, group_count, materialized_group_count)) {
        SetFailure(out_result, UiGridViewStatus::FullPoolRejected);
        return UiGridViewStatus::FullPoolRejected;
    }

    std::uint32_t required_pool_cell_count = 0U;
    const bool pool_count_valid =
        TryMultiplyUint(materialized_group_count, desc.grid_desc.axis_cell_count, &required_pool_cell_count);
    if (!pool_count_valid) {
        SetFailure(out_result, UiGridViewStatus::OutputCapacityExceeded);
        return UiGridViewStatus::OutputCapacityExceeded;
    }

    out_result->visible_group_count = CountRange(desc.first_visible_group, visible_end);
    out_result->first_materialized_group = first_materialized_group;
    out_result->materialized_group_count = materialized_group_count;
    out_result->required_pool_group_count = materialized_group_count;
    out_result->required_pool_cell_count = required_pool_cell_count;

    std::uint32_t materialized_item_count = 0U;
    std::uint32_t visible_item_count = 0U;
    for (std::uint32_t group_index = first_materialized_group; group_index < materialized_end; ++group_index) {
        const std::uint32_t group_item_count = CountCellsInGroup(desc.grid_desc, group_index);
        materialized_item_count += group_item_count;
        if (group_index >= desc.first_visible_group && group_index < visible_end) {
            visible_item_count += group_item_count;
        }
    }

    out_result->materialized_item_count = materialized_item_count;
    out_result->visible_item_count = visible_item_count;

    if (SpanTooSmall(out_cells.size(), required_pool_cell_count)) {
        SetFailure(out_result, UiGridViewStatus::OutputCapacityExceeded);
        return UiGridViewStatus::OutputCapacityExceeded;
    }

    if (required_pool_cell_count > 0U && out_cells.data() == nullptr) {
        SetFailure(out_result, UiGridViewStatus::InvalidOutputBuffer);
        return UiGridViewStatus::InvalidOutputBuffer;
    }

    status = ResolveScrollTarget(desc, out_result);
    if (status != UiGridViewStatus::Success) {
        SetFailure(out_result, status);
        return status;
    }

    std::uint32_t output_index = 0U;
    std::uint32_t dirty_cell_count = 0U;
    for (std::uint32_t group_index = first_materialized_group; group_index < materialized_end; ++group_index) {
        const std::uint32_t group_item_count = CountCellsInGroup(desc.grid_desc, group_index);
        for (std::uint32_t cell_index = 0U; cell_index < desc.grid_desc.axis_cell_count; ++cell_index) {
            UiGridViewVirtualCellRecord record;
            record.pool_cell_index = output_index;
            record.group_index = group_index;
            record.cell_index = cell_index;
            record.has_item = cell_index < group_item_count;
            record.visible = record.has_item && IsGroupVisible(*out_result, group_index);
            record.reused = output_index < desc.existing_pool_cell_count;
            record.init_callback_required = !record.reused;
            record.update_callback_required = record.has_item;
            if (record.has_item) {
                record.item_index = group_index * desc.grid_desc.axis_cell_count + cell_index;
                record.selected = record.item_index == desc.selected_index;
                record.dirty = ShouldDirtyCell(desc, *out_result, record.item_index);
            }

            if (record.dirty) {
                ++dirty_cell_count;
            }

            out_cells[output_index] = record;
            ++output_index;
        }
    }

    out_result->status = UiGridViewStatus::Success;
    out_result->pool_cell_count = required_pool_cell_count;
    out_result->reused_cell_count = MinUint(desc.existing_pool_cell_count, required_pool_cell_count);
    out_result->created_cell_count = required_pool_cell_count - out_result->reused_cell_count;
    out_result->dirty_cell_count = dirty_cell_count;
    return UiGridViewStatus::Success;
}

UiGridViewStatus UiGridViewVirtualizer::ValidateDesc(const UiGridViewVirtualizationDesc &desc) const {
    if (desc.grid_desc.axis_cell_count == 0U) {
        return UiGridViewStatus::InvalidAxisCellCount;
    }

    if (desc.grid_desc.visible_group_count == 0U) {
        return UiGridViewStatus::InvalidVisibleGroupCount;
    }

    if (desc.grid_desc.pool_group_count == 0U) {
        return UiGridViewStatus::InvalidPoolGroupCount;
    }

    if (!IsOptionalIndexValid(desc.grid_desc.item_count, desc.selected_index)) {
        return UiGridViewStatus::InvalidItemIndex;
    }

    if (!IsOptionalIndexValid(desc.grid_desc.item_count, desc.previous_selected_index)) {
        return UiGridViewStatus::InvalidItemIndex;
    }

    if (!IsOptionalIndexValid(desc.grid_desc.item_count, desc.updated_item_index)) {
        return UiGridViewStatus::InvalidItemIndex;
    }

    if (!IsOptionalIndexValid(desc.grid_desc.item_count, desc.scroll_to_index)) {
        return UiGridViewStatus::InvalidItemIndex;
    }

    return UiGridViewStatus::Success;
}

bool UiGridViewVirtualizer::TryCountGroups(
    const UiGridViewDesc &desc,
    std::uint32_t *out_group_count) const {
    if (out_group_count == nullptr) {
        return false;
    }

    *out_group_count = 0U;
    if (desc.item_count == 0U) {
        return true;
    }

    const std::uint64_t adjusted_count =
        static_cast<std::uint64_t>(desc.item_count) + static_cast<std::uint64_t>(desc.axis_cell_count) - 1U;
    const std::uint64_t group_count = adjusted_count / static_cast<std::uint64_t>(desc.axis_cell_count);
    if (group_count > std::numeric_limits<std::uint32_t>::max()) {
        return false;
    }

    *out_group_count = static_cast<std::uint32_t>(group_count);
    return true;
}

std::uint32_t UiGridViewVirtualizer::CountCellsInGroup(
    const UiGridViewDesc &desc,
    std::uint32_t group_index) const {
    const std::uint64_t first_item_index =
        static_cast<std::uint64_t>(group_index) * static_cast<std::uint64_t>(desc.axis_cell_count);
    if (first_item_index >= desc.item_count) {
        return 0U;
    }

    const std::uint32_t first_item_index_uint = static_cast<std::uint32_t>(first_item_index);
    const std::uint32_t remaining = desc.item_count - first_item_index_uint;
    return MinUint(desc.axis_cell_count, remaining);
}

UiGridViewStatus UiGridViewVirtualizer::ValidateVisibleGroup(
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

bool UiGridViewVirtualizer::IsFullPoolRejected(
    const UiGridViewDesc &desc,
    std::uint32_t group_count,
    std::uint32_t materialized_group_count) const {
    if (group_count <= materialized_group_count) {
        return false;
    }

    return desc.pool_group_count >= group_count;
}

UiGridViewStatus UiGridViewVirtualizer::ResolveScrollTarget(
    const UiGridViewVirtualizationDesc &desc,
    UiGridViewVirtualizationResult *out_result) const {
    std::uint32_t scroll_target_index = desc.scroll_to_index;
    if (scroll_target_index == INVALID_UI_GRID_INDEX && desc.auto_scroll_to_selection) {
        scroll_target_index = desc.selected_index;
    }

    if (scroll_target_index == INVALID_UI_GRID_INDEX) {
        return UiGridViewStatus::Success;
    }

    UiGridViewIndexResult index_result{};
    UiGridViewSemantics semantics{};
    const UiGridViewStatus status =
        semantics.ResolveScrollToIndex(desc.grid_desc, desc.first_visible_group, scroll_target_index, &index_result);
    if (status != UiGridViewStatus::Success) {
        return status;
    }

    out_result->scroll_target_index = index_result.item_index;
    out_result->scroll_group_index = index_result.group_index;
    out_result->scroll_cell_index = index_result.cell_index;
    out_result->scroll_required = index_result.requires_scroll;
    return UiGridViewStatus::Success;
}

bool UiGridViewVirtualizer::IsGroupVisible(
    const UiGridViewVirtualizationResult &result,
    std::uint32_t group_index) const {
    if (group_index < result.first_visible_group) {
        return false;
    }

    const std::uint32_t visible_end = result.first_visible_group + result.visible_group_count;
    return group_index < visible_end;
}

bool UiGridViewVirtualizer::ShouldDirtyCell(
    const UiGridViewVirtualizationDesc &desc,
    const UiGridViewVirtualizationResult &result,
    std::uint32_t item_index) const {
    if (item_index == desc.updated_item_index) {
        return true;
    }

    if (desc.grid_desc.kind == UiGridViewKind::BtnGridView) {
        const bool has_selection_change =
            desc.selected_index != INVALID_UI_GRID_INDEX || desc.previous_selected_index != INVALID_UI_GRID_INDEX;
        if (!has_selection_change) {
            return false;
        }

        const std::uint32_t group_index = item_index / desc.grid_desc.axis_cell_count;
        return IsGroupVisible(result, group_index);
    }

    if (item_index == desc.selected_index) {
        return true;
    }

    return item_index == desc.previous_selected_index;
}
}
