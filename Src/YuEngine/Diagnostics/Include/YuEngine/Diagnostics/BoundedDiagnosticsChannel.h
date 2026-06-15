// Module: YuEngine Diagnostics
// File: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/BoundedDiagnosticsChannel.h

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
    /**
     * @comment Constructs a BoundedDiagnosticsChannel instance.
     * @param config Input configuration.
     */
    explicit BoundedDiagnosticsChannel(DiagnosticsChannelConfig config);

    /**
     * @comment Registers event id.
     * @param event_id Input event id.
     * @return Explicit operation status.
     */
    DiagnosticsStatus RegisterEventId(DiagnosticsEventId event_id);
    /**
     * @comment Registers counter id.
     * @param counter_id Input counter id.
     * @return Explicit operation status.
     */
    DiagnosticsStatus RegisterCounterId(DiagnosticsCounterId counter_id);
    /**
     * @comment Records event.
     * @param event_id Input event id.
     * @param payload Input payload.
     * @return Explicit operation status.
     */
    DiagnosticsStatus RecordEvent(DiagnosticsEventId event_id, std::uint64_t payload);
    /**
     * @comment Increments counter.
     * @param counter_id Input counter id.
     * @return Explicit operation status.
     */
    DiagnosticsStatus IncrementCounter(DiagnosticsCounterId counter_id);
    /**
     * @comment Adds counter.
     * @param counter_id Input counter id.
     * @param delta Input delta.
     * @return Explicit operation status.
     */
    DiagnosticsStatus AddCounter(DiagnosticsCounterId counter_id, std::uint64_t delta);
    /**
     * @comment Shuts down the component.
     * @return Explicit operation status.
     */
    DiagnosticsStatus Shutdown();
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
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
