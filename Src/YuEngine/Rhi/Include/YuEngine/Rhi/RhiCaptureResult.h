#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rhi {
struct RhiCaptureResult final {
    RhiStatus Status = RhiStatus::InvalidLifecycle;
    std::size_t BytesWritten = 0U;
};
}
