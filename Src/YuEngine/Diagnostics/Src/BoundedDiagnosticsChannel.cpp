// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Src/BoundedDiagnosticsChannel.cpp

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
    snapshot_.last_status = configuration_status_;
}

DiagnosticsStatus BoundedDiagnosticsChannel::RegisterEventId(DiagnosticsEventId event_id) {
    if (configuration_status_ != DiagnosticsStatus::Success) {
        snapshot_.last_status = configuration_status_;
        return configuration_status_;
    }

    if (snapshot_.stopped) {
        snapshot_.last_status = DiagnosticsStatus::Stopped;
        return DiagnosticsStatus::Stopped;
    }

    if (!event_id.IsValid()) {
        snapshot_.last_status = DiagnosticsStatus::UnknownEventId;
        return DiagnosticsStatus::UnknownEventId;
    }

    if (HasAcceptedEventId(event_id)) {
        snapshot_.last_status = DiagnosticsStatus::Success;
        return DiagnosticsStatus::Success;
    }

    if (accepted_event_id_count_ >= config_.accepted_event_id_capacity) {
        snapshot_.required_event_id_count = accepted_event_id_count_ + 1U;
        snapshot_.last_status = DiagnosticsStatus::CapacityExceeded;
        return DiagnosticsStatus::CapacityExceeded;
    }

    accepted_event_ids_[accepted_event_id_count_] = event_id;
    ++accepted_event_id_count_;
    snapshot_.last_status = DiagnosticsStatus::Success;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::RegisterCounterId(DiagnosticsCounterId counter_id) {
    if (configuration_status_ != DiagnosticsStatus::Success) {
        snapshot_.last_status = configuration_status_;
        return configuration_status_;
    }

    if (snapshot_.stopped) {
        snapshot_.last_status = DiagnosticsStatus::Stopped;
        return DiagnosticsStatus::Stopped;
    }

    if (!counter_id.IsValid()) {
        snapshot_.last_status = DiagnosticsStatus::UnknownCounterId;
        return DiagnosticsStatus::UnknownCounterId;
    }

    if (HasAcceptedCounterId(counter_id)) {
        snapshot_.last_status = DiagnosticsStatus::Success;
        return DiagnosticsStatus::Success;
    }

    if (accepted_counter_id_count_ >= config_.accepted_counter_id_capacity) {
        snapshot_.required_counter_id_count = accepted_counter_id_count_ + 1U;
        snapshot_.last_status = DiagnosticsStatus::CapacityExceeded;
        return DiagnosticsStatus::CapacityExceeded;
    }

    if (snapshot_.counter_count >= config_.counter_capacity) {
        snapshot_.required_counter_slot_count = snapshot_.counter_count + 1U;
        snapshot_.last_status = DiagnosticsStatus::CapacityExceeded;
        return DiagnosticsStatus::CapacityExceeded;
    }

    accepted_counter_ids_[accepted_counter_id_count_] = counter_id;
    ++accepted_counter_id_count_;
    snapshot_.counters[snapshot_.counter_count] = DiagnosticsCounterSnapshot{counter_id, 0U, 0U};
    ++snapshot_.counter_count;
    snapshot_.last_status = DiagnosticsStatus::Success;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::RecordEvent(DiagnosticsEventId event_id, std::uint64_t payload) {
    if (configuration_status_ != DiagnosticsStatus::Success) {
        snapshot_.last_status = configuration_status_;
        return configuration_status_;
    }

    if (snapshot_.stopped) {
        snapshot_.last_status = DiagnosticsStatus::Stopped;
        return DiagnosticsStatus::Stopped;
    }

    if (config_.validate_ids && !HasAcceptedEventId(event_id)) {
        snapshot_.last_status = DiagnosticsStatus::UnknownEventId;
        return DiagnosticsStatus::UnknownEventId;
    }

    if (snapshot_.event_count >= config_.event_capacity) {
        ++snapshot_.dropped_event_count;
        snapshot_.required_event_record_count = snapshot_.event_count + 1U;
        snapshot_.last_status = DiagnosticsStatus::Dropped;
        return DiagnosticsStatus::Dropped;
    }

    snapshot_.events[snapshot_.event_count] = DiagnosticsEvent{event_id, payload};
    ++snapshot_.event_count;
    ++snapshot_.accepted_event_count;
    snapshot_.last_status = DiagnosticsStatus::Success;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::IncrementCounter(DiagnosticsCounterId counter_id) {
    return AddCounter(counter_id, 1U);
}

DiagnosticsStatus BoundedDiagnosticsChannel::AddCounter(DiagnosticsCounterId counter_id, std::uint64_t delta) {
    if (configuration_status_ != DiagnosticsStatus::Success) {
        snapshot_.last_status = configuration_status_;
        return configuration_status_;
    }

    if (snapshot_.stopped) {
        snapshot_.last_status = DiagnosticsStatus::Stopped;
        return DiagnosticsStatus::Stopped;
    }

    const std::size_t counter_index = CounterIndex(counter_id);
    if (counter_index == INVALID_INDEX) {
        snapshot_.last_status = DiagnosticsStatus::UnknownCounterId;
        return DiagnosticsStatus::UnknownCounterId;
    }

    DiagnosticsCounterSnapshot& counter = snapshot_.counters[counter_index];
    const std::uint64_t max_value = std::numeric_limits<std::uint64_t>::max();
    if (counter.value > max_value - delta) {
        snapshot_.last_status = DiagnosticsStatus::CounterOverflow;
        return DiagnosticsStatus::CounterOverflow;
    }

    counter.value += delta;
    ++counter.successful_update_count;
    ++snapshot_.successful_counter_update_count;
    snapshot_.last_status = DiagnosticsStatus::Success;
    return DiagnosticsStatus::Success;
}

DiagnosticsStatus BoundedDiagnosticsChannel::Shutdown() {
    if (configuration_status_ != DiagnosticsStatus::Success) {
        snapshot_.last_status = configuration_status_;
        return configuration_status_;
    }

    snapshot_.stopped = true;
    snapshot_.last_status = DiagnosticsStatus::Success;
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

bool BoundedDiagnosticsChannel::HasAcceptedEventId(DiagnosticsEventId event_id) const {
    for (std::size_t index = 0U; index < accepted_event_id_count_; ++index) {
        if (accepted_event_ids_[index].value == event_id.value) {
            return true;
        }
    }

    return false;
}

bool BoundedDiagnosticsChannel::HasAcceptedCounterId(DiagnosticsCounterId counter_id) const {
    return CounterIndex(counter_id) != INVALID_INDEX;
}

std::size_t BoundedDiagnosticsChannel::CounterIndex(DiagnosticsCounterId counter_id) const {
    for (std::size_t index = 0U; index < snapshot_.counter_count; ++index) {
        if (snapshot_.counters[index].id.value == counter_id.value) {
            return index;
        }
    }

    return INVALID_INDEX;
}
}
