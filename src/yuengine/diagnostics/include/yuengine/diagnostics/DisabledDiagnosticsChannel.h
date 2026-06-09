#pragma once

#include <cstdint>

#include "yuengine/diagnostics/DiagnosticsCounterId.h"
#include "yuengine/diagnostics/DiagnosticsEventId.h"
#include "yuengine/diagnostics/DiagnosticsSnapshot.h"
#include "yuengine/diagnostics/DiagnosticsStatus.h"

namespace yuengine::diagnostics
{
class DisabledDiagnosticsChannel final
{
public:
    DiagnosticsStatus RegisterEventId(DiagnosticsEventId eventId);
    DiagnosticsStatus RegisterCounterId(DiagnosticsCounterId counterId);
    DiagnosticsStatus RecordEvent(DiagnosticsEventId eventId, std::uint64_t payload);
    DiagnosticsStatus IncrementCounter(DiagnosticsCounterId counterId);
    DiagnosticsStatus AddCounter(DiagnosticsCounterId counterId, std::uint64_t delta);
    DiagnosticsStatus Shutdown();
    DiagnosticsSnapshot Snapshot() const;
};
}
