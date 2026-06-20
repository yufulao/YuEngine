// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiButtonVisualStateDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiImageTint.h"

namespace yuengine::uicore {
struct UiButtonVisualStateDesc final {
    std::uint32_t image_sprite_key = 0U;
    std::uint32_t image_style_key = 0U;
    std::uint32_t image_material_key = 0U;
    UiImageTint image_tint;
    std::uint32_t text_style_key = 0U;
    std::uint32_t text_material_key = 0U;
    UiImageTint text_tint;
};
}
