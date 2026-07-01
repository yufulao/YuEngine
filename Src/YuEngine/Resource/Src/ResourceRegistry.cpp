// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Src/ResourceRegistry.cpp

#include "YuEngine/Resource/ResourceRegistry.h"

#include <limits>

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::resource {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}

std::uint32_t ReadU32LittleEndian(const std::array<std::uint8_t, MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD> &bytes,
    std::uint32_t offset) {
    const std::uint32_t byte_zero = bytes[offset];
    const std::uint32_t byte_one = static_cast<std::uint32_t>(bytes[offset + 1U]) << 8U;
    const std::uint32_t byte_two = static_cast<std::uint32_t>(bytes[offset + 2U]) << 16U;
    const std::uint32_t byte_three = static_cast<std::uint32_t>(bytes[offset + 3U]) << 24U;
    return byte_zero | byte_one | byte_two | byte_three;
}

bool MatchesDecodeResultClass(
    ResourceDecodePlanAssetClass asset_class,
    ResourceDecodeResultClass result_class) {
    switch (asset_class) {
        case ResourceDecodePlanAssetClass::Texture:
            return result_class == ResourceDecodeResultClass::Texture;
        case ResourceDecodePlanAssetClass::Audio:
            return result_class == ResourceDecodeResultClass::Audio;
        case ResourceDecodePlanAssetClass::Mesh:
            return result_class == ResourceDecodeResultClass::Mesh;
        case ResourceDecodePlanAssetClass::Material:
            return result_class == ResourceDecodeResultClass::Material;
        case ResourceDecodePlanAssetClass::Unknown:
        default:
            break;
    }

    return false;
}

bool HasExplicitPayloadWindow(std::uint64_t payload_window_byte_offset, std::uint64_t payload_window_byte_size) {
    if (payload_window_byte_offset != 0U) {
        return true;
    }

    return payload_window_byte_size != 0U;
}

bool PayloadWindowEndOverflows(std::uint64_t payload_window_byte_offset, std::uint64_t payload_window_byte_size) {
    const std::uint64_t maximum_size = std::numeric_limits<std::uint64_t>::max() - payload_window_byte_offset;
    return payload_window_byte_size > maximum_size;
}

std::uint64_t EffectivePayloadWindowByteOffset(
    std::uint64_t payload_window_byte_offset,
    std::uint64_t payload_window_byte_size) {
    if (payload_window_byte_size == 0U) {
        return 0U;
    }

    return payload_window_byte_offset;
}

std::uint64_t EffectivePayloadWindowByteSize(
    std::uint64_t payload_window_byte_size,
    std::uint32_t payload_byte_count) {
    if (payload_window_byte_size == 0U) {
        return payload_byte_count;
    }

    return payload_window_byte_size;
}

bool PayloadLogicalByteCountOutOfBounds(
    std::uint64_t payload_logical_byte_count,
    std::uint64_t payload_window_byte_offset,
    std::uint64_t payload_window_byte_size) {
    if (payload_logical_byte_count == 0U) {
        return false;
    }

    if (PayloadWindowEndOverflows(payload_window_byte_offset, payload_window_byte_size)) {
        return true;
    }

    const std::uint64_t payload_window_end = payload_window_byte_offset + payload_window_byte_size;
    return payload_logical_byte_count < payload_window_end;
}

void ClearLastRequiredCounts(ResourceSnapshot &snapshot) {
    snapshot.last_required_resource_count = 0U;
    snapshot.last_required_type_count = 0U;
    snapshot.last_required_dependency_edge_count = 0U;
}

std::uint64_t EffectivePayloadLogicalByteCount(
    std::uint64_t payload_logical_byte_count,
    std::uint64_t payload_window_byte_offset,
    std::uint64_t payload_window_byte_size,
    std::uint32_t payload_byte_count) {
    if (payload_logical_byte_count != 0U) {
        return payload_logical_byte_count;
    }

    if (payload_window_byte_size == 0U) {
        return payload_byte_count;
    }

    return payload_window_byte_offset + payload_window_byte_size;
}
}

ResourceRegistry::ResourceRegistry()
    : ResourceRegistry(ResourceRegistryDesc{}) {
}

ResourceRegistry::ResourceRegistry(ResourceRegistryDesc desc)
    : slots_{},
      dependency_edges_{},
      load_commit_records_{},
      residency_records_{},
      cache_payload_records_{},
      decode_plan_records_{},
      decode_result_records_{},
      decoded_payload_records_{},
      cache_payload_bytes_{},
      decoded_payload_bytes_{},
      types_{},
      snapshot_{
          ClampCapacity(desc.resource_capacity, MAX_RESOURCE_COUNT),
          ClampCapacity(desc.type_capacity, MAX_RESOURCE_TYPE_COUNT),
          ClampCapacity(desc.dependency_edge_capacity, MAX_DEPENDENCY_EDGE_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
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
          ResourceStatus::Success,
          ResourceLoadState::Unloaded,
          ResourceLoadCommitStatus::Success},
      residency_snapshot_{},
      cache_payload_snapshot_{},
      decode_plan_snapshot_{},
      decode_result_snapshot_{},
      decoded_payload_snapshot_{} {
}

ResourceRegistrationResult ResourceRegistry::RegisterSyntheticDescriptor(const ResourceDescriptor& descriptor) {
    if (!descriptor.type.IsValid()) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::InvalidDescriptor));
    }

    if (!descriptor.logical_key.IsWithinBounds()) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::InvalidDescriptor));
    }

    if (!descriptor.logical_key.IsValid()) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::InvalidDescriptor));
    }

    if (HasDuplicateActiveResource(descriptor)) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::DuplicateResource));
    }

    if (snapshot_.registered_resource_count >= snapshot_.resource_capacity) {
        const std::uint32_t required_resource_count = snapshot_.registered_resource_count + 1U;
        const ResourceStatus status = RecordFailure(ResourceStatus::CapacityExceeded);
        snapshot_.last_required_resource_count = required_resource_count;
        return ResourceRegistrationResult::Failure(status, required_resource_count, 0U, 0U);
    }

    ResourceSlot* free_slot = nullptr;
    std::uint32_t free_slot_index = 0U;
    std::uint32_t slot_index = 0U;
    for (ResourceSlot& slot : slots_) {
        if (slot_index >= snapshot_.resource_capacity) {
            break;
        }

        if (slot.is_active) {
            ++slot_index;
            continue;
        }

        free_slot = &slot;
        free_slot_index = slot_index;
        break;
    }

    if (free_slot == nullptr) {
        const std::uint32_t required_resource_count = snapshot_.registered_resource_count + 1U;
        const ResourceStatus status = RecordFailure(ResourceStatus::CapacityExceeded);
        snapshot_.last_required_resource_count = required_resource_count;
        return ResourceRegistrationResult::Failure(status, required_resource_count, 0U, 0U);
    }

    const ResourceStatus type_status = RegisterTypeIfNeeded(descriptor.type);
    if (type_status != ResourceStatus::Success) {
        std::uint32_t required_type_count = 0U;
        if (type_status == ResourceStatus::CapacityExceeded) {
            required_type_count = snapshot_.type_count + 1U;
        }

        const ResourceStatus status = RecordFailure(type_status);
        snapshot_.last_required_type_count = required_type_count;
        return ResourceRegistrationResult::Failure(status, 0U, required_type_count, 0U);
    }

    if (free_slot->generation == INVALID_RESOURCE_GENERATION) {
        free_slot->generation = 1U;
    }

    free_slot->type = descriptor.type;
    free_slot->logical_key = descriptor.logical_key;
    free_slot->reference_count = descriptor.initial_reference_count;
    free_slot->is_active = true;
    ++snapshot_.registered_resource_count;
    snapshot_.acquired_handle_count += descriptor.initial_reference_count;
    RecordSuccess();
    return ResourceRegistrationResult::Success(ResourceHandle{free_slot_index, free_slot->generation});
}

ResourceStatus ResourceRegistry::AddDependency(ResourceHandle dependent, ResourceHandle dependency) {
    ++snapshot_.dependency_validation_count;

    std::size_t dependent_index = 0U;
    const ResourceStatus dependent_status = ResolveHandle(dependent, dependent_index);
    if (dependent_status != ResourceStatus::Success) {
        return RecordFailure(dependent_status);
    }

    std::size_t dependency_index = 0U;
    const ResourceStatus dependency_status = ResolveHandle(dependency, dependency_index);
    if (dependency_status != ResourceStatus::Success) {
        return RecordFailure(ResourceStatus::DependencyMissing);
    }

    if (dependent_index == dependency_index) {
        return RecordFailure(ResourceStatus::DependencyCycle);
    }

    if (HasDependencyPath(dependency_index, dependent_index)) {
        return RecordFailure(ResourceStatus::DependencyCycle);
    }

    for (const ResourceDependencyEdge& edge : dependency_edges_) {
        if (!edge.is_active) {
            continue;
        }

        if (edge.dependent_slot != dependent.slot) {
            continue;
        }

        if (edge.dependency_slot == dependency.slot) {
            RecordSuccess();
            return ResourceStatus::Success;
        }
    }

    if (snapshot_.dependency_edge_count >= snapshot_.dependency_edge_capacity) {
        const std::uint32_t required_dependency_edge_count = snapshot_.dependency_edge_count + 1U;
        const ResourceStatus status = RecordFailure(ResourceStatus::CapacityExceeded);
        snapshot_.last_required_dependency_edge_count = required_dependency_edge_count;
        return status;
    }

    for (ResourceDependencyEdge& edge : dependency_edges_) {
        if (edge.is_active) {
            continue;
        }

        edge.dependent_slot = dependent.slot;
        edge.dependency_slot = dependency.slot;
        edge.is_active = true;
        ++snapshot_.dependency_edge_count;
        RecordSuccess();
        return ResourceStatus::Success;
    }

    const std::uint32_t required_dependency_edge_count = snapshot_.dependency_edge_count + 1U;
    const ResourceStatus status = RecordFailure(ResourceStatus::CapacityExceeded);
    snapshot_.last_required_dependency_edge_count = required_dependency_edge_count;
    return status;
}

ResourceStatus ResourceRegistry::Acquire(ResourceHandle handle, ResourceTypeId expected_type) {
    std::size_t slot_index = 0U;
    const ResourceStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != ResourceStatus::Success) {
        return RecordFailure(handle_status);
    }

    ResourceSlot& slot = slots_[slot_index];
    if (slot.type.value != expected_type.value) {
        return RecordFailure(ResourceStatus::TypeMismatch);
    }

    if (slot.reference_count == std::numeric_limits<std::uint32_t>::max()) {
        return RecordFailure(ResourceStatus::ReferenceCountOverflow);
    }

    if (IsEvictionCandidate(slot)) {
        residency_snapshot_.evictable_byte_count -= slot.loaded_byte_count;
        slot.residency_state = ResourceResidencyState::Resident;
    }

    ++slot.reference_count;
    ++snapshot_.acquired_handle_count;
    RecordSuccess();
    return ResourceStatus::Success;
}

ResourceStatus ResourceRegistry::ValidateAcquire(
    ResourceHandle handle,
    ResourceTypeId expected_type,
    std::uint32_t projected_acquire_count) const {
    std::size_t slot_index = 0U;
    const ResourceStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != ResourceStatus::Success) {
        return handle_status;
    }

    const ResourceSlot &slot = slots_[slot_index];
    if (slot.type.value != expected_type.value) {
        return ResourceStatus::TypeMismatch;
    }

    if (projected_acquire_count == 0U) {
        return ResourceStatus::Success;
    }

    const std::uint32_t max_reference_count = std::numeric_limits<std::uint32_t>::max();
    const std::uint32_t remaining_reference_count = max_reference_count - slot.reference_count;
    if (projected_acquire_count > remaining_reference_count) {
        return ResourceStatus::ReferenceCountOverflow;
    }

    return ResourceStatus::Success;
}

ResourceLoadCommitStatus ResourceRegistry::CommitUploadCompletion(const ResourceLoadCommitRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceLoadCommitStatus validation_status = ValidateLoadCommitRequest(request, &slot_index);
    if (validation_status != ResourceLoadCommitStatus::Success) {
        return RecordLoadCommitRejected(validation_status);
    }

    ResourceSlot &slot = slots_[slot_index];
    slot.load_state = request.load_state;
    slot.last_load_commit_id = request.commit_id;
    slot.last_upload_id = request.upload_id;
    slot.last_staging_request_id = request.staging_request_id;
    slot.loaded_byte_count = request.upload_byte_count;
    if (request.load_state == ResourceLoadState::Uploaded) {
        slot.residency_state = ResourceResidencyState::Uploaded;
    }

    if (request.load_state == ResourceLoadState::Failed) {
        slot.residency_state = ResourceResidencyState::Failed;
    }

    StoreLoadCommitRecord(request);
    RecordLoadCommitSuccess(request.load_state);
    return ResourceLoadCommitStatus::Success;
}

ResourceLoadCommitStatus ResourceRegistry::GetLoadState(
    ResourceHandle handle,
    ResourceTypeId expected_type,
    ResourceLoadState *output_state) const {
    if (output_state == nullptr) {
        return ResourceLoadCommitStatus::InvalidArgument;
    }

    std::size_t slot_index = 0U;
    const ResourceStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != ResourceStatus::Success) {
        return MapHandleStatus(handle_status);
    }

    const ResourceSlot &slot = slots_[slot_index];
    if (slot.type.value != expected_type.value) {
        return ResourceLoadCommitStatus::TypeMismatch;
    }

    *output_state = slot.load_state;
    return ResourceLoadCommitStatus::Success;
}

ResourceResidencyStatus ResourceRegistry::SetResidencyBudget(ResourceResidencyBudgetDesc desc) {
    const ResourceResidencyRequest request{};
    if (residency_snapshot_.residency_record_count >= MAX_RESOURCE_RESIDENCY_RECORD_COUNT) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::ConfigureBudget,
            request,
            ResourceResidencyStatus::CapacityExceeded);
    }

    if (desc.byte_capacity == 0U) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::ConfigureBudget,
            request,
            ResourceResidencyStatus::InvalidArgument);
    }

    if (desc.byte_capacity < residency_snapshot_.resident_byte_count) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::ConfigureBudget,
            request,
            ResourceResidencyStatus::BudgetExceeded);
    }

    residency_snapshot_.budget_byte_capacity = desc.byte_capacity;
    RecordResidencySuccess(
        ResourceResidencyOperation::ConfigureBudget,
        request,
        ResourceResidencyState::Unloaded);
    return ResourceResidencyStatus::Success;
}

ResourceResidencyStatus ResourceRegistry::AdmitResident(const ResourceResidencyRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceResidencyStatus validation_status = ValidateResidencyRequest(request, &slot_index);
    if (validation_status != ResourceResidencyStatus::Success) {
        return RecordResidencyRejected(ResourceResidencyOperation::Admit, request, validation_status);
    }

    ResourceSlot &slot = slots_[slot_index];
    if (slot.load_state == ResourceLoadState::Failed) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::Admit,
            request,
            ResourceResidencyStatus::FailedLoad);
    }

    if (slot.load_state != ResourceLoadState::Uploaded) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::Admit,
            request,
            ResourceResidencyStatus::NotUploaded);
    }

    if (IsResidentState(slot.residency_state)) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::Admit,
            request,
            ResourceResidencyStatus::AlreadyResident);
    }

    const std::uint32_t remaining_byte_capacity =
        residency_snapshot_.budget_byte_capacity - residency_snapshot_.resident_byte_count;
    if (slot.loaded_byte_count > remaining_byte_capacity) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::Admit,
            request,
            ResourceResidencyStatus::BudgetExceeded);
    }

    if (slot.residency_state == ResourceResidencyState::Evicted) {
        --residency_snapshot_.evicted_resource_count;
    }

    slot.residency_state = ResourceResidencyState::Resident;
    ++residency_snapshot_.resident_resource_count;
    residency_snapshot_.resident_byte_count += slot.loaded_byte_count;
    RefreshEvictableState(slot);
    RecordResidencySuccess(ResourceResidencyOperation::Admit, request, slot.residency_state);
    return ResourceResidencyStatus::Success;
}

ResourceResidencyStatus ResourceRegistry::PinResident(const ResourceResidencyRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceResidencyStatus validation_status = ValidateResidencyRequest(request, &slot_index);
    if (validation_status != ResourceResidencyStatus::Success) {
        return RecordResidencyRejected(ResourceResidencyOperation::Pin, request, validation_status);
    }

    ResourceSlot &slot = slots_[slot_index];
    if (!IsResidentState(slot.residency_state)) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::Pin,
            request,
            ResourceResidencyStatus::NotResident);
    }

    if (slot.residency_state == ResourceResidencyState::Pinned) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::Pin,
            request,
            ResourceResidencyStatus::AlreadyPinned);
    }

    if (IsEvictionCandidate(slot)) {
        residency_snapshot_.evictable_byte_count -= slot.loaded_byte_count;
    }

    slot.residency_state = ResourceResidencyState::Pinned;
    ++residency_snapshot_.pinned_resource_count;
    residency_snapshot_.pinned_byte_count += slot.loaded_byte_count;
    RecordResidencySuccess(ResourceResidencyOperation::Pin, request, slot.residency_state);
    return ResourceResidencyStatus::Success;
}

