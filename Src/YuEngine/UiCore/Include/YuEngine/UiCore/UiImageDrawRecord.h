// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiImageDrawRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiDrawElement.h"
#include "YuEngine/UiCore/UiImageTint.h"
#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"

namespace yuengine::uicore {
struct UiImageDrawRecord final {
    UiDrawElement draw_element;
    UiStaticAtlasUvRect uv_rect;
    UiImageTint tint;
    std::uint32_t sprite_key = 0U;
    std::uint32_t slice_index = 0U;
};
}
