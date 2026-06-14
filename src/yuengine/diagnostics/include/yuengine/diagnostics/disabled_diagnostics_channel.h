#pragma once

#include <cstdint>

#include "yuengine/diagnostics/diagnostics_counter_id.h"
#include "yuengine/diagnostics/diagnostics_event_id.h"
#include "yuengine/diagnostics/diagnostics_snapshot.h"
#include "yuengine/diagnostics/diagnostics_status.h"

namespace yuengine::diagnostics {
class DisabledDiagnosticsChannel final {
public:
    DIAGNOSTICS_STATUS RegisterEventId(DiagnosticsEventId eventId);
    DIAGNOSTICS_STATUS RegisterCounterId(DiagnosticsCounterId counterId);
    DIAGNOSTICS_STATUS RecordEvent(DiagnosticsEventId eventId, std::uint64_t payload);
    DIAGNOSTICS_STATUS IncrementCounter(DiagnosticsCounterId counterId);
    DIAGNOSTICS_STATUS AddCounter(DiagnosticsCounterId counterId, std::uint64_t delta);
    DIAGNOSTICS_STATUS Shutdown();
    DiagnosticsSnapshot Snapshot() const;
};
}
