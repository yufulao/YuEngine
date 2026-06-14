#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsCounterSnapshot.h"
#include "YuEngine/Diagnostics/DiagnosticsEvent.h"
#include "YuEngine/Diagnostics/DiagnosticsLimits.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"

namespace yuengine::diagnostics {
struct DiagnosticsSnapshot {
    std::array<DiagnosticsEvent, MAX_DIAGNOSTICS_EVENTS> events;
    std::array<DiagnosticsCounterSnapshot, MAX_DIAGNOSTICS_COUNTERS> counters;
    std::size_t event_count;
    std::size_t counter_count;
    std::uint64_t accepted_event_count;
    std::uint64_t dropped_event_count;
    std::uint64_t successful_counter_update_count;
    std::uint64_t snapshot_query_count;
    bool enabled;
    bool stopped;
    memory::MemoryAccountingStatus allocation_accounting_status;
};
}
