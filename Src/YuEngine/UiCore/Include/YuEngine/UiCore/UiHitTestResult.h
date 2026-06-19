// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiHitTestResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiHitTestStatus.h"
#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
struct UiHitTestResult final {
    UiHitTestStatus status = UiHitTestStatus::Miss;
    UiNodeId node_id;
    std::int32_t layer = 0;
    std::uint32_t sibling_order = 0U;

    /**
     * @comment 检查 hit-test 是否命中。
     * @return 命中时返回 true，否则返回 false。
     */
    bool Hit() const {
        return status == UiHitTestStatus::Success;
    }
};
}