ResourceResidencyStatus ResourceRegistry::UnpinResident(const ResourceResidencyRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceResidencyStatus validation_status = ValidateResidencyRequest(request, &slot_index);
    if (validation_status != ResourceResidencyStatus::Success) {
        return RecordResidencyRejected(ResourceResidencyOperation::Unpin, request, validation_status);
    }

    ResourceSlot &slot = slots_[slot_index];
    if (!IsResidentState(slot.residency_state)) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::Unpin,
            request,
            ResourceResidencyStatus::NotResident);
    }

    if (slot.residency_state != ResourceResidencyState::Pinned) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::Unpin,
            request,
            ResourceResidencyStatus::NotPinned);
    }

    slot.residency_state = ResourceResidencyState::Resident;
    --residency_snapshot_.pinned_resource_count;
    residency_snapshot_.pinned_byte_count -= slot.loaded_byte_count;
    RefreshEvictableState(slot);
    RecordResidencySuccess(ResourceResidencyOperation::Unpin, request, slot.residency_state);
    return ResourceResidencyStatus::Success;
}

ResourceResidencyStatus ResourceRegistry::EvictResident(const ResourceResidencyRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceResidencyStatus validation_status = ValidateResidencyRequest(request, &slot_index);
    if (validation_status != ResourceResidencyStatus::Success) {
        return RecordResidencyRejected(ResourceResidencyOperation::Evict, request, validation_status);
    }

    ResourceSlot &slot = slots_[slot_index];
    if (!IsResidentState(slot.residency_state)) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::Evict,
            request,
            ResourceResidencyStatus::NotResident);
    }

    if (slot.residency_state == ResourceResidencyState::Pinned) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::Evict,
            request,
            ResourceResidencyStatus::Pinned);
    }

    if (slot.reference_count != 0U) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::Evict,
            request,
            ResourceResidencyStatus::StillReferenced);
    }

    ClearCachePayloadForSlot(slot_index);
    RemoveResidentCounters(slot);
    slot.residency_state = ResourceResidencyState::Evicted;
    ++residency_snapshot_.evicted_resource_count;
    RecordResidencySuccess(ResourceResidencyOperation::Evict, request, slot.residency_state);
    return ResourceResidencyStatus::Success;
}

ResourceResidencyStatus ResourceRegistry::SelectEvictionCandidate(ResourceHandle *output_handle) {
    const ResourceResidencyRequest empty_request{};
    if (output_handle == nullptr) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::SelectCandidate,
            empty_request,
            ResourceResidencyStatus::InvalidArgument);
    }

    *output_handle = ResourceHandle{};
    if (residency_snapshot_.residency_record_count >= MAX_RESOURCE_RESIDENCY_RECORD_COUNT) {
        return RecordResidencyRejected(
            ResourceResidencyOperation::SelectCandidate,
            empty_request,
            ResourceResidencyStatus::CapacityExceeded);
    }

    std::uint32_t slot_index = 0U;
    for (const ResourceSlot &slot : slots_) {
        if (slot_index >= snapshot_.resource_capacity) {
            break;
        }

        if (!IsEvictionCandidate(slot)) {
            ++slot_index;
            continue;
        }

        const ResourceHandle candidate{slot_index, slot.generation};
        *output_handle = candidate;
        residency_snapshot_.last_candidate = candidate;
        ResourceResidencyRequest candidate_request;
        candidate_request.resource = candidate;
        candidate_request.expected_type = slot.type;
        RecordResidencySuccess(
            ResourceResidencyOperation::SelectCandidate,
            candidate_request,
            slot.residency_state);
        return ResourceResidencyStatus::Success;
    }

    ++residency_snapshot_.eviction_candidate_miss_count;
    return RecordResidencyRejected(
        ResourceResidencyOperation::SelectCandidate,
        empty_request,
        ResourceResidencyStatus::NoCandidate);
}

ResourceResidencyStatus ResourceRegistry::GetResidencyState(
    ResourceHandle handle,
    ResourceTypeId expected_type,
    ResourceResidencyState *output_state) const {
    if (output_state == nullptr) {
        return ResourceResidencyStatus::InvalidArgument;
    }

    std::size_t slot_index = 0U;
    const ResourceStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != ResourceStatus::Success) {
        return MapHandleResidencyStatus(handle_status);
    }

    const ResourceSlot &slot = slots_[slot_index];
    if (slot.type.value != expected_type.value) {
        return ResourceResidencyStatus::TypeMismatch;
    }

    *output_state = slot.residency_state;
    return ResourceResidencyStatus::Success;
}

ResourceResidencySnapshot ResourceRegistry::ResidencySnapshot() const {
    return residency_snapshot_;
}

ResourceCachePayloadStatus ResourceRegistry::SetCachePayloadBudget(ResourceCachePayloadBudgetDesc desc) {
    const ResourceCachePayloadRequest request{};
    if (desc.byte_capacity == 0U) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::ConfigureBudget,
            request,
            ResourceCachePayloadStatus::InvalidArgument);
    }

    if (desc.payload_reference_capacity == 0U) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::ConfigureBudget,
            request,
            ResourceCachePayloadStatus::InvalidArgument);
    }

    if (desc.payload_reference_capacity > MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::ConfigureBudget,
            request,
            ResourceCachePayloadStatus::InvalidArgument);
    }

    if (desc.byte_capacity < cache_payload_snapshot_.cached_byte_count) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::ConfigureBudget,
            request,
            ResourceCachePayloadStatus::BudgetExceeded);
    }

    if (desc.payload_reference_capacity < cache_payload_snapshot_.cached_payload_count) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::ConfigureBudget,
            request,
            ResourceCachePayloadStatus::ReferenceBudgetExceeded);
    }

    cache_payload_snapshot_.budget_byte_capacity = desc.byte_capacity;
    cache_payload_snapshot_.budget_payload_reference_capacity = desc.payload_reference_capacity;
    RecordCachePayloadSuccess(
        ResourceCachePayloadOperation::ConfigureBudget,
        request,
        INVALID_RESOURCE_SLOT,
        0U);
    return ResourceCachePayloadStatus::Success;
}

ResourceCachePayloadStatus ResourceRegistry::StoreCachePayload(const ResourceCachePayloadRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceCachePayloadStatus validation_status = ValidateCachePayloadRequest(request, &slot_index);
    if (validation_status != ResourceCachePayloadStatus::Success) {
        return RecordCachePayloadRejected(ResourceCachePayloadOperation::Store, request, validation_status);
    }

    if (request.payload_bytes == nullptr) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Store,
            request,
            ResourceCachePayloadStatus::InvalidArgument);
    }

    if (request.payload_byte_count == 0U) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Store,
            request,
            ResourceCachePayloadStatus::EmptyPayload);
    }

    if (request.payload_byte_count > MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Store,
            request,
            ResourceCachePayloadStatus::CapacityExceeded);
    }

    const ResourceCachePayloadStatus window_status = ValidateCachePayloadStoreWindow(request);
    if (window_status != ResourceCachePayloadStatus::Success) {
        return RecordCachePayloadRejected(ResourceCachePayloadOperation::Store, request, window_status);
    }

    if (HasCachePayloadId(request.payload_id)) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Store,
            request,
            ResourceCachePayloadStatus::DuplicatePayloadId);
    }

    if (cache_payload_snapshot_.cached_payload_count >= cache_payload_snapshot_.budget_payload_reference_capacity) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Store,
            request,
            ResourceCachePayloadStatus::ReferenceBudgetExceeded);
    }

    if (cache_payload_snapshot_.cached_payload_count >= MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Store,
            request,
            ResourceCachePayloadStatus::CapacityExceeded);
    }

    const std::uint32_t remaining_byte_capacity =
        cache_payload_snapshot_.budget_byte_capacity - cache_payload_snapshot_.cached_byte_count;
    if (request.payload_byte_count > remaining_byte_capacity) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Store,
            request,
            ResourceCachePayloadStatus::BudgetExceeded);
    }

    std::size_t record_index = 0U;
    if (!FindFreeCachePayloadRecord(&record_index)) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Store,
            request,
            ResourceCachePayloadStatus::CapacityExceeded);
    }

    std::uint32_t byte_index = 0U;
    while (byte_index < request.payload_byte_count) {
        cache_payload_bytes_[record_index][byte_index] = request.payload_bytes[byte_index];
        ++byte_index;
    }

    ResourceCachePayloadRecord &record = cache_payload_records_[record_index];
    record.operation = ResourceCachePayloadOperation::Store;
    record.resource = request.resource;
    record.expected_type = request.expected_type;
    record.payload_id = request.payload_id;
    record.payload_logical_byte_count = EffectivePayloadLogicalByteCount(
        request.payload_logical_byte_count,
        request.payload_window_byte_offset,
        request.payload_window_byte_size,
        request.payload_byte_count);
    record.payload_window_byte_offset = EffectivePayloadWindowByteOffset(
        request.payload_window_byte_offset,
        request.payload_window_byte_size);
    record.payload_window_byte_size = EffectivePayloadWindowByteSize(
        request.payload_window_byte_size,
        request.payload_byte_count);
    record.payload_byte_count = request.payload_byte_count;
    record.cache_slot_index = static_cast<std::uint32_t>(record_index);
    record.status = ResourceCachePayloadStatus::Success;
    record.is_active = true;
    ++cache_payload_snapshot_.cached_payload_count;
    ++cache_payload_snapshot_.cache_payload_record_count;
    cache_payload_snapshot_.cached_byte_count += request.payload_byte_count;
    RecordCachePayloadSuccess(
        ResourceCachePayloadOperation::Store,
        request,
        record.cache_slot_index,
        request.payload_byte_count);
    return ResourceCachePayloadStatus::Success;
}

ResourceCachePayloadStatus ResourceRegistry::ReadCachePayload(
    const ResourceCachePayloadRequest &request,
    std::uint8_t *output_bytes,
    std::uint32_t output_byte_capacity,
    std::uint32_t *output_byte_count) {
    if (output_byte_count == nullptr) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Read,
            request,
            ResourceCachePayloadStatus::InvalidArgument);
    }

    *output_byte_count = 0U;
    if (output_bytes == nullptr) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Read,
            request,
            ResourceCachePayloadStatus::InvalidArgument);
    }

    std::size_t slot_index = 0U;
    const ResourceCachePayloadStatus validation_status = ValidateCachePayloadRequest(request, &slot_index);
    if (validation_status != ResourceCachePayloadStatus::Success) {
        return RecordCachePayloadRejected(ResourceCachePayloadOperation::Read, request, validation_status);
    }

    std::size_t record_index = 0U;
    if (!FindCachePayloadRecord(request.resource, request.payload_id, &record_index)) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Read,
            request,
            ResourceCachePayloadStatus::MissingPayload);
    }

    const ResourceCachePayloadRecord &record = cache_payload_records_[record_index];
    std::uint32_t read_byte_offset = 0U;
    std::uint32_t read_byte_count = 0U;
    const ResourceCachePayloadStatus window_status = ResolveCachePayloadReadWindow(
        request,
        record,
        &read_byte_offset,
        &read_byte_count);
    if (window_status != ResourceCachePayloadStatus::Success) {
        return RecordCachePayloadRejected(ResourceCachePayloadOperation::Read, request, window_status);
    }

    if (output_byte_capacity < read_byte_count) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Read,
            request,
            ResourceCachePayloadStatus::OutputBufferTooSmall);
    }

    std::uint32_t byte_index = 0U;
    while (byte_index < read_byte_count) {
        const std::uint32_t source_byte_index = read_byte_offset + byte_index;
        output_bytes[byte_index] = cache_payload_bytes_[record_index][source_byte_index];
        ++byte_index;
    }

    *output_byte_count = read_byte_count;
    RecordCachePayloadSuccess(
        ResourceCachePayloadOperation::Read,
        request,
        record.cache_slot_index,
        read_byte_count);
    return ResourceCachePayloadStatus::Success;
}

ResourceCachePayloadStatus ResourceRegistry::ReleaseCachePayload(const ResourceCachePayloadRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceCachePayloadStatus validation_status = ValidateCachePayloadRequest(request, &slot_index);
    if (validation_status != ResourceCachePayloadStatus::Success) {
        return RecordCachePayloadRejected(ResourceCachePayloadOperation::Release, request, validation_status);
    }

    std::size_t record_index = 0U;
    if (!FindCachePayloadRecord(request.resource, request.payload_id, &record_index)) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Release,
            request,
            ResourceCachePayloadStatus::MissingPayload);
    }

    const ResourceSlot &slot = slots_[slot_index];
    if (slot.residency_state == ResourceResidencyState::Pinned) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Release,
            request,
            ResourceCachePayloadStatus::Pinned);
    }

    const ResourceCachePayloadRecord &record = cache_payload_records_[record_index];
    const std::uint32_t cache_slot_index = record.cache_slot_index;
    const std::uint32_t payload_byte_count = record.payload_byte_count;
    ClearCachePayloadRecord(record_index);
    RecordCachePayloadSuccess(
        ResourceCachePayloadOperation::Release,
        request,
        cache_slot_index,
        payload_byte_count);
    return ResourceCachePayloadStatus::Success;
}

ResourceCachePayloadSnapshot ResourceRegistry::CachePayloadSnapshot() const {
    return cache_payload_snapshot_;
}

ResourceDecodePlanStatus ResourceRegistry::SetDecodePlanBudget(ResourceDecodePlanBudgetDesc desc) {
    const ResourceDecodePlanRequest request{};
    if (desc.decoded_byte_capacity == 0U) {
        return RecordDecodePlanRejected(
            ResourceDecodePlanOperation::ConfigureBudget,
            request,
            ResourceDecodePlanStatus::InvalidArgument);
    }

    if (desc.decoded_byte_capacity < decode_plan_snapshot_.planned_decoded_byte_count) {
        return RecordDecodePlanRejected(
            ResourceDecodePlanOperation::ConfigureBudget,
            request,
            ResourceDecodePlanStatus::BudgetExceeded);
    }

    decode_plan_snapshot_.budget_decoded_byte_capacity = desc.decoded_byte_capacity;
    RecordDecodePlanSuccess(
        ResourceDecodePlanOperation::ConfigureBudget,
        request,
        INVALID_RESOURCE_SLOT,
        0U);
    return ResourceDecodePlanStatus::Success;
}

ResourceDecodePlanStatus ResourceRegistry::CreateDecodePlan(const ResourceDecodePlanRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceDecodePlanStatus validation_status = ValidateDecodePlanRequest(request, &slot_index);
    if (validation_status != ResourceDecodePlanStatus::Success) {
        return RecordDecodePlanRejected(ResourceDecodePlanOperation::Create, request, validation_status);
    }

    std::size_t cache_record_index = 0U;
    if (!FindCachePayloadRecord(request.resource, request.payload_id, &cache_record_index)) {
        return RecordDecodePlanRejected(
            ResourceDecodePlanOperation::Create,
            request,
            ResourceDecodePlanStatus::MissingCachePayload);
    }

    const ResourceCachePayloadRecord &cache_payload_record = cache_payload_records_[cache_record_index];
    std::uint32_t header_version = 0U;
    const ResourceDecodePlanStatus header_status = ValidateDecodePlanHeader(
        request,
        cache_payload_record,
        &header_version);
    if (header_status != ResourceDecodePlanStatus::Success) {
        return RecordDecodePlanRejected(ResourceDecodePlanOperation::Create, request, header_status);
    }

    if (HasDecodePlanId(request.decode_plan_id)) {
        return RecordDecodePlanRejected(
            ResourceDecodePlanOperation::Create,
            request,
            ResourceDecodePlanStatus::DuplicatePlanId);
    }

    if (decode_plan_snapshot_.active_plan_count >= MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT) {
        return RecordDecodePlanRejected(
            ResourceDecodePlanOperation::Create,
            request,
            ResourceDecodePlanStatus::CapacityExceeded);
    }

    const std::uint32_t remaining_decoded_byte_capacity =
        decode_plan_snapshot_.budget_decoded_byte_capacity - decode_plan_snapshot_.planned_decoded_byte_count;
    if (request.expected_decoded_byte_count > remaining_decoded_byte_capacity) {
        return RecordDecodePlanRejected(
            ResourceDecodePlanOperation::Create,
            request,
            ResourceDecodePlanStatus::BudgetExceeded);
    }

    std::size_t record_index = 0U;
    if (!FindFreeDecodePlanRecord(&record_index)) {
        return RecordDecodePlanRejected(
            ResourceDecodePlanOperation::Create,
            request,
            ResourceDecodePlanStatus::CapacityExceeded);
    }

    ResourceDecodePlanRecord &record = decode_plan_records_[record_index];
    record.operation = ResourceDecodePlanOperation::Create;
    record.resource = request.resource;
    record.expected_type = request.expected_type;
    record.payload_id = request.payload_id;
    record.decode_plan_id = request.decode_plan_id;
    record.asset_class = request.asset_class;
    record.source_byte_count = request.source_byte_count;
    record.expected_decoded_byte_count = request.expected_decoded_byte_count;
    record.header_version = header_version;
    record.cache_slot_index = cache_payload_record.cache_slot_index;
    record.status = ResourceDecodePlanStatus::Success;
    record.is_active = true;
    ++decode_plan_snapshot_.active_plan_count;
    ++decode_plan_snapshot_.decode_plan_record_count;
    decode_plan_snapshot_.planned_decoded_byte_count += request.expected_decoded_byte_count;
    RecordDecodePlanSuccess(
        ResourceDecodePlanOperation::Create,
        request,
        record.cache_slot_index,
        header_version);
    return ResourceDecodePlanStatus::Success;
}

