#include "yuengine/diagnostics/bounded_diagnostics_channel.h"

#include <limits>

namespace yuengine::diagnostics {
namespace {
constexpr std::size_t INVALID_INDEX = MAX_DIAGNOSTICS_COUNTERS;
}

BoundedDiagnosticsChannel::BoundedDiagnosticsChannel(diagnostics_channel_config_t config)
    : _config(config),
      _configurationStatus(ValidateConfig(config)),
      _snapshot{
          {},
          {},
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          _configurationStatus == DIAGNOSTICS_STATUS::Success,
          false,
          memory::MEMORY_ACCOUNTING_STATUS::ExplicitlyTrackedOnly},
      _acceptedEventIds{},
      _acceptedCounterIds{},
      _acceptedEventIdCount(0U),
      _acceptedCounterIdCount(0U) {
}

DIAGNOSTICS_STATUS BoundedDiagnosticsChannel::RegisterEventId(diagnostics_event_id_t eventId) {
    if (_configurationStatus != DIAGNOSTICS_STATUS::Success) {
        return _configurationStatus;
    }

    if (_snapshot.Stopped) {
        return DIAGNOSTICS_STATUS::Stopped;
    }

    if (!eventId.IsValid()) {
        return DIAGNOSTICS_STATUS::UnknownEventId;
    }

    if (HasAcceptedEventId(eventId)) {
        return DIAGNOSTICS_STATUS::Success;
    }

    if (_acceptedEventIdCount >= _config.AcceptedEventIdCapacity) {
        return DIAGNOSTICS_STATUS::CapacityExceeded;
    }

    _acceptedEventIds[_acceptedEventIdCount] = eventId;
    ++_acceptedEventIdCount;
    return DIAGNOSTICS_STATUS::Success;
}

DIAGNOSTICS_STATUS BoundedDiagnosticsChannel::RegisterCounterId(diagnostics_counter_id_t counterId) {
    if (_configurationStatus != DIAGNOSTICS_STATUS::Success) {
        return _configurationStatus;
    }

    if (_snapshot.Stopped) {
        return DIAGNOSTICS_STATUS::Stopped;
    }

    if (!counterId.IsValid()) {
        return DIAGNOSTICS_STATUS::UnknownCounterId;
    }

    if (HasAcceptedCounterId(counterId)) {
        return DIAGNOSTICS_STATUS::Success;
    }

    if (_acceptedCounterIdCount >= _config.AcceptedCounterIdCapacity) {
        return DIAGNOSTICS_STATUS::CapacityExceeded;
    }

    if (_snapshot.CounterCount >= _config.CounterCapacity) {
        return DIAGNOSTICS_STATUS::CapacityExceeded;
    }

    _acceptedCounterIds[_acceptedCounterIdCount] = counterId;
    ++_acceptedCounterIdCount;
    _snapshot.Counters[_snapshot.CounterCount] = diagnostics_counter_snapshot_t{counterId, 0U, 0U};
    ++_snapshot.CounterCount;
    return DIAGNOSTICS_STATUS::Success;
}

DIAGNOSTICS_STATUS BoundedDiagnosticsChannel::RecordEvent(diagnostics_event_id_t eventId, std::uint64_t payload) {
    if (_configurationStatus != DIAGNOSTICS_STATUS::Success) {
        return _configurationStatus;
    }

    if (_snapshot.Stopped) {
        return DIAGNOSTICS_STATUS::Stopped;
    }

    if (_config.ValidateIds && !HasAcceptedEventId(eventId)) {
        return DIAGNOSTICS_STATUS::UnknownEventId;
    }

    if (_snapshot.EventCount >= _config.EventCapacity) {
        ++_snapshot.DroppedEventCount;
        return DIAGNOSTICS_STATUS::Dropped;
    }

    _snapshot.Events[_snapshot.EventCount] = diagnostics_event_t{eventId, payload};
    ++_snapshot.EventCount;
    ++_snapshot.AcceptedEventCount;
    return DIAGNOSTICS_STATUS::Success;
}

DIAGNOSTICS_STATUS BoundedDiagnosticsChannel::IncrementCounter(diagnostics_counter_id_t counterId) {
    return AddCounter(counterId, 1U);
}

DIAGNOSTICS_STATUS BoundedDiagnosticsChannel::AddCounter(diagnostics_counter_id_t counterId, std::uint64_t delta) {
    if (_configurationStatus != DIAGNOSTICS_STATUS::Success) {
        return _configurationStatus;
    }

    if (_snapshot.Stopped) {
        return DIAGNOSTICS_STATUS::Stopped;
    }

    const std::size_t counterIndex = CounterIndex(counterId);
    if (counterIndex == INVALID_INDEX) {
        return DIAGNOSTICS_STATUS::UnknownCounterId;
    }

    diagnostics_counter_snapshot_t& counter = _snapshot.Counters[counterIndex];
    const std::uint64_t maxValue = std::numeric_limits<std::uint64_t>::max();
    if (counter.Value > maxValue - delta) {
        return DIAGNOSTICS_STATUS::CounterOverflow;
    }

    counter.Value += delta;
    ++counter.SuccessfulUpdateCount;
    ++_snapshot.SuccessfulCounterUpdateCount;
    return DIAGNOSTICS_STATUS::Success;
}

DIAGNOSTICS_STATUS BoundedDiagnosticsChannel::Shutdown() {
    if (_configurationStatus != DIAGNOSTICS_STATUS::Success) {
        return _configurationStatus;
    }

    _snapshot.Stopped = true;
    return DIAGNOSTICS_STATUS::Success;
}

diagnostics_snapshot_t BoundedDiagnosticsChannel::Snapshot() {
    if (_snapshot.Stopped) {
        return _snapshot;
    }

    ++_snapshot.SnapshotQueryCount;
    return _snapshot;
}

DIAGNOSTICS_STATUS BoundedDiagnosticsChannel::ValidateConfig(diagnostics_channel_config_t config) const {
    if (config.EventCapacity == 0U) {
        return DIAGNOSTICS_STATUS::InvalidCapacity;
    }

    if (config.CounterCapacity == 0U) {
        return DIAGNOSTICS_STATUS::InvalidCapacity;
    }

    if (config.AcceptedEventIdCapacity == 0U) {
        return DIAGNOSTICS_STATUS::InvalidCapacity;
    }

    if (config.AcceptedCounterIdCapacity == 0U) {
        return DIAGNOSTICS_STATUS::InvalidCapacity;
    }

    if (config.EventCapacity > MAX_DIAGNOSTICS_EVENTS) {
        return DIAGNOSTICS_STATUS::InvalidCapacity;
    }

    if (config.CounterCapacity > MAX_DIAGNOSTICS_COUNTERS) {
        return DIAGNOSTICS_STATUS::InvalidCapacity;
    }

    if (config.AcceptedEventIdCapacity > MAX_DIAGNOSTICS_EVENT_IDS) {
        return DIAGNOSTICS_STATUS::InvalidCapacity;
    }

    if (config.AcceptedCounterIdCapacity > MAX_DIAGNOSTICS_COUNTER_IDS) {
        return DIAGNOSTICS_STATUS::InvalidCapacity;
    }

    return DIAGNOSTICS_STATUS::Success;
}

bool BoundedDiagnosticsChannel::HasAcceptedEventId(diagnostics_event_id_t eventId) const {
    for (std::size_t index = 0U; index < _acceptedEventIdCount; ++index) {
        if (_acceptedEventIds[index].Value == eventId.Value) {
            return true;
        }
    }

    return false;
}

bool BoundedDiagnosticsChannel::HasAcceptedCounterId(diagnostics_counter_id_t counterId) const {
    return CounterIndex(counterId) != INVALID_INDEX;
}

std::size_t BoundedDiagnosticsChannel::CounterIndex(diagnostics_counter_id_t counterId) const {
    for (std::size_t index = 0U; index < _snapshot.CounterCount; ++index) {
        if (_snapshot.Counters[index].Id.Value == counterId.Value) {
            return index;
        }
    }

    return INVALID_INDEX;
}
}
