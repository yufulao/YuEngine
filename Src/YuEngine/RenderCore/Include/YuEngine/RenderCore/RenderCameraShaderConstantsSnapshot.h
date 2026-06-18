// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraShaderConstantsSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderCameraShaderConstantsStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Contains camera shader constant writer counters and last status values.
 */
struct RenderCameraShaderConstantsSnapshot final {
    std::uint64_t accepted_write_count = 0U;
    std::uint64_t rejected_write_count = 0U;
    RenderCameraShaderConstantsStatus last_status = RenderCameraShaderConstantsStatus::InvalidArgument;
};
}
