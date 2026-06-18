// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 定义 explicit RenderCore submission batch fixture 状态 值.
 */
enum class RenderSubmissionBatchFixtureStatus {
    Success,
    InvalidArgument,
    EmptyBatch,
    InvalidRequestStorage,
    InvalidResultStorage,
    InvalidPassRequest,
    DuplicatePassId,
    BatchCapacityExceeded,
    RenderFixturePassFailed
};
}