ResourceDecodePlanStatus ResourceRegistry::QueryDecodePlan(
    const ResourceDecodePlanRequest &request,
    ResourceDecodePlanRecord *output_record) {
    if (output_record == nullptr) {
        return RecordDecodePlanRejected(
            ResourceDecodePlanOperation::Query,
            request,
            ResourceDecodePlanStatus::InvalidArgument);
    }

    *output_record = ResourceDecodePlanRecord{};
    std::size_t slot_index = 0U;
    const ResourceDecodePlanStatus validation_status = ValidateDecodePlanRequest(request, &slot_index);
    if (validation_status != ResourceDecodePlanStatus::Success) {
        return RecordDecodePlanRejected(ResourceDecodePlanOperation::Query, request, validation_status);
    }

    std::size_t record_index = 0U;
    if (!FindDecodePlanRecord(request.resource, request.payload_id, request.decode_plan_id, &record_index)) {
        return RecordDecodePlanRejected(
            ResourceDecodePlanOperation::Query,
            request,
            ResourceDecodePlanStatus::MissingPlan);
    }

    *output_record = decode_plan_records_[record_index];
    RecordDecodePlanSuccess(
        ResourceDecodePlanOperation::Query,
        request,
        output_record->cache_slot_index,
        output_record->header_version);
    return ResourceDecodePlanStatus::Success;
}

ResourceDecodePlanStatus ResourceRegistry::ReleaseDecodePlan(const ResourceDecodePlanRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceDecodePlanStatus validation_status = ValidateDecodePlanRequest(request, &slot_index);
    if (validation_status != ResourceDecodePlanStatus::Success) {
        return RecordDecodePlanRejected(ResourceDecodePlanOperation::Release, request, validation_status);
    }

    std::size_t record_index = 0U;
    if (!FindDecodePlanRecord(request.resource, request.payload_id, request.decode_plan_id, &record_index)) {
        return RecordDecodePlanRejected(
            ResourceDecodePlanOperation::Release,
            request,
            ResourceDecodePlanStatus::MissingPlan);
    }

    const ResourceDecodePlanRecord &record = decode_plan_records_[record_index];
    const std::uint32_t cache_slot_index = record.cache_slot_index;
    const std::uint32_t header_version = record.header_version;
    ClearDecodePlanRecord(record_index);
    RecordDecodePlanSuccess(
        ResourceDecodePlanOperation::Release,
        request,
        cache_slot_index,
        header_version);
    return ResourceDecodePlanStatus::Success;
}

ResourceDecodePlanSnapshot ResourceRegistry::DecodePlanSnapshot() const {
    return decode_plan_snapshot_;
}

ResourceDecodeResultStatus ResourceRegistry::SetDecodeResultBudget(ResourceDecodeResultBudgetDesc desc) {
    const ResourceDecodeResultRequest request{};
    if (desc.decoded_byte_capacity == 0U) {
        return RecordDecodeResultRejected(
            ResourceDecodeResultOperation::ConfigureBudget,
            request,
            ResourceDecodeResultStatus::InvalidArgument);
    }

    if (desc.decoded_byte_capacity < decode_result_snapshot_.committed_decoded_byte_count) {
        return RecordDecodeResultRejected(
            ResourceDecodeResultOperation::ConfigureBudget,
            request,
            ResourceDecodeResultStatus::BudgetExceeded);
    }

    decode_result_snapshot_.budget_decoded_byte_capacity = desc.decoded_byte_capacity;
    RecordDecodeResultSuccess(
        ResourceDecodeResultOperation::ConfigureBudget,
        request,
        INVALID_RESOURCE_SLOT);
    return ResourceDecodeResultStatus::Success;
}

ResourceDecodeResultStatus ResourceRegistry::CommitDecodeResult(const ResourceDecodeResultRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceDecodeResultStatus validation_status = ValidateDecodeResultRequest(request, &slot_index);
    if (validation_status != ResourceDecodeResultStatus::Success) {
        return RecordDecodeResultRejected(ResourceDecodeResultOperation::Commit, request, validation_status);
    }

    std::size_t decode_plan_record_index = 0U;
    if (!FindDecodePlanRecord(request.resource, request.payload_id, request.decode_plan_id, &decode_plan_record_index)) {
        return RecordDecodeResultRejected(
            ResourceDecodeResultOperation::Commit,
            request,
            ResourceDecodeResultStatus::MissingDecodePlan);
    }

    const ResourceDecodePlanRecord &decode_plan_record = decode_plan_records_[decode_plan_record_index];
    const ResourceDecodeResultStatus plan_status = ValidateDecodeResultPlan(request, decode_plan_record);
    if (plan_status != ResourceDecodeResultStatus::Success) {
        return RecordDecodeResultRejected(ResourceDecodeResultOperation::Commit, request, plan_status);
    }

    if (HasDecodeResultId(request.decode_result_id)) {
        return RecordDecodeResultRejected(
            ResourceDecodeResultOperation::Commit,
            request,
            ResourceDecodeResultStatus::DuplicateDecodeResultId);
    }

    if (decode_result_snapshot_.active_result_count >= MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT) {
        return RecordDecodeResultRejected(
            ResourceDecodeResultOperation::Commit,
            request,
            ResourceDecodeResultStatus::CapacityExceeded);
    }

    const std::uint32_t remaining_decoded_byte_capacity =
        decode_result_snapshot_.budget_decoded_byte_capacity - decode_result_snapshot_.committed_decoded_byte_count;
    if (request.decoded_byte_count > remaining_decoded_byte_capacity) {
        return RecordDecodeResultRejected(
            ResourceDecodeResultOperation::Commit,
            request,
            ResourceDecodeResultStatus::BudgetExceeded);
    }

    std::size_t record_index = 0U;
    if (!FindFreeDecodeResultRecord(&record_index)) {
        return RecordDecodeResultRejected(
            ResourceDecodeResultOperation::Commit,
            request,
            ResourceDecodeResultStatus::CapacityExceeded);
    }

    ResourceDecodeResultRecord &record = decode_result_records_[record_index];
    record.operation = ResourceDecodeResultOperation::Commit;
    record.resource = request.resource;
    record.expected_type = request.expected_type;
    record.payload_id = request.payload_id;
    record.decode_plan_id = request.decode_plan_id;
    record.decode_result_id = request.decode_result_id;
    record.asset_class = request.asset_class;
    record.result_class = request.result_class;
    record.decoded_byte_count = request.decoded_byte_count;
    record.decode_plan_slot_index = static_cast<std::uint32_t>(decode_plan_record_index);
    record.status = ResourceDecodeResultStatus::Success;
    record.is_active = true;
    ++decode_result_snapshot_.active_result_count;
    ++decode_result_snapshot_.decode_result_record_count;
    decode_result_snapshot_.committed_decoded_byte_count += request.decoded_byte_count;
    RecordDecodeResultSuccess(
        ResourceDecodeResultOperation::Commit,
        request,
        record.decode_plan_slot_index);
    return ResourceDecodeResultStatus::Success;
}

ResourceDecodeResultStatus ResourceRegistry::QueryDecodeResult(
    const ResourceDecodeResultRequest &request,
    ResourceDecodeResultRecord *output_record) {
    if (output_record == nullptr) {
        return RecordDecodeResultRejected(
            ResourceDecodeResultOperation::Query,
            request,
            ResourceDecodeResultStatus::InvalidArgument);
    }

    *output_record = ResourceDecodeResultRecord{};
    std::size_t slot_index = 0U;
    const ResourceDecodeResultStatus validation_status = ValidateDecodeResultRequest(request, &slot_index);
    if (validation_status != ResourceDecodeResultStatus::Success) {
        return RecordDecodeResultRejected(ResourceDecodeResultOperation::Query, request, validation_status);
    }

    std::size_t record_index = 0U;
    if (!FindDecodeResultRecord(
        request.resource,
        request.payload_id,
        request.decode_plan_id,
        request.decode_result_id,
        &record_index)) {
        return RecordDecodeResultRejected(
            ResourceDecodeResultOperation::Query,
            request,
            ResourceDecodeResultStatus::MissingDecodeResult);
    }

    *output_record = decode_result_records_[record_index];
    RecordDecodeResultSuccess(
        ResourceDecodeResultOperation::Query,
        request,
        output_record->decode_plan_slot_index);
    return ResourceDecodeResultStatus::Success;
}

ResourceDecodeResultStatus ResourceRegistry::ReleaseDecodeResult(const ResourceDecodeResultRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceDecodeResultStatus validation_status = ValidateDecodeResultRequest(request, &slot_index);
    if (validation_status != ResourceDecodeResultStatus::Success) {
        return RecordDecodeResultRejected(ResourceDecodeResultOperation::Release, request, validation_status);
    }

    std::size_t record_index = 0U;
    if (!FindDecodeResultRecord(
        request.resource,
        request.payload_id,
        request.decode_plan_id,
        request.decode_result_id,
        &record_index)) {
        return RecordDecodeResultRejected(
            ResourceDecodeResultOperation::Release,
            request,
            ResourceDecodeResultStatus::MissingDecodeResult);
    }

    const ResourceDecodeResultRecord &record = decode_result_records_[record_index];
    const std::uint32_t decode_plan_slot_index = record.decode_plan_slot_index;
    ClearDecodeResultRecord(record_index);
    RecordDecodeResultSuccess(
        ResourceDecodeResultOperation::Release,
        request,
        decode_plan_slot_index);
    return ResourceDecodeResultStatus::Success;
}

ResourceDecodeResultSnapshot ResourceRegistry::DecodeResultSnapshot() const {
    return decode_result_snapshot_;
}

ResourceDecodedPayloadStatus ResourceRegistry::SetDecodedPayloadBudget(ResourceDecodedPayloadBudgetDesc desc) {
    const ResourceDecodedPayloadRequest request{};
    if (desc.decoded_byte_capacity == 0U) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::ConfigureBudget,
            request,
            ResourceDecodedPayloadStatus::InvalidArgument);
    }

    if (desc.payload_reference_capacity == 0U) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::ConfigureBudget,
            request,
            ResourceDecodedPayloadStatus::InvalidArgument);
    }

    if (desc.payload_reference_capacity > MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::ConfigureBudget,
            request,
            ResourceDecodedPayloadStatus::InvalidArgument);
    }

    if (desc.decoded_byte_capacity < decoded_payload_snapshot_.stored_decoded_byte_count) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::ConfigureBudget,
            request,
            ResourceDecodedPayloadStatus::BudgetExceeded);
    }

    if (desc.payload_reference_capacity < decoded_payload_snapshot_.active_payload_count) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::ConfigureBudget,
            request,
            ResourceDecodedPayloadStatus::ReferenceBudgetExceeded);
    }

    decoded_payload_snapshot_.budget_decoded_byte_capacity = desc.decoded_byte_capacity;
    decoded_payload_snapshot_.budget_payload_reference_capacity = desc.payload_reference_capacity;
    RecordDecodedPayloadSuccess(
        ResourceDecodedPayloadOperation::ConfigureBudget,
        request,
        INVALID_RESOURCE_SLOT,
        INVALID_RESOURCE_SLOT,
        0U);
    return ResourceDecodedPayloadStatus::Success;
}

ResourceDecodedPayloadStatus ResourceRegistry::StoreDecodedPayload(const ResourceDecodedPayloadRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceDecodedPayloadStatus validation_status = ValidateDecodedPayloadRequest(request, &slot_index);
    if (validation_status != ResourceDecodedPayloadStatus::Success) {
        return RecordDecodedPayloadRejected(ResourceDecodedPayloadOperation::Store, request, validation_status);
    }

    if (request.decoded_bytes == nullptr) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Store,
            request,
            ResourceDecodedPayloadStatus::InvalidArgument);
    }

    if (request.decoded_byte_count > MAX_RESOURCE_DECODED_PAYLOAD_BYTES_PER_RECORD) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Store,
            request,
            ResourceDecodedPayloadStatus::CapacityExceeded);
    }

    const ResourceDecodedPayloadStatus window_status = ValidateDecodedPayloadStoreWindow(request);
    if (window_status != ResourceDecodedPayloadStatus::Success) {
        return RecordDecodedPayloadRejected(ResourceDecodedPayloadOperation::Store, request, window_status);
    }

    std::size_t cache_payload_record_index = 0U;
    if (!FindCachePayloadRecord(request.resource, request.payload_id, &cache_payload_record_index)) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Store,
            request,
            ResourceDecodedPayloadStatus::MissingCachePayload);
    }

    std::size_t decode_plan_record_index = 0U;
    if (!FindDecodePlanRecord(request.resource, request.payload_id, request.decode_plan_id, &decode_plan_record_index)) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Store,
            request,
            ResourceDecodedPayloadStatus::MissingDecodePlan);
    }

    std::size_t decode_result_record_index = 0U;
    if (!FindDecodeResultRecord(
        request.resource,
        request.payload_id,
        request.decode_plan_id,
        request.decode_result_id,
        &decode_result_record_index)) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Store,
            request,
            ResourceDecodedPayloadStatus::MissingDecodeResult);
    }

    const ResourceDecodeResultRecord &decode_result_record = decode_result_records_[decode_result_record_index];
    const ResourceDecodedPayloadStatus result_status = ValidateDecodedPayloadResult(request, decode_result_record);
    if (result_status != ResourceDecodedPayloadStatus::Success) {
        return RecordDecodedPayloadRejected(ResourceDecodedPayloadOperation::Store, request, result_status);
    }

    if (HasDecodedPayloadId(request.decoded_payload_id)) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Store,
            request,
            ResourceDecodedPayloadStatus::DuplicateDecodedPayloadId);
    }

    if (decoded_payload_snapshot_.active_payload_count >= MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Store,
            request,
            ResourceDecodedPayloadStatus::CapacityExceeded);
    }

    if (decoded_payload_snapshot_.active_payload_count >= decoded_payload_snapshot_.budget_payload_reference_capacity) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Store,
            request,
            ResourceDecodedPayloadStatus::ReferenceBudgetExceeded);
    }

    const std::uint32_t remaining_decoded_byte_capacity =
        decoded_payload_snapshot_.budget_decoded_byte_capacity - decoded_payload_snapshot_.stored_decoded_byte_count;
    if (request.decoded_byte_count > remaining_decoded_byte_capacity) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Store,
            request,
            ResourceDecodedPayloadStatus::BudgetExceeded);
    }

    std::size_t record_index = 0U;
    if (!FindFreeDecodedPayloadRecord(&record_index)) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Store,
            request,
            ResourceDecodedPayloadStatus::CapacityExceeded);
    }

    std::uint32_t byte_index = 0U;
    while (byte_index < request.decoded_byte_count) {
        decoded_payload_bytes_[record_index][byte_index] = request.decoded_bytes[byte_index];
        ++byte_index;
    }

    ResourceDecodedPayloadRecord &record = decoded_payload_records_[record_index];
    record.operation = ResourceDecodedPayloadOperation::Store;
    record.resource = request.resource;
    record.expected_type = request.expected_type;
    record.payload_id = request.payload_id;
    record.decode_plan_id = request.decode_plan_id;
    record.decode_result_id = request.decode_result_id;
    record.decoded_payload_id = request.decoded_payload_id;
    record.payload_logical_byte_count = EffectivePayloadLogicalByteCount(
        request.payload_logical_byte_count,
        request.payload_window_byte_offset,
        request.payload_window_byte_size,
        request.decoded_byte_count);
    record.payload_window_byte_offset = EffectivePayloadWindowByteOffset(
        request.payload_window_byte_offset,
        request.payload_window_byte_size);
    record.payload_window_byte_size = EffectivePayloadWindowByteSize(
        request.payload_window_byte_size,
        request.decoded_byte_count);
    record.asset_class = request.asset_class;
    record.result_class = request.result_class;
    record.decoded_byte_count = request.decoded_byte_count;
    record.decode_result_slot_index = static_cast<std::uint32_t>(decode_result_record_index);
    record.decoded_payload_slot_index = static_cast<std::uint32_t>(record_index);
    record.status = ResourceDecodedPayloadStatus::Success;
    record.is_active = true;
    ++decoded_payload_snapshot_.active_payload_count;
    ++decoded_payload_snapshot_.decoded_payload_record_count;
    decoded_payload_snapshot_.stored_decoded_byte_count += request.decoded_byte_count;
    RecordDecodedPayloadSuccess(
        ResourceDecodedPayloadOperation::Store,
        request,
        record.decode_result_slot_index,
        record.decoded_payload_slot_index,
        request.decoded_byte_count);
    return ResourceDecodedPayloadStatus::Success;
}

ResourceDecodedPayloadStatus ResourceRegistry::QueryDecodedPayload(
    const ResourceDecodedPayloadRequest &request,
    ResourceDecodedPayloadRecord *output_record) {
    if (output_record == nullptr) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Query,
            request,
            ResourceDecodedPayloadStatus::InvalidArgument);
    }

    *output_record = ResourceDecodedPayloadRecord{};
    std::size_t slot_index = 0U;
    const ResourceDecodedPayloadStatus validation_status = ValidateDecodedPayloadRequest(request, &slot_index);
    if (validation_status != ResourceDecodedPayloadStatus::Success) {
        return RecordDecodedPayloadRejected(ResourceDecodedPayloadOperation::Query, request, validation_status);
    }

    std::size_t record_index = 0U;
    if (!FindDecodedPayloadRecord(
        request.resource,
        request.payload_id,
        request.decode_plan_id,
        request.decode_result_id,
        request.decoded_payload_id,
        &record_index)) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Query,
            request,
            ResourceDecodedPayloadStatus::MissingDecodedPayload);
    }

    *output_record = decoded_payload_records_[record_index];
    RecordDecodedPayloadSuccess(
        ResourceDecodedPayloadOperation::Query,
        request,
        output_record->decode_result_slot_index,
        output_record->decoded_payload_slot_index,
        output_record->decoded_byte_count);
    return ResourceDecodedPayloadStatus::Success;
}

