#pragma once

#include <string>

#include "yuengine/diagnostics/log_level.h"

namespace yuengine::diagnostics {
struct log_event_t {
    LOG_LEVEL Level;
    std::string Message;
};
}
