// Module: YuEngine World
// File: Src/YuEngine/World/Src/WorldSceneApplyTimeRestoreProofBridge.cpp

#include "YuEngine/World/WorldSceneApplyTimeRestoreProofBridge.h"

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectRegistry.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/World/WorldComponentAttachmentBridge.h"
#include "YuEngine/World/WorldComponentResourceBindingBridge.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectIdentityBridge.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanBridge.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanBridgeDesc.h"
#include "YuEngine/World/WorldTransformBridge.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::object::ObjectRegistry;
using yuengine::object::ObjectStatus;
using yuengine::resource::ResourceRegistry;
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

WorldSceneApplyTimeRestoreProofBridge::WorldSceneApplyTimeRestoreProofBridge(
    WorldSceneApplyTimeRestoreProofBridgeDesc desc)
    : identity_capacity_(ClampCapacity(desc.identity_capacity, MAX_WORLD_OBJECT_COUNT)),
      transform_capacity_(ClampCapacity(desc.transform_capacity, MAX_WORLD_OBJECT_COUNT)),
      attachment_capacity_(ClampCapacity(desc.attachment_capacity, MAX_WORLD_OBJECT_COUNT)),
      binding_capacity_(ClampCapacity(desc.binding_capacity, MAX_WORLD_OBJECT_COUNT)),
      plan_scratch_capacity_(ClampCapacity(
          desc.plan_scratch_capacity,
          MAX_WORLD_SCENE_DECODED_RESTORE_PLAN_RECORD_COUNT)),
      proof_capacity_(ClampCapacity(
          desc.proof_capacity,
          MAX_WORLD_SCENE_APPLY_TIME_RESTORE_PROOF_RECORD_COUNT)),
      slice_capacity_(ClampCapacity(
          desc.slice_capacity,
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
              desc.proof_capacity,
              MAX_WORLD_SCENE_APPLY_TIME_RESTORE_PROOF_RECORD_COUNT),
          ClampCapacity(
              desc.slice_capacity,
              MAX_WORLD_SCENE_APPLY_TIME_RESTORE_SLICE_RECORD_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          WorldObjectIdentityStatus::Success,
          WorldTransformStatus::Success,
          WorldComponentAttachmentStatus::Success,
          WorldComponentResourceBindingStatus::Success,
          ObjectStatus::Success,
          ResourceStatus::Success,
          WorldSceneDecodedRestorePlanStatus::Success,
          WorldSceneApplyTimeRestoreProofStatus::Success} {
    if ((desc.identity_capacity == 0U) ||
        (desc.transform_capacity == 0U) ||
        (desc.attachment_capacity == 0U) ||
        (desc.binding_capacity == 0U) ||
        (desc.plan_scratch_capacity == 0U) ||
        (desc.proof_capacity == 0U) ||
        (desc.slice_capacity == 0U)) {
        snapshot_.last_status = WorldSceneApplyTimeRestoreProofStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldSceneApplyTimeRestoreProofResult WorldSceneApplyTimeRestoreProofBridge::Prove(
    const WorldInstance *world,
    const ObjectRegistry *object_registry,
    const ResourceRegistry *resource_registry,
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
    WorldSceneApplyTimeRestoreProofRecord *output_proofs,
    std::uint32_t output_proof_capacity,
    WorldSceneApplyTimeRestoreProofSliceRecord *output_slices,
    std::uint32_t output_slice_capacity) {
    ++snapshot_.proof_attempt_count;

    const WorldSceneApplyTimeRestoreProofStatus bridge_status =
        ValidateBridgeCapacity();
    if (bridge_status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return RecordFailure(bridge_status);
    }

    const WorldSceneApplyTimeRestoreProofStatus input_status = ValidateInputs(
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
        output_proofs,
        output_slices);
    if (input_status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return RecordFailure(input_status);
    }

    std::uint32_t required_record_count = 0U;
    const WorldSceneApplyTimeRestoreProofStatus count_status = ValidateCounts(
        input_identity_count,
        input_transform_count,
        input_attachment_count,
        input_binding_count,
        plan_scratch_capacity,
        output_proof_capacity,
        output_slice_capacity,
        &required_record_count);
    if (count_status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return RecordRejectedFailure(count_status);
    }

    WorldSceneDecodedRestorePlanBridgeDesc plan_desc{};
    plan_desc.identity_capacity = identity_capacity_;
    plan_desc.transform_capacity = transform_capacity_;
    plan_desc.attachment_capacity = attachment_capacity_;
    plan_desc.binding_capacity = binding_capacity_;
    plan_desc.plan_capacity = plan_scratch_capacity_;
    WorldSceneDecodedRestorePlanBridge plan_bridge(plan_desc);
    const WorldSceneDecodedRestorePlanResult plan_result = plan_bridge.Plan(
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
        plan_scratch_capacity);
    if (!plan_result.Succeeded()) {
        return RecordPlanFailure(plan_result);
    }

    if (plan_result.state.output_plan_count != required_record_count) {
        return RecordFailure(
            WorldSceneApplyTimeRestoreProofStatus::PlanInputMismatch,
            plan_result.status,
            plan_result.identity_status,
            plan_result.transform_status,
            plan_result.attachment_status,
            plan_result.binding_status,
            plan_result.object_status,
            plan_result.resource_status);
    }

    const WorldSceneApplyTimeRestoreProofStatus plan_record_status =
        ValidatePlanRecords(
            plan_scratch,
            plan_result.state.output_plan_count,
            input_identity_count,
            input_transform_count,
            input_attachment_count,
            input_binding_count);
    if (plan_record_status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return RecordFailure(
            plan_record_status,
            plan_result.status,
            plan_result.identity_status,
            plan_result.transform_status,
            plan_result.attachment_status,
            plan_result.binding_status,
            plan_result.object_status,
            plan_result.resource_status);
    }

    WorldSceneApplyTimeRestoreProofState state{};
    state.input_identity_count = input_identity_count;
    state.input_transform_count = input_transform_count;
    state.input_attachment_count = input_attachment_count;
    state.input_binding_count = input_binding_count;
    state.plan_record_count = plan_result.state.output_plan_count;
    state.proof_record_count = plan_result.state.output_plan_count;
    state.slice_record_count = plan_result.state.output_plan_count;
    WriteProofOutputs(
        plan_scratch,
        plan_result.state.output_plan_count,
        output_proofs,
        output_slices,
        &state);
    return RecordSuccess(state);
}

WorldSceneApplyTimeRestoreProofSnapshot
WorldSceneApplyTimeRestoreProofBridge::Snapshot() const {
    return snapshot_;
}

WorldSceneApplyTimeRestoreProofResult
WorldSceneApplyTimeRestoreProofBridge::RecordFailure(
    WorldSceneApplyTimeRestoreProofStatus status) {
    return RecordFailure(
        status,
        WorldSceneDecodedRestorePlanStatus::Success,
        WorldObjectIdentityStatus::Success,
        WorldTransformStatus::Success,
        WorldComponentAttachmentStatus::Success,
        WorldComponentResourceBindingStatus::Success,
        ObjectStatus::Success,
        ResourceStatus::Success);
}

WorldSceneApplyTimeRestoreProofResult
WorldSceneApplyTimeRestoreProofBridge::RecordFailure(
    WorldSceneApplyTimeRestoreProofStatus status,
    WorldSceneDecodedRestorePlanStatus plan_status,
    WorldObjectIdentityStatus identity_status,
    WorldTransformStatus transform_status,
    WorldComponentAttachmentStatus attachment_status,
    WorldComponentResourceBindingStatus binding_status,
    ObjectStatus object_status,
    ResourceStatus resource_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_plan_status = plan_status;
    snapshot_.last_identity_status = identity_status;
    snapshot_.last_transform_status = transform_status;
    snapshot_.last_attachment_status = attachment_status;
    snapshot_.last_binding_status = binding_status;
    snapshot_.last_object_status = object_status;
    snapshot_.last_resource_status = resource_status;
    return WorldSceneApplyTimeRestoreProofResult::Failure(
        status,
        plan_status,
        identity_status,
        transform_status,
        attachment_status,
        binding_status,
        object_status,
        resource_status);
}

WorldSceneApplyTimeRestoreProofResult
WorldSceneApplyTimeRestoreProofBridge::RecordRejectedFailure(
    WorldSceneApplyTimeRestoreProofStatus status) {
    ++snapshot_.rejected_operation_count;
    return RecordFailure(status);
}

WorldSceneApplyTimeRestoreProofResult
WorldSceneApplyTimeRestoreProofBridge::RecordPlanFailure(
    const WorldSceneDecodedRestorePlanResult &plan_result) {
    ++snapshot_.rejected_operation_count;
    const WorldSceneApplyTimeRestoreProofStatus status = MapPlanStatus(
        plan_result.status);
    return RecordFailure(
        status,
        plan_result.status,
        plan_result.identity_status,
        plan_result.transform_status,
        plan_result.attachment_status,
        plan_result.binding_status,
        plan_result.object_status,
        plan_result.resource_status);
}

WorldSceneApplyTimeRestoreProofResult
WorldSceneApplyTimeRestoreProofBridge::RecordSuccess(
    const WorldSceneApplyTimeRestoreProofState &state) {
    snapshot_.proven_identity_count += state.proven_identity_count;
    snapshot_.proven_transform_count += state.proven_transform_count;
    snapshot_.proven_attachment_count += state.proven_attachment_count;
    snapshot_.proven_binding_count += state.proven_binding_count;
    snapshot_.emitted_slice_count += state.emitted_slice_count;
    snapshot_.last_status = WorldSceneApplyTimeRestoreProofStatus::Success;
    snapshot_.last_plan_status = WorldSceneDecodedRestorePlanStatus::Success;
    snapshot_.last_identity_status = WorldObjectIdentityStatus::Success;
    snapshot_.last_transform_status = WorldTransformStatus::Success;
    snapshot_.last_attachment_status = WorldComponentAttachmentStatus::Success;
    snapshot_.last_binding_status = WorldComponentResourceBindingStatus::Success;
    snapshot_.last_object_status = ObjectStatus::Success;
    snapshot_.last_resource_status = ResourceStatus::Success;
    return WorldSceneApplyTimeRestoreProofResult::Success(state);
}

WorldSceneApplyTimeRestoreProofStatus
WorldSceneApplyTimeRestoreProofBridge::ValidateBridgeCapacity() const {
    if ((identity_capacity_ == 0U) ||
        (transform_capacity_ == 0U) ||
        (attachment_capacity_ == 0U) ||
        (binding_capacity_ == 0U) ||
        (plan_scratch_capacity_ == 0U) ||
        (proof_capacity_ == 0U) ||
        (slice_capacity_ == 0U)) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidBridgeCapacity;
    }

    return WorldSceneApplyTimeRestoreProofStatus::Success;
}

WorldSceneApplyTimeRestoreProofStatus
WorldSceneApplyTimeRestoreProofBridge::ValidateInputs(
    const WorldInstance *world,
    const ObjectRegistry *object_registry,
    const ResourceRegistry *resource_registry,
    const WorldObjectIdentityBridge *identity_destination,
    const WorldTransformBridge *transform_destination,
    const WorldComponentAttachmentBridge *attachment_destination,
    const WorldComponentResourceBindingBridge *binding_destination,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    const WorldSceneDecodedRestorePlanRecord *plan_scratch,
    const WorldSceneApplyTimeRestoreProofRecord *output_proofs,
    const WorldSceneApplyTimeRestoreProofSliceRecord *output_slices) const {
    if (world == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidWorld;
    }

    if (object_registry == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidObjectRegistry;
    }

    if (resource_registry == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidResourceRegistry;
    }

    if (identity_destination == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidIdentityDestination;
    }

    if (transform_destination == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidTransformDestination;
    }

    if (attachment_destination == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidAttachmentDestination;
    }

    if (binding_destination == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidBindingDestination;
    }

    if (input_identities == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidIdentityInput;
    }

    if (input_transforms == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidTransformInput;
    }

    if (input_attachments == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidAttachmentInput;
    }

    if (input_bindings == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidBindingInput;
    }

    if (plan_scratch == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidPlanScratch;
    }

    if (output_proofs == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidProofOutput;
    }

    if (output_slices == nullptr) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidSliceOutput;
    }

    return WorldSceneApplyTimeRestoreProofStatus::Success;
}

WorldSceneApplyTimeRestoreProofStatus
WorldSceneApplyTimeRestoreProofBridge::ValidateCounts(
    std::uint32_t input_identity_count,
    std::uint32_t input_transform_count,
    std::uint32_t input_attachment_count,
    std::uint32_t input_binding_count,
    std::uint32_t plan_scratch_capacity,
    std::uint32_t output_proof_capacity,
    std::uint32_t output_slice_capacity,
    std::uint32_t *out_record_count) const {
    if (input_identity_count > identity_capacity_) {
        return WorldSceneApplyTimeRestoreProofStatus::IdentityCapacityExceeded;
    }

    if (input_transform_count > transform_capacity_) {
        return WorldSceneApplyTimeRestoreProofStatus::TransformCapacityExceeded;
    }

    if (input_attachment_count > attachment_capacity_) {
        return WorldSceneApplyTimeRestoreProofStatus::AttachmentCapacityExceeded;
    }

    if (input_binding_count > binding_capacity_) {
        return WorldSceneApplyTimeRestoreProofStatus::BindingCapacityExceeded;
    }

    std::uint32_t record_count = input_identity_count + input_transform_count;
    record_count += input_attachment_count;
    record_count += input_binding_count;
    if (out_record_count != nullptr) {
        *out_record_count = record_count;
    }

    if (record_count > plan_scratch_capacity_) {
        return WorldSceneApplyTimeRestoreProofStatus::PlanScratchCapacityExceeded;
    }

    if (record_count > plan_scratch_capacity) {
        return WorldSceneApplyTimeRestoreProofStatus::PlanScratchCapacityExceeded;
    }

    if (record_count > proof_capacity_) {
        return WorldSceneApplyTimeRestoreProofStatus::ProofOutputCapacityExceeded;
    }

    if (record_count > output_proof_capacity) {
        return WorldSceneApplyTimeRestoreProofStatus::ProofOutputCapacityExceeded;
    }

    if (record_count > slice_capacity_) {
        return WorldSceneApplyTimeRestoreProofStatus::SliceOutputCapacityExceeded;
    }

    if (record_count > output_slice_capacity) {
        return WorldSceneApplyTimeRestoreProofStatus::SliceOutputCapacityExceeded;
    }

    return WorldSceneApplyTimeRestoreProofStatus::Success;
}

WorldSceneApplyTimeRestoreProofStatus
WorldSceneApplyTimeRestoreProofBridge::ValidatePlanRecords(
    const WorldSceneDecodedRestorePlanRecord *plan_scratch,
    std::uint32_t plan_record_count,
    std::uint32_t input_identity_count,
    std::uint32_t input_transform_count,
    std::uint32_t input_attachment_count,
    std::uint32_t input_binding_count) const {
    std::uint32_t expected_record_count = input_identity_count + input_transform_count;
    expected_record_count += input_attachment_count;
    expected_record_count += input_binding_count;
    if (plan_record_count != expected_record_count) {
        return WorldSceneApplyTimeRestoreProofStatus::PlanInputMismatch;
    }

    std::uint32_t range_begin = 0U;
    WorldSceneApplyTimeRestoreProofStatus range_status =
        ValidatePlanFamilyRange(
            plan_scratch,
            range_begin,
            input_identity_count,
            WorldSceneDecodedRestorePlanRecordFamily::Identity);
    if (range_status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return range_status;
    }

    range_begin += input_identity_count;
    range_status = ValidatePlanFamilyRange(
        plan_scratch,
        range_begin,
        input_transform_count,
        WorldSceneDecodedRestorePlanRecordFamily::Transform);
    if (range_status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return range_status;
    }

    range_begin += input_transform_count;
    range_status = ValidatePlanFamilyRange(
        plan_scratch,
        range_begin,
        input_attachment_count,
        WorldSceneDecodedRestorePlanRecordFamily::Attachment);
    if (range_status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return range_status;
    }

    range_begin += input_attachment_count;
    range_status = ValidatePlanFamilyRange(
        plan_scratch,
        range_begin,
        input_binding_count,
        WorldSceneDecodedRestorePlanRecordFamily::Binding);
    if (range_status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return range_status;
    }

    return WorldSceneApplyTimeRestoreProofStatus::Success;
}

WorldSceneApplyTimeRestoreProofStatus
WorldSceneApplyTimeRestoreProofBridge::ValidatePlanFamilyRange(
    const WorldSceneDecodedRestorePlanRecord *plan_scratch,
    std::uint32_t range_begin,
    std::uint32_t range_count,
    WorldSceneDecodedRestorePlanRecordFamily expected_family) const {
    std::uint32_t range_index = 0U;
    while (range_index < range_count) {
        const std::uint32_t plan_index = range_begin + range_index;
        const WorldSceneDecodedRestorePlanRecord &record = plan_scratch[plan_index];
        if (record.family != expected_family) {
            return WorldSceneApplyTimeRestoreProofStatus::UnexpectedPlanFamily;
        }

        if (record.input_index != range_index) {
            return WorldSceneApplyTimeRestoreProofStatus::PlanInputMismatch;
        }

        if (record.status != WorldSceneDecodedRestorePlanStatus::Success) {
            return WorldSceneApplyTimeRestoreProofStatus::InvalidPlanRecord;
        }

        ++range_index;
    }

    return WorldSceneApplyTimeRestoreProofStatus::Success;
}

WorldSceneApplyTimeRestoreProofStatus
WorldSceneApplyTimeRestoreProofBridge::MapPlanStatus(
    WorldSceneDecodedRestorePlanStatus plan_status) const {
    if (plan_status == WorldSceneDecodedRestorePlanStatus::Success) {
        return WorldSceneApplyTimeRestoreProofStatus::Success;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidBridgeCapacity) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidBridgeCapacity;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidWorld) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidWorld;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidObjectRegistry) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidObjectRegistry;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidResourceRegistry) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidResourceRegistry;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidIdentityDestination) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidIdentityDestination;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidTransformDestination) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidTransformDestination;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidAttachmentDestination) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidAttachmentDestination;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidBindingDestination) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidBindingDestination;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidIdentityInput) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidIdentityInput;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidTransformInput) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidTransformInput;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidAttachmentInput) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidAttachmentInput;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidBindingInput) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidBindingInput;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::InvalidPlanOutput) {
        return WorldSceneApplyTimeRestoreProofStatus::InvalidPlanScratch;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::PlanOutputCapacityExceeded) {
        return WorldSceneApplyTimeRestoreProofStatus::PlanScratchCapacityExceeded;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::IdentityCapacityExceeded) {
        return WorldSceneApplyTimeRestoreProofStatus::IdentityCapacityExceeded;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::TransformCapacityExceeded) {
        return WorldSceneApplyTimeRestoreProofStatus::TransformCapacityExceeded;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::AttachmentCapacityExceeded) {
        return WorldSceneApplyTimeRestoreProofStatus::AttachmentCapacityExceeded;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::BindingCapacityExceeded) {
        return WorldSceneApplyTimeRestoreProofStatus::BindingCapacityExceeded;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::DestinationNotEmpty) {
        return WorldSceneApplyTimeRestoreProofStatus::DestinationNotEmpty;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::ObjectAcquireWouldOverflow) {
        return WorldSceneApplyTimeRestoreProofStatus::ObjectAcquireFailed;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::ObjectAcquireFailed) {
        return WorldSceneApplyTimeRestoreProofStatus::ObjectAcquireFailed;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::ResourceAcquireWouldOverflow) {
        return WorldSceneApplyTimeRestoreProofStatus::ResourceAcquireFailed;
    }

    if (plan_status == WorldSceneDecodedRestorePlanStatus::ResourceAcquireFailed) {
        return WorldSceneApplyTimeRestoreProofStatus::ResourceAcquireFailed;
    }

    return WorldSceneApplyTimeRestoreProofStatus::PlanFailed;
}

