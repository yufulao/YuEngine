#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "yuengine/diagnostics/diagnostics_counter_snapshot.h"
#include "yuengine/diagnostics/diagnostics_event.h"
#include "yuengine/diagnostics/diagnostics_limits.h"
#include "yuengine/memory/memory_accounting_status.h"

namespace yuengine::diagnostics {
struct diagnostics_snapshot_t {
    std::array<diagnostics_event_t, MAX_DIAGNOSTICS_EVENTS> Events;
    std::array<diagnostics_counter_snapshot_t, MAX_DIAGNOSTICS_COUNTERS> Counters;
    std::size_t EventCount;
    std::size_t CounterCount;
    std::uint64_t AcceptedEventCount;
    std::uint64_t DroppedEventCount;
    std::uint64_t SuccessfulCounterUpdateCount;
    std::uint64_t SnapshotQueryCount;
    bool Enabled;
    bool Stopped;
    memory::MEMORY_ACCOUNTING_STATUS AllocationAccountingStatus;
};
}
