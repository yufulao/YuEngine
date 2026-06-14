#pragma once

#include <string>

#include "yuengine/diagnostics/log_level.h"

namespace yuengine::diagnostics {
struct LogEvent {
    LOG_LEVEL Level;
    std::string Message;
};
}
