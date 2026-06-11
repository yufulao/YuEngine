#include "yuengine/package/PackageRegistry.h"

#include <array>
#include <cstdint>
#include <limits>

#include "yuengine/memory/MemoryAccountingStatus.h"

namespace yuengine::package
{
namespace
{
std::uint32_t ClampCapacity(std::uint32_t requestedCapacity, std::uint32_t maximumCapacity)
{
    if (requestedCapacity > maximumCapacity)
    {
        return maximumCapacity;
    }

    return requestedCapacity;
}

bool ByteRangeIsWithinBounds(std::uint32_t byteOffset, std::uint32_t byteSize)
{
    if (byteSize > MAX_DECLARED_ENTRY_SIZE)
    {
        return false;
    }

    const std::uint64_t endOffset = static_cast<std::uint64_t>(byteOffset) + static_cast<std::uint64_t>(byteSize);
    return endOffset <= std::numeric_limits<std::uint32_t>::max();
}
}

PackageRegistry::PackageRegistry()
    : PackageRegistry(PackageRegistryDesc{})
{
}

PackageRegistry::PackageRegistry(PackageRegistryDesc desc)
    : _manifests{},
      _entries{},
      _dependencyEdges{},
      _snapshot{
          ClampCapacity(desc.ManifestCapacity, MAX_PACKAGE_MANIFEST_COUNT),
          ClampCapacity(desc.EntryCapacity, MAX_PACKAGE_ENTRY_COUNT),
          ClampCapacity(desc.DependencyEdgeCapacity, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT),
          ClampCapacity(desc.LoadPlanRecordCapacity, MAX_LOAD_PLAN_RECORD_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly,
          PackageStatus::Success}
{
}

PackageRegistrationResult PackageRegistry::RegisterSyntheticManifest(const PackageManifestDescriptor& descriptor)
{
    if (!descriptor.Id.IsValid())
    {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::InvalidPackageId));
    }

    if (HasDuplicateManifest(descriptor.Id))
    {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::DuplicateManifest));
    }

    if (_snapshot.ManifestCount >= _snapshot.ManifestCapacity)
    {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::ManifestCapacityExceeded));
    }

    std::uint32_t index = 0U;
    for (ManifestSlot& manifest : _manifests)
    {
        if (index >= _snapshot.ManifestCapacity)
        {
            return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::ManifestCapacityExceeded));
        }

        if (manifest.IsActive)
        {
            ++index;
            continue;
        }

        manifest.Id = descriptor.Id;
        manifest.IsActive = true;
        ++_snapshot.ManifestCount;
        RecordSuccess();
        return PackageRegistrationResult::ManifestSuccess(descriptor.Id);
    }

    return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::ManifestCapacityExceeded));
}

PackageRegistrationResult PackageRegistry::RegisterEntry(const PackageEntryDescriptor& descriptor)
{
    const PackageStatus validationStatus = ValidateEntryDescriptor(descriptor);
    if (validationStatus != PackageStatus::Success)
    {
        return PackageRegistrationResult::Failure(RecordFailure(validationStatus));
    }

    if (!HasManifest(descriptor.Package))
    {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::NotFound));
    }

    if (HasDuplicateEntry(descriptor))
    {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::DuplicateEntry));
    }

    if (HasDuplicateResourceKey(descriptor))
    {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::DuplicateResourceKey));
    }

    if (_snapshot.EntryCount >= _snapshot.EntryCapacity)
    {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::EntryCapacityExceeded));
    }

    std::uint32_t index = 0U;
    for (EntrySlot& entry : _entries)
    {
        if (index >= _snapshot.EntryCapacity)
        {
            return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::EntryCapacityExceeded));
        }

        if (entry.IsActive)
        {
            ++index;
            continue;
        }

        entry.Descriptor = descriptor;
        entry.IsActive = true;
        ++_snapshot.EntryCount;
        RecordSuccess();
        return PackageRegistrationResult::EntrySuccess(descriptor.Package, descriptor.Entry);
    }

    return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::EntryCapacityExceeded));
}

