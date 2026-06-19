// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDrawElementDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiDrawElementType.h"
#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
struct UiDrawElementDesc final {
    UiNodeId node_id;
    UiDrawElementType type = UiDrawElementType::Rect;
    std::uint32_t style_key = 0U;
    std::uint32_t material_key = 0U;
    std::uint32_t texture_key = 0U;
    std::uint32_t text_key = 0U;
    bool scissor_enabled = false;
};
}
