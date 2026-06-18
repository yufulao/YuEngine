// 模块: YuEngine Object
// 文件: Src/YuEngine/Object/Include/YuEngine/Object/ObjectConstants.h

#pragma once

#include <cstdint>
#include <limits>

namespace yuengine::object {
constexpr std::uint32_t MAX_OBJECT_COUNT = 64U;
constexpr std::uint32_t MAX_OBJECT_TYPE_COUNT = 16U;
constexpr std::uint32_t INVALID_OBJECT_GENERATION = 0U;
constexpr std::uint32_t INVALID_OBJECT_SLOT = std::numeric_limits<std::uint32_t>::max();
}
