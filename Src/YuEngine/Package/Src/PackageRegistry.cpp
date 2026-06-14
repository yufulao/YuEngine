#include "YuEngine/Package/PackageRegistry.h"

#include <array>
#include <cstdint>
#include <limits>

#include "YuEngine/Memory/MemoryAccountingStatus.h"

namespace yuengine::package {
namespace {
using memory::MemoryAccountingStatus;

constexpr std::uint32_t INVALID_INDEX = 0xFFFFFFFFU;
constexpr std::uint32_t HASH_OFFSET = 2166136261U;
constexpr std::uint32_t HASH_MULTIPLIER = 16777619U;

std::uint32_t ClampCapacity(std::uint32_t requestedCapacity, std::uint32_t maximumCapacity) {
    if (requestedCapacity > maximumCapacity) {
        return maximumCapacity;
    }

    return requestedCapacity;
}

bool ByteRangeIsWithinBounds(std::uint32_t byteOffset, std::uint32_t byteSize) {
    if (byteSize > MAX_DECLARED_ENTRY_SIZE) {
        return false;
    }

    const std::uint64_t endOffset = static_cast<std::uint64_t>(byteOffset) + static_cast<std::uint64_t>(byteSize);
    return endOffset <= std::numeric_limits<std::uint32_t>::max();
}

std::uint32_t MixHash(std::uint32_t hash, std::uint32_t value) {
    hash ^= value;
    return hash * HASH_MULTIPLIER;
}

std::uint32_t HashLogicalKey(const ResourceLogicalKey& logicalKey) {
    std::uint32_t hash = HASH_OFFSET;
    for (const char character : logicalKey.Value()) {
        hash = MixHash(hash, static_cast<std::uint32_t>(static_cast<unsigned char>(character)));
    }

    return hash;
}

std::uint32_t HashPackageEntry(PackageId package, PackageEntryId entry) {
    std::uint32_t hash = HASH_OFFSET;
    hash = MixHash(hash, package.value);
    return MixHash(hash, entry.value);
}

std::uint32_t HashResourceTuple(PackageId package, ResourceTypeId type, const ResourceLogicalKey& logicalKey) {
    std::uint32_t hash = HASH_OFFSET;
    hash = MixHash(hash, package.value);
    hash = MixHash(hash, type.value);
    return MixHash(hash, HashLogicalKey(logicalKey));
}

std::uint32_t HashResourceKey(PackageId package, const ResourceLogicalKey& logicalKey) {
    std::uint32_t hash = HASH_OFFSET;
    hash = MixHash(hash, package.value);
    return MixHash(hash, HashLogicalKey(logicalKey));
}

std::uint32_t HashDependencyEdge(PackageId package, PackageEntryId dependent, PackageEntryId dependency) {
    std::uint32_t hash = HASH_OFFSET;
    hash = MixHash(hash, package.value);
    hash = MixHash(hash, dependent.value);
    return MixHash(hash, dependency.value);
}

std::uint32_t FirstProbeIndex(std::uint32_t hash, std::uint32_t capacity) {
    if (capacity == 0U) {
        return 0U;
    }

    return hash % capacity;
}

std::uint32_t NextProbeIndex(std::uint32_t index, std::uint32_t capacity) {
    ++index;
    if (index >= capacity) {
        return 0U;
    }

    return index;
}
}

PackageRegistry::PackageRegistry()
    : PackageRegistry(PackageRegistryDesc{}) {
}

PackageRegistry::PackageRegistry(PackageRegistryDesc desc)
    : manifests_{},
      entries_{},
      dependency_edges_{},
      entry_index_packages_{},
      entry_index_entries_{},
      entry_index_values_{},
      entry_index_active_{},
      resource_index_packages_{},
      resource_index_types_{},
      resource_index_keys_{},
      resource_index_values_{},
      resource_index_active_{},
      resource_key_index_packages_{},
      resource_key_index_keys_{},
      resource_key_index_values_{},
      resource_key_index_active_{},
      dependency_index_packages_{},
      dependency_index_dependents_{},
      dependency_index_dependencies_{},
      dependency_index_values_{},
      dependency_index_active_{},
      first_dependency_edge_for_entry_{},
      last_dependency_edge_for_entry_{},
      next_dependency_edge_{},
      snapshot_{
          ClampCapacity(desc.manifest_capacity, MAX_PACKAGE_MANIFEST_COUNT),
          ClampCapacity(desc.entry_capacity, MAX_PACKAGE_ENTRY_COUNT),
          ClampCapacity(desc.dependency_edge_capacity, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT),
          ClampCapacity(desc.load_plan_record_capacity, MAX_LOAD_PLAN_RECORD_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          PackageStatus::Success} {
    first_dependency_edge_for_entry_.fill(INVALID_INDEX);
    last_dependency_edge_for_entry_.fill(INVALID_INDEX);
    next_dependency_edge_.fill(INVALID_INDEX);
}

PackageRegistrationResult PackageRegistry::RegisterSyntheticManifest(const PackageManifestDescriptor& descriptor) {
    if (!descriptor.id.IsValid()) {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::InvalidPackageId));
    }

