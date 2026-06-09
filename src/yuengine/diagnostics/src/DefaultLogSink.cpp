#include "yuengine/diagnostics/DefaultLogSink.h"

#include <iostream>

namespace yuengine::diagnostics
{
void DefaultLogSink::Write(LogLevel level, std::string_view message)
{
    if (level == LogLevel::Error)
    {
        std::cerr << message << '\n';
        return;
    }

    std::clog << message << '\n';
}

bool DefaultLogSink::IsEnabled() const
{
    return true;
}
}
