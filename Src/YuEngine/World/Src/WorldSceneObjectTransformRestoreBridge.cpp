// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldSceneObjectTransformRestoreBridge.cpp

#include "YuEngine/World/WorldSceneObjectTransformRestoreBridge.h"

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectRegistry.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectIdentityBridge.h"
#include "YuEngine/World/WorldObjectIdentityResult.h"
#include "YuEngine/World/WorldObjectIdentitySnapshot.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformSnapshot.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::object::ObjectRegistry;
using yuengine::object::ObjectStatus;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity) {
    if (requested_capacity > MAX_WORLD_OBJECT_COUNT) {
        return MAX_WORLD_OBJECT_COUNT;
    }

    return requested_capacity;
}
}

WorldSceneObjectTransformRestoreBridge::WorldSceneObjectTransformRestoreBridge(
    WorldSceneObjectTransformRestoreBridgeDesc desc)
    : identity_capacity_(ClampCapacity(desc.identity_capacity)),
      transform_capacity_(ClampCapacity(desc.transform_capacity)),
      snapshot_{
          ClampCapacity(desc.identity_capacity),
          ClampCapacity(desc.transform_capacity),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          WorldObjectIdentityStatus::Success,
          WorldTransformStatus::Success,
          ObjectStatus::Success,
          WorldSceneObjectTransformRestoreStatus::Success} {
    if ((desc.identity_capacity == 0U) || (desc.transform_capacity == 0U)) {
        snapshot_.last_status = WorldSceneObjectTransformRestoreStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldSceneObjectTransformRestoreResult WorldSceneObjectTransformRestoreBridge::Restore(
    WorldInstance *world,
    ObjectRegistry *object_registry,
    WorldObjectIdentityBridge *identity_destination,
    WorldTransformBridge *transform_destination,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count) {
    ++snapshot_.restore_attempt_count;

    const WorldSceneObjectTransformRestoreStatus bridge_status = ValidateBridgeCapacity();
    if (bridge_status != WorldSceneObjectTransformRestoreStatus::Success) {
        return RecordFailure(bridge_status);
    }

    if (world == nullptr) {
        return RecordFailure(WorldSceneObjectTransformRestoreStatus::InvalidWorld);
    }

    if (object_registry == nullptr) {
        return RecordFailure(WorldSceneObjectTransformRestoreStatus::InvalidObjectRegistry);
    }

    if (identity_destination == nullptr) {
        return RecordFailure(WorldSceneObjectTransformRestoreStatus::InvalidIdentityDestination);
    }

    if (transform_destination == nullptr) {
        return RecordFailure(WorldSceneObjectTransformRestoreStatus::InvalidTransformDestination);
    }

    if (input_identities == nullptr) {
        return RecordFailure(WorldSceneObjectTransformRestoreStatus::InvalidIdentityInput);
    }

    if (input_transforms == nullptr) {
        return RecordFailure(WorldSceneObjectTransformRestoreStatus::InvalidTransformInput);
    }

    if (input_identity_count > identity_capacity_) {
        return RecordRejectedFailure(WorldSceneObjectTransformRestoreStatus::IdentityCapacityExceeded);
    }

    if (input_transform_count > transform_capacity_) {
        return RecordRejectedFailure(WorldSceneObjectTransformRestoreStatus::TransformCapacityExceeded);
    }

    WorldObjectIdentityStatus identity_status = WorldObjectIdentityStatus::Success;
    WorldTransformStatus transform_status = WorldTransformStatus::Success;
    const WorldSceneObjectTransformRestoreStatus destination_status = ValidateDestinations(
        *identity_destination,
        *transform_destination,
        input_identity_count,
        input_transform_count,
        &identity_status,
        &transform_status);
    if (destination_status != WorldSceneObjectTransformRestoreStatus::Success) {
        return RecordFailure(destination_status, identity_status, transform_status, ObjectStatus::Success);
    }

    ObjectStatus object_status = ObjectStatus::Success;
    const WorldSceneObjectTransformRestoreStatus identity_record_status = ValidateIdentityRecords(
        *world,
        *object_registry,
        input_identities,
        input_identity_count,
        &object_status);
    if (identity_record_status != WorldSceneObjectTransformRestoreStatus::Success) {
        return RecordRejectedFailure(identity_record_status, object_status);
    }

    const WorldSceneObjectTransformRestoreStatus transform_record_status = ValidateTransformRecords(
        *world,
        input_identities,
        input_identity_count,
        input_transforms,
        input_transform_count);
    if (transform_record_status != WorldSceneObjectTransformRestoreStatus::Success) {
        return RecordRejectedFailure(transform_record_status);
    }

    WorldSceneObjectTransformRestoreState state{};
    state.input_identity_count = input_identity_count;
    state.input_transform_count = input_transform_count;

    WorldSceneObjectTransformRestoreResult identity_result = ApplyIdentities(
        identity_destination,
        input_identities,
        input_identity_count,
        &state);
    if (!identity_result.Succeeded()) {
        return identity_result;
    }

    return ApplyTransforms(
        transform_destination,
        input_transforms,
        input_transform_count,
        &state);
}

WorldSceneObjectTransformRestoreSnapshot WorldSceneObjectTransformRestoreBridge::Snapshot() const {
    return snapshot_;
}

WorldSceneObjectTransformRestoreResult WorldSceneObjectTransformRestoreBridge::RecordFailure(
    WorldSceneObjectTransformRestoreStatus status) {
    return RecordFailure(
        status,
        WorldObjectIdentityStatus::Success,
        WorldTransformStatus::Success,
        ObjectStatus::Success);
}

WorldSceneObjectTransformRestoreResult WorldSceneObjectTransformRestoreBridge::RecordFailure(
    WorldSceneObjectTransformRestoreStatus status,
    WorldObjectIdentityStatus identity_status) {
    return RecordFailure(status, identity_status, WorldTransformStatus::Success, ObjectStatus::Success);
}

WorldSceneObjectTransformRestoreResult WorldSceneObjectTransformRestoreBridge::RecordFailure(
    WorldSceneObjectTransformRestoreStatus status,
    WorldTransformStatus transform_status) {
    return RecordFailure(status, WorldObjectIdentityStatus::Success, transform_status, ObjectStatus::Success);
}

WorldSceneObjectTransformRestoreResult WorldSceneObjectTransformRestoreBridge::RecordFailure(
    WorldSceneObjectTransformRestoreStatus status,
    ObjectStatus object_status) {
    return RecordFailure(
        status,
        WorldObjectIdentityStatus::Success,
        WorldTransformStatus::Success,
        object_status);
}

WorldSceneObjectTransformRestoreResult WorldSceneObjectTransformRestoreBridge::RecordFailure(
    WorldSceneObjectTransformRestoreStatus status,
    WorldObjectIdentityStatus identity_status,
    WorldTransformStatus transform_status,
    ObjectStatus object_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_identity_status = identity_status;
    snapshot_.last_transform_status = transform_status;
    snapshot_.last_object_status = object_status;
    return WorldSceneObjectTransformRestoreResult::Failure(
        status,
        identity_status,
        transform_status,
        object_status);
}

WorldSceneObjectTransformRestoreResult WorldSceneObjectTransformRestoreBridge::RecordRejectedFailure(
    WorldSceneObjectTransformRestoreStatus status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status);
}

WorldSceneObjectTransformRestoreResult WorldSceneObjectTransformRestoreBridge::RecordRejectedFailure(
    WorldSceneObjectTransformRestoreStatus status,
    ObjectStatus object_status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status, object_status);
}

