// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldIdentityBaseline.cpp

#include "YuEngine/World/WorldIdentityBaseline.h"

#include "YuEngine/Object/ObjectRegistrationResult.h"
#include "YuEngine/World/WorldComponentAttachmentResult.h"
#include "YuEngine/World/WorldObjectDesc.h"
#include "YuEngine/World/WorldObjectIdentityResult.h"
#include "YuEngine/World/WorldRegistrationResult.h"
#include "YuEngine/World/WorldTransformResult.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::object::ObjectHandle;
using yuengine::object::ObjectRegistrationResult;
using yuengine::object::ObjectStatus;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

WorldIdentityBaseline::WorldIdentityBaseline(WorldIdentityBaselineDesc desc)
    : object_registry_(desc.object_registry_desc),
      world_(desc.world_desc),
      identity_bridge_(world_, object_registry_, desc.identity_bridge_desc),
      transform_bridge_(world_, desc.transform_bridge_desc),
      component_bridge_(desc.component_attachment_desc),
      records_{},
      snapshot_{
          ClampCapacity(desc.record_capacity, MAX_WORLD_OBJECT_COUNT),
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ObjectStatus::Success,
          WorldStatus::Success,
          WorldObjectIdentityStatus::Success,
          WorldTransformStatus::Success,
          WorldComponentAttachmentStatus::Success,
          WorldIdentityBaselineStatus::Success} {
    if (desc.record_capacity == 0U) {
        snapshot_.last_status = WorldIdentityBaselineStatus::InvalidBaselineCapacity;
    }
}

WorldIdentityBaselineResult WorldIdentityBaseline::CreateObject(const WorldIdentityBaselineObjectDesc &desc) {
    const WorldIdentityBaselineStatus desc_status = ValidateObjectDesc(desc);
    if (desc_status != WorldIdentityBaselineStatus::Success) {
        return RecordFailureResult(desc_status);
    }

    ObjectRegistrationResult object_result = object_registry_.CreateSyntheticObject(desc.object_descriptor);
    snapshot_.last_object_status = object_result.status;
    if (!object_result.Succeeded()) {
        return RecordFailureResult(WorldIdentityBaselineStatus::ObjectCreateFailed);
    }

    WorldObjectDesc world_object_desc{};
    world_object_desc.id = desc.world_object_id;
    world_object_desc.is_enabled = desc.is_enabled;
    const WorldRegistrationResult world_result = world_.RegisterObject(world_object_desc);
    snapshot_.last_world_status = world_result.status;
    if (!world_result.Succeeded()) {
        RollbackObjectHandle(object_result.handle);
        return RecordFailureResult(WorldIdentityBaselineStatus::WorldRegisterFailed);
    }

    const WorldObjectIdentityResult identity_result = identity_bridge_.Bind(
        desc.world_object_id,
        object_result.handle);
    snapshot_.last_identity_status = identity_result.status;
    if (!identity_result.Succeeded()) {
        RollbackWorldObject(desc.world_object_id);
        RollbackObjectHandle(object_result.handle);
        return RecordFailureResult(WorldIdentityBaselineStatus::IdentityBindFailed);
    }

    const WorldTransformResult transform_result = transform_bridge_.Register(
        desc.world_object_id,
        desc.transform_state);
    snapshot_.last_transform_status = transform_result.status;
    if (!transform_result.Succeeded()) {
        RollbackIdentity(desc.world_object_id);
        RollbackWorldObject(desc.world_object_id);
        RollbackObjectHandle(object_result.handle);
        return RecordFailureResult(WorldIdentityBaselineStatus::TransformRegisterFailed);
    }

    const WorldComponentAttachmentResult component_result = component_bridge_.Add(
        desc.world_object_id,
        desc.component_type_id,
        desc.component_slot_id);
    snapshot_.last_component_status = component_result.status;
    if (!component_result.Succeeded()) {
        RollbackTransform(desc.world_object_id);
        RollbackIdentity(desc.world_object_id);
        RollbackWorldObject(desc.world_object_id);
        RollbackObjectHandle(object_result.handle);
        return RecordFailureResult(WorldIdentityBaselineStatus::ComponentAttachFailed);
    }

    WorldIdentityBaselineRecord *record = FindFreeRecord();
    if (record == nullptr) {
        RollbackComponent(desc.world_object_id, desc.component_type_id);
        RollbackCreatedObject(desc.world_object_id, object_result.handle);
        return RecordFailureResult(WorldIdentityBaselineStatus::CapacityExceeded);
    }

    record->world_object_id = desc.world_object_id;
    record->object_handle = object_result.handle;
    record->component_type_id = desc.component_type_id;
    record->component_slot_id = desc.component_slot_id;
    record->transform_state = desc.transform_state;
    record->is_active = true;
    ++snapshot_.active_record_count;
    ++snapshot_.created_record_count;
    RecordSuccess();
    return WorldIdentityBaselineResult::Success(*record);
}

