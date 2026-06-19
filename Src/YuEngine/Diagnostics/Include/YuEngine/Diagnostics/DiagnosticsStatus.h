// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DiagnosticsStatus.h

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
    InvalidArgument,
    InvalidCapacity
};
}
