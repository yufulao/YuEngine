// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldSceneDecodedRestorePlanBridge.cpp

#include "YuEngine/World/WorldSceneDecodedRestorePlanBridge.h"

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectRegistry.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/World/WorldComponentAttachmentBridge.h"
#include "YuEngine/World/WorldComponentResourceBindingBridge.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectIdentityBridge.h"
#include "YuEngine/World/WorldObjectIdentitySnapshot.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformSnapshot.h"

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

WorldSceneDecodedRestorePlanBridge::WorldSceneDecodedRestorePlanBridge(
    WorldSceneDecodedRestorePlanBridgeDesc desc)
    : identity_capacity_(ClampCapacity(desc.identity_capacity, MAX_WORLD_OBJECT_COUNT)),
      transform_capacity_(ClampCapacity(desc.transform_capacity, MAX_WORLD_OBJECT_COUNT)),
      attachment_capacity_(ClampCapacity(desc.attachment_capacity, MAX_WORLD_OBJECT_COUNT)),
      binding_capacity_(ClampCapacity(desc.binding_capacity, MAX_WORLD_OBJECT_COUNT)),
      plan_capacity_(ClampCapacity(
          desc.plan_capacity,
          MAX_WORLD_SCENE_DECODED_RESTORE_PLAN_RECORD_COUNT)),
      snapshot_{
          ClampCapacity(desc.identity_capacity, MAX_WORLD_OBJECT_COUNT),
          ClampCapacity(desc.transform_capacity, MAX_WORLD_OBJECT_COUNT),
          ClampCapacity(desc.attachment_capacity, MAX_WORLD_OBJECT_COUNT),
          ClampCapacity(desc.binding_capacity, MAX_WORLD_OBJECT_COUNT),
          ClampCapacity(desc.plan_capacity, MAX_WORLD_SCENE_DECODED_RESTORE_PLAN_RECORD_COUNT),
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
          WorldSceneDecodedRestorePlanStatus::Success} {
    if ((desc.identity_capacity == 0U) ||
        (desc.transform_capacity == 0U) ||
        (desc.attachment_capacity == 0U) ||
        (desc.binding_capacity == 0U) ||
        (desc.plan_capacity == 0U)) {
        snapshot_.last_status = WorldSceneDecodedRestorePlanStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldSceneDecodedRestorePlanResult WorldSceneDecodedRestorePlanBridge::Plan(
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
    WorldSceneDecodedRestorePlanRecord *output_plan,
    std::uint32_t output_plan_capacity) {
    ++snapshot_.plan_attempt_count;

    const WorldSceneDecodedRestorePlanStatus bridge_status = ValidateBridgeCapacity();
    if (bridge_status != WorldSceneDecodedRestorePlanStatus::Success) {
        return RecordFailure(bridge_status);
    }

    if (world == nullptr) {
        return RecordFailure(WorldSceneDecodedRestorePlanStatus::InvalidWorld);
    }

    if (object_registry == nullptr) {
        return RecordFailure(WorldSceneDecodedRestorePlanStatus::InvalidObjectRegistry);
    }

    if (resource_registry == nullptr) {
        return RecordFailure(WorldSceneDecodedRestorePlanStatus::InvalidResourceRegistry);
    }

    if (input_identities == nullptr) {
        return RecordFailure(WorldSceneDecodedRestorePlanStatus::InvalidIdentityInput);
    }

    if (input_transforms == nullptr) {
        return RecordFailure(WorldSceneDecodedRestorePlanStatus::InvalidTransformInput);
    }

    if (input_attachments == nullptr) {
        return RecordFailure(WorldSceneDecodedRestorePlanStatus::InvalidAttachmentInput);
    }

    if (input_bindings == nullptr) {
        return RecordFailure(WorldSceneDecodedRestorePlanStatus::InvalidBindingInput);
    }

    if (output_plan == nullptr) {
        return RecordFailure(WorldSceneDecodedRestorePlanStatus::InvalidPlanOutput);
    }

    std::uint32_t output_plan_count = 0U;
    const WorldSceneDecodedRestorePlanStatus count_status = ValidateInputCounts(
        input_identity_count,
        input_transform_count,
        input_attachment_count,
        input_binding_count,
        output_plan_capacity,
        &output_plan_count);
    if (count_status != WorldSceneDecodedRestorePlanStatus::Success) {
        return RecordRejectedFailure(count_status);
    }

    WorldObjectIdentityStatus identity_status = WorldObjectIdentityStatus::Success;
    WorldTransformStatus transform_status = WorldTransformStatus::Success;
    WorldComponentAttachmentStatus attachment_status = WorldComponentAttachmentStatus::Success;
    WorldComponentResourceBindingStatus binding_status = WorldComponentResourceBindingStatus::Success;
    const WorldSceneDecodedRestorePlanStatus destination_status = ValidateDestinations(
        identity_destination,
        transform_destination,
        attachment_destination,
        binding_destination,
        input_identity_count,
        input_transform_count,
        input_attachment_count,
        input_binding_count,
        &identity_status,
        &transform_status,
        &attachment_status,
        &binding_status);
    if (destination_status != WorldSceneDecodedRestorePlanStatus::Success) {
        return RecordFailure(
            destination_status,
            identity_status,
            transform_status,
            attachment_status,
            binding_status,
            ObjectStatus::Success,
            ResourceStatus::Success);
    }

    ObjectStatus object_status = ObjectStatus::Success;
    const WorldSceneDecodedRestorePlanStatus identity_status_result = ValidateIdentityRecords(
        *world,
        *object_registry,
        input_identities,
        input_identity_count,
        &object_status);
    if (identity_status_result != WorldSceneDecodedRestorePlanStatus::Success) {
        return RecordRejectedFailure(identity_status_result, object_status);
    }

    const WorldSceneDecodedRestorePlanStatus transform_status_result = ValidateTransformRecords(
        *world,
        input_identities,
        input_identity_count,
        input_transforms,
        input_transform_count);
    if (transform_status_result != WorldSceneDecodedRestorePlanStatus::Success) {
        return RecordRejectedFailure(transform_status_result);
    }

    const WorldSceneDecodedRestorePlanStatus attachment_status_result = ValidateAttachmentRecords(
        *world,
        input_identities,
        input_identity_count,
        input_attachments,
        input_attachment_count);
    if (attachment_status_result != WorldSceneDecodedRestorePlanStatus::Success) {
        return RecordRejectedFailure(attachment_status_result);
    }

    ResourceStatus resource_status = ResourceStatus::Success;
    const WorldSceneDecodedRestorePlanStatus binding_status_result = ValidateBindingRecords(
        *world,
        *resource_registry,
        input_identities,
        input_identity_count,
        input_attachments,
        input_attachment_count,
        input_bindings,
        input_binding_count,
        &resource_status);
    if (binding_status_result != WorldSceneDecodedRestorePlanStatus::Success) {
        return RecordRejectedFailure(binding_status_result, resource_status);
    }

    WorldSceneDecodedRestorePlanState state{};
    state.input_identity_count = input_identity_count;
    state.input_transform_count = input_transform_count;
    state.input_attachment_count = input_attachment_count;
    state.input_binding_count = input_binding_count;
    state.output_plan_count = output_plan_count;
    WritePlanRecords(
        input_identities,
        input_identity_count,
        input_transforms,
        input_transform_count,
        input_attachments,
        input_attachment_count,
        input_bindings,
        input_binding_count,
        output_plan,
        &state);
    return RecordSuccess(state);
}

WorldSceneDecodedRestorePlanSnapshot WorldSceneDecodedRestorePlanBridge::Snapshot() const {
    return snapshot_;
}

WorldSceneDecodedRestorePlanResult WorldSceneDecodedRestorePlanBridge::RecordFailure(
    WorldSceneDecodedRestorePlanStatus status) {
    return RecordFailure(
        status,
        WorldObjectIdentityStatus::Success,
        WorldTransformStatus::Success,
        WorldComponentAttachmentStatus::Success,
        WorldComponentResourceBindingStatus::Success,
        ObjectStatus::Success,
        ResourceStatus::Success);
}

WorldSceneDecodedRestorePlanResult WorldSceneDecodedRestorePlanBridge::RecordFailure(
    WorldSceneDecodedRestorePlanStatus status,
    WorldObjectIdentityStatus identity_status,
    WorldTransformStatus transform_status,
    WorldComponentAttachmentStatus attachment_status,
    WorldComponentResourceBindingStatus binding_status,
    ObjectStatus object_status,
    ResourceStatus resource_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_identity_status = identity_status;
    snapshot_.last_transform_status = transform_status;
    snapshot_.last_attachment_status = attachment_status;
    snapshot_.last_binding_status = binding_status;
    snapshot_.last_object_status = object_status;
    snapshot_.last_resource_status = resource_status;
    return WorldSceneDecodedRestorePlanResult::Failure(
        status,
        identity_status,
        transform_status,
        attachment_status,
        binding_status,
        object_status,
        resource_status);
}

WorldSceneDecodedRestorePlanResult WorldSceneDecodedRestorePlanBridge::RecordRejectedFailure(
    WorldSceneDecodedRestorePlanStatus status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status);
}

WorldSceneDecodedRestorePlanResult WorldSceneDecodedRestorePlanBridge::RecordRejectedFailure(
    WorldSceneDecodedRestorePlanStatus status,
    ObjectStatus object_status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(
        status,
        WorldObjectIdentityStatus::Success,
        WorldTransformStatus::Success,
        WorldComponentAttachmentStatus::Success,
        WorldComponentResourceBindingStatus::Success,
        object_status,
        ResourceStatus::Success);
}

WorldSceneDecodedRestorePlanResult WorldSceneDecodedRestorePlanBridge::RecordRejectedFailure(
    WorldSceneDecodedRestorePlanStatus status,
    ResourceStatus resource_status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(
        status,
        WorldObjectIdentityStatus::Success,
        WorldTransformStatus::Success,
        WorldComponentAttachmentStatus::Success,
        WorldComponentResourceBindingStatus::Success,
        ObjectStatus::Success,
        resource_status);
}

WorldSceneDecodedRestorePlanResult WorldSceneDecodedRestorePlanBridge::RecordSuccess(
    const WorldSceneDecodedRestorePlanState &state) {
    snapshot_.planned_identity_count += state.planned_identity_count;
    snapshot_.planned_transform_count += state.planned_transform_count;
    snapshot_.planned_attachment_count += state.planned_attachment_count;
    snapshot_.planned_binding_count += state.planned_binding_count;
    snapshot_.last_status = WorldSceneDecodedRestorePlanStatus::Success;
    snapshot_.last_identity_status = WorldObjectIdentityStatus::Success;
    snapshot_.last_transform_status = WorldTransformStatus::Success;
    snapshot_.last_attachment_status = WorldComponentAttachmentStatus::Success;
    snapshot_.last_binding_status = WorldComponentResourceBindingStatus::Success;
    snapshot_.last_object_status = ObjectStatus::Success;
    snapshot_.last_resource_status = ResourceStatus::Success;
    return WorldSceneDecodedRestorePlanResult::Success(state);
}

WorldSceneDecodedRestorePlanStatus WorldSceneDecodedRestorePlanBridge::ValidateBridgeCapacity() const {
    if ((identity_capacity_ == 0U) ||
        (transform_capacity_ == 0U) ||
        (attachment_capacity_ == 0U) ||
        (binding_capacity_ == 0U) ||
        (plan_capacity_ == 0U)) {
        return WorldSceneDecodedRestorePlanStatus::InvalidBridgeCapacity;
    }

    return WorldSceneDecodedRestorePlanStatus::Success;
}

WorldSceneDecodedRestorePlanStatus WorldSceneDecodedRestorePlanBridge::ValidateInputCounts(
    std::uint32_t input_identity_count,
    std::uint32_t input_transform_count,
    std::uint32_t input_attachment_count,
    std::uint32_t input_binding_count,
    std::uint32_t output_plan_capacity,
    std::uint32_t *out_plan_record_count) const {
    if (input_identity_count > identity_capacity_) {
        return WorldSceneDecodedRestorePlanStatus::IdentityCapacityExceeded;
    }

    if (input_transform_count > transform_capacity_) {
        return WorldSceneDecodedRestorePlanStatus::TransformCapacityExceeded;
    }

    if (input_attachment_count > attachment_capacity_) {
        return WorldSceneDecodedRestorePlanStatus::AttachmentCapacityExceeded;
    }

    if (input_binding_count > binding_capacity_) {
        return WorldSceneDecodedRestorePlanStatus::BindingCapacityExceeded;
    }

    std::uint32_t plan_record_count = input_identity_count + input_transform_count;
    plan_record_count += input_attachment_count;
    plan_record_count += input_binding_count;
    if (out_plan_record_count != nullptr) {
        *out_plan_record_count = plan_record_count;
    }

    if (plan_record_count > plan_capacity_) {
        return WorldSceneDecodedRestorePlanStatus::PlanOutputCapacityExceeded;
    }

    if (plan_record_count > output_plan_capacity) {
        return WorldSceneDecodedRestorePlanStatus::PlanOutputCapacityExceeded;
    }

    return WorldSceneDecodedRestorePlanStatus::Success;
}

WorldSceneDecodedRestorePlanStatus WorldSceneDecodedRestorePlanBridge::ValidateDestinations(
    const WorldObjectIdentityBridge *identity_destination,
    const WorldTransformBridge *transform_destination,
    const WorldComponentAttachmentBridge *attachment_destination,
    const WorldComponentResourceBindingBridge *binding_destination,
    std::uint32_t input_identity_count,
    std::uint32_t input_transform_count,
    std::uint32_t input_attachment_count,
    std::uint32_t input_binding_count,
    WorldObjectIdentityStatus *out_identity_status,
    WorldTransformStatus *out_transform_status,
    WorldComponentAttachmentStatus *out_attachment_status,
    WorldComponentResourceBindingStatus *out_binding_status) const {
    if (identity_destination != nullptr) {
        const WorldObjectIdentitySnapshot identity_snapshot = identity_destination->Snapshot();
        if (identity_snapshot.bridge_capacity == 0U) {
            if (out_identity_status != nullptr) {
                *out_identity_status = WorldObjectIdentityStatus::InvalidBridgeCapacity;
            }

            return WorldSceneDecodedRestorePlanStatus::InvalidBridgeCapacity;
        }

        if (identity_snapshot.binding_count != 0U) {
            return WorldSceneDecodedRestorePlanStatus::DestinationNotEmpty;
        }

        if (input_identity_count > identity_snapshot.bridge_capacity) {
            if (out_identity_status != nullptr) {
                *out_identity_status = WorldObjectIdentityStatus::CapacityExceeded;
            }

            return WorldSceneDecodedRestorePlanStatus::IdentityCapacityExceeded;
        }
    }

    if (transform_destination != nullptr) {
        const WorldTransformSnapshot transform_snapshot = transform_destination->Snapshot();
        if (transform_snapshot.bridge_capacity == 0U) {
            if (out_transform_status != nullptr) {
                *out_transform_status = WorldTransformStatus::InvalidBridgeCapacity;
            }

            return WorldSceneDecodedRestorePlanStatus::InvalidBridgeCapacity;
        }

        if (transform_snapshot.record_count != 0U) {
            return WorldSceneDecodedRestorePlanStatus::DestinationNotEmpty;
        }

        if (input_transform_count > transform_snapshot.bridge_capacity) {
            if (out_transform_status != nullptr) {
                *out_transform_status = WorldTransformStatus::CapacityExceeded;
            }

            return WorldSceneDecodedRestorePlanStatus::TransformCapacityExceeded;
        }
    }

    if (attachment_destination != nullptr) {
        const WorldComponentAttachmentStatus attachment_status =
            attachment_destination->ValidateRestoreDestination(input_attachment_count);
        if (out_attachment_status != nullptr) {
            *out_attachment_status = attachment_status;
        }

        if (attachment_status != WorldComponentAttachmentStatus::Success) {
            return MapAttachmentDestinationStatus(attachment_status);
        }
    }

    if (binding_destination != nullptr) {
        const WorldComponentResourceBindingStatus binding_status =
            binding_destination->ValidateRestoreDestination(input_binding_count);
        if (out_binding_status != nullptr) {
            *out_binding_status = binding_status;
        }

        if (binding_status != WorldComponentResourceBindingStatus::Success) {
            return MapBindingDestinationStatus(binding_status);
        }
    }

    return WorldSceneDecodedRestorePlanStatus::Success;
}

WorldSceneDecodedRestorePlanStatus WorldSceneDecodedRestorePlanBridge::ValidateIdentityRecords(
    const WorldInstance &world,
    const ObjectRegistry &object_registry,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    ObjectStatus *out_object_status) const {
    std::uint32_t record_index = 0U;
    while (record_index < input_identity_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &identity = input_identities[record_index];
        if (!identity.world_object_id.IsValid()) {
            return WorldSceneDecodedRestorePlanStatus::InvalidWorldObjectId;
        }

        if (!world.ContainsObject(identity.world_object_id)) {
            return WorldSceneDecodedRestorePlanStatus::MissingWorldObject;
        }

        if (!identity.object_handle.IsValid()) {
            if (out_object_status != nullptr) {
                *out_object_status = ObjectStatus::InvalidHandle;
            }

            return WorldSceneDecodedRestorePlanStatus::InvalidObjectHandle;
        }

        if (HasDuplicateIdentityWorldObjectId(input_identities, input_identity_count, record_index)) {
            return WorldSceneDecodedRestorePlanStatus::DuplicateIdentityWorldObjectId;
        }

        if (HasDuplicateIdentityObjectHandle(input_identities, input_identity_count, record_index)) {
            return WorldSceneDecodedRestorePlanStatus::DuplicateIdentityObjectHandle;
        }

        const std::uint32_t projected_acquire_count = CountProjectedObjectAcquire(
            input_identities,
            input_identity_count,
            record_index);
        const ObjectStatus object_status = object_registry.ValidateAcquire(
            identity.object_handle,
            projected_acquire_count);
        if (object_status != ObjectStatus::Success) {
            if (out_object_status != nullptr) {
                *out_object_status = object_status;
            }

            return MapObjectStatus(object_status);
        }

        ++record_index;
    }

    return WorldSceneDecodedRestorePlanStatus::Success;
}

WorldSceneDecodedRestorePlanStatus WorldSceneDecodedRestorePlanBridge::ValidateTransformRecords(
    const WorldInstance &world,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < input_transform_count) {
        const WorldSceneObjectTransformRestoreTransformRecord &transform = input_transforms[record_index];
        if (!transform.world_object_id.IsValid()) {
            return WorldSceneDecodedRestorePlanStatus::InvalidWorldObjectId;
        }

        if (!world.ContainsObject(transform.world_object_id)) {
            return WorldSceneDecodedRestorePlanStatus::MissingWorldObject;
        }

        if (HasDuplicateTransformWorldObjectId(input_transforms, input_transform_count, record_index)) {
            return WorldSceneDecodedRestorePlanStatus::DuplicateTransformWorldObjectId;
        }

        if (!HasIdentityRecord(input_identities, input_identity_count, transform.world_object_id)) {
            return WorldSceneDecodedRestorePlanStatus::MissingIdentityForTransform;
        }

        ++record_index;
    }

    return WorldSceneDecodedRestorePlanStatus::Success;
}

