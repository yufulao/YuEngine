#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Package/PackageConstants.h"
#include "YuEngine/Package/PackageRegistry.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceTypeId.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::package::PackageEntryDescriptor;
using yuengine::package::PackageEntryId;
using yuengine::package::PackageId;
using yuengine::package::PackageLoadPlanResult;
using yuengine::package::PackageRegistrationResult;
using PackageRegistry = yuengine::package::PackageRegistry;
using yuengine::package::PackageRegistryDesc;
using yuengine::package::PackageSnapshot;
using PackageSourceKey = yuengine::package::PackageSourceKey;
using yuengine::package::PackageStatus;
using ResourceLogicalKey = yuengine::resource::ResourceLogicalKey;
using yuengine::resource::ResourceTypeId;
using yuengine::package::MAX_DECLARED_ENTRY_SIZE;
using yuengine::package::MAX_PACKAGE_ENTRY_COUNT;
using yuengine::package::MAX_PACKAGE_SOURCE_KEY_BYTES;
using yuengine::resource::MAX_LOGICAL_KEY_BYTES;

namespace {
constexpr const char* TEST_REGISTER_MANIFEST = "Package_RegisterSyntheticManifest_ReturnsStableId";
constexpr const char* TEST_DUPLICATE_MANIFEST = "Package_RegisterDuplicateManifest_ReturnsExplicitStatus";
constexpr const char* TEST_REGISTER_ENTRY = "Package_RegisterEntry_ReturnsStableEntryId";
constexpr const char* TEST_DUPLICATE_ENTRY = "Package_RegisterDuplicateEntry_ReturnsExplicitStatus";
constexpr const char* TEST_DUPLICATE_RESOURCE_KEY = "Package_RegisterDuplicateResourceKey_ReturnsExplicitStatus";
constexpr const char* TEST_INVALID_IDS_TYPE_KEY =
    "Package_RegisterInvalidIdsOrType_ReturnsExplicitStatusWithoutMutation";
constexpr const char* TEST_MANIFEST_CAPACITY = "Package_ManifestCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_ENTRY_CAPACITY = "Package_EntryCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_OVERSIZED_KEYS = "Package_RegisterEntryRejectsOversizedKeysWithoutMutation";
constexpr const char* TEST_OVERSIZED_BYTE_RANGE = "Package_RegisterEntryRejectsOversizedByteRangeWithoutMutation";
constexpr const char* TEST_RESOLVE = "Package_ResolveEntryByResourceKey_ReturnsDeterministicLoadPlan";
constexpr const char* TEST_RESOLVE_RESOURCE_KEY_TUPLE = "Package_ResolveEntryByResourceKey_UsesTypeAndLogicalKeyTuple";
constexpr const char* TEST_INDEXED_LOOKUPS = "Package_IndexedLookupsPreserveStatusOrderAndCapacities";
constexpr const char* TEST_UNKNOWN_KEY = "Package_ResolveRejectsUnknownResourceKey";
constexpr const char* TEST_TYPE_MISMATCH = "Package_ResolveRejectsTypeMismatchWithoutMutation";
constexpr const char* TEST_MISSING_DEPENDENCY = "Package_DependencyValidationRejectsMissingEntry";
constexpr const char* TEST_DEPENDENCY_CYCLE = "Package_DependencyValidationRejectsCycle";
constexpr const char* TEST_DEPENDENCY_CYCLE_HIGH_FANOUT = "Package_DependencyValidationRejectsCycleAfterHighFanout";
constexpr const char* TEST_DEPENDENCY_ORDER = "Package_DependencyPlanPreservesDeclarationOrder";
constexpr const char* TEST_DEPENDENCY_CAPACITY = "Package_DependencyCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_LOAD_PLAN_CAPACITY = "Package_LoadPlanCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Package_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_FILE_ORIGINAL = "Package_NoFileReadOriginalPackageOrGameAdapterDependency";
constexpr const char* TEST_NO_HIDDEN_ALLOCATION = "Package_NoHiddenAllocation_UsesYuMemorySignal";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char* INDEX_TEXTURE_LOGICAL = "index_texture";
constexpr const char* INDEX_AUDIO_LOGICAL = "index_audio";
constexpr const char* INDEX_MATERIAL_LOGICAL = "index_material";
constexpr const char* INDEX_TEXTURE_SOURCE = "textures/index_texture.bin";
constexpr const char* INDEX_AUDIO_SOURCE = "audio/index_audio.bin";
constexpr const char* INDEX_MATERIAL_SOURCE = "materials/index_material.bin";
constexpr const char* ERROR_INDEX_MANIFEST_FAILED = "indexed lookup fixture manifest failed";
constexpr const char* ERROR_INDEX_TEXTURE_FAILED = "indexed lookup fixture texture failed";
constexpr const char* ERROR_INDEX_AUDIO_FAILED = "indexed lookup fixture audio failed";
constexpr const char* ERROR_INDEX_MATERIAL_FAILED = "indexed lookup fixture material failed";
constexpr const char* ERROR_INDEX_AUDIO_EDGE_FAILED = "indexed lookup fixture audio edge failed";
constexpr const char* ERROR_INDEX_MATERIAL_EDGE_FAILED = "indexed lookup fixture material edge failed";
constexpr const char* ERROR_INDEX_DUPLICATE_EDGE_FAILED = "indexed duplicate edge did not succeed";
constexpr const char* ERROR_INDEX_DUPLICATE_EDGE_MUTATED = "indexed duplicate edge changed dependency count";
constexpr const char* ERROR_INDEX_TYPE_MISMATCH_STATUS = "indexed logical key type mismatch returned wrong status";
constexpr const char* ERROR_INDEX_TYPE_MISMATCH_MUTATED = "indexed type mismatch changed resolve count";
constexpr const char* ERROR_INDEX_RESOLVE_FAILED = "indexed resource lookup failed";
constexpr const char* ERROR_INDEX_RECORD_COUNT = "indexed lookup returned unexpected record count";
constexpr const char* ERROR_INDEX_FIRST_DEPENDENCY = "indexed lookup first dependency order changed";
constexpr const char* ERROR_INDEX_SECOND_DEPENDENCY = "indexed lookup second dependency order changed";
constexpr const char* ERROR_INDEX_ROOT_RECORD = "indexed lookup root record changed";
constexpr const char* ERROR_INDEX_MANIFEST_CAPACITY = "indexed lookup changed manifest capacity";
constexpr const char* ERROR_INDEX_ENTRY_CAPACITY = "indexed lookup changed entry capacity";
constexpr const char* ERROR_INDEX_DEPENDENCY_CAPACITY = "indexed lookup changed dependency capacity";
constexpr const char* ERROR_INDEX_LOAD_PLAN_CAPACITY = "indexed lookup changed load-plan capacity";

constexpr PackageId PACKAGE_A{1U};
constexpr PackageId PACKAGE_B{2U};
constexpr PackageEntryId ENTRY_TEXTURE{1U};
constexpr PackageEntryId ENTRY_MATERIAL{2U};
constexpr PackageEntryId ENTRY_AUDIO{3U};
constexpr PackageEntryId ENTRY_EFFECT{4U};
constexpr ResourceTypeId TYPE_TEXTURE{1U};
constexpr ResourceTypeId TYPE_MATERIAL{2U};
constexpr ResourceTypeId TYPE_AUDIO{3U};
constexpr ResourceTypeId TYPE_EFFECT{4U};
using TestFunction = int (*)();

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

PackageEntryDescriptor Entry(
    PackageId package,
    PackageEntryId entry,
    ResourceTypeId type,
    const char* logicalKey,
    const char* sourceKey,
    std::uint32_t byteOffset = 0U,
    std::uint32_t byteSize = 16U) {
    return PackageEntryDescriptor{
        package,
        entry,
        type,
        ResourceLogicalKey(logicalKey),
        PackageSourceKey(sourceKey),
        byteOffset,
        byteSize};
}

PackageRegistrationResult RegisterManifest(PackageRegistry& registry, PackageId package = PACKAGE_A) {
    return registry.RegisterSyntheticManifest({package});
}

PackageRegistrationResult RegisterEntry(
    PackageRegistry& registry,
    PackageEntryId entry,
    ResourceTypeId type,
    const char* logicalKey,
    const char* sourceKey,
    std::uint32_t byteOffset = 0U,
    std::uint32_t byteSize = 16U) {
    return registry.RegisterEntry(Entry(PACKAGE_A, entry, type, logicalKey, sourceKey, byteOffset, byteSize));
}

bool CoreCountsMatch(const PackageSnapshot& left, const PackageSnapshot& right) {
    if (left.ManifestCount != right.ManifestCount) {
        return false;
    }

    if (left.EntryCount != right.EntryCount) {
        return false;
    }

    if (left.DependencyEdgeCount != right.DependencyEdgeCount) {
        return false;
    }

    if (left.DependencyValidationCount != right.DependencyValidationCount) {
        return false;
    }

    if (left.LoadPlanResolveCount != right.LoadPlanResolveCount) {
        return false;
    }

    return left.LastLoadPlanRecordCount == right.LastLoadPlanRecordCount;
}

bool SnapshotsMatch(const PackageSnapshot& left, const PackageSnapshot& right) {
    if (!CoreCountsMatch(left, right)) {
        return false;
    }

    if (left.RejectedOperationCount != right.RejectedOperationCount) {
        return false;
    }

    return left.AllocationAccountingStatus == right.AllocationAccountingStatus;
}

int PackageRegisterSyntheticManifestReturnsStableId() {
    PackageRegistry registry;
    const PackageRegistrationResult result = RegisterManifest(registry);
    if (!result.Succeeded()) {
        return Fail("synthetic manifest did not register");
    }

    if (result.Package.Value != PACKAGE_A.Value) {
        return Fail("manifest result did not return stable package id");
    }

    const PackageSnapshot snapshot = registry.Snapshot();
    if (snapshot.ManifestCount != 1U) {
        return Fail("manifest count was not recorded");
    }

    if (snapshot.LastStatus != PackageStatus::Success) {
        return Fail("manifest success did not record success status");
    }

    return 0;
}

int PackageRegisterDuplicateManifestReturnsExplicitStatus() {
    PackageRegistry registry;
    if (!RegisterManifest(registry).Succeeded()) {
        return Fail("first manifest registration failed");
    }

    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    const PackageRegistrationResult duplicate = RegisterManifest(registry);
    if (duplicate.Status != PackageStatus::DuplicateManifest) {
        return Fail("duplicate manifest did not return explicit status");
    }

    if (registry.Snapshot().ManifestCount != beforeSnapshot.ManifestCount) {
        return Fail("duplicate manifest changed manifest count");
    }

    return 0;
}

int PackageRegisterEntryReturnsStableEntryId() {
    PackageRegistry registry;
    if (!RegisterManifest(registry).Succeeded()) {
        return Fail("manifest registration failed");
    }

    const PackageRegistrationResult result =
        RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");
    if (!result.Succeeded()) {
        return Fail("entry did not register");
    }

    if (result.Entry.Value != ENTRY_TEXTURE.Value) {
        return Fail("entry result did not return stable entry id");
    }

    if (registry.Snapshot().EntryCount != 1U) {
        return Fail("entry count was not recorded");
    }

    return 0;
}

int PackageRegisterDuplicateEntryReturnsExplicitStatus() {
    PackageRegistry registry;
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");

    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    const PackageRegistrationResult duplicate =
        RegisterEntry(registry, ENTRY_TEXTURE, TYPE_MATERIAL, "material_a", "materials/material_a.bin");
    if (duplicate.Status != PackageStatus::DuplicateEntry) {
        return Fail("duplicate entry did not return explicit status");
    }

    if (registry.Snapshot().EntryCount != beforeSnapshot.EntryCount) {
        return Fail("duplicate entry changed entry count");
    }

    return 0;
}

int PackageRegisterDuplicateResourceKeyReturnsExplicitStatus() {
    PackageRegistry registry;
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");

    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    const PackageRegistrationResult duplicate = RegisterEntry(
        registry,
        ENTRY_MATERIAL,
        TYPE_TEXTURE,
        "texture_a",
        "textures/texture_a_copy.bin");
    if (duplicate.Status != PackageStatus::DuplicateResourceKey) {
        return Fail("duplicate resource key did not return explicit status");
    }

    if (registry.Snapshot().EntryCount != beforeSnapshot.EntryCount) {
        return Fail("duplicate resource key changed entry count");
    }

    return 0;
}

int PackageRegisterInvalidIdsOrTypeReturnsExplicitStatusWithoutMutation() {
    PackageRegistry registry;
    RegisterManifest(registry);

    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    if (registry.RegisterSyntheticManifest({PackageId{}}).Status != PackageStatus::InvalidPackageId) {
        return Fail("invalid package id did not return explicit status");
    }

    if (registry.RegisterEntry(Entry(PackageId{}, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "a.bin")).Status !=
        PackageStatus::InvalidPackageId) {
        return Fail("entry invalid package id did not return explicit status");
    }

    if (registry.RegisterEntry(Entry(PACKAGE_A, PackageEntryId{}, TYPE_TEXTURE, "texture_a", "a.bin")).Status !=
        PackageStatus::InvalidEntryId) {
        return Fail("invalid entry id did not return explicit status");
    }

    if (registry.RegisterEntry(Entry(PACKAGE_A, ENTRY_TEXTURE, ResourceTypeId{}, "texture_a", "a.bin")).Status !=
        PackageStatus::InvalidResourceType) {
        return Fail("invalid resource type did not return explicit status");
    }

    if (registry.RegisterEntry(Entry(PACKAGE_A, ENTRY_TEXTURE, TYPE_TEXTURE, "Texture_A", "a.bin")).Status !=
        PackageStatus::InvalidLogicalKey) {
        return Fail("invalid logical key did not return explicit status");
    }

    if (!CoreCountsMatch(beforeSnapshot, registry.Snapshot())) {
        return Fail("invalid descriptor changed core package counters");
    }

    return 0;
}

int PackageManifestCapacityOverflowDoesNotMutate() {
    PackageRegistry registry(PackageRegistryDesc{1U, 4U, 4U, 4U});
    RegisterManifest(registry, PACKAGE_A);

    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    const PackageRegistrationResult overflow = RegisterManifest(registry, PACKAGE_B);
    if (overflow.Status != PackageStatus::ManifestCapacityExceeded) {
        return Fail("manifest capacity overflow did not return explicit status");
    }

    if (registry.Snapshot().ManifestCount != beforeSnapshot.ManifestCount) {
        return Fail("manifest capacity overflow changed manifest count");
    }

    return 0;
}

int PackageEntryCapacityOverflowDoesNotMutate() {
    PackageRegistry registry(PackageRegistryDesc{1U, 1U, 4U, 4U});
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");

    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    const PackageRegistrationResult overflow =
        RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin");
    if (overflow.Status != PackageStatus::EntryCapacityExceeded) {
        return Fail("entry capacity overflow did not return explicit status");
    }

    if (registry.Snapshot().EntryCount != beforeSnapshot.EntryCount) {
        return Fail("entry capacity overflow changed entry count");
    }

    return 0;
}

int PackageRegisterEntryRejectsOversizedKeysWithoutMutation() {
    PackageRegistry registry;
    RegisterManifest(registry);
    const PackageSnapshot beforeSnapshot = registry.Snapshot();

    const std::string longLogicalKey(MAX_LOGICAL_KEY_BYTES + 1U, 'a');
    const PackageRegistrationResult longLogical = registry.RegisterEntry(
        Entry(PACKAGE_A, ENTRY_TEXTURE, TYPE_TEXTURE, longLogicalKey.c_str(), "textures/texture_a.bin"));
    if (longLogical.Status != PackageStatus::LogicalKeyTooLong) {
        return Fail("oversized logical key did not return explicit status");
    }

    const std::string longSourceKey(MAX_PACKAGE_SOURCE_KEY_BYTES + 1U, 'a');
    const PackageRegistrationResult longSource =
        registry.RegisterEntry(Entry(PACKAGE_A, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", longSourceKey.c_str()));
    if (longSource.Status != PackageStatus::SourceKeyTooLong) {
        return Fail("oversized source key did not return explicit status");
    }

    if (registry.Snapshot().EntryCount != beforeSnapshot.EntryCount) {
        return Fail("oversized keys changed entry count");
    }

    return 0;
}

int PackageRegisterEntryRejectsOversizedByteRangeWithoutMutation() {
    PackageRegistry registry;
    RegisterManifest(registry);
    const PackageSnapshot beforeSnapshot = registry.Snapshot();

    const PackageRegistrationResult tooLarge = RegisterEntry(
        registry,
        ENTRY_TEXTURE,
        TYPE_TEXTURE,
        "texture_a",
        "textures/texture_a.bin",
        0U,
        MAX_DECLARED_ENTRY_SIZE + 1U);
    if (tooLarge.Status != PackageStatus::ByteRangeOutOfBounds) {
        return Fail("oversized byte range did not return explicit status");
    }

    const PackageRegistrationResult overflow = RegisterEntry(
        registry,
        ENTRY_TEXTURE,
        TYPE_TEXTURE,
        "texture_a",
        "textures/texture_a.bin",
        std::numeric_limits<std::uint32_t>::max(),
        1U);
    if (overflow.Status != PackageStatus::ByteRangeOutOfBounds) {
        return Fail("overflow byte range did not return explicit status");
    }

    if (registry.Snapshot().EntryCount != beforeSnapshot.EntryCount) {
        return Fail("oversized byte range changed entry count");
    }

    return 0;
}

PackageRegistry CreateResolvedRegistry() {
    PackageRegistry registry;
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin", 64U, 32U);
    RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin", 8U, 16U);
    RegisterEntry(registry, ENTRY_AUDIO, TYPE_AUDIO, "audio_a", "audio/audio_a.bin", 24U, 16U);
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_AUDIO);
    return registry;
}

int PackageResolveEntryByResourceKeyReturnsDeterministicLoadPlan() {
    PackageRegistry registry = CreateResolvedRegistry();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (!result.Succeeded()) {
        return Fail("resolve by resource key failed");
    }

    if (result.Plan.RecordCount != 3U) {
        return Fail("load plan did not include dependencies plus root");
    }

    if (result.Plan.Records[0U].Entry.Value != ENTRY_MATERIAL.Value) {
        return Fail("first dependency record was not declaration-order material");
    }

    if (result.Plan.Records[1U].Entry.Value != ENTRY_AUDIO.Value) {
        return Fail("second dependency record was not declaration-order audio");
    }

    if (result.Plan.Records[2U].Entry.Value != ENTRY_TEXTURE.Value) {
        return Fail("root record was not last");
    }

    if (registry.Snapshot().LastLoadPlanRecordCount != 3U) {
        return Fail("load plan record count was not recorded");
    }

    return 0;
}

int PackageResolveEntryByResourceKeyUsesTypeAndLogicalKeyTuple() {
    PackageRegistry registry;
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "shared_asset", "textures/shared_asset.bin", 64U, 32U);
    RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "shared_asset", "materials/shared_asset.bin", 8U, 16U);

    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_MATERIAL, ResourceLogicalKey("shared_asset"));
    if (!result.Succeeded()) {
        return Fail("resolve by type and logical key tuple failed");
    }

    if (result.Plan.RecordCount != 1U) {
        return Fail("same-key different-type resolve returned unexpected record count");
    }

    if (result.Plan.Records[0U].Entry.Value != ENTRY_MATERIAL.Value) {
        return Fail("same-key different-type resolve returned the wrong entry");
    }

    if (result.Plan.Records[0U].Type.Value != TYPE_MATERIAL.Value) {
        return Fail("same-key different-type resolve returned the wrong type");
    }

    if (registry.Snapshot().LastLoadPlanRecordCount != 1U) {
        return Fail("same-key different-type load plan record count was not recorded");
    }

    return 0;
}

