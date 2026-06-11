#pragma once

#include <array>
#include <cstddef>

#include "yuengine/package/PackageConstants.h"
#include "yuengine/package/PackageEntryDescriptor.h"
#include "yuengine/package/PackageLoadPlanResult.h"
#include "yuengine/package/PackageManifestDescriptor.h"
#include "yuengine/package/PackageRegistrationResult.h"
#include "yuengine/package/PackageRegistryDesc.h"
#include "yuengine/package/PackageSnapshot.h"

namespace yuengine::package
{
class PackageRegistry final
{
public:
    PackageRegistry();
    explicit PackageRegistry(PackageRegistryDesc desc);

    PackageRegistrationResult RegisterSyntheticManifest(const PackageManifestDescriptor& descriptor);
    PackageRegistrationResult RegisterEntry(const PackageEntryDescriptor& descriptor);
    PackageStatus AddDependency(PackageId package, PackageEntryId dependent, PackageEntryId dependency);
    PackageLoadPlanResult ResolveEntryByResourceKey(
        PackageId package,
        yuengine::resource::ResourceTypeId expectedType,
        const yuengine::resource::ResourceLogicalKey& logicalKey);
    PackageSnapshot Snapshot() const;

private:
    struct ManifestSlot final
    {
        PackageId Id;
        bool IsActive = false;
    };

    struct EntrySlot final
    {
        PackageEntryDescriptor Descriptor;
        bool IsActive = false;
    };

    struct DependencyEdge final
    {
        PackageId Package;
        PackageEntryId Dependent;
        PackageEntryId Dependency;
        bool IsActive = false;
    };

    PackageStatus RecordFailure(PackageStatus status);
    void RecordSuccess();
    bool HasManifest(PackageId package) const;
    bool HasDuplicateManifest(PackageId package) const;
    bool HasDuplicateEntry(const PackageEntryDescriptor& descriptor) const;
    bool HasDuplicateResourceKey(const PackageEntryDescriptor& descriptor) const;
    PackageStatus ValidateEntryDescriptor(const PackageEntryDescriptor& descriptor) const;
    PackageStatus FindEntryIndex(PackageId package, PackageEntryId entry, std::size_t& outIndex) const;
    PackageStatus FindEntryByResourceKey(
        PackageId package,
        yuengine::resource::ResourceTypeId expectedType,
        const yuengine::resource::ResourceLogicalKey& logicalKey,
        std::size_t& outIndex) const;
    bool HasDependencyEdge(PackageId package, PackageEntryId dependent, PackageEntryId dependency) const;
    bool HasDependencyPath(PackageId package, PackageEntryId start, PackageEntryId target) const;
    std::uint32_t CountDirectDependencies(PackageId package, PackageEntryId entry) const;
    void AppendRecord(PackageLoadPlan& plan, const PackageEntryDescriptor& descriptor) const;

    std::array<ManifestSlot, MAX_PACKAGE_MANIFEST_COUNT> _manifests;
    std::array<EntrySlot, MAX_PACKAGE_ENTRY_COUNT> _entries;
    std::array<DependencyEdge, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> _dependencyEdges;
    PackageSnapshot _snapshot;
};
}
