// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiButtonEventHookDesc.h

#pragma once

#include <cstdint>

namespace yuengine::uicore {
struct UiButtonEventHookDesc final {
    std::uint32_t activation_event_key = 0U;
    std::uint32_t activation_sound_key = 0U;
};
}
