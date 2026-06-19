// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiInvalidatedNode.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
struct UiInvalidatedNode final {
    UiNodeId node_id;
    std::uint32_t domains = 0U;
};
}
