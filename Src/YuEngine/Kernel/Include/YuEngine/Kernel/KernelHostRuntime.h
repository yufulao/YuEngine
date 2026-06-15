// Module: YuEngine Kernel
// File: Src/YuEngine/Kernel/Include/YuEngine/Kernel/KernelHostRuntime.h

#pragma once

#include "YuEngine/Kernel/EngineKernel.h"
#include "YuEngine/Platform/IHostRuntime.h"

namespace yuengine::kernel {
class KernelHostRuntime final : public platform::IHostRuntime {
public:
    /**
     * @comment Constructs a KernelHostRuntime instance.
     * @param kernel Kernel updated by the function.
     */
    explicit KernelHostRuntime(EngineKernel& kernel);

    /**
     * @comment Starts the component.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Start value.
     */
    platform::HostError Start(std::vector<std::string>& lifecycle_trace) override;
    /**
     * @comment Ticks the runtime for one frame.
     * @param frame_index Input frame index.
     * @param tick_time_nanoseconds Input tick time nanoseconds.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Tick value.
     */
    platform::HostError Tick(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) override;
    /**
     * @comment Shuts down the component.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Shutdown value.
     */
    platform::HostError Shutdown(std::vector<std::string>& lifecycle_trace) override;

private:
    EngineKernel& kernel_;
};
}
