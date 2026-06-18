// 模块: YuEngine Hardware
// 文件: Src/YuEngine/Hardware/Src/HardwareFrameHost.cpp

#include "YuEngine/Hardware/HardwareFrameHost.h"

#include <array>
#include <cstdint>
#include <span>

#include "YuEngine/Audio/AudioCallbackSnapshot.h"
#include "YuEngine/Input/InputControlId.h"
#include "YuEngine/Input/InputDeviceKind.h"
#include "YuEngine/Platform/PlatformNativeSurface.h"
#include "YuEngine/Platform/PlatformWindowSnapshot.h"
#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceCreateResult.h"
#include "YuEngine/Rhi/RhiDeviceFactory.h"
#include "YuEngine/Rhi/RhiNativeSurfaceDesc.h"
#include "YuEngine/Rhi/RhiSwapchainResizeRequest.h"

namespace yuengine::hardware {
namespace {
constexpr std::uint32_t MOUSE_MOVE_CONTROL = 0U;
constexpr std::uint32_t MOUSE_WHEEL_CONTROL = 1U;

rhi::RhiNativeSurfaceDesc ToRhiSurface(const platform::PlatformNativeSurface &surface) {
    return rhi::RhiNativeSurfaceDesc{surface.window_value, surface.instance_value, surface.valid};
}

bool IsOptionalAudioUnavailable(audio::AudioStatus status) {
    if (status == audio::AudioStatus::DeviceUnavailable) {
        return true;
    }

    return status == audio::AudioStatus::UnsupportedBackend;
}

bool IsRenderCaptureValid(const HardwareFrameHostTickRequest &request) {
    if (request.capture_byte_budget == 0U) {
        return false;
    }

    if (request.capture_output.data() == nullptr) {
        return false;
    }

    return request.capture_output.size() >= request.capture_byte_budget;
}

bool IsResizeExtentValid(const rhi::RhiExtent2D &extent) {
    if (extent.width == 0U) {
        return false;
    }

    return extent.height != 0U;
}

input::InputBridgeEvent MakeKeyEvent(
    const input::InputBridgeDesc &desc,
    const platform::PlatformWindowEvent &event,
    input::InputBridgeEventType type) {
    input::InputBridgeEvent input_event{};
    input_event.device_kind = input::InputDeviceKind::Keyboard;
    input_event.device = desc.keyboard_device;
    input_event.control = input::InputControlId{event.raw_code};
    input_event.type = type;
    input_event.raw_code = event.raw_code;
    return input_event;
}

input::InputBridgeEvent MakeMouseMoveEvent(
    const input::InputBridgeDesc &desc,
    const platform::PlatformWindowEvent &event) {
    input::InputBridgeEvent input_event{};
    input_event.device_kind = input::InputDeviceKind::Mouse;
    input_event.device = desc.mouse_device;
    input_event.control = input::InputControlId{MOUSE_MOVE_CONTROL};
    input_event.type = input::InputBridgeEventType::MouseMoved;
    input_event.pointer_x = event.pointer_x;
    input_event.pointer_y = event.pointer_y;
    return input_event;
}

input::InputBridgeEvent MakeMouseWheelEvent(
    const input::InputBridgeDesc &desc,
    const platform::PlatformWindowEvent &event) {
    input::InputBridgeEvent input_event{};
    input_event.device_kind = input::InputDeviceKind::Mouse;
    input_event.device = desc.mouse_device;
    input_event.control = input::InputControlId{MOUSE_WHEEL_CONTROL};
    input_event.type = input::InputBridgeEventType::MouseWheel;
    input_event.pointer_x = event.pointer_x;
    input_event.pointer_y = event.pointer_y;
    input_event.wheel_delta = event.wheel_delta;
    input_event.axis_value = event.wheel_delta;
    return input_event;
}

input::InputBridgeEvent MakeMouseButtonEvent(
    const input::InputBridgeDesc &desc,
    const platform::PlatformWindowEvent &event,
    input::InputBridgeEventType type) {
    input::InputBridgeEvent input_event{};
    input_event.device_kind = input::InputDeviceKind::Mouse;
    input_event.device = desc.mouse_device;
    input_event.control = input::InputControlId{event.raw_code};
    input_event.type = type;
    input_event.raw_code = event.raw_code;
    input_event.pointer_x = event.pointer_x;
    input_event.pointer_y = event.pointer_y;
    return input_event;
}
}

HardwareFrameHost::HardwareFrameHost()
    : desc_(),
      window_(),
      input_bridge_(),
      audio_device_(),
      render_pipeline_(),
      platform_events_(),
      rhi_device_storage_(),
      rhi_device_(nullptr),
      snapshot_(),
      initialized_(false),
      audio_initialized_(false),
      audio_available_(false) {
}

HardwareFrameHost::~HardwareFrameHost() {
    static_cast<void>(Shutdown());
}

HardwareFrameHostStatus HardwareFrameHost::Initialize(const HardwareFrameHostDesc &desc) {
    if (initialized_) {
        snapshot_.last_status = HardwareFrameHostStatus::AlreadyInitialized;
        return HardwareFrameHostStatus::AlreadyInitialized;
    }

    const HardwareFrameHostStatus desc_status = ValidateDesc(desc);
    if (desc_status != HardwareFrameHostStatus::Success) {
        snapshot_.last_status = desc_status;
        return desc_status;
    }

    desc_ = desc;
    snapshot_ = HardwareFrameHostSnapshot{};
    snapshot_.platform_event_capacity = desc_.platform_event_capacity;
    snapshot_.render_enabled = desc_.render_enabled;
    snapshot_.audio_enabled = desc_.audio_enabled;

    const platform::PlatformWindowStatus window_status = window_.Create(desc_.window_desc);
    snapshot_.last_window_status = window_status;
    if (window_status != platform::PlatformWindowStatus::Success) {
        snapshot_.last_status = HardwareFrameHostStatus::WindowCreateFailed;
        return HardwareFrameHostStatus::WindowCreateFailed;
    }

    const input::InputStatus input_status = input_bridge_.Initialize(desc_.input_desc);
    snapshot_.last_input_status = input_status;
    if (input_status != input::InputStatus::Success) {
        static_cast<void>(window_.Destroy());
        snapshot_.last_status = HardwareFrameHostStatus::InputInitializeFailed;
        return HardwareFrameHostStatus::InputInitializeFailed;
    }

    if (desc_.render_enabled) {
        const HardwareFrameHostStatus rhi_status = CreateRhiDevice();
        if (rhi_status != HardwareFrameHostStatus::Success) {
            static_cast<void>(input_bridge_.Shutdown());
            static_cast<void>(window_.Destroy());
            snapshot_.last_status = rhi_status;
            return rhi_status;
        }
    }

    if (desc_.audio_enabled) {
        const HardwareFrameHostStatus audio_status = InitializeAudio();
        if (audio_status != HardwareFrameHostStatus::Success) {
            DestroyRhiDevice();
            static_cast<void>(input_bridge_.Shutdown());
            static_cast<void>(window_.Destroy());
            snapshot_.last_status = audio_status;
            return audio_status;
        }
    }

    initialized_ = true;
    snapshot_.initialized = true;
    snapshot_.rhi_device_created = rhi_device_ != nullptr;
    snapshot_.audio_available = audio_available_;
    snapshot_.last_status = HardwareFrameHostStatus::Success;
    return HardwareFrameHostStatus::Success;
}

HardwareFrameHostTickResult HardwareFrameHost::Tick(const HardwareFrameHostTickRequest &request) {
    HardwareFrameHostTickResult result = MakeBaseTickResult(request.frame_id);

    if (!initialized_) {
        result.status = HardwareFrameHostStatus::NotInitialized;
        return RecordTickFailed(result);
    }

    if (request.frame_id == 0U) {
        result.status = HardwareFrameHostStatus::InvalidArgument;
        return RecordTickFailed(result);
    }

    if (request.out_input_event_count == nullptr) {
        result.status = HardwareFrameHostStatus::InvalidArgument;
        return RecordTickFailed(result);
    }

    if (desc_.render_enabled && !IsRenderCaptureValid(request)) {
        result.status = HardwareFrameHostStatus::InvalidArgument;
        return RecordTickFailed(result);
    }

    *request.out_input_event_count = 0U;
    ++snapshot_.tick_count;

    ResizeState resize_state{};
    result = ProcessPlatformEvents(request, &resize_state);
    if (result.status != HardwareFrameHostStatus::Success) {
        return RecordTickFailed(result);
    }

    result = PollGamepadIfRequested(request, result);
    if (result.status != HardwareFrameHostStatus::Success) {
        return RecordTickFailed(result);
    }

    result = DrainInputEvents(request, result);
    if (result.status != HardwareFrameHostStatus::Success) {
        return RecordTickFailed(result);
    }

    result = SubmitAudio(request, result);
    if (result.status != HardwareFrameHostStatus::Success) {
        return RecordTickFailed(result);
    }

    result = ExecuteRenderFrame(request, resize_state, result);
    if (result.status != HardwareFrameHostStatus::Success) {
        return RecordTickFailed(result);
    }

    return RecordTickCompleted(result);
}

HardwareFrameHostStatus HardwareFrameHost::Shutdown() {
    if (!initialized_) {
        snapshot_.last_status = HardwareFrameHostStatus::NotInitialized;
        return HardwareFrameHostStatus::NotInitialized;
    }

    if (audio_initialized_) {
        if (audio_available_) {
            const audio::AudioStatus stop_status = audio_device_.Stop();
            snapshot_.last_audio_status = stop_status;
            if (stop_status != audio::AudioStatus::Success) {
                snapshot_.last_status = HardwareFrameHostStatus::ShutdownFailed;
                return HardwareFrameHostStatus::ShutdownFailed;
            }
        }

        const audio::AudioStatus shutdown_status = audio_device_.Shutdown();
        snapshot_.last_audio_status = shutdown_status;
        if (shutdown_status != audio::AudioStatus::ShutdownComplete) {
            snapshot_.last_status = HardwareFrameHostStatus::ShutdownFailed;
            return HardwareFrameHostStatus::ShutdownFailed;
        }
    }

    DestroyRhiDevice();

    const input::InputStatus input_status = input_bridge_.Shutdown();
    snapshot_.last_input_status = input_status;
    if (input_status != input::InputStatus::Success) {
        snapshot_.last_status = HardwareFrameHostStatus::ShutdownFailed;
        return HardwareFrameHostStatus::ShutdownFailed;
    }

    const platform::PlatformWindowStatus window_status = window_.Destroy();
    snapshot_.last_window_status = window_status;
    if (window_status != platform::PlatformWindowStatus::Success) {
        snapshot_.last_status = HardwareFrameHostStatus::ShutdownFailed;
        return HardwareFrameHostStatus::ShutdownFailed;
    }

    initialized_ = false;
    audio_initialized_ = false;
    audio_available_ = false;
    snapshot_.initialized = false;
    snapshot_.rhi_device_created = false;
    snapshot_.audio_available = false;
    snapshot_.last_status = HardwareFrameHostStatus::Success;
    return HardwareFrameHostStatus::Success;
}

HardwareFrameHostSnapshot HardwareFrameHost::Snapshot() const {
    return snapshot_;
}

HardwareFrameHostStatus HardwareFrameHost::ValidateDesc(const HardwareFrameHostDesc &desc) const {
    if (desc.platform_event_capacity < HardwareFrameHostDesc::MIN_PLATFORM_EVENT_CAPACITY) {
        return HardwareFrameHostStatus::InvalidArgument;
    }

    if (desc.platform_event_capacity > HardwareFrameHostDesc::MAX_PLATFORM_EVENT_CAPACITY) {
        return HardwareFrameHostStatus::InvalidArgument;
    }

    return HardwareFrameHostStatus::Success;
}

HardwareFrameHostTickResult HardwareFrameHost::MakeBaseTickResult(std::uint32_t frame_id) const {
    HardwareFrameHostTickResult result{};
    result.frame_id = frame_id;
    result.poll_result.status = snapshot_.last_window_status;
    result.input_status = snapshot_.last_input_status;
    result.render_result.status = snapshot_.last_render_status;
    result.rhi_status = snapshot_.last_rhi_status;
    result.audio_status = snapshot_.last_audio_status;
    return result;
}

HardwareFrameHostStatus HardwareFrameHost::CreateRhiDevice() {
    const std::size_t required_size = rhi::RhiDeviceFactory::RequiredDeviceStorageSize(desc_.rhi_desc.backend_kind);
    const std::size_t required_alignment = rhi::RhiDeviceFactory::RequiredDeviceStorageAlignment(
        desc_.rhi_desc.backend_kind);
    if (required_size == 0U) {
        snapshot_.last_rhi_status = rhi::RhiStatus::UnsupportedBackend;
        return HardwareFrameHostStatus::RhiCreateFailed;
    }

    if (required_size > rhi_device_storage_.size()) {
        snapshot_.last_rhi_status = rhi::RhiStatus::CapacityExceeded;
        return HardwareFrameHostStatus::RhiStorageTooSmall;
    }

    if (required_alignment == 0U) {
        snapshot_.last_rhi_status = rhi::RhiStatus::InvalidDescriptor;
        return HardwareFrameHostStatus::RhiStorageTooSmall;
    }

    if (required_alignment > MAX_RHI_DEVICE_STORAGE_ALIGNMENT) {
        snapshot_.last_rhi_status = rhi::RhiStatus::InvalidDescriptor;
        return HardwareFrameHostStatus::RhiStorageTooSmall;
    }

    const auto base_address = reinterpret_cast<std::uintptr_t>(rhi_device_storage_.data());
    std::uintptr_t aligned_address = base_address;
    const std::uintptr_t remainder = aligned_address % required_alignment;
    if (remainder != 0U) {
        aligned_address += required_alignment - remainder;
    }

    const std::size_t storage_offset = static_cast<std::size_t>(aligned_address - base_address);
    if (storage_offset + required_size > rhi_device_storage_.size()) {
        snapshot_.last_rhi_status = rhi::RhiStatus::InvalidDescriptor;
        return HardwareFrameHostStatus::RhiStorageTooSmall;
    }

    rhi::RhiDeviceDesc device_desc = desc_.rhi_desc;
    if (device_desc.backend_kind == rhi::RhiBackendKind::D3D11) {
        device_desc.native_surface = ToRhiSurface(window_.GetNativeSurface());
        device_desc.requires_native_surface = true;
        device_desc.requires_swapchain = true;
    }

    std::byte *storage_bytes = rhi_device_storage_.data() + storage_offset;
    std::span<std::byte> storage_span(storage_bytes, required_size);
    const rhi::RhiDeviceCreateResult create_result = rhi::RhiDeviceFactory::CreateDevice(device_desc, storage_span);
    snapshot_.last_rhi_status = create_result.status;
    if (create_result.status != rhi::RhiStatus::Success) {
        rhi_device_ = nullptr;
        return HardwareFrameHostStatus::RhiCreateFailed;
    }

    rhi_device_ = create_result.device;
    snapshot_.rhi_device_created = true;
    return HardwareFrameHostStatus::Success;
}

HardwareFrameHostStatus HardwareFrameHost::InitializeAudio() {
    const audio::AudioStatus initialize_status = audio_device_.Initialize(desc_.audio_desc);
    snapshot_.last_audio_status = initialize_status;
    audio_initialized_ = true;
    if (initialize_status != audio::AudioStatus::Success) {
        if (!desc_.require_audio_device && IsOptionalAudioUnavailable(initialize_status)) {
            audio_available_ = false;
            snapshot_.audio_available = false;
            return HardwareFrameHostStatus::Success;
        }

        audio_available_ = false;
        snapshot_.audio_available = false;
        return HardwareFrameHostStatus::AudioInitializeFailed;
    }

    const audio::AudioStatus start_status = audio_device_.Start();
    snapshot_.last_audio_status = start_status;
    if (start_status != audio::AudioStatus::Success) {
        if (!desc_.require_audio_device) {
            audio_available_ = false;
            snapshot_.audio_available = false;
            return HardwareFrameHostStatus::Success;
        }

        audio_available_ = false;
        snapshot_.audio_available = false;
        return HardwareFrameHostStatus::AudioStartFailed;
    }

    audio_available_ = true;
    snapshot_.audio_available = true;
    return HardwareFrameHostStatus::Success;
}

HardwareFrameHostTickResult HardwareFrameHost::ProcessPlatformEvents(
    const HardwareFrameHostTickRequest &request,
    ResizeState *resize_state) {
    HardwareFrameHostTickResult result = MakeBaseTickResult(request.frame_id);
    result.status = HardwareFrameHostStatus::Success;

    if (resize_state == nullptr) {
        result.status = HardwareFrameHostStatus::InvalidArgument;
        return result;
    }

    platform::PlatformWindowPollResult poll_result = window_.PollEvents(
        platform_events_.data(),
        desc_.platform_event_capacity);
    result.poll_result = poll_result;
    result.platform_event_count = poll_result.event_count;
    snapshot_.last_window_status = poll_result.status;

    for (std::size_t index = 0U; index < poll_result.event_count; ++index) {
        result = ProcessPlatformEvent(platform_events_[index], resize_state, result);
        if (result.status != HardwareFrameHostStatus::Success) {
            return result;
        }
    }

    for (const platform::PlatformWindowEvent &event : request.injected_platform_events) {
        ++result.platform_event_count;
        result = ProcessPlatformEvent(event, resize_state, result);
        if (result.status != HardwareFrameHostStatus::Success) {
            return result;
        }
    }

    if (poll_result.status != platform::PlatformWindowStatus::Success) {
        result.status = HardwareFrameHostStatus::WindowPollFailed;
        return result;
    }

    return result;
}

HardwareFrameHostTickResult HardwareFrameHost::ProcessPlatformEvent(
    const platform::PlatformWindowEvent &event,
    ResizeState *resize_state,
    HardwareFrameHostTickResult result) {
    if (event.type == platform::PlatformWindowEventType::Resized) {
        resize_state->extent.width = static_cast<std::uint16_t>(event.client_width);
        resize_state->extent.height = static_cast<std::uint16_t>(event.client_height);
        resize_state->requested = IsResizeExtentValid(resize_state->extent);
        return result;
    }

    if (event.type == platform::PlatformWindowEventType::FocusGained) {
        result.input_status = input_bridge_.SetFocus(true);
        snapshot_.last_input_status = result.input_status;
        if (result.input_status != input::InputStatus::Success) {
            result.status = HardwareFrameHostStatus::InputSubmitFailed;
        }

        return result;
    }

    if (event.type == platform::PlatformWindowEventType::FocusLost) {
        result.input_status = input_bridge_.SetFocus(false);
        snapshot_.last_input_status = result.input_status;
        if (result.input_status != input::InputStatus::Success) {
            result.status = HardwareFrameHostStatus::InputSubmitFailed;
        }

        return result;
    }

    input::InputBridgeEvent input_event{};
    bool has_input_event = false;
    if (event.type == platform::PlatformWindowEventType::RawKeyDown) {
        input_event = MakeKeyEvent(desc_.input_desc, event, input::InputBridgeEventType::KeyPressed);
        has_input_event = true;
    }

    if (event.type == platform::PlatformWindowEventType::RawKeyUp) {
        input_event = MakeKeyEvent(desc_.input_desc, event, input::InputBridgeEventType::KeyReleased);
        has_input_event = true;
    }

    if (event.type == platform::PlatformWindowEventType::RawMouseMove) {
        input_event = MakeMouseMoveEvent(desc_.input_desc, event);
        has_input_event = true;
        if (event.wheel_delta != 0) {
            input_event = MakeMouseWheelEvent(desc_.input_desc, event);
        }
    }

    if (event.type == platform::PlatformWindowEventType::RawMouseButtonDown) {
        input_event = MakeMouseButtonEvent(desc_.input_desc, event, input::InputBridgeEventType::MouseButtonPressed);
        has_input_event = true;
    }

    if (event.type == platform::PlatformWindowEventType::RawMouseButtonUp) {
        input_event = MakeMouseButtonEvent(desc_.input_desc, event, input::InputBridgeEventType::MouseButtonReleased);
        has_input_event = true;
    }

    if (!has_input_event) {
        return result;
    }

    result.input_status = input_bridge_.SubmitEvent(input_event);
    snapshot_.last_input_status = result.input_status;
    if (result.input_status != input::InputStatus::Success) {
        result.status = HardwareFrameHostStatus::InputSubmitFailed;
        return result;
    }

    ++result.translated_input_event_count;
    return result;
}

HardwareFrameHostTickResult HardwareFrameHost::PollGamepadIfRequested(
    const HardwareFrameHostTickRequest &request,
    HardwareFrameHostTickResult result) {
    if (!request.poll_gamepad) {
        return result;
    }

    result.gamepad_polled = true;
    result.gamepad_poll_status = input_bridge_.PollGamepad(request.gamepad_user_index);
    result.input_status = result.gamepad_poll_status;
    snapshot_.last_input_status = result.input_status;
    if (result.gamepad_poll_status == input::InputStatus::Success) {
        return result;
    }

    if (result.gamepad_poll_status == input::InputStatus::SourceUnavailable) {
        return result;
    }

    result.status = HardwareFrameHostStatus::InputPollFailed;
    return result;
}

HardwareFrameHostTickResult HardwareFrameHost::DrainInputEvents(
    const HardwareFrameHostTickRequest &request,
    HardwareFrameHostTickResult result) {
    std::size_t input_event_count = 0U;
    result.input_status = input_bridge_.DrainEvents(
        request.input_events.data(),
        request.input_events.size(),
        input_event_count);
    snapshot_.last_input_status = result.input_status;
    if (result.input_status != input::InputStatus::Success) {
        result.status = HardwareFrameHostStatus::InputDrainFailed;
        return result;
    }

    *request.out_input_event_count = input_event_count;
    result.drained_input_event_count = input_event_count;
    return result;
}

HardwareFrameHostTickResult HardwareFrameHost::SubmitAudio(
    const HardwareFrameHostTickRequest &request,
    HardwareFrameHostTickResult result) {
    if (!desc_.audio_enabled || !audio_available_) {
        result.audio_status = snapshot_.last_audio_status;
        return result;
    }

    if (request.audio_samples.empty()) {
        result.audio_status = audio::AudioStatus::Success;
        return result;
    }

    result.audio_status = audio_device_.SubmitS16Buffer(request.audio_samples, request.audio_frame_count);
    snapshot_.last_audio_status = result.audio_status;
    if (result.audio_status != audio::AudioStatus::Success) {
        result.status = HardwareFrameHostStatus::AudioSubmitFailed;
        return result;
    }

    result.audio_submitted = true;
    ++snapshot_.audio_submit_count;
    if (request.audio_completion_target > 0U) {
        result.audio_status = audio_device_.WaitForCompletedCallbacks(
            request.audio_completion_target,
            request.audio_wait_timeout_milliseconds);
        snapshot_.last_audio_status = result.audio_status;
        if (result.audio_status != audio::AudioStatus::Success) {
            result.status = HardwareFrameHostStatus::AudioWaitFailed;
            return result;
        }
    }

    std::array<audio::AudioCallbackCompletion, audio::AudioCallbackDeviceDesc::MAX_BUFFER_COUNT> completions{};
    std::size_t completion_count = 0U;
    result.audio_status = audio_device_.DrainCompletions(completions.data(), completions.size(), completion_count);
    snapshot_.last_audio_status = result.audio_status;
    if (result.audio_status != audio::AudioStatus::Success) {
        result.status = HardwareFrameHostStatus::AudioDrainFailed;
        return result;
    }

    result.audio_completion_count = completion_count;
    snapshot_.audio_completion_count += completion_count;
    return result;
}

HardwareFrameHostTickResult HardwareFrameHost::ExecuteRenderFrame(
    const HardwareFrameHostTickRequest &request,
    const ResizeState &resize_state,
    HardwareFrameHostTickResult result) {
    if (!desc_.render_enabled) {
        return result;
    }

    rendercore::RenderSwapchainFramePipelineRequest render_request{};
    render_request.rhi_device = rhi_device_;
    render_request.clear_color = request.clear_color;
    render_request.capture_output = request.capture_output;
    render_request.capture_byte_budget = request.capture_byte_budget;
    render_request.frame_id = request.frame_id;
    render_request.resize_before_submit = resize_state.requested;
    render_request.resize_request.extent = resize_state.extent;

    result.render_result = render_pipeline_.Execute(render_request);
    result.rhi_status = result.render_result.rhi_status;
    snapshot_.last_render_status = result.render_result.status;
    snapshot_.last_rhi_status = result.render_result.rhi_status;
    if (result.render_result.status != rendercore::RenderSwapchainFramePipelineStatus::Success) {
        result.status = HardwareFrameHostStatus::RenderFrameFailed;
        return result;
    }

    ++snapshot_.render_frame_count;
    return result;
}

HardwareFrameHostTickResult HardwareFrameHost::RecordTickFailed(HardwareFrameHostTickResult result) {
    ++snapshot_.failed_tick_count;
    StoreLastTickResult(result);
    return result;
}

HardwareFrameHostTickResult HardwareFrameHost::RecordTickCompleted(HardwareFrameHostTickResult result) {
    result.status = HardwareFrameHostStatus::Success;
    ++snapshot_.completed_tick_count;
    StoreLastTickResult(result);
    return result;
}

void HardwareFrameHost::StoreLastTickResult(const HardwareFrameHostTickResult &result) {
    snapshot_.platform_event_count += result.platform_event_count;
    snapshot_.translated_input_event_count += result.translated_input_event_count;
    snapshot_.drained_input_event_count += result.drained_input_event_count;
    snapshot_.last_status = result.status;
    snapshot_.last_window_status = result.poll_result.status;
    snapshot_.last_input_status = result.input_status;
    snapshot_.last_render_status = result.render_result.status;
    snapshot_.last_rhi_status = result.rhi_status;
    snapshot_.last_audio_status = result.audio_status;
}

void HardwareFrameHost::DestroyRhiDevice() {
    if (rhi_device_ == nullptr) {
        snapshot_.rhi_device_created = false;
        return;
    }

    const rhi::RhiStatus destroy_status = rhi::RhiDeviceFactory::DestroyDevice(rhi_device_);
    snapshot_.last_rhi_status = destroy_status;
    rhi_device_ = nullptr;
    snapshot_.rhi_device_created = false;
}
}