WorldSceneApplyTimeRestoreProofFamily
WorldSceneApplyTimeRestoreProofBridge::MapPlanFamily(
    WorldSceneDecodedRestorePlanRecordFamily family) const {
    if (family == WorldSceneDecodedRestorePlanRecordFamily::Identity) {
        return WorldSceneApplyTimeRestoreProofFamily::Identity;
    }

    if (family == WorldSceneDecodedRestorePlanRecordFamily::Transform) {
        return WorldSceneApplyTimeRestoreProofFamily::Transform;
    }

    if (family == WorldSceneDecodedRestorePlanRecordFamily::Attachment) {
        return WorldSceneApplyTimeRestoreProofFamily::Attachment;
    }

    if (family == WorldSceneDecodedRestorePlanRecordFamily::Binding) {
        return WorldSceneApplyTimeRestoreProofFamily::Binding;
    }

    return WorldSceneApplyTimeRestoreProofFamily::None;
}

void WorldSceneApplyTimeRestoreProofBridge::WriteProofOutputs(
    const WorldSceneDecodedRestorePlanRecord *plan_scratch,
    std::uint32_t plan_record_count,
    WorldSceneApplyTimeRestoreProofRecord *output_proofs,
    WorldSceneApplyTimeRestoreProofSliceRecord *output_slices,
    WorldSceneApplyTimeRestoreProofState *state) const {
    std::uint32_t plan_index = 0U;
    while (plan_index < plan_record_count) {
        const WorldSceneDecodedRestorePlanRecord &plan_record = plan_scratch[plan_index];
        const WorldSceneApplyTimeRestoreProofFamily family = MapPlanFamily(
            plan_record.family);

        WorldSceneApplyTimeRestoreProofRecord &proof_record = output_proofs[plan_index];
        proof_record = WorldSceneApplyTimeRestoreProofRecord{};
        proof_record.family = family;
        proof_record.plan_index = plan_index;
        proof_record.input_index = plan_record.input_index;
        proof_record.world_object_id = plan_record.world_object_id;
        proof_record.object_handle = plan_record.object_handle;
        proof_record.component_type_id = plan_record.component_type_id;
        proof_record.component_slot_id = plan_record.component_slot_id;
        proof_record.resource_handle = plan_record.resource_handle;
        proof_record.expected_resource_type = plan_record.expected_resource_type;
        proof_record.projected_object_acquire_count =
            plan_record.projected_object_acquire_count;
        proof_record.projected_resource_acquire_count =
            plan_record.projected_resource_acquire_count;
        proof_record.plan_status = plan_record.status;
        proof_record.status = WorldSceneApplyTimeRestoreProofStatus::Success;

        WorldSceneApplyTimeRestoreProofSliceRecord &slice_record = output_slices[plan_index];
        slice_record = WorldSceneApplyTimeRestoreProofSliceRecord{};
        slice_record.family = family;
        slice_record.plan_index = plan_index;
        slice_record.input_index = plan_record.input_index;
        slice_record.world_object_id = plan_record.world_object_id;
        slice_record.object_handle = plan_record.object_handle;
        slice_record.component_type_id = plan_record.component_type_id;
        slice_record.component_slot_id = plan_record.component_slot_id;
        slice_record.resource_handle = plan_record.resource_handle;
        slice_record.expected_resource_type = plan_record.expected_resource_type;

        if (state != nullptr) {
            ++state->emitted_slice_count;
            state->projected_object_acquire_count +=
                plan_record.projected_object_acquire_count;
            state->projected_resource_acquire_count +=
                plan_record.projected_resource_acquire_count;
            if (family == WorldSceneApplyTimeRestoreProofFamily::Identity) {
                ++state->proven_identity_count;
            }

            if (family == WorldSceneApplyTimeRestoreProofFamily::Transform) {
                ++state->proven_transform_count;
            }

            if (family == WorldSceneApplyTimeRestoreProofFamily::Attachment) {
                ++state->proven_attachment_count;
            }

            if (family == WorldSceneApplyTimeRestoreProofFamily::Binding) {
                ++state->proven_binding_count;
            }
        }

        ++plan_index;
    }
}
}
