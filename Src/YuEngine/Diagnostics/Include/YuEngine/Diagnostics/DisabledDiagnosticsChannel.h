// Module: YuEngine Diagnostics
// File: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DisabledDiagnosticsChannel.h

#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsCounterId.h"
#include "YuEngine/Diagnostics/DiagnosticsEventId.h"
#include "YuEngine/Diagnostics/DiagnosticsSnapshot.h"
#include "YuEngine/Diagnostics/DiagnosticsStatus.h"

namespace yuengine::diagnostics {
class DisabledDiagnosticsChannel final {
public:
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
    DiagnosticsSnapshot Snapshot() const;
};
}
