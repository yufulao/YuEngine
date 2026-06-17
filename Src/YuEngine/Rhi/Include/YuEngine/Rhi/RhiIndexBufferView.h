// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiIndexBufferView.h

#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"

namespace yuengine::rhi {
struct RhiIndexBufferView final {
    RhiBufferHandle buffer{};
    std::size_t offset_bytes = 0U;
    std::size_t size_bytes = 0U;
    RhiIndexFormat format = RhiIndexFormat::Unsupported;
};
}
