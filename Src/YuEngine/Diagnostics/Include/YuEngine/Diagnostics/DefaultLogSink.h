// Module: YuEngine Diagnostics
// File: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DefaultLogSink.h

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "YuEngine/Diagnostics/ILogSink.h"

namespace yuengine::diagnostics {
class DefaultLogSink final : public ILogSink {
public:
    /**
     * @comment Writes a log message.
     * @param module_name Input log module name.
     * @param level Input level.
     * @param message Input message text.
     */
    void Write(std::string_view module_name, LogLevel level, std::string_view message) override;
    /**
     * @comment Sets the global logging switch.
     * @param enabled Input enabled state.
     */
    void SetEnabled(bool enabled) override;
    /**
     * @comment Checks whether the component is enabled.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsEnabled() const override;
    /**
     * @comment Sets the logging switch for one module.
     * @param module_name Input log module name.
     * @param enabled Input enabled state.
     * @return True when the module switch is stored; false otherwise.
     */
    bool SetModuleEnabled(std::string_view module_name, bool enabled) override;
    /**
     * @comment Checks whether one log module is enabled.
     * @param module_name Input log module name.
     * @return True when the module is enabled; false otherwise.
     */
    bool IsModuleEnabled(std::string_view module_name) const override;

private:
    bool ContainsDisabledModule(std::string_view module_name) const;
    void RemoveDisabledModule(std::string_view module_name);

    std::vector<std::string> disabled_modules_;
    bool is_enabled_ = true;
};
}
