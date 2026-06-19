// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDirtyState.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiDirtyDomain.h"

namespace yuengine::uicore {
struct UiDirtyState final {
    std::uint32_t domains = UI_DIRTY_NONE;
    std::uint32_t layout_rebuild_count = 0U;
    std::uint32_t paint_change_count = 0U;
    std::uint32_t hit_test_rebuild_count = 0U;

    /**
     * @comment 检查指定 dirty domain 是否存在。
     * @param domain 输入 dirty domain bit。
     * @return 存在时返回 true，否则返回 false。
     */
    bool HasDomain(std::uint32_t domain) const {
        return (domains & domain) != 0U;
    }
};
}
