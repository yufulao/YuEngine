#pragma once

#include "yuengine/diagnostics/ILogSink.h"

namespace yuengine::diagnostics {
class DefaultLogSink final : public ILogSink {
public:
    void Write(LogLevel level, std::string_view message) override;
    bool IsEnabled() const override;
};
}
