#include "yuengine/diagnostics/DisabledDiagnosticsChannel.h"

namespace yuengine::diagnostics {
DiagnosticsStatus DisabledDiagnosticsChannel::RegisterEventId(DiagnosticsEventId eventId) {
    static_cast<void>(eventId);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::RegisterCounterId(DiagnosticsCounterId counterId) {
    static_cast<void>(counterId);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::RecordEvent(DiagnosticsEventId eventId, std::uint64_t payload) {
    static_cast<void>(eventId);
    static_cast<void>(payload);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::IncrementCounter(DiagnosticsCounterId counterId) {
    static_cast<void>(counterId);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::AddCounter(DiagnosticsCounterId counterId, std::uint64_t delta) {
    static_cast<void>(counterId);
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
