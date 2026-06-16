// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiFenceHandle.h

#pragma once

#include <cstdint>

namespace yuengine::rhi {
struct RhiFenceHandle final {
    std::uint32_t slot = 0U;
    std::uint32_t generation = 0U;
};
}
