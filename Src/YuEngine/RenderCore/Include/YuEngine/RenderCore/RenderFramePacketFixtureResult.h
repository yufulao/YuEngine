// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFramePacketFixtureResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Contains the result of one RenderCore frame packet operation.
 */
struct RenderFramePacketFixtureResult final {
    RenderFramePacketFixtureStatus status = RenderFramePacketFixtureStatus::InvalidArgument;
    RenderSubmissionBatchFixtureStatus batch_status = RenderSubmissionBatchFixtureStatus::InvalidArgument;
    RenderFixturePassStatus pass_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    std::uint32_t frame_id = 0U;
    std::size_t entry_count = 0U;
    std::size_t completed_entry_count = 0U;
    std::size_t failed_entry_count = 0U;
    std::size_t failed_entry_index = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t material_id = 0U;
};
}
