#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsCounterId.h"
#include "YuEngine/Diagnostics/DiagnosticsEventId.h"
#include "YuEngine/Diagnostics/DiagnosticsSnapshot.h"
#include "YuEngine/Diagnostics/DiagnosticsStatus.h"

namespace yuengine::diagnostics {
class DisabledDiagnosticsChannel final {
public:
    DiagnosticsStatus RegisterEventId(DiagnosticsEventId event_id);
    DiagnosticsStatus RegisterCounterId(DiagnosticsCounterId counter_id);
    DiagnosticsStatus RecordEvent(DiagnosticsEventId event_id, std::uint64_t payload);
    DiagnosticsStatus IncrementCounter(DiagnosticsCounterId counter_id);
    DiagnosticsStatus AddCounter(DiagnosticsCounterId counter_id, std::uint64_t delta);
    DiagnosticsStatus Shutdown();
    DiagnosticsSnapshot Snapshot() const;
};
}