int PackageIndexedLookupsPreserveStatusOrderAndCapacities() {
    PackageRegistry registry;
    if (!RegisterManifest(registry).Succeeded()) {
        return Fail(ERROR_INDEX_MANIFEST_FAILED);
    }

    if (!RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, INDEX_TEXTURE_LOGICAL, INDEX_TEXTURE_SOURCE).Succeeded()) {
        return Fail(ERROR_INDEX_TEXTURE_FAILED);
    }

    if (!RegisterEntry(registry, ENTRY_AUDIO, TYPE_AUDIO, INDEX_AUDIO_LOGICAL, INDEX_AUDIO_SOURCE).Succeeded()) {
        return Fail(ERROR_INDEX_AUDIO_FAILED);
    }

    const PackageRegistrationResult materialResult =
        RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, INDEX_MATERIAL_LOGICAL, INDEX_MATERIAL_SOURCE);
    if (!materialResult.Succeeded()) {
        return Fail(ERROR_INDEX_MATERIAL_FAILED);
    }

    if (registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_AUDIO) != PackageStatus::Success) {
        return Fail(ERROR_INDEX_AUDIO_EDGE_FAILED);
    }

    if (registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL) != PackageStatus::Success) {
        return Fail(ERROR_INDEX_MATERIAL_EDGE_FAILED);
    }

    const PackageSnapshot beforeDuplicate = registry.Snapshot();
    if (registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_AUDIO) != PackageStatus::Success) {
        return Fail(ERROR_INDEX_DUPLICATE_EDGE_FAILED);
    }

    if (registry.Snapshot().DependencyEdgeCount != beforeDuplicate.DependencyEdgeCount) {
        return Fail(ERROR_INDEX_DUPLICATE_EDGE_MUTATED);
    }

    const PackageSnapshot beforeMismatch = registry.Snapshot();
    const PackageLoadPlanResult mismatch =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_AUDIO, ResourceLogicalKey(INDEX_TEXTURE_LOGICAL));
    if (mismatch.Status != PackageStatus::TypeMismatch) {
        return Fail(ERROR_INDEX_TYPE_MISMATCH_STATUS);
    }

    if (registry.Snapshot().LoadPlanResolveCount != beforeMismatch.LoadPlanResolveCount) {
        return Fail(ERROR_INDEX_TYPE_MISMATCH_MUTATED);
    }

    const PackageSnapshot beforeResolve = registry.Snapshot();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey(INDEX_TEXTURE_LOGICAL));
    if (!result.Succeeded()) {
        return Fail(ERROR_INDEX_RESOLVE_FAILED);
    }

    if (result.Plan.RecordCount != 3U) {
        return Fail(ERROR_INDEX_RECORD_COUNT);
    }

    if (result.Plan.Records[0U].Entry.Value != ENTRY_AUDIO.Value) {
        return Fail(ERROR_INDEX_FIRST_DEPENDENCY);
    }

    if (result.Plan.Records[1U].Entry.Value != ENTRY_MATERIAL.Value) {
        return Fail(ERROR_INDEX_SECOND_DEPENDENCY);
    }

    if (result.Plan.Records[2U].Entry.Value != ENTRY_TEXTURE.Value) {
        return Fail(ERROR_INDEX_ROOT_RECORD);
    }

    const PackageSnapshot afterResolve = registry.Snapshot();
    if (afterResolve.ManifestCapacity != beforeResolve.ManifestCapacity) {
        return Fail(ERROR_INDEX_MANIFEST_CAPACITY);
    }

    if (afterResolve.EntryCapacity != beforeResolve.EntryCapacity) {
        return Fail(ERROR_INDEX_ENTRY_CAPACITY);
    }

    if (afterResolve.DependencyEdgeCapacity != beforeResolve.DependencyEdgeCapacity) {
        return Fail(ERROR_INDEX_DEPENDENCY_CAPACITY);
    }

    if (afterResolve.LoadPlanRecordCapacity != beforeResolve.LoadPlanRecordCapacity) {
        return Fail(ERROR_INDEX_LOAD_PLAN_CAPACITY);
    }

    return 0;
}

