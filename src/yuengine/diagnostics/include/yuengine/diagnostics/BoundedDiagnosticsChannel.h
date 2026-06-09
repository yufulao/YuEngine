#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "yuengine/diagnostics/DiagnosticsChannelConfig.h"
#include "yuengine/diagnostics/DiagnosticsCounterId.h"
#include "yuengine/diagnostics/DiagnosticsCounterSnapshot.h"
#include "yuengine/diagnostics/DiagnosticsEvent.h"
#include "yuengine/diagnostics/DiagnosticsEventId.h"
#include "yuengine/diagnostics/DiagnosticsLimits.h"
#include "yuengine/diagnostics/DiagnosticsSnapshot.h"
#include "yuengine/diagnostics/DiagnosticsStatus.h"

namespace yuengine::diagnostics
{
class BoundedDiagnosticsChannel final
{
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
