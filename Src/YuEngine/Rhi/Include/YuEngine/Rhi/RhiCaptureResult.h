// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCaptureResult.h

#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
struct RhiCaptureResult final {
    RhiStatus status = RhiStatus::InvalidLifecycle;
    std::size_t bytes_written = 0U;
    RhiExtent2D extent{};
    std::size_t capture_byte_capacity = 0U;
    std::size_t current_byte_count = 0U;
    std::size_t required_byte_count = 0U;
    RhiTextureHandle target{};
};
}
