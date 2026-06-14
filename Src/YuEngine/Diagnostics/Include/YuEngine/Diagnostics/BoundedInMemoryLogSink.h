#pragma once

#include <cstddef>
#include <vector>

#include "YuEngine/Diagnostics/ILogSink.h"
#include "YuEngine/Diagnostics/LogEvent.h"

namespace yuengine::diagnostics {
class BoundedInMemoryLogSink final : public ILogSink {
public:
    explicit BoundedInMemoryLogSink(std::size_t capacity);

    void Write(LogLevel level, std::string_view message) override;
    bool IsEnabled() const override;

    const std::vector<LogEvent>& Events() const;
    std::size_t DroppedCount() const;
    void Clear();

private:
    std::size_t _capacity;
    std::vector<LogEvent> _events;
    std::size_t _droppedCount;
};
}
