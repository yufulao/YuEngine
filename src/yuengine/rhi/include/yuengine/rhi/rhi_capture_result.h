#pragma once

#include <cstddef>

#include "yuengine/rhi/rhi_status.h"

namespace yuengine::rhi {
struct RhiCaptureResult final {
    RHI_STATUS Status = RHI_STATUS::InvalidLifecycle;
    std::size_t BytesWritten = 0U;
};
}
