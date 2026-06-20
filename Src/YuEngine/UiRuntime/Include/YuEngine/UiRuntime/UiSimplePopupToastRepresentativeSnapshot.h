// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiSimplePopupToastRepresentativeSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiPanelOpenArgsSnapshot.h"

namespace yuengine::uiruntime {
struct UiSimplePopupToastRepresentativeSnapshot final {
    bool visible = false;
    bool displayed = false;
    bool cleared = false;
    std::uint32_t message_key = 0U;
    std::uint32_t style_key = 0U;
    UiPanelOpenArgsSnapshot open_args{};
    std::uint32_t open_display_count = 0U;
    std::uint32_t close_display_count = 0U;
    std::uint32_t clear_display_count = 0U;
};
}
