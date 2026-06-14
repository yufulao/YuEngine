#pragma once

#include "YuEngine/Kernel/EngineKernel.h"
#include "YuEngine/Platform/IHostRuntime.h"

namespace yuengine::kernel {
class KernelHostRuntime final : public platform::IHostRuntime {
public:
    explicit KernelHostRuntime(EngineKernel& kernel);

    platform::HostError Start(std::vector<std::string>& lifecycle_trace) override;
    platform::HostError Tick(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) override;
    platform::HostError Shutdown(std::vector<std::string>& lifecycle_trace) override;

private:
    EngineKernel& kernel_;
};
}
