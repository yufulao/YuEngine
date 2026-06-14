#pragma once

#include "yuengine/diagnostics/i_log_sink.h"

namespace yuengine::diagnostics {
class DefaultLogSink final : public ILogSink {
public:
    void Write(LOG_LEVEL level, std::string_view message) override;
    bool IsEnabled() const override;
};
}
