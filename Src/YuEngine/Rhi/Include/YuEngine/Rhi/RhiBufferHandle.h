// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiBufferHandle.h

#pragma once

#include <cstdint>

namespace yuengine::rhi {
struct RhiBufferHandle final {
    std::uint32_t slot = 0U;
    std::uint32_t generation = 0U;
};
}
