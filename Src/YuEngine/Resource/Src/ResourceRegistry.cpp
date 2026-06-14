#include "YuEngine/Resource/ResourceRegistry.h"

#include <limits>

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::resource {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requestedCapacity, std::uint32_t maximumCapacity) {
    if (requestedCapacity > maximumCapacity) {
        return maximumCapacity;
    }

    return requestedCapacity;
}
}

ResourceRegistry::ResourceRegistry()
    : ResourceRegistry(ResourceRegistryDesc{}) {
}

ResourceRegistry::ResourceRegistry(ResourceRegistryDesc desc)
    : _slots{},
      _dependencyEdges{},
      _types{},
      _snapshot{
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

    if (_snapshot.registered_resource_count >= _snapshot.resource_capacity) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::CapacityExceeded));
    }

    ResourceSlot* freeSlot = nullptr;
    std::uint32_t freeSlotIndex = 0U;
    std::uint32_t slotIndex = 0U;
    for (ResourceSlot& slot : _slots) {
        if (slotIndex >= _snapshot.resource_capacity) {
            break;
        }

        if (slot.is_active) {
            ++slotIndex;
            continue;
        }

        freeSlot = &slot;
        freeSlotIndex = slotIndex;
        break;
    }

    if (freeSlot == nullptr) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::CapacityExceeded));
    }

    const ResourceStatus typeStatus = RegisterTypeIfNeeded(descriptor.type);
    if (typeStatus != ResourceStatus::Success) {
        return ResourceRegistrationResult::Failure(RecordFailure(typeStatus));
    }

    if (freeSlot->generation == INVALID_RESOURCE_GENERATION) {
        freeSlot->generation = 1U;
    }

    freeSlot->type = descriptor.type;
    freeSlot->logical_key = descriptor.logical_key;
    freeSlot->reference_count = descriptor.initial_reference_count;
    freeSlot->is_active = true;
    ++_snapshot.registered_resource_count;
    _snapshot.acquired_handle_count += descriptor.initial_reference_count;
    RecordSuccess();
    return ResourceRegistrationResult::Success(ResourceHandle{freeSlotIndex, freeSlot->generation});
}

ResourceStatus ResourceRegistry::AddDependency(ResourceHandle dependent, ResourceHandle dependency) {
    ++_snapshot.dependency_validation_count;

    std::size_t dependentIndex = 0U;
    const ResourceStatus dependentStatus = ResolveHandle(dependent, dependentIndex);
    if (dependentStatus != ResourceStatus::Success) {
        return RecordFailure(dependentStatus);
    }

    std::size_t dependencyIndex = 0U;
    const ResourceStatus dependencyStatus = ResolveHandle(dependency, dependencyIndex);
    if (dependencyStatus != ResourceStatus::Success) {
        return RecordFailure(ResourceStatus::DependencyMissing);
    }

    if (dependentIndex == dependencyIndex) {
        return RecordFailure(ResourceStatus::DependencyCycle);
    }

    if (HasDependencyPath(dependencyIndex, dependentIndex)) {
        return RecordFailure(ResourceStatus::DependencyCycle);
    }

    for (const ResourceDependencyEdge& edge : _dependencyEdges) {
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

    if (_snapshot.dependency_edge_count >= _snapshot.dependency_edge_capacity) {
        return RecordFailure(ResourceStatus::CapacityExceeded);
    }

    for (ResourceDependencyEdge& edge : _dependencyEdges) {
        if (edge.is_active) {
            continue;
        }

        edge.dependent_slot = dependent.slot;
        edge.dependency_slot = dependency.slot;
        edge.is_active = true;
        ++_snapshot.dependency_edge_count;
        RecordSuccess();
        return ResourceStatus::Success;
    }

    return RecordFailure(ResourceStatus::CapacityExceeded);
}

ResourceStatus ResourceRegistry::Acquire(ResourceHandle handle, ResourceTypeId expectedType) {
    std::size_t slotIndex = 0U;
    const ResourceStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ResourceStatus::Success) {
        return RecordFailure(handleStatus);
    }

    ResourceSlot& slot = _slots[slotIndex];
    if (slot.type.value != expectedType.value) {
        return RecordFailure(ResourceStatus::TypeMismatch);
    }

    if (slot.reference_count == std::numeric_limits<std::uint32_t>::max()) {
        return RecordFailure(ResourceStatus::ReferenceCountOverflow);
    }

    ++slot.reference_count;
    ++_snapshot.acquired_handle_count;
    RecordSuccess();
    return ResourceStatus::Success;
}

ResourceStatus ResourceRegistry::Release(ResourceHandle handle) {
    std::size_t slotIndex = 0U;
    const ResourceStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ResourceStatus::Success) {
        return RecordFailure(handleStatus);
    }

    ResourceSlot& slot = _slots[slotIndex];
    if (slot.reference_count == 0U) {
        return RecordFailure(ResourceStatus::NotAcquired);
    }

    --slot.reference_count;
    --_snapshot.acquired_handle_count;
    ++_snapshot.released_handle_count;
    RecordSuccess();
    return ResourceStatus::Success;
}

