// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Src/AssetManager.cpp

#include "YuEngine/Asset/AssetManager.h"

#include <limits>

#include "YuEngine/Resource/ResourceRegistry.h"

namespace yuengine::asset {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}

void ClearRequiredCounts(AssetSnapshot &snapshot) {
    snapshot.last_required_asset_count = 0U;
    snapshot.last_required_type_count = 0U;
    snapshot.last_required_dependency_edge_count = 0U;
    snapshot.last_failed_dependency_output_dependent = AssetHandle{};
    snapshot.last_dependency_output_capacity = 0U;
    snapshot.last_required_dependency_output_count = 0U;
    snapshot.last_failed_dependency_output_dependency = AssetHandle{};
}

void ClearCapacityEntry(AssetSnapshot &snapshot) {
    snapshot.last_capacity_entry_asset_id = 0U;
    snapshot.last_capacity_entry_resource_handle = yuengine::resource::ResourceHandle{};
    snapshot.last_capacity_entry_resource_type = yuengine::resource::ResourceTypeId{};
    snapshot.last_capacity_entry_asset_type = AssetTypeId{};
    snapshot.last_capacity_entry_dependent_asset = AssetHandle{};
    snapshot.last_capacity_entry_dependency_asset = AssetHandle{};
    snapshot.last_capacity_entry_asset_capacity = 0U;
    snapshot.last_capacity_entry_type_capacity = 0U;
    snapshot.last_capacity_entry_dependency_edge_capacity = 0U;
    snapshot.last_capacity_entry_asset_count = 0U;
    snapshot.last_capacity_entry_type_count = 0U;
    snapshot.last_capacity_entry_dependency_edge_count = 0U;
}

void RecordRequiredCounts(
    AssetSnapshot &snapshot,
    std::uint32_t required_asset_count,
    std::uint32_t required_type_count,
    std::uint32_t required_dependency_edge_count) {
    snapshot.last_required_asset_count = required_asset_count;
    snapshot.last_required_type_count = required_type_count;
    snapshot.last_required_dependency_edge_count = required_dependency_edge_count;
}

void RecordRegistrationCapacityEntry(AssetSnapshot &snapshot, const AssetDescriptor &descriptor) {
    snapshot.last_capacity_entry_asset_id = descriptor.stable_id;
    snapshot.last_capacity_entry_resource_handle = descriptor.resource;
    snapshot.last_capacity_entry_resource_type = descriptor.resource_type;
    snapshot.last_capacity_entry_asset_type = descriptor.asset_type;
    snapshot.last_capacity_entry_dependent_asset = AssetHandle{};
    snapshot.last_capacity_entry_dependency_asset = AssetHandle{};
    snapshot.last_capacity_entry_asset_capacity = snapshot.asset_capacity;
    snapshot.last_capacity_entry_type_capacity = snapshot.type_capacity;
    snapshot.last_capacity_entry_dependency_edge_capacity = snapshot.dependency_edge_capacity;
    snapshot.last_capacity_entry_asset_count = snapshot.active_asset_count;
    snapshot.last_capacity_entry_type_count = snapshot.type_count;
    snapshot.last_capacity_entry_dependency_edge_count = snapshot.active_dependency_edge_count;
}

void RecordDependencyCapacityEntry(
    AssetSnapshot &snapshot,
    AssetHandle dependent,
    AssetHandle dependency) {
    snapshot.last_capacity_entry_asset_id = 0U;
    snapshot.last_capacity_entry_resource_handle = yuengine::resource::ResourceHandle{};
    snapshot.last_capacity_entry_resource_type = yuengine::resource::ResourceTypeId{};
    snapshot.last_capacity_entry_asset_type = AssetTypeId{};
    snapshot.last_capacity_entry_dependent_asset = dependent;
    snapshot.last_capacity_entry_dependency_asset = dependency;
    snapshot.last_capacity_entry_asset_capacity = snapshot.asset_capacity;
    snapshot.last_capacity_entry_type_capacity = snapshot.type_capacity;
    snapshot.last_capacity_entry_dependency_edge_capacity = snapshot.dependency_edge_capacity;
    snapshot.last_capacity_entry_asset_count = snapshot.active_asset_count;
    snapshot.last_capacity_entry_type_count = snapshot.type_count;
    snapshot.last_capacity_entry_dependency_edge_count = snapshot.active_dependency_edge_count;
}