    if (HasDuplicateManifest(descriptor.id)) {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::DuplicateManifest));
    }

    if (snapshot_.manifest_count >= snapshot_.manifest_capacity) {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::ManifestCapacityExceeded));
    }

    std::uint32_t index = 0U;
    for (ManifestSlot& manifest : manifests_) {
        if (index >= snapshot_.manifest_capacity) {
            return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::ManifestCapacityExceeded));
        }

        if (manifest.is_active) {
            ++index;
            continue;
        }

        manifest.id = descriptor.id;
        manifest.is_active = true;
        ++snapshot_.manifest_count;
        RecordSuccess();
        return PackageRegistrationResult::ManifestSuccess(descriptor.id);
    }

    return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::ManifestCapacityExceeded));
}

PackageRegistrationResult PackageRegistry::RegisterEntry(const PackageEntryDescriptor& descriptor) {
    const PackageStatus validationStatus = ValidateEntryDescriptor(descriptor);
    if (validationStatus != PackageStatus::Success) {
        return PackageRegistrationResult::Failure(RecordFailure(validationStatus));
    }

    if (!HasManifest(descriptor.package)) {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::NotFound));
    }

    if (HasDuplicateEntry(descriptor)) {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::DuplicateEntry));
    }

    if (HasDuplicateResourceKey(descriptor)) {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::DuplicateResourceKey));
    }

    if (snapshot_.entry_count >= snapshot_.entry_capacity) {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::EntryCapacityExceeded));
    }

    std::uint32_t index = 0U;
    for (EntrySlot& entry : entries_) {
        if (index >= snapshot_.entry_capacity) {
            return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::EntryCapacityExceeded));
        }

        if (entry.is_active) {
            ++index;
            continue;
        }

        entry.descriptor = descriptor;
        entry.is_active = true;
        AddEntryIndex(descriptor.package, descriptor.entry, index);
        AddResourceIndex(descriptor.package, descriptor.type, descriptor.logical_key, index);
        AddResourceKeyIndex(descriptor.package, descriptor.logical_key, index);
        ++snapshot_.entry_count;
        RecordSuccess();
        return PackageRegistrationResult::EntrySuccess(descriptor.package, descriptor.entry);
    }

    return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::EntryCapacityExceeded));
}