WorldSceneObjectTransformRestoreResult WorldSceneObjectTransformRestoreBridge::RecordSuccess(
    const WorldSceneObjectTransformRestoreState &state) {
    snapshot_.restored_identity_count += state.restored_identity_count;
    snapshot_.restored_transform_count += state.restored_transform_count;
    snapshot_.last_status = WorldSceneObjectTransformRestoreStatus::Success;
    snapshot_.last_identity_status = WorldObjectIdentityStatus::Success;
    snapshot_.last_transform_status = WorldTransformStatus::Success;
    snapshot_.last_object_status = ObjectStatus::Success;
    return WorldSceneObjectTransformRestoreResult::Success(state);
}

WorldSceneObjectTransformRestoreStatus WorldSceneObjectTransformRestoreBridge::ValidateBridgeCapacity() const {
    if ((identity_capacity_ == 0U) || (transform_capacity_ == 0U)) {
        return WorldSceneObjectTransformRestoreStatus::InvalidBridgeCapacity;
    }

    return WorldSceneObjectTransformRestoreStatus::Success;
}

WorldSceneObjectTransformRestoreStatus WorldSceneObjectTransformRestoreBridge::ValidateDestinations(
    const WorldObjectIdentityBridge &identity_destination,
    const WorldTransformBridge &transform_destination,
    std::uint32_t input_identity_count,
    std::uint32_t input_transform_count,
    WorldObjectIdentityStatus *out_identity_status,
    WorldTransformStatus *out_transform_status) const {
    const WorldObjectIdentitySnapshot identity_snapshot = identity_destination.Snapshot();
    const WorldTransformSnapshot transform_snapshot = transform_destination.Snapshot();
    if (identity_snapshot.bridge_capacity == 0U) {
        if (out_identity_status != nullptr) {
            *out_identity_status = WorldObjectIdentityStatus::InvalidBridgeCapacity;
        }

        return WorldSceneObjectTransformRestoreStatus::InvalidBridgeCapacity;
    }

    if (transform_snapshot.bridge_capacity == 0U) {
        if (out_transform_status != nullptr) {
            *out_transform_status = WorldTransformStatus::InvalidBridgeCapacity;
        }

        return WorldSceneObjectTransformRestoreStatus::InvalidBridgeCapacity;
    }

    if (identity_snapshot.binding_count != 0U) {
        return WorldSceneObjectTransformRestoreStatus::DestinationNotEmpty;
    }

    if (transform_snapshot.record_count != 0U) {
        return WorldSceneObjectTransformRestoreStatus::DestinationNotEmpty;
    }

    if (input_identity_count > identity_snapshot.bridge_capacity) {
        if (out_identity_status != nullptr) {
            *out_identity_status = WorldObjectIdentityStatus::CapacityExceeded;
        }

        return WorldSceneObjectTransformRestoreStatus::IdentityCapacityExceeded;
    }

    if (input_transform_count > transform_snapshot.bridge_capacity) {
        if (out_transform_status != nullptr) {
            *out_transform_status = WorldTransformStatus::CapacityExceeded;
        }

        return WorldSceneObjectTransformRestoreStatus::TransformCapacityExceeded;
    }

    return WorldSceneObjectTransformRestoreStatus::Success;
}

