// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiLayoutContainerDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiLayoutContainerType.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiStackDirection.h"
#include "YuEngine/UiCore/UiVector2.h"

namespace yuengine::uicore {
struct UiLayoutContainerDesc final {
    UiNodeId container_id;
    UiLayoutContainerType type = UiLayoutContainerType::Absolute;
    UiStackDirection stack_direction = UiStackDirection::Vertical;
    std::uint32_t grid_column_count = 1U;
    float item_width = 0.0F;
    float item_height = 0.0F;
    float spacing_x = 0.0F;
    float spacing_y = 0.0F;
    UiVector2 scroll_offset;
};
}
