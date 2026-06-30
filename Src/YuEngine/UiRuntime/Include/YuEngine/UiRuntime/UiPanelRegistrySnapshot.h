// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiPanelRegistrySnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiPanelRegistryStatus.h"

namespace yuengine::uiruntime {
struct UiPanelRegistrySnapshot final {
    std::uint32_t panel_capacity = 0U;
    std::uint32_t registered_panel_count = 0U;
    std::uint32_t accepted_operation_count = 0U;
    std::uint32_t rejected_operation_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    std::uint32_t duplicate_panel_rejected_count = 0U;
    UiPanelRegistryStatus last_status = UiPanelRegistryStatus::Success;
};
}
