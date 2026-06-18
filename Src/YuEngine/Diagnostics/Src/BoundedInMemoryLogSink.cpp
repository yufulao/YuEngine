// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Src/BoundedInMemoryLogSink.cpp

#include "YuEngine/Diagnostics/BoundedInMemoryLogSink.h"

#include <string>

namespace yuengine::diagnostics {
BoundedInMemoryLogSink::BoundedInMemoryLogSink(std::size_t capacity)
    : capacity_(capacity),
      events_(),
      disabled_modules_(),
      dropped_count_(0U),
      is_enabled_(true) {
    events_.reserve(capacity_);
    disabled_modules_.reserve(capacity_);
}

void BoundedInMemoryLogSink::Write(std::string_view module_name, LogLevel level, std::string_view message) {
    if (!IsModuleEnabled(module_name)) {
        return;
    }

    if (events_.size() >= capacity_) {
        ++dropped_count_;
        return;
    }

    events_.push_back(LogEvent{std::string(module_name), level, std::string(message)});
}

void BoundedInMemoryLogSink::SetEnabled(bool enabled) {
    is_enabled_ = enabled;
}

bool BoundedInMemoryLogSink::IsEnabled() const {
    return is_enabled_;
}

bool BoundedInMemoryLogSink::SetModuleEnabled(std::string_view module_name, bool enabled) {
    if (module_name.empty()) {
        return false;
    }

    if (enabled) {
        RemoveDisabledModule(module_name);
        return true;
    }

    if (ContainsDisabledModule(module_name)) {
        return true;
    }

    if (disabled_modules_.size() >= capacity_) {
        return false;
    }

    disabled_modules_.emplace_back(module_name);
    return true;
}

bool BoundedInMemoryLogSink::IsModuleEnabled(std::string_view module_name) const {
    if (!is_enabled_) {
        return false;
    }

    if (module_name.empty()) {
        return false;
    }

    return !ContainsDisabledModule(module_name);
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

bool BoundedInMemoryLogSink::ContainsDisabledModule(std::string_view module_name) const {
    for (const std::string& disabled_module : disabled_modules_) {
        if (disabled_module == module_name) {
            return true;
        }
    }

    return false;
}

void BoundedInMemoryLogSink::RemoveDisabledModule(std::string_view module_name) {
    for (auto module_iterator = disabled_modules_.begin(); module_iterator != disabled_modules_.end(); ++module_iterator) {
        if (*module_iterator != module_name) {
            continue;
        }

        disabled_modules_.erase(module_iterator);
        return;
    }
}
}
