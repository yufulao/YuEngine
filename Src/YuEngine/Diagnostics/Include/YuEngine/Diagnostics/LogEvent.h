#pragma once

#include <string>

#include "YuEngine/Diagnostics/LogLevel.h"

namespace yuengine::diagnostics {
struct LogEvent {
    LogLevel Level;
    std::string Message;
};
}