WorldSceneDecodedRestorePlanStatus WorldSceneDecodedRestorePlanBridge::ValidateAttachmentRecords(
    const WorldInstance &world,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < input_attachment_count) {
        const WorldComponentAttachmentSnapshotRecord &attachment = input_attachments[record_index];
        if (!attachment.world_object_id.IsValid()) {
            return WorldSceneDecodedRestorePlanStatus::InvalidWorldObjectId;
        }

        if (!world.ContainsObject(attachment.world_object_id)) {
            return WorldSceneDecodedRestorePlanStatus::MissingWorldObject;
        }

        if (!attachment.component_type_id.IsValid()) {
            return WorldSceneDecodedRestorePlanStatus::InvalidComponentTypeId;
        }

        if (!attachment.component_slot_id.IsValid()) {
            return WorldSceneDecodedRestorePlanStatus::InvalidComponentSlotId;
        }

        if (!HasIdentityRecord(input_identities, input_identity_count, attachment.world_object_id)) {
            return WorldSceneDecodedRestorePlanStatus::MissingIdentityForAttachment;
        }

        if (HasDuplicateAttachmentTuple(input_attachments, record_index)) {
            return WorldSceneDecodedRestorePlanStatus::DuplicateAttachmentTuple;
        }

        ++record_index;
    }

    return WorldSceneDecodedRestorePlanStatus::Success;
}

