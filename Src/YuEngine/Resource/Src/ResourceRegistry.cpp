// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Src/ResourceRegistry.cpp

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
      cache_payload_bytes_{},
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
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ResourceStatus::Success,
          ResourceLoadState::Unloaded,
          ResourceLoadCommitStatus::Success},
      residency_snapshot_{},
      cache_payload_snapshot_{} {
}

ResourceRegistrationResult ResourceRegistry::RegisterSyntheticDescriptor(const ResourceDescriptor& descriptor) {
    if (!descriptor.type.IsValid()) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::UnsupportedInThisGate));
    }

    if (!descriptor.logical_key.IsWithinBounds()) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::CapacityExceeded));
    }

    if (!descriptor.logical_key.IsValid()) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::UnsupportedInThisGate));
    }

    if (HasDuplicateActiveResource(descriptor)) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::DuplicateResource));
    }

    if (snapshot_.registered_resource_count >= snapshot_.resource_capacity) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::CapacityExceeded));
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
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::CapacityExceeded));
    }

    const ResourceStatus type_status = RegisterTypeIfNeeded(descriptor.type);
    if (type_status != ResourceStatus::Success) {
        return ResourceRegistrationResult::Failure(RecordFailure(type_status));
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
        return RecordFailure(ResourceStatus::CapacityExceeded);
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

    return RecordFailure(ResourceStatus::CapacityExceeded);
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

    if (desc.byte_capacity < cache_payload_snapshot_.cached_byte_count) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::ConfigureBudget,
            request,
            ResourceCachePayloadStatus::BudgetExceeded);
    }

    cache_payload_snapshot_.budget_byte_capacity = desc.byte_capacity;
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

    if (HasCachePayloadId(request.payload_id)) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Store,
            request,
            ResourceCachePayloadStatus::DuplicatePayloadId);
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
    if (output_byte_capacity < record.payload_byte_count) {
        return RecordCachePayloadRejected(
            ResourceCachePayloadOperation::Read,
            request,
            ResourceCachePayloadStatus::OutputBufferTooSmall);
    }

    std::uint32_t byte_index = 0U;
    while (byte_index < record.payload_byte_count) {
        output_bytes[byte_index] = cache_payload_bytes_[record_index][byte_index];
        ++byte_index;
    }

    *output_byte_count = record.payload_byte_count;
    RecordCachePayloadSuccess(
        ResourceCachePayloadOperation::Read,
        request,
        record.cache_slot_index,
        record.payload_byte_count);
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
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return status;
}

void ResourceRegistry::RecordSuccess() {
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
    if (status == ResourceCachePayloadStatus::DuplicatePayloadId) {
        ++cache_payload_snapshot_.duplicate_payload_rejected_count;
    }

    if (status == ResourceCachePayloadStatus::CapacityExceeded) {
        ++cache_payload_snapshot_.capacity_rejected_payload_count;
    }

    if (status == ResourceCachePayloadStatus::BudgetExceeded) {
        ++cache_payload_snapshot_.budget_rejected_payload_count;
    }

    ++snapshot_.failed_operation_count;
    cache_payload_snapshot_.last_operation = operation;
    cache_payload_snapshot_.last_status = status;
    cache_payload_snapshot_.last_resource = request.resource;
    cache_payload_snapshot_.last_payload_id = request.payload_id;
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
    cache_payload_snapshot_.last_resource = request.resource;
    cache_payload_snapshot_.last_payload_id = request.payload_id;
    cache_payload_snapshot_.last_cache_slot_index = cache_slot_index;
    cache_payload_snapshot_.last_payload_byte_count = payload_byte_count;
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
        case ResourceCachePayloadStatus::OutputBufferTooSmall:
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

void ResourceRegistry::ClearCachePayloadRecord(std::size_t record_index) {
    if (record_index >= MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT) {
        return;
    }

    ResourceCachePayloadRecord &record = cache_payload_records_[record_index];
    if (!record.is_active) {
        return;
    }

    const std::uint32_t payload_byte_count = record.payload_byte_count;
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