PackageStatus PackageRegistry::AddDependency(PackageId package, PackageEntryId dependent, PackageEntryId dependency) {
    ++snapshot_.dependency_validation_count;

    if (!package.IsValid()) {
        return RecordFailure(PackageStatus::InvalidPackageId);
    }

    if (!dependent.IsValid() || !dependency.IsValid()) {
        return RecordFailure(PackageStatus::InvalidEntryId);
    }

    std::size_t dependentIndex = 0U;
    const PackageStatus dependentStatus = FindEntryIndex(package, dependent, dependentIndex);
    if (dependentStatus != PackageStatus::Success) {
        return RecordFailure(PackageStatus::DependencyMissing);
    }

    std::size_t dependencyIndex = 0U;
    const PackageStatus dependencyStatus = FindEntryIndex(package, dependency, dependencyIndex);
    if (dependencyStatus != PackageStatus::Success) {
        return RecordFailure(PackageStatus::DependencyMissing);
    }

    if (dependent.value == dependency.value) {
        return RecordFailure(PackageStatus::DependencyCycle);
    }

    if (HasDependencyPath(package, dependency, dependent)) {
        return RecordFailure(PackageStatus::DependencyCycle);
    }

    if (HasDependencyEdge(package, dependent, dependency)) {
        RecordSuccess();
        return PackageStatus::Success;
    }

    if (snapshot_.dependency_edge_count >= snapshot_.dependency_edge_capacity) {
        return RecordFailure(PackageStatus::DependencyCapacityExceeded);
    }

    std::uint32_t index = 0U;
    for (DependencyEdge& edge : dependency_edges_) {
        if (index >= snapshot_.dependency_edge_capacity) {
            return RecordFailure(PackageStatus::DependencyCapacityExceeded);
        }

        if (edge.is_active) {
            ++index;
            continue;
        }

        edge.package = package;
        edge.dependent = dependent;
        edge.dependency = dependency;
        edge.is_active = true;
        AddDependencyIndex(package, dependent, dependency, index);
        AppendDependencyEdge(static_cast<std::uint32_t>(dependentIndex), index);
        ++snapshot_.dependency_edge_count;
        RecordSuccess();
        return PackageStatus::Success;
    }

    return RecordFailure(PackageStatus::DependencyCapacityExceeded);
}

PackageLoadPlanResult PackageRegistry::ResolveEntryByResourceKey(
    PackageId package,
    ResourceTypeId expectedType,
    const ResourceLogicalKey& logicalKey) {
    if (!package.IsValid()) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::InvalidPackageId));
    }

    if (!expectedType.IsValid()) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::InvalidResourceType));
    }

    if (!logicalKey.IsWithinBounds()) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::LogicalKeyTooLong));
    }

    if (!logicalKey.IsValid()) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::InvalidLogicalKey));
    }

    std::size_t rootIndex = 0U;
    const PackageStatus findStatus = FindEntryByResourceKey(package, expectedType, logicalKey, rootIndex);
    if (findStatus != PackageStatus::Success) {
        return PackageLoadPlanResult::Failure(RecordFailure(findStatus));
    }

    const PackageEntryDescriptor& rootDescriptor = entries_[rootIndex].descriptor;
    if (rootDescriptor.type.value != expectedType.value) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::TypeMismatch));
    }

    const std::uint32_t directDependencyCount = CountDirectDependencies(package, rootDescriptor.entry);
    const std::uint32_t requiredRecordCount = directDependencyCount + 1U;
    if (requiredRecordCount > snapshot_.load_plan_record_capacity) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::LoadPlanCapacityExceeded));
    }

    PackageLoadPlan plan{};
    std::uint32_t edgeIndex = first_dependency_edge_for_entry_[rootIndex];
    while (edgeIndex != INVALID_INDEX) {
        const DependencyEdge& edge = dependency_edges_[edgeIndex];
        const std::uint32_t nextEdgeIndex = next_dependency_edge_[edgeIndex];

        std::size_t dependencyIndex = 0U;
        if (FindEntryIndex(package, edge.dependency, dependencyIndex) != PackageStatus::Success) {
            return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::DependencyMissing));
        }

        AppendRecord(plan, entries_[dependencyIndex].descriptor);
        edgeIndex = nextEdgeIndex;
    }

    AppendRecord(plan, rootDescriptor);
    ++snapshot_.load_plan_resolve_count;
    snapshot_.last_load_plan_record_count = plan.record_count;
    RecordSuccess();
    return PackageLoadPlanResult::Success(plan);
}

