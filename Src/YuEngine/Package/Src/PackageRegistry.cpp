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
    : _manifests{},
      _entries{},
      _dependencyEdges{},
      _entryIndexPackages{},
      _entryIndexEntries{},
      _entryIndexValues{},
      _entryIndexActive{},
      _resourceIndexPackages{},
      _resourceIndexTypes{},
      _resourceIndexKeys{},
      _resourceIndexValues{},
      _resourceIndexActive{},
      _resourceKeyIndexPackages{},
      _resourceKeyIndexKeys{},
      _resourceKeyIndexValues{},
      _resourceKeyIndexActive{},
      _dependencyIndexPackages{},
      _dependencyIndexDependents{},
      _dependencyIndexDependencies{},
      _dependencyIndexValues{},
      _dependencyIndexActive{},
      _firstDependencyEdgeForEntry{},
      _lastDependencyEdgeForEntry{},
      _nextDependencyEdge{},
      _snapshot{
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
    _firstDependencyEdgeForEntry.fill(INVALID_INDEX);
    _lastDependencyEdgeForEntry.fill(INVALID_INDEX);
    _nextDependencyEdge.fill(INVALID_INDEX);
}

PackageRegistrationResult PackageRegistry::RegisterSyntheticManifest(const PackageManifestDescriptor& descriptor) {
    if (!descriptor.id.IsValid()) {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::InvalidPackageId));
    }

    if (HasDuplicateManifest(descriptor.id)) {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::DuplicateManifest));
    }

    if (_snapshot.manifest_count >= _snapshot.manifest_capacity) {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::ManifestCapacityExceeded));
    }

    std::uint32_t index = 0U;
    for (ManifestSlot& manifest : _manifests) {
        if (index >= _snapshot.manifest_capacity) {
            return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::ManifestCapacityExceeded));
        }

        if (manifest.is_active) {
            ++index;
            continue;
        }

        manifest.id = descriptor.id;
        manifest.is_active = true;
        ++_snapshot.manifest_count;
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

    if (_snapshot.entry_count >= _snapshot.entry_capacity) {
        return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::EntryCapacityExceeded));
    }

    std::uint32_t index = 0U;
    for (EntrySlot& entry : _entries) {
        if (index >= _snapshot.entry_capacity) {
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
        ++_snapshot.entry_count;
        RecordSuccess();
        return PackageRegistrationResult::EntrySuccess(descriptor.package, descriptor.entry);
    }

    return PackageRegistrationResult::Failure(RecordFailure(PackageStatus::EntryCapacityExceeded));
}

PackageStatus PackageRegistry::AddDependency(PackageId package, PackageEntryId dependent, PackageEntryId dependency) {
    ++_snapshot.dependency_validation_count;

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

    if (_snapshot.dependency_edge_count >= _snapshot.dependency_edge_capacity) {
        return RecordFailure(PackageStatus::DependencyCapacityExceeded);
    }

    std::uint32_t index = 0U;
    for (DependencyEdge& edge : _dependencyEdges) {
        if (index >= _snapshot.dependency_edge_capacity) {
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
        ++_snapshot.dependency_edge_count;
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

    const PackageEntryDescriptor& rootDescriptor = _entries[rootIndex].descriptor;
    if (rootDescriptor.type.value != expectedType.value) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::TypeMismatch));
    }

    const std::uint32_t directDependencyCount = CountDirectDependencies(package, rootDescriptor.entry);
    const std::uint32_t requiredRecordCount = directDependencyCount + 1U;
    if (requiredRecordCount > _snapshot.load_plan_record_capacity) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::LoadPlanCapacityExceeded));
    }

    PackageLoadPlan plan{};
    std::uint32_t edgeIndex = _firstDependencyEdgeForEntry[rootIndex];
    while (edgeIndex != INVALID_INDEX) {
        const DependencyEdge& edge = _dependencyEdges[edgeIndex];
        const std::uint32_t nextEdgeIndex = _nextDependencyEdge[edgeIndex];

        std::size_t dependencyIndex = 0U;
        if (FindEntryIndex(package, edge.dependency, dependencyIndex) != PackageStatus::Success) {
            return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::DependencyMissing));
        }

        AppendRecord(plan, _entries[dependencyIndex].descriptor);
        edgeIndex = nextEdgeIndex;
    }

    AppendRecord(plan, rootDescriptor);
    ++_snapshot.load_plan_resolve_count;
    _snapshot.last_load_plan_record_count = plan.record_count;
    RecordSuccess();
    return PackageLoadPlanResult::Success(plan);
}

