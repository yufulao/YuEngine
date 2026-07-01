// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DiagnosticsSnapshot.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsCounterSnapshot.h"
#include "YuEngine/Diagnostics/DiagnosticsEvent.h"
#include "YuEngine/Diagnostics/DiagnosticsLimits.h"
#include "YuEngine/Diagnostics/DiagnosticsStatus.h"
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
    DiagnosticsStatus last_status = DiagnosticsStatus::Success;
    std::size_t required_event_id_count = 0U;
    std::size_t required_counter_id_count = 0U;
    std::size_t required_counter_slot_count = 0U;
    std::size_t required_event_record_count = 0U;
};
}