PackageSnapshot PackageRegistry::Snapshot() const {
    return snapshot_;
}

PackageStatus PackageRegistry::RecordFailure(PackageStatus status) {
    ++snapshot_.rejected_operation_count;
    snapshot_.last_status = status;
    return status;
}

void PackageRegistry::RecordSuccess() {
    snapshot_.last_status = PackageStatus::Success;
}

bool PackageRegistry::HasManifest(PackageId package) const {
    return HasDuplicateManifest(package);
}

bool PackageRegistry::HasDuplicateManifest(PackageId package) const {
    std::uint32_t index = 0U;
    for (const ManifestSlot& manifest : manifests_) {
        if (index >= snapshot_.manifest_capacity) {
            return false;
        }

        if (!manifest.is_active) {
            ++index;
            continue;
        }

        if (manifest.id.value == package.value) {
            return true;
        }

        ++index;
    }

    return false;
}

bool PackageRegistry::HasDuplicateEntry(const PackageEntryDescriptor& descriptor) const {
    std::size_t index = 0U;
    return TryFindEntryIndex(descriptor.package, descriptor.entry, index);
}

bool PackageRegistry::HasDuplicateResourceKey(const PackageEntryDescriptor& descriptor) const {
    std::size_t index = 0U;
    return TryFindResourceIndex(descriptor.package, descriptor.type, descriptor.logical_key, index);
}

PackageStatus PackageRegistry::ValidateEntryDescriptor(const PackageEntryDescriptor& descriptor) const {
    if (!descriptor.package.IsValid()) {
        return PackageStatus::InvalidPackageId;
    }

    if (!descriptor.entry.IsValid()) {
        return PackageStatus::InvalidEntryId;
    }

    if (!descriptor.type.IsValid()) {
        return PackageStatus::InvalidResourceType;
    }

    if (!descriptor.logical_key.IsWithinBounds()) {
        return PackageStatus::LogicalKeyTooLong;
    }

    if (!descriptor.logical_key.IsValid()) {
        return PackageStatus::InvalidLogicalKey;
    }

    if (!descriptor.source_key.IsWithinBounds()) {
        return PackageStatus::SourceKeyTooLong;
    }

    if (!descriptor.source_key.IsValid()) {
        return PackageStatus::InvalidSourceKey;
    }

    if (!ByteRangeIsWithinBounds(descriptor.byte_offset, descriptor.byte_size)) {
        return PackageStatus::ByteRangeOutOfBounds;
    }

    return PackageStatus::Success;
}

PackageStatus PackageRegistry::FindEntryIndex(PackageId package, PackageEntryId entry, std::size_t& outIndex) const {
    if (!package.IsValid()) {
        return PackageStatus::InvalidPackageId;
    }

    if (!entry.IsValid()) {
        return PackageStatus::InvalidEntryId;
    }

    if (TryFindEntryIndex(package, entry, outIndex)) {
        return PackageStatus::Success;
    }

    return PackageStatus::NotFound;
}

PackageStatus PackageRegistry::FindEntryByResourceKey(
    PackageId package,
    ResourceTypeId expectedType,
    const ResourceLogicalKey& logicalKey,
    std::size_t& outIndex) const {
    if (!HasManifest(package)) {
        return PackageStatus::NotFound;
    }

    if (TryFindResourceIndex(package, expectedType, logicalKey, outIndex)) {
        return PackageStatus::Success;
    }

    if (HasResourceLogicalKey(package, logicalKey)) {
        return PackageStatus::TypeMismatch;
    }

    return PackageStatus::NotFound;
}

bool PackageRegistry::HasDependencyEdge(PackageId package, PackageEntryId dependent, PackageEntryId dependency) const {
    std::size_t index = 0U;
    return TryFindDependencyEdgeIndex(package, dependent, dependency, index);
}

