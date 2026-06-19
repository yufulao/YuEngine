// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDrawBatchKey.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiDrawElementType.h"
#include "YuEngine/UiCore/UiRect.h"

namespace yuengine::uicore {
struct UiDrawBatchKey final {
    UiDrawElementType type = UiDrawElementType::Rect;
    UiRect clip_rect;
    std::int32_t layer = 0;
    std::uint32_t style_key = 0U;
    std::uint32_t material_key = 0U;
    std::uint32_t texture_key = 0U;
    std::uint32_t text_key = 0U;
    bool scissor_enabled = false;
};
}
