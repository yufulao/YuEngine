#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rhi {
struct RhiCaptureResult final {
    RhiStatus status = RhiStatus::InvalidLifecycle;
    std::size_t bytes_written = 0U;
};
}
