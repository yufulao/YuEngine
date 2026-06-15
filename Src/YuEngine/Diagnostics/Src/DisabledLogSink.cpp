// Module: YuEngine Diagnostics
// File: Src/YuEngine/Diagnostics/Src/DisabledLogSink.cpp

#include "YuEngine/Diagnostics/DisabledLogSink.h"

namespace yuengine::diagnostics {
void DisabledLogSink::Write(std::string_view module_name, LogLevel level, std::string_view message) {
    static_cast<void>(module_name);
    static_cast<void>(level);
    static_cast<void>(message);
}

void DisabledLogSink::SetEnabled(bool enabled) {
    static_cast<void>(enabled);
}

bool DisabledLogSink::IsEnabled() const {
    return false;
}

bool DisabledLogSink::SetModuleEnabled(std::string_view module_name, bool enabled) {
    static_cast<void>(module_name);
    static_cast<void>(enabled);
    return false;
}

bool DisabledLogSink::IsModuleEnabled(std::string_view module_name) const {
    static_cast<void>(module_name);
    return false;
}
}
