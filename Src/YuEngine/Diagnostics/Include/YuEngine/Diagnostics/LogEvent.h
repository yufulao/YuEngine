// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/LogEvent.h

#pragma once

#include <string>

#include "YuEngine/Diagnostics/LogLevel.h"

namespace yuengine::diagnostics {
struct LogEvent {
    std::string module_name;
    LogLevel level;
    std::string message;
};
}
