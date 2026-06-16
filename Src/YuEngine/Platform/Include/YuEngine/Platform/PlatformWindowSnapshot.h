// Module: YuEngine Platform
// File: Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Platform/PlatformNativeSurface.h"
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
    std::uint32_t dropped_event_count = 0U;
    PlatformWindowStatus last_status = PlatformWindowStatus::NotCreated;
    PlatformNativeSurface native_surface;
};
}
