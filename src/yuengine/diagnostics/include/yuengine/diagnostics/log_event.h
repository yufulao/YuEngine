#pragma once

#include <string>

#include "yuengine/diagnostics/log_level.h"

namespace yuengine::diagnostics {
struct LogEvent {
    LogLevel Level;
    std::string Message;
};
}