WorldSceneDecodedRestorePlanStatus WorldSceneDecodedRestorePlanBridge::ValidateBindingRecords(
    const WorldInstance &world,
    const ResourceRegistry &resource_registry,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count,
    ResourceStatus *out_resource_status) const {
    std::uint32_t record_index = 0U;
    while (record_index < input_binding_count) {
        const WorldComponentResourceBindingSnapshotRecord &binding = input_bindings[record_index];
        if (!binding.world_object_id.IsValid()) {
            return WorldSceneDecodedRestorePlanStatus::InvalidWorldObjectId;
        }

        if (!world.ContainsObject(binding.world_object_id)) {
            return WorldSceneDecodedRestorePlanStatus::MissingWorldObject;
        }

        if (!binding.component_type_id.IsValid()) {
            return WorldSceneDecodedRestorePlanStatus::InvalidComponentTypeId;
        }

        if (!binding.component_slot_id.IsValid()) {
            return WorldSceneDecodedRestorePlanStatus::InvalidComponentSlotId;
        }

        if (!binding.resource_handle.IsValid()) {
            if (out_resource_status != nullptr) {
                *out_resource_status = ResourceStatus::InvalidHandle;
            }

            return WorldSceneDecodedRestorePlanStatus::InvalidResourceHandle;
        }

        if (!binding.expected_resource_type.IsValid()) {
            return WorldSceneDecodedRestorePlanStatus::InvalidResourceTypeId;
        }

        if (!HasIdentityRecord(input_identities, input_identity_count, binding.world_object_id)) {
            return WorldSceneDecodedRestorePlanStatus::MissingIdentityForBinding;
        }

        if (HasDuplicateBindingTuple(input_bindings, record_index)) {
            return WorldSceneDecodedRestorePlanStatus::DuplicateBindingTuple;
        }

        if (!HasAttachmentTuple(input_attachments, input_attachment_count, binding)) {
            return WorldSceneDecodedRestorePlanStatus::MissingAttachmentForBinding;
        }

        const std::uint32_t projected_acquire_count = CountProjectedResourceAcquire(
            input_bindings,
            input_binding_count,
            record_index);
        const ResourceStatus resource_status = resource_registry.ValidateAcquire(
            binding.resource_handle,
            binding.expected_resource_type,
            projected_acquire_count);
        if (resource_status != ResourceStatus::Success) {
            if (out_resource_status != nullptr) {
                *out_resource_status = resource_status;
            }

            return MapResourceStatus(resource_status);
        }

        ++record_index;
    }

    return WorldSceneDecodedRestorePlanStatus::Success;
}

