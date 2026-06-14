#pragma once

#include <string>

#include "yuengine/diagnostics/LogLevel.h"

namespace yuengine::diagnostics {
struct LogEvent {
    LogLevel Level;
    std::string Message;
};
}