AssetRegistrationResult MakeRegistrationCapacityResult(
    AssetStatus status,
    const AssetSnapshot &snapshot,
    const AssetDescriptor &descriptor,
    std::uint32_t required_asset_count,
    std::uint32_t required_type_count,
    std::uint32_t required_dependency_edge_count) {
    AssetRegistrationResult result = AssetRegistrationResult::Failure(
        status,
        required_asset_count,
        required_type_count,
        required_dependency_edge_count);
    result.capacity_entry_asset_id = descriptor.stable_id;
    result.capacity_entry_resource_handle = descriptor.resource;
    result.capacity_entry_resource_type = descriptor.resource_type;
    result.capacity_entry_asset_type = descriptor.asset_type;
    result.capacity_entry_asset_capacity = snapshot.asset_capacity;
    result.capacity_entry_type_capacity = snapshot.type_capacity;
    result.capacity_entry_dependency_edge_capacity = snapshot.dependency_edge_capacity;
    result.capacity_entry_asset_count = snapshot.active_asset_count;
    result.capacity_entry_type_count = snapshot.type_count;
    result.capacity_entry_dependency_edge_count = snapshot.active_dependency_edge_count;
    return result;
}

void RecordDependencyOutputCapacityEntry(
    AssetSnapshot &snapshot,
    AssetHandle dependent,
    std::uint32_t output_capacity,
    std::uint32_t required_dependency_output_count,
    AssetHandle dependency) {
    ClearRequiredCounts(snapshot);
    snapshot.last_failed_dependency_output_dependent = dependent;
    snapshot.last_dependency_output_capacity = output_capacity;
    snapshot.last_required_dependency_output_count = required_dependency_output_count;
    snapshot.last_failed_dependency_output_dependency = dependency;
}
}

AssetManager::AssetManager()
    : AssetManager(AssetManagerDesc{}) {
}

AssetManager::AssetManager(AssetManagerDesc desc)
    : slots_{},
      types_{},
      dependency_edges_{},
      snapshot_{
          ClampCapacity(desc.asset_capacity, MAX_ASSET_COUNT),
          ClampCapacity(desc.type_capacity, MAX_ASSET_TYPE_COUNT),
          ClampCapacity(desc.dependency_edge_capacity, MAX_ASSET_DEPENDENCY_EDGE_COUNT)} {
}

AssetRegistrationResult AssetManager::RegisterRuntimeAsset(
    yuengine::resource::ResourceRegistry *resource_registry,
    const AssetDescriptor &descriptor) {
    if (resource_registry == nullptr) {
        return AssetRegistrationResult::Failure(RecordFailure(AssetStatus::InvalidArgument));
    }

    AssetStatus descriptor_status = AssetStatus::Success;
    if (!ValidateDescriptor(descriptor, &descriptor_status)) {
        return AssetRegistrationResult::Failure(RecordFailure(descriptor_status));
    }

    if (HasDuplicateActiveAsset(descriptor)) {
        return AssetRegistrationResult::Failure(RecordFailure(AssetStatus::DuplicateAsset));
    }

    const bool needs_type_registration = !HasType(descriptor.asset_type);
    const std::uint32_t required_asset_count = snapshot_.active_asset_count + 1U;
    std::uint32_t required_type_count = snapshot_.type_count;
    if (needs_type_registration) {
        ++required_type_count;
    }

    if (snapshot_.active_asset_count >= snapshot_.asset_capacity) {
        const AssetStatus status = RecordFailure(AssetStatus::CapacityExceeded);
        RecordRequiredCounts(snapshot_, required_asset_count, required_type_count, 0U);
        RecordRegistrationCapacityEntry(snapshot_, descriptor);
        return MakeRegistrationCapacityResult(status, snapshot_, descriptor, required_asset_count, required_type_count, 0U);
    }

    if (needs_type_registration && snapshot_.type_count >= snapshot_.type_capacity) {
        const AssetStatus status = RecordFailure(AssetStatus::CapacityExceeded);
        RecordRequiredCounts(snapshot_, required_asset_count, required_type_count, 0U);
        RecordRegistrationCapacityEntry(snapshot_, descriptor);
        return MakeRegistrationCapacityResult(status, snapshot_, descriptor, required_asset_count, required_type_count, 0U);
    }

    std::uint32_t free_slot_index = 0U;
    AssetSlot *free_slot = FindFreeSlot(&free_slot_index);
    if (free_slot == nullptr) {
        const AssetStatus status = RecordFailure(AssetStatus::CapacityExceeded);
        RecordRequiredCounts(snapshot_, required_asset_count, required_type_count, 0U);
        RecordRegistrationCapacityEntry(snapshot_, descriptor);
        return MakeRegistrationCapacityResult(status, snapshot_, descriptor, required_asset_count, required_type_count, 0U);
    }

    const yuengine::resource::ResourceStatus resource_status =
        resource_registry->Acquire(descriptor.resource, descriptor.resource_type);
    if (resource_status != yuengine::resource::ResourceStatus::Success) {
        return AssetRegistrationResult::Failure(
            RecordResourceFailure(AssetStatus::ResourceAcquireFailed, resource_status));
    }

    RegisterTypeIfNeeded(descriptor.asset_type);
    if (free_slot->generation == INVALID_ASSET_GENERATION) {
        free_slot->generation = 1U;
    }

    free_slot->record = AssetRecord{};
    free_slot->record.handle = AssetHandle{free_slot_index, free_slot->generation};
    free_slot->record.stable_id = descriptor.stable_id;
    free_slot->record.asset_type = descriptor.asset_type;
    free_slot->record.resource = descriptor.resource;
    free_slot->record.resource_type = descriptor.resource_type;
    free_slot->record.state = AssetLoadState::Unloaded;
    free_slot->record.reference_count = descriptor.initial_reference_count;
    free_slot->record.is_active = true;
    free_slot->is_active = true;

    ++snapshot_.active_asset_count;
    ++snapshot_.registered_asset_count;
    snapshot_.referenced_asset_count += descriptor.initial_reference_count;
    snapshot_.last_resource_status = yuengine::resource::ResourceStatus::Success;
    RecordSuccess();
    return AssetRegistrationResult::Success(free_slot->record.handle);
}