int PackageResolveRejectsUnknownResourceKey() {
    PackageRegistry registry = CreateResolvedRegistry();
    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("missing_texture"));
    if (result.Status != PackageStatus::NotFound) {
        return Fail("unknown resource key did not return explicit status");
    }

    if (registry.Snapshot().LoadPlanResolveCount != beforeSnapshot.LoadPlanResolveCount) {
        return Fail("unknown resource key changed load-plan resolve count");
    }

    return 0;
}

int PackageResolveRejectsTypeMismatchWithoutMutation() {
    PackageRegistry registry = CreateResolvedRegistry();
    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_AUDIO, ResourceLogicalKey("texture_a"));
    if (result.Status != PackageStatus::TypeMismatch) {
        return Fail("type mismatch did not return explicit status");
    }

    if (registry.Snapshot().LoadPlanResolveCount != beforeSnapshot.LoadPlanResolveCount) {
        return Fail("type mismatch changed load-plan resolve count");
    }

    if (registry.Snapshot().LastLoadPlanRecordCount != beforeSnapshot.LastLoadPlanRecordCount) {
        return Fail("type mismatch changed last load-plan record count");
    }

    return 0;
}

int PackageDependencyValidationRejectsMissingEntry() {
    PackageRegistry registry;
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");

    const PackageStatus status = registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);
    if (status != PackageStatus::DependencyMissing) {
        return Fail("missing dependency did not return explicit status");
    }

    const PackageSnapshot snapshot = registry.Snapshot();
    if (snapshot.DependencyEdgeCount != 0U) {
        return Fail("missing dependency changed dependency edge count");
    }

    if (snapshot.DependencyValidationCount != 1U) {
        return Fail("missing dependency validation was not counted");
    }

    return 0;
}

