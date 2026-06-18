// 模块: YuEngine Hardware
// 文件: Src/YuEngine/Hardware/Include/YuEngine/Hardware/HardwareFrameHost.h

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
     * @comment 构造空的 hardware frame host。
     */
    HardwareFrameHost();
    /**
     * @comment 析构时释放持有的硬件对象。
     */
    ~HardwareFrameHost();

    HardwareFrameHost(const HardwareFrameHost &) = delete;
    HardwareFrameHost &operator=(const HardwareFrameHost &) = delete;

    /**
     * @comment 按固定容量描述创建窗口、input、可选 RHI 和可选 audio 对象。
     * @param desc 输入 host 描述。
     * @return 显式操作状态。
     */
    HardwareFrameHostStatus Initialize(const HardwareFrameHostDesc &desc);
    /**
     * @comment 运行一次集成的 platform input、render 和可选 audio host tick。
     * @param request 输入 tick 请求和调用方持有的 output buffer。
     * @return 显式 tick 结果。
     */
    HardwareFrameHostTickResult Tick(const HardwareFrameHostTickRequest &request);
    /**
     * @comment 停止并销毁持有的硬件对象。
     * @return 显式操作状态。
     */
    HardwareFrameHostStatus Shutdown();
    /**
     * @comment 返回当前 host 计数器和生命周期标志。
     * @return 快照值。
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
    bool shutdown_completed_ = false;
    bool audio_initialized_ = false;
    bool audio_available_ = false;
};
}
