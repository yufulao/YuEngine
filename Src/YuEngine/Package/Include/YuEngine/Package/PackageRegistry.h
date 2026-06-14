#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Package/PackageConstants.h"
#include "YuEngine/Package/PackageEntryDescriptor.h"
#include "YuEngine/Package/PackageLoadPlanResult.h"
#include "YuEngine/Package/PackageManifestDescriptor.h"
#include "YuEngine/Package/PackageRegistrationResult.h"
#include "YuEngine/Package/PackageRegistryDesc.h"
#include "YuEngine/Package/PackageSnapshot.h"
#include "YuEngine/Package/DependencyEdge.h"
#include "YuEngine/Package/EntrySlot.h"
#include "YuEngine/Package/ManifestSlot.h"

namespace yuengine::package {
class PackageRegistry final {
public:
    PackageRegistry();
    explicit PackageRegistry(PackageRegistryDesc desc);

    PackageRegistrationResult RegisterSyntheticManifest(const PackageManifestDescriptor& descriptor);
    PackageRegistrationResult RegisterEntry(const PackageEntryDescriptor& descriptor);
    PackageStatus AddDependency(PackageId package, PackageEntryId dependent, PackageEntryId dependency);
    PackageLoadPlanResult ResolveEntryByResourceKey(
        PackageId package,
        ResourceTypeId expected_type,
        const ResourceLogicalKey& logical_key);
    PackageSnapshot Snapshot() const;

private:
    PackageStatus RecordFailure(PackageStatus status);
    void RecordSuccess();
    bool HasManifest(PackageId package) const;
    bool HasDuplicateManifest(PackageId package) const;
    bool HasDuplicateEntry(const PackageEntryDescriptor& descriptor) const;
    bool HasDuplicateResourceKey(const PackageEntryDescriptor& descriptor) const;
    PackageStatus ValidateEntryDescriptor(const PackageEntryDescriptor& descriptor) const;
    PackageStatus FindEntryIndex(PackageId package, PackageEntryId entry, std::size_t& out_index) const;
    PackageStatus FindEntryByResourceKey(
        PackageId package,
        ResourceTypeId expected_type,
        const ResourceLogicalKey& logical_key,
        std::size_t& out_index) const;
    bool HasDependencyEdge(PackageId package, PackageEntryId dependent, PackageEntryId dependency) const;
    bool HasDependencyPath(PackageId package, PackageEntryId start, PackageEntryId target) const;
    std::uint32_t CountDirectDependencies(PackageId package, PackageEntryId entry) const;
    void AppendRecord(PackageLoadPlan& plan, const PackageEntryDescriptor& descriptor) const;
    bool TryFindEntryIndex(PackageId package, PackageEntryId entry, std::size_t& out_index) const;
    bool TryFindResourceIndex(
        PackageId package,
        ResourceTypeId expected_type,
        const ResourceLogicalKey& logical_key,
        std::size_t& out_index) const;
    bool HasResourceLogicalKey(PackageId package, const ResourceLogicalKey& logical_key) const;
    bool TryFindDependencyEdgeIndex(
        PackageId package,
        PackageEntryId dependent,
        PackageEntryId dependency,
        std::size_t& out_index) const;
    void AddEntryIndex(PackageId package, PackageEntryId entry, std::uint32_t entry_index);
    void AddResourceIndex(
        PackageId package,
        ResourceTypeId type,
        const ResourceLogicalKey& logical_key,
        std::uint32_t entry_index);
    void AddResourceKeyIndex(PackageId package, const ResourceLogicalKey& logical_key, std::uint32_t entry_index);
    void AddDependencyIndex(
        PackageId package,
        PackageEntryId dependent,
        PackageEntryId dependency,
        std::uint32_t edge_index);
    void AppendDependencyEdge(std::uint32_t dependent_entry_index, std::uint32_t edge_index);

    std::array<ManifestSlot, MAX_PACKAGE_MANIFEST_COUNT> manifests_;
    std::array<EntrySlot, MAX_PACKAGE_ENTRY_COUNT> entries_;
    std::array<DependencyEdge, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> dependency_edges_;
    std::array<PackageId, MAX_PACKAGE_ENTRY_COUNT> entry_index_packages_;
    std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT> entry_index_entries_;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> entry_index_values_;
    std::array<bool, MAX_PACKAGE_ENTRY_COUNT> entry_index_active_;
    std::array<PackageId, MAX_PACKAGE_ENTRY_COUNT> resource_index_packages_;
    std::array<ResourceTypeId, MAX_PACKAGE_ENTRY_COUNT> resource_index_types_;
    std::array<ResourceLogicalKey, MAX_PACKAGE_ENTRY_COUNT> resource_index_keys_;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> resource_index_values_;
    std::array<bool, MAX_PACKAGE_ENTRY_COUNT> resource_index_active_;
    std::array<PackageId, MAX_PACKAGE_ENTRY_COUNT> resource_key_index_packages_;
    std::array<ResourceLogicalKey, MAX_PACKAGE_ENTRY_COUNT> resource_key_index_keys_;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> resource_key_index_values_;
    std::array<bool, MAX_PACKAGE_ENTRY_COUNT> resource_key_index_active_;
    std::array<PackageId, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> dependency_index_packages_;
    std::array<PackageEntryId, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> dependency_index_dependents_;
    std::array<PackageEntryId, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> dependency_index_dependencies_;
    std::array<std::uint32_t, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> dependency_index_values_;
    std::array<bool, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> dependency_index_active_;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> first_dependency_edge_for_entry_;
    std::array<std::uint32_t, MAX_PACKAGE_ENTRY_COUNT> last_dependency_edge_for_entry_;
    std::array<std::uint32_t, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> next_dependency_edge_;
    PackageSnapshot snapshot_;
};
}
