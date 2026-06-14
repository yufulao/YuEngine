#include "yuengine/diagnostics/disabled_log_sink.h"

namespace yuengine::diagnostics {
void DisabledLogSink::Write(LOG_LEVEL level, std::string_view message) {
    static_cast<void>(level);
    static_cast<void>(message);
}

bool DisabledLogSink::IsEnabled() const {
    return false;
}
}
