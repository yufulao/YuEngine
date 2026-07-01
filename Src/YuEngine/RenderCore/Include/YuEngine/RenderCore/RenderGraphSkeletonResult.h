// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonOperation.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureRequest.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 结果 的 一个 RenderCore render graph skeleton 操作。
 */
struct RenderGraphSkeletonResult final {
    RenderGraphSkeletonStatus status = RenderGraphSkeletonStatus::InvalidArgument;
    RenderGraphSkeletonOperation operation = RenderGraphSkeletonOperation::None;
    RenderFixturePassStatus pass_status = RenderFixturePassStatus::InvalidArgument;
    RenderSubmissionBatchFixtureRequest submission_batch_request{};
    std::uint32_t graph_id = 0U;
    std::size_t pass_count = 0U;
    std::size_t dependency_count = 0U;
    std::size_t required_pass_record_count = 0U;
    std::size_t required_dependency_record_count = 0U;
    std::size_t pass_record_capacity = 0U;
    std::size_t current_pass_record_count = 0U;
    std::size_t dependency_record_capacity = 0U;
    std::size_t current_dependency_record_count = 0U;
    std::size_t failed_pass_index = 0U;
    std::size_t failed_dependency_index = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t dependency_before_pass_id = 0U;
    std::uint32_t dependency_after_pass_id = 0U;
};
}
