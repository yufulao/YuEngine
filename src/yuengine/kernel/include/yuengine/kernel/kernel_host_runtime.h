#pragma once

#include "yuengine/kernel/engine_kernel.h"
#include "yuengine/platform/i_host_runtime.h"

namespace yuengine::kernel {
class KernelHostRuntime final : public platform::IHostRuntime {
public:
    explicit KernelHostRuntime(EngineKernel& kernel);

    platform::host_error_t Start(std::vector<std::string>& lifecycleTrace) override;
    platform::host_error_t Tick(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) override;
    platform::host_error_t Shutdown(std::vector<std::string>& lifecycleTrace) override;

private:
    EngineKernel& _kernel;
};
}
