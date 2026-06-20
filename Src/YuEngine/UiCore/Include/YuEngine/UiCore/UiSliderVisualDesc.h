// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiSliderVisualDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiImageTint.h"

namespace yuengine::uicore {
struct UiSliderVisualDesc final {
    std::uint32_t sprite_key = 0U;
    std::uint32_t style_key = 0U;
    std::uint32_t material_key = 0U;
    UiImageTint tint;
};
}