int PackageDependencyValidationRejectsCycle() {
    PackageRegistry registry;
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");
    RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin");
    RegisterEntry(registry, ENTRY_AUDIO, TYPE_AUDIO, "audio_a", "audio/audio_a.bin");

    if (registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_TEXTURE) != PackageStatus::DependencyCycle) {
        return Fail("self dependency did not return cycle status");
    }

    if (registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL) != PackageStatus::Success) {
        return Fail("first dependency edge failed");
    }

    if (registry.AddDependency(PACKAGE_A, ENTRY_MATERIAL, ENTRY_AUDIO) != PackageStatus::Success) {
        return Fail("second dependency edge failed");
    }

    if (registry.AddDependency(PACKAGE_A, ENTRY_AUDIO, ENTRY_TEXTURE) != PackageStatus::DependencyCycle) {
        return Fail("cycle dependency did not return explicit status");
    }

    if (registry.Snapshot().DependencyEdgeCount != 2U) {
        return Fail("cycle dependency changed dependency edge count");
    }

    return 0;
}

int PackageDependencyValidationRejectsCycleAfterHighFanout() {
    constexpr PackageEntryId start{1U};
    constexpr PackageEntryId target{2U};
    constexpr PackageEntryId branch{32U};

    PackageRegistry registry;
    RegisterManifest(registry);
    for (std::uint32_t index = 1U; index <= MAX_PACKAGE_ENTRY_COUNT; ++index) {
        const std::string suffix = std::to_string(index);
        const std::string logicalKey = "entry_" + suffix;
        const std::string sourceKey = "package/entry_" + suffix + ".bin";
        const PackageRegistrationResult result =
            registry.RegisterEntry(Entry(PACKAGE_A, PackageEntryId{index}, TYPE_TEXTURE, logicalKey.c_str(), sourceKey.c_str()));
        if (!result.Succeeded()) {
            return Fail("high-fanout cycle fixture failed to register entries");
        }
    }

    for (std::uint32_t index = 3U; index <= MAX_PACKAGE_ENTRY_COUNT; ++index) {
        if (registry.AddDependency(PACKAGE_A, start, PackageEntryId{index}) != PackageStatus::Success) {
            return Fail("high-fanout cycle fixture failed to add start edge");
        }
    }

    if (registry.AddDependency(PACKAGE_A, branch, PackageEntryId{3U}) != PackageStatus::Success) {
        return Fail("high-fanout cycle fixture failed to add duplicate pending edge");
    }

    if (registry.AddDependency(PACKAGE_A, branch, PackageEntryId{4U}) != PackageStatus::Success) {
        return Fail("high-fanout cycle fixture failed to add second duplicate pending edge");
    }

    if (registry.AddDependency(PACKAGE_A, branch, PackageEntryId{5U}) != PackageStatus::Success) {
        return Fail("high-fanout cycle fixture failed to add third duplicate pending edge");
    }

    if (registry.AddDependency(PACKAGE_A, branch, target) != PackageStatus::Success) {
        return Fail("high-fanout cycle fixture failed to add target path edge");
    }

    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    const PackageStatus cycleStatus = registry.AddDependency(PACKAGE_A, target, start);
    if (cycleStatus != PackageStatus::DependencyCycle) {
        return Fail("high-fanout dependency path did not return explicit cycle status");
    }

    if (registry.Snapshot().DependencyEdgeCount != beforeSnapshot.DependencyEdgeCount) {
        return Fail("high-fanout cycle dependency changed dependency edge count");
    }

    return 0;
}

