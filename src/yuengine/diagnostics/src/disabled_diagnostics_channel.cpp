#include "yuengine/diagnostics/disabled_diagnostics_channel.h"

namespace yuengine::diagnostics {
DiagnosticsStatus DisabledDiagnosticsChannel::RegisterEventId(diagnostics_event_id_t eventId) {
    static_cast<void>(eventId);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::RegisterCounterId(diagnostics_counter_id_t counterId) {
    static_cast<void>(counterId);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::RecordEvent(diagnostics_event_id_t eventId, std::uint64_t payload) {
    static_cast<void>(eventId);
    static_cast<void>(payload);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::IncrementCounter(diagnostics_counter_id_t counterId) {
    static_cast<void>(counterId);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::AddCounter(diagnostics_counter_id_t counterId, std::uint64_t delta) {
    static_cast<void>(counterId);
    static_cast<void>(delta);
    return DiagnosticsStatus::Disabled;
}

DiagnosticsStatus DisabledDiagnosticsChannel::Shutdown() {
    return DiagnosticsStatus::Disabled;
}

diagnostics_snapshot_t DisabledDiagnosticsChannel::Snapshot() const {
    return diagnostics_snapshot_t{
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
