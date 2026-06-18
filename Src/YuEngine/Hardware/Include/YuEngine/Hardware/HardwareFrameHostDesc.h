// Module: YuEngine Hardware
// File: Src/YuEngine/Hardware/Include/YuEngine/Hardware/HardwareFrameHostDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/Audio/AudioCallbackDeviceDesc.h"
#include "YuEngine/Input/InputBridgeDesc.h"
#include "YuEngine/Platform/PlatformWindowDesc.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"

namespace yuengine::hardware {
struct HardwareFrameHostDesc final {
    static constexpr std::size_t MIN_PLATFORM_EVENT_CAPACITY = 1U;
    static constexpr std::size_t DEFAULT_PLATFORM_EVENT_CAPACITY = 16U;
    static constexpr std::size_t MAX_PLATFORM_EVENT_CAPACITY = 64U;

    platform::PlatformWindowDesc window_desc{};
    input::InputBridgeDesc input_desc{};
    rhi::RhiDeviceDesc rhi_desc{};
    audio::AudioCallbackDeviceDesc audio_desc{};
    std::size_t platform_event_capacity = DEFAULT_PLATFORM_EVENT_CAPACITY;
    bool render_enabled = false;
    bool audio_enabled = false;
    bool require_audio_device = false;
};
}
