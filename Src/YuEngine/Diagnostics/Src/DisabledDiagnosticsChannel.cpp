// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Src/DisabledDiagnosticsChannel.cpp

#include "YuEngine/Diagnostics/DisabledDiagnosticsChannel.h"

namespace yuengine::diagnostics {
DiagnosticsStatus DisabledDiagnosticsChannel::RegisterEventId(DiagnosticsEventId event_id) {
    static_cast<void>(event_id);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::RegisterCounterId(DiagnosticsCounterId counter_id) {
    static_cast<void>(counter_id);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::RecordEvent(DiagnosticsEventId event_id, std::uint64_t payload) {
    static_cast<void>(event_id);
    static_cast<void>(payload);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::IncrementCounter(DiagnosticsCounterId counter_id) {
    static_cast<void>(counter_id);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::AddCounter(DiagnosticsCounterId counter_id, std::uint64_t delta) {
    static_cast<void>(counter_id);
    static_cast<void>(delta);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::Shutdown() {
    return DiagnosticsStatus::Disabled;
}

DiagnosticsSnapshot DisabledDiagnosticsChannel::Snapshot() const {
    return DiagnosticsSnapshot{
        {},
        {},
        0U,
        0U,
        0U,
        0U,
        0U,
        0U,
        false,
        false,
        memory::MemoryAccountingStatus::ExplicitlyTrackedOnly};
}
}