bool PackageRegistry::HasDependencyPath(PackageId package, PackageEntryId start, PackageEntryId target) const {
    std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT> stack{};
    std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT> visited{};
    std::uint32_t stackCount = 0U;
    std::uint32_t visitedCount = 0U;
    const auto containsEntry = [](const std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT>& entries,
                                  std::uint32_t count,
                                  PackageEntryId entry) {
        for (std::uint32_t index = 0U; index < count; ++index) {
            if (entries[index].value == entry.value) {
                return true;
            }
        }

        return false;
    };

    stack[stackCount] = start;
    ++stackCount;

    while (stackCount > 0U) {
        --stackCount;
        const PackageEntryId current = stack[stackCount];
        if (current.value == target.value) {
            return true;
        }

        if (containsEntry(visited, visitedCount, current)) {
            continue;
        }

        if (visitedCount < MAX_PACKAGE_ENTRY_COUNT) {
            visited[visitedCount] = current;
            ++visitedCount;
        }

        std::size_t currentIndex = 0U;
        if (FindEntryIndex(package, current, currentIndex) != PackageStatus::Success) {
            continue;
        }

        std::uint32_t edgeIndex = first_dependency_edge_for_entry_[currentIndex];
        while (edgeIndex != INVALID_INDEX) {
            const DependencyEdge& edge = dependency_edges_[edgeIndex];
            const std::uint32_t nextEdgeIndex = next_dependency_edge_[edgeIndex];

            if (!containsEntry(visited, visitedCount, edge.dependency) &&
                !containsEntry(stack, stackCount, edge.dependency) &&
                stackCount < MAX_PACKAGE_ENTRY_COUNT) {
                stack[stackCount] = edge.dependency;
                ++stackCount;
            }

            edgeIndex = nextEdgeIndex;
        }
    }

    return false;
}

std::uint32_t PackageRegistry::CountDirectDependencies(PackageId package, PackageEntryId entry) const {
    std::size_t entryIndex = 0U;
    if (FindEntryIndex(package, entry, entryIndex) != PackageStatus::Success) {
        return 0U;
    }

    std::uint32_t count = 0U;
    std::uint32_t edgeIndex = first_dependency_edge_for_entry_[entryIndex];
    while (edgeIndex != INVALID_INDEX) {
        ++count;
        edgeIndex = next_dependency_edge_[edgeIndex];
    }

    return count;
}

void PackageRegistry::AppendRecord(PackageLoadPlan& plan, const PackageEntryDescriptor& descriptor) const {
    plan.records[plan.record_count] = PackageLoadPlanRecord{
        descriptor.package,
        descriptor.entry,
        descriptor.type,
        descriptor.logical_key,
        descriptor.source_key,
        descriptor.byte_offset,
        descriptor.byte_size};
    ++plan.record_count;
}

bool PackageRegistry::TryFindEntryIndex(PackageId package, PackageEntryId entry, std::size_t& outIndex) const {
    std::uint32_t probeCount = 0U;
    std::uint32_t probeIndex = FirstProbeIndex(HashPackageEntry(package, entry), snapshot_.entry_capacity);
    while (probeCount < snapshot_.entry_capacity) {
        if (!entry_index_active_[probeIndex]) {
            return false;
        }

        if (entry_index_packages_[probeIndex].value == package.value &&
            entry_index_entries_[probeIndex].value == entry.value) {
            outIndex = entry_index_values_[probeIndex];
            return true;
        }

        probeIndex = NextProbeIndex(probeIndex, snapshot_.entry_capacity);
        ++probeCount;
    }

    return false;
}

