// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiButtonComponentDesc.h

#pragma once

#include "YuEngine/UiCore/UiButtonEventHookDesc.h"
#include "YuEngine/UiCore/UiButtonInteractionDesc.h"
#include "YuEngine/UiCore/UiButtonVisualStateDesc.h"
#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
struct UiButtonComponentDesc final {
    UiNodeId node_id;
    UiNodeId image_node_id;
    UiNodeId text_node_id;
    UiButtonInteractionDesc interaction;
    UiButtonEventHookDesc hooks;
    UiButtonVisualStateDesc normal_visual;
    UiButtonVisualStateDesc hover_visual;
    UiButtonVisualStateDesc pressed_visual;
    UiButtonVisualStateDesc disabled_visual;
    UiButtonVisualStateDesc selected_visual;
    bool is_enabled = true;
    bool is_selected = false;
    bool scissor_enabled = true;
};
}
