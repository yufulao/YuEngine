#include "yuengine/diagnostics/DisabledLogSink.h"

namespace yuengine::diagnostics {
void DisabledLogSink::Write(LogLevel level, std::string_view message) {
    static_cast<void>(level);
    static_cast<void>(message);
}

bool DisabledLogSink::IsEnabled() const {
    return false;
}
}
