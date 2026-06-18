// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldConstants.h

#pragma once

#include <cstdint>

namespace yuengine::world {
inline constexpr std::uint32_t INVALID_WORLD_OBJECT_ID_VALUE = 0U;
inline constexpr std::uint32_t MAX_WORLD_OBJECT_COUNT = 64U;
inline constexpr std::uint32_t WORLD_UPDATE_PHASE_COUNT = 4U;
inline constexpr std::uint32_t MAX_WORLD_PHASE_TRACE_COUNT = 32U;
}
