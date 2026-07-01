// 模块: YuEngine Hardware
// 文件: Src/YuEngine/Hardware/Include/YuEngine/Hardware/HardwareFrameHostSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioStatus.h"
#include "YuEngine/Hardware/HardwareFrameHostStatus.h"
#include "YuEngine/Input/InputStatus.h"
#include "YuEngine/Platform/PlatformWindowStatus.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::hardware {
struct HardwareFrameHostSnapshot final {
    std::uint64_t tick_count = 0U;
    std::uint64_t completed_tick_count = 0U;
    std::uint64_t failed_tick_count = 0U;
    std::uint64_t platform_event_count = 0U;
    std::uint64_t translated_input_event_count = 0U;
    std::uint64_t drained_input_event_count = 0U;
    std::uint64_t render_frame_count = 0U;
    std::uint64_t audio_submit_count = 0U;
    std::uint64_t audio_completion_count = 0U;
    std::size_t platform_event_capacity = 0U;
    std::size_t required_rhi_device_storage_bytes = 0U;
    std::size_t required_rhi_device_storage_alignment = 0U;
    HardwareFrameHostStatus last_status = HardwareFrameHostStatus::NotInitialized;
    platform::PlatformWindowStatus last_window_status = platform::PlatformWindowStatus::NotCreated;
    input::InputStatus last_input_status = input::InputStatus::NotInitialized;
    rendercore::RenderSwapchainFramePipelineStatus last_render_status =
        rendercore::RenderSwapchainFramePipelineStatus::InvalidArgument;
    rhi::RhiStatus last_rhi_status = rhi::RhiStatus::InvalidLifecycle;
    audio::AudioStatus last_audio_status = audio::AudioStatus::NotInitialized;
    bool initialized = false;
    bool render_enabled = false;
    bool audio_enabled = false;
    bool audio_available = false;
    bool rhi_device_created = false;
};
}
