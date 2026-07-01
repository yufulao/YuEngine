// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiInvalidationResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiCacheCounters.h"
#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiDirtyState.h"
#include "YuEngine/UiCore/UiInvalidationScope.h"
#include "YuEngine/UiCore/UiInvalidationStatus.h"
#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
struct UiInvalidationResult final {
    UiInvalidationStatus status = UiInvalidationStatus::Success;
    UiDirtyState dirty_state;
    UiCacheCounters cache_counters;
    std::uint32_t affected_node_count = 0U;
    UiNodeId failed_request_node_id{};
    UiInvalidationScope failed_scope = UiInvalidationScope::Self;
    UiDirtyChangeType failed_change_type = UiDirtyChangeType::PaintOnly;
    std::uint32_t failed_output_node_capacity = 0U;
    std::uint32_t current_affected_node_count = 0U;
    std::uint32_t required_affected_node_count = 0U;

    /**
     * @comment 检查 invalidation 是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiInvalidationStatus::Success;
    }
};
}
