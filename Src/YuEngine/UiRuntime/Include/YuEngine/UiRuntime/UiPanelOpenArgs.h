// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiPanelOpenArgs.h

#pragma once

#include <cstdint>

namespace yuengine::uiruntime {
struct UiPanelOpenArgs final {
    std::uint32_t request_key = 0U;
    const std::uint32_t *values = nullptr;
    std::uint32_t value_count = 0U;

    /**
     * @comment 检查本次 open 是否携带参数。
     * @return 携带 request key 或 value 时返回 true，否则返回 false。
     */
    bool HasArgs() const {
        if (request_key != 0U) {
            return true;
        }

        return value_count > 0U;
    }

    /**
     * @comment 检查参数值是否需要外部存储。
     * @return 需要读取 values 时返回 true，否则返回 false。
     */
    bool HasValues() const {
        return value_count > 0U;
    }
};
}
