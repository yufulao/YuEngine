#pragma once

#include <cstdint>
#include <vector>

#include "YuEngine/Rhi/RhiColorTargetDesc.h"

namespace yuengine::rhi {
struct RhiTargetSlot final {
    bool IsActive = false;
    std::uint32_t Generation = 1U;
    RhiColorTargetDesc Desc{};
    std::vector<std::uint8_t> Bytes{};
};
}