bool PackageRegistry::TryFindResourceIndex(
    PackageId package,
    ResourceTypeId expectedType,
    const ResourceLogicalKey& logicalKey,
    std::size_t& outIndex) const {
    std::uint32_t probeCount = 0U;
    std::uint32_t probeIndex =
        FirstProbeIndex(HashResourceTuple(package, expectedType, logicalKey), snapshot_.entry_capacity);
    while (probeCount < snapshot_.entry_capacity) {
        if (!resource_index_active_[probeIndex]) {
            return false;
        }

        if (resource_index_packages_[probeIndex].value == package.value &&
            resource_index_types_[probeIndex].value == expectedType.value &&
            resource_index_keys_[probeIndex].Equals(logicalKey)) {
            outIndex = resource_index_values_[probeIndex];
            return true;
        }

        probeIndex = NextProbeIndex(probeIndex, snapshot_.entry_capacity);
        ++probeCount;
    }

    return false;
}

bool PackageRegistry::HasResourceLogicalKey(PackageId package, const ResourceLogicalKey& logicalKey) const {
    std::uint32_t probeCount = 0U;
    std::uint32_t probeIndex = FirstProbeIndex(HashResourceKey(package, logicalKey), snapshot_.entry_capacity);
    while (probeCount < snapshot_.entry_capacity) {
        if (!resource_key_index_active_[probeIndex]) {
            return false;
        }

        if (resource_key_index_packages_[probeIndex].value == package.value &&
            resource_key_index_keys_[probeIndex].Equals(logicalKey)) {
            return true;
        }

        probeIndex = NextProbeIndex(probeIndex, snapshot_.entry_capacity);
        ++probeCount;
    }

    return false;
}

bool PackageRegistry::TryFindDependencyEdgeIndex(
    PackageId package,
    PackageEntryId dependent,
    PackageEntryId dependency,
    std::size_t& outIndex) const {
    std::uint32_t probeCount = 0U;
    std::uint32_t probeIndex =
        FirstProbeIndex(HashDependencyEdge(package, dependent, dependency), snapshot_.dependency_edge_capacity);
    while (probeCount < snapshot_.dependency_edge_capacity) {
        if (!dependency_index_active_[probeIndex]) {
            return false;
        }

        if (dependency_index_packages_[probeIndex].value == package.value &&
            dependency_index_dependents_[probeIndex].value == dependent.value &&
            dependency_index_dependencies_[probeIndex].value == dependency.value) {
            outIndex = dependency_index_values_[probeIndex];
            return true;
        }

        probeIndex = NextProbeIndex(probeIndex, snapshot_.dependency_edge_capacity);
        ++probeCount;
    }

    return false;
}

void PackageRegistry::AddEntryIndex(PackageId package, PackageEntryId entry, std::uint32_t entryIndex) {
    std::uint32_t probeCount = 0U;
    std::uint32_t probeIndex = FirstProbeIndex(HashPackageEntry(package, entry), snapshot_.entry_capacity);
    while (probeCount < snapshot_.entry_capacity) {
        if (!entry_index_active_[probeIndex]) {
            entry_index_packages_[probeIndex] = package;
            entry_index_entries_[probeIndex] = entry;
            entry_index_values_[probeIndex] = entryIndex;
            entry_index_active_[probeIndex] = true;
            return;
        }

        if (entry_index_packages_[probeIndex].value == package.value &&
            entry_index_entries_[probeIndex].value == entry.value) {
            entry_index_values_[probeIndex] = entryIndex;
            return;
        }

        probeIndex = NextProbeIndex(probeIndex, snapshot_.entry_capacity);
        ++probeCount;
    }
}

