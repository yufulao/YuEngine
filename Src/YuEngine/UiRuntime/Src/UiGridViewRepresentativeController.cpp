// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Src/UiGridViewRepresentativeController.cpp

#include "YuEngine/UiRuntime/UiGridViewRepresentativeController.h"

#include <span>

#include "YuEngine/UiCore/UiGridViewSemantics.h"
#include "YuEngine/UiCore/UiGridViewVirtualizer.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgsConstants.h"

namespace yuengine::uiruntime {
namespace {
constexpr std::uint32_t DEFAULT_ITEM_COUNT = 48U;
constexpr std::uint32_t DEFAULT_AXIS_CELL_COUNT = 4U;
constexpr std::uint32_t DEFAULT_VISIBLE_GROUP_COUNT = 2U;
constexpr std::uint32_t DEFAULT_BUFFER_GROUP_COUNT = 1U;
constexpr std::uint32_t DEFAULT_POOL_GROUP_COUNT = 4U;
constexpr std::uint32_t FIRST_VISIBLE_GROUP_ARG_INDEX = 0U;
constexpr std::uint32_t SELECTED_ITEM_ARG_INDEX = 1U;
constexpr std::uint32_t ITEM_KEY_BASE = 70000U;
constexpr std::uint32_t PRICE_BASE = 120U;

std::uint32_t ClampOpenArgValueCount(std::uint32_t value_count) {
    if (value_count > MAX_UI_PANEL_OPEN_ARG_VALUE_COUNT) {
        return MAX_UI_PANEL_OPEN_ARG_VALUE_COUNT;
    }

    return value_count;
}

UiPanelOpenArgsSnapshot CopyOpenArgs(const UiPanelOpenArgs &open_args) {
    UiPanelOpenArgsSnapshot snapshot{};
    snapshot.request_key = open_args.request_key;
    snapshot.value_count = ClampOpenArgValueCount(open_args.value_count);
    snapshot.has_args = snapshot.request_key != 0U || snapshot.value_count > 0U;
    if (snapshot.value_count == 0U) {
        return snapshot;
    }

    if (open_args.values == nullptr) {
        snapshot.value_count = 0U;
        snapshot.has_args = snapshot.request_key != 0U;
        return snapshot;
    }

    for (std::uint32_t index = 0U; index < snapshot.value_count; ++index) {
        snapshot.values[index] = open_args.values[index];
    }

    return snapshot;
}

std::uint32_t ReadOpenArgValue(
    const UiPanelOpenArgsSnapshot &snapshot,
    std::uint32_t value_index,
    std::uint32_t fallback_value) {
    if (value_index >= snapshot.value_count) {
        return fallback_value;
    }

    return snapshot.values[value_index];
}

std::uint32_t CountGroups(std::uint32_t item_count, std::uint32_t axis_cell_count) {
    if (item_count == 0U) {
        return 0U;
    }

    return (item_count + axis_cell_count - 1U) / axis_cell_count;
}

bool IsValidItemIndex(std::uint32_t item_count, std::uint32_t item_index) {
    return item_index < item_count;
}

yuengine::uicore::UiGridViewDesc MakeGridDesc(const UiGridViewRepresentativeSnapshot &snapshot) {
    yuengine::uicore::UiGridViewDesc desc{};
    desc.item_count = snapshot.item_count;
    desc.axis_cell_count = snapshot.axis_cell_count;
    desc.visible_group_count = snapshot.visible_group_count;
    desc.buffer_group_count = snapshot.buffer_group_count;
    desc.pool_group_count = snapshot.pool_group_count;
    desc.kind = yuengine::uicore::UiGridViewKind::BtnGridView;
    return desc;
}

UiGridViewRepresentativeStatus TranslateGridStatus(yuengine::uicore::UiGridViewStatus status) {
    if (status == yuengine::uicore::UiGridViewStatus::Success) {
        return UiGridViewRepresentativeStatus::Success;
    }

    if (status == yuengine::uicore::UiGridViewStatus::InvalidItemIndex) {
        return UiGridViewRepresentativeStatus::InvalidItemIndex;
    }

    if (status == yuengine::uicore::UiGridViewStatus::InvalidVisibleGroupIndex) {
        return UiGridViewRepresentativeStatus::InvalidGroupIndex;
    }

    if (status == yuengine::uicore::UiGridViewStatus::InvalidOutputBuffer ||
        status == yuengine::uicore::UiGridViewStatus::OutputCapacityExceeded) {
        return UiGridViewRepresentativeStatus::InvalidOutputBuffer;
    }

    if (status == yuengine::uicore::UiGridViewStatus::FullPoolRejected) {
        return UiGridViewRepresentativeStatus::FullPoolRejected;
    }

    return UiGridViewRepresentativeStatus::InvalidGridDesc;
}

UiGridViewRepresentativeCellSnapshot MakeCellSnapshot(
    const yuengine::uicore::UiGridViewVirtualCellRecord &cell_record) {
    UiGridViewRepresentativeCellSnapshot snapshot{};
    snapshot.pool_cell_index = cell_record.pool_cell_index;
    snapshot.item_index = cell_record.item_index;
    snapshot.group_index = cell_record.group_index;
    snapshot.cell_index = cell_record.cell_index;
    snapshot.has_item = cell_record.has_item;
    snapshot.visible = cell_record.visible;
    snapshot.selected = cell_record.selected;
    snapshot.reused = cell_record.reused;
    snapshot.init_callback_required = cell_record.init_callback_required;
    snapshot.update_callback_required = cell_record.update_callback_required;
    snapshot.dirty = cell_record.dirty;
    if (!cell_record.has_item) {
        return snapshot;
    }

    snapshot.item_key = ITEM_KEY_BASE + cell_record.item_index;
    snapshot.stack_count = cell_record.item_index % 5U + 1U;
    snapshot.price = PRICE_BASE + cell_record.item_index * 3U;
    return snapshot;
}
}

const UiGridViewRepresentativeSnapshot &
UiGridViewRepresentativeController::GetRepresentativeSnapshot() const {
    return representative_snapshot_;
}

UiGridViewRepresentativeStatus UiGridViewRepresentativeController::ScrollToItem(
    std::uint32_t item_index) {
    if (!IsValidItemIndex(representative_snapshot_.item_count, item_index)) {
        return SetLastStatus(UiGridViewRepresentativeStatus::InvalidItemIndex);
    }

    representative_snapshot_.first_visible_group = item_index / representative_snapshot_.axis_cell_count;
    ++representative_snapshot_.scroll_count;
    return RebuildGrid(item_index);
}

UiGridViewRepresentativeStatus UiGridViewRepresentativeController::ScrollToGroup(
    std::uint32_t group_index) {
    const std::uint32_t group_count =
        CountGroups(representative_snapshot_.item_count, representative_snapshot_.axis_cell_count);
    if (group_index >= group_count) {
        return SetLastStatus(UiGridViewRepresentativeStatus::InvalidGroupIndex);
    }

    representative_snapshot_.first_visible_group = group_index;
    ++representative_snapshot_.scroll_count;
    return RebuildGrid(INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX);
}

UiGridViewRepresentativeStatus UiGridViewRepresentativeController::SelectItem(
    std::uint32_t item_index) {
    if (!IsValidItemIndex(representative_snapshot_.item_count, item_index)) {
        return SetLastStatus(UiGridViewRepresentativeStatus::InvalidItemIndex);
    }

    representative_snapshot_.previous_selected_index = representative_snapshot_.selected_index;
    representative_snapshot_.selected_index = item_index;
    representative_snapshot_.updated_item_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    ++representative_snapshot_.selection_change_count;
    return RebuildGrid(INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX);
}

UiGridViewRepresentativeStatus UiGridViewRepresentativeController::RefreshItem(
    std::uint32_t item_index) {
    if (!IsValidItemIndex(representative_snapshot_.item_count, item_index)) {
        return SetLastStatus(UiGridViewRepresentativeStatus::InvalidItemIndex);
    }

    representative_snapshot_.updated_item_index = item_index;
    ++representative_snapshot_.refresh_count;
    return RebuildGrid(INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX);
}

UiGridViewRepresentativeStatus UiGridViewRepresentativeController::ClearGrid() {
    const std::uint32_t open_display_count = representative_snapshot_.open_display_count;
    const std::uint32_t close_display_count = representative_snapshot_.close_display_count;
    const std::uint32_t clear_display_count = representative_snapshot_.clear_display_count;
    const std::uint32_t scroll_count = representative_snapshot_.scroll_count;
    const std::uint32_t selection_change_count = representative_snapshot_.selection_change_count;
    const std::uint32_t refresh_count = representative_snapshot_.refresh_count;
    const std::uint32_t clear_grid_count = representative_snapshot_.clear_grid_count + 1U;
    const UiPanelOpenArgsSnapshot open_args = representative_snapshot_.open_args;
    const std::uint32_t store_key = representative_snapshot_.store_key;
    const bool visible = representative_snapshot_.visible;

    representative_snapshot_ = UiGridViewRepresentativeSnapshot{};
    representative_snapshot_.visible = visible;
    representative_snapshot_.displayed = false;
    representative_snapshot_.grid_displayed = false;
    representative_snapshot_.grid_cleared = true;
    representative_snapshot_.store_key = store_key;
    representative_snapshot_.open_args = open_args;
    representative_snapshot_.open_display_count = open_display_count;
    representative_snapshot_.close_display_count = close_display_count;
    representative_snapshot_.clear_display_count = clear_display_count;
    representative_snapshot_.scroll_count = scroll_count;
    representative_snapshot_.selection_change_count = selection_change_count;
    representative_snapshot_.refresh_count = refresh_count;
    representative_snapshot_.clear_grid_count = clear_grid_count;
    return SetLastStatus(UiGridViewRepresentativeStatus::Success);
}

BaseUiLifecycleStatus UiGridViewRepresentativeController::OnInitEvent() {
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiGridViewRepresentativeController::OnBindEvent() {
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiGridViewRepresentativeController::OnOpenEvent() {
    UiPanelOpenArgs open_args{};
    return OnOpenWithArgsEvent(open_args);
}

BaseUiLifecycleStatus UiGridViewRepresentativeController::OnOpenWithArgsEvent(
    const UiPanelOpenArgs &open_args) {
    representative_snapshot_.open_args = CopyOpenArgs(open_args);
    representative_snapshot_.store_key = representative_snapshot_.open_args.request_key;
    representative_snapshot_.item_count = DEFAULT_ITEM_COUNT;
    representative_snapshot_.axis_cell_count = DEFAULT_AXIS_CELL_COUNT;
    representative_snapshot_.visible_group_count = DEFAULT_VISIBLE_GROUP_COUNT;
    representative_snapshot_.buffer_group_count = DEFAULT_BUFFER_GROUP_COUNT;
    representative_snapshot_.pool_group_count = DEFAULT_POOL_GROUP_COUNT;
    representative_snapshot_.first_visible_group =
        ReadOpenArgValue(representative_snapshot_.open_args, FIRST_VISIBLE_GROUP_ARG_INDEX, 0U);
    representative_snapshot_.selected_index = ReadOpenArgValue(
        representative_snapshot_.open_args,
        SELECTED_ITEM_ARG_INDEX,
        INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX);
    representative_snapshot_.previous_selected_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    representative_snapshot_.updated_item_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    representative_snapshot_.visible = true;
    representative_snapshot_.displayed = true;
    representative_snapshot_.grid_cleared = false;
    representative_snapshot_.cleared = false;
    ++representative_snapshot_.open_display_count;

    const UiGridViewRepresentativeStatus status =
        RebuildGrid(INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX);
    if (status != UiGridViewRepresentativeStatus::Success) {
        return BaseUiLifecycleStatus::HookFailed;
    }

    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiGridViewRepresentativeController::OnCloseEvent() {
    representative_snapshot_.visible = false;
    representative_snapshot_.displayed = false;
    representative_snapshot_.grid_displayed = false;
    ++representative_snapshot_.close_display_count;
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiGridViewRepresentativeController::OnClearEvent() {
    const std::uint32_t open_display_count = representative_snapshot_.open_display_count;
    const std::uint32_t close_display_count = representative_snapshot_.close_display_count;
    const std::uint32_t clear_display_count = representative_snapshot_.clear_display_count + 1U;
    const std::uint32_t scroll_count = representative_snapshot_.scroll_count;
    const std::uint32_t selection_change_count = representative_snapshot_.selection_change_count;
    const std::uint32_t refresh_count = representative_snapshot_.refresh_count;
    const std::uint32_t clear_grid_count = representative_snapshot_.clear_grid_count;
    representative_snapshot_ = UiGridViewRepresentativeSnapshot{};
    representative_snapshot_.open_display_count = open_display_count;
    representative_snapshot_.close_display_count = close_display_count;
    representative_snapshot_.clear_display_count = clear_display_count;
    representative_snapshot_.scroll_count = scroll_count;
    representative_snapshot_.selection_change_count = selection_change_count;
    representative_snapshot_.refresh_count = refresh_count;
    representative_snapshot_.clear_grid_count = clear_grid_count;
    representative_snapshot_.cleared = true;
    return BaseUiLifecycleStatus::Success;
}

UiGridViewRepresentativeStatus UiGridViewRepresentativeController::RebuildGrid(
    std::uint32_t scroll_to_index) {
    std::array<yuengine::uicore::UiGridViewVirtualCellRecord, MAX_UI_GRID_VIEW_REPRESENTATIVE_CELL_COUNT> cells{};
    yuengine::uicore::UiGridViewVirtualizationDesc desc{};
    desc.grid_desc = MakeGridDesc(representative_snapshot_);
    desc.first_visible_group = representative_snapshot_.first_visible_group;
    desc.selected_index = representative_snapshot_.selected_index;
    desc.previous_selected_index = representative_snapshot_.previous_selected_index;
    desc.updated_item_index = representative_snapshot_.updated_item_index;
    desc.scroll_to_index = scroll_to_index;
    desc.existing_pool_cell_count = representative_snapshot_.pool_cell_count;

    yuengine::uicore::UiGridViewVirtualizationResult result{};
    yuengine::uicore::UiGridViewVirtualizer virtualizer{};
    const yuengine::uicore::UiGridViewStatus grid_status =
        virtualizer.Build(desc, std::span<yuengine::uicore::UiGridViewVirtualCellRecord>(cells), &result);
    const UiGridViewRepresentativeStatus status = TranslateGridStatus(grid_status);
    if (status != UiGridViewRepresentativeStatus::Success) {
        return SetLastStatus(status);
    }

    representative_snapshot_.group_count = result.group_count;
    representative_snapshot_.first_visible_group = result.first_visible_group;
    representative_snapshot_.visible_group_count = result.visible_group_count;
    representative_snapshot_.first_materialized_group = result.first_materialized_group;
    representative_snapshot_.materialized_group_count = result.materialized_group_count;
    representative_snapshot_.materialized_item_count = result.materialized_item_count;
    representative_snapshot_.visible_item_count = result.visible_item_count;
    representative_snapshot_.pool_cell_count = result.pool_cell_count;
    representative_snapshot_.reused_cell_count = result.reused_cell_count;
    representative_snapshot_.created_cell_count = result.created_cell_count;
    representative_snapshot_.dirty_cell_count = result.dirty_cell_count;
    representative_snapshot_.scroll_target_index = result.scroll_target_index;
    representative_snapshot_.scroll_group_index = result.scroll_group_index;
    representative_snapshot_.scroll_cell_index = result.scroll_cell_index;
    representative_snapshot_.scroll_required = result.scroll_required;
    representative_snapshot_.cell_count = result.pool_cell_count;
    representative_snapshot_.grid_displayed = result.visible_item_count > 0U;
    representative_snapshot_.displayed = representative_snapshot_.visible && representative_snapshot_.grid_displayed;
    representative_snapshot_.grid_cleared = false;

    for (std::uint32_t index = 0U; index < result.pool_cell_count; ++index) {
        representative_snapshot_.cells[index] = MakeCellSnapshot(cells[index]);
    }

    for (std::uint32_t index = result.pool_cell_count;
         index < MAX_UI_GRID_VIEW_REPRESENTATIVE_CELL_COUNT;
         ++index) {
        representative_snapshot_.cells[index] = UiGridViewRepresentativeCellSnapshot{};
    }

    return SetLastStatus(UiGridViewRepresentativeStatus::Success);
}

UiGridViewRepresentativeStatus UiGridViewRepresentativeController::SetLastStatus(
    UiGridViewRepresentativeStatus status) {
    representative_snapshot_.last_status = status;
    return status;
}
}