bool WorldSceneDecodedRestorePlanBridge::HasDuplicateIdentityWorldObjectId(
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    std::uint32_t record_index) const {
    const WorldObjectId world_object_id = input_identities[record_index].world_object_id;
    std::uint32_t scan_index = 0U;
    while (scan_index < input_identity_count) {
        if (scan_index == record_index) {
            ++scan_index;
            continue;
        }

        const WorldSceneObjectTransformRestoreIdentityRecord &identity = input_identities[scan_index];
        if (identity.world_object_id.value == world_object_id.value) {
            return true;
        }

        ++scan_index;
    }

    return false;
}

bool WorldSceneDecodedRestorePlanBridge::HasDuplicateIdentityObjectHandle(
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    std::uint32_t record_index) const {
    const yuengine::object::ObjectHandle object_handle = input_identities[record_index].object_handle;
    std::uint32_t scan_index = 0U;
    while (scan_index < input_identity_count) {
        if (scan_index == record_index) {
            ++scan_index;
            continue;
        }

        const WorldSceneObjectTransformRestoreIdentityRecord &identity = input_identities[scan_index];
        if (identity.object_handle.slot != object_handle.slot) {
            ++scan_index;
            continue;
        }

        if (identity.object_handle.generation == object_handle.generation) {
            return true;
        }

        ++scan_index;
    }

    return false;
}

