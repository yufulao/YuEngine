// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerFullscreenStackSnapshot.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerFullscreenStackConstants.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStackResult.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStackStatus.h"
#include "YuEngine/UiRuntime/UiPanelId.h"

namespace yuengine::uiruntime {
struct UiManagerFullscreenStackSnapshot final {
    std::array<UiPanelId, MAX_UI_MANAGER_FULLSCREEN_STACK_COUNT> fullscreen_order;
    UiPanelId top_panel_id;
    std::uint32_t fullscreen_capacity = MAX_UI_MANAGER_FULLSCREEN_STACK_COUNT;
    std::uint32_t fullscreen_count = 0U;
    std::uint32_t last_required_fullscreen_order_count = 0U;
    UiPanelId last_failed_panel_id;
    UiPanelId last_failed_previous_top_panel_id;
    std::uint32_t last_failed_fullscreen_order_index = MAX_UI_MANAGER_FULLSCREEN_STACK_COUNT;
    UiManagerFullscreenStackOperation last_failed_operation = UiManagerFullscreenStackOperation::None;
    std::uint32_t open_operation_count = 0U;
    std::uint32_t back_navigation_operation_count = 0U;
    std::uint32_t close_operation_count = 0U;
    std::uint32_t release_operation_count = 0U;
    std::uint32_t restore_operation_count = 0U;
    std::uint32_t idempotent_open_count = 0U;
    std::uint32_t idempotent_close_count = 0U;
    std::uint32_t accepted_operation_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    std::uint32_t rejected_operation_count = 0U;
    UiManagerFullscreenStackStatus last_status = UiManagerFullscreenStackStatus::Success;
};
}
