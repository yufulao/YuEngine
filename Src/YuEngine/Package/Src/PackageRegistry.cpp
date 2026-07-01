// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Src/PackageRegistry.cpp

#include "YuEngine/Package/PackageRegistry.h"

#include <array>
#include <cstdint>
#include <limits>
#include <string_view>

#include "YuEngine/Memory/MemoryAccountingStatus.h"

namespace yuengine::package {
namespace {
using memory::MemoryAccountingStatus;

constexpr std::uint32_t INVALID_INDEX = 0xFFFFFFFFU;
constexpr std::uint32_t HASH_OFFSET = 2166136261U;
constexpr std::uint32_t HASH_MULTIPLIER = 16777619U;
constexpr std::uint64_t PAYLOAD_HASH_OFFSET = 14695981039346656037ULL;
constexpr std::uint64_t PAYLOAD_HASH_MULTIPLIER = 1099511628211ULL;

std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}

std::uint64_t ClampArchiveByteBudget(std::uint64_t requested_budget) {
    if (requested_budget > MAX_LOAD_PLAN_ARCHIVE_BYTE_BUDGET) {
        return MAX_LOAD_PLAN_ARCHIVE_BYTE_BUDGET;
    }

    return requested_budget;
}

bool ByteRangeIsWithinBounds(std::uint64_t byte_offset, std::uint64_t byte_size) {
    if (byte_size == 0ULL) {
        return false;
    }

    if (byte_size > MAX_DECLARED_ENTRY_SIZE) {
        return false;
    }

    const std::uint64_t max_offset = std::numeric_limits<std::uint64_t>::max();
    return byte_offset <= max_offset - byte_size;
}

std::uint64_t GetArchiveByteOffset(const PackageEntryDescriptor& descriptor) {
    if (descriptor.archive_byte_size > 0ULL) {
        return descriptor.archive_byte_offset;
    }

    return descriptor.byte_offset;
}

std::uint64_t GetArchiveByteSize(const PackageEntryDescriptor& descriptor) {
    if (descriptor.archive_byte_size > 0ULL) {
        return descriptor.archive_byte_size;
    }

    return descriptor.byte_size;
}

bool HasExplicitPayloadMetadata(const PackageEntryDescriptor& descriptor) {
    if (descriptor.payload_logical_byte_count != 0ULL) {
        return true;
    }

    if (descriptor.payload_window_byte_offset != 0ULL) {
        return true;
    }

    return descriptor.payload_window_byte_size != 0ULL;
}

std::uint64_t GetPayloadLogicalByteCount(const PackageEntryDescriptor& descriptor) {
    if (HasExplicitPayloadMetadata(descriptor)) {
        return descriptor.payload_logical_byte_count;
    }

    return GetArchiveByteSize(descriptor);
}

std::uint64_t GetPayloadWindowByteOffset(const PackageEntryDescriptor& descriptor) {
    if (HasExplicitPayloadMetadata(descriptor)) {
        return descriptor.payload_window_byte_offset;
    }

    return 0ULL;
}

std::uint64_t GetPayloadWindowByteSize(const PackageEntryDescriptor& descriptor) {
    if (HasExplicitPayloadMetadata(descriptor)) {
        return descriptor.payload_window_byte_size;
    }

    return GetArchiveByteSize(descriptor);
}

bool PayloadWindowIsWithinBounds(
    std::uint64_t payload_logical_byte_count,
    std::uint64_t payload_window_byte_offset,
    std::uint64_t payload_window_byte_size) {
    if (payload_logical_byte_count == 0ULL) {
        return false;
    }

    if (payload_window_byte_size == 0ULL) {
        return false;
    }

    const std::uint64_t max_offset = std::numeric_limits<std::uint64_t>::max();
    if (payload_window_byte_offset > max_offset - payload_window_byte_size) {
        return false;
    }

    const std::uint64_t payload_window_byte_end = payload_window_byte_offset + payload_window_byte_size;
    return payload_window_byte_end <= payload_logical_byte_count;
}

bool PayloadMetadataMatchesArchiveRange(const PackageEntryDescriptor& descriptor) {
    const std::uint64_t archive_byte_size = GetArchiveByteSize(descriptor);
    const std::uint64_t payload_logical_byte_count = GetPayloadLogicalByteCount(descriptor);
    const std::uint64_t payload_window_byte_offset = GetPayloadWindowByteOffset(descriptor);
    const std::uint64_t payload_window_byte_size = GetPayloadWindowByteSize(descriptor);
    if (payload_window_byte_size != archive_byte_size) {
        return false;
    }

    return PayloadWindowIsWithinBounds(
        payload_logical_byte_count,
        payload_window_byte_offset,
        payload_window_byte_size);
}

std::uint32_t MakeLegacyByteValue(std::uint64_t value) {
    const std::uint64_t max_legacy_value = std::numeric_limits<std::uint32_t>::max();
    if (value > max_legacy_value) {
        return 0U;
    }

    return static_cast<std::uint32_t>(value);
}

std::uint64_t MixPayloadHash(std::uint64_t hash, std::uint64_t value) {
    hash ^= value;
    return hash * PAYLOAD_HASH_MULTIPLIER;
}

std::uint64_t HashPayloadText(std::uint64_t hash, std::string_view text) {
    for (const char character : text) {
        hash = MixPayloadHash(hash, static_cast<std::uint64_t>(static_cast<unsigned char>(character)));
    }

    return hash;
}

std::uint64_t MakeEntryPayloadHash(const PackageEntryDescriptor& descriptor) {
    std::uint64_t hash = PAYLOAD_HASH_OFFSET;
    hash = HashPayloadText(hash, descriptor.source_key.Value());
    hash = MixPayloadHash(hash, GetArchiveByteOffset(descriptor));
    hash = MixPayloadHash(hash, GetArchiveByteSize(descriptor));
    if (hash == 0ULL) {
        return PAYLOAD_HASH_OFFSET;
    }

    return hash;
}

PackageEntryDescriptor NormalizeEntryDescriptor(const PackageEntryDescriptor& descriptor) {
    const std::uint64_t archive_byte_offset = GetArchiveByteOffset(descriptor);
    const std::uint64_t archive_byte_size = GetArchiveByteSize(descriptor);
    PackageEntryDescriptor normalized = descriptor;
    normalized.byte_offset = MakeLegacyByteValue(archive_byte_offset);
    normalized.byte_size = MakeLegacyByteValue(archive_byte_size);
    normalized.archive_byte_offset = archive_byte_offset;
    normalized.archive_byte_size = archive_byte_size;
    normalized.payload_logical_byte_count = GetPayloadLogicalByteCount(descriptor);
    normalized.payload_window_byte_offset = GetPayloadWindowByteOffset(descriptor);
    normalized.payload_window_byte_size = GetPayloadWindowByteSize(descriptor);
    if (normalized.payload_hash == 0ULL) {
        normalized.payload_hash = MakeEntryPayloadHash(normalized);
    }

    return normalized;
}

std::uint32_t MixHash(std::uint32_t hash, std::uint32_t value) {
    hash ^= value;
    return hash * HASH_MULTIPLIER;
}

std::uint32_t HashLogicalKey(const ResourceLogicalKey& logical_key) {
    std::uint32_t hash = HASH_OFFSET;
    for (const char character : logical_key.Value()) {
        hash = MixHash(hash, static_cast<std::uint32_t>(static_cast<unsigned char>(character)));
    }

    return hash;
}

std::uint32_t HashPackageEntry(PackageId package, PackageEntryId entry) {
    std::uint32_t hash = HASH_OFFSET;
    hash = MixHash(hash, package.value);
    return MixHash(hash, entry.value);
}

std::uint32_t HashResourceTuple(PackageId package, ResourceTypeId type, const ResourceLogicalKey& logical_key) {
    std::uint32_t hash = HASH_OFFSET;
    hash = MixHash(hash, package.value);
    hash = MixHash(hash, type.value);
    return MixHash(hash, HashLogicalKey(logical_key));
}

std::uint32_t HashResourceKey(PackageId package, const ResourceLogicalKey& logical_key) {
    std::uint32_t hash = HASH_OFFSET;
    hash = MixHash(hash, package.value);
    return MixHash(hash, HashLogicalKey(logical_key));
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

bool ContainsEntry(
    const std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT>& entries,
    std::uint32_t count,
    PackageEntryId entry) {
    for (std::uint32_t index = 0U; index < count; ++index) {
        if (entries[index].value == entry.value) {
            return true;
        }
    }

    return false;
}

bool PlanContainsEntry(const PackageLoadPlan& plan, PackageEntryId entry) {
    for (std::uint32_t index = 0U; index < plan.record_count; ++index) {
        if (plan.records[index].entry.value == entry.value) {
            return true;
        }
    }

    return false;
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
          ClampArchiveByteBudget(desc.load_plan_archive_byte_budget),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0ULL,
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
        PackageRegistrationResult result =
            PackageRegistrationResult::Failure(RecordFailure(PackageStatus::ManifestCapacityExceeded));
        result.required_manifest_record_count = snapshot_.manifest_count + 1U;
        snapshot_.required_manifest_record_count = result.required_manifest_record_count;
        return result;
    }

    std::uint32_t index = 0U;
    for (ManifestSlot& manifest : manifests_) {
        if (index >= snapshot_.manifest_capacity) {
            PackageRegistrationResult result =
                PackageRegistrationResult::Failure(RecordFailure(PackageStatus::ManifestCapacityExceeded));
            result.required_manifest_record_count = index + 1U;
            snapshot_.required_manifest_record_count = result.required_manifest_record_count;
            return result;
        }

        if (manifest.is_active) {
            ++index;
            continue;
        }

        manifest.id = descriptor.id;
        manifest.is_active = true;
        ++snapshot_.manifest_count;
        snapshot_.required_manifest_record_count = snapshot_.manifest_count;
        RecordSuccess();
        PackageRegistrationResult result = PackageRegistrationResult::ManifestSuccess(descriptor.id);
        result.required_manifest_record_count = snapshot_.manifest_count;
        return result;
    }

    PackageRegistrationResult result =
        PackageRegistrationResult::Failure(RecordFailure(PackageStatus::ManifestCapacityExceeded));
    result.required_manifest_record_count = snapshot_.manifest_count + 1U;
    snapshot_.required_manifest_record_count = result.required_manifest_record_count;
    return result;
}

PackageRegistrationResult PackageRegistry::RegisterEntry(const PackageEntryDescriptor& descriptor) {
    const PackageStatus validation_status = ValidateEntryDescriptor(descriptor);
    if (validation_status != PackageStatus::Success) {
        return PackageRegistrationResult::Failure(RecordFailure(validation_status));
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
        PackageRegistrationResult result =
            PackageRegistrationResult::Failure(RecordFailure(PackageStatus::EntryCapacityExceeded));
        result.required_entry_record_count = snapshot_.entry_count + 1U;
        snapshot_.required_entry_record_count = result.required_entry_record_count;
        return result;
    }

    std::uint32_t index = 0U;
    for (EntrySlot& entry : entries_) {
        if (index >= snapshot_.entry_capacity) {
            PackageRegistrationResult result =
                PackageRegistrationResult::Failure(RecordFailure(PackageStatus::EntryCapacityExceeded));
            result.required_entry_record_count = index + 1U;
            snapshot_.required_entry_record_count = result.required_entry_record_count;
            return result;
        }

        if (entry.is_active) {
            ++index;
            continue;
        }

        const PackageEntryDescriptor stored_descriptor = NormalizeEntryDescriptor(descriptor);
        entry.descriptor = stored_descriptor;
        entry.is_active = true;
        AddEntryIndex(stored_descriptor.package, stored_descriptor.entry, index);
        AddResourceIndex(stored_descriptor.package, stored_descriptor.type, stored_descriptor.logical_key, index);
        AddResourceKeyIndex(stored_descriptor.package, stored_descriptor.logical_key, index);
        ++snapshot_.entry_count;
        snapshot_.required_entry_record_count = snapshot_.entry_count;
        RecordSuccess();
        PackageRegistrationResult result =
            PackageRegistrationResult::EntrySuccess(stored_descriptor.package, stored_descriptor.entry);
        result.required_entry_record_count = snapshot_.entry_count;
        return result;
    }

    PackageRegistrationResult result =
        PackageRegistrationResult::Failure(RecordFailure(PackageStatus::EntryCapacityExceeded));
    result.required_entry_record_count = snapshot_.entry_count + 1U;
    snapshot_.required_entry_record_count = result.required_entry_record_count;
    return result;
}

PackageStatus PackageRegistry::AddDependency(PackageId package, PackageEntryId dependent, PackageEntryId dependency) {
    ++snapshot_.dependency_validation_count;

    if (!package.IsValid()) {
        return RecordFailure(PackageStatus::InvalidPackageId);
    }

    if (!dependent.IsValid() || !dependency.IsValid()) {
        return RecordFailure(PackageStatus::InvalidEntryId);
    }

    std::size_t dependent_index = 0U;
    const PackageStatus dependent_status = FindEntryIndex(package, dependent, dependent_index);
    if (dependent_status != PackageStatus::Success) {
        return RecordFailure(PackageStatus::DependencyMissing);
    }

    std::size_t dependency_index = 0U;
    const PackageStatus dependency_status = FindEntryIndex(package, dependency, dependency_index);
    if (dependency_status != PackageStatus::Success) {
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
        AppendDependencyEdge(static_cast<std::uint32_t>(dependent_index), index);
        ++snapshot_.dependency_edge_count;
        RecordSuccess();
        return PackageStatus::Success;
    }

    return RecordFailure(PackageStatus::DependencyCapacityExceeded);
}

PackageLoadPlanResult PackageRegistry::ResolveEntryByResourceKey(
    PackageId package,
    ResourceTypeId expected_type,
    const ResourceLogicalKey& logical_key) {
    if (!package.IsValid()) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::InvalidPackageId));
    }

    if (!expected_type.IsValid()) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::InvalidResourceType));
    }

    if (!logical_key.IsWithinBounds()) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::LogicalKeyTooLong));
    }

    if (!logical_key.IsValid()) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::InvalidLogicalKey));
    }

    std::size_t root_index = 0U;
    const PackageStatus find_status = FindEntryByResourceKey(package, expected_type, logical_key, root_index);
    if (find_status != PackageStatus::Success) {
        return PackageLoadPlanResult::Failure(RecordFailure(find_status));
    }

    const PackageEntryDescriptor& root_descriptor = entries_[root_index].descriptor;
    if (root_descriptor.type.value != expected_type.value) {
        return PackageLoadPlanResult::Failure(RecordFailure(PackageStatus::TypeMismatch));
    }

    PackageLoadPlan plan{};
    const PackageStatus plan_status = BuildDependencyClosurePlan(package, root_index, plan);
    if (plan_status != PackageStatus::Success) {
        PackageLoadPlanResult result = PackageLoadPlanResult::Failure(RecordFailure(plan_status));
        if (plan_status == PackageStatus::LoadPlanCapacityExceeded) {
            result.required_load_plan_record_count = plan.record_count + 1U;
            snapshot_.required_load_plan_record_count = result.required_load_plan_record_count;
        }

        return result;
    }

    ++snapshot_.load_plan_resolve_count;
    snapshot_.last_load_plan_record_count = plan.record_count;
    snapshot_.required_load_plan_record_count = plan.record_count;
    snapshot_.last_load_plan_archive_byte_count = plan.archive_byte_count;
    RecordSuccess();
    PackageLoadPlanResult result = PackageLoadPlanResult::Success(plan);
    result.required_load_plan_record_count = plan.record_count;
    return result;
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
    ++snapshot_.accepted_operation_count;
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

    const std::uint64_t archive_byte_offset = GetArchiveByteOffset(descriptor);
    const std::uint64_t archive_byte_size = GetArchiveByteSize(descriptor);
    if (!ByteRangeIsWithinBounds(archive_byte_offset, archive_byte_size)) {
        return PackageStatus::ByteRangeOutOfBounds;
    }

    if (!PayloadMetadataMatchesArchiveRange(descriptor)) {
        return PackageStatus::ByteRangeOutOfBounds;
    }

    return PackageStatus::Success;
}

