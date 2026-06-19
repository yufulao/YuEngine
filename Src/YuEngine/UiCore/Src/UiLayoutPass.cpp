// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiLayoutPass.cpp

#include "YuEngine/UiCore/UiLayoutPass.h"

#include <array>

namespace yuengine::uicore {
namespace {
float PositiveOr(float value, float fallback) {
    if (value > 0.0F) {
        return value;
    }

    return fallback;
}

UiRectTransform BuildTransformFromWorldRect(const UiRect &parent_rect, const UiRect &target_rect) {
    UiRectTransform transform;
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.offset_min = {target_rect.x - parent_rect.x, target_rect.y - parent_rect.y};
    transform.offset_max = {
        (target_rect.x + target_rect.width) - (parent_rect.x + parent_rect.width),
        (target_rect.y + target_rect.height) - (parent_rect.y + parent_rect.height)};
    return transform;
}
}

UiLayoutPassResult UiLayoutPass::Apply(UiNodeTree *tree, std::span<const UiLayoutContainerDesc> containers) {
    result_ = UiLayoutPassResult{};
    if (tree == nullptr) {
        result_.status = UiLayoutPassStatus::InvalidTree;
        return result_;
    }

    for (const UiLayoutContainerDesc &desc : containers) {
        const UiLayoutPassStatus validate_status = ValidateContainer(*tree, desc);
        if (validate_status != UiLayoutPassStatus::Success) {
            result_.status = validate_status;
            return result_;
        }

        const UiLayoutPassStatus apply_status = ApplyContainer(tree, desc);
        if (apply_status != UiLayoutPassStatus::Success) {
            result_.status = apply_status;
            return result_;
        }

        ++result_.container_count;
    }

    result_.status = UiLayoutPassStatus::Success;
    return result_;
}

UiLayoutPassStatus UiLayoutPass::ApplyContainer(UiNodeTree *tree, const UiLayoutContainerDesc &desc) {
    const UiNodeTreeResult container_result = tree->QueryNode(desc.container_id);
    if (!container_result.Succeeded()) {
        return UiLayoutPassStatus::ContainerNotFound;
    }

    std::array<UiNodeRecord, MAX_UI_NODE_COUNT> children{};
    const std::uint32_t child_count = tree->ExportChildren(desc.container_id, children.data(), MAX_UI_NODE_COUNT);
    if (child_count == 0U) {
        return UiLayoutPassStatus::Success;
    }

    for (std::uint32_t index = 0U; index < child_count; ++index) {
        const UiRect child_rect = BuildChildRect(desc, container_result.record, index, child_count);
        if ((child_rect.width < 0.0F) || (child_rect.height < 0.0F)) {
            return UiLayoutPassStatus::InvalidItemSize;
        }

        if (desc.type == UiLayoutContainerType::Absolute) {
            ++result_.arranged_node_count;
            continue;
        }

        const UiLayoutPassStatus status =
            ApplyChildRect(tree, container_result.record, children[index].node_id, child_rect);
        if (status != UiLayoutPassStatus::Success) {
            return status;
        }

        ++result_.arranged_node_count;
    }

    ++result_.layout_rebuild_count;
    return UiLayoutPassStatus::Success;
}

UiLayoutPassStatus UiLayoutPass::ApplyChildRect(
    UiNodeTree *tree,
    const UiNodeRecord &container,
    UiNodeId child_id,
    UiRect rect) {
    const UiRectTransform transform = BuildTransformFromWorldRect(container.world_rect, rect);
    const UiNodeTreeResult result = tree->SetNodeRect(child_id, transform);
    if (!result.Succeeded()) {
        return UiLayoutPassStatus::TreeMutationFailed;
    }

    return UiLayoutPassStatus::Success;
}

UiLayoutPassStatus UiLayoutPass::ValidateContainer(
    const UiNodeTree &tree,
    const UiLayoutContainerDesc &desc) const {
    const UiNodeTreeResult container_result = tree.QueryNode(desc.container_id);
    if (!container_result.Succeeded()) {
        return UiLayoutPassStatus::ContainerNotFound;
    }

    if ((desc.spacing_x < 0.0F) || (desc.spacing_y < 0.0F)) {
        return UiLayoutPassStatus::InvalidItemSize;
    }

    if ((desc.item_width < 0.0F) || (desc.item_height < 0.0F)) {
        return UiLayoutPassStatus::InvalidItemSize;
    }

    if ((desc.type == UiLayoutContainerType::Grid) && (desc.grid_column_count == 0U)) {
        return UiLayoutPassStatus::InvalidGridColumnCount;
    }

    return UiLayoutPassStatus::Success;
}

UiRect UiLayoutPass::BuildChildRect(
    const UiLayoutContainerDesc &desc,
    const UiNodeRecord &container,
    std::uint32_t child_index,
    std::uint32_t child_count) const {
    const UiRect area = container.content_rect;
    switch (desc.type) {
        case UiLayoutContainerType::Absolute:
        {
            return UiRect{};
        }
        case UiLayoutContainerType::Stack:
        {
            return BuildStackRect(desc, area, child_index, child_count);
        }
        case UiLayoutContainerType::Grid:
        {
            return BuildGridRect(desc, area, child_index, child_count);
        }
        case UiLayoutContainerType::Overlay:
        {
            return BuildOverlayRect(area);
        }
        case UiLayoutContainerType::ScrollViewport:
        {
            return BuildScrollViewportRect(desc, area);
        }
        default:
        {
            break;
        }
    }

    return UiRect{};
}

UiRect UiLayoutPass::BuildStackRect(
    const UiLayoutContainerDesc &desc,
    const UiRect &area,
    std::uint32_t child_index,
    std::uint32_t child_count) const {
    const float child_count_value = static_cast<float>(child_count);
    const float gap_count = static_cast<float>(child_count - 1U);
    if (desc.stack_direction == UiStackDirection::Horizontal) {
        const float total_gap_width = desc.spacing_x * gap_count;
        const float available_width = area.width - total_gap_width;
        const float item_width = PositiveOr(desc.item_width, available_width / child_count_value);
        const float x = area.x + ((item_width + desc.spacing_x) * static_cast<float>(child_index));
        return UiRect{x, area.y, item_width, area.height};
    }

    const float total_gap_height = desc.spacing_y * gap_count;
    const float available_height = area.height - total_gap_height;
    const float item_height = PositiveOr(desc.item_height, available_height / child_count_value);
    const float y = area.y + area.height - item_height -
        ((item_height + desc.spacing_y) * static_cast<float>(child_index));
    return UiRect{area.x, y, area.width, item_height};
}

UiRect UiLayoutPass::BuildGridRect(
    const UiLayoutContainerDesc &desc,
    const UiRect &area,
    std::uint32_t child_index,
    std::uint32_t child_count) const {
    const std::uint32_t column_count = desc.grid_column_count;
    const std::uint32_t row_count = (child_count + column_count - 1U) / column_count;
    const std::uint32_t column_index = child_index % column_count;
    const std::uint32_t row_index = child_index / column_count;
    const float gap_width = desc.spacing_x * static_cast<float>(column_count - 1U);
    const float gap_height = desc.spacing_y * static_cast<float>(row_count - 1U);
    const float available_width = area.width - gap_width;
    const float available_height = area.height - gap_height;
    const float item_width = PositiveOr(desc.item_width, available_width / static_cast<float>(column_count));
    const float item_height = PositiveOr(desc.item_height, available_height / static_cast<float>(row_count));
    const float x = area.x + ((item_width + desc.spacing_x) * static_cast<float>(column_index));
    const float y = area.y + area.height - item_height -
        ((item_height + desc.spacing_y) * static_cast<float>(row_index));
    return UiRect{x, y, item_width, item_height};
}

UiRect UiLayoutPass::BuildOverlayRect(const UiRect &area) const {
    return area;
}

UiRect UiLayoutPass::BuildScrollViewportRect(const UiLayoutContainerDesc &desc, const UiRect &area) const {
    const float item_width = PositiveOr(desc.item_width, area.width);
    const float item_height = PositiveOr(desc.item_height, area.height);
    return UiRect{area.x - desc.scroll_offset.x, area.y - desc.scroll_offset.y, item_width, item_height};
}
}