WorldIdentityBaselineResult WorldIdentityBaseline::QueryObject(WorldObjectId world_object_id) {
    const WorldIdentityBaselineStatus capacity_status = ValidateBaselineCapacity();
    if (capacity_status != WorldIdentityBaselineStatus::Success) {
        return RecordFailureResult(capacity_status);
    }

    if (!world_object_id.IsValid()) {
        return RecordFailureResult(WorldIdentityBaselineStatus::InvalidWorldObjectId);
    }

    const WorldIdentityBaselineRecord *record = FindRecordByWorldObjectId(world_object_id);
    if (record == nullptr) {
        return RecordFailureResult(WorldIdentityBaselineStatus::RecordNotFound);
    }

    if (!world_.ContainsObject(world_object_id)) {
        return RecordFailureResult(WorldIdentityBaselineStatus::WorldObjectMissing);
    }

    const ObjectStatus object_status = object_registry_.Validate(record->object_handle);
    snapshot_.last_object_status = object_status;
    if (object_status != ObjectStatus::Success) {
        return RecordFailureResult(WorldIdentityBaselineStatus::ObjectValidateFailed);
    }

    const WorldObjectIdentityStatus identity_status = identity_bridge_.Validate(world_object_id);
    snapshot_.last_identity_status = identity_status;
    if (identity_status != WorldObjectIdentityStatus::Success) {
        return RecordFailureResult(WorldIdentityBaselineStatus::IdentityValidateFailed);
    }

    const WorldTransformResult transform_result = transform_bridge_.Query(world_object_id);
    snapshot_.last_transform_status = transform_result.status;
    if (!transform_result.Succeeded()) {
        return RecordFailureResult(WorldIdentityBaselineStatus::TransformValidateFailed);
    }

    const WorldComponentAttachmentResult component_result = component_bridge_.Query(
        world_object_id,
        record->component_type_id);
    snapshot_.last_component_status = component_result.status;
    if (!component_result.Succeeded()) {
        return RecordFailureResult(WorldIdentityBaselineStatus::ComponentValidateFailed);
    }

    RecordSuccess();
    return WorldIdentityBaselineResult::Success(*record);
}

WorldIdentityBaselineStatus WorldIdentityBaseline::DestroyObject(WorldObjectId world_object_id) {
    WorldIdentityBaselineRecord *record = nullptr;
    const WorldIdentityBaselineStatus validate_status = ValidateRecordForMutation(world_object_id, record);
    if (validate_status != WorldIdentityBaselineStatus::Success) {
        return RecordFailure(validate_status);
    }

    const WorldComponentAttachmentStatus component_status = component_bridge_.Remove(
        world_object_id,
        record->component_type_id);
    snapshot_.last_component_status = component_status;
    if (component_status != WorldComponentAttachmentStatus::Success) {
        return RecordFailure(WorldIdentityBaselineStatus::ComponentRemoveFailed);
    }

    const WorldTransformStatus transform_status = transform_bridge_.Remove(world_object_id);
    snapshot_.last_transform_status = transform_status;
    if (transform_status != WorldTransformStatus::Success) {
        return RecordFailure(WorldIdentityBaselineStatus::TransformRemoveFailed);
    }

    const WorldObjectIdentityStatus identity_status = identity_bridge_.Remove(world_object_id);
    snapshot_.last_identity_status = identity_status;
    if (identity_status != WorldObjectIdentityStatus::Success) {
        return RecordFailure(WorldIdentityBaselineStatus::IdentityRemoveFailed);
    }

    const WorldStatus world_status = world_.RemoveObject(world_object_id);
    snapshot_.last_world_status = world_status;
    if (world_status != WorldStatus::Success) {
        return RecordFailure(WorldIdentityBaselineStatus::WorldRemoveFailed);
    }

    const ObjectStatus object_status = object_registry_.Destroy(record->object_handle);
    snapshot_.last_object_status = object_status;
    if (object_status != ObjectStatus::Success) {
        return RecordFailure(WorldIdentityBaselineStatus::ObjectDestroyFailed);
    }

    ClearRecord(*record);
    RecountRecords();
    ++snapshot_.destroyed_record_count;
    RecordSuccess();
    return WorldIdentityBaselineStatus::Success;
}