AssetStatus AssetManager::AcquireAsset(AssetHandle handle) {
    std::size_t slot_index = 0U;
    const AssetStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != AssetStatus::Success) {
        return RecordFailure(handle_status);
    }

    AssetRecord &record = slots_[slot_index].record;
    if (record.reference_count == std::numeric_limits<std::uint32_t>::max()) {
        return RecordFailure(AssetStatus::ReferenceCountOverflow);
    }

    ++record.reference_count;
    ++snapshot_.referenced_asset_count;
    RecordSuccess();
    return AssetStatus::Success;
}

AssetStatus AssetManager::ReleaseAssetReference(AssetHandle handle) {
    std::size_t slot_index = 0U;
    const AssetStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != AssetStatus::Success) {
        return RecordFailure(handle_status);
    }

    AssetRecord &record = slots_[slot_index].record;
    if (record.reference_count == 0U) {
        return RecordFailure(AssetStatus::NotAcquired);
    }

    --record.reference_count;
    ++snapshot_.released_reference_count;
    RecordSuccess();
    return AssetStatus::Success;
}

AssetStatus AssetManager::ReleaseRuntimeAsset(
    yuengine::resource::ResourceRegistry *resource_registry,
    AssetHandle handle) {
    if (resource_registry == nullptr) {
        return RecordFailure(AssetStatus::InvalidArgument);
    }

    std::size_t slot_index = 0U;
    const AssetStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != AssetStatus::Success) {
        return RecordFailure(handle_status);
    }

    AssetSlot &slot = slots_[slot_index];
    if (slot.record.reference_count != 0U) {
        return RecordFailure(AssetStatus::StillReferenced);
    }

    const yuengine::resource::ResourceStatus resource_status = resource_registry->Release(slot.record.resource);
    if (resource_status != yuengine::resource::ResourceStatus::Success) {
        return RecordResourceFailure(AssetStatus::ResourceReleaseFailed, resource_status);
    }

    if (slot.record.texture_ready.is_ready) {
        --snapshot_.texture_ready_count;
    }

    if (slot.record.audio_ready.is_ready) {
        --snapshot_.audio_ready_count;
    }

    ClearDependencyEdgesForSlot(static_cast<std::uint32_t>(slot_index));
    slot.is_active = false;
    slot.record = AssetRecord{};
    AdvanceGeneration(slot);
    --snapshot_.active_asset_count;
    ++snapshot_.released_asset_count;
    snapshot_.last_resource_status = yuengine::resource::ResourceStatus::Success;
    RecordSuccess();
    return AssetStatus::Success;
}

