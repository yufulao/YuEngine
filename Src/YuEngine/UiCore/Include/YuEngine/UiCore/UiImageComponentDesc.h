// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiImageComponentDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiImageTint.h"
#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
struct UiImageComponentDesc final {
    UiNodeId node_id;
    std::uint32_t sprite_key = 0U;
    std::uint32_t style_key = 0U;
    std::uint32_t material_key = 0U;
    UiImageTint tint;
    bool scissor_enabled = true;
};
}
