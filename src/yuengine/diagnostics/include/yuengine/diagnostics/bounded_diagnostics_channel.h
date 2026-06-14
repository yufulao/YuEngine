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
    explicit BoundedDiagnosticsChannel(diagnostics_channel_config_t config);

    DIAGNOSTICS_STATUS RegisterEventId(diagnostics_event_id_t eventId);
    DIAGNOSTICS_STATUS RegisterCounterId(diagnostics_counter_id_t counterId);
    DIAGNOSTICS_STATUS RecordEvent(diagnostics_event_id_t eventId, std::uint64_t payload);
    DIAGNOSTICS_STATUS IncrementCounter(diagnostics_counter_id_t counterId);
    DIAGNOSTICS_STATUS AddCounter(diagnostics_counter_id_t counterId, std::uint64_t delta);
    DIAGNOSTICS_STATUS Shutdown();
    diagnostics_snapshot_t Snapshot();

private:
    DIAGNOSTICS_STATUS ValidateConfig(diagnostics_channel_config_t config) const;
    bool HasAcceptedEventId(diagnostics_event_id_t eventId) const;
    bool HasAcceptedCounterId(diagnostics_counter_id_t counterId) const;
    std::size_t CounterIndex(diagnostics_counter_id_t counterId) const;

    diagnostics_channel_config_t _config;
    DIAGNOSTICS_STATUS _configurationStatus;
    diagnostics_snapshot_t _snapshot;
    std::array<diagnostics_event_id_t, MAX_DIAGNOSTICS_EVENT_IDS> _acceptedEventIds;
    std::array<diagnostics_counter_id_t, MAX_DIAGNOSTICS_COUNTER_IDS> _acceptedCounterIds;
    std::size_t _acceptedEventIdCount;
    std::size_t _acceptedCounterIdCount;
};
}