AssetStatus AssetManager::AddDependency(AssetHandle dependent, AssetHandle dependency) {
    std::size_t dependent_index = 0U;
    const AssetStatus dependent_status = ResolveHandle(dependent, dependent_index);
    if (dependent_status != AssetStatus::Success) {
        return RecordFailure(dependent_status);
    }

    std::size_t dependency_index = 0U;
    const AssetStatus dependency_status = ResolveHandle(dependency, dependency_index);
    if (dependency_status != AssetStatus::Success) {
        return RecordFailure(dependency_status);
    }

    if (dependent_index == dependency_index) {
        return RecordFailure(AssetStatus::DependencyCycle);
    }

    if (HasDependencyEdge(dependent, dependency)) {
        return RecordFailure(AssetStatus::DuplicateDependency);
    }

    if (HasDependencyPath(static_cast<std::uint32_t>(dependency_index), static_cast<std::uint32_t>(dependent_index))) {
        return RecordFailure(AssetStatus::DependencyCycle);
    }

    if (snapshot_.active_dependency_edge_count >= snapshot_.dependency_edge_capacity) {
        const std::uint32_t required_dependency_edge_count =
            snapshot_.active_dependency_edge_count + 1U;
        const AssetStatus status = RecordFailure(AssetStatus::CapacityExceeded);
        RecordRequiredCounts(snapshot_, 0U, 0U, required_dependency_edge_count);
        RecordDependencyCapacityEntry(snapshot_, dependent, dependency);
        return status;
    }

    AssetDependencyEdge *edge = FindFreeDependencyEdge();
    if (edge == nullptr) {
        const std::uint32_t required_dependency_edge_count =
            snapshot_.active_dependency_edge_count + 1U;
        const AssetStatus status = RecordFailure(AssetStatus::CapacityExceeded);
        RecordRequiredCounts(snapshot_, 0U, 0U, required_dependency_edge_count);
        RecordDependencyCapacityEntry(snapshot_, dependent, dependency);
        return status;
    }

    edge->dependent = dependent;
    edge->dependency = dependency;
    edge->is_active = true;
    ++snapshot_.active_dependency_edge_count;
    RecordSuccess();
    return AssetStatus::Success;
}

AssetStatus AssetManager::AddAuthoringDependency(const AssetAuthoringDependencyEdge &edge) {
    if (edge.stable_resource_id == 0U) {
        return RecordFailure(AssetStatus::InvalidAssetId);
    }

    if (!edge.expected_resource.IsValid()) {
        return RecordFailure(AssetStatus::InvalidArgument);
    }

    if (!edge.expected_resource_type.IsValid()) {
        return RecordFailure(AssetStatus::InvalidArgument);
    }

    std::size_t dependency_index = 0U;
    const AssetStatus dependency_status = ResolveHandle(edge.dependency, dependency_index);
    if (dependency_status != AssetStatus::Success) {
        return RecordFailure(dependency_status);
    }

    const AssetRecord &dependency_record = slots_[dependency_index].record;
    if (!DoResourceHandlesMatch(dependency_record.resource, edge.expected_resource)) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    if (!DoResourceTypesMatch(dependency_record.resource_type, edge.expected_resource_type)) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    return AddDependency(edge.dependent, edge.dependency);
}

AssetAuthoringDependencyBatchResult AssetManager::AddAuthoringDependencies(
    const AssetAuthoringDependencyEdge *edges,
    std::uint32_t edge_count) {
    AssetAuthoringDependencyBatchResult result{};
    if (edge_count == 0U) {
        RecordSuccess();
        return result;
    }

    if (edges == nullptr) {
        result.status = RecordFailure(AssetStatus::InvalidArgument);
        return result;
    }

    for (std::uint32_t edge_index = 0U; edge_index < edge_count; ++edge_index) {
        const AssetStatus edge_status = AddAuthoringDependency(edges[edge_index]);
        if (edge_status != AssetStatus::Success) {
            result.status = edge_status;
            result.failed_edge_index = edge_index;
            return result;
        }

        ++result.committed_edge_count;
    }

    result.status = AssetStatus::Success;
    result.failed_edge_index = 0U;
    return result;
}

AssetStatus AssetManager::TraverseDependencies(
    AssetHandle root,
    AssetHandle *output_assets,
    std::uint32_t output_asset_capacity,
    std::uint32_t *output_asset_count) {
    if (output_asset_count == nullptr) {
        return RecordFailure(AssetStatus::InvalidArgument);
    }

    *output_asset_count = 0U;
    if (output_asset_capacity > 0U && output_assets == nullptr) {
        return RecordFailure(AssetStatus::InvalidArgument);
    }

    std::size_t root_index = 0U;
    const AssetStatus root_status = ResolveHandle(root, root_index);
    if (root_status != AssetStatus::Success) {
        return RecordFailure(root_status);
    }

    std::array<std::uint32_t, MAX_ASSET_COUNT> pending_slots{};
    std::array<bool, MAX_ASSET_COUNT> visited_slots{};
    std::array<AssetHandle, MAX_ASSET_COUNT> staged_dependencies{};
    std::uint32_t read_index = 0U;
    std::uint32_t pending_count = 1U;
    std::uint32_t staged_dependency_count = 0U;
    pending_slots[0U] = static_cast<std::uint32_t>(root_index);
    visited_slots[root_index] = true;

    while (read_index < pending_count) {
        const std::uint32_t current_slot = pending_slots[read_index];
        ++read_index;

        for (const AssetDependencyEdge &edge : dependency_edges_) {
            if (!edge.is_active) {
                continue;
            }

            if (edge.dependent.slot != current_slot) {
                continue;
            }

            const std::uint32_t dependency_slot = edge.dependency.slot;
            if (dependency_slot >= snapshot_.asset_capacity) {
                continue;
            }

            if (visited_slots[dependency_slot]) {
                continue;
            }

            if (staged_dependency_count >= output_asset_capacity) {
                const AssetHandle dependent = slots_[current_slot].record.handle;
                const std::uint32_t required_dependency_output_count = staged_dependency_count + 1U;
                const AssetHandle dependency = edge.dependency;
                const AssetStatus status = RecordFailure(AssetStatus::OutputBufferTooSmall);
                RecordDependencyOutputCapacityEntry(
                    snapshot_,
                    dependent,
                    output_asset_capacity,
                    required_dependency_output_count,
                    dependency);
                return status;
            }

            if (pending_count >= MAX_ASSET_COUNT) {
                return RecordFailure(AssetStatus::CapacityExceeded);
            }

            staged_dependencies[staged_dependency_count] = edge.dependency;
            ++staged_dependency_count;
            visited_slots[dependency_slot] = true;
            pending_slots[pending_count] = dependency_slot;
            ++pending_count;
        }
    }

    for (std::uint32_t dependency_index = 0U; dependency_index < staged_dependency_count; ++dependency_index) {
        output_assets[dependency_index] = staged_dependencies[dependency_index];
    }

    *output_asset_count = staged_dependency_count;
    RecordSuccess();
    return AssetStatus::Success;
}

