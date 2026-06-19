// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiHitTestRequest.h

#pragma once

#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiVector2.h"

namespace yuengine::uicore {
struct UiHitTestRequest final {
    UiVector2 point;
    UiNodeId root_id;
    bool clip_to_parent_content = true;
};
}