bool WorldSceneDecodedRestorePlanBridge::HasDuplicateTransformWorldObjectId(
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count,
    std::uint32_t record_index) const {
    const WorldObjectId world_object_id = input_transforms[record_index].world_object_id;
    std::uint32_t scan_index = 0U;
    while (scan_index < input_transform_count) {
        if (scan_index == record_index) {
            ++scan_index;
            continue;
        }

        const WorldSceneObjectTransformRestoreTransformRecord &transform = input_transforms[scan_index];
        if (transform.world_object_id.value == world_object_id.value) {
            return true;
        }

        ++scan_index;
    }

    return false;
}

bool WorldSceneDecodedRestorePlanBridge::HasDuplicateAttachmentTuple(
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t record_index) const {
    const WorldComponentAttachmentSnapshotRecord &attachment = input_attachments[record_index];
    std::uint32_t scan_index = 0U;
    while (scan_index < record_index) {
        const WorldComponentAttachmentSnapshotRecord &scan_attachment = input_attachments[scan_index];
        if (scan_attachment.world_object_id.value != attachment.world_object_id.value) {
            ++scan_index;
            continue;
        }

        if (scan_attachment.component_type_id.value == attachment.component_type_id.value) {
            return true;
        }

        ++scan_index;
    }

    return false;
}