AssetStatus AssetManager::MarkAssetLoading(AssetHandle handle) {
    std::size_t slot_index = 0U;
    const AssetStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != AssetStatus::Success) {
        return RecordFailure(handle_status);
    }

    AssetRecord &record = slots_[slot_index].record;
    if (record.state != AssetLoadState::Unloaded && record.state != AssetLoadState::Failed) {
        return RecordFailure(AssetStatus::InvalidState);
    }

    record.state = AssetLoadState::Loading;
    RecordSuccess();
    return AssetStatus::Success;
}

AssetStatus AssetManager::MarkAssetDecoded(
    AssetHandle handle,
    const yuengine::resource::ResourceDecodedPayloadRecord &decoded_payload) {
    std::size_t slot_index = 0U;
    const AssetStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != AssetStatus::Success) {
        return RecordFailure(handle_status);
    }

    AssetRecord &record = slots_[slot_index].record;
    if (!decoded_payload.is_active) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    if (!DoResourceHandlesMatch(decoded_payload.resource, record.resource)) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    if (!DoResourceTypesMatch(decoded_payload.expected_type, record.resource_type)) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    if (decoded_payload.status != yuengine::resource::ResourceDecodedPayloadStatus::Success) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    record.decoded_payload = decoded_payload;
    record.state = AssetLoadState::Decoded;
    RecordSuccess();
    return AssetStatus::Success;
}

AssetStatus AssetManager::MarkTextureReady(
    AssetHandle handle,
    const yuengine::streaming::ResourceDecodedTextureBridgeResult &texture_result) {
    std::size_t slot_index = 0U;
    const AssetStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != AssetStatus::Success) {
        return RecordFailure(handle_status);
    }

    if (texture_result.status != yuengine::streaming::ResourceDecodedTextureBridgeStatus::Success) {
        return RecordTextureFailure(AssetStatus::TextureReadyFailed, texture_result.status);
    }

    const yuengine::resource::ResourceDecodedPayloadRecord &decoded_payload = texture_result.decoded_payload_record;
    AssetRecord &record = slots_[slot_index].record;
    if (!decoded_payload.is_active) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    if (!DoResourceHandlesMatch(decoded_payload.resource, record.resource)) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    if (!DoResourceTypesMatch(decoded_payload.expected_type, record.resource_type)) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    if (texture_result.texture_handle.generation == 0U) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    record.decoded_payload = decoded_payload;
    record.texture_ready.sampled_texture = texture_result.sampled_texture;
    record.texture_ready.decoded_byte_count = texture_result.decoded_byte_count;
    record.texture_ready.uploaded_byte_count = texture_result.uploaded_byte_count;
    if (!record.texture_ready.is_ready) {
        ++snapshot_.texture_ready_count;
    }

    record.texture_ready.is_ready = true;
    record.state = AssetLoadState::Uploaded;
    snapshot_.last_texture_status = yuengine::streaming::ResourceDecodedTextureBridgeStatus::Success;
    RecordSuccess();
    return AssetStatus::Success;
}

