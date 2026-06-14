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
          ClampCapacity(desc.ResourceCapacity, MAX_RESOURCE_COUNT),
          ClampCapacity(desc.TypeCapacity, MAX_RESOURCE_TYPE_COUNT),
          ClampCapacity(desc.DependencyEdgeCapacity, MAX_DEPENDENCY_EDGE_COUNT),
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
    if (!descriptor.Type.IsValid()) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::UnsupportedInThisGate));
    }

    if (!descriptor.LogicalKey.IsWithinBounds()) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::CapacityExceeded));
    }

    if (!descriptor.LogicalKey.IsValid()) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::UnsupportedInThisGate));
    }

    if (HasDuplicateActiveResource(descriptor)) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::DuplicateResource));
    }

    if (_snapshot.RegisteredResourceCount >= _snapshot.ResourceCapacity) {
        return ResourceRegistrationResult::Failure(RecordFailure(ResourceStatus::CapacityExceeded));
    }

    ResourceSlot* freeSlot = nullptr;
    std::uint32_t freeSlotIndex = 0U;
    std::uint32_t slotIndex = 0U;
    for (ResourceSlot& slot : _slots) {
        if (slotIndex >= _snapshot.ResourceCapacity) {
            break;
        }

        if (slot.IsActive) {
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

    const ResourceStatus typeStatus = RegisterTypeIfNeeded(descriptor.Type);
    if (typeStatus != ResourceStatus::Success) {
        return ResourceRegistrationResult::Failure(RecordFailure(typeStatus));
    }

    if (freeSlot->Generation == INVALID_RESOURCE_GENERATION) {
        freeSlot->Generation = 1U;
    }

    freeSlot->Type = descriptor.Type;
    freeSlot->LogicalKey = descriptor.LogicalKey;
    freeSlot->ReferenceCount = descriptor.InitialReferenceCount;
    freeSlot->IsActive = true;
    ++_snapshot.RegisteredResourceCount;
    _snapshot.AcquiredHandleCount += descriptor.InitialReferenceCount;
    RecordSuccess();
    return ResourceRegistrationResult::Success(ResourceHandle{freeSlotIndex, freeSlot->Generation});
}

ResourceStatus ResourceRegistry::AddDependency(ResourceHandle dependent, ResourceHandle dependency) {
    ++_snapshot.DependencyValidationCount;

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
        if (!edge.IsActive) {
            continue;
        }

        if (edge.DependentSlot != dependent.Slot) {
            continue;
        }

        if (edge.DependencySlot == dependency.Slot) {
            RecordSuccess();
            return ResourceStatus::Success;
        }
    }

    if (_snapshot.DependencyEdgeCount >= _snapshot.DependencyEdgeCapacity) {
        return RecordFailure(ResourceStatus::CapacityExceeded);
    }

    for (ResourceDependencyEdge& edge : _dependencyEdges) {
        if (edge.IsActive) {
            continue;
        }

        edge.DependentSlot = dependent.Slot;
        edge.DependencySlot = dependency.Slot;
        edge.IsActive = true;
        ++_snapshot.DependencyEdgeCount;
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
    if (slot.Type.Value != expectedType.Value) {
        return RecordFailure(ResourceStatus::TypeMismatch);
    }

    if (slot.ReferenceCount == std::numeric_limits<std::uint32_t>::max()) {
        return RecordFailure(ResourceStatus::ReferenceCountOverflow);
    }

    ++slot.ReferenceCount;
    ++_snapshot.AcquiredHandleCount;
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
    if (slot.ReferenceCount == 0U) {
        return RecordFailure(ResourceStatus::NotAcquired);
    }

    --slot.ReferenceCount;
    --_snapshot.AcquiredHandleCount;
    ++_snapshot.ReleasedHandleCount;
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
    if (slot.ReferenceCount != 0U) {
        return RecordFailure(ResourceStatus::StillReferenced);
    }

    if (HasInboundEdge(slotIndex)) {
        return RecordFailure(ResourceStatus::StillDependedOn);
    }

    ClearOutboundEdges(slotIndex);
    slot.IsActive = false;
    slot.LogicalKey = ResourceLogicalKey{};
    slot.Type = ResourceTypeId{};
    slot.ReferenceCount = 0U;
    AdvanceGeneration(slot);
    --_snapshot.RegisteredResourceCount;
    ++_snapshot.RetiredResourceCount;
    RecordSuccess();
    return ResourceStatus::Success;
}

ResourceSnapshot ResourceRegistry::Snapshot() const {
    return _snapshot;
}

ResourceStatus ResourceRegistry::RecordFailure(ResourceStatus status) {
    ++_snapshot.FailedOperationCount;
    _snapshot.LastStatus = status;
    return status;
}

void ResourceRegistry::RecordSuccess() {
    _snapshot.LastStatus = ResourceStatus::Success;
}

ResourceStatus ResourceRegistry::ResolveHandle(ResourceHandle handle, std::size_t& outIndex) const {
    if (!handle.IsValid()) {
        return ResourceStatus::InvalidHandle;
    }

    if (handle.Slot >= _snapshot.ResourceCapacity) {
        return ResourceStatus::InvalidHandle;
    }

    const ResourceSlot& slot = _slots[handle.Slot];
    if (slot.Generation != handle.Generation) {
        return ResourceStatus::GenerationMismatch;
    }

    if (!slot.IsActive) {
        return ResourceStatus::InvalidHandle;
    }

    outIndex = handle.Slot;
    return ResourceStatus::Success;
}

ResourceStatus ResourceRegistry::RegisterTypeIfNeeded(ResourceTypeId type) {
    if (HasType(type)) {
        return ResourceStatus::Success;
    }

    if (_snapshot.TypeCount >= _snapshot.TypeCapacity) {
        return ResourceStatus::CapacityExceeded;
    }

    _types[_snapshot.TypeCount] = type;
    ++_snapshot.TypeCount;
    return ResourceStatus::Success;
}

bool ResourceRegistry::HasType(ResourceTypeId type) const {
    std::uint32_t index = 0U;
    for (const ResourceTypeId& registeredType : _types) {
        if (index >= _snapshot.TypeCount) {
            return false;
        }

        if (registeredType.Value == type.Value) {
            return true;
        }

        ++index;
    }

    return false;
}

bool ResourceRegistry::HasDuplicateActiveResource(const ResourceDescriptor& descriptor) const {
    std::uint32_t index = 0U;
    for (const ResourceSlot& slot : _slots) {
        if (index >= _snapshot.ResourceCapacity) {
            return false;
        }

        if (!slot.IsActive) {
            ++index;
            continue;
        }

        if (slot.Type.Value != descriptor.Type.Value) {
            ++index;
            continue;
        }

        if (slot.LogicalKey.Equals(descriptor.LogicalKey)) {
            return true;
        }

        ++index;
    }

    return false;
}

bool ResourceRegistry::HasInboundEdge(std::size_t slotIndex) const {
    for (const ResourceDependencyEdge& edge : _dependencyEdges) {
        if (!edge.IsActive) {
            continue;
        }

        if (edge.DependencySlot == slotIndex) {
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

        if (currentSlot >= _snapshot.ResourceCapacity) {
            continue;
        }

        if (visited[currentSlot]) {
            continue;
        }

        visited[currentSlot] = true;
        for (const ResourceDependencyEdge& edge : _dependencyEdges) {
            if (!edge.IsActive) {
                continue;
            }

            if (edge.DependentSlot != currentSlot) {
                continue;
            }

            if (edge.DependencySlot >= _snapshot.ResourceCapacity) {
                continue;
            }

            if (visited[edge.DependencySlot]) {
                continue;
            }

            if (stackCount >= MAX_RESOURCE_COUNT) {
                continue;
            }

            stack[stackCount] = edge.DependencySlot;
            ++stackCount;
        }
    }

    return false;
}

void ResourceRegistry::ClearOutboundEdges(std::size_t slotIndex) {
    for (ResourceDependencyEdge& edge : _dependencyEdges) {
        if (!edge.IsActive) {
            continue;
        }

        if (edge.DependentSlot != slotIndex) {
            continue;
        }

        edge = ResourceDependencyEdge{};
        --_snapshot.DependencyEdgeCount;
    }
}

void ResourceRegistry::AdvanceGeneration(ResourceSlot& slot) {
    if (slot.Generation == std::numeric_limits<std::uint32_t>::max()) {
        slot.Generation = 1U;
        return;
    }

    ++slot.Generation;
}
}
