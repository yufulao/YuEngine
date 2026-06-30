// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Src/RuntimeAssetWorldObjectAdapterBridge.cpp

#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterBridge.h"

#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"

using yuengine::object::ObjectHandle;
using yuengine::runtimeasset::RuntimeAssetRuntimeInstanceMappingRecord;
using yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using yuengine::runtimeasset::RuntimeAssetSceneTransformOutputRecord;
using yuengine::runtimeasset::RuntimeAssetTargetIdentityKind;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;

namespace yuengine::runtimeassetworldadapter {
namespace {
bool WorldObjectIdsMatch(WorldObjectId left, WorldObjectId right) {
    return left.value == right.value;
}

bool RuntimeAssetTargetKindIsSupported(RuntimeAssetTargetIdentityKind target_kind) {
    if (target_kind == RuntimeAssetTargetIdentityKind::SceneNode) {
        return true;
    }

    if (target_kind == RuntimeAssetTargetIdentityKind::ModelNode) {
        return true;
    }

    return target_kind == RuntimeAssetTargetIdentityKind::SkeletonJoint;
}
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterResult::Success(
    RuntimeAssetWorldObjectAdapterState state) {
    return RuntimeAssetWorldObjectAdapterResult{
        RuntimeAssetWorldObjectAdapterStatus::Success,
        state,
        0U,
        0U};
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterResult::Failure(
    RuntimeAssetWorldObjectAdapterStatus status) {
    return RuntimeAssetWorldObjectAdapterResult{status, RuntimeAssetWorldObjectAdapterState{}, 0U, 0U};
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterResult::Failure(
    RuntimeAssetWorldObjectAdapterStatus status,
    std::uint32_t failed_mapping_index,
    std::uint64_t failed_target_id) {
    return RuntimeAssetWorldObjectAdapterResult{
        status,
        RuntimeAssetWorldObjectAdapterState{},
        failed_mapping_index,
        failed_target_id};
}

bool RuntimeAssetWorldObjectAdapterResult::Succeeded() const {
    return status == RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::BuildRestoreRecords(
    const RuntimeAssetWorldObjectAdapterRequest &request) {
    ++snapshot_.build_attempt_count;

    std::uint32_t failed_mapping_index = 0U;
    std::uint64_t failed_target_id = 0U;
    const RuntimeAssetWorldObjectAdapterStatus request_status = ValidateRequest(
        request,
        &failed_mapping_index,
        &failed_target_id);
    if (request_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        return RecordFailure(request_status, failed_mapping_index, failed_target_id);
    }

    std::uint32_t mapping_index = 0U;
    while (mapping_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &mapping = request.runtime_instance_mappings[mapping_index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_record =
            FindIdentityRecord(request, mapping.target_id);
        const RuntimeAssetSceneTransformOutputRecord &scene_transform =
            request.scene_transforms[mapping.scene_transform_index];
        WorldSceneObjectTransformRestoreIdentityRecord identity_output{};
        identity_output.world_object_id = identity_record->world_object_id;
        identity_output.object_handle = identity_record->object_handle;
        request.output_identities[mapping_index] = identity_output;

        WorldSceneObjectTransformRestoreTransformRecord transform_output{};
        transform_output.world_object_id = scene_transform.world_object_id;
        transform_output.transform_state = scene_transform.transform;
        request.output_transforms[mapping_index] = transform_output;

        ++mapping_index;
    }

    RuntimeAssetWorldObjectAdapterState state{};
    state.input_mapping_count = request.runtime_instance_mapping_count;
    state.output_identity_count = request.runtime_instance_mapping_count;
    state.output_transform_count = request.runtime_instance_mapping_count;
    return RecordSuccess(state);
}

RuntimeAssetWorldObjectAdapterSnapshot RuntimeAssetWorldObjectAdapterBridge::Snapshot() const {
    return snapshot_;
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::RecordFailure(
    RuntimeAssetWorldObjectAdapterStatus status) {
    return RecordFailure(status, 0U, 0U);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::RecordFailure(
    RuntimeAssetWorldObjectAdapterStatus status,
    std::uint32_t failed_mapping_index,
    std::uint64_t failed_target_id) {
    ++snapshot_.failed_operation_count;
    ++snapshot_.rejected_record_count;
    snapshot_.last_status = status;
    return RuntimeAssetWorldObjectAdapterResult::Failure(status, failed_mapping_index, failed_target_id);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::RecordSuccess(
    const RuntimeAssetWorldObjectAdapterState &state) {
    snapshot_.built_identity_count += state.output_identity_count;
    snapshot_.built_transform_count += state.output_transform_count;
    snapshot_.last_status = RuntimeAssetWorldObjectAdapterStatus::Success;
    return RuntimeAssetWorldObjectAdapterResult::Success(state);
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateRequest(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint32_t *out_failed_mapping_index,
    std::uint64_t *out_failed_target_id) const {
    if (request.runtime_instance_mapping_count > 0U && request.runtime_instance_mappings == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.scene_entities == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSceneEntityInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.scene_transforms == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSceneTransformInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.identity_records == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.output_identities == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityOutput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.output_transforms == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformOutput;
    }

    if (request.runtime_instance_mapping_count > request.output_identity_capacity) {
        return RuntimeAssetWorldObjectAdapterStatus::IdentityOutputCapacityExceeded;
    }

    if (request.runtime_instance_mapping_count > request.output_transform_capacity) {
        return RuntimeAssetWorldObjectAdapterStatus::TransformOutputCapacityExceeded;
    }

    std::uint32_t mapping_index = 0U;
    while (mapping_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetWorldObjectAdapterStatus mapping_status = ValidateRuntimeInstanceMapping(
            request,
            mapping_index);
        if (mapping_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            if (out_failed_mapping_index != nullptr) {
                *out_failed_mapping_index = mapping_index;
            }

            if (out_failed_target_id != nullptr) {
                *out_failed_target_id = request.runtime_instance_mappings[mapping_index].target_id;
            }

            return mapping_status;
        }

        ++mapping_index;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateRuntimeInstanceMapping(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint32_t mapping_index) const {
    const RuntimeAssetRuntimeInstanceMappingRecord &mapping = request.runtime_instance_mappings[mapping_index];
    if (!mapping.is_valid) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceMapping;
    }

    if (!RuntimeAssetTargetKindIsSupported(mapping.target_kind)) {
        return RuntimeAssetWorldObjectAdapterStatus::UnsupportedTargetKind;
    }

    if (mapping.scene_entity_index >= request.scene_entity_count) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSceneEntityIndex;
    }

    if (mapping.scene_transform_index >= request.scene_transform_count) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSceneTransformIndex;
    }

    const RuntimeAssetSceneEntityRecord &scene_entity = request.scene_entities[mapping.scene_entity_index];
    if (scene_entity.entity_id != mapping.scene_entity_id) {
        return RuntimeAssetWorldObjectAdapterStatus::MissingSceneEntity;
    }

    if (!scene_entity.world_object_id.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId;
    }

    const RuntimeAssetSceneTransformOutputRecord &scene_transform =
        request.scene_transforms[mapping.scene_transform_index];
    if (!scene_transform.world_object_id.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId;
    }

    if (!WorldObjectIdsMatch(scene_entity.world_object_id, scene_transform.world_object_id)) {
        return RuntimeAssetWorldObjectAdapterStatus::MissingSceneTransform;
    }

    const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_record =
        FindIdentityRecord(request, mapping.target_id);
    if (identity_record == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::MissingIdentityRecord;
    }

    if (!identity_record->world_object_id.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId;
    }

    if (!WorldObjectIdsMatch(scene_entity.world_object_id, identity_record->world_object_id)) {
        return RuntimeAssetWorldObjectAdapterStatus::WorldObjectMismatch;
    }

    if (!identity_record->object_handle.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidObjectHandle;
    }

    if (HasDuplicateMappingTargetId(request, mapping_index)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateTargetId;
    }

    if (HasDuplicateWorldObjectId(request, mapping_index, identity_record->world_object_id)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateWorldObjectId;
    }

    if (HasDuplicateObjectHandle(request, mapping_index, identity_record->object_handle)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateObjectHandle;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

const RuntimeAssetWorldObjectAdapterIdentityRecord *RuntimeAssetWorldObjectAdapterBridge::FindIdentityRecord(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint64_t target_id) const {
    std::uint32_t identity_index = 0U;
    while (identity_index < request.identity_record_count) {
        const RuntimeAssetWorldObjectAdapterIdentityRecord &identity_record =
            request.identity_records[identity_index];
        if (identity_record.target_id == target_id) {
            return &identity_record;
        }

        ++identity_index;
    }

    return nullptr;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateMappingTargetId(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint32_t mapping_index) const {
    const RuntimeAssetRuntimeInstanceMappingRecord &mapping = request.runtime_instance_mappings[mapping_index];
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        if (other_mapping.target_id == mapping.target_id) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateWorldObjectId(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint32_t mapping_index,
    WorldObjectId world_object_id) const {
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord *other_identity =
            FindIdentityRecord(request, other_mapping.target_id);
        if (other_identity != nullptr && WorldObjectIdsMatch(other_identity->world_object_id, world_object_id)) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateObjectHandle(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint32_t mapping_index,
    ObjectHandle object_handle) const {
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord *other_identity =
            FindIdentityRecord(request, other_mapping.target_id);
        if (other_identity != nullptr && ObjectHandlesMatch(other_identity->object_handle, object_handle)) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::ObjectHandlesMatch(
    ObjectHandle left,
    ObjectHandle right) const {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}
}
