#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "yuengine/package/package_constants.h"
#include "yuengine/package/package_entry_descriptor.h"
#include "yuengine/package/package_load_plan_result.h"
#include "yuengine/package/package_manifest_descriptor.h"
#include "yuengine/package/package_registration_result.h"
#include "yuengine/package/package_registry_desc.h"
#include "yuengine/package/package_snapshot.h"
#include "yuengine/package/dependency_edge.h"
#include "yuengine/package/entry_slot.h"
#include "yuengine/package/manifest_slot.h"

namespace yuengine::package {
class PackageRegistry final {
public:
    PackageRegistry();
    explicit PackageRegistry(PackageRegistryDesc desc);

    PackageRegistrationResult RegisterSyntheticManifest(const PackageManifestDescriptor& descriptor);
    PackageRegistrationResult RegisterEntry(const PackageEntryDescriptor& descriptor);
    PACKAGE_STATUS AddDependency(PackageId package, PackageEntryId dependent, PackageEntryId dependency);
    PackageLoadPlanResult ResolveEntryByResourceKey(
        PackageId package,
        ResourceTypeId expectedType,
        const ResourceLogicalKey& logicalKey);
    PackageSnapshot Snapshot() const;

private:
    PACKAGE_STATUS RecordFailure(PACKAGE_STATUS status);
    void RecordSuccess();
    bool HasManifest(PackageId package) const;
    bool HasDuplicateManifest(PackageId package) const;
    bool HasDuplicateEntry(const PackageEntryDescriptor& descriptor) const;
    bool HasDuplicateResourceKey(const PackageEntryDescriptor& descriptor) const;
    PACKAGE_STATUS ValidateEntryDescriptor(const PackageEntryDescriptor& descriptor) const;
    PACKAGE_STATUS FindEntryIndex(PackageId package, PackageEntryId entry, std::size_t& outIndex) const;
    PACKAGE_STATUS FindEntryByResourceKey(
        PackageId package,
        ResourceTypeId expectedType,
        const ResourceLogicalKey& logicalKey,
        std::size_t& outIndex) const;
    bool HasDependencyEdge(PackageId package, PackageEntryId dependent, PackageEntryId dependency) const;
    bool HasDependencyPath(PackageId package, PackageEntryId start, PackageEntryId target) const;
    std::uint32_t CountDirectDependencies(PackageId package, PackageEntryId entry) const;
    void AppendRecord(PackageLoadPlan& plan, const PackageEntryDescriptor& descriptor) const;
    bool TryFindEntryIndex(PackageId package, PackageEntryId entry, std::size_t& outIndex) const;
    bool TryFindResourceIndex(
        PackageId package,
        ResourceTypeId expectedType,
        const ResourceLogicalKey& logicalKey,
        std::size_t& outIndex) const;
    bool HasResourceLogicalKey(PackageId package, const ResourceLogicalKey& logicalKey) const;
    bool TryFindDependencyEdgeIndex(
        PackageId package,
        PackageEntryId dependent,
        PackageEntryId dependency,
        std::size_t& outIndex) const;
    void AddEntryIndex(PackageId package, PackageEntryId entry, std::uint32_t entryIndex);
    void AddResourceIndex(
        PackageId package,
        ResourceTypeId type,
        const ResourceLogicalKey& logicalKey,
        std::uint32_t entryIndex);
    void AddResourceKeyIndex(PackageId package, const ResourceLogicalKey& logicalKey, std::uint32_t entryIndex);
    void AddDependencyIndex(
        PackageId package,
        PackageEntryId dependent,
        PackageEntryId dependency,
        std::uint32_t edgeIndex);
    void AppendDependencyEdge(std::uint32_t dependentEntryIndex, std::uint32_t edgeIndex);

    std::array<ManifestSlot, MAX_PACKAGE_MANIFEST_COUNT> _manifests;
    std::array<EntrySlot, MAX_PACKAGE_ENTRY_COUNT> _entries;
    std::array<DependencyEdge, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyEdges;
    std::array<PackageId, MAX_PACKAGE_ENTRY_COUNT> _entryIndexPackages;
    std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT> _entryIndexEntries;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> _entryIndexValues;
    std::array<bool, MAX_PACKAGE_ENTRY_COUNT> _entryIndexActive;
    std::array<PackageId, MAX_PACKAGE_ENTRY_COUNT> _resourceIndexPackages;
    std::array<ResourceTypeId, MAX_PACKAGE_ENTRY_COUNT> _resourceIndexTypes;
    std::array<ResourceLogicalKey, MAX_PACKAGE_ENTRY_COUNT> _resourceIndexKeys;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> _resourceIndexValues;
    std::array<bool, MAX_PACKAGE_ENTRY_COUNT> _resourceIndexActive;
    std::array<PackageId, MAX_PACKAGE_ENTRY_COUNT> _resourceKeyIndexPackages;
    std::array<ResourceLogicalKey, MAX_PACKAGE_ENTRY_COUNT> _resourceKeyIndexKeys;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> _resourceKeyIndexValues;
    std::array<bool, MAX_PACKAGE_ENTRY_COUNT> _resourceKeyIndexActive;
    std::array<PackageId, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyIndexPackages;
    std::array<PackageEntryId, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyIndexDependents;
    std::array<PackageEntryId, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyIndexDependencies;
    std::array<std::uint32_t, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyIndexValues;
    std::array<bool, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyIndexActive;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> _firstDependencyEdgeForEntry;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> _lastDependencyEdgeForEntry;
    std::array<std::uint32_t, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _nextDependencyEdge;
    PackageSnapshot _snapshot;
};
}
