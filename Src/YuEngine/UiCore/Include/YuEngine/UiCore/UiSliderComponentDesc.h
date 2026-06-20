// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiSliderComponentDesc.h

#pragma once

#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiSliderAxis.h"
#include "YuEngine/UiCore/UiSliderEventHookDesc.h"
#include "YuEngine/UiCore/UiSliderInteractionDesc.h"
#include "YuEngine/UiCore/UiSliderVisualDesc.h"

namespace yuengine::uicore {
struct UiSliderComponentDesc final {
    UiNodeId node_id;
    UiNodeId fill_node_id;
    UiNodeId handle_node_id;
    UiSliderInteractionDesc interaction;
    UiSliderEventHookDesc hooks;
    UiSliderVisualDesc fill_visual;
    UiSliderVisualDesc handle_visual;
    UiSliderAxis axis = UiSliderAxis::Horizontal;
    float min_value = 0.0F;
    float max_value = 1.0F;
    float value = 0.0F;
    float step_size = 0.0F;
    bool is_enabled = true;
    bool scissor_enabled = true;
};
}
