// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiVertexBufferView.h

#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiBufferHandle.h"

namespace yuengine::rhi {
struct RhiVertexBufferView final {
    RhiBufferHandle buffer{};
    std::size_t offset_bytes = 0U;
    std::size_t stride_bytes = 0U;
    std::size_t size_bytes = 0U;
};
}
