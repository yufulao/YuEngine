// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiDrawListBuilder.cpp

#include "YuEngine/UiCore/UiDrawListBuilder.h"

#include <array>

#include "YuEngine/UiCore/UiCoreConstants.h"

namespace yuengine::uicore {
namespace {
bool IsDrawOrderBefore(const UiNodeRecord &left, const UiNodeRecord &right) {
    if (left.layer < right.layer) {
        return true;
    }

    if (left.layer > right.layer) {
        return false;
    }

    if (left.sibling_order < right.sibling_order) {
        return true;
    }

    if (left.sibling_order > right.sibling_order) {
        return false;
    }

    return left.node_id.value < right.node_id.value;
}

void SetResult(UiDrawListResult *out_result, UiDrawListStatus status, std::uint32_t element_count) {
    out_result->status = status;
    out_result->element_count = element_count;
}
}

UiDrawListStatus UiDrawListBuilder::Build(
    const UiNodeTree &tree,
    std::span<const UiDrawElementDesc> descs,
    std::span<UiDrawElement> out_elements,
    UiDrawListResult *out_result) const {
    const UiDrawListStatus validate_status = Validate(tree, descs, out_elements, out_result);
    if (validate_status != UiDrawListStatus::Success) {
        return validate_status;
    }

    std::array<bool, MAX_UI_NODE_COUNT> emitted{};
    std::uint32_t emitted_count = 0U;
    const std::size_t desc_count = descs.size();
    while (emitted_count < out_result->element_count) {
        bool found_desc = false;
        std::size_t best_index = 0U;
        UiNodeRecord best_record;

        for (std::size_t index = 0U; index < desc_count; ++index) {
            if (emitted[index]) {
                continue;
            }

            const UiNodeTreeResult query_result = tree.QueryNode(descs[index].node_id);
            if (!query_result.Succeeded()) {
                continue;
            }

            if (!query_result.record.is_visible) {
                emitted[index] = true;
                continue;
            }

            if (!found_desc || IsDrawOrderBefore(query_result.record, best_record)) {
                found_desc = true;
                best_index = index;
                best_record = query_result.record;
            }
        }

        if (!found_desc) {
            break;
        }

        out_elements[emitted_count] = BuildElement(best_record, descs[best_index]);
        emitted[best_index] = true;
        ++emitted_count;
    }

    out_result->element_count = emitted_count;
    out_result->status = UiDrawListStatus::Success;
    return UiDrawListStatus::Success;
}

UiDrawListStatus UiDrawListBuilder::Validate(
    const UiNodeTree &tree,
    std::span<const UiDrawElementDesc> descs,
    std::span<UiDrawElement> out_elements,
    UiDrawListResult *out_result) const {
    if (out_result == nullptr) {
        return UiDrawListStatus::InvalidOutputBuffer;
    }

    *out_result = UiDrawListResult{};
    const std::size_t max_count = static_cast<std::size_t>(MAX_UI_NODE_COUNT);
    if (descs.size() > max_count) {
        SetResult(out_result, UiDrawListStatus::OutputCapacityExceeded, 0U);
        return UiDrawListStatus::OutputCapacityExceeded;
    }

    std::uint32_t visible_count = 0U;
    for (const UiDrawElementDesc &desc : descs) {
        const UiNodeTreeResult query_result = tree.QueryNode(desc.node_id);
        if (!query_result.Succeeded()) {
            SetResult(out_result, UiDrawListStatus::NodeNotFound, 0U);
            return UiDrawListStatus::NodeNotFound;
        }

        if (!query_result.record.is_visible) {
            ++out_result->skipped_node_count;
            continue;
        }

        ++visible_count;
    }

    if (out_elements.size() < static_cast<std::size_t>(visible_count)) {
        SetResult(out_result, UiDrawListStatus::OutputCapacityExceeded, 0U);
        return UiDrawListStatus::OutputCapacityExceeded;
    }

    if ((visible_count > 0U) && (out_elements.data() == nullptr)) {
        SetResult(out_result, UiDrawListStatus::InvalidOutputBuffer, 0U);
        return UiDrawListStatus::InvalidOutputBuffer;
    }

    out_result->element_count = visible_count;
    return UiDrawListStatus::Success;
}

UiDrawElement UiDrawListBuilder::BuildElement(const UiNodeRecord &record, const UiDrawElementDesc &desc) const {
    UiDrawElement element;
    element.node_id = record.node_id;
    element.type = desc.type;
    element.rect = record.world_rect;
    element.clip_rect = record.content_rect;
    element.layer = record.layer;
    element.sibling_order = record.sibling_order;
    element.style_key = desc.style_key;
    element.material_key = desc.material_key;
    element.texture_key = desc.texture_key;
    element.text_key = desc.text_key;
    element.scissor_enabled = desc.scissor_enabled;
    return element;
}
}
