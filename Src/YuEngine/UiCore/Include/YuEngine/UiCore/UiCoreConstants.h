// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiCoreConstants.h

#pragma once

#include <cstdint>
#include <limits>

namespace yuengine::uicore {
constexpr std::uint32_t MAX_UI_NODE_COUNT = 128U;
constexpr std::uint32_t INVALID_UI_NODE_ID_VALUE = 0U;
constexpr std::uint32_t INVALID_UI_NODE_ORDER = std::numeric_limits<std::uint32_t>::max();
}
