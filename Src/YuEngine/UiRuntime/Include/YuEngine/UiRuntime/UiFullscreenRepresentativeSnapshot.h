// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiFullscreenRepresentativeSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiPanelOpenArgsSnapshot.h"

namespace yuengine::uiruntime {
struct UiFullscreenRepresentativeSnapshot final {
    bool visible = false;
    bool displayed = false;
    bool top_active = false;
    bool cleared = false;
    std::uint32_t screen_key = 0U;
    std::uint32_t display_mode_key = 0U;
    std::uint32_t focus_key = 0U;
    UiPanelOpenArgsSnapshot open_args{};
    std::uint32_t open_display_count = 0U;
    std::uint32_t close_display_count = 0U;
    std::uint32_t clear_display_count = 0U;
};
}
