// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiPanelOpenArgsSnapshot.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/UiRuntime/UiPanelOpenArgs.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgsConstants.h"

namespace yuengine::uiruntime {
struct UiPanelOpenArgsSnapshot final {
    std::uint32_t request_key = 0U;
    std::uint32_t value_count = 0U;
    std::array<std::uint32_t, MAX_UI_PANEL_OPEN_ARG_VALUE_COUNT> values{};
    bool has_args = false;

    /**
     * @comment 将已复制的 open 参数快照转换为只读调用视图。
     * @return 指向快照内部值存储的参数视图。
     */
    UiPanelOpenArgs ToArgs() const {
        UiPanelOpenArgs args{};
        args.request_key = request_key;
        args.value_count = value_count;
        if (value_count == 0U) {
            return args;
        }

        args.values = values.data();
        return args;
    }
};
}
