// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiGridViewVirtualizer.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiGridViewSemantics.h"

namespace yuengine::uicore {
struct UiGridViewVirtualizationDesc final {
    UiGridViewDesc grid_desc;
    std::uint32_t first_visible_group = 0U;
    std::uint32_t selected_index = INVALID_UI_GRID_INDEX;
    std::uint32_t previous_selected_index = INVALID_UI_GRID_INDEX;
    std::uint32_t updated_item_index = INVALID_UI_GRID_INDEX;
    std::uint32_t scroll_to_index = INVALID_UI_GRID_INDEX;
    std::uint32_t existing_pool_cell_count = 0U;
    bool auto_scroll_to_selection = false;
};

struct UiGridViewVirtualCellRecord final {
    std::uint32_t pool_cell_index = INVALID_UI_GRID_INDEX;
    std::uint32_t item_index = INVALID_UI_GRID_INDEX;
    std::uint32_t group_index = INVALID_UI_GRID_INDEX;
    std::uint32_t cell_index = INVALID_UI_GRID_INDEX;
    bool has_item = false;
    bool visible = false;
    bool selected = false;
    bool reused = false;
    bool init_callback_required = false;
    bool update_callback_required = false;
    bool dirty = false;
};

struct UiGridViewVirtualizationResult final {
    UiGridViewStatus status = UiGridViewStatus::InvalidOutputBuffer;
    std::uint32_t group_count = 0U;
    std::uint32_t first_visible_group = 0U;
    std::uint32_t visible_group_count = 0U;
    std::uint32_t first_materialized_group = 0U;
    std::uint32_t materialized_group_count = 0U;
    std::uint32_t required_pool_group_count = 0U;
    std::uint32_t required_pool_cell_count = 0U;
    std::uint32_t capacity_entry_pool_cell_capacity = 0U;
    std::uint32_t capacity_entry_current_pool_cell_count = 0U;
    std::uint32_t capacity_entry_required_pool_cell_count = 0U;
    std::uint32_t failed_pool_group_index = INVALID_UI_GRID_INDEX;
    std::uint32_t failed_pool_cell_index = INVALID_UI_GRID_INDEX;
    std::uint32_t pool_cell_count = 0U;
    std::uint32_t materialized_item_count = 0U;
    std::uint32_t visible_item_count = 0U;
    std::uint32_t reused_cell_count = 0U;
    std::uint32_t created_cell_count = 0U;
    std::uint32_t dirty_cell_count = 0U;
    std::uint32_t scroll_target_index = INVALID_UI_GRID_INDEX;
    std::uint32_t scroll_group_index = INVALID_UI_GRID_INDEX;
    std::uint32_t scroll_cell_index = INVALID_UI_GRID_INDEX;
    bool scroll_required = false;

    /**
     * @comment 检查 GridView virtualization 是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiGridViewStatus::Success;
    }
};

class UiGridViewVirtualizer final {
public:
    /**
     * @comment 构建固定尺寸 GridView/List 的可见加 buffer 虚拟化池。
     * @param desc virtualization 描述。
     * @param out_cells 调用方持有的 pooled cell 输出 buffer。
     * @param out_result 输出 virtualization 结果。
     * @return 显式 GridView 状态。
     */
    UiGridViewStatus Build(
        const UiGridViewVirtualizationDesc &desc,
        std::span<UiGridViewVirtualCellRecord> out_cells,
        UiGridViewVirtualizationResult *out_result) const;

private:
    UiGridViewStatus ValidateDesc(const UiGridViewVirtualizationDesc &desc) const;
    bool TryCountGroups(const UiGridViewDesc &desc, std::uint32_t *out_group_count) const;
    std::uint32_t CountCellsInGroup(const UiGridViewDesc &desc, std::uint32_t group_index) const;
    UiGridViewStatus ValidateVisibleGroup(std::uint32_t group_count, std::uint32_t first_visible_group) const;
    bool IsFullPoolRejected(
        const UiGridViewDesc &desc,
        std::uint32_t group_count,
        std::uint32_t materialized_group_count) const;
    UiGridViewStatus ResolveScrollTarget(
        const UiGridViewVirtualizationDesc &desc,
        UiGridViewVirtualizationResult *out_result) const;
    bool IsGroupVisible(
        const UiGridViewVirtualizationResult &result,
        std::uint32_t group_index) const;
    bool ShouldDirtyCell(
        const UiGridViewVirtualizationDesc &desc,
        const UiGridViewVirtualizationResult &result,
        std::uint32_t item_index) const;
};
}
