// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlanRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderGraphSkeletonResult.h"

namespace yuengine::rendercore {
class RenderFramePacketFixture;
class RenderSubmissionBatchFixture;

/**
 * @comment 描述 一个 调用方持有 RenderCore render graph execution-plan 操作.
 */
struct RenderGraphExecutionPlanRequest final {
    std::uint32_t plan_id = 0U;
    std::uint32_t frame_id = 0U;
    const RenderGraphSkeletonResult *prepared_graph_result = nullptr;
    RenderFramePacketFixture *frame_packet = nullptr;
    RenderSubmissionBatchFixture *submission_batch = nullptr;
};
}