PackageStatus PackageRegistry::FindEntryIndex(PackageId package, PackageEntryId entry, std::size_t& out_index) const {
    if (!package.IsValid()) {
        return PackageStatus::InvalidPackageId;
    }

    if (!entry.IsValid()) {
        return PackageStatus::InvalidEntryId;
    }

    if (TryFindEntryIndex(package, entry, out_index)) {
        return PackageStatus::Success;
    }

    return PackageStatus::NotFound;
}

PackageStatus PackageRegistry::FindEntryByResourceKey(
    PackageId package,
    ResourceTypeId expected_type,
    const ResourceLogicalKey& logical_key,
    std::size_t& out_index) const {
    if (!HasManifest(package)) {
        return PackageStatus::NotFound;
    }

    if (TryFindResourceIndex(package, expected_type, logical_key, out_index)) {
        return PackageStatus::Success;
    }

    if (HasResourceLogicalKey(package, logical_key)) {
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
    std::uint32_t stack_count = 0U;
    std::uint32_t visited_count = 0U;
    const auto contains_entry = [](const std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT>& entries,
                                  std::uint32_t count,
                                  PackageEntryId entry) {
        for (std::uint32_t index = 0U; index < count; ++index) {
            if (entries[index].value == entry.value) {
                return true;
            }
        }

        return false;
    };

    stack[stack_count] = start;
    ++stack_count;

    while (stack_count > 0U) {
        --stack_count;
        const PackageEntryId current = stack[stack_count];
        if (current.value == target.value) {
            return true;
        }

        if (contains_entry(visited, visited_count, current)) {
            continue;
        }

        if (visited_count < MAX_PACKAGE_ENTRY_COUNT) {
            visited[visited_count] = current;
            ++visited_count;
        }

        std::size_t current_index = 0U;
        if (FindEntryIndex(package, current, current_index) != PackageStatus::Success) {
            continue;
        }

        std::uint32_t edge_index = first_dependency_edge_for_entry_[current_index];
        while (edge_index != INVALID_INDEX) {
            const DependencyEdge& edge = dependency_edges_[edge_index];
            const std::uint32_t next_edge_index = next_dependency_edge_[edge_index];

            if (!contains_entry(visited, visited_count, edge.dependency) &&
                !contains_entry(stack, stack_count, edge.dependency) &&
                stack_count < MAX_PACKAGE_ENTRY_COUNT) {
                stack[stack_count] = edge.dependency;
                ++stack_count;
            }

            edge_index = next_edge_index;
        }
    }

    return false;
}

PackageStatus PackageRegistry::BuildDependencyClosurePlan(
    PackageId package,
    std::size_t root_index,
    PackageLoadPlan& out_plan) const {
    if (root_index >= snapshot_.entry_capacity) {
        return PackageStatus::DependencyMissing;
    }

    std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT> visiting_entries{};
    std::uint32_t visiting_count = 0U;
    return AppendDependencyClosure(package, root_index, visiting_entries, visiting_count, out_plan);
}

PackageStatus PackageRegistry::AppendDependencyClosure(
    PackageId package,
    std::size_t entry_index,
    std::array<PackageEntryId, MAX_PACKAGE_ENTRY_COUNT>& visiting_entries,
    std::uint32_t& visiting_count,
    PackageLoadPlan& out_plan) const {
    if (entry_index >= snapshot_.entry_capacity) {
        return PackageStatus::DependencyMissing;
    }

    const PackageEntryDescriptor& descriptor = entries_[entry_index].descriptor;
    if (PlanContainsEntry(out_plan, descriptor.entry)) {
        return PackageStatus::Success;
    }

    if (ContainsEntry(visiting_entries, visiting_count, descriptor.entry)) {
        return PackageStatus::DependencyCycle;
    }

    if (visiting_count >= MAX_PACKAGE_ENTRY_COUNT) {
        return PackageStatus::LoadPlanCapacityExceeded;
    }

    visiting_entries[visiting_count] = descriptor.entry;
    ++visiting_count;

    std::uint32_t edge_index = first_dependency_edge_for_entry_[entry_index];
    while (edge_index != INVALID_INDEX) {
        const DependencyEdge& edge = dependency_edges_[edge_index];
        const std::uint32_t next_edge_index = next_dependency_edge_[edge_index];

        std::size_t dependency_index = 0U;
        const PackageStatus dependency_find_status = FindEntryIndex(package, edge.dependency, dependency_index);
        if (dependency_find_status != PackageStatus::Success) {
            --visiting_count;
            return PackageStatus::DependencyMissing;
        }

        const PackageStatus dependency_status =
            AppendDependencyClosure(package, dependency_index, visiting_entries, visiting_count, out_plan);
        if (dependency_status != PackageStatus::Success) {
            --visiting_count;
            return dependency_status;
        }

        edge_index = next_edge_index;
    }

    --visiting_count;
    return TryAppendRecord(out_plan, descriptor);
}

PackageStatus PackageRegistry::TryAppendRecord(PackageLoadPlan& plan, const PackageEntryDescriptor& descriptor) const {
    if (plan.record_count >= snapshot_.load_plan_record_capacity) {
        return PackageStatus::LoadPlanCapacityExceeded;
    }

    if (plan.archive_byte_count > snapshot_.load_plan_archive_byte_budget) {
        return PackageStatus::LoadPlanByteBudgetExceeded;
    }

    const std::uint64_t archive_byte_size = GetArchiveByteSize(descriptor);
    const std::uint64_t remaining_budget = snapshot_.load_plan_archive_byte_budget - plan.archive_byte_count;
    if (archive_byte_size > remaining_budget) {
        return PackageStatus::LoadPlanByteBudgetExceeded;
    }

    AppendRecord(plan, descriptor);
    plan.archive_byte_count += archive_byte_size;
    return PackageStatus::Success;
}

void PackageRegistry::AppendRecord(PackageLoadPlan& plan, const PackageEntryDescriptor& descriptor) const {
    const std::uint64_t archive_byte_offset = GetArchiveByteOffset(descriptor);
    const std::uint64_t archive_byte_size = GetArchiveByteSize(descriptor);
    const std::uint32_t legacy_byte_offset = MakeLegacyByteValue(archive_byte_offset);
    const std::uint32_t legacy_byte_size = MakeLegacyByteValue(archive_byte_size);
    const std::uint64_t payload_logical_byte_count = GetPayloadLogicalByteCount(descriptor);
    const std::uint64_t payload_window_byte_offset = GetPayloadWindowByteOffset(descriptor);
    const std::uint64_t payload_window_byte_size = GetPayloadWindowByteSize(descriptor);
    plan.records[plan.record_count] = PackageLoadPlanRecord{
        descriptor.package,
        descriptor.entry,
        descriptor.type,
        descriptor.logical_key,
        descriptor.source_key,
        legacy_byte_offset,
        legacy_byte_size,
        archive_byte_offset,
        archive_byte_size,
        descriptor.payload_hash,
        payload_logical_byte_count,
        payload_window_byte_offset,
        payload_window_byte_size};
    ++plan.record_count;
}

bool PackageRegistry::TryFindEntryIndex(PackageId package, PackageEntryId entry, std::size_t& out_index) const {
    std::uint32_t probe_count = 0U;
    std::uint32_t probe_index = FirstProbeIndex(HashPackageEntry(package, entry), snapshot_.entry_capacity);
    while (probe_count < snapshot_.entry_capacity) {
        if (!entry_index_active_[probe_index]) {
            return false;
        }

        if (entry_index_packages_[probe_index].value == package.value &&
            entry_index_entries_[probe_index].value == entry.value) {
            out_index = entry_index_values_[probe_index];
            return true;
        }

        probe_index = NextProbeIndex(probe_index, snapshot_.entry_capacity);
        ++probe_count;
    }

    return false;
}

bool PackageRegistry::TryFindResourceIndex(
    PackageId package,
    ResourceTypeId expected_type,
    const ResourceLogicalKey& logical_key,
    std::size_t& out_index) const {
    std::uint32_t probe_count = 0U;
    std::uint32_t probe_index =
        FirstProbeIndex(HashResourceTuple(package, expected_type, logical_key), snapshot_.entry_capacity);
    while (probe_count < snapshot_.entry_capacity) {
        if (!resource_index_active_[probe_index]) {
            return false;
        }

        if (resource_index_packages_[probe_index].value == package.value &&
            resource_index_types_[probe_index].value == expected_type.value &&
            resource_index_keys_[probe_index].Equals(logical_key)) {
            out_index = resource_index_values_[probe_index];
            return true;
        }

        probe_index = NextProbeIndex(probe_index, snapshot_.entry_capacity);
        ++probe_count;
    }

    return false;
}

bool PackageRegistry::HasResourceLogicalKey(PackageId package, const ResourceLogicalKey& logical_key) const {
    std::uint32_t probe_count = 0U;
    std::uint32_t probe_index = FirstProbeIndex(HashResourceKey(package, logical_key), snapshot_.entry_capacity);
    while (probe_count < snapshot_.entry_capacity) {
        if (!resource_key_index_active_[probe_index]) {
            return false;
        }

        if (resource_key_index_packages_[probe_index].value == package.value &&
            resource_key_index_keys_[probe_index].Equals(logical_key)) {
            return true;
        }

        probe_index = NextProbeIndex(probe_index, snapshot_.entry_capacity);
        ++probe_count;
    }

    return false;
}

bool PackageRegistry::TryFindDependencyEdgeIndex(
    PackageId package,
    PackageEntryId dependent,
    PackageEntryId dependency,
    std::size_t& out_index) const {
    std::uint32_t probe_count = 0U;
    std::uint32_t probe_index =
        FirstProbeIndex(HashDependencyEdge(package, dependent, dependency), snapshot_.dependency_edge_capacity);
    while (probe_count < snapshot_.dependency_edge_capacity) {
        if (!dependency_index_active_[probe_index]) {
            return false;
        }

        if (dependency_index_packages_[probe_index].value == package.value &&
            dependency_index_dependents_[probe_index].value == dependent.value &&
            dependency_index_dependencies_[probe_index].value == dependency.value) {
            out_index = dependency_index_values_[probe_index];
            return true;
        }

        probe_index = NextProbeIndex(probe_index, snapshot_.dependency_edge_capacity);
        ++probe_count;
    }

    return false;
}

void PackageRegistry::AddEntryIndex(PackageId package, PackageEntryId entry, std::uint32_t entry_index) {
    std::uint32_t probe_count = 0U;
    std::uint32_t probe_index = FirstProbeIndex(HashPackageEntry(package, entry), snapshot_.entry_capacity);
    while (probe_count < snapshot_.entry_capacity) {
        if (!entry_index_active_[probe_index]) {
            entry_index_packages_[probe_index] = package;
            entry_index_entries_[probe_index] = entry;
            entry_index_values_[probe_index] = entry_index;
            entry_index_active_[probe_index] = true;
            return;
        }

        if (entry_index_packages_[probe_index].value == package.value &&
            entry_index_entries_[probe_index].value == entry.value) {
            entry_index_values_[probe_index] = entry_index;
            return;
        }

        probe_index = NextProbeIndex(probe_index, snapshot_.entry_capacity);
        ++probe_count;
    }
}

void PackageRegistry::AddResourceIndex(
    PackageId package,
    ResourceTypeId type,
    const ResourceLogicalKey& logical_key,
    std::uint32_t entry_index) {
    std::uint32_t probe_count = 0U;
    std::uint32_t probe_index =
        FirstProbeIndex(HashResourceTuple(package, type, logical_key), snapshot_.entry_capacity);
    while (probe_count < snapshot_.entry_capacity) {
        if (!resource_index_active_[probe_index]) {
            resource_index_packages_[probe_index] = package;
            resource_index_types_[probe_index] = type;
            resource_index_keys_[probe_index] = logical_key;
            resource_index_values_[probe_index] = entry_index;
            resource_index_active_[probe_index] = true;
            return;
        }

        if (resource_index_packages_[probe_index].value == package.value &&
            resource_index_types_[probe_index].value == type.value &&
            resource_index_keys_[probe_index].Equals(logical_key)) {
            resource_index_values_[probe_index] = entry_index;
            return;
        }

        probe_index = NextProbeIndex(probe_index, snapshot_.entry_capacity);
        ++probe_count;
    }
}

void PackageRegistry::AddResourceKeyIndex(
    PackageId package,
    const ResourceLogicalKey& logical_key,
    std::uint32_t entry_index) {
    std::uint32_t probe_count = 0U;
    std::uint32_t probe_index = FirstProbeIndex(HashResourceKey(package, logical_key), snapshot_.entry_capacity);
    while (probe_count < snapshot_.entry_capacity) {
        if (!resource_key_index_active_[probe_index]) {
            resource_key_index_packages_[probe_index] = package;
            resource_key_index_keys_[probe_index] = logical_key;
            resource_key_index_values_[probe_index] = entry_index;
            resource_key_index_active_[probe_index] = true;
            return;
        }

        if (resource_key_index_packages_[probe_index].value == package.value &&
            resource_key_index_keys_[probe_index].Equals(logical_key)) {
            resource_key_index_values_[probe_index] = entry_index;
            return;
        }

        probe_index = NextProbeIndex(probe_index, snapshot_.entry_capacity);
        ++probe_count;
    }
}

void PackageRegistry::AddDependencyIndex(
    PackageId package,
    PackageEntryId dependent,
    PackageEntryId dependency,
    std::uint32_t edge_index) {
    std::uint32_t probe_count = 0U;
    std::uint32_t probe_index =
        FirstProbeIndex(HashDependencyEdge(package, dependent, dependency), snapshot_.dependency_edge_capacity);
    while (probe_count < snapshot_.dependency_edge_capacity) {
        if (!dependency_index_active_[probe_index]) {
            dependency_index_packages_[probe_index] = package;
            dependency_index_dependents_[probe_index] = dependent;
            dependency_index_dependencies_[probe_index] = dependency;
            dependency_index_values_[probe_index] = edge_index;
            dependency_index_active_[probe_index] = true;
            return;
        }

        if (dependency_index_packages_[probe_index].value == package.value &&
            dependency_index_dependents_[probe_index].value == dependent.value &&
            dependency_index_dependencies_[probe_index].value == dependency.value) {
            dependency_index_values_[probe_index] = edge_index;
            return;
        }

        probe_index = NextProbeIndex(probe_index, snapshot_.dependency_edge_capacity);
        ++probe_count;
    }
}

void PackageRegistry::AppendDependencyEdge(std::uint32_t dependent_entry_index, std::uint32_t edge_index) {
    if (dependent_entry_index >= snapshot_.entry_capacity) {
        return;
    }

    if (edge_index >= snapshot_.dependency_edge_capacity) {
        return;
    }

    if (first_dependency_edge_for_entry_[dependent_entry_index] == INVALID_INDEX) {
        first_dependency_edge_for_entry_[dependent_entry_index] = edge_index;
        last_dependency_edge_for_entry_[dependent_entry_index] = edge_index;
        return;
    }

    const std::uint32_t last_edge_index = last_dependency_edge_for_entry_[dependent_entry_index];
    next_dependency_edge_[last_edge_index] = edge_index;
    last_dependency_edge_for_entry_[dependent_entry_index] = edge_index;
}
}
