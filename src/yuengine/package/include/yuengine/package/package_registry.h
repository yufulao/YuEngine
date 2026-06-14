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
    explicit PackageRegistry(package_registry_desc_t desc);

    package_registration_result_t RegisterSyntheticManifest(const package_manifest_descriptor_t& descriptor);
    package_registration_result_t RegisterEntry(const package_entry_descriptor_t& descriptor);
    PACKAGE_STATUS AddDependency(package_id_t package, package_entry_id_t dependent, package_entry_id_t dependency);
    package_load_plan_result_t ResolveEntryByResourceKey(
        package_id_t package,
        resource_type_id_t expectedType,
        const ResourceLogicalKey& logicalKey);
    package_snapshot_t Snapshot() const;

private:
    PACKAGE_STATUS RecordFailure(PACKAGE_STATUS status);
    void RecordSuccess();
    bool HasManifest(package_id_t package) const;
    bool HasDuplicateManifest(package_id_t package) const;
    bool HasDuplicateEntry(const package_entry_descriptor_t& descriptor) const;
    bool HasDuplicateResourceKey(const package_entry_descriptor_t& descriptor) const;
    PACKAGE_STATUS ValidateEntryDescriptor(const package_entry_descriptor_t& descriptor) const;
    PACKAGE_STATUS FindEntryIndex(package_id_t package, package_entry_id_t entry, std::size_t& outIndex) const;
    PACKAGE_STATUS FindEntryByResourceKey(
        package_id_t package,
        resource_type_id_t expectedType,
        const ResourceLogicalKey& logicalKey,
        std::size_t& outIndex) const;
    bool HasDependencyEdge(package_id_t package, package_entry_id_t dependent, package_entry_id_t dependency) const;
    bool HasDependencyPath(package_id_t package, package_entry_id_t start, package_entry_id_t target) const;
    std::uint32_t CountDirectDependencies(package_id_t package, package_entry_id_t entry) const;
    void AppendRecord(package_load_plan_t& plan, const package_entry_descriptor_t& descriptor) const;
    bool TryFindEntryIndex(package_id_t package, package_entry_id_t entry, std::size_t& outIndex) const;
    bool TryFindResourceIndex(
        package_id_t package,
        resource_type_id_t expectedType,
        const ResourceLogicalKey& logicalKey,
        std::size_t& outIndex) const;
    bool HasResourceLogicalKey(package_id_t package, const ResourceLogicalKey& logicalKey) const;
    bool TryFindDependencyEdgeIndex(
        package_id_t package,
        package_entry_id_t dependent,
        package_entry_id_t dependency,
        std::size_t& outIndex) const;
    void AddEntryIndex(package_id_t package, package_entry_id_t entry, std::uint32_t entryIndex);
    void AddResourceIndex(
        package_id_t package,
        resource_type_id_t type,
        const ResourceLogicalKey& logicalKey,
        std::uint32_t entryIndex);
    void AddResourceKeyIndex(package_id_t package, const ResourceLogicalKey& logicalKey, std::uint32_t entryIndex);
    void AddDependencyIndex(
        package_id_t package,
        package_entry_id_t dependent,
        package_entry_id_t dependency,
        std::uint32_t edgeIndex);
    void AppendDependencyEdge(std::uint32_t dependentEntryIndex, std::uint32_t edgeIndex);

    std::array<manifest_slot_t, MAX_PACKAGE_MANIFEST_COUNT> _manifests;
    std::array<entry_slot_t, MAX_PACKAGE_ENTRY_COUNT> _entries;
    std::array<dependency_edge_t, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyEdges;
    std::array<package_id_t, MAX_PACKAGE_ENTRY_COUNT> _entryIndexPackages;
    std::array<package_entry_id_t, MAX_PACKAGE_ENTRY_COUNT> _entryIndexEntries;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> _entryIndexValues;
    std::array<bool, MAX_PACKAGE_ENTRY_COUNT> _entryIndexActive;
    std::array<package_id_t, MAX_PACKAGE_ENTRY_COUNT> _resourceIndexPackages;
    std::array<resource_type_id_t, MAX_PACKAGE_ENTRY_COUNT> _resourceIndexTypes;
    std::array<ResourceLogicalKey, MAX_PACKAGE_ENTRY_COUNT> _resourceIndexKeys;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> _resourceIndexValues;
    std::array<bool, MAX_PACKAGE_ENTRY_COUNT> _resourceIndexActive;
    std::array<package_id_t, MAX_PACKAGE_ENTRY_COUNT> _resourceKeyIndexPackages;
    std::array<ResourceLogicalKey, MAX_PACKAGE_ENTRY_COUNT> _resourceKeyIndexKeys;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> _resourceKeyIndexValues;
    std::array<bool, MAX_PACKAGE_ENTRY_COUNT> _resourceKeyIndexActive;
    std::array<package_id_t, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyIndexPackages;
    std::array<package_entry_id_t, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyIndexDependents;
    std::array<package_entry_id_t, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyIndexDependencies;
    std::array<std::uint32_t, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyIndexValues;
    std::array<bool, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyIndexActive;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> _firstDependencyEdgeForEntry;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> _lastDependencyEdgeForEntry;
    std::array<std::uint32_t, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _nextDependencyEdge;
    package_snapshot_t _snapshot;
};
}