bool WorldSceneDecodedRestorePlanBridge::HasDuplicateBindingTuple(
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t record_index) const {
    const WorldComponentResourceBindingSnapshotRecord &binding = input_bindings[record_index];
    std::uint32_t scan_index = 0U;
    while (scan_index < record_index) {
        const WorldComponentResourceBindingSnapshotRecord &scan_binding = input_bindings[scan_index];
        if (scan_binding.world_object_id.value != binding.world_object_id.value) {
            ++scan_index;
            continue;
        }

        if (scan_binding.component_type_id.value != binding.component_type_id.value) {
            ++scan_index;
            continue;
        }

        if (scan_binding.component_slot_id.value == binding.component_slot_id.value) {
            return true;
        }

        ++scan_index;
    }

    return false;
}

bool WorldSceneDecodedRestorePlanBridge::HasIdentityRecord(
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    WorldObjectId world_object_id) const {
    std::uint32_t record_index = 0U;
    while (record_index < input_identity_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &identity = input_identities[record_index];
        if (identity.world_object_id.value == world_object_id.value) {
            return true;
        }

        ++record_index;
    }

    return false;
}

bool WorldSceneDecodedRestorePlanBridge::HasAttachmentTuple(
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord &binding) const {
    std::uint32_t attachment_index = 0U;
    while (attachment_index < input_attachment_count) {
        const WorldComponentAttachmentSnapshotRecord &attachment = input_attachments[attachment_index];
        if (attachment.world_object_id.value != binding.world_object_id.value) {
            ++attachment_index;
            continue;
        }

        if (attachment.component_type_id.value != binding.component_type_id.value) {
            ++attachment_index;
            continue;
        }

        if (attachment.component_slot_id.value == binding.component_slot_id.value) {
            return true;
        }

        ++attachment_index;
    }

    return false;
}

