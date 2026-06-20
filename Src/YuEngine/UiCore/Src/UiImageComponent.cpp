// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiImageComponent.cpp

#include "YuEngine/UiCore/UiImageComponent.h"

#include <array>
#include <cstddef>

#include "YuEngine/UiCore/UiDrawElementType.h"
#include "YuEngine/UiCore/UiNodeTreeResult.h"

namespace yuengine::uicore {
namespace {
constexpr std::uint32_t SIMPLE_IMAGE_RECORD_COUNT = 1U;
constexpr std::uint32_t NINE_SLICE_RECORD_COUNT = 9U;
constexpr std::uint32_t AXIS_EDGE_COUNT = 4U;
constexpr std::uint32_t AXIS_RATIO_COUNT = 4U;

bool IsTintChannelValid(float value) {
    if (value < 0.0F) {
        return false;
    }

    return value <= 1.0F;
}

UiImageComponentStatus ValidateOutputStorage(
    std::span<UiImageDrawRecord> out_records,
    std::uint32_t record_count) {
    if (record_count == 0U) {
        return UiImageComponentStatus::Success;
    }

    if (out_records.size() < static_cast<std::size_t>(record_count)) {
        return UiImageComponentStatus::OutputCapacityExceeded;
    }

    if (out_records.data() == nullptr) {
        return UiImageComponentStatus::InvalidOutputBuffer;
    }

    return UiImageComponentStatus::Success;
}

UiImageComponentStatus MapAtlasStatus(UiStaticAtlasStatus status) {
    if (status == UiStaticAtlasStatus::Success) {
        return UiImageComponentStatus::Success;
    }

    if (status == UiStaticAtlasStatus::SpriteNotFound) {
        return UiImageComponentStatus::SpriteNotFound;
    }

    if (status == UiStaticAtlasStatus::InvalidSpriteKey) {
        return UiImageComponentStatus::InvalidDesc;
    }

    return UiImageComponentStatus::InvalidAtlasMetadata;
}

std::array<float, AXIS_EDGE_COUNT> BuildAxisEdges(float start, float size, float leading, float trailing) {
    std::array<float, AXIS_EDGE_COUNT> edges{};
    edges[0U] = start;
    edges[1U] = start + leading;
    edges[2U] = start + size - trailing;
    edges[3U] = start + size;
    return edges;
}

std::array<float, AXIS_RATIO_COUNT> BuildAxisRatios(float leading, float size, float trailing) {
    std::array<float, AXIS_RATIO_COUNT> ratios{};
    ratios[0U] = 0.0F;
    ratios[1U] = leading / size;
    ratios[2U] = 1.0F - (trailing / size);
    ratios[3U] = 1.0F;
    return ratios;
}

UiRect BuildSliceRect(
    const std::array<float, AXIS_EDGE_COUNT> &x_edges,
    const std::array<float, AXIS_EDGE_COUNT> &y_edges,
    std::uint32_t column,
    std::uint32_t row) {
    UiRect rect{};
    rect.x = x_edges[column];
    rect.y = y_edges[row];
    rect.width = x_edges[column + 1U] - x_edges[column];
    rect.height = y_edges[row + 1U] - y_edges[row];
    return rect;
}

void SetResult(
    UiImageComponentResult *out_result,
    UiImageComponentStatus status,
    std::uint32_t required_record_count) {
    out_result->status = status;
    out_result->required_draw_record_count = required_record_count;
}
}

UiImageComponentStatus UiImageComponent::Build(
    const UiNodeTree &tree,
    const UiStaticAtlasMetadataDesc &atlas_metadata,
    const UiImageComponentDesc &desc,
    std::span<UiImageDrawRecord> out_records,
    UiImageComponentResult *out_result) const {
    if (out_result == nullptr) {
        return UiImageComponentStatus::InvalidOutputBuffer;
    }

    *out_result = UiImageComponentResult{};
    out_result->node_id = desc.node_id;
    out_result->sprite_key = desc.sprite_key;
    UiImageComponentStatus status = ValidateDesc(desc);
    if (status != UiImageComponentStatus::Success) {
        SetResult(out_result, status, 0U);
        return status;
    }

    const UiNodeTreeResult node_result = tree.QueryNode(desc.node_id);
    if (!node_result.Succeeded()) {
        SetResult(out_result, UiImageComponentStatus::NodeNotFound, 0U);
        return UiImageComponentStatus::NodeNotFound;
    }

    UiStaticAtlasResolveResult sprite_result{};
    status = ResolveSprite(atlas_metadata, desc, &sprite_result);
    if (status != UiImageComponentStatus::Success) {
        SetResult(out_result, status, 0U);
        return status;
    }

    out_result->texture_key = sprite_result.texture_key;
    out_result->nine_slice = sprite_result.nine_slice;
    if (!node_result.record.is_visible) {
        SetResult(out_result, UiImageComponentStatus::Success, 0U);
        return UiImageComponentStatus::Success;
    }

    status = WriteRecords(node_result.record, sprite_result, desc, out_records, out_result);
    if (status != UiImageComponentStatus::Success) {
        return status;
    }

    out_result->status = UiImageComponentStatus::Success;
    return UiImageComponentStatus::Success;
}

UiImageComponentStatus UiImageComponent::ValidateDesc(const UiImageComponentDesc &desc) const {
    if (desc.node_id.value == 0U) {
        return UiImageComponentStatus::InvalidDesc;
    }

    if (desc.sprite_key == 0U) {
        return UiImageComponentStatus::InvalidDesc;
    }

    if (!IsTintChannelValid(desc.tint.red)) {
        return UiImageComponentStatus::InvalidDesc;
    }

    if (!IsTintChannelValid(desc.tint.green)) {
        return UiImageComponentStatus::InvalidDesc;
    }

    if (!IsTintChannelValid(desc.tint.blue)) {
        return UiImageComponentStatus::InvalidDesc;
    }

    if (!IsTintChannelValid(desc.tint.alpha)) {
        return UiImageComponentStatus::InvalidDesc;
    }

    return UiImageComponentStatus::Success;
}

UiImageComponentStatus UiImageComponent::ResolveSprite(
    const UiStaticAtlasMetadataDesc &atlas_metadata,
    const UiImageComponentDesc &desc,
    UiStaticAtlasResolveResult *out_sprite) const {
    if (out_sprite == nullptr) {
        return UiImageComponentStatus::InvalidOutputBuffer;
    }

    UiStaticAtlasMetadata metadata{};
    const UiStaticAtlasResolveResult result = metadata.ResolveSprite(atlas_metadata, desc.sprite_key);
    const UiImageComponentStatus status = MapAtlasStatus(result.status);
    if (status != UiImageComponentStatus::Success) {
        return status;
    }

    *out_sprite = result;
    return UiImageComponentStatus::Success;
}

UiImageComponentStatus UiImageComponent::WriteRecords(
    const UiNodeRecord &node_record,
    const UiStaticAtlasResolveResult &sprite,
    const UiImageComponentDesc &desc,
    std::span<UiImageDrawRecord> out_records,
    UiImageComponentResult *out_result) const {
    if (sprite.nine_slice.enabled) {
        return WriteNineSliceRecords(node_record, sprite, desc, out_records, out_result);
    }

    return WriteSimpleRecord(node_record, sprite, desc, out_records, out_result);
}

UiImageComponentStatus UiImageComponent::WriteSimpleRecord(
    const UiNodeRecord &node_record,
    const UiStaticAtlasResolveResult &sprite,
    const UiImageComponentDesc &desc,
    std::span<UiImageDrawRecord> out_records,
    UiImageComponentResult *out_result) const {
    const UiImageComponentStatus storage_status = ValidateOutputStorage(out_records, SIMPLE_IMAGE_RECORD_COUNT);
    if (storage_status != UiImageComponentStatus::Success) {
        SetResult(out_result, storage_status, SIMPLE_IMAGE_RECORD_COUNT);
        return storage_status;
    }

    out_records[0U] = BuildRecord(node_record, sprite, desc, node_record.world_rect, sprite.uv_rect, 0U);
    out_result->draw_record_count = SIMPLE_IMAGE_RECORD_COUNT;
    out_result->required_draw_record_count = SIMPLE_IMAGE_RECORD_COUNT;
    return UiImageComponentStatus::Success;
}

UiImageComponentStatus UiImageComponent::WriteNineSliceRecords(
    const UiNodeRecord &node_record,
    const UiStaticAtlasResolveResult &sprite,
    const UiImageComponentDesc &desc,
    std::span<UiImageDrawRecord> out_records,
    UiImageComponentResult *out_result) const {
    const float left = static_cast<float>(sprite.nine_slice.border_left);
    const float right = static_cast<float>(sprite.nine_slice.border_right);
    const float top = static_cast<float>(sprite.nine_slice.border_top);
    const float bottom = static_cast<float>(sprite.nine_slice.border_bottom);
    if ((left + right) > node_record.world_rect.width) {
        SetResult(out_result, UiImageComponentStatus::InvalidNineSliceTarget, NINE_SLICE_RECORD_COUNT);
        return UiImageComponentStatus::InvalidNineSliceTarget;
    }

    if ((top + bottom) > node_record.world_rect.height) {
        SetResult(out_result, UiImageComponentStatus::InvalidNineSliceTarget, NINE_SLICE_RECORD_COUNT);
        return UiImageComponentStatus::InvalidNineSliceTarget;
    }

    const UiImageComponentStatus storage_status = ValidateOutputStorage(out_records, NINE_SLICE_RECORD_COUNT);
    if (storage_status != UiImageComponentStatus::Success) {
        SetResult(out_result, storage_status, NINE_SLICE_RECORD_COUNT);
        return storage_status;
    }

    const std::array<float, AXIS_EDGE_COUNT> x_edges =
        BuildAxisEdges(node_record.world_rect.x, node_record.world_rect.width, left, right);
    const std::array<float, AXIS_EDGE_COUNT> y_edges =
        BuildAxisEdges(node_record.world_rect.y, node_record.world_rect.height, bottom, top);
    const std::array<float, AXIS_RATIO_COUNT> u_ratios =
        BuildAxisRatios(left, static_cast<float>(sprite.width), right);
    const std::array<float, AXIS_RATIO_COUNT> v_ratios =
        BuildAxisRatios(bottom, static_cast<float>(sprite.height), top);

    std::uint32_t slice_index = 0U;
    for (std::uint32_t row = 0U; row < 3U; ++row) {
        for (std::uint32_t column = 0U; column < 3U; ++column) {
            const UiRect rect = BuildSliceRect(x_edges, y_edges, column, row);
            const UiStaticAtlasUvRect uv_rect = BuildUvRect(
                sprite.uv_rect,
                u_ratios[column],
                v_ratios[row],
                u_ratios[column + 1U],
                v_ratios[row + 1U]);
            out_records[slice_index] = BuildRecord(node_record, sprite, desc, rect, uv_rect, slice_index);
            ++slice_index;
        }
    }

    out_result->draw_record_count = NINE_SLICE_RECORD_COUNT;
    out_result->required_draw_record_count = NINE_SLICE_RECORD_COUNT;
    return UiImageComponentStatus::Success;
}

UiImageDrawRecord UiImageComponent::BuildRecord(
    const UiNodeRecord &node_record,
    const UiStaticAtlasResolveResult &sprite,
    const UiImageComponentDesc &desc,
    UiRect rect,
    UiStaticAtlasUvRect uv_rect,
    std::uint32_t slice_index) const {
    UiImageDrawRecord record{};
    record.draw_element.node_id = node_record.node_id;
    record.draw_element.type = UiDrawElementType::TexturedQuad;
    record.draw_element.rect = rect;
    record.draw_element.clip_rect = node_record.content_rect;
    record.draw_element.layer = node_record.layer;
    record.draw_element.sibling_order = node_record.sibling_order;
    record.draw_element.style_key = desc.style_key;
    record.draw_element.material_key = desc.material_key;
    record.draw_element.texture_key = sprite.texture_key;
    record.draw_element.text_key = 0U;
    record.draw_element.scissor_enabled = desc.scissor_enabled;
    record.uv_rect = uv_rect;
    record.tint = desc.tint;
    record.sprite_key = sprite.sprite_key;
    record.slice_index = slice_index;
    return record;
}

UiStaticAtlasUvRect UiImageComponent::BuildUvRect(
    const UiStaticAtlasUvRect &base_rect,
    float u_min_ratio,
    float v_min_ratio,
    float u_max_ratio,
    float v_max_ratio) const {
    const float width = base_rect.u_max - base_rect.u_min;
    const float height = base_rect.v_max - base_rect.v_min;
    UiStaticAtlasUvRect rect{};
    rect.u_min = base_rect.u_min + (width * u_min_ratio);
    rect.v_min = base_rect.v_min + (height * v_min_ratio);
    rect.u_max = base_rect.u_min + (width * u_max_ratio);
    rect.v_max = base_rect.v_min + (height * v_max_ratio);
    return rect;
}
}
