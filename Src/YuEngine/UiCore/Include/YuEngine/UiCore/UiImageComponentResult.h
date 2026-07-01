// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiImageComponentResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiImageComponentStatus.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"

namespace yuengine::uicore {
struct UiImageComponentResult final {
    UiImageComponentStatus status = UiImageComponentStatus::InvalidOutputBuffer;
    UiNodeId node_id;
    std::uint32_t sprite_key = 0U;
    std::uint32_t texture_key = 0U;
    std::uint32_t draw_record_count = 0U;
    std::uint32_t required_draw_record_count = 0U;
    std::uint32_t capacity_entry_output_capacity = 0U;
    std::uint32_t capacity_entry_current_output_count = 0U;
    std::uint32_t capacity_entry_required_output_count = 0U;
    std::uint32_t failed_draw_record_index = 0U;
    std::uint32_t failed_slice_index = 0U;
    UiNodeId failed_node_id;
    std::uint32_t failed_sprite_key = 0U;
    std::uint32_t failed_texture_key = 0U;
    UiDirtyChangeType dirty_change_type = UiDirtyChangeType::PaintOnly;
    UiStaticAtlasNineSlice nine_slice;

    /**
     * @comment 检查 Image component 输出是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiImageComponentStatus::Success;
    }
};
}
