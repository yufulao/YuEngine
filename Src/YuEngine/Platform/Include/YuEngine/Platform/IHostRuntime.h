#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "YuEngine/Platform/HostError.h"

namespace yuengine::platform {
class IHostRuntime {
public:
    virtual ~IHostRuntime() = default;

    virtual HostError Start(std::vector<std::string>& lifecycle_trace) = 0;
    virtual HostError Tick(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) = 0;
    virtual HostError Shutdown(std::vector<std::string>& lifecycle_trace) = 0;
};
}
