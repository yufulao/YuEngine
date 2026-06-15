// Module: YuEngine Diagnostics
// File: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DiagnosticsStatus.h

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
