#include "yuengine/kernel/kernel_host_runtime.h"

namespace yuengine::kernel {
namespace {
platform::host_error_t ToHostError(const kernel_result_t& result) {
    if (result.Succeeded) {
        return platform::host_error_t::Success();
    }

    return platform::host_error_t::Failure(result.Message);
}
}

KernelHostRuntime::KernelHostRuntime(EngineKernel& kernel)
    : _kernel(kernel) {
}

platform::host_error_t KernelHostRuntime::Start(std::vector<std::string>& lifecycleTrace) {
    return ToHostError(_kernel.Start(lifecycleTrace));
}

platform::host_error_t KernelHostRuntime::Tick(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) {
    return ToHostError(_kernel.Update(frameIndex, tickTimeNanoseconds, lifecycleTrace));
}

platform::host_error_t KernelHostRuntime::Shutdown(std::vector<std::string>& lifecycleTrace) {
    return ToHostError(_kernel.Shutdown(lifecycleTrace));
}
}