void PackageRegistry::AddResourceIndex(
    PackageId package,
    ResourceTypeId type,
    const ResourceLogicalKey& logicalKey,
    std::uint32_t entryIndex) {
    std::uint32_t probeCount = 0U;
    std::uint32_t probeIndex =
        FirstProbeIndex(HashResourceTuple(package, type, logicalKey), snapshot_.entry_capacity);
    while (probeCount < snapshot_.entry_capacity) {
        if (!resource_index_active_[probeIndex]) {
            resource_index_packages_[probeIndex] = package;
            resource_index_types_[probeIndex] = type;
            resource_index_keys_[probeIndex] = logicalKey;
            resource_index_values_[probeIndex] = entryIndex;
            resource_index_active_[probeIndex] = true;
            return;
        }

        if (resource_index_packages_[probeIndex].value == package.value &&
            resource_index_types_[probeIndex].value == type.value &&
            resource_index_keys_[probeIndex].Equals(logicalKey)) {
            resource_index_values_[probeIndex] = entryIndex;
            return;
        }

        probeIndex = NextProbeIndex(probeIndex, snapshot_.entry_capacity);
        ++probeCount;
    }
}

void PackageRegistry::AddResourceKeyIndex(
    PackageId package,
    const ResourceLogicalKey& logicalKey,
    std::uint32_t entryIndex) {
    std::uint32_t probeCount = 0U;
    std::uint32_t probeIndex = FirstProbeIndex(HashResourceKey(package, logicalKey), snapshot_.entry_capacity);
    while (probeCount < snapshot_.entry_capacity) {
        if (!resource_key_index_active_[probeIndex]) {
            resource_key_index_packages_[probeIndex] = package;
            resource_key_index_keys_[probeIndex] = logicalKey;
            resource_key_index_values_[probeIndex] = entryIndex;
            resource_key_index_active_[probeIndex] = true;
            return;
        }

        if (resource_key_index_packages_[probeIndex].value == package.value &&
            resource_key_index_keys_[probeIndex].Equals(logicalKey)) {
            resource_key_index_values_[probeIndex] = entryIndex;
            return;
        }

        probeIndex = NextProbeIndex(probeIndex, snapshot_.entry_capacity);
        ++probeCount;
    }
}

void PackageRegistry::AddDependencyIndex(
    PackageId package,
    PackageEntryId dependent,
    PackageEntryId dependency,
    std::uint32_t edgeIndex) {
    std::uint32_t probeCount = 0U;
    std::uint32_t probeIndex =
        FirstProbeIndex(HashDependencyEdge(package, dependent, dependency), snapshot_.dependency_edge_capacity);
    while (probeCount < snapshot_.dependency_edge_capacity) {
        if (!dependency_index_active_[probeIndex]) {
            dependency_index_packages_[probeIndex] = package;
            dependency_index_dependents_[probeIndex] = dependent;
            dependency_index_dependencies_[probeIndex] = dependency;
            dependency_index_values_[probeIndex] = edgeIndex;
            dependency_index_active_[probeIndex] = true;
            return;
        }

        if (dependency_index_packages_[probeIndex].value == package.value &&
            dependency_index_dependents_[probeIndex].value == dependent.value &&
            dependency_index_dependencies_[probeIndex].value == dependency.value) {
            dependency_index_values_[probeIndex] = edgeIndex;
            return;
        }

        probeIndex = NextProbeIndex(probeIndex, snapshot_.dependency_edge_capacity);
        ++probeCount;
    }
}

void PackageRegistry::AppendDependencyEdge(std::uint32_t dependentEntryIndex, std::uint32_t edgeIndex) {
    if (dependentEntryIndex >= snapshot_.entry_capacity) {
        return;
    }

    if (edgeIndex >= snapshot_.dependency_edge_capacity) {
        return;
    }

    if (first_dependency_edge_for_entry_[dependentEntryIndex] == INVALID_INDEX) {
        first_dependency_edge_for_entry_[dependentEntryIndex] = edgeIndex;
        last_dependency_edge_for_entry_[dependentEntryIndex] = edgeIndex;
        return;
    }

    const std::uint32_t lastEdgeIndex = last_dependency_edge_for_entry_[dependentEntryIndex];
    next_dependency_edge_[lastEdgeIndex] = edgeIndex;
    last_dependency_edge_for_entry_[dependentEntryIndex] = edgeIndex;
}
}
