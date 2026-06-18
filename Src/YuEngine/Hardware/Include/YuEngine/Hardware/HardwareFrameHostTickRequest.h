// Module: YuEngine Hardware
// File: Src/YuEngine/Hardware/Include/YuEngine/Hardware/HardwareFrameHostTickRequest.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Input/InputBridgeEvent.h"
#include "YuEngine/Platform/PlatformWindowEvent.h"
#include "YuEngine/Rhi/RhiColor.h"

namespace yuengine::hardware {
struct HardwareFrameHostTickRequest final {
    std::span<const platform::PlatformWindowEvent> injected_platform_events{};
    std::span<input::InputBridgeEvent> input_events{};
    std::size_t *out_input_event_count = nullptr;
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget = 0U;
    std::span<const std::int16_t> audio_samples{};
    std::size_t audio_frame_count = 0U;
    std::uint64_t audio_completion_target = 0U;
    std::uint32_t audio_wait_timeout_milliseconds = 0U;
    rhi::RhiColor clear_color{};
    std::uint32_t frame_id = 0U;
};
}
