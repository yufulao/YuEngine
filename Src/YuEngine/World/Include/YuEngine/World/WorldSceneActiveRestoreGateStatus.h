// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneActiveRestoreGateStatus.h

#pragma once

namespace yuengine::world {
enum class WorldSceneActiveRestoreGateStatus {
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
    InvalidProofScratch,
    InvalidSliceScratch,
    InvalidGateOutput,
    PlanScratchCapacityExceeded,
    ProofScratchCapacityExceeded,
    SliceScratchCapacityExceeded,
    GateOutputCapacityExceeded,
    IdentityCapacityExceeded,
    TransformCapacityExceeded,
    AttachmentCapacityExceeded,
    BindingCapacityExceeded,
    DestinationNotEmpty,
    ObjectPreflightFailed,
    ResourcePreflightFailed,
    ProofFailed,
    InvalidProofSlice,
    InvalidCleanupPolicy
};
}