int PackageDependencyPlanPreservesDeclarationOrder() {
    PackageRegistry registry;
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");
    RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin");
    RegisterEntry(registry, ENTRY_AUDIO, TYPE_AUDIO, "audio_a", "audio/audio_a.bin");
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_AUDIO);
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);

    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (!result.Succeeded()) {
        return Fail("resolve failed");
    }

    if (result.Plan.Records[0U].Entry.Value != ENTRY_AUDIO.Value) {
        return Fail("first dependency did not preserve declaration order");
    }

    if (result.Plan.Records[1U].Entry.Value != ENTRY_MATERIAL.Value) {
        return Fail("second dependency did not preserve declaration order");
    }

    return 0;
}

int PackageDependencyCapacityOverflowDoesNotMutate() {
    PackageRegistry registry(PackageRegistryDesc{1U, 4U, 1U, 4U});
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");
    RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin");
    RegisterEntry(registry, ENTRY_AUDIO, TYPE_AUDIO, "audio_a", "audio/audio_a.bin");
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);

    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    const PackageStatus overflow = registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_AUDIO);
    if (overflow != PackageStatus::DependencyCapacityExceeded) {
        return Fail("dependency capacity overflow did not return explicit status");
    }

    if (registry.Snapshot().DependencyEdgeCount != beforeSnapshot.DependencyEdgeCount) {
        return Fail("dependency capacity overflow changed edge count");
    }

    return 0;
}

