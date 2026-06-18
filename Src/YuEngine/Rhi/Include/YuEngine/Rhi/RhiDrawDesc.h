// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDrawDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Rhi/RhiPrimitiveTopology.h"

namespace yuengine::rhi {
struct RhiDrawDesc final {
    RhiPrimitiveTopology topology = RhiPrimitiveTopology::Unsupported;
    std::uint32_t vertex_count = 0U;
    std::uint32_t first_vertex = 0U;
};
}
