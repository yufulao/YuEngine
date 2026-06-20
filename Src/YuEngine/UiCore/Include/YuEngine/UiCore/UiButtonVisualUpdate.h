// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiButtonVisualUpdate.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiImageComponentDesc.h"
#include "YuEngine/UiCore/UiImageTint.h"
#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
struct UiButtonTextVisualUpdate final {
    UiNodeId node_id;
    std::uint32_t style_key = 0U;
    std::uint32_t material_key = 0U;
    UiImageTint tint;
    bool enabled = false;
};

struct UiButtonVisualUpdate final {
    UiImageComponentDesc image_desc;
    UiButtonTextVisualUpdate text_desc;
    bool has_image_update = false;
    bool has_text_update = false;
};
}
