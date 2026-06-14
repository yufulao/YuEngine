#include "YuEngine/Kernel/KernelHostRuntime.h"

namespace yuengine::kernel {
namespace {
platform::HostError ToHostError(const KernelResult& result) {
    if (result.succeeded) {
        return platform::HostError::Success();
    }

    return platform::HostError::Failure(result.message);
}
}

KernelHostRuntime::KernelHostRuntime(EngineKernel& kernel)
    : _kernel(kernel) {
}

platform::HostError KernelHostRuntime::Start(std::vector<std::string>& lifecycleTrace) {
    return ToHostError(_kernel.Start(lifecycleTrace));
}

platform::HostError KernelHostRuntime::Tick(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) {
    return ToHostError(_kernel.Update(frameIndex, tickTimeNanoseconds, lifecycleTrace));
}

platform::HostError KernelHostRuntime::Shutdown(std::vector<std::string>& lifecycleTrace) {
    return ToHostError(_kernel.Shutdown(lifecycleTrace));
}
}
