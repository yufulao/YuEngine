// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiGridViewRepresentativeSnapshot.h

#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include "YuEngine/UiRuntime/UiGridViewRepresentativeStatus.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgsSnapshot.h"

namespace yuengine::uiruntime {
constexpr std::uint32_t INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX =
    std::numeric_limits<std::uint32_t>::max();
constexpr std::uint32_t MAX_UI_GRID_VIEW_REPRESENTATIVE_CELL_COUNT = 24U;

struct UiGridViewRepresentativeCellSnapshot final {
    std::uint32_t pool_cell_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    std::uint32_t item_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    std::uint32_t group_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    std::uint32_t cell_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    std::uint32_t item_key = 0U;
    std::uint32_t stack_count = 0U;
    std::uint32_t price = 0U;
    bool has_item = false;
    bool visible = false;
    bool selected = false;
    bool reused = false;
    bool init_callback_required = false;
    bool update_callback_required = false;
    bool dirty = false;
};

struct UiGridViewRepresentativeSnapshot final {
    bool visible = false;
    bool displayed = false;
    bool grid_displayed = false;
    bool grid_cleared = false;
    bool cleared = false;
    std::uint32_t store_key = 0U;
    std::uint32_t item_count = 0U;
    std::uint32_t axis_cell_count = 0U;
    std::uint32_t visible_group_count = 0U;
    std::uint32_t buffer_group_count = 0U;
    std::uint32_t pool_group_count = 0U;
    std::uint32_t group_count = 0U;
    std::uint32_t first_visible_group = 0U;
    std::uint32_t first_materialized_group = 0U;
    std::uint32_t materialized_group_count = 0U;
    std::uint32_t materialized_item_count = 0U;
    std::uint32_t visible_item_count = 0U;
    std::uint32_t pool_cell_count = 0U;
    std::uint32_t reused_cell_count = 0U;
    std::uint32_t created_cell_count = 0U;
    std::uint32_t dirty_cell_count = 0U;
    std::uint32_t selected_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    std::uint32_t previous_selected_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    std::uint32_t updated_item_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    std::uint32_t scroll_target_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    std::uint32_t scroll_group_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    std::uint32_t scroll_cell_index = INVALID_UI_GRID_VIEW_REPRESENTATIVE_INDEX;
    bool scroll_required = false;
    UiPanelOpenArgsSnapshot open_args;
    std::array<UiGridViewRepresentativeCellSnapshot, MAX_UI_GRID_VIEW_REPRESENTATIVE_CELL_COUNT> cells{};
    std::uint32_t cell_count = 0U;
    std::uint32_t open_display_count = 0U;
    std::uint32_t close_display_count = 0U;
    std::uint32_t clear_display_count = 0U;
    std::uint32_t scroll_count = 0U;
    std::uint32_t selection_change_count = 0U;
    std::uint32_t refresh_count = 0U;
    std::uint32_t clear_grid_count = 0U;
    UiGridViewRepresentativeStatus last_status = UiGridViewRepresentativeStatus::Success;
};
}
