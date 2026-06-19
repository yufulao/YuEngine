// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiRectTransform.h

#pragma once

#include "YuEngine/UiCore/UiThickness.h"
#include "YuEngine/UiCore/UiVector2.h"

namespace yuengine::uicore {
struct UiRectTransform final {
    UiVector2 anchor_min{0.0F, 0.0F};
    UiVector2 anchor_max{0.0F, 0.0F};
    UiVector2 pivot{0.5F, 0.5F};
    UiVector2 offset_min{0.0F, 0.0F};
    UiVector2 offset_max{0.0F, 0.0F};
    UiThickness margin;
    UiThickness padding;
    float dpi_scale = 1.0F;
};
}
