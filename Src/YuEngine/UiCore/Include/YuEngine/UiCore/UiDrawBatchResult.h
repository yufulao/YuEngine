// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDrawBatchResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiDrawBatchStatus.h"
#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
struct UiDrawBatchResult final {
    UiDrawBatchStatus status = UiDrawBatchStatus::Success;
    std::uint32_t draw_element_count = 0U;
    std::uint32_t batch_count = 0U;
    std::uint32_t failed_element_index = 0U;
    UiNodeId failed_node_id{};

    /**
     * @comment 检查 draw batch build 是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiDrawBatchStatus::Success;
    }
};
}
