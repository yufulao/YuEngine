// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonOperation.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureRequest.h"

namespace yuengine::rendercore {
/**
 * @comment Contains the result of one RenderCore render graph skeleton operation.
 */
struct RenderGraphSkeletonResult final {
    RenderGraphSkeletonStatus status = RenderGraphSkeletonStatus::InvalidArgument;
    RenderGraphSkeletonOperation operation = RenderGraphSkeletonOperation::None;
    RenderFixturePassStatus pass_status = RenderFixturePassStatus::InvalidArgument;
    RenderSubmissionBatchFixtureRequest submission_batch_request{};
    std::uint32_t graph_id = 0U;
    std::size_t pass_count = 0U;
    std::size_t dependency_count = 0U;
    std::size_t failed_pass_index = 0U;
    std::size_t failed_dependency_index = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t dependency_before_pass_id = 0U;
    std::uint32_t dependency_after_pass_id = 0U;
};
}
