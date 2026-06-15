// Module: YuEngine Diagnostics
// File: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/ILogSink.h

#pragma once

#include <string_view>

#include "YuEngine/Diagnostics/LogLevel.h"

namespace yuengine::diagnostics {
class ILogSink {
public:
    virtual ~ILogSink() = default;

    /**
     * @comment Writes a log message.
     * @param module_name Input log module name.
     * @param level Input level.
     * @param message Input message text.
     */
    virtual void Write(std::string_view module_name, LogLevel level, std::string_view message) = 0;
    /**
     * @comment Sets the global logging switch.
     * @param enabled Input enabled state.
     */
    virtual void SetEnabled(bool enabled) = 0;
    /**
     * @comment Checks whether the component is enabled.
     * @return True when the condition is satisfied; false otherwise.
     */
    virtual bool IsEnabled() const = 0;
    /**
     * @comment Sets the logging switch for one module.
     * @param module_name Input log module name.
     * @param enabled Input enabled state.
     * @return True when the module switch is stored; false otherwise.
     */
    virtual bool SetModuleEnabled(std::string_view module_name, bool enabled) = 0;
    /**
     * @comment Checks whether one log module is enabled.
     * @param module_name Input log module name.
     * @return True when the module is enabled; false otherwise.
     */
    virtual bool IsModuleEnabled(std::string_view module_name) const = 0;
};
}
