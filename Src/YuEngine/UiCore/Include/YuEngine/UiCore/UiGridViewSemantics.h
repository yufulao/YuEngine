// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiGridViewSemantics.h

#pragma once

#include <cstdint>
#include <limits>
#include <span>

namespace yuengine::uicore {
constexpr std::uint32_t INVALID_UI_GRID_INDEX = std::numeric_limits<std::uint32_t>::max();

enum class UiGridViewKind {
    GridView,
    BtnGridView
};

enum class UiGridViewStatus {
    Success,
    InvalidOutputBuffer,
    InvalidAxisCellCount,
    InvalidVisibleGroupCount,
    InvalidPoolGroupCount,
    InvalidVisibleGroupIndex,
    InvalidItemIndex,
    OutputCapacityExceeded,
    FullPoolRejected
};

struct UiGridViewDesc final {
    std::uint32_t item_count = 0U;
    std::uint32_t axis_cell_count = 1U;
    std::uint32_t visible_group_count = 1U;
    std::uint32_t buffer_group_count = 0U;
    std::uint32_t pool_group_count = 1U;
    UiGridViewKind kind = UiGridViewKind::GridView;
};

struct UiGridViewCellRecord final {
    std::uint32_t item_index = INVALID_UI_GRID_INDEX;
    std::uint32_t group_index = INVALID_UI_GRID_INDEX;
    std::uint32_t cell_index = INVALID_UI_GRID_INDEX;
    bool visible = false;
    bool selected = false;
};

struct UiGridViewRangeResult final {
    UiGridViewStatus status = UiGridViewStatus::InvalidOutputBuffer;
    std::uint32_t item_count = 0U;
    std::uint32_t group_count = 0U;
    std::uint32_t first_visible_group = 0U;
    std::uint32_t visible_group_count = 0U;
    std::uint32_t first_materialized_group = 0U;
    std::uint32_t materialized_group_count = 0U;
    std::uint32_t visible_cell_count = 0U;
    std::uint32_t materialized_cell_count = 0U;
    std::uint32_t required_pool_group_count = 0U;

    /**
     * @comment 检查 GridView range 语义是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiGridViewStatus::Success;
    }
};

struct UiGridViewIndexResult final {
    UiGridViewStatus status = UiGridViewStatus::InvalidOutputBuffer;
    std::uint32_t item_index = INVALID_UI_GRID_INDEX;
    std::uint32_t group_index = INVALID_UI_GRID_INDEX;
    std::uint32_t cell_index = INVALID_UI_GRID_INDEX;
    bool visible = false;
    bool requires_scroll = false;
    std::uint32_t affected_visible_cell_count = 0U;

    /**
     * @comment 检查 GridView index 语义是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiGridViewStatus::Success;
    }
};

class UiGridViewSemantics final {
public:
    /**
     * @comment 构建 FancyScrollView 风格 GridView 的可见与 buffer cell 语义。
     * @param desc GridView value 描述。
     * @param first_visible_group 当前第一个可见 group。
     * @param selected_index 当前选中 item；无选中时使用 INVALID_UI_GRID_INDEX。
     * @param out_cells 调用方持有的 cell record 输出 buffer。
     * @param out_result 输出 range 结果。
     * @return 显式语义状态。
     */
    UiGridViewStatus BuildVisibleRange(
        const UiGridViewDesc &desc,
        std::uint32_t first_visible_group,
        std::uint32_t selected_index,
        std::span<UiGridViewCellRecord> out_cells,
        UiGridViewRangeResult *out_result) const;

    /**
     * @comment 解析 scroll-to-index 的 group/cell 语义。
     * @param desc GridView value 描述。
     * @param first_visible_group 当前第一个可见 group。
     * @param item_index 目标 item index。
     * @param out_result 输出 index 结果。
     * @return 显式语义状态。
     */
    UiGridViewStatus ResolveScrollToIndex(
        const UiGridViewDesc &desc,
        std::uint32_t first_visible_group,
        std::uint32_t item_index,
        UiGridViewIndexResult *out_result) const;

    /**
     * @comment 解析选择变化影响的可见 cell 范围。
     * @param desc GridView value 描述。
     * @param range_result 已构建的可见 range。
     * @param selected_index 目标选中 item index。
     * @param out_result 输出 index 结果。
     * @return 显式语义状态。
     */
    UiGridViewStatus ResolveSelection(
        const UiGridViewDesc &desc,
        const UiGridViewRangeResult &range_result,
        std::uint32_t selected_index,
        UiGridViewIndexResult *out_result) const;

private:
    UiGridViewStatus ValidateDesc(const UiGridViewDesc &desc) const;
    std::uint32_t CountGroups(const UiGridViewDesc &desc) const;
    std::uint32_t CountCellsInGroup(const UiGridViewDesc &desc, std::uint32_t group_index) const;
    bool IsGroupVisible(const UiGridViewRangeResult &range_result, std::uint32_t group_index) const;
    bool IsFullPoolRejected(
        const UiGridViewDesc &desc,
        std::uint32_t group_count,
        std::uint32_t materialized_group_count) const;
    UiGridViewStatus ValidateVisibleGroup(
        std::uint32_t group_count,
        std::uint32_t first_visible_group) const;
    UiGridViewStatus FillIndexResult(
        const UiGridViewDesc &desc,
        std::uint32_t first_visible_group,
        std::uint32_t item_index,
        UiGridViewIndexResult *out_result) const;
};
}
