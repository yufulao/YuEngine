// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFramePacketFixtureRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureRequest.h"

namespace yuengine::rendercore {
class RenderSubmissionBatchFixture;

/**
 * @comment Describes one caller-owned RenderCore frame packet operation.
 */
struct RenderFramePacketFixtureRequest final {
    std::uint32_t frame_id = 0U;
    RenderSubmissionBatchFixture *submission_batch = nullptr;
    const RenderSubmissionBatchFixtureRequest *batch_request = nullptr;
};
}
