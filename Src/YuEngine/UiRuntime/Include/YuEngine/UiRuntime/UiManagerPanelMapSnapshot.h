// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPanelMapSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerPanelMapStatus.h"

namespace yuengine::uiruntime {
struct UiManagerPanelMapSnapshot final {
    std::uint32_t panel_capacity = 0U;
    std::uint32_t loaded_panel_count = 0U;
    std::uint32_t active_panel_count = 0U;
    std::uint32_t open_operation_count = 0U;
    std::uint32_t close_operation_count = 0U;
    std::uint32_t reused_loaded_count = 0U;
    std::uint32_t idempotent_open_count = 0U;
    std::uint32_t idempotent_close_count = 0U;
    std::uint32_t accepted_operation_count = 0U;
    std::uint32_t rejected_operation_count = 0U;
    UiManagerPanelMapStatus last_status = UiManagerPanelMapStatus::Success;
};
}
