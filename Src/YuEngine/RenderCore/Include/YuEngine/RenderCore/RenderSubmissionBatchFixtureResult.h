// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSubmissionBatchFixtureResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 结果 的 一个 RenderCore submission batch 操作。
 */
struct RenderSubmissionBatchFixtureResult final {
    RenderSubmissionBatchFixtureStatus status = RenderSubmissionBatchFixtureStatus::InvalidArgument;
    RenderFixturePassStatus pass_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    std::size_t entry_count = 0U;
    std::size_t completed_entry_count = 0U;
    std::size_t required_submission_record_count = 0U;
    std::size_t failed_entry_index = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t material_id = 0U;
};
}