PackageStatus PackageRegistry::AddDependency(PackageId package, PackageEntryId dependent, PackageEntryId dependency)
{
    ++_snapshot.DependencyValidationCount;

    if (!package.IsValid())
    {
        return RecordFailure(PackageStatus::InvalidPackageId);
    }

    if (!dependent.IsValid() || !dependency.IsValid())
    {
        return RecordFailure(PackageStatus::InvalidEntryId);
    }

    std::size_t dependentIndex = 0U;
    const PackageStatus dependentStatus = FindEntryIndex(package, dependent, dependentIndex);
    if (dependentStatus != PackageStatus::Success)
    {
        return RecordFailure(PackageStatus::DependencyMissing);
    }

    std::size_t dependencyIndex = 0U;
    const PackageStatus dependencyStatus = FindEntryIndex(package, dependency, dependencyIndex);
    if (dependencyStatus != PackageStatus::Success)
    {
        return RecordFailure(PackageStatus::DependencyMissing);
    }

    if (dependent.Value == dependency.Value)
    {
        return RecordFailure(PackageStatus::DependencyCycle);
    }

    if (HasDependencyPath(package, dependency, dependent))
    {
        return RecordFailure(PackageStatus::DependencyCycle);
    }

    if (HasDependencyEdge(package, dependent, dependency))
    {
        RecordSuccess();
        return PackageStatus::Success;
    }

    if (_snapshot.DependencyEdgeCount >= _snapshot.DependencyEdgeCapacity)
    {
        return RecordFailure(PackageStatus::DependencyCapacityExceeded);
    }

    std::uint32_t index = 0U;
    for (DependencyEdge& edge : _dependencyEdges)
    {
        if (index >= _snapshot.DependencyEdgeCapacity)
        {
            return RecordFailure(PackageStatus::DependencyCapacityExceeded);
        }

        if (edge.IsActive)
        {
            ++index;
            continue;
        }

        edge.Package = package;
        edge.Dependent = dependent;
        edge.Dependency = dependency;
        edge.IsActive = true;
        ++_snapshot.DependencyEdgeCount;
        RecordSuccess();
        return PackageStatus::Success;
    }

    return RecordFailure(PackageStatus::DependencyCapacityExceeded);
}

PackageLoadPlanResult PackageRegistry::ResolveEntryByResourceKey(
    PackageId package,
    yuengine::resource::ResourceTypeId expectedType,
    const yuengine::resource::ResourceLogicalKey& logicalKey)
{
    if (!package.IsValid())
    {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::InvalidPackageId));
    }

    if (!expectedType.IsValid())
    {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::InvalidResourceType));
    }

    if (!logicalKey.IsWithinBounds())
    {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::LogicalKeyTooLong));
    }

    if (!logicalKey.IsValid())
    {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::InvalidLogicalKey));
    }

    std::size_t rootIndex = 0U;
    const PackageStatus findStatus = FindEntryByResourceKey(package, expectedType, logicalKey, rootIndex);
    if (findStatus != PackageStatus::Success)
    {
        return PackageLoadPlanResult::Failure(RecordFailure(findStatus));
    }

    const PackageEntryDescriptor& rootDescriptor = _entries[rootIndex].Descriptor;
    if (rootDescriptor.Type.Value != expectedType.Value)
    {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::TypeMismatch));
    }

    const std::uint32_t directDependencyCount = CountDirectDependencies(package, rootDescriptor.Entry);
    const std::uint32_t requiredRecordCount = directDependencyCount + 1U;
    if (requiredRecordCount > _snapshot.LoadPlanRecordCapacity)
    {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::LoadPlanCapacityExceeded));
    }

    PackageLoadPlan plan{};
    for (const DependencyEdge& edge : _dependencyEdges)
    {
        if (!edge.IsActive)
        {
            continue;
        }

        if (edge.Package.Value != package.Value)
        {
            continue;
        }

        if (edge.Dependent.Value != rootDescriptor.Entry.Value)
        {
            continue;
        }

        std::size_t dependencyIndex = 0U;
        if (FindEntryIndex(package, edge.Dependency, dependencyIndex) != PackageStatus::Success)
        {
            return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::DependencyMissing));
        }

        AppendRecord(plan, _entries[dependencyIndex].Descriptor);
    }

    AppendRecord(plan, rootDescriptor);
    ++_snapshot.LoadPlanResolveCount;
    _snapshot.LastLoadPlanRecordCount = plan.RecordCount;
    RecordSuccess();
    return PackageLoadPlanResult::Success(plan);
}

