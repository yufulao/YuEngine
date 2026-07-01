// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Platform/PlatformNativeSurface.h"
#include "YuEngine/Platform/PlatformWindowEvent.h"
#include "YuEngine/Platform/PlatformWindowEventType.h"
#include "YuEngine/Platform/PlatformWindowStatus.h"

namespace yuengine::platform {
struct PlatformWindowSnapshot {
    bool created = false;
    bool visible = false;
    bool focused = false;
    bool close_requested = false;
    bool minimized = false;
    bool restored = false;
    std::uint32_t client_width = 0U;
    std::uint32_t client_height = 0U;
    std::size_t queued_event_count = 0U;
    std::size_t event_queue_capacity = 0U;
    std::size_t required_queued_event_count = 0U;
    PlatformWindowEvent last_failed_event{};
    PlatformWindowEventType last_failed_event_type = PlatformWindowEventType::None;
    std::uint32_t last_failed_event_raw_code = 0U;
    std::int32_t last_failed_event_pointer_x = 0;
    std::int32_t last_failed_event_pointer_y = 0;
    std::int32_t last_failed_event_wheel_delta = 0;
    std::size_t last_failed_event_index = 0U;
    std::size_t last_failed_event_queue_capacity = 0U;
    std::size_t last_failed_queued_event_count = 0U;
    std::size_t last_required_queued_event_count = 0U;
    std::uint32_t dropped_event_count = 0U;
    std::size_t last_poll_output_capacity = 0U;
    std::size_t last_poll_output_event_count = 0U;
    std::size_t last_poll_queued_event_count = 0U;
    std::size_t last_required_poll_output_event_count = 0U;
    std::size_t last_first_undrained_poll_event_index = 0U;
    PlatformWindowEvent last_first_undrained_poll_event{};
    PlatformWindowStatus last_status = PlatformWindowStatus::NotCreated;
    PlatformNativeSurface native_surface;
};
}
