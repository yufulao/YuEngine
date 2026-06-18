// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneApplyTimeRestoreProofStatus.h

#pragma once

namespace yuengine::world {
enum class WorldSceneApplyTimeRestoreProofStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidWorld,
    InvalidObjectRegistry,
    InvalidResourceRegistry,
    InvalidIdentityDestination,
    InvalidTransformDestination,
    InvalidAttachmentDestination,
    InvalidBindingDestination,
    InvalidIdentityInput,
    InvalidTransformInput,
    InvalidAttachmentInput,
    InvalidBindingInput,
    InvalidPlanScratch,
    InvalidProofOutput,
    InvalidSliceOutput,
    PlanScratchCapacityExceeded,
    ProofOutputCapacityExceeded,
    SliceOutputCapacityExceeded,
    IdentityCapacityExceeded,
    TransformCapacityExceeded,
    AttachmentCapacityExceeded,
    BindingCapacityExceeded,
    DestinationNotEmpty,
    DestinationCapacityExceeded,
    ObjectAcquireFailed,
    ResourceAcquireFailed,
    PlanFailed,
    PlanInputMismatch,
    InvalidPlanRecord,
    UnexpectedPlanFamily
};
}
