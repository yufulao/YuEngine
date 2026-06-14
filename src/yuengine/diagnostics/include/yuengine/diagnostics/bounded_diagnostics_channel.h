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

    DIAGNOSTICS_STATUS RegisterEventId(DiagnosticsEventId eventId);
    DIAGNOSTICS_STATUS RegisterCounterId(DiagnosticsCounterId counterId);
    DIAGNOSTICS_STATUS RecordEvent(DiagnosticsEventId eventId, std::uint64_t payload);
    DIAGNOSTICS_STATUS IncrementCounter(DiagnosticsCounterId counterId);
    DIAGNOSTICS_STATUS AddCounter(DiagnosticsCounterId counterId, std::uint64_t delta);
    DIAGNOSTICS_STATUS Shutdown();
    DiagnosticsSnapshot Snapshot();

private:
    DIAGNOSTICS_STATUS ValidateConfig(DiagnosticsChannelConfig config) const;
    bool HasAcceptedEventId(DiagnosticsEventId eventId) const;
    bool HasAcceptedCounterId(DiagnosticsCounterId counterId) const;
    std::size_t CounterIndex(DiagnosticsCounterId counterId) const;

    DiagnosticsChannelConfig _config;
    DIAGNOSTICS_STATUS _configurationStatus;
    DiagnosticsSnapshot _snapshot;
    std::array<DiagnosticsEventId, MAX_DIAGNOSTICS_EVENT_IDS> _acceptedEventIds;
    std::array<DiagnosticsCounterId, MAX_DIAGNOSTICS_COUNTER_IDS> _acceptedCounterIds;
    std::size_t _acceptedEventIdCount;
    std::size_t _acceptedCounterIdCount;
};
}
