#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "yuengine/platform/host_error.h"

namespace yuengine::platform {
class IHostRuntime {
public:
    virtual ~IHostRuntime() = default;

    virtual host_error_t Start(std::vector<std::string>& lifecycleTrace) = 0;
    virtual host_error_t Tick(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) = 0;
    virtual host_error_t Shutdown(std::vector<std::string>& lifecycleTrace) = 0;
};
}