ResourceDecodedPayloadStatus ResourceRegistry::ReadDecodedPayload(
    const ResourceDecodedPayloadRequest &request,
    std::uint8_t *output_bytes,
    std::uint32_t output_byte_capacity,
    std::uint32_t *output_byte_count) {
    if (output_byte_count == nullptr) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Read,
            request,
            ResourceDecodedPayloadStatus::InvalidArgument);
    }

    *output_byte_count = 0U;
    if (output_bytes == nullptr) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Read,
            request,
            ResourceDecodedPayloadStatus::InvalidArgument);
    }

    std::size_t slot_index = 0U;
    const ResourceDecodedPayloadStatus validation_status = ValidateDecodedPayloadRequest(request, &slot_index);
    if (validation_status != ResourceDecodedPayloadStatus::Success) {
        return RecordDecodedPayloadRejected(ResourceDecodedPayloadOperation::Read, request, validation_status);
    }

    std::size_t record_index = 0U;
    if (!FindDecodedPayloadRecord(
        request.resource,
        request.payload_id,
        request.decode_plan_id,
        request.decode_result_id,
        request.decoded_payload_id,
        &record_index)) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Read,
            request,
            ResourceDecodedPayloadStatus::MissingDecodedPayload);
    }

    const ResourceDecodedPayloadRecord &record = decoded_payload_records_[record_index];
    std::uint32_t read_byte_offset = 0U;
    std::uint32_t read_byte_count = 0U;
    const ResourceDecodedPayloadStatus window_status = ResolveDecodedPayloadReadWindow(
        request,
        record,
        &read_byte_offset,
        &read_byte_count);
    if (window_status != ResourceDecodedPayloadStatus::Success) {
        return RecordDecodedPayloadRejected(ResourceDecodedPayloadOperation::Read, request, window_status);
    }

    if (output_byte_capacity < read_byte_count) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Read,
            request,
            ResourceDecodedPayloadStatus::OutputBufferTooSmall);
    }

    std::uint32_t byte_index = 0U;
    while (byte_index < read_byte_count) {
        const std::uint32_t source_byte_index = read_byte_offset + byte_index;
        output_bytes[byte_index] = decoded_payload_bytes_[record_index][source_byte_index];
        ++byte_index;
    }

    *output_byte_count = read_byte_count;
    RecordDecodedPayloadSuccess(
        ResourceDecodedPayloadOperation::Read,
        request,
        record.decode_result_slot_index,
        record.decoded_payload_slot_index,
        read_byte_count);
    return ResourceDecodedPayloadStatus::Success;
}

ResourceDecodedPayloadStatus ResourceRegistry::ReleaseDecodedPayload(const ResourceDecodedPayloadRequest &request) {
    std::size_t slot_index = 0U;
    const ResourceDecodedPayloadStatus validation_status = ValidateDecodedPayloadRequest(request, &slot_index);
    if (validation_status != ResourceDecodedPayloadStatus::Success) {
        return RecordDecodedPayloadRejected(ResourceDecodedPayloadOperation::Release, request, validation_status);
    }

    std::size_t record_index = 0U;
    if (!FindDecodedPayloadRecord(
        request.resource,
        request.payload_id,
        request.decode_plan_id,
        request.decode_result_id,
        request.decoded_payload_id,
        &record_index)) {
        return RecordDecodedPayloadRejected(
            ResourceDecodedPayloadOperation::Release,
            request,
            ResourceDecodedPayloadStatus::MissingDecodedPayload);
    }

    const ResourceDecodedPayloadRecord &record = decoded_payload_records_[record_index];
    const std::uint32_t decode_result_slot_index = record.decode_result_slot_index;
    const std::uint32_t decoded_payload_slot_index = record.decoded_payload_slot_index;
    const std::uint32_t decoded_byte_count = record.decoded_byte_count;
    ClearDecodedPayloadRecord(record_index);
    RecordDecodedPayloadSuccess(
        ResourceDecodedPayloadOperation::Release,
        request,
        decode_result_slot_index,
        decoded_payload_slot_index,
        decoded_byte_count);
    return ResourceDecodedPayloadStatus::Success;
}

ResourceDecodedPayloadSnapshot ResourceRegistry::DecodedPayloadSnapshot() const {
    return decoded_payload_snapshot_;
}

ResourceStatus ResourceRegistry::Release(ResourceHandle handle) {
    std::size_t slot_index = 0U;
    const ResourceStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != ResourceStatus::Success) {
        return RecordFailure(handle_status);
    }

    ResourceSlot& slot = slots_[slot_index];
    if (slot.reference_count == 0U) {
        return RecordFailure(ResourceStatus::NotAcquired);
    }

    --slot.reference_count;
    RefreshEvictableState(slot);
    --snapshot_.acquired_handle_count;
    ++snapshot_.released_handle_count;
    RecordSuccess();
    return ResourceStatus::Success;
}

ResourceStatus ResourceRegistry::Retire(ResourceHandle handle) {
    std::size_t slot_index = 0U;
    const ResourceStatus handle_status = ResolveHandle(handle, slot_index);
    if (handle_status != ResourceStatus::Success) {
        return RecordFailure(handle_status);
    }

    ResourceSlot& slot = slots_[slot_index];
    if (slot.reference_count != 0U) {
        return RecordFailure(ResourceStatus::StillReferenced);
    }

    if (HasInboundEdge(slot_index)) {
        return RecordFailure(ResourceStatus::StillDependedOn);
    }

    ClearOutboundEdges(slot_index);
    ClearCachePayloadForSlot(slot_index);
    ClearResidencySlot(slot);
    slot.is_active = false;
    slot.logical_key = ResourceLogicalKey{};
    slot.type = ResourceTypeId{};
    slot.load_state = ResourceLoadState::Unloaded;
    slot.residency_state = ResourceResidencyState::Unloaded;
    slot.reference_count = 0U;
    slot.last_load_commit_id = 0U;
    slot.last_upload_id = 0U;
    slot.last_staging_request_id = 0U;
    slot.loaded_byte_count = 0U;
    AdvanceGeneration(slot);
    --snapshot_.registered_resource_count;
    ++snapshot_.retired_resource_count;
    RecordSuccess();
    return ResourceStatus::Success;
}

ResourceSnapshot ResourceRegistry::Snapshot() const {
    return snapshot_;
}

ResourceStatus ResourceRegistry::RecordFailure(ResourceStatus status) {
    ClearLastRequiredCounts(snapshot_);
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return status;
}

void ResourceRegistry::RecordSuccess() {
    ClearLastRequiredCounts(snapshot_);
    snapshot_.last_status = ResourceStatus::Success;
}

ResourceLoadCommitStatus ResourceRegistry::RecordLoadCommitRejected(ResourceLoadCommitStatus status) {
    ++snapshot_.rejected_load_commit_count;
    ++snapshot_.failed_operation_count;
    if (status == ResourceLoadCommitStatus::DuplicateCommitId) {
        ++snapshot_.duplicate_load_commit_count;
    }

    if (status == ResourceLoadCommitStatus::InvalidTransition) {
        ++snapshot_.invalid_load_transition_count;
    }

    snapshot_.last_load_commit_status = status;
    snapshot_.last_status = MapLoadCommitStatus(status);
    return status;
}

void ResourceRegistry::RecordLoadCommitSuccess(ResourceLoadState load_state) {
    ++snapshot_.load_commit_count;
    if (load_state == ResourceLoadState::Uploaded) {
        ++snapshot_.loaded_resource_count;
    }

    if (load_state == ResourceLoadState::Failed) {
        ++snapshot_.failed_resource_count;
    }

    snapshot_.last_load_state = load_state;
    snapshot_.last_load_commit_status = ResourceLoadCommitStatus::Success;
    snapshot_.last_status = ResourceStatus::Success;
}

ResourceResidencyStatus ResourceRegistry::RecordResidencyRejected(
    ResourceResidencyOperation operation,
    const ResourceResidencyRequest &request,
    ResourceResidencyStatus status) {
    ++residency_snapshot_.rejected_residency_request_count;
    if (status == ResourceResidencyStatus::BudgetExceeded) {
        ++residency_snapshot_.budget_rejected_residency_count;
    }

    ++snapshot_.failed_operation_count;
    residency_snapshot_.last_status = status;
    snapshot_.last_status = MapResidencyStatus(status);
    StoreResidencyRecord(operation, request, status, ResourceResidencyState::Unloaded);
    return status;
}

void ResourceRegistry::RecordResidencySuccess(
    ResourceResidencyOperation operation,
    const ResourceResidencyRequest &request,
    ResourceResidencyState state) {
    switch (operation) {
        case ResourceResidencyOperation::Admit:
            ++residency_snapshot_.admitted_resident_count;
            break;
        case ResourceResidencyOperation::Pin:
            ++residency_snapshot_.pinned_resident_count;
            break;
        case ResourceResidencyOperation::Unpin:
            ++residency_snapshot_.unpinned_resident_count;
            break;
        case ResourceResidencyOperation::Evict:
            ++residency_snapshot_.evicted_resident_count;
            break;
        case ResourceResidencyOperation::SelectCandidate:
            ++residency_snapshot_.eviction_candidate_count;
            break;
        case ResourceResidencyOperation::ConfigureBudget:
        case ResourceResidencyOperation::None:
        default:
            break;
    }

    residency_snapshot_.last_status = ResourceResidencyStatus::Success;
    residency_snapshot_.last_state = state;
    snapshot_.last_status = ResourceStatus::Success;
    StoreResidencyRecord(operation, request, ResourceResidencyStatus::Success, state);
}

ResourceCachePayloadStatus ResourceRegistry::RecordCachePayloadRejected(
    ResourceCachePayloadOperation operation,
    const ResourceCachePayloadRequest &request,
    ResourceCachePayloadStatus status) {
    ++cache_payload_snapshot_.rejected_payload_request_count;
    cache_payload_snapshot_.last_required_payload_byte_count = 0U;
    cache_payload_snapshot_.last_required_payload_reference_count = 0U;
    if (status == ResourceCachePayloadStatus::DuplicatePayloadId) {
        ++cache_payload_snapshot_.duplicate_payload_rejected_count;
    }

    if (status == ResourceCachePayloadStatus::CapacityExceeded) {
        ++cache_payload_snapshot_.capacity_rejected_payload_count;
        if (request.payload_byte_count > MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD) {
            cache_payload_snapshot_.last_required_payload_byte_count = request.payload_byte_count;
        }

        if (request.payload_byte_count <= MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD) {
            cache_payload_snapshot_.last_required_payload_reference_count =
                cache_payload_snapshot_.cached_payload_count + 1U;
        }
    }

    if (status == ResourceCachePayloadStatus::BudgetExceeded) {
        ++cache_payload_snapshot_.budget_rejected_payload_count;
        if (operation == ResourceCachePayloadOperation::Store) {
            cache_payload_snapshot_.last_required_payload_byte_count =
                cache_payload_snapshot_.cached_byte_count + request.payload_byte_count;
        }

        if (operation == ResourceCachePayloadOperation::ConfigureBudget) {
            cache_payload_snapshot_.last_required_payload_byte_count =
                cache_payload_snapshot_.cached_byte_count;
        }
    }

    if (status == ResourceCachePayloadStatus::ReferenceBudgetExceeded) {
        ++cache_payload_snapshot_.reference_budget_rejected_payload_count;
        if (operation == ResourceCachePayloadOperation::Store) {
            cache_payload_snapshot_.last_required_payload_reference_count =
                cache_payload_snapshot_.cached_payload_count + 1U;
        }

        if (operation == ResourceCachePayloadOperation::ConfigureBudget) {
            cache_payload_snapshot_.last_required_payload_reference_count =
                cache_payload_snapshot_.cached_payload_count;
        }
    }

    if (status == ResourceCachePayloadStatus::PayloadWindowOutOfBounds) {
        ++cache_payload_snapshot_.payload_window_rejected_count;
    }

    ++snapshot_.failed_operation_count;
    cache_payload_snapshot_.last_operation = operation;
    cache_payload_snapshot_.last_status = status;
    cache_payload_snapshot_.last_resource = request.resource;
    cache_payload_snapshot_.last_payload_id = request.payload_id;
    cache_payload_snapshot_.last_payload_logical_byte_count = request.payload_logical_byte_count;
    cache_payload_snapshot_.last_payload_window_byte_offset = request.payload_window_byte_offset;
    cache_payload_snapshot_.last_payload_window_byte_size = request.payload_window_byte_size;
    cache_payload_snapshot_.last_cache_slot_index = INVALID_RESOURCE_SLOT;
    cache_payload_snapshot_.last_payload_byte_count = request.payload_byte_count;
    snapshot_.last_status = MapCachePayloadStatus(status);
    return status;
}

void ResourceRegistry::RecordCachePayloadSuccess(
    ResourceCachePayloadOperation operation,
    const ResourceCachePayloadRequest &request,
    std::uint32_t cache_slot_index,
    std::uint32_t payload_byte_count) {
    switch (operation) {
        case ResourceCachePayloadOperation::Store:
            ++cache_payload_snapshot_.stored_payload_count;
            break;
        case ResourceCachePayloadOperation::Read:
            ++cache_payload_snapshot_.read_payload_count;
            break;
        case ResourceCachePayloadOperation::Release:
            ++cache_payload_snapshot_.released_payload_count;
            break;
        case ResourceCachePayloadOperation::ConfigureBudget:
        case ResourceCachePayloadOperation::None:
        default:
            break;
    }

    cache_payload_snapshot_.last_operation = operation;
    cache_payload_snapshot_.last_status = ResourceCachePayloadStatus::Success;
    cache_payload_snapshot_.last_required_payload_byte_count = 0U;
    cache_payload_snapshot_.last_required_payload_reference_count = 0U;
    cache_payload_snapshot_.last_resource = request.resource;
    cache_payload_snapshot_.last_payload_id = request.payload_id;
    cache_payload_snapshot_.last_payload_logical_byte_count = EffectivePayloadLogicalByteCount(
        request.payload_logical_byte_count,
        request.payload_window_byte_offset,
        request.payload_window_byte_size,
        payload_byte_count);
    cache_payload_snapshot_.last_payload_window_byte_offset = EffectivePayloadWindowByteOffset(
        request.payload_window_byte_offset,
        request.payload_window_byte_size);
    cache_payload_snapshot_.last_payload_window_byte_size = EffectivePayloadWindowByteSize(
        request.payload_window_byte_size,
        payload_byte_count);
    cache_payload_snapshot_.last_cache_slot_index = cache_slot_index;
    cache_payload_snapshot_.last_payload_byte_count = payload_byte_count;
    snapshot_.last_status = ResourceStatus::Success;
}

ResourceDecodePlanStatus ResourceRegistry::RecordDecodePlanRejected(
    ResourceDecodePlanOperation operation,
    const ResourceDecodePlanRequest &request,
    ResourceDecodePlanStatus status) {
    ++decode_plan_snapshot_.rejected_plan_request_count;
    decode_plan_snapshot_.last_required_plan_count = 0U;
    decode_plan_snapshot_.last_required_decoded_byte_count = 0U;
    if (status == ResourceDecodePlanStatus::DuplicatePlanId) {
        ++decode_plan_snapshot_.duplicate_plan_rejected_count;
    }

    if (status == ResourceDecodePlanStatus::CapacityExceeded) {
        ++decode_plan_snapshot_.capacity_rejected_plan_count;
        decode_plan_snapshot_.last_required_plan_count =
            decode_plan_snapshot_.active_plan_count + 1U;
    }

    if (status == ResourceDecodePlanStatus::BudgetExceeded) {
        ++decode_plan_snapshot_.budget_rejected_plan_count;
        if (operation == ResourceDecodePlanOperation::Create) {
            decode_plan_snapshot_.last_required_decoded_byte_count =
                decode_plan_snapshot_.planned_decoded_byte_count + request.expected_decoded_byte_count;
        }

        if (operation == ResourceDecodePlanOperation::ConfigureBudget) {
            decode_plan_snapshot_.last_required_decoded_byte_count =
                decode_plan_snapshot_.planned_decoded_byte_count;
        }
    }

    if (status == ResourceDecodePlanStatus::InvalidHeader) {
        ++decode_plan_snapshot_.invalid_header_rejected_count;
    }

    if (status == ResourceDecodePlanStatus::UnsupportedHeaderVersion) {
        ++decode_plan_snapshot_.invalid_header_rejected_count;
    }

    ++snapshot_.failed_operation_count;
    decode_plan_snapshot_.last_operation = operation;
    decode_plan_snapshot_.last_status = status;
    decode_plan_snapshot_.last_resource = request.resource;
    decode_plan_snapshot_.last_payload_id = request.payload_id;
    decode_plan_snapshot_.last_decode_plan_id = request.decode_plan_id;
    decode_plan_snapshot_.last_asset_class = request.asset_class;
    decode_plan_snapshot_.last_cache_slot_index = INVALID_RESOURCE_SLOT;
    decode_plan_snapshot_.last_source_byte_count = request.source_byte_count;
    decode_plan_snapshot_.last_expected_decoded_byte_count = request.expected_decoded_byte_count;
    decode_plan_snapshot_.last_header_version = 0U;
    snapshot_.last_status = MapDecodePlanStatus(status);
    return status;
}