ResourceStatus ResourceRegistry::Retire(ResourceHandle handle) {
    std::size_t slotIndex = 0U;
    const ResourceStatus handleStatus = ResolveHandle(handle, slotIndex);
    if (handleStatus != ResourceStatus::Success) {
        return RecordFailure(handleStatus);
    }

    ResourceSlot& slot = _slots[slotIndex];
    if (slot.reference_count != 0U) {
        return RecordFailure(ResourceStatus::StillReferenced);
    }

    if (HasInboundEdge(slotIndex)) {
        return RecordFailure(ResourceStatus::StillDependedOn);
    }

    ClearOutboundEdges(slotIndex);
    slot.is_active = false;
    slot.logical_key = ResourceLogicalKey{};
    slot.type = ResourceTypeId{};
    slot.reference_count = 0U;
    AdvanceGeneration(slot);
    --_snapshot.registered_resource_count;
    ++_snapshot.retired_resource_count;
    RecordSuccess();
    return ResourceStatus::Success;
}

ResourceSnapshot ResourceRegistry::Snapshot() const {
    return _snapshot;
}

ResourceStatus ResourceRegistry::RecordFailure(ResourceStatus status) {
    ++_snapshot.failed_operation_count;
    _snapshot.last_status = status;
    return status;
}

void ResourceRegistry::RecordSuccess() {
    _snapshot.last_status = ResourceStatus::Success;
}

ResourceStatus ResourceRegistry::ResolveHandle(ResourceHandle handle, std::size_t& outIndex) const {
    if (!handle.IsValid()) {
        return ResourceStatus::InvalidHandle;
    }

    if (handle.slot >= _snapshot.resource_capacity) {
        return ResourceStatus::InvalidHandle;
    }

    const ResourceSlot& slot = _slots[handle.slot];
    if (slot.generation != handle.generation) {
        return ResourceStatus::GenerationMismatch;
    }

    if (!slot.is_active) {
        return ResourceStatus::InvalidHandle;
    }

    outIndex = handle.slot;
    return ResourceStatus::Success;
}

ResourceStatus ResourceRegistry::RegisterTypeIfNeeded(ResourceTypeId type) {
    if (HasType(type)) {
        return ResourceStatus::Success;
    }

    if (_snapshot.type_count >= _snapshot.type_capacity) {
        return ResourceStatus::CapacityExceeded;
    }

    _types[_snapshot.type_count] = type;
    ++_snapshot.type_count;
    return ResourceStatus::Success;
}

bool ResourceRegistry::HasType(ResourceTypeId type) const {
    std::uint32_t index = 0U;
    for (const ResourceTypeId& registeredType : _types) {
        if (index >= _snapshot.type_count) {
            return false;
        }

        if (registeredType.value == type.value) {
            return true;
        }

        ++index;
    }

    return false;
}

bool ResourceRegistry::HasDuplicateActiveResource(const ResourceDescriptor& descriptor) const {
    std::uint32_t index = 0U;
    for (const ResourceSlot& slot : _slots) {
        if (index >= _snapshot.resource_capacity) {
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

bool ResourceRegistry::HasInboundEdge(std::size_t slotIndex) const {
    for (const ResourceDependencyEdge& edge : _dependencyEdges) {
        if (!edge.is_active) {
            continue;
        }

        if (edge.dependency_slot == slotIndex) {
            return true;
        }
    }

    return false;
}

bool ResourceRegistry::HasDependencyPath(std::size_t startSlot, std::size_t targetSlot) const {
    std::array<bool, MAX_RESOURCE_COUNT> visited{};
    std::array<std::size_t, MAX_RESOURCE_COUNT> stack{};
    std::size_t stackCount = 0U;
    stack[stackCount] = startSlot;
    ++stackCount;

    while (stackCount > 0U) {
        --stackCount;
        const std::size_t currentSlot = stack[stackCount];
        if (currentSlot == targetSlot) {
            return true;
        }

        if (currentSlot >= _snapshot.resource_capacity) {
            continue;
        }

        if (visited[currentSlot]) {
            continue;
        }

        visited[currentSlot] = true;
        for (const ResourceDependencyEdge& edge : _dependencyEdges) {
            if (!edge.is_active) {
                continue;
            }

            if (edge.dependent_slot != currentSlot) {
                continue;
            }

            if (edge.dependency_slot >= _snapshot.resource_capacity) {
                continue;
            }

            if (visited[edge.dependency_slot]) {
                continue;
            }

            if (stackCount >= MAX_RESOURCE_COUNT) {
                continue;
            }

            stack[stackCount] = edge.dependency_slot;
            ++stackCount;
        }
    }

    return false;
}

void ResourceRegistry::ClearOutboundEdges(std::size_t slotIndex) {
    for (ResourceDependencyEdge& edge : _dependencyEdges) {
        if (!edge.is_active) {
            continue;
        }

        if (edge.dependent_slot != slotIndex) {
            continue;
        }

        edge = ResourceDependencyEdge{};
        --_snapshot.dependency_edge_count;
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
