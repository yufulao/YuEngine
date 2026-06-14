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
    : kernel_(kernel) {
}

platform::HostError KernelHostRuntime::Start(std::vector<std::string>& lifecycle_trace) {
    return ToHostError(kernel_.Start(lifecycle_trace));
}

platform::HostError KernelHostRuntime::Tick(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) {
    return ToHostError(kernel_.Update(frame_index, tick_time_nanoseconds, lifecycle_trace));
}

platform::HostError KernelHostRuntime::Shutdown(std::vector<std::string>& lifecycle_trace) {
    return ToHostError(kernel_.Shutdown(lifecycle_trace));
}
}
