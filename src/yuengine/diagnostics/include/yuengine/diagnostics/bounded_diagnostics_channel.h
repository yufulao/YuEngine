#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "yuengine/diagnostics/diagnostics_channel_config.h"
#include "yuengine/diagnostics/diagnostics_counter_id.h"
#include "yuengine/diagnostics/diagnostics_counter_snapshot.h"
#include "yuengine/diagnostics/diagnostics_event.h"
#include "yuengine/diagnostics/diagnostics_event_id.h"
#include "yuengine/diagnostics/diagnostics_limits.h"
#include "yuengine/diagnostics/diagnostics_snapshot.h"
#include "yuengine/diagnostics/diagnostics_status.h"

namespace yuengine::diagnostics {
class BoundedDiagnosticsChannel final {
public:
    explicit BoundedDiagnosticsChannel(DiagnosticsChannelConfig config);

    DiagnosticsStatus RegisterEventId(DiagnosticsEventId eventId);
    DiagnosticsStatus RegisterCounterId(DiagnosticsCounterId counterId);
    DiagnosticsStatus RecordEvent(DiagnosticsEventId eventId, std::uint64_t payload);
    DiagnosticsStatus IncrementCounter(DiagnosticsCounterId counterId);
    DiagnosticsStatus AddCounter(DiagnosticsCounterId counterId, std::uint64_t delta);
    DiagnosticsStatus Shutdown();
    DiagnosticsSnapshot Snapshot();

private:
    DiagnosticsStatus ValidateConfig(DiagnosticsChannelConfig config) const;
    bool HasAcceptedEventId(DiagnosticsEventId eventId) const;
    bool HasAcceptedCounterId(DiagnosticsCounterId counterId) const;
    std::size_t CounterIndex(DiagnosticsCounterId counterId) const;

    DiagnosticsChannelConfig _config;
    DiagnosticsStatus _configurationStatus;
    DiagnosticsSnapshot _snapshot;
    std::array<DiagnosticsEventId, MAX_DIAGNOSTICS_EVENT_IDS> _acceptedEventIds;
    std::array<DiagnosticsCounterId, MAX_DIAGNOSTICS_COUNTER_IDS> _acceptedCounterIds;
    std::size_t _acceptedEventIdCount;
    std::size_t _acceptedCounterIdCount;
};
}
