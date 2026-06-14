#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "yuengine/diagnostics/DiagnosticsCounterSnapshot.h"
#include "yuengine/diagnostics/DiagnosticsEvent.h"
#include "yuengine/diagnostics/DiagnosticsLimits.h"
#include "yuengine/memory/MemoryAccountingStatus.h"

namespace yuengine::diagnostics {
struct DiagnosticsSnapshot {
    std::array<DiagnosticsEvent, MAX_DIAGNOSTICS_EVENTS> Events;
    std::array<DiagnosticsCounterSnapshot, MAX_DIAGNOSTICS_COUNTERS> Counters;
    std::size_t EventCount;
    std::size_t CounterCount;
    std::uint64_t AcceptedEventCount;
    std::uint64_t DroppedEventCount;
    std::uint64_t SuccessfulCounterUpdateCount;
    std::uint64_t SnapshotQueryCount;
    bool Enabled;
    bool Stopped;
    memory::MemoryAccountingStatus AllocationAccountingStatus;
};
}