void ResourceRegistry::RecordDecodePlanSuccess(
    ResourceDecodePlanOperation operation,
    const ResourceDecodePlanRequest &request,
    std::uint32_t cache_slot_index,
    std::uint32_t header_version) {
    switch (operation) {
        case ResourceDecodePlanOperation::Create:
            ++decode_plan_snapshot_.created_plan_count;
            break;
        case ResourceDecodePlanOperation::Query:
            ++decode_plan_snapshot_.queried_plan_count;
            break;
        case ResourceDecodePlanOperation::Release:
            ++decode_plan_snapshot_.released_plan_count;
            break;
        case ResourceDecodePlanOperation::ConfigureBudget:
        case ResourceDecodePlanOperation::None:
        default:
            break;
    }

    decode_plan_snapshot_.last_operation = operation;
    decode_plan_snapshot_.last_status = ResourceDecodePlanStatus::Success;
    decode_plan_snapshot_.last_required_plan_count = 0U;
    decode_plan_snapshot_.last_required_decoded_byte_count = 0U;
    decode_plan_snapshot_.last_resource = request.resource;
    decode_plan_snapshot_.last_payload_id = request.payload_id;
    decode_plan_snapshot_.last_decode_plan_id = request.decode_plan_id;
    decode_plan_snapshot_.last_asset_class = request.asset_class;
    decode_plan_snapshot_.last_cache_slot_index = cache_slot_index;
    decode_plan_snapshot_.last_source_byte_count = request.source_byte_count;
    decode_plan_snapshot_.last_expected_decoded_byte_count = request.expected_decoded_byte_count;
    decode_plan_snapshot_.last_header_version = header_version;
    snapshot_.last_status = ResourceStatus::Success;
}

ResourceDecodeResultStatus ResourceRegistry::RecordDecodeResultRejected(
    ResourceDecodeResultOperation operation,
    const ResourceDecodeResultRequest &request,
    ResourceDecodeResultStatus status) {
    ++decode_result_snapshot_.rejected_result_request_count;
    decode_result_snapshot_.last_required_result_count = 0U;
    decode_result_snapshot_.last_required_decoded_byte_count = 0U;
    if (status == ResourceDecodeResultStatus::DuplicateDecodeResultId) {
        ++decode_result_snapshot_.duplicate_result_rejected_count;
    }

    if (status == ResourceDecodeResultStatus::CapacityExceeded) {
        ++decode_result_snapshot_.capacity_rejected_result_count;
        decode_result_snapshot_.last_required_result_count =
            decode_result_snapshot_.active_result_count + 1U;
    }

    if (status == ResourceDecodeResultStatus::BudgetExceeded) {
        ++decode_result_snapshot_.budget_rejected_result_count;
        if (operation == ResourceDecodeResultOperation::Commit) {
            decode_result_snapshot_.last_required_decoded_byte_count =
                decode_result_snapshot_.committed_decoded_byte_count + request.decoded_byte_count;
        }

        if (operation == ResourceDecodeResultOperation::ConfigureBudget) {
            decode_result_snapshot_.last_required_decoded_byte_count =
                decode_result_snapshot_.committed_decoded_byte_count;
        }
    }

    ++snapshot_.failed_operation_count;
    decode_result_snapshot_.last_operation = operation;
    decode_result_snapshot_.last_status = status;
    decode_result_snapshot_.last_resource = request.resource;
    decode_result_snapshot_.last_payload_id = request.payload_id;
    decode_result_snapshot_.last_decode_plan_id = request.decode_plan_id;
    decode_result_snapshot_.last_decode_result_id = request.decode_result_id;
    decode_result_snapshot_.last_asset_class = request.asset_class;
    decode_result_snapshot_.last_result_class = request.result_class;
    decode_result_snapshot_.last_decoded_byte_count = request.decoded_byte_count;
    decode_result_snapshot_.last_decode_plan_slot_index = INVALID_RESOURCE_SLOT;
    snapshot_.last_status = MapDecodeResultStatus(status);
    return status;
}

void ResourceRegistry::RecordDecodeResultSuccess(
    ResourceDecodeResultOperation operation,
    const ResourceDecodeResultRequest &request,
    std::uint32_t decode_plan_slot_index) {
    switch (operation) {
        case ResourceDecodeResultOperation::Commit:
            ++decode_result_snapshot_.committed_result_count;
            break;
        case ResourceDecodeResultOperation::Query:
            ++decode_result_snapshot_.queried_result_count;
            break;
        case ResourceDecodeResultOperation::Release:
            ++decode_result_snapshot_.released_result_count;
            break;
        case ResourceDecodeResultOperation::ConfigureBudget:
        case ResourceDecodeResultOperation::None:
        default:
            break;
    }

    decode_result_snapshot_.last_operation = operation;
    decode_result_snapshot_.last_status = ResourceDecodeResultStatus::Success;
    decode_result_snapshot_.last_required_result_count = 0U;
    decode_result_snapshot_.last_required_decoded_byte_count = 0U;
    decode_result_snapshot_.last_resource = request.resource;
    decode_result_snapshot_.last_payload_id = request.payload_id;
    decode_result_snapshot_.last_decode_plan_id = request.decode_plan_id;
    decode_result_snapshot_.last_decode_result_id = request.decode_result_id;
    decode_result_snapshot_.last_asset_class = request.asset_class;
    decode_result_snapshot_.last_result_class = request.result_class;
    decode_result_snapshot_.last_decoded_byte_count = request.decoded_byte_count;
    decode_result_snapshot_.last_decode_plan_slot_index = decode_plan_slot_index;
    snapshot_.last_status = ResourceStatus::Success;
}

ResourceDecodedPayloadStatus ResourceRegistry::RecordDecodedPayloadRejected(
    ResourceDecodedPayloadOperation operation,
    const ResourceDecodedPayloadRequest &request,
    ResourceDecodedPayloadStatus status) {
    ++decoded_payload_snapshot_.rejected_payload_request_count;
    decoded_payload_snapshot_.last_required_decoded_byte_count = 0U;
    decoded_payload_snapshot_.last_required_payload_reference_count = 0U;
    if (status == ResourceDecodedPayloadStatus::DuplicateDecodedPayloadId) {
        ++decoded_payload_snapshot_.duplicate_payload_rejected_count;
    }

    if (status == ResourceDecodedPayloadStatus::CapacityExceeded) {
        ++decoded_payload_snapshot_.capacity_rejected_payload_count;
        if (request.decoded_byte_count > MAX_RESOURCE_DECODED_PAYLOAD_BYTES_PER_RECORD) {
            decoded_payload_snapshot_.last_required_decoded_byte_count = request.decoded_byte_count;
        }

        if (request.decoded_byte_count <= MAX_RESOURCE_DECODED_PAYLOAD_BYTES_PER_RECORD) {
            decoded_payload_snapshot_.last_required_payload_reference_count =
                decoded_payload_snapshot_.active_payload_count + 1U;
        }
    }

    if (status == ResourceDecodedPayloadStatus::BudgetExceeded) {
        ++decoded_payload_snapshot_.budget_rejected_payload_count;
        if (operation == ResourceDecodedPayloadOperation::Store) {
            decoded_payload_snapshot_.last_required_decoded_byte_count =
                decoded_payload_snapshot_.stored_decoded_byte_count + request.decoded_byte_count;
        }

        if (operation == ResourceDecodedPayloadOperation::ConfigureBudget) {
            decoded_payload_snapshot_.last_required_decoded_byte_count =
                decoded_payload_snapshot_.stored_decoded_byte_count;
        }
    }

    if (status == ResourceDecodedPayloadStatus::ReferenceBudgetExceeded) {
        ++decoded_payload_snapshot_.reference_budget_rejected_payload_count;
        if (operation == ResourceDecodedPayloadOperation::Store) {
            decoded_payload_snapshot_.last_required_payload_reference_count =
                decoded_payload_snapshot_.active_payload_count + 1U;
        }

        if (operation == ResourceDecodedPayloadOperation::ConfigureBudget) {
            decoded_payload_snapshot_.last_required_payload_reference_count =
                decoded_payload_snapshot_.active_payload_count;
        }
    }

    if (status == ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds) {
        ++decoded_payload_snapshot_.payload_window_rejected_count;
    }

    if (status == ResourceDecodedPayloadStatus::OutputBufferTooSmall) {
        ++decoded_payload_snapshot_.output_buffer_too_small_count;
    }

    ++snapshot_.failed_operation_count;
    decoded_payload_snapshot_.last_operation = operation;
    decoded_payload_snapshot_.last_status = status;
    decoded_payload_snapshot_.last_resource = request.resource;
    decoded_payload_snapshot_.last_payload_id = request.payload_id;
    decoded_payload_snapshot_.last_decode_plan_id = request.decode_plan_id;
    decoded_payload_snapshot_.last_decode_result_id = request.decode_result_id;
    decoded_payload_snapshot_.last_decoded_payload_id = request.decoded_payload_id;
    decoded_payload_snapshot_.last_payload_logical_byte_count = request.payload_logical_byte_count;
    decoded_payload_snapshot_.last_payload_window_byte_offset = request.payload_window_byte_offset;
    decoded_payload_snapshot_.last_payload_window_byte_size = request.payload_window_byte_size;
    decoded_payload_snapshot_.last_asset_class = request.asset_class;
    decoded_payload_snapshot_.last_result_class = request.result_class;
    decoded_payload_snapshot_.last_decoded_byte_count = request.decoded_byte_count;
    decoded_payload_snapshot_.last_decode_result_slot_index = INVALID_RESOURCE_SLOT;
    decoded_payload_snapshot_.last_decoded_payload_slot_index = INVALID_RESOURCE_SLOT;
    snapshot_.last_status = MapDecodedPayloadStatus(status);
    return status;
}

void ResourceRegistry::RecordDecodedPayloadSuccess(
    ResourceDecodedPayloadOperation operation,
    const ResourceDecodedPayloadRequest &request,
    std::uint32_t decode_result_slot_index,
    std::uint32_t decoded_payload_slot_index,
    std::uint32_t decoded_byte_count) {
    switch (operation) {
        case ResourceDecodedPayloadOperation::Store:
            ++decoded_payload_snapshot_.stored_payload_count;
            break;
        case ResourceDecodedPayloadOperation::Query:
            ++decoded_payload_snapshot_.queried_payload_count;
            break;
        case ResourceDecodedPayloadOperation::Read:
            ++decoded_payload_snapshot_.read_payload_count;
            break;
        case ResourceDecodedPayloadOperation::Release:
            ++decoded_payload_snapshot_.released_payload_count;
            break;
        case ResourceDecodedPayloadOperation::ConfigureBudget:
        case ResourceDecodedPayloadOperation::None:
        default:
            break;
    }

    decoded_payload_snapshot_.last_operation = operation;
    decoded_payload_snapshot_.last_status = ResourceDecodedPayloadStatus::Success;
    decoded_payload_snapshot_.last_required_decoded_byte_count = 0U;
    decoded_payload_snapshot_.last_required_payload_reference_count = 0U;
    decoded_payload_snapshot_.last_resource = request.resource;
    decoded_payload_snapshot_.last_payload_id = request.payload_id;
    decoded_payload_snapshot_.last_decode_plan_id = request.decode_plan_id;
    decoded_payload_snapshot_.last_decode_result_id = request.decode_result_id;
    decoded_payload_snapshot_.last_decoded_payload_id = request.decoded_payload_id;
    decoded_payload_snapshot_.last_payload_logical_byte_count = EffectivePayloadLogicalByteCount(
        request.payload_logical_byte_count,
        request.payload_window_byte_offset,
        request.payload_window_byte_size,
        decoded_byte_count);
    decoded_payload_snapshot_.last_payload_window_byte_offset = EffectivePayloadWindowByteOffset(
        request.payload_window_byte_offset,
        request.payload_window_byte_size);
    decoded_payload_snapshot_.last_payload_window_byte_size = EffectivePayloadWindowByteSize(
        request.payload_window_byte_size,
        decoded_byte_count);
    decoded_payload_snapshot_.last_asset_class = request.asset_class;
    decoded_payload_snapshot_.last_result_class = request.result_class;
    decoded_payload_snapshot_.last_decoded_byte_count = decoded_byte_count;
    decoded_payload_snapshot_.last_decode_result_slot_index = decode_result_slot_index;
    decoded_payload_snapshot_.last_decoded_payload_slot_index = decoded_payload_slot_index;
    snapshot_.last_status = ResourceStatus::Success;
}

ResourceLoadCommitStatus ResourceRegistry::ValidateLoadCommitRequest(
    const ResourceLoadCommitRequest &request,
    std::size_t *out_slot_index) const {
    if (out_slot_index == nullptr) {
        return ResourceLoadCommitStatus::InvalidArgument;
    }

    *out_slot_index = 0U;
    if (request.commit_id == 0U) {
        return ResourceLoadCommitStatus::InvalidArgument;
    }

    if (request.upload_id == 0U) {
        return ResourceLoadCommitStatus::InvalidArgument;
    }

    if (!request.expected_type.IsValid()) {
        return ResourceLoadCommitStatus::InvalidArgument;
    }

    if (request.load_state == ResourceLoadState::Unloaded) {
        return ResourceLoadCommitStatus::InvalidArgument;
    }

    if (request.upload_byte_count == 0U) {
        return ResourceLoadCommitStatus::InvalidArgument;
    }

    const ResourceStatus handle_status = ResolveHandle(request.resource, *out_slot_index);
    if (handle_status != ResourceStatus::Success) {
        return MapHandleStatus(handle_status);
    }

    const ResourceSlot &slot = slots_[*out_slot_index];
    if (slot.type.value != request.expected_type.value) {
        return ResourceLoadCommitStatus::TypeMismatch;
    }

    if (HasLoadCommitId(request.commit_id)) {
        return ResourceLoadCommitStatus::DuplicateCommitId;
    }

    if (slot.load_state != ResourceLoadState::Unloaded) {
        return ResourceLoadCommitStatus::InvalidTransition;
    }

    if (snapshot_.load_commit_record_count >= MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT) {
        return ResourceLoadCommitStatus::CapacityExceeded;
    }

    return ResourceLoadCommitStatus::Success;
}

ResourceResidencyStatus ResourceRegistry::ValidateResidencyRequest(
    const ResourceResidencyRequest &request,
    std::size_t *out_slot_index) const {
    if (out_slot_index == nullptr) {
        return ResourceResidencyStatus::InvalidArgument;
    }

    *out_slot_index = 0U;
    if (!request.expected_type.IsValid()) {
        return ResourceResidencyStatus::InvalidArgument;
    }

    if (residency_snapshot_.residency_record_count >= MAX_RESOURCE_RESIDENCY_RECORD_COUNT) {
        return ResourceResidencyStatus::CapacityExceeded;
    }

    const ResourceStatus handle_status = ResolveHandle(request.resource, *out_slot_index);
    if (handle_status != ResourceStatus::Success) {
        return MapHandleResidencyStatus(handle_status);
    }

    const ResourceSlot &slot = slots_[*out_slot_index];
    if (slot.type.value != request.expected_type.value) {
        return ResourceResidencyStatus::TypeMismatch;
    }

    return ResourceResidencyStatus::Success;
}

ResourceCachePayloadStatus ResourceRegistry::ValidateCachePayloadRequest(
    const ResourceCachePayloadRequest &request,
    std::size_t *out_slot_index) const {
    if (out_slot_index == nullptr) {
        return ResourceCachePayloadStatus::InvalidArgument;
    }

    *out_slot_index = 0U;
    if (!request.expected_type.IsValid()) {
        return ResourceCachePayloadStatus::InvalidArgument;
    }

    if (request.payload_id == 0U) {
        return ResourceCachePayloadStatus::InvalidArgument;
    }

    const ResourceStatus handle_status = ResolveHandle(request.resource, *out_slot_index);
    if (handle_status != ResourceStatus::Success) {
        return MapHandleCachePayloadStatus(handle_status);
    }

    const ResourceSlot &slot = slots_[*out_slot_index];
    if (slot.type.value != request.expected_type.value) {
        return ResourceCachePayloadStatus::TypeMismatch;
    }

    if (slot.load_state == ResourceLoadState::Failed) {
        return ResourceCachePayloadStatus::FailedLoad;
    }

    if (slot.load_state != ResourceLoadState::Uploaded) {
        return ResourceCachePayloadStatus::NotUploaded;
    }

    if (!IsResidentState(slot.residency_state)) {
        return ResourceCachePayloadStatus::NotResident;
    }

    return ResourceCachePayloadStatus::Success;
}

