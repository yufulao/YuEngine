// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixtureResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/MaterialBindingFixtureStatus.h"
#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Contains the result of one material binding fixture operation.
 */
struct MaterialBindingFixtureResult final {
    MaterialBindingFixtureStatus status = MaterialBindingFixtureStatus::InvalidArgument;
    RenderFixturePassStatus pass_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    std::uint32_t material_id = 0U;
    std::uint32_t pass_id = 0U;
    std::size_t constant_byte_count = 0U;
};
}