PackageSnapshot PackageRegistry::Snapshot() const
{
    return _snapshot;
}

PackageStatus PackageRegistry::RecordFailure(PackageStatus status)
{
    ++_snapshot.RejectedOperationCount;
    _snapshot.LastStatus = status;
    return status;
}

void PackageRegistry::RecordSuccess()
{
    _snapshot.LastStatus = PackageStatus::Success;
}

bool PackageRegistry::HasManifest(PackageId package) const
{
    return HasDuplicateManifest(package);
}

bool PackageRegistry::HasDuplicateManifest(PackageId package) const
{
    std::uint32_t index = 0U;
    for (const ManifestSlot& manifest : _manifests)
    {
        if (index >= _snapshot.ManifestCapacity)
        {
            return false;
        }

        if (!manifest.IsActive)
        {
            ++index;
            continue;
        }

        if (manifest.Id.Value == package.Value)
        {
            return true;
        }

        ++index;
    }

    return false;
}

bool PackageRegistry::HasDuplicateEntry(const PackageEntryDescriptor& descriptor) const
{
    std::uint32_t index = 0U;
    for (const EntrySlot& entry : _entries)
    {
        if (index >= _snapshot.EntryCapacity)
        {
            return false;
        }

        if (!entry.IsActive)
        {
            ++index;
            continue;
        }

        if (entry.Descriptor.Package.Value == descriptor.Package.Value &&
            entry.Descriptor.Entry.Value == descriptor.Entry.Value)
        {
            return true;
        }

        ++index;
    }

    return false;
}

bool PackageRegistry::HasDuplicateResourceKey(const PackageEntryDescriptor& descriptor) const
{
    std::uint32_t index = 0U;
    for (const EntrySlot& entry : _entries)
    {
        if (index >= _snapshot.EntryCapacity)
        {
            return false;
        }

        if (!entry.IsActive)
        {
            ++index;
            continue;
        }

        if (entry.Descriptor.Package.Value != descriptor.Package.Value)
        {
            ++index;
            continue;
        }

        if (entry.Descriptor.Type.Value != descriptor.Type.Value)
        {
            ++index;
            continue;
        }

        if (entry.Descriptor.LogicalKey.Equals(descriptor.LogicalKey))
        {
            return true;
        }

        ++index;
    }

    return false;
}

PackageStatus PackageRegistry::ValidateEntryDescriptor(const PackageEntryDescriptor& descriptor) const
{
    if (!descriptor.Package.IsValid())
    {
        return PackageStatus::InvalidPackageId;
    }

    if (!descriptor.Entry.IsValid())
    {
        return PackageStatus::InvalidEntryId;
    }

    if (!descriptor.Type.IsValid())
    {
        return PackageStatus::InvalidResourceType;
    }

    if (!descriptor.LogicalKey.IsWithinBounds())
    {
        return PackageStatus::LogicalKeyTooLong;
    }

    if (!descriptor.LogicalKey.IsValid())
    {
        return PackageStatus::InvalidLogicalKey;
    }

    if (!descriptor.SourceKey.IsWithinBounds())
    {
        return PackageStatus::SourceKeyTooLong;
    }

    if (!descriptor.SourceKey.IsValid())
    {
        return PackageStatus::InvalidSourceKey;
    }

    if (!ByteRangeIsWithinBounds(descriptor.ByteOffset, descriptor.ByteSize))
    {
        return PackageStatus::ByteRangeOutOfBounds;
    }

    return PackageStatus::Success;
}

PackageStatus PackageRegistry::FindEntryIndex(PackageId package, PackageEntryId entry, std::size_t& outIndex) const
{
    if (!package.IsValid())
    {
        return PackageStatus::InvalidPackageId;
    }

    if (!entry.IsValid())
    {
        return PackageStatus::InvalidEntryId;
    }

    std::uint32_t index = 0U;
    for (const EntrySlot& slot : _entries)
    {
        if (index >= _snapshot.EntryCapacity)
        {
            return PackageStatus::NotFound;
        }

        if (!slot.IsActive)
        {
            ++index;
            continue;
        }

        if (slot.Descriptor.Package.Value == package.Value && slot.Descriptor.Entry.Value == entry.Value)
        {
            outIndex = index;
            return PackageStatus::Success;
        }

        ++index;
    }

    return PackageStatus::NotFound;
}

