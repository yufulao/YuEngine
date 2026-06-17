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
          ResourceLoadCommitStatus::Success} {
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
    slot.is_active = false;
    slot.logical_key = ResourceLogicalKey{};
    slot.type = ResourceTypeId{};
    slot.load_state = ResourceLoadState::Unloaded;
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