AssetStatus AssetManager::MarkAudioReady(
    AssetHandle handle,
    const yuengine::audioresource::AudioResourcePcmPacketImportRecord &audio_record) {
    std::size_t slot_index = 0U;
    const AssetStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != AssetStatus::Success) {
        return RecordFailure(handle_status);
    }

    if (audio_record.status != yuengine::audioresource::AudioResourcePcmPacketImportStatus::Success) {
        return RecordAudioFailure(AssetStatus::AudioReadyFailed, audio_record.status);
    }

    AssetRecord &record = slots_[slot_index].record;
    if (!audio_record.is_active) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    if (!DoResourceHandlesMatch(audio_record.resource, record.resource)) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    if (!DoResourceTypesMatch(audio_record.expected_type, record.resource_type)) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    if (!audio_record.handle.IsValid()) {
        return RecordFailure(AssetStatus::ReadyRecordMismatch);
    }

    record.audio_ready.import_handle = audio_record.handle;
    record.audio_ready.packet_request = audio_record.packet_request;
    if (!record.audio_ready.is_ready) {
        ++snapshot_.audio_ready_count;
    }

    record.audio_ready.is_ready = true;
    record.state = AssetLoadState::Uploaded;
    snapshot_.last_audio_status = yuengine::audioresource::AudioResourcePcmPacketImportStatus::Success;
    RecordSuccess();
    return AssetStatus::Success;
}

AssetStatus AssetManager::RefreshStateFromResource(
    yuengine::resource::ResourceRegistry *resource_registry,
    AssetHandle handle) {
    if (resource_registry == nullptr) {
        return RecordFailure(AssetStatus::InvalidArgument);
    }

    std::size_t slot_index = 0U;
    const AssetStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != AssetStatus::Success) {
        return RecordFailure(handle_status);
    }

    AssetRecord &record = slots_[slot_index].record;
    yuengine::resource::ResourceLoadState resource_load_state = yuengine::resource::ResourceLoadState::Unloaded;
    const yuengine::resource::ResourceLoadCommitStatus load_status =
        resource_registry->GetLoadState(record.resource, record.resource_type, &resource_load_state);
    if (load_status != yuengine::resource::ResourceLoadCommitStatus::Success) {
        return RecordResourceLoadFailure(AssetStatus::ResourceQueryFailed, load_status);
    }

    snapshot_.last_resource_load_status = yuengine::resource::ResourceLoadCommitStatus::Success;
    if (resource_load_state == yuengine::resource::ResourceLoadState::Failed) {
        record.state = AssetLoadState::Failed;
        RecordSuccess();
        return AssetStatus::Success;
    }

    if (resource_load_state == yuengine::resource::ResourceLoadState::Unloaded) {
        record.state = AssetLoadState::Unloaded;
        RecordSuccess();
        return AssetStatus::Success;
    }

    yuengine::resource::ResourceResidencyState resource_residency_state =
        yuengine::resource::ResourceResidencyState::Unloaded;
    const yuengine::resource::ResourceResidencyStatus residency_status =
        resource_registry->GetResidencyState(record.resource, record.resource_type, &resource_residency_state);
    if (residency_status != yuengine::resource::ResourceResidencyStatus::Success) {
        return RecordResourceResidencyFailure(AssetStatus::ResourceQueryFailed, residency_status);
    }

    snapshot_.last_resource_residency_status = yuengine::resource::ResourceResidencyStatus::Success;
    if (resource_residency_state == yuengine::resource::ResourceResidencyState::Failed) {
        record.state = AssetLoadState::Failed;
        RecordSuccess();
        return AssetStatus::Success;
    }

    if (IsResourceResidentState(resource_residency_state)) {
        record.state = AssetLoadState::Resident;
        RecordSuccess();
        return AssetStatus::Success;
    }

    record.state = AssetLoadState::Uploaded;
    RecordSuccess();
    return AssetStatus::Success;
}

AssetStatus AssetManager::QueryAsset(AssetHandle handle, AssetRecord *output_record) {
    if (output_record == nullptr) {
        return RecordFailure(AssetStatus::InvalidArgument);
    }

    std::size_t slot_index = 0U;
    const AssetStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != AssetStatus::Success) {
        return RecordFailure(handle_status);
    }

    *output_record = slots_[slot_index].record;
    RecordSuccess();
    return AssetStatus::Success;
}

AssetStatus AssetManager::GetAssetRecord(AssetHandle handle, AssetRecord *output_record) const {
    if (output_record == nullptr) {
        return AssetStatus::InvalidArgument;
    }

    std::size_t slot_index = 0U;
    const AssetStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != AssetStatus::Success) {
        return handle_status;
    }

    *output_record = slots_[slot_index].record;
    return AssetStatus::Success;
}

