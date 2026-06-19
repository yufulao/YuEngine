// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldSceneActiveRestoreGateBridge.cpp

#include "YuEngine/World/WorldSceneActiveRestoreGateBridge.h"

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofBridge.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofBridgeDesc.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofResult.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::object::ObjectStatus;
using yuengine::resource::ResourceStatus;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

WorldSceneActiveRestoreGateBridge::WorldSceneActiveRestoreGateBridge(
    WorldSceneActiveRestoreGateDesc desc)
    : identity_capacity_(ClampCapacity(desc.identity_capacity, MAX_WORLD_OBJECT_COUNT)),
      transform_capacity_(ClampCapacity(desc.transform_capacity, MAX_WORLD_OBJECT_COUNT)),
      attachment_capacity_(ClampCapacity(desc.attachment_capacity, MAX_WORLD_OBJECT_COUNT)),
      binding_capacity_(ClampCapacity(desc.binding_capacity, MAX_WORLD_OBJECT_COUNT)),
      plan_scratch_capacity_(ClampCapacity(
          desc.plan_scratch_capacity,
          MAX_WORLD_SCENE_DECODED_RESTORE_PLAN_RECORD_COUNT)),
      proof_scratch_capacity_(ClampCapacity(
          desc.proof_scratch_capacity,
          MAX_WORLD_SCENE_APPLY_TIME_RESTORE_PROOF_RECORD_COUNT)),
      slice_scratch_capacity_(ClampCapacity(
          desc.slice_scratch_capacity,
          MAX_WORLD_SCENE_APPLY_TIME_RESTORE_SLICE_RECORD_COUNT)),
      gate_output_capacity_(ClampCapacity(
          desc.gate_output_capacity,
          MAX_WORLD_SCENE_APPLY_TIME_RESTORE_SLICE_RECORD_COUNT)),
      snapshot_{
          ClampCapacity(desc.identity_capacity, MAX_WORLD_OBJECT_COUNT),
          ClampCapacity(desc.transform_capacity, MAX_WORLD_OBJECT_COUNT),
          ClampCapacity(desc.attachment_capacity, MAX_WORLD_OBJECT_COUNT),
          ClampCapacity(desc.binding_capacity, MAX_WORLD_OBJECT_COUNT),
          ClampCapacity(
              desc.plan_scratch_capacity,
              MAX_WORLD_SCENE_DECODED_RESTORE_PLAN_RECORD_COUNT),
          ClampCapacity(
              desc.proof_scratch_capacity,
              MAX_WORLD_SCENE_APPLY_TIME_RESTORE_PROOF_RECORD_COUNT),
          ClampCapacity(
              desc.slice_scratch_capacity,
              MAX_WORLD_SCENE_APPLY_TIME_RESTORE_SLICE_RECORD_COUNT),
          ClampCapacity(
              desc.gate_output_capacity,
              MAX_WORLD_SCENE_APPLY_TIME_RESTORE_SLICE_RECORD_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          WorldSceneApplyTimeRestoreProofStatus::Success,
          WorldSceneActiveRestoreGateStatus::Success} {
    if ((desc.identity_capacity == 0U) ||
        (desc.transform_capacity == 0U) ||
        (desc.attachment_capacity == 0U) ||
        (desc.binding_capacity == 0U) ||
        (desc.plan_scratch_capacity == 0U) ||
        (desc.proof_scratch_capacity == 0U) ||
        (desc.slice_scratch_capacity == 0U) ||
        (desc.gate_output_capacity == 0U)) {
        snapshot_.last_status = WorldSceneActiveRestoreGateStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldSceneActiveRestoreGateResult WorldSceneActiveRestoreGateBridge::BuildGate(
    const WorldInstance *world,
    const yuengine::object::ObjectRegistry *object_registry,
    const yuengine::resource::ResourceRegistry *resource_registry,
    const WorldObjectIdentityBridge *identity_destination,
    const WorldTransformBridge *transform_destination,
    const WorldComponentAttachmentBridge *attachment_destination,
    const WorldComponentResourceBindingBridge *binding_destination,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count,
    WorldSceneDecodedRestorePlanRecord *plan_scratch,
    std::uint32_t plan_scratch_capacity,
    WorldSceneApplyTimeRestoreProofRecord *proof_scratch,
    std::uint32_t proof_scratch_capacity,
    WorldSceneApplyTimeRestoreProofSliceRecord *slice_scratch,
    std::uint32_t slice_scratch_capacity,
    WorldSceneActiveRestoreGateRecord *output_gates,
    std::uint32_t output_gate_capacity) {
    ++snapshot_.gate_attempt_count;

    const WorldSceneActiveRestoreGateStatus bridge_status = ValidateBridgeCapacity();
    if (bridge_status != WorldSceneActiveRestoreGateStatus::Success) {
        return RecordFailure(bridge_status);
    }

    const WorldSceneActiveRestoreGateStatus input_status = ValidateInputs(
        world,
        object_registry,
        resource_registry,
        identity_destination,
        transform_destination,
        attachment_destination,
        binding_destination,
        input_identities,
        input_transforms,
        input_attachments,
        input_bindings,
        plan_scratch,
        proof_scratch,
        slice_scratch,
        output_gates);
    if (input_status != WorldSceneActiveRestoreGateStatus::Success) {
        return RecordFailure(input_status);
    }

    std::uint32_t record_count = 0U;
    const WorldSceneActiveRestoreGateStatus count_status = ValidateCounts(
        input_identity_count,
        input_transform_count,
        input_attachment_count,
        input_binding_count,
        plan_scratch_capacity,
        proof_scratch_capacity,
        slice_scratch_capacity,
        output_gate_capacity,
        &record_count);
    if (count_status != WorldSceneActiveRestoreGateStatus::Success) {
        return RecordRejectedFailure(count_status);
    }

    WorldSceneApplyTimeRestoreProofBridgeDesc proof_desc{};
    proof_desc.identity_capacity = identity_capacity_;
    proof_desc.transform_capacity = transform_capacity_;
    proof_desc.attachment_capacity = attachment_capacity_;
    proof_desc.binding_capacity = binding_capacity_;
    proof_desc.plan_scratch_capacity = plan_scratch_capacity_;
    proof_desc.proof_capacity = proof_scratch_capacity_;
    proof_desc.slice_capacity = slice_scratch_capacity_;
    WorldSceneApplyTimeRestoreProofBridge proof_bridge(proof_desc);
    const WorldSceneApplyTimeRestoreProofResult proof_result = proof_bridge.Prove(
        world,
        object_registry,
        resource_registry,
        identity_destination,
        transform_destination,
        attachment_destination,
        binding_destination,
        input_identities,
        input_identity_count,
        input_transforms,
        input_transform_count,
        input_attachments,
        input_attachment_count,
        input_bindings,
        input_binding_count,
        plan_scratch,
        plan_scratch_capacity,
        proof_scratch,
        proof_scratch_capacity,
        slice_scratch,
        slice_scratch_capacity);
    if (!proof_result.Succeeded()) {
        return RecordProofFailure(proof_result);
    }

    if (proof_result.state.slice_record_count != record_count) {
        return RecordFailure(
            WorldSceneActiveRestoreGateStatus::InvalidProofSlice,
            proof_result.status);
    }

    const WorldSceneActiveRestoreGateStatus slice_status =
        ValidateSlices(slice_scratch, proof_result.state.slice_record_count);
    if (slice_status != WorldSceneActiveRestoreGateStatus::Success) {
        return RecordFailure(slice_status, proof_result.status);
    }

    WorldSceneActiveRestoreGateState state{};
    state.input_identity_count = input_identity_count;
    state.input_transform_count = input_transform_count;
    state.input_attachment_count = input_attachment_count;
    state.input_binding_count = input_binding_count;
    state.proof_record_count = proof_result.state.proof_record_count;
    state.slice_record_count = proof_result.state.slice_record_count;
    state.projected_object_acquire_count = proof_result.state.projected_object_acquire_count;
    state.projected_resource_acquire_count = proof_result.state.projected_resource_acquire_count;
    WriteGateRecords(
        slice_scratch,
        proof_result.state.slice_record_count,
        output_gates,
        &state);
    return RecordSuccess(state);
}

WorldSceneActiveRestoreGateSnapshot WorldSceneActiveRestoreGateBridge::Snapshot() const {
    return snapshot_;
}

WorldSceneActiveRestoreGateResult WorldSceneActiveRestoreGateBridge::RecordFailure(
    WorldSceneActiveRestoreGateStatus status) {
    return RecordFailure(status, WorldSceneApplyTimeRestoreProofStatus::Success);
}

WorldSceneActiveRestoreGateResult WorldSceneActiveRestoreGateBridge::RecordFailure(
    WorldSceneActiveRestoreGateStatus status,
    WorldSceneApplyTimeRestoreProofStatus proof_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_proof_status = proof_status;
    return WorldSceneActiveRestoreGateResult::Failure(status, proof_status);
}

WorldSceneActiveRestoreGateResult WorldSceneActiveRestoreGateBridge::RecordRejectedFailure(
    WorldSceneActiveRestoreGateStatus status) {
    ++snapshot_.rejected_operation_count;
    return RecordFailure(status);
}

WorldSceneActiveRestoreGateResult WorldSceneActiveRestoreGateBridge::RecordProofFailure(
    const WorldSceneApplyTimeRestoreProofResult &proof_result) {
    ++snapshot_.rejected_operation_count;
    const WorldSceneActiveRestoreGateStatus status = MapProofStatus(proof_result);
    return RecordFailure(status, proof_result.status);
}

WorldSceneActiveRestoreGateResult WorldSceneActiveRestoreGateBridge::RecordSuccess(
    const WorldSceneActiveRestoreGateState &state) {
    ++snapshot_.accepted_gate_count;
    snapshot_.emitted_gate_record_count += state.gate_record_count;
    snapshot_.identity_gate_count += state.identity_gate_count;
    snapshot_.transform_gate_count += state.transform_gate_count;
    snapshot_.attachment_gate_count += state.attachment_gate_count;
    snapshot_.binding_gate_count += state.binding_gate_count;
    snapshot_.last_status = WorldSceneActiveRestoreGateStatus::Success;
    snapshot_.last_proof_status = WorldSceneApplyTimeRestoreProofStatus::Success;
    return WorldSceneActiveRestoreGateResult::Success(state);
}

WorldSceneActiveRestoreGateStatus WorldSceneActiveRestoreGateBridge::ValidateBridgeCapacity() const {
    if ((identity_capacity_ == 0U) ||
        (transform_capacity_ == 0U) ||
        (attachment_capacity_ == 0U) ||
        (binding_capacity_ == 0U) ||
        (plan_scratch_capacity_ == 0U) ||
        (proof_scratch_capacity_ == 0U) ||
        (slice_scratch_capacity_ == 0U) ||
        (gate_output_capacity_ == 0U)) {
        return WorldSceneActiveRestoreGateStatus::InvalidBridgeCapacity;
    }

    return WorldSceneActiveRestoreGateStatus::Success;
}

WorldSceneActiveRestoreGateStatus WorldSceneActiveRestoreGateBridge::ValidateInputs(
    const WorldInstance *world,
    const yuengine::object::ObjectRegistry *object_registry,
    const yuengine::resource::ResourceRegistry *resource_registry,
    const WorldObjectIdentityBridge *identity_destination,
    const WorldTransformBridge *transform_destination,
    const WorldComponentAttachmentBridge *attachment_destination,
    const WorldComponentResourceBindingBridge *binding_destination,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    const WorldSceneDecodedRestorePlanRecord *plan_scratch,
    const WorldSceneApplyTimeRestoreProofRecord *proof_scratch,
    const WorldSceneApplyTimeRestoreProofSliceRecord *slice_scratch,
    const WorldSceneActiveRestoreGateRecord *output_gates) const {
    if (world == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidWorld;
    }

    if (object_registry == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidObjectRegistry;
    }

    if (resource_registry == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidResourceRegistry;
    }

    if (identity_destination == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidIdentityDestination;
    }

    if (transform_destination == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidTransformDestination;
    }

    if (attachment_destination == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidAttachmentDestination;
    }

    if (binding_destination == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidBindingDestination;
    }

    if (input_identities == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidIdentityInput;
    }

    if (input_transforms == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidTransformInput;
    }

    if (input_attachments == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidAttachmentInput;
    }

    if (input_bindings == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidBindingInput;
    }

    if (plan_scratch == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidPlanScratch;
    }

    if (proof_scratch == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidProofScratch;
    }

    if (slice_scratch == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidSliceScratch;
    }

    if (output_gates == nullptr) {
        return WorldSceneActiveRestoreGateStatus::InvalidGateOutput;
    }

    return WorldSceneActiveRestoreGateStatus::Success;
}

WorldSceneActiveRestoreGateStatus WorldSceneActiveRestoreGateBridge::ValidateCounts(
    std::uint32_t input_identity_count,
    std::uint32_t input_transform_count,
    std::uint32_t input_attachment_count,
    std::uint32_t input_binding_count,
    std::uint32_t plan_scratch_capacity,
    std::uint32_t proof_scratch_capacity,
    std::uint32_t slice_scratch_capacity,
    std::uint32_t output_gate_capacity,
    std::uint32_t *out_record_count) const {
    if (input_identity_count > identity_capacity_) {
        return WorldSceneActiveRestoreGateStatus::IdentityCapacityExceeded;
    }

    if (input_transform_count > transform_capacity_) {
        return WorldSceneActiveRestoreGateStatus::TransformCapacityExceeded;
    }

    if (input_attachment_count > attachment_capacity_) {
        return WorldSceneActiveRestoreGateStatus::AttachmentCapacityExceeded;
    }

    if (input_binding_count > binding_capacity_) {
        return WorldSceneActiveRestoreGateStatus::BindingCapacityExceeded;
    }

    std::uint32_t record_count = input_identity_count + input_transform_count;
    record_count += input_attachment_count;
    record_count += input_binding_count;
    if (out_record_count != nullptr) {
        *out_record_count = record_count;
    }

    if (record_count > plan_scratch_capacity_) {
        return WorldSceneActiveRestoreGateStatus::PlanScratchCapacityExceeded;
    }

    if (record_count > plan_scratch_capacity) {
        return WorldSceneActiveRestoreGateStatus::PlanScratchCapacityExceeded;
    }

    if (record_count > proof_scratch_capacity_) {
        return WorldSceneActiveRestoreGateStatus::ProofScratchCapacityExceeded;
    }

    if (record_count > proof_scratch_capacity) {
        return WorldSceneActiveRestoreGateStatus::ProofScratchCapacityExceeded;
    }

    if (record_count > slice_scratch_capacity_) {
        return WorldSceneActiveRestoreGateStatus::SliceScratchCapacityExceeded;
    }

    if (record_count > slice_scratch_capacity) {
        return WorldSceneActiveRestoreGateStatus::SliceScratchCapacityExceeded;
    }

    if (record_count > gate_output_capacity_) {
        return WorldSceneActiveRestoreGateStatus::GateOutputCapacityExceeded;
    }

    if (record_count > output_gate_capacity) {
        return WorldSceneActiveRestoreGateStatus::GateOutputCapacityExceeded;
    }

    return WorldSceneActiveRestoreGateStatus::Success;
}

WorldSceneActiveRestoreGateStatus WorldSceneActiveRestoreGateBridge::ValidateSlices(
    const WorldSceneApplyTimeRestoreProofSliceRecord *slice_scratch,
    std::uint32_t slice_record_count) const {
    std::uint32_t slice_index = 0U;
    while (slice_index < slice_record_count) {
        const WorldSceneApplyTimeRestoreProofSliceRecord &slice = slice_scratch[slice_index];
        if (slice.family == WorldSceneApplyTimeRestoreProofFamily::None) {
            return WorldSceneActiveRestoreGateStatus::InvalidProofSlice;
        }

        const WorldSceneActiveRestoreGateCleanupPolicy cleanup_policy =
            MapCleanupPolicy(slice.family);
        if (cleanup_policy == WorldSceneActiveRestoreGateCleanupPolicy::None) {
            return WorldSceneActiveRestoreGateStatus::InvalidCleanupPolicy;
        }

        ++slice_index;
    }

    return WorldSceneActiveRestoreGateStatus::Success;
}

WorldSceneActiveRestoreGateCleanupPolicy WorldSceneActiveRestoreGateBridge::MapCleanupPolicy(
    WorldSceneApplyTimeRestoreProofFamily family) const {
    if (family == WorldSceneApplyTimeRestoreProofFamily::Identity) {
        return WorldSceneActiveRestoreGateCleanupPolicy::ReleaseObjectIdentity;
    }

    if (family == WorldSceneApplyTimeRestoreProofFamily::Transform) {
        return WorldSceneActiveRestoreGateCleanupPolicy::ClearTransformRecord;
    }

    if (family == WorldSceneApplyTimeRestoreProofFamily::Attachment) {
        return WorldSceneActiveRestoreGateCleanupPolicy::RemoveComponentAttachment;
    }

    if (family == WorldSceneApplyTimeRestoreProofFamily::Binding) {
        return WorldSceneActiveRestoreGateCleanupPolicy::ReleaseResourceBinding;
    }

    return WorldSceneActiveRestoreGateCleanupPolicy::None;
}

WorldSceneActiveRestoreGateStatus WorldSceneActiveRestoreGateBridge::MapProofStatus(
    const WorldSceneApplyTimeRestoreProofResult &proof_result) const {
    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::InvalidWorld) {
        return WorldSceneActiveRestoreGateStatus::InvalidWorld;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::InvalidObjectRegistry) {
        return WorldSceneActiveRestoreGateStatus::InvalidObjectRegistry;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::InvalidResourceRegistry) {
        return WorldSceneActiveRestoreGateStatus::InvalidResourceRegistry;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::InvalidIdentityDestination) {
        return WorldSceneActiveRestoreGateStatus::InvalidIdentityDestination;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::InvalidTransformDestination) {
        return WorldSceneActiveRestoreGateStatus::InvalidTransformDestination;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::InvalidAttachmentDestination) {
        return WorldSceneActiveRestoreGateStatus::InvalidAttachmentDestination;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::InvalidBindingDestination) {
        return WorldSceneActiveRestoreGateStatus::InvalidBindingDestination;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::InvalidIdentityInput) {
        return WorldSceneActiveRestoreGateStatus::InvalidIdentityInput;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::InvalidTransformInput) {
        return WorldSceneActiveRestoreGateStatus::InvalidTransformInput;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::InvalidAttachmentInput) {
        return WorldSceneActiveRestoreGateStatus::InvalidAttachmentInput;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::InvalidBindingInput) {
        return WorldSceneActiveRestoreGateStatus::InvalidBindingInput;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::PlanScratchCapacityExceeded) {
        return WorldSceneActiveRestoreGateStatus::PlanScratchCapacityExceeded;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::ProofOutputCapacityExceeded) {
        return WorldSceneActiveRestoreGateStatus::ProofScratchCapacityExceeded;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::SliceOutputCapacityExceeded) {
        return WorldSceneActiveRestoreGateStatus::SliceScratchCapacityExceeded;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::IdentityCapacityExceeded) {
        return WorldSceneActiveRestoreGateStatus::IdentityCapacityExceeded;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::TransformCapacityExceeded) {
        return WorldSceneActiveRestoreGateStatus::TransformCapacityExceeded;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::AttachmentCapacityExceeded) {
        return WorldSceneActiveRestoreGateStatus::AttachmentCapacityExceeded;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::BindingCapacityExceeded) {
        return WorldSceneActiveRestoreGateStatus::BindingCapacityExceeded;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::DestinationNotEmpty) {
        return WorldSceneActiveRestoreGateStatus::DestinationNotEmpty;
    }

    if (proof_result.object_status != ObjectStatus::Success) {
        return WorldSceneActiveRestoreGateStatus::ObjectPreflightFailed;
    }

    if (proof_result.resource_status != ResourceStatus::Success) {
        return WorldSceneActiveRestoreGateStatus::ResourcePreflightFailed;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::ObjectAcquireFailed) {
        return WorldSceneActiveRestoreGateStatus::ObjectPreflightFailed;
    }

    if (proof_result.status == WorldSceneApplyTimeRestoreProofStatus::ResourceAcquireFailed) {
        return WorldSceneActiveRestoreGateStatus::ResourcePreflightFailed;
    }

    return WorldSceneActiveRestoreGateStatus::ProofFailed;
}

void WorldSceneActiveRestoreGateBridge::WriteGateRecords(
    const WorldSceneApplyTimeRestoreProofSliceRecord *slice_scratch,
    std::uint32_t slice_record_count,
    WorldSceneActiveRestoreGateRecord *output_gates,
    WorldSceneActiveRestoreGateState *state) const {
    std::uint32_t slice_index = 0U;
    while (slice_index < slice_record_count) {
        const WorldSceneApplyTimeRestoreProofSliceRecord &slice = slice_scratch[slice_index];
        WorldSceneActiveRestoreGateRecord &gate = output_gates[slice_index];
        gate = WorldSceneActiveRestoreGateRecord{};
        gate.family = slice.family;
        gate.cleanup_policy = MapCleanupPolicy(slice.family);
        gate.gate_index = slice_index;
        gate.plan_index = slice.plan_index;
        gate.input_index = slice.input_index;
        gate.world_object_id = slice.world_object_id;
        gate.object_handle = slice.object_handle;
        gate.component_type_id = slice.component_type_id;
        gate.component_slot_id = slice.component_slot_id;
        gate.resource_handle = slice.resource_handle;
        gate.expected_resource_type = slice.expected_resource_type;
        gate.status = WorldSceneActiveRestoreGateStatus::Success;

        if (state != nullptr) {
            ++state->gate_record_count;
            ++state->policy_record_count;
            if (slice.family == WorldSceneApplyTimeRestoreProofFamily::Identity) {
                ++state->identity_gate_count;
            }

            if (slice.family == WorldSceneApplyTimeRestoreProofFamily::Transform) {
                ++state->transform_gate_count;
            }

            if (slice.family == WorldSceneApplyTimeRestoreProofFamily::Attachment) {
                ++state->attachment_gate_count;
            }

            if (slice.family == WorldSceneApplyTimeRestoreProofFamily::Binding) {
                ++state->binding_gate_count;
            }
        }

        ++slice_index;
    }
}
}