PackageStatus PackageRegistry::FindEntryByResourceKey(
    PackageId package,
    yuengine::resource::ResourceTypeId expectedType,
    const yuengine::resource::ResourceLogicalKey& logicalKey,
    std::size_t& outIndex) const
{
    if (!HasManifest(package))
    {
        return PackageStatus::NotFound;
    }

    bool foundLogicalKeyWithDifferentType = false;
    std::uint32_t index = 0U;
    for (const EntrySlot& entry : _entries)
    {
        if (index >= _snapshot.EntryCapacity)
        {
            return foundLogicalKeyWithDifferentType ? PackageStatus::TypeMismatch : PackageStatus::NotFound;
        }

        if (!entry.IsActive)
        {
            ++index;
            continue;
        }

        if (entry.Descriptor.Package.Value != package.Value)
        {
            ++index;
            continue;
        }

        if (entry.Descriptor.LogicalKey.Equals(logicalKey))
        {
            if (entry.Descriptor.Type.Value == expectedType.Value)
            {
                outIndex = index;
                return PackageStatus::Success;
            }

            foundLogicalKeyWithDifferentType = true;
        }

        ++index;
    }

    return foundLogicalKeyWithDifferentType ? PackageStatus::TypeMismatch : PackageStatus::NotFound;
}

bool PackageRegistry::HasDependencyEdge(PackageId package, PackageEntryId dependent, PackageEntryId dependency) const
{
    for (const DependencyEdge& edge : _dependencyEdges)
    {
        if (!edge.IsActive)
        {
            continue;
        }

        if (edge.Package.Value != package.Value)
        {
            continue;
        }

        if (edge.Dependent.Value == dependent.Value && edge.Dependency.Value == dependency.Value)
        {
            return true;
        }
    }

    return false;
}

bool PackageRegistry::HasDependencyPath(PackageId package, PackageEntryId start, PackageEntryId target) const
{
    std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT> stack{};
    std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT> visited{};
    std::uint32_t stackCount = 0U;
    std::uint32_t visitedCount = 0U;
    const auto containsEntry = [](const std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT>& entries,
                                  std::uint32_t count,
                                  PackageEntryId entry) {
        for (std::uint32_t index = 0U; index < count; ++index)
        {
            if (entries[index].Value == entry.Value)
            {
                return true;
            }
        }

        return false;
    };

    stack[stackCount] = start;
    ++stackCount;

    while (stackCount > 0U)
    {
        --stackCount;
        const PackageEntryId current = stack[stackCount];
        if (current.Value == target.Value)
        {
            return true;
        }

        if (containsEntry(visited, visitedCount, current))
        {
            continue;
        }

        if (visitedCount < MAX_PACKAGE_ENTRY_COUNT)
        {
            visited[visitedCount] = current;
            ++visitedCount;
        }

        for (const DependencyEdge& edge : _dependencyEdges)
        {
            if (!edge.IsActive)
            {
                continue;
            }

            if (edge.Package.Value != package.Value)
            {
                continue;
            }

            if (edge.Dependent.Value != current.Value)
            {
                continue;
            }

            if (containsEntry(visited, visitedCount, edge.Dependency) ||
                containsEntry(stack, stackCount, edge.Dependency))
            {
                continue;
            }

            stack[stackCount] = edge.Dependency;
            ++stackCount;
        }
    }

    return false;
}

std::uint32_t PackageRegistry::CountDirectDependencies(PackageId package, PackageEntryId entry) const
{
    std::uint32_t count = 0U;
    for (const DependencyEdge& edge : _dependencyEdges)
    {
        if (!edge.IsActive)
        {
            continue;
        }

        if (edge.Package.Value != package.Value)
        {
            continue;
        }

        if (edge.Dependent.Value == entry.Value)
        {
            ++count;
        }
    }

    return count;
}

void PackageRegistry::AppendRecord(PackageLoadPlan& plan, const PackageEntryDescriptor& descriptor) const
{
    plan.Records[plan.RecordCount] = PackageLoadPlanRecord{
        descriptor.Package,
        descriptor.Entry,
        descriptor.Type,
        descriptor.LogicalKey,
        descriptor.SourceKey,
        descriptor.ByteOffset,
        descriptor.ByteSize};
    ++plan.RecordCount;
}
}
