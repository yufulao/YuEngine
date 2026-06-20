// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiSliderVisualUpdate.h

#pragma once

#include "YuEngine/UiCore/UiImageComponentDesc.h"
#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiVector2.h"

namespace yuengine::uicore {
struct UiSliderVisualUpdate final {
    UiImageComponentDesc fill_image_desc;
    UiImageComponentDesc handle_image_desc;
    UiRect fill_rect;
    UiRect handle_rect;
    UiVector2 handle_center;
    bool has_fill_update = false;
    bool has_handle_update = false;
};
}
