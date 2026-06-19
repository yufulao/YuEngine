// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiNodeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiRectTransform.h"

namespace yuengine::uicore {
struct UiNodeDesc final {
    UiNodeId node_id;
    UiNodeId parent_id;
    UiRectTransform rect_transform;
    std::uint32_t sibling_order = 0U;
    std::int32_t layer = 0;
    bool is_visible = true;
    bool is_enabled = true;
    bool is_hit_testable = true;
};
}
