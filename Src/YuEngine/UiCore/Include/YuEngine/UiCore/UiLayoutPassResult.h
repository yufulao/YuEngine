// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiLayoutPassResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiLayoutPassStatus.h"

namespace yuengine::uicore {
struct UiLayoutPassResult final {
    UiLayoutPassStatus status = UiLayoutPassStatus::Success;
    std::uint32_t container_count = 0U;
    std::uint32_t arranged_node_count = 0U;
    std::uint32_t layout_rebuild_count = 0U;

    /**
     * @comment 检查 layout pass 是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiLayoutPassStatus::Success;
    }
};
}