std::uint32_t WorldIdentityBaseline::ExportRecords(
    WorldIdentityBaselineRecord *output_records,
    std::uint32_t output_capacity) const {
    std::uint32_t exported_record_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.record_capacity; ++index) {
        const WorldIdentityBaselineRecord &record = records_[index];
        if (!record.is_active) {
            continue;
        }

        if ((output_records != nullptr) && (exported_record_count < output_capacity)) {
            output_records[exported_record_count] = record;
        }

        ++exported_record_count;
    }

    return exported_record_count;
}

WorldIdentityBaselineSnapshot WorldIdentityBaseline::Snapshot() const {
    return snapshot_;
}

WorldIdentityBaselineStatus WorldIdentityBaseline::RecordFailure(WorldIdentityBaselineStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return status;
}

WorldIdentityBaselineResult WorldIdentityBaseline::RecordFailureResult(WorldIdentityBaselineStatus status) {
    RecordFailure(status);
    WorldIdentityBaselineResult result = WorldIdentityBaselineResult::Failure(status);
    result.object_status = snapshot_.last_object_status;
    result.world_status = snapshot_.last_world_status;
    result.identity_status = snapshot_.last_identity_status;
    result.transform_status = snapshot_.last_transform_status;
    result.component_status = snapshot_.last_component_status;
    return result;
}

void WorldIdentityBaseline::RecordSuccess() {
    snapshot_.last_status = WorldIdentityBaselineStatus::Success;
    snapshot_.last_object_status = ObjectStatus::Success;
    snapshot_.last_world_status = WorldStatus::Success;
    snapshot_.last_identity_status = WorldObjectIdentityStatus::Success;
    snapshot_.last_transform_status = WorldTransformStatus::Success;
    snapshot_.last_component_status = WorldComponentAttachmentStatus::Success;
}

WorldIdentityBaselineStatus WorldIdentityBaseline::ValidateBaselineCapacity() const {
    if (snapshot_.record_capacity == 0U) {
        return WorldIdentityBaselineStatus::InvalidBaselineCapacity;
    }

    return WorldIdentityBaselineStatus::Success;
}

WorldIdentityBaselineStatus WorldIdentityBaseline::ValidateObjectDesc(
    const WorldIdentityBaselineObjectDesc &desc) const {
    const WorldIdentityBaselineStatus capacity_status = ValidateBaselineCapacity();
    if (capacity_status != WorldIdentityBaselineStatus::Success) {
        return capacity_status;
    }

    if (!desc.world_object_id.IsValid()) {
        return WorldIdentityBaselineStatus::InvalidWorldObjectId;
    }

    if (!desc.object_descriptor.type.IsValid()) {
        return WorldIdentityBaselineStatus::InvalidObjectType;
    }

    if (desc.object_descriptor.initial_reference_count != 0U) {
        return WorldIdentityBaselineStatus::InvalidObjectReferenceCount;
    }

    if (!desc.component_type_id.IsValid()) {
        return WorldIdentityBaselineStatus::InvalidComponentTypeId;
    }

    if (!desc.component_slot_id.IsValid()) {
        return WorldIdentityBaselineStatus::InvalidComponentSlotId;
    }

    if (FindRecordByWorldObjectId(desc.world_object_id) != nullptr) {
        return WorldIdentityBaselineStatus::DuplicateWorldObjectId;
    }

    if (snapshot_.active_record_count >= snapshot_.record_capacity) {
        return WorldIdentityBaselineStatus::CapacityExceeded;
    }

    if (world_.ContainsObject(desc.world_object_id)) {
        return WorldIdentityBaselineStatus::DuplicateWorldObjectId;
    }

    return WorldIdentityBaselineStatus::Success;
}

