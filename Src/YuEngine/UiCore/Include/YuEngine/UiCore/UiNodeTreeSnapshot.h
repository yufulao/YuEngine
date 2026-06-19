// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiNodeTreeSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiNodeTreeStatus.h"

namespace yuengine::uicore {
struct UiNodeTreeSnapshot final {
    std::uint32_t node_capacity = 0U;
    std::uint32_t active_node_count = 0U;
    std::uint64_t created_node_count = 0U;
    std::uint64_t destroyed_node_count = 0U;
    std::uint32_t accepted_operation_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    UiNodeTreeStatus last_status = UiNodeTreeStatus::Success;
};
}
