// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiPrimitiveRetirementRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveKind.h"

namespace yuengine::rhi {
struct RhiPrimitiveRetirementRequest final {
    std::uint64_t request_id = 0U;
    RhiPrimitiveKind primitive_kind = RhiPrimitiveKind::Unsupported;
    std::uint32_t primitive_slot = 0U;
    std::uint32_t primitive_generation = 0U;
    RhiFenceHandle wait_fence{};
};
}
