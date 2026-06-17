// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDrawIndexedDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Rhi/RhiPrimitiveTopology.h"

namespace yuengine::rhi {
struct RhiDrawIndexedDesc final {
    RhiPrimitiveTopology topology = RhiPrimitiveTopology::Unsupported;
    std::uint32_t index_count = 0U;
    std::uint32_t first_index = 0U;
    std::int32_t vertex_offset = 0;
};
}