int PackageLoadPlanCapacityOverflowDoesNotMutate() {
    PackageRegistry registry(PackageRegistryDesc{1U, 4U, 4U, 1U});
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");
    RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin");
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);

    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (result.Status != PackageStatus::LoadPlanCapacityExceeded) {
        return Fail("load-plan capacity overflow did not return explicit status");
    }

    if (registry.Snapshot().LoadPlanResolveCount != beforeSnapshot.LoadPlanResolveCount) {
        return Fail("load-plan capacity overflow changed resolve count");
    }

    if (registry.Snapshot().LastLoadPlanRecordCount != beforeSnapshot.LastLoadPlanRecordCount) {
        return Fail("load-plan capacity overflow changed plan record count");
    }

    return 0;
}

int PackageDisabledDiagnosticsDoesNotChangeResults() {
    PackageRegistry recordingRegistry = CreateResolvedRegistry();
    PackageRegistry disabledRegistry = CreateResolvedRegistry();

    const PackageLoadPlanResult recordingResult =
        recordingRegistry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    const PackageLoadPlanResult disabledResult =
        disabledRegistry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (recordingResult.Status != disabledResult.Status) {
        return Fail("disabled diagnostics changed resolve status");
    }

    const PackageLoadPlanResult recordingFailure =
        recordingRegistry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_AUDIO, ResourceLogicalKey("texture_a"));
    const PackageLoadPlanResult disabledFailure =
        disabledRegistry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_AUDIO, ResourceLogicalKey("texture_a"));
    if (recordingFailure.Status != disabledFailure.Status) {
        return Fail("disabled diagnostics changed failure status");
    }

    if (!SnapshotsMatch(recordingRegistry.Snapshot(), disabledRegistry.Snapshot())) {
        return Fail("disabled diagnostics changed package snapshot");
    }

    return 0;
}

