// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDirtyDomain.h

#pragma once

#include <cstdint>

namespace yuengine::uicore {
constexpr std::uint32_t UI_DIRTY_NONE = 0U;
constexpr std::uint32_t UI_DIRTY_LAYOUT = 1U << 0U;
constexpr std::uint32_t UI_DIRTY_PAINT = 1U << 1U;
constexpr std::uint32_t UI_DIRTY_TRANSFORM = 1U << 2U;
constexpr std::uint32_t UI_DIRTY_HIT_TEST = 1U << 3U;
constexpr std::uint32_t UI_DIRTY_TEXT = 1U << 4U;
}