PackageSnapshot PackageRegistry::Snapshot() const {
    return _snapshot;
}

PackageStatus PackageRegistry::RecordFailure(PackageStatus status) {
    ++_snapshot.rejected_operation_count;
    _snapshot.last_status = status;
    return status;
}

void PackageRegistry::RecordSuccess() {
    _snapshot.last_status = PackageStatus::Success;
}

bool PackageRegistry::HasManifest(PackageId package) const {
    return HasDuplicateManifest(package);
}

bool PackageRegistry::HasDuplicateManifest(PackageId package) const {
    std::uint32_t index = 0U;
    for (const ManifestSlot& manifest : _manifests) {
        if (index >= _snapshot.manifest_capacity) {
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

        std::uint32_t edgeIndex = _firstDependencyEdgeForEntry[currentIndex];
        while (edgeIndex != INVALID_INDEX) {
            const DependencyEdge& edge = _dependencyEdges[edgeIndex];
            const std::uint32_t nextEdgeIndex = _nextDependencyEdge[edgeIndex];

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
    std::uint32_t edgeIndex = _firstDependencyEdgeForEntry[entryIndex];
    while (edgeIndex != INVALID_INDEX) {
        ++count;
        edgeIndex = _nextDependencyEdge[edgeIndex];
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
    std::uint32_t probeIndex = FirstProbeIndex(HashPackageEntry(package, entry), _snapshot.entry_capacity);
    while (probeCount < _snapshot.entry_capacity) {
        if (!_entryIndexActive[probeIndex]) {
            return false;
        }

        if (_entryIndexPackages[probeIndex].value == package.value &&
            _entryIndexEntries[probeIndex].value == entry.value) {
            outIndex = _entryIndexValues[probeIndex];
            return true;
        }

        probeIndex = NextProbeIndex(probeIndex, _snapshot.entry_capacity);
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
        FirstProbeIndex(HashResourceTuple(package, expectedType, logicalKey), _snapshot.entry_capacity);
    while (probeCount < _snapshot.entry_capacity) {
        if (!_resourceIndexActive[probeIndex]) {
            return false;
        }

        if (_resourceIndexPackages[probeIndex].value == package.value &&
            _resourceIndexTypes[probeIndex].value == expectedType.value &&
            _resourceIndexKeys[probeIndex].Equals(logicalKey)) {
            outIndex = _resourceIndexValues[probeIndex];
            return true;
        }

        probeIndex = NextProbeIndex(probeIndex, _snapshot.entry_capacity);
        ++probeCount;
    }

    return false;
}

bool PackageRegistry::HasResourceLogicalKey(PackageId package, const ResourceLogicalKey& logicalKey) const {
    std::uint32_t probeCount = 0U;
    std::uint32_t probeIndex = FirstProbeIndex(HashResourceKey(package, logicalKey), _snapshot.entry_capacity);
    while (probeCount < _snapshot.entry_capacity) {
        if (!_resourceKeyIndexActive[probeIndex]) {
            return false;
        }

        if (_resourceKeyIndexPackages[probeIndex].value == package.value &&
            _resourceKeyIndexKeys[probeIndex].Equals(logicalKey)) {
            return true;
        }

        probeIndex = NextProbeIndex(probeIndex, _snapshot.entry_capacity);
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
        FirstProbeIndex(HashDependencyEdge(package, dependent, dependency), _snapshot.dependency_edge_capacity);
    while (probeCount < _snapshot.dependency_edge_capacity) {
        if (!_dependencyIndexActive[probeIndex]) {
            return false;
        }

        if (_dependencyIndexPackages[probeIndex].value == package.value &&
            _dependencyIndexDependents[probeIndex].value == dependent.value &&
            _dependencyIndexDependencies[probeIndex].value == dependency.value) {
            outIndex = _dependencyIndexValues[probeIndex];
            return true;
        }

        probeIndex = NextProbeIndex(probeIndex, _snapshot.dependency_edge_capacity);
        ++probeCount;
    }

    return false;
}

void PackageRegistry::AddEntryIndex(PackageId package, PackageEntryId entry, std::uint32_t entryIndex) {
    std::uint32_t probeCount = 0U;
    std::uint32_t probeIndex = FirstProbeIndex(HashPackageEntry(package, entry), _snapshot.entry_capacity);
    while (probeCount < _snapshot.entry_capacity) {
        if (!_entryIndexActive[probeIndex]) {
            _entryIndexPackages[probeIndex] = package;
            _entryIndexEntries[probeIndex] = entry;
            _entryIndexValues[probeIndex] = entryIndex;
            _entryIndexActive[probeIndex] = true;
            return;
        }

        if (_entryIndexPackages[probeIndex].value == package.value &&
            _entryIndexEntries[probeIndex].value == entry.value) {
            _entryIndexValues[probeIndex] = entryIndex;
            return;
        }

        probeIndex = NextProbeIndex(probeIndex, _snapshot.entry_capacity);
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
        FirstProbeIndex(HashResourceTuple(package, type, logicalKey), _snapshot.entry_capacity);
    while (probeCount < _snapshot.entry_capacity) {
        if (!_resourceIndexActive[probeIndex]) {
            _resourceIndexPackages[probeIndex] = package;
            _resourceIndexTypes[probeIndex] = type;
            _resourceIndexKeys[probeIndex] = logicalKey;
            _resourceIndexValues[probeIndex] = entryIndex;
            _resourceIndexActive[probeIndex] = true;
            return;
        }

        if (_resourceIndexPackages[probeIndex].value == package.value &&
            _resourceIndexTypes[probeIndex].value == type.value &&
            _resourceIndexKeys[probeIndex].Equals(logicalKey)) {
            _resourceIndexValues[probeIndex] = entryIndex;
            return;
        }

        probeIndex = NextProbeIndex(probeIndex, _snapshot.entry_capacity);
        ++probeCount;
    }
}

void PackageRegistry::AddResourceKeyIndex(
    PackageId package,
    const ResourceLogicalKey& logicalKey,
    std::uint32_t entryIndex) {
    std::uint32_t probeCount = 0U;
    std::uint32_t probeIndex = FirstProbeIndex(HashResourceKey(package, logicalKey), _snapshot.entry_capacity);
    while (probeCount < _snapshot.entry_capacity) {
        if (!_resourceKeyIndexActive[probeIndex]) {
            _resourceKeyIndexPackages[probeIndex] = package;
            _resourceKeyIndexKeys[probeIndex] = logicalKey;
            _resourceKeyIndexValues[probeIndex] = entryIndex;
            _resourceKeyIndexActive[probeIndex] = true;
            return;
        }

        if (_resourceKeyIndexPackages[probeIndex].value == package.value &&
            _resourceKeyIndexKeys[probeIndex].Equals(logicalKey)) {
            _resourceKeyIndexValues[probeIndex] = entryIndex;
            return;
        }

        probeIndex = NextProbeIndex(probeIndex, _snapshot.entry_capacity);
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
        FirstProbeIndex(HashDependencyEdge(package, dependent, dependency), _snapshot.dependency_edge_capacity);
    while (probeCount < _snapshot.dependency_edge_capacity) {
        if (!_dependencyIndexActive[probeIndex]) {
            _dependencyIndexPackages[probeIndex] = package;
            _dependencyIndexDependents[probeIndex] = dependent;
            _dependencyIndexDependencies[probeIndex] = dependency;
            _dependencyIndexValues[probeIndex] = edgeIndex;
            _dependencyIndexActive[probeIndex] = true;
            return;
        }

        if (_dependencyIndexPackages[probeIndex].value == package.value &&
            _dependencyIndexDependents[probeIndex].value == dependent.value &&
            _dependencyIndexDependencies[probeIndex].value == dependency.value) {
            _dependencyIndexValues[probeIndex] = edgeIndex;
            return;
        }

        probeIndex = NextProbeIndex(probeIndex, _snapshot.dependency_edge_capacity);
        ++probeCount;
    }
}

void PackageRegistry::AppendDependencyEdge(std::uint32_t dependentEntryIndex, std::uint32_t edgeIndex) {
    if (dependentEntryIndex >= _snapshot.entry_capacity) {
        return;
    }

    if (edgeIndex >= _snapshot.dependency_edge_capacity) {
        return;
    }

    if (_firstDependencyEdgeForEntry[dependentEntryIndex] == INVALID_INDEX) {
        _firstDependencyEdgeForEntry[dependentEntryIndex] = edgeIndex;
        _lastDependencyEdgeForEntry[dependentEntryIndex] = edgeIndex;
        return;
    }

    const std::uint32_t lastEdgeIndex = _lastDependencyEdgeForEntry[dependentEntryIndex];
    _nextDependencyEdge[lastEdgeIndex] = edgeIndex;
    _lastDependencyEdgeForEntry[dependentEntryIndex] = edgeIndex;
}
}
