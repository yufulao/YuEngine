#pragma once

#include <cstddef>

#include "yuengine/rhi/RhiStatus.h"

namespace yuengine::rhi {
struct RhiCaptureResult final {
    RhiStatus Status = RhiStatus::InvalidLifecycle;
    std::size_t BytesWritten = 0U;
};
}
