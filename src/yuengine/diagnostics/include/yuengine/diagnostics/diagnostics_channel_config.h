#pragma once

#include <cstddef>

namespace yuengine::diagnostics {
struct diagnostics_channel_config_t {
    std::size_t EventCapacity;
    std::size_t CounterCapacity;
    std::size_t AcceptedEventIdCapacity;
    std::size_t AcceptedCounterIdCapacity;
    bool ValidateIds;
};
}
