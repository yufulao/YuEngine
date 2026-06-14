#pragma once

#include <cstdint>

#include "yuengine/diagnostics/diagnostics_counter_id.h"
#include "yuengine/diagnostics/diagnostics_event_id.h"
#include "yuengine/diagnostics/diagnostics_snapshot.h"
#include "yuengine/diagnostics/diagnostics_status.h"

namespace yuengine::diagnostics {
class DisabledDiagnosticsChannel final {
public:
    DiagnosticsStatus RegisterEventId(diagnostics_event_id_t eventId);
    DiagnosticsStatus RegisterCounterId(diagnostics_counter_id_t counterId);
    DiagnosticsStatus RecordEvent(diagnostics_event_id_t eventId, std::uint64_t payload);
    DiagnosticsStatus IncrementCounter(diagnostics_counter_id_t counterId);
    DiagnosticsStatus AddCounter(diagnostics_counter_id_t counterId, std::uint64_t delta);
    DiagnosticsStatus Shutdown();
    diagnostics_snapshot_t Snapshot() const;
};
}
