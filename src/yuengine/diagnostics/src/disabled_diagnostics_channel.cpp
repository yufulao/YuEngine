#include "yuengine/diagnostics/disabled_diagnostics_channel.h"

namespace yuengine::diagnostics {
DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::RegisterEventId(DiagnosticsEventId eventId) {
    static_cast<void>(eventId);
    return DIAGNOSTICS_STATUS::Disabled;
}

DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::RegisterCounterId(DiagnosticsCounterId counterId) {
    static_cast<void>(counterId);
    return DIAGNOSTICS_STATUS::Disabled;
}

DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::RecordEvent(DiagnosticsEventId eventId, std::uint64_t payload) {
    static_cast<void>(eventId);
    static_cast<void>(payload);
    return DIAGNOSTICS_STATUS::Disabled;
}

DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::IncrementCounter(DiagnosticsCounterId counterId) {
    static_cast<void>(counterId);
    return DIAGNOSTICS_STATUS::Disabled;
}

DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::AddCounter(DiagnosticsCounterId counterId, std::uint64_t delta) {
    static_cast<void>(counterId);
    static_cast<void>(delta);
    return DIAGNOSTICS_STATUS::Disabled;
}

DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::Shutdown() {
    return DIAGNOSTICS_STATUS::Disabled;
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
        memory::MEMORY_ACCOUNTING_STATUS::ExplicitlyTrackedOnly};
}
}
