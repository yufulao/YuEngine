// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiNodeTreeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiCoreConstants.h"
#include "YuEngine/UiCore/UiRect.h"

namespace yuengine::uicore {
struct UiNodeTreeDesc final {
    std::uint32_t node_capacity = MAX_UI_NODE_COUNT;
    UiRect viewport_rect{0.0F, 0.0F, 0.0F, 0.0F};
};
}
