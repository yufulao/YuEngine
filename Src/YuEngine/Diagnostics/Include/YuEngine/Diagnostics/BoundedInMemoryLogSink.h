// Module: YuEngine Diagnostics
// File: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/BoundedInMemoryLogSink.h

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "YuEngine/Diagnostics/ILogSink.h"
#include "YuEngine/Diagnostics/LogEvent.h"

namespace yuengine::diagnostics {
class BoundedInMemoryLogSink final : public ILogSink {
public:
    /**
     * @comment Constructs a BoundedInMemoryLogSink instance.
     * @param capacity Input capacity.
     */
    explicit BoundedInMemoryLogSink(std::size_t capacity);

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

    /**
     * @comment Returns recorded log events.
     * @return Reference to the requested object.
     */
    const std::vector<LogEvent>& Events() const;
    /**
     * @comment Returns the number of dropped records.
     * @return Dropped count value.
     */
    std::size_t DroppedCount() const;
    /**
     * @comment Clears stored state.
     */
    void Clear();

private:
    bool ContainsDisabledModule(std::string_view module_name) const;
    void RemoveDisabledModule(std::string_view module_name);

    std::size_t capacity_;
    std::vector<LogEvent> events_;
    std::vector<std::string> disabled_modules_;
    std::size_t dropped_count_;
    bool is_enabled_;
};
}
