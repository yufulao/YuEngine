#include "YuEngine/Diagnostics/BoundedDiagnosticsChannel.h"

#include <limits>

namespace yuengine::diagnostics {
namespace {
constexpr std::size_t INVALID_INDEX = MAX_DIAGNOSTICS_COUNTERS;
}

BoundedDiagnosticsChannel::BoundedDiagnosticsChannel(DiagnosticsChannelConfig config)
    : config_(config),
      configuration_status_(ValidateConfig(config)),
      snapshot_{
          {},
          {},
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          configuration_status_ == DiagnosticsStatus::Success,
          false,
          memory::MemoryAccountingStatus::ExplicitlyTrackedOnly},
      accepted_event_ids_{},
      accepted_counter_ids_{},
      accepted_event_id_count_(0U),
      accepted_counter_id_count_(0U) {
}

DiagnosticsStatus BoundedDiagnosticsChannel::RegisterEventId(DiagnosticsEventId eventId) {
    if (configuration_status_ != DiagnosticsStatus::Success) {
        return configuration_status_;
    }

    if (snapshot_.stopped) {
        return DiagnosticsStatus::Stopped;
    }

    if (!eventId.IsValid()) {
        return DiagnosticsStatus::UnknownEventId;
    }

    if (HasAcceptedEventId(eventId)) {
        return DiagnosticsStatus::Success;
    }

    if (accepted_event_id_count_ >= config_.accepted_event_id_capacity) {
        return DiagnosticsStatus::CapacityExceeded;
    }

    accepted_event_ids_[accepted_event_id_count_] = eventId;
    ++accepted_event_id_count_;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::RegisterCounterId(DiagnosticsCounterId counterId) {
    if (configuration_status_ != DiagnosticsStatus::Success) {
        return configuration_status_;
    }

    if (snapshot_.stopped) {
        return DiagnosticsStatus::Stopped;
    }

    if (!counterId.IsValid()) {
        return DiagnosticsStatus::UnknownCounterId;
    }

    if (HasAcceptedCounterId(counterId)) {
        return DiagnosticsStatus::Success;
    }

    if (accepted_counter_id_count_ >= config_.accepted_counter_id_capacity) {
        return DiagnosticsStatus::CapacityExceeded;
    }

    if (snapshot_.counter_count >= config_.counter_capacity) {
        return DiagnosticsStatus::CapacityExceeded;
    }

    accepted_counter_ids_[accepted_counter_id_count_] = counterId;
    ++accepted_counter_id_count_;
    snapshot_.counters[snapshot_.counter_count] = DiagnosticsCounterSnapshot{counterId, 0U, 0U};
    ++snapshot_.counter_count;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::RecordEvent(DiagnosticsEventId eventId, std::uint64_t payload) {
    if (configuration_status_ != DiagnosticsStatus::Success) {
        return configuration_status_;
    }

    if (snapshot_.stopped) {
        return DiagnosticsStatus::Stopped;
    }

    if (config_.validate_ids && !HasAcceptedEventId(eventId)) {
        return DiagnosticsStatus::UnknownEventId;
    }

    if (snapshot_.event_count >= config_.event_capacity) {
        ++snapshot_.dropped_event_count;
        return DiagnosticsStatus::Dropped;
    }

    snapshot_.events[snapshot_.event_count] = DiagnosticsEvent{eventId, payload};
    ++snapshot_.event_count;
    ++snapshot_.accepted_event_count;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::IncrementCounter(DiagnosticsCounterId counterId) {
    return AddCounter(counterId, 1U);
}

DiagnosticsStatus BoundedDiagnosticsChannel::AddCounter(DiagnosticsCounterId counterId, std::uint64_t delta) {
    if (configuration_status_ != DiagnosticsStatus::Success) {
        return configuration_status_;
    }

    if (snapshot_.stopped) {
        return DiagnosticsStatus::Stopped;
    }

    const std::size_t counterIndex = CounterIndex(counterId);
    if (counterIndex == INVALID_INDEX) {
        return DiagnosticsStatus::UnknownCounterId;
    }

    DiagnosticsCounterSnapshot& counter = snapshot_.counters[counterIndex];
    const std::uint64_t maxValue = std::numeric_limits<std::uint64_t>::max();
    if (counter.value > maxValue - delta) {
        return DiagnosticsStatus::CounterOverflow;
    }

    counter.value += delta;
    ++counter.successful_update_count;
    ++snapshot_.successful_counter_update_count;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::Shutdown() {
    if (configuration_status_ != DiagnosticsStatus::Success) {
        return configuration_status_;
    }

    snapshot_.stopped = true;
    return DiagnosticsStatus::Success;
}

DiagnosticsSnapshot BoundedDiagnosticsChannel::Snapshot() {
    if (snapshot_.stopped) {
        return snapshot_;
    }

    ++snapshot_.snapshot_query_count;
    return snapshot_;
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
    for (std::size_t index = 0U; index < accepted_event_id_count_; ++index) {
        if (accepted_event_ids_[index].value == eventId.value) {
            return true;
        }
    }

    return false;
}

bool BoundedDiagnosticsChannel::HasAcceptedCounterId(DiagnosticsCounterId counterId) const {
    return CounterIndex(counterId) != INVALID_INDEX;
}

std::size_t BoundedDiagnosticsChannel::CounterIndex(DiagnosticsCounterId counterId) const {
    for (std::size_t index = 0U; index < snapshot_.counter_count; ++index) {
        if (snapshot_.counters[index].id.value == counterId.value) {
            return index;
        }
    }

    return INVALID_INDEX;
}
}
