#include "yuengine/diagnostics/BoundedInMemoryLogSink.h"

#include <string>

namespace yuengine::diagnostics {
BoundedInMemoryLogSink::BoundedInMemoryLogSink(std::size_t capacity)
    : _capacity(capacity),
      _events(),
      _droppedCount(0U) {
    _events.reserve(_capacity);
}

void BoundedInMemoryLogSink::Write(LogLevel level, std::string_view message) {
    if (_events.size() >= _capacity) {
        ++_droppedCount;
        return;
    }

    _events.push_back(LogEvent{level, std::string(message)});
}

bool BoundedInMemoryLogSink::IsEnabled() const {
    return true;
}

const std::vector<LogEvent>& BoundedInMemoryLogSink::Events() const {
    return _events;
}

std::size_t BoundedInMemoryLogSink::DroppedCount() const {
    return _droppedCount;
}

void BoundedInMemoryLogSink::Clear() {
    _events.clear();
    _droppedCount = 0U;
}
}
