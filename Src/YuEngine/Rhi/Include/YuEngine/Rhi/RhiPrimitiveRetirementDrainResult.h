// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiPrimitiveRetirementDrainResult.h

#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiPrimitiveRetirementStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rhi {
struct RhiPrimitiveRetirementDrainResult final {
    RhiStatus status = RhiStatus::Success;
    std::size_t drained_count = 0U;
    std::size_t pending_count = 0U;
    std::size_t rejected_count = 0U;
    RhiPrimitiveRetirementStatus last_rejection_status = RhiPrimitiveRetirementStatus::Invalid;
};
}