WorldIdentityBaselineStatus WorldIdentityBaseline::ValidateRecordForMutation(
    WorldObjectId world_object_id,
    WorldIdentityBaselineRecord *&out_record) {
    const WorldIdentityBaselineStatus capacity_status = ValidateBaselineCapacity();
    if (capacity_status != WorldIdentityBaselineStatus::Success) {
        return capacity_status;
    }

    if (!world_object_id.IsValid()) {
        return WorldIdentityBaselineStatus::InvalidWorldObjectId;
    }

    WorldIdentityBaselineRecord *record = FindRecordByWorldObjectId(world_object_id);
    if (record == nullptr) {
        return WorldIdentityBaselineStatus::RecordNotFound;
    }

    if (!world_.ContainsObject(world_object_id)) {
        return WorldIdentityBaselineStatus::WorldObjectMissing;
    }

    const ObjectStatus object_status = object_registry_.Validate(record->object_handle);
    snapshot_.last_object_status = object_status;
    if (object_status != ObjectStatus::Success) {
        return WorldIdentityBaselineStatus::ObjectValidateFailed;
    }

    const WorldObjectIdentityStatus identity_status = identity_bridge_.Validate(world_object_id);
    snapshot_.last_identity_status = identity_status;
    if (identity_status != WorldObjectIdentityStatus::Success) {
        return WorldIdentityBaselineStatus::IdentityValidateFailed;
    }

    const WorldTransformResult transform_result = transform_bridge_.Query(world_object_id);
    snapshot_.last_transform_status = transform_result.status;
    if (!transform_result.Succeeded()) {
        return WorldIdentityBaselineStatus::TransformValidateFailed;
    }

    const WorldComponentAttachmentResult component_result = component_bridge_.Query(
        world_object_id,
        record->component_type_id);
    snapshot_.last_component_status = component_result.status;
    if (!component_result.Succeeded()) {
        return WorldIdentityBaselineStatus::ComponentValidateFailed;
    }

    out_record = record;
    return WorldIdentityBaselineStatus::Success;
}

WorldIdentityBaselineRecord *WorldIdentityBaseline::FindRecordByWorldObjectId(WorldObjectId world_object_id) {
    for (std::uint32_t index = 0U; index < snapshot_.record_capacity; ++index) {
        WorldIdentityBaselineRecord &record = records_[index];
        if (!record.is_active) {
            continue;
        }

        if (record.world_object_id.value == world_object_id.value) {
            return &record;
        }
    }

    return nullptr;
}

const WorldIdentityBaselineRecord *WorldIdentityBaseline::FindRecordByWorldObjectId(
    WorldObjectId world_object_id) const {
    for (std::uint32_t index = 0U; index < snapshot_.record_capacity; ++index) {
        const WorldIdentityBaselineRecord &record = records_[index];
        if (!record.is_active) {
            continue;
        }

        if (record.world_object_id.value == world_object_id.value) {
            return &record;
        }
    }

    return nullptr;
}

WorldIdentityBaselineRecord *WorldIdentityBaseline::FindFreeRecord() {
    for (std::uint32_t index = 0U; index < snapshot_.record_capacity; ++index) {
        WorldIdentityBaselineRecord &record = records_[index];
        if (record.is_active) {
            continue;
        }

        return &record;
    }

    return nullptr;
}

void WorldIdentityBaseline::ClearRecord(WorldIdentityBaselineRecord &record) {
    record = WorldIdentityBaselineRecord{};
}

void WorldIdentityBaseline::RecountRecords() {
    std::uint32_t active_record_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.record_capacity; ++index) {
        const WorldIdentityBaselineRecord &record = records_[index];
        if (!record.is_active) {
            continue;
        }

        ++active_record_count;
    }

    snapshot_.active_record_count = active_record_count;
}

void WorldIdentityBaseline::RollbackCreatedObject(
    WorldObjectId world_object_id,
    ObjectHandle object_handle) {
    RollbackIdentity(world_object_id);
    RollbackTransform(world_object_id);
    RollbackWorldObject(world_object_id);
    RollbackObjectHandle(object_handle);
}

void WorldIdentityBaseline::RollbackComponent(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id) {
    const WorldComponentAttachmentStatus component_status = component_bridge_.Remove(
        world_object_id,
        component_type_id);
    snapshot_.last_component_status = component_status;
}

void WorldIdentityBaseline::RollbackIdentity(WorldObjectId world_object_id) {
    const WorldObjectIdentityStatus identity_status = identity_bridge_.Remove(world_object_id);
    snapshot_.last_identity_status = identity_status;
}

void WorldIdentityBaseline::RollbackTransform(WorldObjectId world_object_id) {
    const WorldTransformStatus transform_status = transform_bridge_.Remove(world_object_id);
    snapshot_.last_transform_status = transform_status;
}

void WorldIdentityBaseline::RollbackWorldObject(WorldObjectId world_object_id) {
    const WorldStatus world_status = world_.RemoveObject(world_object_id);
    snapshot_.last_world_status = world_status;
}

void WorldIdentityBaseline::RollbackObjectHandle(ObjectHandle object_handle) {
    const ObjectStatus object_status = object_registry_.Destroy(object_handle);
    snapshot_.last_object_status = object_status;
}
}
