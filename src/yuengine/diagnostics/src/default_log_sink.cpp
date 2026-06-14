#include "yuengine/diagnostics/default_log_sink.h"

#include <cstddef>
#include <cstdio>

namespace yuengine::diagnostics {
namespace {
void writeMessage(std::FILE *stream, std::string_view message) {
    if (stream == nullptr) {
        return;
    }

    const std::size_t byte_count = message.size();
    if (byte_count > 0U) {
        const char *data = message.data();
        if (data == nullptr) {
            return;
        }

        const std::size_t write_result = std::fwrite(data, sizeof(char), byte_count, stream);
        static_cast<void>(write_result);
    }

    const int line_result = std::fputc('\n', stream);
    static_cast<void>(line_result);
}
}

void DefaultLogSink::Write(LogLevel level, std::string_view message) {
    if (level == LogLevel::Error) {
        writeMessage(stderr, message);
        return;
    }

    writeMessage(stderr, message);
}

bool DefaultLogSink::IsEnabled() const {
    return true;
}
}
