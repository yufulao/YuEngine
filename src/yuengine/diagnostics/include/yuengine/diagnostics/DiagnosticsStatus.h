#pragma once

namespace yuengine::diagnostics {
enum class DiagnosticsStatus {
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
