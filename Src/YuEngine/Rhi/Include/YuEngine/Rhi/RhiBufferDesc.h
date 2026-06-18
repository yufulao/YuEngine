// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiBufferDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiBufferUsage.h"

namespace yuengine::rhi {
struct RhiBufferDesc final {
    RhiBufferUsage usage = RhiBufferUsage::Unsupported;
    std::size_t size_bytes = 0U;
};
}
