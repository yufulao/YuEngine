#pragma once

#include <string_view>

#include "yuengine/diagnostics/log_level.h"

namespace yuengine::diagnostics {
class ILogSink {
public:
    virtual ~ILogSink() = default;

    virtual void Write(LOG_LEVEL level, std::string_view message) = 0;
    virtual bool IsEnabled() const = 0;
};
}