ResourceCachePayloadStatus ResourceRegistry::ValidateCachePayloadStoreWindow(
    const ResourceCachePayloadRequest &request) const {
    const bool has_payload_window = HasExplicitPayloadWindow(
        request.payload_window_byte_offset,
        request.payload_window_byte_size);
    if (!has_payload_window) {
        const std::uint64_t payload_byte_count = request.payload_byte_count;
        if (request.payload_logical_byte_count == 0U) {
            return ResourceCachePayloadStatus::Success;
        }

        if (request.payload_logical_byte_count != payload_byte_count) {
            return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
        }

        return ResourceCachePayloadStatus::Success;
    }

    if (request.payload_window_byte_size == 0U) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    if (PayloadWindowEndOverflows(request.payload_window_byte_offset, request.payload_window_byte_size)) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    const std::uint64_t payload_byte_count = request.payload_byte_count;
    if (request.payload_window_byte_size != payload_byte_count) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    if (PayloadLogicalByteCountOutOfBounds(
        request.payload_logical_byte_count,
        request.payload_window_byte_offset,
        request.payload_window_byte_size)) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    return ResourceCachePayloadStatus::Success;
}

ResourceCachePayloadStatus ResourceRegistry::ResolveCachePayloadReadWindow(
    const ResourceCachePayloadRequest &request,
    const ResourceCachePayloadRecord &record,
    std::uint32_t *out_byte_offset,
    std::uint32_t *out_byte_count) const {
    if (out_byte_offset == nullptr) {
        return ResourceCachePayloadStatus::InvalidArgument;
    }

    if (out_byte_count == nullptr) {
        return ResourceCachePayloadStatus::InvalidArgument;
    }

    *out_byte_offset = 0U;
    *out_byte_count = 0U;
    if (request.payload_logical_byte_count != 0U &&
        request.payload_logical_byte_count != record.payload_logical_byte_count) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    if (PayloadLogicalByteCountOutOfBounds(
        record.payload_logical_byte_count,
        record.payload_window_byte_offset,
        record.payload_window_byte_size)) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    const bool has_payload_window = HasExplicitPayloadWindow(
        request.payload_window_byte_offset,
        request.payload_window_byte_size);
    if (!has_payload_window) {
        *out_byte_count = record.payload_byte_count;
        return ResourceCachePayloadStatus::Success;
    }

    if (request.payload_window_byte_size == 0U) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    if (PayloadWindowEndOverflows(request.payload_window_byte_offset, request.payload_window_byte_size)) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    if (PayloadWindowEndOverflows(record.payload_window_byte_offset, record.payload_window_byte_size)) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    const std::uint64_t requested_window_end =
        request.payload_window_byte_offset + request.payload_window_byte_size;
    const std::uint64_t record_window_end = record.payload_window_byte_offset + record.payload_window_byte_size;
    if (request.payload_window_byte_offset < record.payload_window_byte_offset) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    if (requested_window_end > record_window_end) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    const std::uint64_t relative_byte_offset =
        request.payload_window_byte_offset - record.payload_window_byte_offset;
    if (relative_byte_offset > record.payload_byte_count) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    const std::uint64_t remaining_byte_count =
        static_cast<std::uint64_t>(record.payload_byte_count) - relative_byte_offset;
    if (request.payload_window_byte_size > remaining_byte_count) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    const std::uint64_t maximum_read_byte_count = std::numeric_limits<std::uint32_t>::max();
    if (request.payload_window_byte_size > maximum_read_byte_count) {
        return ResourceCachePayloadStatus::PayloadWindowOutOfBounds;
    }

    *out_byte_offset = static_cast<std::uint32_t>(relative_byte_offset);
    *out_byte_count = static_cast<std::uint32_t>(request.payload_window_byte_size);
    return ResourceCachePayloadStatus::Success;
}

ResourceDecodePlanStatus ResourceRegistry::ValidateDecodePlanRequest(
    const ResourceDecodePlanRequest &request,
    std::size_t *out_slot_index) const {
    if (out_slot_index == nullptr) {
        return ResourceDecodePlanStatus::InvalidArgument;
    }

    *out_slot_index = 0U;
    if (!request.expected_type.IsValid()) {
        return ResourceDecodePlanStatus::InvalidArgument;
    }

    if (request.payload_id == 0U) {
        return ResourceDecodePlanStatus::InvalidPayloadId;
    }

    if (request.decode_plan_id == 0U) {
        return ResourceDecodePlanStatus::InvalidArgument;
    }

    if (request.asset_class == ResourceDecodePlanAssetClass::Unknown) {
        return ResourceDecodePlanStatus::InvalidArgument;
    }

    if (request.source_byte_count < RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT) {
        return ResourceDecodePlanStatus::InvalidHeader;
    }

    if (request.expected_decoded_byte_count == 0U) {
        return ResourceDecodePlanStatus::InvalidArgument;
    }

    const ResourceStatus handle_status = ResolveHandle(request.resource, *out_slot_index);
    if (handle_status != ResourceStatus::Success) {
        return MapHandleDecodePlanStatus(handle_status);
    }

    const ResourceSlot &slot = slots_[*out_slot_index];
    if (slot.type.value != request.expected_type.value) {
        return ResourceDecodePlanStatus::TypeMismatch;
    }

    if (slot.load_state == ResourceLoadState::Failed) {
        return ResourceDecodePlanStatus::FailedLoad;
    }

    if (slot.load_state != ResourceLoadState::Uploaded) {
        return ResourceDecodePlanStatus::NotUploaded;
    }

    if (!IsResidentState(slot.residency_state)) {
        return ResourceDecodePlanStatus::NotResident;
    }

    return ResourceDecodePlanStatus::Success;
}

ResourceDecodePlanStatus ResourceRegistry::ValidateDecodePlanHeader(
    const ResourceDecodePlanRequest &request,
    const ResourceCachePayloadRecord &cache_payload_record,
    std::uint32_t *out_header_version) const {
    if (out_header_version == nullptr) {
        return ResourceDecodePlanStatus::InvalidArgument;
    }

    *out_header_version = 0U;
    if (cache_payload_record.cache_slot_index >= MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT) {
        return ResourceDecodePlanStatus::InvalidHeader;
    }

    if (cache_payload_record.payload_byte_count != request.source_byte_count) {
        return ResourceDecodePlanStatus::InvalidHeader;
    }

    if (cache_payload_record.payload_byte_count < RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT) {
        return ResourceDecodePlanStatus::InvalidHeader;
    }

    const std::array<std::uint8_t, MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD> &bytes =
        cache_payload_bytes_[cache_payload_record.cache_slot_index];
    if (bytes[0U] != RESOURCE_DECODE_PLAN_HEADER_MAGIC_0) {
        return ResourceDecodePlanStatus::InvalidHeader;
    }

    if (bytes[1U] != RESOURCE_DECODE_PLAN_HEADER_MAGIC_1) {
        return ResourceDecodePlanStatus::InvalidHeader;
    }

    if (bytes[2U] != RESOURCE_DECODE_PLAN_HEADER_MAGIC_2) {
        return ResourceDecodePlanStatus::InvalidHeader;
    }

    if (bytes[3U] != RESOURCE_DECODE_PLAN_HEADER_MAGIC_3) {
        return ResourceDecodePlanStatus::InvalidHeader;
    }

    const std::uint32_t header_version = ReadU32LittleEndian(bytes, 4U);
    if (header_version != RESOURCE_DECODE_PLAN_HEADER_VERSION) {
        return ResourceDecodePlanStatus::UnsupportedHeaderVersion;
    }

    const std::uint32_t asset_class = ReadU32LittleEndian(bytes, 8U);
    const std::uint32_t request_asset_class = static_cast<std::uint32_t>(request.asset_class);
    if (asset_class != request_asset_class) {
        return ResourceDecodePlanStatus::InvalidHeader;
    }

    const std::uint32_t source_byte_count = ReadU32LittleEndian(bytes, 12U);
    if (source_byte_count != request.source_byte_count) {
        return ResourceDecodePlanStatus::InvalidHeader;
    }

    const std::uint32_t expected_decoded_byte_count = ReadU32LittleEndian(bytes, 16U);
    if (expected_decoded_byte_count != request.expected_decoded_byte_count) {
        return ResourceDecodePlanStatus::InvalidHeader;
    }

    *out_header_version = header_version;
    return ResourceDecodePlanStatus::Success;
}

ResourceDecodeResultStatus ResourceRegistry::ValidateDecodeResultRequest(
    const ResourceDecodeResultRequest &request,
    std::size_t *out_slot_index) const {
    if (out_slot_index == nullptr) {
        return ResourceDecodeResultStatus::InvalidArgument;
    }

    *out_slot_index = 0U;
    if (!request.expected_type.IsValid()) {
        return ResourceDecodeResultStatus::InvalidArgument;
    }

    if (request.payload_id == 0U) {
        return ResourceDecodeResultStatus::InvalidPayloadId;
    }

    if (request.decode_plan_id == 0U) {
        return ResourceDecodeResultStatus::InvalidDecodePlanId;
    }

    if (request.decode_result_id == 0U) {
        return ResourceDecodeResultStatus::InvalidDecodeResultId;
    }

    if (request.asset_class == ResourceDecodePlanAssetClass::Unknown) {
        return ResourceDecodeResultStatus::InvalidArgument;
    }

    if (request.result_class == ResourceDecodeResultClass::Unknown) {
        return ResourceDecodeResultStatus::InvalidArgument;
    }

    if (request.decoded_byte_count == 0U) {
        return ResourceDecodeResultStatus::InvalidArgument;
    }

    const ResourceStatus handle_status = ResolveHandle(request.resource, *out_slot_index);
    if (handle_status != ResourceStatus::Success) {
        return MapHandleDecodeResultStatus(handle_status);
    }

    const ResourceSlot &slot = slots_[*out_slot_index];
    if (slot.type.value != request.expected_type.value) {
        return ResourceDecodeResultStatus::TypeMismatch;
    }

    if (slot.load_state == ResourceLoadState::Failed) {
        return ResourceDecodeResultStatus::FailedLoad;
    }

    if (slot.load_state != ResourceLoadState::Uploaded) {
        return ResourceDecodeResultStatus::NotUploaded;
    }

    if (!IsResidentState(slot.residency_state)) {
        return ResourceDecodeResultStatus::NotResident;
    }

    return ResourceDecodeResultStatus::Success;
}

ResourceDecodeResultStatus ResourceRegistry::ValidateDecodeResultPlan(
    const ResourceDecodeResultRequest &request,
    const ResourceDecodePlanRecord &decode_plan_record) const {
    if (!decode_plan_record.is_active) {
        return ResourceDecodeResultStatus::MissingDecodePlan;
    }

    if (decode_plan_record.asset_class != request.asset_class) {
        return ResourceDecodeResultStatus::AssetClassMismatch;
    }

    if (!MatchesDecodeResultClass(request.asset_class, request.result_class)) {
        return ResourceDecodeResultStatus::ResultClassMismatch;
    }

    if (decode_plan_record.expected_decoded_byte_count != request.decoded_byte_count) {
        return ResourceDecodeResultStatus::DecodedByteCountMismatch;
    }

    return ResourceDecodeResultStatus::Success;
}

ResourceDecodedPayloadStatus ResourceRegistry::ValidateDecodedPayloadRequest(
    const ResourceDecodedPayloadRequest &request,
    std::size_t *out_slot_index) const {
    if (out_slot_index == nullptr) {
        return ResourceDecodedPayloadStatus::InvalidArgument;
    }

    *out_slot_index = 0U;
    if (!request.expected_type.IsValid()) {
        return ResourceDecodedPayloadStatus::InvalidArgument;
    }

    if (request.payload_id == 0U) {
        return ResourceDecodedPayloadStatus::InvalidPayloadId;
    }

    if (request.decode_plan_id == 0U) {
        return ResourceDecodedPayloadStatus::InvalidDecodePlanId;
    }

    if (request.decode_result_id == 0U) {
        return ResourceDecodedPayloadStatus::InvalidDecodeResultId;
    }

    if (request.decoded_payload_id == 0U) {
        return ResourceDecodedPayloadStatus::InvalidDecodedPayloadId;
    }

    if (request.asset_class == ResourceDecodePlanAssetClass::Unknown) {
        return ResourceDecodedPayloadStatus::InvalidArgument;
    }

    if (request.result_class == ResourceDecodeResultClass::Unknown) {
        return ResourceDecodedPayloadStatus::InvalidArgument;
    }

    if (request.decoded_byte_count == 0U) {
        return ResourceDecodedPayloadStatus::EmptyPayload;
    }

    const ResourceStatus handle_status = ResolveHandle(request.resource, *out_slot_index);
    if (handle_status != ResourceStatus::Success) {
        return MapHandleDecodedPayloadStatus(handle_status);
    }

    const ResourceSlot &slot = slots_[*out_slot_index];
    if (slot.type.value != request.expected_type.value) {
        return ResourceDecodedPayloadStatus::TypeMismatch;
    }

    if (slot.load_state == ResourceLoadState::Failed) {
        return ResourceDecodedPayloadStatus::FailedLoad;
    }

    if (slot.load_state != ResourceLoadState::Uploaded) {
        return ResourceDecodedPayloadStatus::NotUploaded;
    }

    if (!IsResidentState(slot.residency_state)) {
        return ResourceDecodedPayloadStatus::NotResident;
    }

    return ResourceDecodedPayloadStatus::Success;
}

ResourceDecodedPayloadStatus ResourceRegistry::ValidateDecodedPayloadStoreWindow(
    const ResourceDecodedPayloadRequest &request) const {
    const bool has_payload_window = HasExplicitPayloadWindow(
        request.payload_window_byte_offset,
        request.payload_window_byte_size);
    if (!has_payload_window) {
        const std::uint64_t decoded_byte_count = request.decoded_byte_count;
        if (request.payload_logical_byte_count == 0U) {
            return ResourceDecodedPayloadStatus::Success;
        }

        if (request.payload_logical_byte_count != decoded_byte_count) {
            return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
        }

        return ResourceDecodedPayloadStatus::Success;
    }

    if (request.payload_window_byte_size == 0U) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    if (PayloadWindowEndOverflows(request.payload_window_byte_offset, request.payload_window_byte_size)) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    const std::uint64_t decoded_byte_count = request.decoded_byte_count;
    if (request.payload_window_byte_size != decoded_byte_count) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    if (PayloadLogicalByteCountOutOfBounds(
        request.payload_logical_byte_count,
        request.payload_window_byte_offset,
        request.payload_window_byte_size)) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    return ResourceDecodedPayloadStatus::Success;
}

ResourceDecodedPayloadStatus ResourceRegistry::ResolveDecodedPayloadReadWindow(
    const ResourceDecodedPayloadRequest &request,
    const ResourceDecodedPayloadRecord &record,
    std::uint32_t *out_byte_offset,
    std::uint32_t *out_byte_count) const {
    if (out_byte_offset == nullptr) {
        return ResourceDecodedPayloadStatus::InvalidArgument;
    }

    if (out_byte_count == nullptr) {
        return ResourceDecodedPayloadStatus::InvalidArgument;
    }

    *out_byte_offset = 0U;
    *out_byte_count = 0U;
    if (request.payload_logical_byte_count != 0U &&
        request.payload_logical_byte_count != record.payload_logical_byte_count) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    if (PayloadLogicalByteCountOutOfBounds(
        record.payload_logical_byte_count,
        record.payload_window_byte_offset,
        record.payload_window_byte_size)) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    const bool has_payload_window = HasExplicitPayloadWindow(
        request.payload_window_byte_offset,
        request.payload_window_byte_size);
    if (!has_payload_window) {
        *out_byte_count = record.decoded_byte_count;
        return ResourceDecodedPayloadStatus::Success;
    }

    if (request.payload_window_byte_size == 0U) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    if (PayloadWindowEndOverflows(request.payload_window_byte_offset, request.payload_window_byte_size)) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    if (PayloadWindowEndOverflows(record.payload_window_byte_offset, record.payload_window_byte_size)) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    const std::uint64_t requested_window_end =
        request.payload_window_byte_offset + request.payload_window_byte_size;
    const std::uint64_t record_window_end = record.payload_window_byte_offset + record.payload_window_byte_size;
    if (request.payload_window_byte_offset < record.payload_window_byte_offset) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    if (requested_window_end > record_window_end) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    const std::uint64_t relative_byte_offset =
        request.payload_window_byte_offset - record.payload_window_byte_offset;
    if (relative_byte_offset > record.decoded_byte_count) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    const std::uint64_t remaining_byte_count =
        static_cast<std::uint64_t>(record.decoded_byte_count) - relative_byte_offset;
    if (request.payload_window_byte_size > remaining_byte_count) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    const std::uint64_t maximum_read_byte_count = std::numeric_limits<std::uint32_t>::max();
    if (request.payload_window_byte_size > maximum_read_byte_count) {
        return ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds;
    }

    *out_byte_offset = static_cast<std::uint32_t>(relative_byte_offset);
    *out_byte_count = static_cast<std::uint32_t>(request.payload_window_byte_size);
    return ResourceDecodedPayloadStatus::Success;
}

ResourceDecodedPayloadStatus ResourceRegistry::ValidateDecodedPayloadResult(
    const ResourceDecodedPayloadRequest &request,
    const ResourceDecodeResultRecord &decode_result_record) const {
    if (!decode_result_record.is_active) {
        return ResourceDecodedPayloadStatus::MissingDecodeResult;
    }

    if (decode_result_record.asset_class != request.asset_class) {
        return ResourceDecodedPayloadStatus::AssetClassMismatch;
    }

    if (decode_result_record.result_class != request.result_class) {
        return ResourceDecodedPayloadStatus::ResultClassMismatch;
    }

    if (decode_result_record.decoded_byte_count != request.decoded_byte_count) {
        return ResourceDecodedPayloadStatus::DecodedByteCountMismatch;
    }

    return ResourceDecodedPayloadStatus::Success;
}

