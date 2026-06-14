#pragma once

#include "yuengine/kernel/EngineKernel.h"
#include "yuengine/platform/IHostRuntime.h"

namespace yuengine::kernel {
class KernelHostRuntime final : public platform::IHostRuntime {
public:
    explicit KernelHostRuntime(EngineKernel& kernel);

    platform::HostError Start(std::vector<std::string>& lifecycleTrace) override;
    platform::HostError Tick(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) override;
    platform::HostError Shutdown(std::vector<std::string>& lifecycleTrace) override;

private:
    EngineKernel& _kernel;
};
}
