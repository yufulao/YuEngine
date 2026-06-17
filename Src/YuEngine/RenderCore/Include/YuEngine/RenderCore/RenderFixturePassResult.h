// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePassResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Contains the result of one fixture pass execution.
 */
struct RenderFixturePassResult final {
    RenderFixturePassStatus status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    std::size_t recorded_command_count = 0U;
    std::size_t capture_bytes_written = 0U;
    std::uint32_t pass_id = 0U;
};
}