ResourceLoadCommitStatus ResourceRegistry::MapHandleStatus(ResourceStatus status) const {
    if (status == ResourceStatus::InvalidHandle) {
        return ResourceLoadCommitStatus::InvalidHandle;
    }

    if (status == ResourceStatus::GenerationMismatch) {
        return ResourceLoadCommitStatus::GenerationMismatch;
    }

    if (status == ResourceStatus::TypeMismatch) {
        return ResourceLoadCommitStatus::TypeMismatch;
    }

    return ResourceLoadCommitStatus::InvalidHandle;
}

ResourceStatus ResourceRegistry::MapLoadCommitStatus(ResourceLoadCommitStatus status) const {
    switch (status) {
        case ResourceLoadCommitStatus::Success:
            return ResourceStatus::Success;
        case ResourceLoadCommitStatus::InvalidHandle:
            return ResourceStatus::InvalidHandle;
        case ResourceLoadCommitStatus::GenerationMismatch:
            return ResourceStatus::GenerationMismatch;
        case ResourceLoadCommitStatus::TypeMismatch:
            return ResourceStatus::TypeMismatch;
        case ResourceLoadCommitStatus::CapacityExceeded:
            return ResourceStatus::CapacityExceeded;
        case ResourceLoadCommitStatus::InvalidArgument:
        case ResourceLoadCommitStatus::DuplicateCommitId:
        case ResourceLoadCommitStatus::InvalidTransition:
        default:
            break;
    }

    return ResourceStatus::UnsupportedInThisGate;
}

ResourceResidencyStatus ResourceRegistry::MapHandleResidencyStatus(ResourceStatus status) const {
    if (status == ResourceStatus::InvalidHandle) {
        return ResourceResidencyStatus::InvalidHandle;
    }

    if (status == ResourceStatus::GenerationMismatch) {
        return ResourceResidencyStatus::GenerationMismatch;
    }

    if (status == ResourceStatus::TypeMismatch) {
        return ResourceResidencyStatus::TypeMismatch;
    }

    return ResourceResidencyStatus::InvalidHandle;
}

ResourceStatus ResourceRegistry::MapResidencyStatus(ResourceResidencyStatus status) const {
    switch (status) {
        case ResourceResidencyStatus::Success:
            return ResourceStatus::Success;
        case ResourceResidencyStatus::InvalidHandle:
            return ResourceStatus::InvalidHandle;
        case ResourceResidencyStatus::GenerationMismatch:
            return ResourceStatus::GenerationMismatch;
        case ResourceResidencyStatus::TypeMismatch:
            return ResourceStatus::TypeMismatch;
        case ResourceResidencyStatus::BudgetExceeded:
        case ResourceResidencyStatus::CapacityExceeded:
            return ResourceStatus::CapacityExceeded;
        case ResourceResidencyStatus::StillReferenced:
            return ResourceStatus::StillReferenced;
        case ResourceResidencyStatus::InvalidArgument:
        case ResourceResidencyStatus::NotUploaded:
        case ResourceResidencyStatus::FailedLoad:
        case ResourceResidencyStatus::AlreadyResident:
        case ResourceResidencyStatus::NotResident:
        case ResourceResidencyStatus::AlreadyPinned:
        case ResourceResidencyStatus::NotPinned:
        case ResourceResidencyStatus::Pinned:
        case ResourceResidencyStatus::NoCandidate:
        default:
            break;
    }

    return ResourceStatus::UnsupportedInThisGate;
}

ResourceCachePayloadStatus ResourceRegistry::MapHandleCachePayloadStatus(ResourceStatus status) const {
    if (status == ResourceStatus::InvalidHandle) {
        return ResourceCachePayloadStatus::InvalidHandle;
    }

    if (status == ResourceStatus::GenerationMismatch) {
        return ResourceCachePayloadStatus::GenerationMismatch;
    }

    if (status == ResourceStatus::TypeMismatch) {
        return ResourceCachePayloadStatus::TypeMismatch;
    }

    return ResourceCachePayloadStatus::InvalidHandle;
}

ResourceStatus ResourceRegistry::MapCachePayloadStatus(ResourceCachePayloadStatus status) const {
    switch (status) {
        case ResourceCachePayloadStatus::Success:
            return ResourceStatus::Success;
        case ResourceCachePayloadStatus::InvalidHandle:
            return ResourceStatus::InvalidHandle;
        case ResourceCachePayloadStatus::GenerationMismatch:
            return ResourceStatus::GenerationMismatch;
        case ResourceCachePayloadStatus::TypeMismatch:
            return ResourceStatus::TypeMismatch;
        case ResourceCachePayloadStatus::DuplicatePayloadId:
            return ResourceStatus::DuplicateResource;
        case ResourceCachePayloadStatus::MissingPayload:
            return ResourceStatus::NotFound;
        case ResourceCachePayloadStatus::CapacityExceeded:
        case ResourceCachePayloadStatus::BudgetExceeded:
        case ResourceCachePayloadStatus::ReferenceBudgetExceeded:
        case ResourceCachePayloadStatus::OutputBufferTooSmall:
        case ResourceCachePayloadStatus::PayloadWindowOutOfBounds:
            return ResourceStatus::CapacityExceeded;
        case ResourceCachePayloadStatus::Pinned:
            return ResourceStatus::StillReferenced;
        case ResourceCachePayloadStatus::InvalidArgument:
        case ResourceCachePayloadStatus::NotUploaded:
        case ResourceCachePayloadStatus::FailedLoad:
        case ResourceCachePayloadStatus::NotResident:
        case ResourceCachePayloadStatus::EmptyPayload:
        default:
            break;
    }

    return ResourceStatus::UnsupportedInThisGate;
}

ResourceDecodePlanStatus ResourceRegistry::MapHandleDecodePlanStatus(ResourceStatus status) const {
    if (status == ResourceStatus::InvalidHandle) {
        return ResourceDecodePlanStatus::InvalidHandle;
    }

    if (status == ResourceStatus::GenerationMismatch) {
        return ResourceDecodePlanStatus::GenerationMismatch;
    }

    if (status == ResourceStatus::TypeMismatch) {
        return ResourceDecodePlanStatus::TypeMismatch;
    }

    return ResourceDecodePlanStatus::InvalidHandle;
}

ResourceStatus ResourceRegistry::MapDecodePlanStatus(ResourceDecodePlanStatus status) const {
    switch (status) {
        case ResourceDecodePlanStatus::Success:
            return ResourceStatus::Success;
        case ResourceDecodePlanStatus::InvalidHandle:
            return ResourceStatus::InvalidHandle;
        case ResourceDecodePlanStatus::GenerationMismatch:
            return ResourceStatus::GenerationMismatch;
        case ResourceDecodePlanStatus::TypeMismatch:
            return ResourceStatus::TypeMismatch;
        case ResourceDecodePlanStatus::MissingCachePayload:
        case ResourceDecodePlanStatus::MissingPlan:
            return ResourceStatus::NotFound;
        case ResourceDecodePlanStatus::DuplicatePlanId:
            return ResourceStatus::DuplicateResource;
        case ResourceDecodePlanStatus::CapacityExceeded:
        case ResourceDecodePlanStatus::BudgetExceeded:
            return ResourceStatus::CapacityExceeded;
        case ResourceDecodePlanStatus::InvalidArgument:
        case ResourceDecodePlanStatus::NotUploaded:
        case ResourceDecodePlanStatus::FailedLoad:
        case ResourceDecodePlanStatus::NotResident:
        case ResourceDecodePlanStatus::InvalidPayloadId:
        case ResourceDecodePlanStatus::InvalidHeader:
        case ResourceDecodePlanStatus::UnsupportedHeaderVersion:
        default:
            break;
    }

    return ResourceStatus::UnsupportedInThisGate;
}

ResourceDecodeResultStatus ResourceRegistry::MapHandleDecodeResultStatus(ResourceStatus status) const {
    if (status == ResourceStatus::InvalidHandle) {
        return ResourceDecodeResultStatus::InvalidHandle;
    }

    if (status == ResourceStatus::GenerationMismatch) {
        return ResourceDecodeResultStatus::GenerationMismatch;
    }

    if (status == ResourceStatus::TypeMismatch) {
        return ResourceDecodeResultStatus::TypeMismatch;
    }

    return ResourceDecodeResultStatus::InvalidHandle;
}

ResourceStatus ResourceRegistry::MapDecodeResultStatus(ResourceDecodeResultStatus status) const {
    switch (status) {
        case ResourceDecodeResultStatus::Success:
            return ResourceStatus::Success;
        case ResourceDecodeResultStatus::InvalidHandle:
            return ResourceStatus::InvalidHandle;
        case ResourceDecodeResultStatus::GenerationMismatch:
            return ResourceStatus::GenerationMismatch;
        case ResourceDecodeResultStatus::TypeMismatch:
            return ResourceStatus::TypeMismatch;
        case ResourceDecodeResultStatus::MissingCachePayload:
        case ResourceDecodeResultStatus::MissingDecodePlan:
        case ResourceDecodeResultStatus::MissingDecodeResult:
            return ResourceStatus::NotFound;
        case ResourceDecodeResultStatus::DuplicateDecodeResultId:
            return ResourceStatus::DuplicateResource;
        case ResourceDecodeResultStatus::CapacityExceeded:
        case ResourceDecodeResultStatus::BudgetExceeded:
            return ResourceStatus::CapacityExceeded;
        case ResourceDecodeResultStatus::InvalidArgument:
        case ResourceDecodeResultStatus::NotUploaded:
        case ResourceDecodeResultStatus::FailedLoad:
        case ResourceDecodeResultStatus::NotResident:
        case ResourceDecodeResultStatus::InvalidPayloadId:
        case ResourceDecodeResultStatus::InvalidDecodePlanId:
        case ResourceDecodeResultStatus::InvalidDecodeResultId:
        case ResourceDecodeResultStatus::AssetClassMismatch:
        case ResourceDecodeResultStatus::ResultClassMismatch:
        case ResourceDecodeResultStatus::DecodedByteCountMismatch:
        default:
            break;
    }

    return ResourceStatus::UnsupportedInThisGate;
}

ResourceDecodedPayloadStatus ResourceRegistry::MapHandleDecodedPayloadStatus(ResourceStatus status) const {
    if (status == ResourceStatus::InvalidHandle) {
        return ResourceDecodedPayloadStatus::InvalidHandle;
    }

    if (status == ResourceStatus::GenerationMismatch) {
        return ResourceDecodedPayloadStatus::GenerationMismatch;
    }

    if (status == ResourceStatus::TypeMismatch) {
        return ResourceDecodedPayloadStatus::TypeMismatch;
    }

    return ResourceDecodedPayloadStatus::InvalidHandle;
}

ResourceStatus ResourceRegistry::MapDecodedPayloadStatus(ResourceDecodedPayloadStatus status) const {
    switch (status) {
        case ResourceDecodedPayloadStatus::Success:
            return ResourceStatus::Success;
        case ResourceDecodedPayloadStatus::InvalidHandle:
            return ResourceStatus::InvalidHandle;
        case ResourceDecodedPayloadStatus::GenerationMismatch:
            return ResourceStatus::GenerationMismatch;
        case ResourceDecodedPayloadStatus::TypeMismatch:
            return ResourceStatus::TypeMismatch;
        case ResourceDecodedPayloadStatus::MissingCachePayload:
        case ResourceDecodedPayloadStatus::MissingDecodePlan:
        case ResourceDecodedPayloadStatus::MissingDecodeResult:
        case ResourceDecodedPayloadStatus::MissingDecodedPayload:
            return ResourceStatus::NotFound;
        case ResourceDecodedPayloadStatus::DuplicateDecodedPayloadId:
            return ResourceStatus::DuplicateResource;
        case ResourceDecodedPayloadStatus::CapacityExceeded:
        case ResourceDecodedPayloadStatus::BudgetExceeded:
        case ResourceDecodedPayloadStatus::ReferenceBudgetExceeded:
        case ResourceDecodedPayloadStatus::OutputBufferTooSmall:
        case ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds:
            return ResourceStatus::CapacityExceeded;
        case ResourceDecodedPayloadStatus::InvalidArgument:
        case ResourceDecodedPayloadStatus::NotUploaded:
        case ResourceDecodedPayloadStatus::FailedLoad:
        case ResourceDecodedPayloadStatus::NotResident:
        case ResourceDecodedPayloadStatus::InvalidPayloadId:
        case ResourceDecodedPayloadStatus::InvalidDecodePlanId:
        case ResourceDecodedPayloadStatus::InvalidDecodeResultId:
        case ResourceDecodedPayloadStatus::InvalidDecodedPayloadId:
        case ResourceDecodedPayloadStatus::AssetClassMismatch:
        case ResourceDecodedPayloadStatus::ResultClassMismatch:
        case ResourceDecodedPayloadStatus::DecodedByteCountMismatch:
        case ResourceDecodedPayloadStatus::EmptyPayload:
        default:
            break;
    }

    return ResourceStatus::UnsupportedInThisGate;
}

ResourceStatus ResourceRegistry::ResolveHandle(ResourceHandle handle, std::size_t& out_index) const {
    if (!handle.IsValid()) {
        return ResourceStatus::InvalidHandle;
    }

    if (handle.slot >= snapshot_.resource_capacity) {
        return ResourceStatus::InvalidHandle;
    }

    const ResourceSlot& slot = slots_[handle.slot];
    if (slot.generation != handle.generation) {
        return ResourceStatus::GenerationMismatch;
    }

    if (!slot.is_active) {
        return ResourceStatus::InvalidHandle;
    }

    out_index = handle.slot;
    return ResourceStatus::Success;
}

ResourceStatus ResourceRegistry::RegisterTypeIfNeeded(ResourceTypeId type) {
    if (HasType(type)) {
        return ResourceStatus::Success;
    }

    if (snapshot_.type_count >= snapshot_.type_capacity) {
        return ResourceStatus::CapacityExceeded;
    }

    types_[snapshot_.type_count] = type;
    ++snapshot_.type_count;
    return ResourceStatus::Success;
}

