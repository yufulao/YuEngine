#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "YuEngine/Kernel/KernelResult.h"
#include "YuEngine/Kernel/ServiceRegistry.h"

namespace yuengine::kernel {
class IModule {
public:
    virtual ~IModule() = default;

    virtual std::string_view Name() const = 0;
    virtual std::vector<std::string_view> Dependencies() const = 0;
    virtual std::vector<std::string_view> RequiredServices() const = 0;
    virtual std::vector<std::string_view> PublishedServices() const = 0;
    virtual KernelResult Start(ServiceRegistry& service_registry, std::vector<std::string>& lifecycle_trace) = 0;
    virtual KernelResult Update(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) = 0;
    virtual KernelResult Shutdown(std::vector<std::string>& lifecycle_trace) = 0;
};
}