WorldSceneObjectTransformRestoreStatus WorldSceneObjectTransformRestoreBridge::ValidateIdentityRecords(
    const WorldInstance &world,
    const ObjectRegistry &object_registry,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    ObjectStatus *out_object_status) const {
    std::uint32_t record_index = 0U;
    while (record_index < input_identity_count) {
        const WorldSceneObjectTransformRestoreStatus status = ValidateIdentityRecord(
            world,
            object_registry,
            input_identities,
            input_identity_count,
            record_index,
            out_object_status);
        if (status != WorldSceneObjectTransformRestoreStatus::Success) {
            return status;
        }

        ++record_index;
    }

    return WorldSceneObjectTransformRestoreStatus::Success;
}

WorldSceneObjectTransformRestoreStatus WorldSceneObjectTransformRestoreBridge::ValidateIdentityRecord(
    const WorldInstance &world,
    const ObjectRegistry &object_registry,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    std::uint32_t record_index,
    ObjectStatus *out_object_status) const {
    const WorldSceneObjectTransformRestoreIdentityRecord &identity = input_identities[record_index];
    if (!identity.world_object_id.IsValid()) {
        return WorldSceneObjectTransformRestoreStatus::InvalidWorldObjectId;
    }

    if (!world.ContainsObject(identity.world_object_id)) {
        return WorldSceneObjectTransformRestoreStatus::MissingWorldObject;
    }

    if (!identity.object_handle.IsValid()) {
        if (out_object_status != nullptr) {
            *out_object_status = ObjectStatus::InvalidHandle;
        }

        return WorldSceneObjectTransformRestoreStatus::InvalidObjectHandle;
    }

    if (HasDuplicateIdentityWorldObjectId(input_identities, input_identity_count, record_index)) {
        return WorldSceneObjectTransformRestoreStatus::DuplicateIdentityWorldObjectId;
    }

    if (HasDuplicateIdentityObjectHandle(input_identities, input_identity_count, record_index)) {
        return WorldSceneObjectTransformRestoreStatus::DuplicateIdentityObjectHandle;
    }

    const std::uint32_t projected_acquire_count = CountProjectedAcquire(
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

    return WorldSceneObjectTransformRestoreStatus::Success;
}

WorldSceneObjectTransformRestoreStatus WorldSceneObjectTransformRestoreBridge::ValidateTransformRecords(
    const WorldInstance &world,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < input_transform_count) {
        const WorldSceneObjectTransformRestoreStatus status = ValidateTransformRecord(
            world,
            input_identities,
            input_identity_count,
            input_transforms,
            input_transform_count,
            record_index);
        if (status != WorldSceneObjectTransformRestoreStatus::Success) {
            return status;
        }

        ++record_index;
    }

    return WorldSceneObjectTransformRestoreStatus::Success;
}

