// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/BaseUiLifecycleSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/BaseUiLifecycleState.h"
#include "YuEngine/UiRuntime/BaseUiLifecycleStatus.h"

namespace yuengine::uiruntime {
struct BaseUiLifecycleSnapshot final {
    BaseUiLifecycleState state = BaseUiLifecycleState::Created;
    BaseUiLifecycleStatus last_status = BaseUiLifecycleStatus::Success;
    std::uint32_t initialize_count = 0U;
    std::uint32_t bind_event_count = 0U;
    std::uint32_t open_count = 0U;
    std::uint32_t close_count = 0U;
    std::uint32_t clear_count = 0U;
    std::uint32_t close_self_request_count = 0U;
    bool initialized = false;
    bool events_bound = false;
    bool open = false;
    bool destroyed = false;
};
}
