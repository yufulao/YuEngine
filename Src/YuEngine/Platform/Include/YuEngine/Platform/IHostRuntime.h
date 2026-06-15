// Module: YuEngine Platform
// File: Src/YuEngine/Platform/Include/YuEngine/Platform/IHostRuntime.h

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "YuEngine/Platform/HostError.h"

namespace yuengine::platform {
class IHostRuntime {
public:
    virtual ~IHostRuntime() = default;

    /**
     * @comment Starts the component.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Start value.
     */
    virtual HostError Start(std::vector<std::string>& lifecycle_trace) = 0;
    /**
     * @comment Ticks the runtime for one frame.
     * @param frame_index Input frame index.
     * @param tick_time_nanoseconds Input tick time nanoseconds.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Tick value.
     */
    virtual HostError Tick(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) = 0;
    /**
     * @comment Shuts down the component.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Shutdown value.
     */
    virtual HostError Shutdown(std::vector<std::string>& lifecycle_trace) = 0;
};
}