std::uint32_t WorldSceneDecodedRestorePlanBridge::CountProjectedObjectAcquire(
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    std::uint32_t record_index) const {
    const yuengine::object::ObjectHandle object_handle = input_identities[record_index].object_handle;
    std::uint32_t projected_acquire_count = 0U;
    std::uint32_t scan_index = 0U;
    while (scan_index < input_identity_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &identity = input_identities[scan_index];
        if (identity.object_handle.slot != object_handle.slot) {
            ++scan_index;
            continue;
        }

        if (identity.object_handle.generation == object_handle.generation) {
            ++projected_acquire_count;
        }

        ++scan_index;
    }

    return projected_acquire_count;
}

std::uint32_t WorldSceneDecodedRestorePlanBridge::CountProjectedResourceAcquire(
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count,
    std::uint32_t record_index) const {
    const yuengine::resource::ResourceHandle resource_handle = input_bindings[record_index].resource_handle;
    std::uint32_t projected_acquire_count = 0U;
    std::uint32_t scan_index = 0U;
    while (scan_index < input_binding_count) {
        const WorldComponentResourceBindingSnapshotRecord &binding = input_bindings[scan_index];
        if (binding.resource_handle.slot != resource_handle.slot) {
            ++scan_index;
            continue;
        }

        if (binding.resource_handle.generation == resource_handle.generation) {
            ++projected_acquire_count;
        }

        ++scan_index;
    }

    return projected_acquire_count;
}

WorldSceneDecodedRestorePlanStatus WorldSceneDecodedRestorePlanBridge::MapObjectStatus(
    ObjectStatus object_status) const {
    if (object_status == ObjectStatus::InvalidHandle) {
        return WorldSceneDecodedRestorePlanStatus::InvalidObjectHandle;
    }

    if (object_status == ObjectStatus::GenerationMismatch) {
        return WorldSceneDecodedRestorePlanStatus::StaleObjectHandle;
    }

    if (object_status == ObjectStatus::ReferenceCountOverflow) {
        return WorldSceneDecodedRestorePlanStatus::ObjectAcquireWouldOverflow;
    }

    return WorldSceneDecodedRestorePlanStatus::ObjectAcquireFailed;
}

WorldSceneDecodedRestorePlanStatus WorldSceneDecodedRestorePlanBridge::MapResourceStatus(
    ResourceStatus resource_status) const {
    if (resource_status == ResourceStatus::InvalidHandle) {
        return WorldSceneDecodedRestorePlanStatus::InvalidResourceHandle;
    }

    if (resource_status == ResourceStatus::GenerationMismatch) {
        return WorldSceneDecodedRestorePlanStatus::StaleResourceHandle;
    }

    if (resource_status == ResourceStatus::TypeMismatch) {
        return WorldSceneDecodedRestorePlanStatus::ResourceTypeMismatch;
    }

    if (resource_status == ResourceStatus::ReferenceCountOverflow) {
        return WorldSceneDecodedRestorePlanStatus::ResourceAcquireWouldOverflow;
    }

    return WorldSceneDecodedRestorePlanStatus::ResourceAcquireFailed;
}

WorldSceneDecodedRestorePlanStatus WorldSceneDecodedRestorePlanBridge::MapAttachmentDestinationStatus(
    WorldComponentAttachmentStatus attachment_status) const {
    if (attachment_status == WorldComponentAttachmentStatus::InvalidBridgeCapacity) {
        return WorldSceneDecodedRestorePlanStatus::InvalidBridgeCapacity;
    }

    if (attachment_status == WorldComponentAttachmentStatus::CapacityExceeded) {
        return WorldSceneDecodedRestorePlanStatus::AttachmentCapacityExceeded;
    }

    if (attachment_status == WorldComponentAttachmentStatus::DuplicateAttachment) {
        return WorldSceneDecodedRestorePlanStatus::DestinationNotEmpty;
    }

    return WorldSceneDecodedRestorePlanStatus::InvalidAttachmentDestination;
}

