// 模块: YuEngine Hardware
// 文件: Src/YuEngine/Hardware/Include/YuEngine/Hardware/HardwareFrameHostTickResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioStatus.h"
#include "YuEngine/Hardware/HardwareFrameHostStatus.h"
#include "YuEngine/Input/InputStatus.h"
#include "YuEngine/Platform/PlatformWindowPollResult.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineResult.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::hardware {
struct HardwareFrameHostTickResult final {
    HardwareFrameHostStatus status = HardwareFrameHostStatus::NotInitialized;
    platform::PlatformWindowPollResult poll_result{};
    input::InputStatus input_status = input::InputStatus::NotInitialized;
    rendercore::RenderSwapchainFramePipelineResult render_result{};
    rhi::RhiStatus rhi_status = rhi::RhiStatus::InvalidLifecycle;
    audio::AudioStatus audio_status = audio::AudioStatus::NotInitialized;
    input::InputStatus gamepad_poll_status = input::InputStatus::NotInitialized;
    std::size_t platform_event_count = 0U;
    std::size_t translated_input_event_count = 0U;
    std::size_t drained_input_event_count = 0U;
    std::size_t audio_completion_count = 0U;
    std::uint32_t frame_id = 0U;
    bool audio_submitted = false;
    bool gamepad_polled = false;
};
}