AssetStatus AssetManager::EnumerateDirectDependencies(
    AssetHandle dependent,
    AssetHandle *output_dependencies,
    std::uint32_t output_dependency_capacity,
    std::uint32_t *output_dependency_count) const {
    if (output_dependency_count == nullptr) {
        return AssetStatus::InvalidArgument;
    }

    *output_dependency_count = 0U;
    if (output_dependency_capacity > 0U && output_dependencies == nullptr) {
        return AssetStatus::InvalidArgument;
    }

    std::size_t dependent_index = 0U;
    const AssetStatus dependent_status = ResolveHandle(dependent, dependent_index);
    if (dependent_status != AssetStatus::Success) {
        return dependent_status;
    }

    std::array<AssetHandle, MAX_ASSET_DEPENDENCY_EDGE_COUNT> staged_dependencies{};
    std::uint32_t staged_dependency_count = 0U;
    for (const AssetDependencyEdge &edge : dependency_edges_) {
        if (!edge.is_active) {
            continue;
        }

        if (edge.dependent.slot != dependent.slot || edge.dependent.generation != dependent.generation) {
            continue;
        }

        if (staged_dependency_count >= output_dependency_capacity) {
            return AssetStatus::OutputBufferTooSmall;
        }

        staged_dependencies[staged_dependency_count] = edge.dependency;
        ++staged_dependency_count;
    }

    for (std::uint32_t index = 0U; index < staged_dependency_count; ++index) {
        output_dependencies[index] = staged_dependencies[index];
    }

    *output_dependency_count = staged_dependency_count;
    return AssetStatus::Success;
}

AssetSnapshot AssetManager::Snapshot() const {
    return snapshot_;
}

AssetStatus AssetManager::RecordFailure(AssetStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    ClearRequiredCounts(snapshot_);
    ClearCapacityEntry(snapshot_);
    return status;
}

AssetStatus AssetManager::RecordResourceFailure(
    AssetStatus status,
    yuengine::resource::ResourceStatus resource_status) {
    snapshot_.last_resource_status = resource_status;
    return RecordFailure(status);
}

AssetStatus AssetManager::RecordResourceLoadFailure(
    AssetStatus status,
    yuengine::resource::ResourceLoadCommitStatus resource_status) {
    snapshot_.last_resource_load_status = resource_status;
    return RecordFailure(status);
}

AssetStatus AssetManager::RecordResourceResidencyFailure(
    AssetStatus status,
    yuengine::resource::ResourceResidencyStatus resource_status) {
    snapshot_.last_resource_residency_status = resource_status;
    return RecordFailure(status);
}

AssetStatus AssetManager::RecordTextureFailure(
    AssetStatus status,
    yuengine::streaming::ResourceDecodedTextureBridgeStatus texture_status) {
    snapshot_.last_texture_status = texture_status;
    return RecordFailure(status);
}

AssetStatus AssetManager::RecordAudioFailure(
    AssetStatus status,
    yuengine::audioresource::AudioResourcePcmPacketImportStatus audio_status) {
    snapshot_.last_audio_status = audio_status;
    return RecordFailure(status);
}

void AssetManager::RecordSuccess() {
    ++snapshot_.accepted_operation_count;
    snapshot_.last_status = AssetStatus::Success;
    ClearRequiredCounts(snapshot_);
    ClearCapacityEntry(snapshot_);
}

AssetStatus AssetManager::ResolveHandle(AssetHandle handle, std::size_t &out_index) const {
    if (!handle.IsValid()) {
        return AssetStatus::InvalidHandle;
    }

    if (handle.slot >= snapshot_.asset_capacity) {
        return AssetStatus::InvalidHandle;
    }

    const AssetSlot &slot = slots_[handle.slot];
    if (slot.generation == INVALID_ASSET_GENERATION) {
        return AssetStatus::InvalidHandle;
    }

    if (slot.generation != handle.generation) {
        return AssetStatus::GenerationMismatch;
    }

    if (!slot.is_active) {
        return AssetStatus::InvalidHandle;
    }

    out_index = handle.slot;
    return AssetStatus::Success;
}

bool AssetManager::ValidateDescriptor(const AssetDescriptor &descriptor, AssetStatus *output_status) const {
    if (output_status == nullptr) {
        return false;
    }

    if (descriptor.stable_id == 0U) {
        *output_status = AssetStatus::InvalidAssetId;
        return false;
    }

    if (!descriptor.asset_type.IsValid()) {
        *output_status = AssetStatus::InvalidAssetType;
        return false;
    }

    if (!descriptor.resource.IsValid()) {
        *output_status = AssetStatus::InvalidArgument;
        return false;
    }

    if (!descriptor.resource_type.IsValid()) {
        *output_status = AssetStatus::InvalidArgument;
        return false;
    }

    *output_status = AssetStatus::Success;
    return true;
}

bool AssetManager::HasType(AssetTypeId type) const {
    std::uint32_t index = 0U;
    for (const AssetTypeId &registered_type : types_) {
        if (index >= snapshot_.type_count) {
            return false;
        }

        if (registered_type.value == type.value) {
            return true;
        }

        ++index;
    }

    return false;
}

