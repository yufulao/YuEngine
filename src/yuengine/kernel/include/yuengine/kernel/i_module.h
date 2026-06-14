#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "yuengine/kernel/kernel_result.h"
#include "yuengine/kernel/service_registry.h"

namespace yuengine::kernel {
class IModule {
public:
    virtual ~IModule() = default;

    virtual std::string_view Name() const = 0;
    virtual std::vector<std::string_view> Dependencies() const = 0;
    virtual std::vector<std::string_view> RequiredServices() const = 0;
    virtual std::vector<std::string_view> PublishedServices() const = 0;
    virtual kernel_result_t Start(ServiceRegistry& serviceRegistry, std::vector<std::string>& lifecycleTrace) = 0;
    virtual kernel_result_t Update(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) = 0;
    virtual kernel_result_t Shutdown(std::vector<std::string>& lifecycleTrace) = 0;
};
}
