// Module: YuEngine Hardware
// File: Src/YuEngine/Hardware/Include/YuEngine/Hardware/HardwareFrameHost.h

#pragma once

#include <array>
#include <cstddef>

#include "YuEngine/Audio/AudioCallbackDevice.h"
#include "YuEngine/Hardware/HardwareFrameHostDesc.h"
#include "YuEngine/Hardware/HardwareFrameHostSnapshot.h"
#include "YuEngine/Hardware/HardwareFrameHostStatus.h"
#include "YuEngine/Hardware/HardwareFrameHostTickRequest.h"
#include "YuEngine/Hardware/HardwareFrameHostTickResult.h"
#include "YuEngine/Input/InputBridge.h"
#include "YuEngine/Platform/PlatformWindowEvent.h"
#include "YuEngine/Platform/WindowsPlatformWindow.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipeline.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiExtent2D.h"

namespace yuengine::hardware {
class HardwareFrameHost final {
public:
    /**
     * @comment Constructs an empty hardware frame host.
     */
    HardwareFrameHost();
    /**
     * @comment Shuts down any owned hardware objects.
     */
    ~HardwareFrameHost();

    HardwareFrameHost(const HardwareFrameHost &) = delete;
    HardwareFrameHost &operator=(const HardwareFrameHost &) = delete;

    /**
     * @comment Creates window, input, optional RHI, and optional audio owners from a bounded descriptor.
     * @param desc Input host descriptor.
     * @return Explicit operation status.
     */
    HardwareFrameHostStatus Initialize(const HardwareFrameHostDesc &desc);
    /**
     * @comment Runs one integrated platform input, render, and optional audio host tick.
     * @param request Input tick request and caller-owned output buffers.
     * @return Explicit tick result.
     */
    HardwareFrameHostTickResult Tick(const HardwareFrameHostTickRequest &request);
    /**
     * @comment Stops and destroys all owned hardware objects.
     * @return Explicit operation status.
     */
    HardwareFrameHostStatus Shutdown();
    /**
     * @comment Returns current host counters and lifecycle flags.
     * @return Snapshot value.
     */
    HardwareFrameHostSnapshot Snapshot() const;

private:
    static constexpr std::size_t MAX_RHI_DEVICE_STORAGE_BYTES = 262144U;
    static constexpr std::size_t MAX_RHI_DEVICE_STORAGE_ALIGNMENT = 64U;

    struct ResizeState final {
        rhi::RhiExtent2D extent{};
        bool requested = false;
    };

    HardwareFrameHostStatus ValidateDesc(const HardwareFrameHostDesc &desc) const;
    HardwareFrameHostTickResult MakeBaseTickResult(std::uint32_t frame_id) const;
    HardwareFrameHostStatus CreateRhiDevice();
    HardwareFrameHostStatus InitializeAudio();
    HardwareFrameHostTickResult ProcessPlatformEvents(
        const HardwareFrameHostTickRequest &request,
        ResizeState *resize_state);
    HardwareFrameHostTickResult ProcessPlatformEvent(
        const platform::PlatformWindowEvent &event,
        ResizeState *resize_state,
        HardwareFrameHostTickResult result);
    HardwareFrameHostTickResult PollGamepadIfRequested(
        const HardwareFrameHostTickRequest &request,
        HardwareFrameHostTickResult result);
    HardwareFrameHostTickResult DrainInputEvents(
        const HardwareFrameHostTickRequest &request,
        HardwareFrameHostTickResult result);
    HardwareFrameHostTickResult SubmitAudio(
        const HardwareFrameHostTickRequest &request,
        HardwareFrameHostTickResult result);
    HardwareFrameHostTickResult ExecuteRenderFrame(
        const HardwareFrameHostTickRequest &request,
        const ResizeState &resize_state,
        HardwareFrameHostTickResult result);
    HardwareFrameHostTickResult RecordTickFailed(HardwareFrameHostTickResult result);
    HardwareFrameHostTickResult RecordTickCompleted(HardwareFrameHostTickResult result);
    void StoreLastTickResult(const HardwareFrameHostTickResult &result);
    void DestroyRhiDevice();

    HardwareFrameHostDesc desc_;
    platform::WindowsPlatformWindow window_;
    input::InputBridge input_bridge_;
    audio::AudioCallbackDevice audio_device_;
    rendercore::RenderSwapchainFramePipeline render_pipeline_;
    std::array<platform::PlatformWindowEvent, HardwareFrameHostDesc::MAX_PLATFORM_EVENT_CAPACITY> platform_events_;
    std::array<std::byte, MAX_RHI_DEVICE_STORAGE_BYTES + MAX_RHI_DEVICE_STORAGE_ALIGNMENT> rhi_device_storage_;
    rhi::IRhiDevice *rhi_device_ = nullptr;
    HardwareFrameHostSnapshot snapshot_;
    bool initialized_ = false;
    bool audio_initialized_ = false;
    bool audio_available_ = false;
};
}
