#pragma once

#include <cstdint>

#include "yuengine/diagnostics/diagnostics_counter_id.h"
#include "yuengine/diagnostics/diagnostics_event_id.h"
#include "yuengine/diagnostics/diagnostics_snapshot.h"
#include "yuengine/diagnostics/diagnostics_status.h"

namespace yuengine::diagnostics {
class DisabledDiagnosticsChannel final {
public:
    DIAGNOSTICS_STATUS RegisterEventId(diagnostics_event_id_t eventId);
    DIAGNOSTICS_STATUS RegisterCounterId(diagnostics_counter_id_t counterId);
    DIAGNOSTICS_STATUS RecordEvent(diagnostics_event_id_t eventId, std::uint64_t payload);
    DIAGNOSTICS_STATUS IncrementCounter(diagnostics_counter_id_t counterId);
    DIAGNOSTICS_STATUS AddCounter(diagnostics_counter_id_t counterId, std::uint64_t delta);
    DIAGNOSTICS_STATUS Shutdown();
    diagnostics_snapshot_t Snapshot() const;
};
}