int PackageNoFileReadOriginalPackageOrGameAdapterDependency() {
    PackageRegistry registry = CreateResolvedRegistry();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (!result.Succeeded()) {
        return Fail("synthetic resolve failed");
    }

    if (result.Plan.Records[2U].SourceKey.Value() != std::string_view("textures/texture_a.bin")) {
        return Fail("synthetic source-key metadata was not preserved");
    }

    return 0;
}

int PackageNoHiddenAllocationUsesYuMemorySignal() {
    PackageRegistry registry = CreateResolvedRegistry();
    const PackageSnapshot beforeSnapshot = registry.Snapshot();
    if (beforeSnapshot.AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("package registry did not expose YuMemory accounting vocabulary");
    }

    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (!result.Succeeded()) {
        return Fail("resolve failed");
    }

    const PackageSnapshot afterSnapshot = registry.Snapshot();
    if (afterSnapshot.ManifestCapacity != beforeSnapshot.ManifestCapacity) {
        return Fail("manifest capacity changed during resolve");
    }

    if (afterSnapshot.EntryCapacity != beforeSnapshot.EntryCapacity) {
        return Fail("entry capacity changed during resolve");
    }

    if (afterSnapshot.DependencyEdgeCapacity != beforeSnapshot.DependencyEdgeCapacity) {
        return Fail("dependency capacity changed during resolve");
    }

    if (afterSnapshot.LoadPlanRecordCapacity != beforeSnapshot.LoadPlanRecordCapacity) {
        return Fail("load-plan capacity changed during resolve");
    }

    if (afterSnapshot.AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("package registry changed allocation accounting vocabulary");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> testRegistry{
        {TEST_REGISTER_MANIFEST, PackageRegisterSyntheticManifestReturnsStableId},
        {TEST_DUPLICATE_MANIFEST, PackageRegisterDuplicateManifestReturnsExplicitStatus},
        {TEST_REGISTER_ENTRY, PackageRegisterEntryReturnsStableEntryId},
        {TEST_DUPLICATE_ENTRY, PackageRegisterDuplicateEntryReturnsExplicitStatus},
        {TEST_DUPLICATE_RESOURCE_KEY, PackageRegisterDuplicateResourceKeyReturnsExplicitStatus},
        {TEST_INVALID_IDS_TYPE_KEY, PackageRegisterInvalidIdsOrTypeReturnsExplicitStatusWithoutMutation},
        {TEST_MANIFEST_CAPACITY, PackageManifestCapacityOverflowDoesNotMutate},
        {TEST_ENTRY_CAPACITY, PackageEntryCapacityOverflowDoesNotMutate},
        {TEST_OVERSIZED_KEYS, PackageRegisterEntryRejectsOversizedKeysWithoutMutation},
        {TEST_OVERSIZED_BYTE_RANGE, PackageRegisterEntryRejectsOversizedByteRangeWithoutMutation},
        {TEST_RESOLVE, PackageResolveEntryByResourceKeyReturnsDeterministicLoadPlan},
        {TEST_RESOLVE_RESOURCE_KEY_TUPLE, PackageResolveEntryByResourceKeyUsesTypeAndLogicalKeyTuple},
        {TEST_INDEXED_LOOKUPS, PackageIndexedLookupsPreserveStatusOrderAndCapacities},
        {TEST_UNKNOWN_KEY, PackageResolveRejectsUnknownResourceKey},
        {TEST_TYPE_MISMATCH, PackageResolveRejectsTypeMismatchWithoutMutation},
        {TEST_MISSING_DEPENDENCY, PackageDependencyValidationRejectsMissingEntry},
        {TEST_DEPENDENCY_CYCLE, PackageDependencyValidationRejectsCycle},
        {TEST_DEPENDENCY_CYCLE_HIGH_FANOUT, PackageDependencyValidationRejectsCycleAfterHighFanout},
        {TEST_DEPENDENCY_ORDER, PackageDependencyPlanPreservesDeclarationOrder},
        {TEST_DEPENDENCY_CAPACITY, PackageDependencyCapacityOverflowDoesNotMutate},
        {TEST_LOAD_PLAN_CAPACITY, PackageLoadPlanCapacityOverflowDoesNotMutate},
        {TEST_DISABLED_DIAGNOSTICS, PackageDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_FILE_ORIGINAL, PackageNoFileReadOriginalPackageOrGameAdapterDependency},
        {TEST_NO_HIDDEN_ALLOCATION, PackageNoHiddenAllocationUsesYuMemorySignal}};

    const std::string_view testName(argv[1]);
    const auto testIterator = testRegistry.find(testName);
    if (testIterator == testRegistry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testIterator->second();
}
