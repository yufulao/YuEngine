// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlanStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 定义 explicit RenderCore render graph execution-plan 状态 值.
 */
enum class RenderGraphExecutionPlanStatus {
    Success,
    InvalidArgument,
    InvalidPlanId,
    InvalidGraphId,
    InvalidFrameId,
    InvalidFrameExecutor,
    InvalidSubmissionExecutor,
    FailedSkeletonPrepare,
    MissingPreparedBatchRequest,
    MissingPassResultStorage,
    EmptyPreparedBatch,
    InvalidPreparedBatch,
    DuplicatePlanId,
    DuplicateGraphExecution,
    PlanCapacityExceeded,
    FramePacketCapacityExceeded,
    SubmissionBatchCapacityExceeded,
    FrameExecutionFailed,
    PlanRecordNotFound
};
}