WorldSceneDecodedRestorePlanStatus WorldSceneDecodedRestorePlanBridge::MapBindingDestinationStatus(
    WorldComponentResourceBindingStatus binding_status) const {
    if (binding_status == WorldComponentResourceBindingStatus::InvalidBridgeCapacity) {
        return WorldSceneDecodedRestorePlanStatus::InvalidBridgeCapacity;
    }

    if (binding_status == WorldComponentResourceBindingStatus::CapacityExceeded) {
        return WorldSceneDecodedRestorePlanStatus::BindingCapacityExceeded;
    }

    if (binding_status == WorldComponentResourceBindingStatus::DuplicateComponentBinding) {
        return WorldSceneDecodedRestorePlanStatus::DestinationNotEmpty;
    }

    return WorldSceneDecodedRestorePlanStatus::InvalidBindingDestination;
}

void WorldSceneDecodedRestorePlanBridge::WritePlanRecords(
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count,
    WorldSceneDecodedRestorePlanRecord *output_plan,
    WorldSceneDecodedRestorePlanState *state) const {
    std::uint32_t output_index = 0U;
    std::uint32_t record_index = 0U;
    while (record_index < input_identity_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &identity = input_identities[record_index];
        WorldSceneDecodedRestorePlanRecord &record = output_plan[output_index];
        record = WorldSceneDecodedRestorePlanRecord{};
        record.family = WorldSceneDecodedRestorePlanRecordFamily::Identity;
        record.input_index = record_index;
        record.world_object_id = identity.world_object_id;
        record.object_handle = identity.object_handle;
        record.projected_object_acquire_count = CountProjectedObjectAcquire(
            input_identities,
            input_identity_count,
            record_index);
        record.status = WorldSceneDecodedRestorePlanStatus::Success;
        if (state != nullptr) {
            ++state->planned_identity_count;
            ++state->projected_object_acquire_count;
        }

        ++output_index;
        ++record_index;
    }

    record_index = 0U;
    while (record_index < input_transform_count) {
        const WorldSceneObjectTransformRestoreTransformRecord &transform = input_transforms[record_index];
        WorldSceneDecodedRestorePlanRecord &record = output_plan[output_index];
        record = WorldSceneDecodedRestorePlanRecord{};
        record.family = WorldSceneDecodedRestorePlanRecordFamily::Transform;
        record.input_index = record_index;
        record.world_object_id = transform.world_object_id;
        record.status = WorldSceneDecodedRestorePlanStatus::Success;
        if (state != nullptr) {
            ++state->planned_transform_count;
        }

        ++output_index;
        ++record_index;
    }

    record_index = 0U;
    while (record_index < input_attachment_count) {
        const WorldComponentAttachmentSnapshotRecord &attachment = input_attachments[record_index];
        WorldSceneDecodedRestorePlanRecord &record = output_plan[output_index];
        record = WorldSceneDecodedRestorePlanRecord{};
        record.family = WorldSceneDecodedRestorePlanRecordFamily::Attachment;
        record.input_index = record_index;
        record.world_object_id = attachment.world_object_id;
        record.component_type_id = attachment.component_type_id;
        record.component_slot_id = attachment.component_slot_id;
        record.status = WorldSceneDecodedRestorePlanStatus::Success;
        if (state != nullptr) {
            ++state->planned_attachment_count;
        }

        ++output_index;
        ++record_index;
    }

    record_index = 0U;
    while (record_index < input_binding_count) {
        const WorldComponentResourceBindingSnapshotRecord &binding = input_bindings[record_index];
        WorldSceneDecodedRestorePlanRecord &record = output_plan[output_index];
        record = WorldSceneDecodedRestorePlanRecord{};
        record.family = WorldSceneDecodedRestorePlanRecordFamily::Binding;
        record.input_index = record_index;
        record.world_object_id = binding.world_object_id;
        record.component_type_id = binding.component_type_id;
        record.component_slot_id = binding.component_slot_id;
        record.resource_handle = binding.resource_handle;
        record.expected_resource_type = binding.expected_resource_type;
        record.projected_resource_acquire_count = CountProjectedResourceAcquire(
            input_bindings,
            input_binding_count,
            record_index);
        record.status = WorldSceneDecodedRestorePlanStatus::Success;
        if (state != nullptr) {
            ++state->planned_binding_count;
            ++state->projected_resource_acquire_count;
        }

        ++output_index;
        ++record_index;
    }
}
}
