#pragma once

namespace yuengine::diagnostics {
enum class DIAGNOSTICS_STATUS {
    Success,
    Dropped,
    Disabled,
    Stopped,
    UnknownEventId,
    UnknownCounterId,
    CapacityExceeded,
    CounterOverflow,
    InvalidCapacity
};
}
