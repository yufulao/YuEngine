// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiInvalidationRequest.h

#pragma once

#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiInvalidationScope.h"
#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
struct UiInvalidationRequest final {
    UiNodeId node_id;
    UiDirtyChangeType change_type = UiDirtyChangeType::PaintOnly;
    UiInvalidationScope scope = UiInvalidationScope::Self;
};
}
