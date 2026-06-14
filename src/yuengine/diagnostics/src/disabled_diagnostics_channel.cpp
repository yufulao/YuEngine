#include "yuengine/diagnostics/disabled_diagnostics_channel.h"

namespace yuengine::diagnostics {
DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::RegisterEventId(diagnostics_event_id_t eventId) {
    static_cast<void>(eventId);
    return DIAGNOSTICS_STATUS::Disabled;
}

DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::RegisterCounterId(diagnostics_counter_id_t counterId) {
    static_cast<void>(counterId);
    return DIAGNOSTICS_STATUS::Disabled;
}

DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::RecordEvent(diagnostics_event_id_t eventId, std::uint64_t payload) {
    static_cast<void>(eventId);
    static_cast<void>(payload);
    return DIAGNOSTICS_STATUS::Disabled;
}

DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::IncrementCounter(diagnostics_counter_id_t counterId) {
    static_cast<void>(counterId);
    return DIAGNOSTICS_STATUS::Disabled;
}

DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::AddCounter(diagnostics_counter_id_t counterId, std::uint64_t delta) {
    static_cast<void>(counterId);
    static_cast<void>(delta);
    return DIAGNOSTICS_STATUS::Disabled;
}

DIAGNOSTICS_STATUS DisabledDiagnosticsChannel::Shutdown() {
    return DIAGNOSTICS_STATUS::Disabled;
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
        memory::MEMORY_ACCOUNTING_STATUS::ExplicitlyTrackedOnly};
}
}