WorldSceneObjectTransformRestoreStatus WorldSceneObjectTransformRestoreBridge::ValidateTransformRecord(
    const WorldInstance &world,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreTransformRecord &transform = input_transforms[record_index];
    if (!transform.world_object_id.IsValid()) {
        return WorldSceneObjectTransformRestoreStatus::InvalidWorldObjectId;
    }

    if (!world.ContainsObject(transform.world_object_id)) {
        return WorldSceneObjectTransformRestoreStatus::MissingWorldObject;
    }

    if (HasDuplicateTransformWorldObjectId(input_transforms, input_transform_count, record_index)) {
        return WorldSceneObjectTransformRestoreStatus::DuplicateTransformWorldObjectId;
    }

    if (!HasIdentityRecord(input_identities, input_identity_count, transform.world_object_id)) {
        return WorldSceneObjectTransformRestoreStatus::MissingIdentityForTransform;
    }

    return WorldSceneObjectTransformRestoreStatus::Success;
}

bool WorldSceneObjectTransformRestoreBridge::HasDuplicateIdentityWorldObjectId(
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

bool WorldSceneObjectTransformRestoreBridge::HasDuplicateIdentityObjectHandle(
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

bool WorldSceneObjectTransformRestoreBridge::HasDuplicateTransformWorldObjectId(
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

bool WorldSceneObjectTransformRestoreBridge::HasIdentityRecord(
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

std::uint32_t WorldSceneObjectTransformRestoreBridge::CountProjectedAcquire(
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    std::uint32_t record_index) const {
    const yuengine::object::ObjectHandle object_handle = input_identities[record_index].object_handle;
    std::uint32_t acquire_count = 0U;
    std::uint32_t scan_index = 0U;
    while (scan_index < input_identity_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &identity = input_identities[scan_index];
        if (identity.object_handle.slot != object_handle.slot) {
            ++scan_index;
            continue;
        }

        if (identity.object_handle.generation == object_handle.generation) {
            ++acquire_count;
        }

        ++scan_index;
    }

    return acquire_count;
}

WorldSceneObjectTransformRestoreStatus WorldSceneObjectTransformRestoreBridge::MapObjectStatus(
    ObjectStatus object_status) const {
    if (object_status == ObjectStatus::InvalidHandle) {
        return WorldSceneObjectTransformRestoreStatus::InvalidObjectHandle;
    }

    if (object_status == ObjectStatus::GenerationMismatch) {
        return WorldSceneObjectTransformRestoreStatus::StaleObjectHandle;
    }

    if (object_status == ObjectStatus::ReferenceCountOverflow) {
        return WorldSceneObjectTransformRestoreStatus::ObjectAcquireWouldOverflow;
    }

    return WorldSceneObjectTransformRestoreStatus::ObjectAcquireFailed;
}

WorldSceneObjectTransformRestoreStatus WorldSceneObjectTransformRestoreBridge::MapIdentityStatus(
    WorldObjectIdentityStatus identity_status) const {
    if (identity_status == WorldObjectIdentityStatus::InvalidBridgeCapacity) {
        return WorldSceneObjectTransformRestoreStatus::InvalidBridgeCapacity;
    }

    if (identity_status == WorldObjectIdentityStatus::InvalidWorldObjectId) {
        return WorldSceneObjectTransformRestoreStatus::InvalidWorldObjectId;
    }

    if (identity_status == WorldObjectIdentityStatus::MissingWorldObject) {
        return WorldSceneObjectTransformRestoreStatus::MissingWorldObject;
    }

    if (identity_status == WorldObjectIdentityStatus::InvalidObjectHandle) {
        return WorldSceneObjectTransformRestoreStatus::InvalidObjectHandle;
    }

    if (identity_status == WorldObjectIdentityStatus::StaleObjectHandle) {
        return WorldSceneObjectTransformRestoreStatus::StaleObjectHandle;
    }

    if (identity_status == WorldObjectIdentityStatus::DuplicateWorldObjectId) {
        return WorldSceneObjectTransformRestoreStatus::DuplicateIdentityWorldObjectId;
    }

    if (identity_status == WorldObjectIdentityStatus::DuplicateObjectHandle) {
        return WorldSceneObjectTransformRestoreStatus::DuplicateIdentityObjectHandle;
    }

    if (identity_status == WorldObjectIdentityStatus::CapacityExceeded) {
        return WorldSceneObjectTransformRestoreStatus::IdentityCapacityExceeded;
    }

    return WorldSceneObjectTransformRestoreStatus::IdentityApplyFailed;
}

WorldSceneObjectTransformRestoreStatus WorldSceneObjectTransformRestoreBridge::MapTransformStatus(
    WorldTransformStatus transform_status) const {
    if (transform_status == WorldTransformStatus::InvalidBridgeCapacity) {
        return WorldSceneObjectTransformRestoreStatus::InvalidBridgeCapacity;
    }

    if (transform_status == WorldTransformStatus::InvalidWorldObjectId) {
        return WorldSceneObjectTransformRestoreStatus::InvalidWorldObjectId;
    }

    if (transform_status == WorldTransformStatus::MissingWorldObject) {
        return WorldSceneObjectTransformRestoreStatus::MissingWorldObject;
    }

    if (transform_status == WorldTransformStatus::DuplicateWorldObjectId) {
        return WorldSceneObjectTransformRestoreStatus::DuplicateTransformWorldObjectId;
    }

    if (transform_status == WorldTransformStatus::CapacityExceeded) {
        return WorldSceneObjectTransformRestoreStatus::TransformCapacityExceeded;
    }

    return WorldSceneObjectTransformRestoreStatus::TransformApplyFailed;
}

WorldSceneObjectTransformRestoreResult WorldSceneObjectTransformRestoreBridge::ApplyIdentities(
    WorldObjectIdentityBridge *identity_destination,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    WorldSceneObjectTransformRestoreState *state) {
    std::uint32_t record_index = 0U;
    while (record_index < input_identity_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &identity = input_identities[record_index];
        const WorldObjectIdentityResult identity_result = identity_destination->Bind(
            identity.world_object_id,
            identity.object_handle);
        if (!identity_result.Succeeded()) {
            const WorldSceneObjectTransformRestoreStatus restore_status =
                MapIdentityStatus(identity_result.status);
            return RecordFailure(restore_status, identity_result.status);
        }

        ++state->restored_identity_count;
        ++record_index;
    }

    return WorldSceneObjectTransformRestoreResult::Success(*state);
}

WorldSceneObjectTransformRestoreResult WorldSceneObjectTransformRestoreBridge::ApplyTransforms(
    WorldTransformBridge *transform_destination,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count,
    WorldSceneObjectTransformRestoreState *state) {
    std::uint32_t record_index = 0U;
    while (record_index < input_transform_count) {
        const WorldSceneObjectTransformRestoreTransformRecord &transform = input_transforms[record_index];
        const WorldTransformResult transform_result = transform_destination->Register(
            transform.world_object_id,
            transform.transform_state);
        if (!transform_result.Succeeded()) {
            const WorldSceneObjectTransformRestoreStatus restore_status =
                MapTransformStatus(transform_result.status);
            return RecordFailure(restore_status, transform_result.status);
        }

        ++state->restored_transform_count;
        ++record_index;
    }

    return RecordSuccess(*state);
}
}
