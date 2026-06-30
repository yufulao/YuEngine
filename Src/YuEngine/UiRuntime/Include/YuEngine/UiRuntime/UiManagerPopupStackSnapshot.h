// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPopupStackSnapshot.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerPopupStackConstants.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackStatus.h"
#include "YuEngine/UiRuntime/UiPanelId.h"

namespace yuengine::uiruntime {
struct UiManagerPopupStackSnapshot final {
    std::array<UiPanelId, MAX_UI_MANAGER_POPUP_STACK_COUNT> popup_order;
    UiPanelId top_panel_id;
    std::uint32_t popup_capacity = MAX_UI_MANAGER_POPUP_STACK_COUNT;
    std::uint32_t popup_count = 0U;
    std::uint32_t open_operation_count = 0U;
    std::uint32_t bring_to_top_operation_count = 0U;
    std::uint32_t close_operation_count = 0U;
    std::uint32_t release_operation_count = 0U;
    std::uint32_t idempotent_open_count = 0U;
    std::uint32_t idempotent_bring_to_top_count = 0U;
    std::uint32_t idempotent_close_count = 0U;
    std::uint32_t accepted_operation_count = 0U;
    std::uint32_t rejected_operation_count = 0U;
    std::uint64_t failed_operation_count = 0ULL;
    UiManagerPopupStackStatus last_status = UiManagerPopupStackStatus::Success;
};
}
