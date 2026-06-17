// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlanStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Defines explicit RenderCore render graph execution-plan status values.
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