bool ResourceRegistry::HasType(ResourceTypeId type) const {
    std::uint32_t index = 0U;
    for (const ResourceTypeId& registered_type : types_) {
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

bool ResourceRegistry::HasDuplicateActiveResource(const ResourceDescriptor& descriptor) const {
    std::uint32_t index = 0U;
    for (const ResourceSlot& slot : slots_) {
        if (index >= snapshot_.resource_capacity) {
            return false;
        }

        if (!slot.is_active) {
            ++index;
            continue;
        }

        if (slot.type.value != descriptor.type.value) {
            ++index;
            continue;
        }

        if (slot.logical_key.Equals(descriptor.logical_key)) {
            return true;
        }

        ++index;
    }

    return false;
}

bool ResourceRegistry::HasLoadCommitId(std::uint64_t commit_id) const {
    for (const ResourceLoadCommitRecord &record : load_commit_records_) {
        if (!record.is_active) {
            continue;
        }

        if (record.request.commit_id == commit_id) {
            return true;
        }
    }

    return false;
}

bool ResourceRegistry::HasCachePayloadId(std::uint64_t payload_id) const {
    for (const ResourceCachePayloadRecord &record : cache_payload_records_) {
        if (!record.is_active) {
            continue;
        }

        if (record.payload_id == payload_id) {
            return true;
        }
    }

    return false;
}

bool ResourceRegistry::HasDecodePlanId(std::uint64_t decode_plan_id) const {
    for (const ResourceDecodePlanRecord &record : decode_plan_records_) {
        if (!record.is_active) {
            continue;
        }

        if (record.decode_plan_id == decode_plan_id) {
            return true;
        }
    }

    return false;
}

bool ResourceRegistry::HasDecodeResultId(std::uint64_t decode_result_id) const {
    for (const ResourceDecodeResultRecord &record : decode_result_records_) {
        if (!record.is_active) {
            continue;
        }

        if (record.decode_result_id == decode_result_id) {
            return true;
        }
    }

    return false;
}

bool ResourceRegistry::HasDecodedPayloadId(std::uint64_t decoded_payload_id) const {
    for (const ResourceDecodedPayloadRecord &record : decoded_payload_records_) {
        if (!record.is_active) {
            continue;
        }

        if (record.decoded_payload_id == decoded_payload_id) {
            return true;
        }
    }

    return false;
}

bool ResourceRegistry::FindCachePayloadRecord(
    ResourceHandle resource,
    std::uint64_t payload_id,
    std::size_t *out_record_index) const {
    if (out_record_index == nullptr) {
        return false;
    }

    *out_record_index = 0U;
    std::size_t record_index = 0U;
    for (const ResourceCachePayloadRecord &record : cache_payload_records_) {
        if (!record.is_active) {
            ++record_index;
            continue;
        }

        if (record.payload_id != payload_id) {
            ++record_index;
            continue;
        }

        if (record.resource.slot != resource.slot) {
            ++record_index;
            continue;
        }

        if (record.resource.generation != resource.generation) {
            ++record_index;
            continue;
        }

        *out_record_index = record_index;
        return true;
    }

    return false;
}

bool ResourceRegistry::FindFreeCachePayloadRecord(std::size_t *out_record_index) const {
    if (out_record_index == nullptr) {
        return false;
    }

    *out_record_index = 0U;
    std::size_t record_index = 0U;
    for (const ResourceCachePayloadRecord &record : cache_payload_records_) {
        if (record.is_active) {
            ++record_index;
            continue;
        }

        *out_record_index = record_index;
        return true;
    }

    return false;
}

bool ResourceRegistry::FindDecodePlanRecord(
    ResourceHandle resource,
    std::uint64_t payload_id,
    std::uint64_t decode_plan_id,
    std::size_t *out_record_index) const {
    if (out_record_index == nullptr) {
        return false;
    }

    *out_record_index = 0U;
    std::size_t record_index = 0U;
    for (const ResourceDecodePlanRecord &record : decode_plan_records_) {
        if (!record.is_active) {
            ++record_index;
            continue;
        }

        if (record.decode_plan_id != decode_plan_id) {
            ++record_index;
            continue;
        }

        if (record.payload_id != payload_id) {
            ++record_index;
            continue;
        }

        if (record.resource.slot != resource.slot) {
            ++record_index;
            continue;
        }

        if (record.resource.generation != resource.generation) {
            ++record_index;
            continue;
        }

        *out_record_index = record_index;
        return true;
    }

    return false;
}

bool ResourceRegistry::FindFreeDecodePlanRecord(std::size_t *out_record_index) const {
    if (out_record_index == nullptr) {
        return false;
    }

    *out_record_index = 0U;
    std::size_t record_index = 0U;
    for (const ResourceDecodePlanRecord &record : decode_plan_records_) {
        if (record.is_active) {
            ++record_index;
            continue;
        }

        *out_record_index = record_index;
        return true;
    }

    return false;
}

bool ResourceRegistry::FindDecodeResultRecord(
    ResourceHandle resource,
    std::uint64_t payload_id,
    std::uint64_t decode_plan_id,
    std::uint64_t decode_result_id,
    std::size_t *out_record_index) const {
    if (out_record_index == nullptr) {
        return false;
    }

    *out_record_index = 0U;
    std::size_t record_index = 0U;
    for (const ResourceDecodeResultRecord &record : decode_result_records_) {
        if (!record.is_active) {
            ++record_index;
            continue;
        }

        if (record.decode_result_id != decode_result_id) {
            ++record_index;
            continue;
        }

        if (record.decode_plan_id != decode_plan_id) {
            ++record_index;
            continue;
        }

        if (record.payload_id != payload_id) {
            ++record_index;
            continue;
        }

        if (record.resource.slot != resource.slot) {
            ++record_index;
            continue;
        }

        if (record.resource.generation != resource.generation) {
            ++record_index;
            continue;
        }

        *out_record_index = record_index;
        return true;
    }

    return false;
}

bool ResourceRegistry::FindFreeDecodeResultRecord(std::size_t *out_record_index) const {
    if (out_record_index == nullptr) {
        return false;
    }

    *out_record_index = 0U;
    std::size_t record_index = 0U;
    for (const ResourceDecodeResultRecord &record : decode_result_records_) {
        if (record.is_active) {
            ++record_index;
            continue;
        }

        *out_record_index = record_index;
        return true;
    }

    return false;
}

bool ResourceRegistry::FindDecodedPayloadRecord(
    ResourceHandle resource,
    std::uint64_t payload_id,
    std::uint64_t decode_plan_id,
    std::uint64_t decode_result_id,
    std::uint64_t decoded_payload_id,
    std::size_t *out_record_index) const {
    if (out_record_index == nullptr) {
        return false;
    }

    *out_record_index = 0U;
    std::size_t record_index = 0U;
    for (const ResourceDecodedPayloadRecord &record : decoded_payload_records_) {
        if (!record.is_active) {
            ++record_index;
            continue;
        }

        if (record.decoded_payload_id != decoded_payload_id) {
            ++record_index;
            continue;
        }

        if (record.decode_result_id != decode_result_id) {
            ++record_index;
            continue;
        }

        if (record.decode_plan_id != decode_plan_id) {
            ++record_index;
            continue;
        }

        if (record.payload_id != payload_id) {
            ++record_index;
            continue;
        }

        if (record.resource.slot != resource.slot) {
            ++record_index;
            continue;
        }

        if (record.resource.generation != resource.generation) {
            ++record_index;
            continue;
        }

        *out_record_index = record_index;
        return true;
    }

    return false;
}

bool ResourceRegistry::FindFreeDecodedPayloadRecord(std::size_t *out_record_index) const {
    if (out_record_index == nullptr) {
        return false;
    }

    *out_record_index = 0U;
    std::size_t record_index = 0U;
    for (const ResourceDecodedPayloadRecord &record : decoded_payload_records_) {
        if (record.is_active) {
            ++record_index;
            continue;
        }

        *out_record_index = record_index;
        return true;
    }

    return false;
}

bool ResourceRegistry::StoreLoadCommitRecord(const ResourceLoadCommitRequest &request) {
    if (snapshot_.load_commit_record_count >= MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT) {
        return false;
    }

    for (ResourceLoadCommitRecord &record : load_commit_records_) {
        if (record.is_active) {
            continue;
        }

        record.request = request;
        record.is_active = true;
        ++snapshot_.load_commit_record_count;
        return true;
    }

    return false;
}

bool ResourceRegistry::StoreResidencyRecord(
    ResourceResidencyOperation operation,
    const ResourceResidencyRequest &request,
    ResourceResidencyStatus status,
    ResourceResidencyState state) {
    if (residency_snapshot_.residency_record_count >= MAX_RESOURCE_RESIDENCY_RECORD_COUNT) {
        return false;
    }

    for (ResourceResidencyRecord &record : residency_records_) {
        if (record.is_active) {
            continue;
        }

        record.operation = operation;
        record.request = request;
        record.status = status;
        record.state = state;
        record.is_active = true;
        ++residency_snapshot_.residency_record_count;
        return true;
    }

    return false;
}

bool ResourceRegistry::IsResidentState(ResourceResidencyState state) const {
    if (state == ResourceResidencyState::Resident) {
        return true;
    }

    if (state == ResourceResidencyState::Pinned) {
        return true;
    }

    return state == ResourceResidencyState::Evictable;
}

bool ResourceRegistry::IsEvictionCandidate(const ResourceSlot &slot) const {
    if (!slot.is_active) {
        return false;
    }

    if (slot.reference_count != 0U) {
        return false;
    }

    if (slot.load_state != ResourceLoadState::Uploaded) {
        return false;
    }

    if (slot.residency_state == ResourceResidencyState::Pinned) {
        return false;
    }

    return IsResidentState(slot.residency_state);
}

void ResourceRegistry::RemoveResidentCounters(const ResourceSlot &slot) {
    if (!IsResidentState(slot.residency_state)) {
        return;
    }

    --residency_snapshot_.resident_resource_count;
    residency_snapshot_.resident_byte_count -= slot.loaded_byte_count;

    if (slot.residency_state == ResourceResidencyState::Pinned) {
        --residency_snapshot_.pinned_resource_count;
        residency_snapshot_.pinned_byte_count -= slot.loaded_byte_count;
        return;
    }

    if (IsEvictionCandidate(slot)) {
        residency_snapshot_.evictable_byte_count -= slot.loaded_byte_count;
    }
}

void ResourceRegistry::RefreshEvictableState(ResourceSlot &slot) {
    if (!IsResidentState(slot.residency_state)) {
        return;
    }

    if (slot.residency_state == ResourceResidencyState::Pinned) {
        return;
    }

    if (slot.reference_count == 0U) {
        if (slot.residency_state != ResourceResidencyState::Evictable) {
            slot.residency_state = ResourceResidencyState::Evictable;
            residency_snapshot_.evictable_byte_count += slot.loaded_byte_count;
        }

        return;
    }

    if (slot.residency_state == ResourceResidencyState::Evictable) {
        residency_snapshot_.evictable_byte_count -= slot.loaded_byte_count;
        slot.residency_state = ResourceResidencyState::Resident;
    }
}

void ResourceRegistry::ClearResidencySlot(ResourceSlot &slot) {
    RemoveResidentCounters(slot);
    if (slot.residency_state == ResourceResidencyState::Evicted) {
        --residency_snapshot_.evicted_resource_count;
    }

    slot.residency_state = ResourceResidencyState::Unloaded;
}

void ResourceRegistry::ClearDecodePlanRecord(std::size_t record_index) {
    if (record_index >= MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT) {
        return;
    }

    ResourceDecodePlanRecord &record = decode_plan_records_[record_index];
    if (!record.is_active) {
        return;
    }

    const std::uint32_t decoded_byte_count = record.expected_decoded_byte_count;
    const ResourceHandle resource = record.resource;
    const std::uint64_t payload_id = record.payload_id;
    const std::uint64_t decode_plan_id = record.decode_plan_id;
    ClearDecodeResultsForDecodePlan(resource, payload_id, decode_plan_id);
    record = ResourceDecodePlanRecord{};
    --decode_plan_snapshot_.active_plan_count;
    --decode_plan_snapshot_.decode_plan_record_count;
    decode_plan_snapshot_.planned_decoded_byte_count -= decoded_byte_count;
}

void ResourceRegistry::ClearDecodePlansForCachePayload(ResourceHandle resource, std::uint64_t payload_id) {
    std::size_t record_index = 0U;
    while (record_index < MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT) {
        const ResourceDecodePlanRecord &record = decode_plan_records_[record_index];
        if (!record.is_active) {
            ++record_index;
            continue;
        }

        if (record.payload_id != payload_id) {
            ++record_index;
            continue;
        }

        if (record.resource.slot != resource.slot) {
            ++record_index;
            continue;
        }

        if (record.resource.generation != resource.generation) {
            ++record_index;
            continue;
        }

        ClearDecodePlanRecord(record_index);
        ++record_index;
    }
}

void ResourceRegistry::ClearDecodedPayloadRecord(std::size_t record_index) {
    if (record_index >= MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT) {
        return;
    }

    ResourceDecodedPayloadRecord &record = decoded_payload_records_[record_index];
    if (!record.is_active) {
        return;
    }

    const std::uint32_t decoded_byte_count = record.decoded_byte_count;
    std::uint32_t byte_index = 0U;
    while (byte_index < MAX_RESOURCE_DECODED_PAYLOAD_BYTES_PER_RECORD) {
        decoded_payload_bytes_[record_index][byte_index] = 0U;
        ++byte_index;
    }

    record = ResourceDecodedPayloadRecord{};
    --decoded_payload_snapshot_.active_payload_count;
    --decoded_payload_snapshot_.decoded_payload_record_count;
    decoded_payload_snapshot_.stored_decoded_byte_count -= decoded_byte_count;
}

void ResourceRegistry::ClearDecodedPayloadsForDecodeResult(
    ResourceHandle resource,
    std::uint64_t payload_id,
    std::uint64_t decode_plan_id,
    std::uint64_t decode_result_id) {
    std::size_t record_index = 0U;
    while (record_index < MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT) {
        const ResourceDecodedPayloadRecord &record = decoded_payload_records_[record_index];
        if (!record.is_active) {
            ++record_index;
            continue;
        }

        if (record.decode_result_id != decode_result_id) {
            ++record_index;
            continue;
        }

        if (record.decode_plan_id != decode_plan_id) {
            ++record_index;
            continue;
        }

        if (record.payload_id != payload_id) {
            ++record_index;
            continue;
        }

        if (record.resource.slot != resource.slot) {
            ++record_index;
            continue;
        }

        if (record.resource.generation != resource.generation) {
            ++record_index;
            continue;
        }

        ClearDecodedPayloadRecord(record_index);
        ++decoded_payload_snapshot_.dependent_cleared_payload_count;
        ++record_index;
    }
}

void ResourceRegistry::ClearDecodeResultRecord(std::size_t record_index) {
    if (record_index >= MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT) {
        return;
    }

    ResourceDecodeResultRecord &record = decode_result_records_[record_index];
    if (!record.is_active) {
        return;
    }

    const std::uint32_t decoded_byte_count = record.decoded_byte_count;
    const ResourceHandle resource = record.resource;
    const std::uint64_t payload_id = record.payload_id;
    const std::uint64_t decode_plan_id = record.decode_plan_id;
    const std::uint64_t decode_result_id = record.decode_result_id;
    ClearDecodedPayloadsForDecodeResult(resource, payload_id, decode_plan_id, decode_result_id);
    record = ResourceDecodeResultRecord{};
    --decode_result_snapshot_.active_result_count;
    --decode_result_snapshot_.decode_result_record_count;
    decode_result_snapshot_.committed_decoded_byte_count -= decoded_byte_count;
}

void ResourceRegistry::ClearDecodeResultsForDecodePlan(
    ResourceHandle resource,
    std::uint64_t payload_id,
    std::uint64_t decode_plan_id) {
    std::size_t record_index = 0U;
    while (record_index < MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT) {
        const ResourceDecodeResultRecord &record = decode_result_records_[record_index];
        if (!record.is_active) {
            ++record_index;
            continue;
        }

        if (record.decode_plan_id != decode_plan_id) {
            ++record_index;
            continue;
        }

        if (record.payload_id != payload_id) {
            ++record_index;
            continue;
        }

        if (record.resource.slot != resource.slot) {
            ++record_index;
            continue;
        }

        if (record.resource.generation != resource.generation) {
            ++record_index;
            continue;
        }

        ClearDecodeResultRecord(record_index);
        ++record_index;
    }
}

void ResourceRegistry::ClearDecodeResultsForCachePayload(ResourceHandle resource, std::uint64_t payload_id) {
    std::size_t record_index = 0U;
    while (record_index < MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT) {
        const ResourceDecodeResultRecord &record = decode_result_records_[record_index];
        if (!record.is_active) {
            ++record_index;
            continue;
        }

        if (record.payload_id != payload_id) {
            ++record_index;
            continue;
        }

        if (record.resource.slot != resource.slot) {
            ++record_index;
            continue;
        }

        if (record.resource.generation != resource.generation) {
            ++record_index;
            continue;
        }

        ClearDecodeResultRecord(record_index);
        ++record_index;
    }
}

void ResourceRegistry::ClearCachePayloadRecord(std::size_t record_index) {
    if (record_index >= MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT) {
        return;
    }

    ResourceCachePayloadRecord &record = cache_payload_records_[record_index];
    if (!record.is_active) {
        return;
    }

    const std::uint32_t payload_byte_count = record.payload_byte_count;
    const ResourceHandle resource = record.resource;
    const std::uint64_t payload_id = record.payload_id;
    ClearDecodeResultsForCachePayload(resource, payload_id);
    ClearDecodePlansForCachePayload(resource, payload_id);
    std::uint32_t byte_index = 0U;
    while (byte_index < MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD) {
        cache_payload_bytes_[record_index][byte_index] = 0U;
        ++byte_index;
    }

    record = ResourceCachePayloadRecord{};
    --cache_payload_snapshot_.cached_payload_count;
    --cache_payload_snapshot_.cache_payload_record_count;
    cache_payload_snapshot_.cached_byte_count -= payload_byte_count;
}

void ResourceRegistry::ClearCachePayloadForSlot(std::size_t slot_index) {
    std::size_t record_index = 0U;
    while (record_index < MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT) {
        const ResourceCachePayloadRecord &record = cache_payload_records_[record_index];
        if (!record.is_active) {
            ++record_index;
            continue;
        }

        if (record.resource.slot != slot_index) {
            ++record_index;
            continue;
        }

        ClearCachePayloadRecord(record_index);
        ++record_index;
    }
}

bool ResourceRegistry::HasInboundEdge(std::size_t slot_index) const {
    for (const ResourceDependencyEdge& edge : dependency_edges_) {
        if (!edge.is_active) {
            continue;
        }

        if (edge.dependency_slot == slot_index) {
            return true;
        }
    }

    return false;
}

bool ResourceRegistry::HasDependencyPath(std::size_t start_slot, std::size_t target_slot) const {
    std::array<bool, MAX_RESOURCE_COUNT> visited{};
    std::array<std::size_t, MAX_RESOURCE_COUNT> stack{};
    std::size_t stack_count = 0U;
    stack[stack_count] = start_slot;
    ++stack_count;

    while (stack_count > 0U) {
        --stack_count;
        const std::size_t current_slot = stack[stack_count];
        if (current_slot == target_slot) {
            return true;
        }

        if (current_slot >= snapshot_.resource_capacity) {
            continue;
        }

        if (visited[current_slot]) {
            continue;
        }

        visited[current_slot] = true;
        for (const ResourceDependencyEdge& edge : dependency_edges_) {
            if (!edge.is_active) {
                continue;
            }

            if (edge.dependent_slot != current_slot) {
                continue;
            }

            if (edge.dependency_slot >= snapshot_.resource_capacity) {
                continue;
            }

            if (visited[edge.dependency_slot]) {
                continue;
            }

            if (stack_count >= MAX_RESOURCE_COUNT) {
                continue;
            }

            stack[stack_count] = edge.dependency_slot;
            ++stack_count;
        }
    }

    return false;
}

void ResourceRegistry::ClearOutboundEdges(std::size_t slot_index) {
    for (ResourceDependencyEdge& edge : dependency_edges_) {
        if (!edge.is_active) {
            continue;
        }

        if (edge.dependent_slot != slot_index) {
            continue;
        }

        edge = ResourceDependencyEdge{};
        --snapshot_.dependency_edge_count;
    }
}

void ResourceRegistry::AdvanceGeneration(ResourceSlot& slot) {
    if (slot.generation == std::numeric_limits<std::uint32_t>::max()) {
        slot.generation = 1U;
        return;
    }

    ++slot.generation;
}
}
