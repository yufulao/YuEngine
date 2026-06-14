#include "YuEngine/Diagnostics/BoundedDiagnosticsChannel.h"

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

    if (_snapshot.stopped) {
        return DiagnosticsStatus::Stopped;
    }

    if (!eventId.IsValid()) {
        return DiagnosticsStatus::UnknownEventId;
    }

    if (HasAcceptedEventId(eventId)) {
        return DiagnosticsStatus::Success;
    }

    if (_acceptedEventIdCount >= _config.accepted_event_id_capacity) {
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

    if (_snapshot.stopped) {
        return DiagnosticsStatus::Stopped;
    }

    if (!counterId.IsValid()) {
        return DiagnosticsStatus::UnknownCounterId;
    }

    if (HasAcceptedCounterId(counterId)) {
        return DiagnosticsStatus::Success;
    }

    if (_acceptedCounterIdCount >= _config.accepted_counter_id_capacity) {
        return DiagnosticsStatus::CapacityExceeded;
    }

    if (_snapshot.counter_count >= _config.counter_capacity) {
        return DiagnosticsStatus::CapacityExceeded;
    }

    _acceptedCounterIds[_acceptedCounterIdCount] = counterId;
    ++_acceptedCounterIdCount;
    _snapshot.counters[_snapshot.counter_count] = DiagnosticsCounterSnapshot{counterId, 0U, 0U};
    ++_snapshot.counter_count;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::RecordEvent(DiagnosticsEventId eventId, std::uint64_t payload) {
    if (_configurationStatus != DiagnosticsStatus::Success) {
        return _configurationStatus;
    }

    if (_snapshot.stopped) {
        return DiagnosticsStatus::Stopped;
    }

    if (_config.validate_ids && !HasAcceptedEventId(eventId)) {
        return DiagnosticsStatus::UnknownEventId;
    }

    if (_snapshot.event_count >= _config.event_capacity) {
        ++_snapshot.dropped_event_count;
        return DiagnosticsStatus::Dropped;
    }

    _snapshot.events[_snapshot.event_count] = DiagnosticsEvent{eventId, payload};
    ++_snapshot.event_count;
    ++_snapshot.accepted_event_count;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::IncrementCounter(DiagnosticsCounterId counterId) {
    return AddCounter(counterId, 1U);
}

DiagnosticsStatus BoundedDiagnosticsChannel::AddCounter(DiagnosticsCounterId counterId, std::uint64_t delta) {
    if (_configurationStatus != DiagnosticsStatus::Success) {
        return _configurationStatus;
    }

    if (_snapshot.stopped) {
        return DiagnosticsStatus::Stopped;
    }

    const std::size_t counterIndex = CounterIndex(counterId);
    if (counterIndex == INVALID_INDEX) {
        return DiagnosticsStatus::UnknownCounterId;
    }

    DiagnosticsCounterSnapshot& counter = _snapshot.counters[counterIndex];
    const std::uint64_t maxValue = std::numeric_limits<std::uint64_t>::max();
    if (counter.value > maxValue - delta) {
        return DiagnosticsStatus::CounterOverflow;
    }

    counter.value += delta;
    ++counter.successful_update_count;
    ++_snapshot.successful_counter_update_count;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::Shutdown() {
    if (_configurationStatus != DiagnosticsStatus::Success) {
        return _configurationStatus;
    }

    _snapshot.stopped = true;
    return DiagnosticsStatus::Success;
}

DiagnosticsSnapshot BoundedDiagnosticsChannel::Snapshot() {
    if (_snapshot.stopped) {
        return _snapshot;
    }

    ++_snapshot.snapshot_query_count;
    return _snapshot;
}

DiagnosticsStatus BoundedDiagnosticsChannel::ValidateConfig(DiagnosticsChannelConfig config) const {
    if (config.event_capacity == 0U) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.counter_capacity == 0U) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.accepted_event_id_capacity == 0U) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.accepted_counter_id_capacity == 0U) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.event_capacity > MAX_DIAGNOSTICS_EVENTS) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.counter_capacity > MAX_DIAGNOSTICS_COUNTERS) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.accepted_event_id_capacity > MAX_DIAGNOSTICS_EVENT_IDS) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    if (config.accepted_counter_id_capacity > MAX_DIAGNOSTICS_COUNTER_IDS) {
        return DiagnosticsStatus::InvalidCapacity;
    }

    return DiagnosticsStatus::Success;
}

bool BoundedDiagnosticsChannel::HasAcceptedEventId(DiagnosticsEventId eventId) const {
    for (std::size_t index = 0U; index < _acceptedEventIdCount; ++index) {
        if (_acceptedEventIds[index].value == eventId.value) {
            return true;
        }
    }

    return false;
}

bool BoundedDiagnosticsChannel::HasAcceptedCounterId(DiagnosticsCounterId counterId) const {
    return CounterIndex(counterId) != INVALID_INDEX;
}

std::size_t BoundedDiagnosticsChannel::CounterIndex(DiagnosticsCounterId counterId) const {
    for (std::size_t index = 0U; index < _snapshot.counter_count; ++index) {
        if (_snapshot.counters[index].id.value == counterId.value) {
            return index;
        }
    }

    return INVALID_INDEX;
}
}
