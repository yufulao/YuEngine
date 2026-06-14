#include "yuengine/diagnostics/bounded_diagnostics_channel.h"

#include <limits>

namespace yuengine::diagnostics {
namespace {
constexpr std::size_t INVALID_INDEX = MAX_DIAGNOSTICS_COUNTERS;
}

BoundedDiagnosticsChannel::BoundedDiagnosticsChannel(DiagnosticsChannelConfig config)
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
          _configurationStatus == DiagnosticsStatus::Success,
          false,
          memory::MemoryAccountingStatus::ExplicitlyTrackedOnly},
      _acceptedEventIds{},
      _acceptedCounterIds{},
      _acceptedEventIdCount(0U),
      _acceptedCounterIdCount(0U) {
}

DiagnosticsStatus BoundedDiagnosticsChannel::RegisterEventId(DiagnosticsEventId eventId) {
    if (_configurationStatus != DiagnosticsStatus::Success) {
        return _configurationStatus;
    }

    if (_snapshot.Stopped) {
        return DiagnosticsStatus::Stopped;
    }

    if (!eventId.IsValid()) {
        return DiagnosticsStatus::UnknownEventId;
    }

    if (HasAcceptedEventId(eventId)) {
        return DiagnosticsStatus::Success;
    }

    if (_acceptedEventIdCount >= _config.AcceptedEventIdCapacity) {
        return DiagnosticsStatus::CapacityExceeded;
    }

    _acceptedEventIds[_acceptedEventIdCount] = eventId;
    ++_acceptedEventIdCount;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::RegisterCounterId(DiagnosticsCounterId counterId) {
    if (_configurationStatus != DiagnosticsStatus::Success) {
        return _configurationStatus;
    }

    if (_snapshot.Stopped) {
        return DiagnosticsStatus::Stopped;
    }

    if (!counterId.IsValid()) {
        return DiagnosticsStatus::UnknownCounterId;
    }

    if (HasAcceptedCounterId(counterId)) {
        return DiagnosticsStatus::Success;
    }

    if (_acceptedCounterIdCount >= _config.AcceptedCounterIdCapacity) {
        return DiagnosticsStatus::CapacityExceeded;
    }

    if (_snapshot.CounterCount >= _config.CounterCapacity) {
        return DiagnosticsStatus::CapacityExceeded;
    }

    _acceptedCounterIds[_acceptedCounterIdCount] = counterId;
    ++_acceptedCounterIdCount;
    _snapshot.Counters[_snapshot.CounterCount] = DiagnosticsCounterSnapshot{counterId, 0U, 0U};
    ++_snapshot.CounterCount;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::RecordEvent(DiagnosticsEventId eventId, std::uint64_t payload) {
    if (_configurationStatus != DiagnosticsStatus::Success) {
        return _configurationStatus;
    }

    if (_snapshot.Stopped) {
        return DiagnosticsStatus::Stopped;
    }

    if (_config.ValidateIds && !HasAcceptedEventId(eventId)) {
        return DiagnosticsStatus::UnknownEventId;
    }

    if (_snapshot.EventCount >= _config.EventCapacity) {
        ++_snapshot.DroppedEventCount;
        return DiagnosticsStatus::Dropped;
    }

    _snapshot.Events[_snapshot.EventCount] = DiagnosticsEvent{eventId, payload};
    ++_snapshot.EventCount;
    ++_snapshot.AcceptedEventCount;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::IncrementCounter(DiagnosticsCounterId counterId) {
    return AddCounter(counterId, 1U);
}

DiagnosticsStatus BoundedDiagnosticsChannel::AddCounter(DiagnosticsCounterId counterId, std::uint64_t delta) {
    if (_configurationStatus != DiagnosticsStatus::Success) {
        return _configurationStatus;
    }

    if (_snapshot.Stopped) {
        return DiagnosticsStatus::Stopped;
    }

    const std::size_t counterIndex = CounterIndex(counterId);
    if (counterIndex == INVALID_INDEX) {
        return DiagnosticsStatus::UnknownCounterId;
    }

    DiagnosticsCounterSnapshot& counter = _snapshot.Counters[counterIndex];
    const std::uint64_t maxValue = std::numeric_limits<std::uint64_t>::max();
    if (counter.Value > maxValue - delta) {
        return DiagnosticsStatus::CounterOverflow;
    }

    counter.Value += delta;
    ++counter.SuccessfulUpdateCount;
    ++_snapshot.SuccessfulCounterUpdateCount;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::Shutdown() {
    if (_configurationStatus != DiagnosticsStatus::Success) {
        return _configurationStatus;
    }

    _snapshot.Stopped = true;
    return DiagnosticsStatus::Success;
}

DiagnosticsSnapshot BoundedDiagnosticsChannel::Snapshot() {
    if (_snapshot.Stopped) {
        return _snapshot;
    }

    ++_snapshot.SnapshotQueryCount;
    return _snapshot;
}

DiagnosticsStatus BoundedDiagnosticsChannel::ValidateConfig(DiagnosticsChannelConfig config) const {
    if (config.EventCapacity == 0U) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.CounterCapacity == 0U) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.AcceptedEventIdCapacity == 0U) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.AcceptedCounterIdCapacity == 0U) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.EventCapacity > MAX_DIAGNOSTICS_EVENTS) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.CounterCapacity > MAX_DIAGNOSTICS_COUNTERS) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.AcceptedEventIdCapacity > MAX_DIAGNOSTICS_EVENT_IDS) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.AcceptedCounterIdCapacity > MAX_DIAGNOSTICS_COUNTER_IDS) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    return DiagnosticsStatus::Success;
}

bool BoundedDiagnosticsChannel::HasAcceptedEventId(DiagnosticsEventId eventId) const {
    for (std::size_t index = 0U; index < _acceptedEventIdCount; ++index) {
        if (_acceptedEventIds[index].Value == eventId.Value) {
            return true;
        }
    }

    return false;
}

bool BoundedDiagnosticsChannel::HasAcceptedCounterId(DiagnosticsCounterId counterId) const {
    return CounterIndex(counterId) != INVALID_INDEX;
}

std::size_t BoundedDiagnosticsChannel::CounterIndex(DiagnosticsCounterId counterId) const {
    for (std::size_t index = 0U; index < _snapshot.CounterCount; ++index) {
        if (_snapshot.Counters[index].Id.Value == counterId.Value) {
            return index;
        }
    }

    return INVALID_INDEX;
}
}
