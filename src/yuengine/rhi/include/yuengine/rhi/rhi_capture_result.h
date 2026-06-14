#pragma once

#include <cstddef>

#include "yuengine/rhi/rhi_status.h"

namespace yuengine::rhi {
struct RhiCaptureResult final {
    RhiStatus Status = RhiStatus::InvalidLifecycle;
    std::size_t BytesWritten = 0U;
};
}
