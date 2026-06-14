#pragma once

#include <string_view>

#include "YuEngine/Diagnostics/LogLevel.h"

namespace yuengine::diagnostics {
class ILogSink {
public:
    virtual ~ILogSink() = default;

    virtual void Write(LogLevel level, std::string_view message) = 0;
    virtual bool IsEnabled() const = 0;
};
}
