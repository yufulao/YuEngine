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
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ResourceStatus::Success} {
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
    slot.reference_count = 0U;
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
