// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Src/DefaultLogSink.cpp

#include "YuEngine/Diagnostics/DefaultLogSink.h"

#include <cstddef>
#include <cstdio>
#include <string>

namespace yuengine::diagnostics {
namespace {
void WriteText(std::FILE *stream, std::string_view text) {
    if (stream == nullptr) {
        return;
    }

    const std::size_t byte_count = text.size();
    if (byte_count > 0U) {
        const char *data = text.data();
        if (data == nullptr) {
            return;
        }

        const std::size_t write_result = std::fwrite(data, sizeof(char), byte_count, stream);
        static_cast<void>(write_result);
    }
}

void WriteMessage(std::FILE *stream, std::string_view module_name, std::string_view message) {
    WriteText(stream, "[");
    WriteText(stream, module_name);
    WriteText(stream, "] ");
    WriteText(stream, message);

    const int line_result = std::fputc('\n', stream);
    static_cast<void>(line_result);
}
}

void DefaultLogSink::Write(std::string_view module_name, LogLevel level, std::string_view message) {
    if (!IsModuleEnabled(module_name)) {
        return;
    }

    if (level == LogLevel::Error) {
        WriteMessage(stderr, module_name, message);
        return;
    }

    WriteMessage(stderr, module_name, message);
}

void DefaultLogSink::SetEnabled(bool enabled) {
    is_enabled_ = enabled;
}

bool DefaultLogSink::IsEnabled() const {
    return is_enabled_;
}

bool DefaultLogSink::SetModuleEnabled(std::string_view module_name, bool enabled) {
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

    disabled_modules_.emplace_back(module_name);
    return true;
}

bool DefaultLogSink::IsModuleEnabled(std::string_view module_name) const {
    if (!is_enabled_) {
        return false;
    }

    if (module_name.empty()) {
        return false;
    }

    return !ContainsDisabledModule(module_name);
}

bool DefaultLogSink::ContainsDisabledModule(std::string_view module_name) const {
    for (const std::string& disabled_module : disabled_modules_) {
        if (disabled_module == module_name) {
            return true;
        }
    }

    return false;
}

void DefaultLogSink::RemoveDisabledModule(std::string_view module_name) {
    for (auto module_iterator = disabled_modules_.begin(); module_iterator != disabled_modules_.end(); ++module_iterator) {
        if (*module_iterator != module_name) {
            continue;
        }

        disabled_modules_.erase(module_iterator);
        return;
    }
}
}
