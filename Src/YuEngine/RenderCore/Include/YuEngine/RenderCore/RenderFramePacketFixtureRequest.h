// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFramePacketFixtureRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureRequest.h"

namespace yuengine::rendercore {
class RenderSubmissionBatchFixture;

/**
 * @comment 描述 一个 调用方持有 RenderCore frame packet 操作.
 */
struct RenderFramePacketFixtureRequest final {
    std::uint32_t frame_id = 0U;
    RenderSubmissionBatchFixture *submission_batch = nullptr;
    const RenderSubmissionBatchFixtureRequest *batch_request = nullptr;
};
}