bool AssetManager::HasDuplicateActiveAsset(const AssetDescriptor &descriptor) const {
    for (const AssetSlot &slot : slots_) {
        if (!slot.is_active) {
            continue;
        }

        if (slot.record.asset_type.value != descriptor.asset_type.value) {
            continue;
        }

        if (slot.record.stable_id == descriptor.stable_id) {
            return true;
        }
    }

    return false;
}

bool AssetManager::HasDependencyEdge(AssetHandle dependent, AssetHandle dependency) const {
    for (const AssetDependencyEdge &edge : dependency_edges_) {
        if (!edge.is_active) {
            continue;
        }

        if (edge.dependent.slot != dependent.slot || edge.dependent.generation != dependent.generation) {
            continue;
        }

        if (edge.dependency.slot == dependency.slot && edge.dependency.generation == dependency.generation) {
            return true;
        }
    }

    return false;
}

bool AssetManager::HasDependencyPath(std::uint32_t start_slot, std::uint32_t target_slot) const {
    std::array<std::uint32_t, MAX_ASSET_COUNT> pending_slots{};
    std::array<bool, MAX_ASSET_COUNT> visited_slots{};
    std::uint32_t read_index = 0U;
    std::uint32_t pending_count = 1U;
    pending_slots[0U] = start_slot;
    visited_slots[start_slot] = true;

    while (read_index < pending_count) {
        const std::uint32_t current_slot = pending_slots[read_index];
        ++read_index;
        if (current_slot == target_slot) {
            return true;
        }

        for (const AssetDependencyEdge &edge : dependency_edges_) {
            if (!edge.is_active) {
                continue;
            }

            if (edge.dependent.slot != current_slot) {
                continue;
            }

            const std::uint32_t dependency_slot = edge.dependency.slot;
            if (dependency_slot >= snapshot_.asset_capacity) {
                continue;
            }

            if (visited_slots[dependency_slot]) {
                continue;
            }

            visited_slots[dependency_slot] = true;
            if (pending_count >= MAX_ASSET_COUNT) {
                return true;
            }

            pending_slots[pending_count] = dependency_slot;
            ++pending_count;
        }
    }

    return false;
}

bool AssetManager::IsResourceResidentState(yuengine::resource::ResourceResidencyState state) const {
    if (state == yuengine::resource::ResourceResidencyState::Resident) {
        return true;
    }

    if (state == yuengine::resource::ResourceResidencyState::Pinned) {
        return true;
    }

    return state == yuengine::resource::ResourceResidencyState::Evictable;
}

bool AssetManager::DoResourceHandlesMatch(
    yuengine::resource::ResourceHandle left,
    yuengine::resource::ResourceHandle right) const {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool AssetManager::DoResourceTypesMatch(
    yuengine::resource::ResourceTypeId left,
    yuengine::resource::ResourceTypeId right) const {
    return left.value == right.value;
}

AssetSlot *AssetManager::FindFreeSlot(std::uint32_t *out_slot_index) {
    if (out_slot_index == nullptr) {
        return nullptr;
    }

    std::uint32_t slot_index = 0U;
    for (AssetSlot &slot : slots_) {
        if (slot_index >= snapshot_.asset_capacity) {
            break;
        }

        if (slot.is_active) {
            ++slot_index;
            continue;
        }

        *out_slot_index = slot_index;
        return &slot;
    }

    return nullptr;
}

AssetDependencyEdge *AssetManager::FindFreeDependencyEdge() {
    std::uint32_t edge_index = 0U;
    for (AssetDependencyEdge &edge : dependency_edges_) {
        if (edge_index >= snapshot_.dependency_edge_capacity) {
            break;
        }

        if (edge.is_active) {
            ++edge_index;
            continue;
        }

        return &edge;
    }

    return nullptr;
}

void AssetManager::RegisterTypeIfNeeded(AssetTypeId type) {
    if (HasType(type)) {
        return;
    }

    types_[snapshot_.type_count] = type;
    ++snapshot_.type_count;
}

void AssetManager::ClearDependencyEdgesForSlot(std::uint32_t slot_index) {
    for (AssetDependencyEdge &edge : dependency_edges_) {
        if (!edge.is_active) {
            continue;
        }

        if (edge.dependent.slot != slot_index && edge.dependency.slot != slot_index) {
            continue;
        }

        edge = AssetDependencyEdge{};
        --snapshot_.active_dependency_edge_count;
    }
}

void AssetManager::AdvanceGeneration(AssetSlot &slot) {
    if (slot.generation == std::numeric_limits<std::uint32_t>::max()) {
        slot.generation = 1U;
        return;
    }

    ++slot.generation;
}
}
