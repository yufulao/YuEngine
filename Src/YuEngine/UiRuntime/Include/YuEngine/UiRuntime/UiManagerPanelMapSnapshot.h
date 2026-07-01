// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPanelMapSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerPanelMapStatus.h"
#include "YuEngine/UiRuntime/UiPanelId.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgsSnapshot.h"

namespace yuengine::uiruntime {
struct UiManagerPanelMapSnapshot final {
    std::uint32_t panel_capacity = 0U;
    std::uint32_t loaded_panel_count = 0U;
    std::uint32_t active_panel_count = 0U;
    std::uint32_t open_operation_count = 0U;
    std::uint32_t close_operation_count = 0U;
    std::uint32_t release_operation_count = 0U;
    std::uint32_t release_active_operation_count = 0U;
    std::uint32_t reused_loaded_count = 0U;
    std::uint32_t idempotent_open_count = 0U;
    std::uint32_t idempotent_close_count = 0U;
    std::uint32_t parameter_open_count = 0U;
    std::uint32_t empty_open_args_count = 0U;
    std::uint32_t reopen_open_args_update_count = 0U;
    std::uint32_t accepted_operation_count = 0U;
    std::uint32_t rejected_operation_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    UiPanelOpenArgsSnapshot last_open_args;
    UiPanelId last_failed_panel_id{};
    UiPanelOpenArgsSnapshot last_failed_open_args;
    UiManagerPanelMapStatus last_status = UiManagerPanelMapStatus::Success;
};
}
