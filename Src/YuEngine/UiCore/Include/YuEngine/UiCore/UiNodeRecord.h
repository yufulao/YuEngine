// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiNodeRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiCore/UiVector2.h"

namespace yuengine::uicore {
struct UiNodeRecord final {
    UiNodeId node_id;
    UiNodeId parent_id;
    UiRectTransform rect_transform;
    UiRect world_rect;
    UiRect content_rect;
    UiVector2 pivot_point;
    std::uint32_t sibling_order = 0U;
    std::int32_t layer = 0;
    bool is_visible = true;
    bool is_enabled = true;
    bool is_hit_testable = true;
    bool is_active = false;
};
}
