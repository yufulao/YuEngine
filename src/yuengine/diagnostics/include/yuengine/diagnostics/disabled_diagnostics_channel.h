#pragma once

#include <cstdint>

#include "yuengine/diagnostics/diagnostics_counter_id.h"
#include "yuengine/diagnostics/diagnostics_event_id.h"
#include "yuengine/diagnostics/diagnostics_snapshot.h"
#include "yuengine/diagnostics/diagnostics_status.h"

namespace yuengine::diagnostics {
class DisabledDiagnosticsChannel final {
public:
    DiagnosticsStatus RegisterEventId(DiagnosticsEventId eventId);
    DiagnosticsStatus RegisterCounterId(DiagnosticsCounterId counterId);
    DiagnosticsStatus RecordEvent(DiagnosticsEventId eventId, std::uint64_t payload);
    DiagnosticsStatus IncrementCounter(DiagnosticsCounterId counterId);
    DiagnosticsStatus AddCounter(DiagnosticsCounterId counterId, std::uint64_t delta);
    DiagnosticsStatus Shutdown();
    DiagnosticsSnapshot Snapshot() const;
};
}
