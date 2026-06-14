#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "yuengine/platform/HostError.h"

namespace yuengine::platform {
class IHostRuntime {
public:
    virtual ~IHostRuntime() = default;

    virtual HostError Start(std::vector<std::string>& lifecycleTrace) = 0;
    virtual HostError Tick(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) = 0;
    virtual HostError Shutdown(std::vector<std::string>& lifecycleTrace) = 0;
};
}
