#include "YuEngine/Diagnostics/BoundedInMemoryLogSink.h"

#include <string>

namespace yuengine::diagnostics {
BoundedInMemoryLogSink::BoundedInMemoryLogSink(std::size_t capacity)
    : capacity_(capacity),
      events_(),
      dropped_count_(0U) {
    events_.reserve(capacity_);
}

void BoundedInMemoryLogSink::Write(LogLevel level, std::string_view message) {
    if (events_.size() >= capacity_) {
        ++dropped_count_;
        return;
    }

    events_.push_back(LogEvent{level, std::string(message)});
}

bool BoundedInMemoryLogSink::IsEnabled() const {
    return true;
}

const std::vector<LogEvent>& BoundedInMemoryLogSink::Events() const {
    return events_;
}

std::size_t BoundedInMemoryLogSink::DroppedCount() const {
    return dropped_count_;
}

void BoundedInMemoryLogSink::Clear() {
    events_.clear();
    dropped_count_ = 0U;
}
}
