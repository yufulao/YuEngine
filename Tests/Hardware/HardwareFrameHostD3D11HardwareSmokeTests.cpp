// 模块：Tests Hardware
// 文件：Tests/Hardware/HardwareFrameHostD3D11HardwareSmokeTests.cpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <vector>

#include "YuEngine/Audio/AudioCallbackDeviceDesc.h"
#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/Hardware/HardwareFrameHost.h"
#include "YuEngine/Hardware/HardwareFrameHostDesc.h"
#include "YuEngine/Hardware/HardwareFrameHostSnapshot.h"
#include "YuEngine/Hardware/HardwareFrameHostStatus.h"
#include "YuEngine/Hardware/HardwareFrameHostTickRequest.h"
#include "YuEngine/Hardware/HardwareFrameHostTickResult.h"
#include "YuEngine/Input/InputBridgeEvent.h"
#include "YuEngine/Platform/PlatformWindowEvent.h"
#include "YuEngine/Platform/PlatformWindowEventType.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineStatus.h"
#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiStatus.h"

using yuengine::audio::AudioCallbackDeviceDesc;
using yuengine::audio::CHANNEL_COUNT;
using yuengine::hardware::HardwareFrameHost;
using yuengine::hardware::HardwareFrameHostDesc;
using yuengine::hardware::HardwareFrameHostSnapshot;
using yuengine::hardware::HardwareFrameHostStatus;
using yuengine::hardware::HardwareFrameHostTickRequest;
using yuengine::hardware::HardwareFrameHostTickResult;
using yuengine::input::InputBridgeEvent;
using yuengine::platform::PlatformWindowEvent;
using yuengine::platform::PlatformWindowEventType;
using yuengine::rendercore::RenderSwapchainFramePipelineStatus;
using yuengine::rhi::RhiBackendKind;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::MAX_COMMANDS;
using yuengine::rhi::RGBA8_BYTES_PER_PIXEL;

namespace {
constexpr const char *TEST_D3D11_HOST_FRAME = "HardwareFrameHost_D3D11IntegratedFrameRuns";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr int SKIP_RETURN_CODE = 77;
constexpr std::uint16_t SMOKE_EXTENT = 4U;
constexpr std::uint32_t SMOKE_FRAME_ID = 1U;
constexpr std::size_t SMOKE_AUDIO_FRAMES = AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER;
constexpr std::size_t SMOKE_AUDIO_SAMPLE_COUNT = SMOKE_AUDIO_FRAMES * CHANNEL_COUNT;
constexpr std::uint32_t AUDIO_WAIT_TIMEOUT_MS = 2000U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int Skip(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stdout);
    std::fputc('\n', stdout);
    return SKIP_RETURN_CODE;
}

std::size_t CaptureByteCount() {
    return static_cast<std::size_t>(SMOKE_EXTENT) * static_cast<std::size_t>(SMOKE_EXTENT) *
        RGBA8_BYTES_PER_PIXEL;
}

HardwareFrameHostDesc D3D11HostDesc() {
    HardwareFrameHostDesc desc{};
    desc.window_desc.title = "YuEngine Hardware Frame Host D3D11 Smoke";
    desc.window_desc.client_width = SMOKE_EXTENT;
    desc.window_desc.client_height = SMOKE_EXTENT;
    desc.window_desc.visible = false;
    desc.rhi_desc.backend_kind = RhiBackendKind::D3D11;
    desc.rhi_desc.command_list_capacity = MAX_COMMANDS;
    desc.rhi_desc.swapchain.extent = {SMOKE_EXTENT, SMOKE_EXTENT};
    desc.audio_desc.frames_per_buffer = SMOKE_AUDIO_FRAMES;
    desc.render_enabled = true;
    desc.audio_enabled = true;
    desc.require_audio_device = false;
    return desc;
}

PlatformWindowEvent FocusGainedEvent() {
    PlatformWindowEvent event{};
    event.type = PlatformWindowEventType::FocusGained;
    return event;
}

PlatformWindowEvent KeyDownEvent() {
    PlatformWindowEvent event{};
    event.type = PlatformWindowEventType::RawKeyDown;
    event.raw_code = 65U;
    return event;
}

int HardwareFrameHostD3D11IntegratedFrameRuns() {
    HardwareFrameHost host;
    const HardwareFrameHostDesc desc = D3D11HostDesc();
    const HardwareFrameHostStatus init_status = host.Initialize(desc);
    if (init_status == HardwareFrameHostStatus::WindowCreateFailed) {
        return Skip("hardware frame host smoke skipped because a native window could not be created");
    }

    const HardwareFrameHostSnapshot init_snapshot = host.Snapshot();
    if (init_status == HardwareFrameHostStatus::RhiCreateFailed) {
        if (init_snapshot.last_rhi_status == RhiStatus::UnsupportedBackend) {
            return Skip("hardware frame host smoke skipped because D3D11 is not compiled");
        }

        if (init_snapshot.last_rhi_status == RhiStatus::MissingHardware) {
            return Skip("hardware frame host smoke skipped because D3D11 hardware is unavailable");
        }
    }

    if (init_status != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host initialize failed");
    }

    std::array<PlatformWindowEvent, 2U> platform_events{};
    platform_events[0U] = FocusGainedEvent();
    platform_events[1U] = KeyDownEvent();

    std::array<InputBridgeEvent, 1U> input_events{};
    std::size_t input_event_count = 0U;
    std::vector<std::uint8_t> capture(CaptureByteCount());
    std::array<std::int16_t, SMOKE_AUDIO_SAMPLE_COUNT> audio_samples{};

    HardwareFrameHostTickRequest request{};
    request.injected_platform_events = std::span<const PlatformWindowEvent>(
        platform_events.data(),
        platform_events.size());
    request.input_events = std::span<InputBridgeEvent>(input_events.data(), input_events.size());
    request.out_input_event_count = &input_event_count;
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget = capture.size();
    request.audio_samples = std::span<const std::int16_t>(audio_samples.data(), audio_samples.size());
    request.audio_frame_count = SMOKE_AUDIO_FRAMES;
    request.audio_completion_target = 1U;
    request.audio_wait_timeout_milliseconds = AUDIO_WAIT_TIMEOUT_MS;
    request.clear_color = RhiColor{16U, 48U, 96U, 255U};
    request.frame_id = SMOKE_FRAME_ID;

    const HardwareFrameHostTickResult result = host.Tick(request);
    if (result.status != HardwareFrameHostStatus::Success) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host tick failed");
    }

    if (result.render_result.status != RenderSwapchainFramePipelineStatus::Success) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host render pipeline failed");
    }

    if (result.render_result.capture_bytes_written != capture.size()) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host capture byte count mismatch");
    }

    if (input_event_count != input_events.size()) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host input event count mismatch");
    }

    const HardwareFrameHostSnapshot tick_snapshot = host.Snapshot();
    if (tick_snapshot.render_frame_count != 1U) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host did not record render frame");
    }

    if (tick_snapshot.audio_available && result.audio_completion_count != 1U) {
        static_cast<void>(host.Shutdown());
        return Fail("hardware frame host audio completion count mismatch");
    }

    const HardwareFrameHostStatus shutdown_status = host.Shutdown();
    if (shutdown_status != HardwareFrameHostStatus::Success) {
        return Fail("hardware frame host shutdown failed");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::string_view test_name(argv[1]);
    if (test_name == TEST_D3D11_HOST_FRAME) {
        return HardwareFrameHostD3D11IntegratedFrameRuns();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
