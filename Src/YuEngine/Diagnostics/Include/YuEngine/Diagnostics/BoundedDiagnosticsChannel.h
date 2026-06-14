#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsChannelConfig.h"
#include "YuEngine/Diagnostics/DiagnosticsCounterId.h"
#include "YuEngine/Diagnostics/DiagnosticsCounterSnapshot.h"
#include "YuEngine/Diagnostics/DiagnosticsEvent.h"
#include "YuEngine/Diagnostics/DiagnosticsEventId.h"
#include "YuEngine/Diagnostics/DiagnosticsLimits.h"
#include "YuEngine/Diagnostics/DiagnosticsSnapshot.h"
#include "YuEngine/Diagnostics/DiagnosticsStatus.h"

namespace yuengine::diagnostics {
class BoundedDiagnosticsChannel final {
public:
    explicit BoundedDiagnosticsChannel(DiagnosticsChannelConfig config);

    DiagnosticsStatus RegisterEventId(DiagnosticsEventId event_id);
    DiagnosticsStatus RegisterCounterId(DiagnosticsCounterId counter_id);
    DiagnosticsStatus RecordEvent(DiagnosticsEventId event_id, std::uint64_t payload);
    DiagnosticsStatus IncrementCounter(DiagnosticsCounterId counter_id);
    DiagnosticsStatus AddCounter(DiagnosticsCounterId counter_id, std::uint64_t delta);
    DiagnosticsStatus Shutdown();
    DiagnosticsSnapshot Snapshot();

private:
    DiagnosticsStatus ValidateConfig(DiagnosticsChannelConfig config) const;
    bool HasAcceptedEventId(DiagnosticsEventId event_id) const;
    bool HasAcceptedCounterId(DiagnosticsCounterId counter_id) const;
    std::size_t CounterIndex(DiagnosticsCounterId counter_id) const;

    DiagnosticsChannelConfig config_;
    DiagnosticsStatus configuration_status_;
    DiagnosticsSnapshot snapshot_;
    std::array<DiagnosticsEventId, MAX_DIAGNOSTICS_EVENT_IDS> accepted_event_ids_;
    std::array<DiagnosticsCounterId, MAX_DIAGNOSTICS_COUNTER_IDS> accepted_counter_ids_;
    std::size_t accepted_event_id_count_;
    std::size_t accepted_counter_id_count_;
};
}
