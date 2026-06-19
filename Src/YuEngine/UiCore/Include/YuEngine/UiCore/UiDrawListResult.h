// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDrawListResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiDrawListStatus.h"

namespace yuengine::uicore {
struct UiDrawListResult final {
    UiDrawListStatus status = UiDrawListStatus::Success;
    std::uint32_t element_count = 0U;
    std::uint32_t skipped_node_count = 0U;

    /**
     * @comment 检查 draw list build 是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiDrawListStatus::Success;
    }
};
}
