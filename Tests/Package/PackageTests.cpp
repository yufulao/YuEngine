// 模块：Tests Package
// 文件：Tests/Package/PackageTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/File/MountTable.h"
#include "YuEngine/File/VirtualPath.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Package/PackageArtifact.h"
#include "YuEngine/Package/PackageConstants.h"
#include "YuEngine/Package/PackageRegistry.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceTypeId.h"

using yuengine::file::MountId;
using yuengine::file::MountTable;
using yuengine::file::VirtualPath;
using yuengine::memory::MemoryAccountingStatus;
using yuengine::package::PackageArtifactDependency;
using yuengine::package::PackageArtifactReadRequest;
using yuengine::package::PackageArtifactResult;
using yuengine::package::PackageArtifactWriteRequest;
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
using yuengine::package::ReadPackageArtifact;
using yuengine::package::WritePackageArtifact;
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
constexpr const char* TEST_ARTIFACT_ROUND_TRIP = "Package_FileBackedArtifactRoundTripsLoadPlanThroughFileVfs";
constexpr const char* TEST_ARTIFACT_INVALID = "Package_FileBackedArtifactRejectsInvalidBytesWithoutMutation";
constexpr const char* TEST_ARTIFACT_MISSING = "Package_FileBackedArtifactReportsMissingFile";
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
constexpr const char* PACKAGE_ARTIFACT_MOUNT = "package_artifact_test";
constexpr const char* PACKAGE_ARTIFACT_PATH = "Packages/RuntimeAssetSample.yupackage";

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

std::filesystem::path PackageArtifactRoot(std::string_view test_name) {
    std::filesystem::path root = std::filesystem::temp_directory_path();
    root /= "YuEnginePackageArtifactTests";
    root /= std::string(test_name);
    return root;
}

MountTable CreatePackageArtifactTable(std::string_view test_name) {
    const std::filesystem::path root = PackageArtifactRoot(test_name);
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    MountTable table;
    table.RegisterLooseMount(MountId(PACKAGE_ARTIFACT_MOUNT), root);
    return table;
}

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

PackageEntryDescriptor Entry(
    PackageId package,
    PackageEntryId entry,
    ResourceTypeId type,
    const char* logical_key,
    const char* source_key,
    std::uint32_t byte_offset = 0U,
    std::uint32_t byte_size = 16U) {
    return PackageEntryDescriptor{
        package,
        entry,
        type,
        ResourceLogicalKey(logical_key),
        PackageSourceKey(source_key),
        byte_offset,
        byte_size};
}

PackageRegistrationResult RegisterManifest(PackageRegistry& registry, PackageId package = PACKAGE_A) {
    return registry.RegisterSyntheticManifest({package});
}

PackageRegistrationResult RegisterEntry(
    PackageRegistry& registry,
    PackageEntryId entry,
    ResourceTypeId type,
    const char* logical_key,
    const char* source_key,
    std::uint32_t byte_offset = 0U,
    std::uint32_t byte_size = 16U) {
    return registry.RegisterEntry(Entry(PACKAGE_A, entry, type, logical_key, source_key, byte_offset, byte_size));
}

bool CoreCountsMatch(const PackageSnapshot& left, const PackageSnapshot& right) {
    if (left.manifest_count != right.manifest_count) {
        return false;
    }

    if (left.entry_count != right.entry_count) {
        return false;
    }

    if (left.dependency_edge_count != right.dependency_edge_count) {
        return false;
    }

    if (left.dependency_validation_count != right.dependency_validation_count) {
        return false;
    }

    if (left.load_plan_resolve_count != right.load_plan_resolve_count) {
        return false;
    }

    return left.last_load_plan_record_count == right.last_load_plan_record_count;
}

bool SnapshotsMatch(const PackageSnapshot& left, const PackageSnapshot& right) {
    if (!CoreCountsMatch(left, right)) {
        return false;
    }

    if (left.rejected_operation_count != right.rejected_operation_count) {
        return false;
    }

    return left.allocation_accounting_status == right.allocation_accounting_status;
}

int PackageRegisterSyntheticManifestReturnsStableId() {
    PackageRegistry registry;
    const PackageRegistrationResult result = RegisterManifest(registry);
    if (!result.Succeeded()) {
        return Fail("synthetic manifest did not register");
    }

    if (result.package.value != PACKAGE_A.value) {
        return Fail("manifest result did not return stable package id");
    }

    const PackageSnapshot snapshot = registry.Snapshot();
    if (snapshot.manifest_count != 1U) {
        return Fail("manifest count was not recorded");
    }

    if (snapshot.last_status != PackageStatus::Success) {
        return Fail("manifest success did not record success status");
    }

    return 0;
}

int PackageRegisterDuplicateManifestReturnsExplicitStatus() {
    PackageRegistry registry;
    if (!RegisterManifest(registry).Succeeded()) {
        return Fail("first manifest registration failed");
    }

    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageRegistrationResult duplicate = RegisterManifest(registry);
    if (duplicate.status != PackageStatus::DuplicateManifest) {
        return Fail("duplicate manifest did not return explicit status");
    }

    if (registry.Snapshot().manifest_count != before_snapshot.manifest_count) {
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

    if (result.entry.value != ENTRY_TEXTURE.value) {
        return Fail("entry result did not return stable entry id");
    }

    if (registry.Snapshot().entry_count != 1U) {
        return Fail("entry count was not recorded");
    }

    return 0;
}

int PackageRegisterDuplicateEntryReturnsExplicitStatus() {
    PackageRegistry registry;
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");

    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageRegistrationResult duplicate =
        RegisterEntry(registry, ENTRY_TEXTURE, TYPE_MATERIAL, "material_a", "materials/material_a.bin");
    if (duplicate.status != PackageStatus::DuplicateEntry) {
        return Fail("duplicate entry did not return explicit status");
    }

    if (registry.Snapshot().entry_count != before_snapshot.entry_count) {
        return Fail("duplicate entry changed entry count");
    }

    return 0;
}

int PackageRegisterDuplicateResourceKeyReturnsExplicitStatus() {
    PackageRegistry registry;
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");

    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageRegistrationResult duplicate = RegisterEntry(
        registry,
        ENTRY_MATERIAL,
        TYPE_TEXTURE,
        "texture_a",
        "textures/texture_a_copy.bin");
    if (duplicate.status != PackageStatus::DuplicateResourceKey) {
        return Fail("duplicate resource key did not return explicit status");
    }

    if (registry.Snapshot().entry_count != before_snapshot.entry_count) {
        return Fail("duplicate resource key changed entry count");
    }

    return 0;
}

int PackageRegisterInvalidIdsOrTypeReturnsExplicitStatusWithoutMutation() {
    PackageRegistry registry;
    RegisterManifest(registry);

    const PackageSnapshot before_snapshot = registry.Snapshot();
    if (registry.RegisterSyntheticManifest({PackageId{}}).status != PackageStatus::InvalidPackageId) {
        return Fail("invalid package id did not return explicit status");
    }

    if (registry.RegisterEntry(Entry(PackageId{}, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "a.bin")).status !=
        PackageStatus::InvalidPackageId) {
        return Fail("entry invalid package id did not return explicit status");
    }

    if (registry.RegisterEntry(Entry(PACKAGE_A, PackageEntryId{}, TYPE_TEXTURE, "texture_a", "a.bin")).status !=
        PackageStatus::InvalidEntryId) {
        return Fail("invalid entry id did not return explicit status");
    }

    if (registry.RegisterEntry(Entry(PACKAGE_A, ENTRY_TEXTURE, ResourceTypeId{}, "texture_a", "a.bin")).status !=
        PackageStatus::InvalidResourceType) {
        return Fail("invalid resource type did not return explicit status");
    }

    if (registry.RegisterEntry(Entry(PACKAGE_A, ENTRY_TEXTURE, TYPE_TEXTURE, "Texture_A", "a.bin")).status !=
        PackageStatus::InvalidLogicalKey) {
        return Fail("invalid logical key did not return explicit status");
    }

    if (!CoreCountsMatch(before_snapshot, registry.Snapshot())) {
        return Fail("invalid descriptor changed core package counters");
    }

    return 0;
}

int PackageManifestCapacityOverflowDoesNotMutate() {
    PackageRegistry registry(PackageRegistryDesc{1U, 4U, 4U, 4U});
    RegisterManifest(registry, PACKAGE_A);

    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageRegistrationResult overflow = RegisterManifest(registry, PACKAGE_B);
    if (overflow.status != PackageStatus::ManifestCapacityExceeded) {
        return Fail("manifest capacity overflow did not return explicit status");
    }

    if (registry.Snapshot().manifest_count != before_snapshot.manifest_count) {
        return Fail("manifest capacity overflow changed manifest count");
    }

    return 0;
}

int PackageEntryCapacityOverflowDoesNotMutate() {
    PackageRegistry registry(PackageRegistryDesc{1U, 1U, 4U, 4U});
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");

    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageRegistrationResult overflow =
        RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin");
    if (overflow.status != PackageStatus::EntryCapacityExceeded) {
        return Fail("entry capacity overflow did not return explicit status");
    }

    if (registry.Snapshot().entry_count != before_snapshot.entry_count) {
        return Fail("entry capacity overflow changed entry count");
    }

    return 0;
}

int PackageRegisterEntryRejectsOversizedKeysWithoutMutation() {
    PackageRegistry registry;
    RegisterManifest(registry);
    const PackageSnapshot before_snapshot = registry.Snapshot();

    const std::string long_logical_key(MAX_LOGICAL_KEY_BYTES + 1U, 'a');
    const PackageRegistrationResult long_logical = registry.RegisterEntry(
        Entry(PACKAGE_A, ENTRY_TEXTURE, TYPE_TEXTURE, long_logical_key.c_str(), "textures/texture_a.bin"));
    if (long_logical.status != PackageStatus::LogicalKeyTooLong) {
        return Fail("oversized logical key did not return explicit status");
    }

    const std::string long_source_key(MAX_PACKAGE_SOURCE_KEY_BYTES + 1U, 'a');
    const PackageRegistrationResult long_source =
        registry.RegisterEntry(Entry(PACKAGE_A, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", long_source_key.c_str()));
    if (long_source.status != PackageStatus::SourceKeyTooLong) {
        return Fail("oversized source key did not return explicit status");
    }

    if (registry.Snapshot().entry_count != before_snapshot.entry_count) {
        return Fail("oversized keys changed entry count");
    }

    return 0;
}

int PackageRegisterEntryRejectsOversizedByteRangeWithoutMutation() {
    PackageRegistry registry;
    RegisterManifest(registry);
    const PackageSnapshot before_snapshot = registry.Snapshot();

    const PackageRegistrationResult too_large = RegisterEntry(
        registry,
        ENTRY_TEXTURE,
        TYPE_TEXTURE,
        "texture_a",
        "textures/texture_a.bin",
        0U,
        MAX_DECLARED_ENTRY_SIZE + 1U);
    if (too_large.status != PackageStatus::ByteRangeOutOfBounds) {
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
    if (overflow.status != PackageStatus::ByteRangeOutOfBounds) {
        return Fail("overflow byte range did not return explicit status");
    }

    if (registry.Snapshot().entry_count != before_snapshot.entry_count) {
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

    if (result.plan.record_count != 3U) {
        return Fail("load plan did not include dependencies plus root");
    }

    if (result.plan.records[0U].entry.value != ENTRY_MATERIAL.value) {
        return Fail("first dependency record was not declaration-order material");
    }

    if (result.plan.records[1U].entry.value != ENTRY_AUDIO.value) {
        return Fail("second dependency record was not declaration-order audio");
    }

    if (result.plan.records[2U].entry.value != ENTRY_TEXTURE.value) {
        return Fail("root record was not last");
    }

    if (registry.Snapshot().last_load_plan_record_count != 3U) {
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

    if (result.plan.record_count != 1U) {
        return Fail("same-key different-type resolve returned unexpected record count");
    }

    if (result.plan.records[0U].entry.value != ENTRY_MATERIAL.value) {
        return Fail("same-key different-type resolve returned the wrong entry");
    }

    if (result.plan.records[0U].type.value != TYPE_MATERIAL.value) {
        return Fail("same-key different-type resolve returned the wrong type");
    }

    if (registry.Snapshot().last_load_plan_record_count != 1U) {
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

    const PackageRegistrationResult material_result =
        RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, INDEX_MATERIAL_LOGICAL, INDEX_MATERIAL_SOURCE);
    if (!material_result.Succeeded()) {
        return Fail(ERROR_INDEX_MATERIAL_FAILED);
    }

    if (registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_AUDIO) != PackageStatus::Success) {
        return Fail(ERROR_INDEX_AUDIO_EDGE_FAILED);
    }

    if (registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL) != PackageStatus::Success) {
        return Fail(ERROR_INDEX_MATERIAL_EDGE_FAILED);
    }

    const PackageSnapshot before_duplicate = registry.Snapshot();
    if (registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_AUDIO) != PackageStatus::Success) {
        return Fail(ERROR_INDEX_DUPLICATE_EDGE_FAILED);
    }

    if (registry.Snapshot().dependency_edge_count != before_duplicate.dependency_edge_count) {
        return Fail(ERROR_INDEX_DUPLICATE_EDGE_MUTATED);
    }

    const PackageSnapshot before_mismatch = registry.Snapshot();
    const PackageLoadPlanResult mismatch =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_AUDIO, ResourceLogicalKey(INDEX_TEXTURE_LOGICAL));
    if (mismatch.status != PackageStatus::TypeMismatch) {
        return Fail(ERROR_INDEX_TYPE_MISMATCH_STATUS);
    }

    if (registry.Snapshot().load_plan_resolve_count != before_mismatch.load_plan_resolve_count) {
        return Fail(ERROR_INDEX_TYPE_MISMATCH_MUTATED);
    }

    const PackageSnapshot before_resolve = registry.Snapshot();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey(INDEX_TEXTURE_LOGICAL));
    if (!result.Succeeded()) {
        return Fail(ERROR_INDEX_RESOLVE_FAILED);
    }

    if (result.plan.record_count != 3U) {
        return Fail(ERROR_INDEX_RECORD_COUNT);
    }

    if (result.plan.records[0U].entry.value != ENTRY_AUDIO.value) {
        return Fail(ERROR_INDEX_FIRST_DEPENDENCY);
    }

    if (result.plan.records[1U].entry.value != ENTRY_MATERIAL.value) {
        return Fail(ERROR_INDEX_SECOND_DEPENDENCY);
    }

    if (result.plan.records[2U].entry.value != ENTRY_TEXTURE.value) {
        return Fail(ERROR_INDEX_ROOT_RECORD);
    }

    const PackageSnapshot after_resolve = registry.Snapshot();
    if (after_resolve.manifest_capacity != before_resolve.manifest_capacity) {
        return Fail(ERROR_INDEX_MANIFEST_CAPACITY);
    }

    if (after_resolve.entry_capacity != before_resolve.entry_capacity) {
        return Fail(ERROR_INDEX_ENTRY_CAPACITY);
    }

    if (after_resolve.dependency_edge_capacity != before_resolve.dependency_edge_capacity) {
        return Fail(ERROR_INDEX_DEPENDENCY_CAPACITY);
    }

    if (after_resolve.load_plan_record_capacity != before_resolve.load_plan_record_capacity) {
        return Fail(ERROR_INDEX_LOAD_PLAN_CAPACITY);
    }

    return 0;
}

int PackageResolveRejectsUnknownResourceKey() {
    PackageRegistry registry = CreateResolvedRegistry();
    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("missing_texture"));
    if (result.status != PackageStatus::NotFound) {
        return Fail("unknown resource key did not return explicit status");
    }

    if (registry.Snapshot().load_plan_resolve_count != before_snapshot.load_plan_resolve_count) {
        return Fail("unknown resource key changed load-plan resolve count");
    }

    return 0;
}

int PackageResolveRejectsTypeMismatchWithoutMutation() {
    PackageRegistry registry = CreateResolvedRegistry();
    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_AUDIO, ResourceLogicalKey("texture_a"));
    if (result.status != PackageStatus::TypeMismatch) {
        return Fail("type mismatch did not return explicit status");
    }

    if (registry.Snapshot().load_plan_resolve_count != before_snapshot.load_plan_resolve_count) {
        return Fail("type mismatch changed load-plan resolve count");
    }

    if (registry.Snapshot().last_load_plan_record_count != before_snapshot.last_load_plan_record_count) {
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
    if (snapshot.dependency_edge_count != 0U) {
        return Fail("missing dependency changed dependency edge count");
    }

    if (snapshot.dependency_validation_count != 1U) {
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

    if (registry.Snapshot().dependency_edge_count != 2U) {
        return Fail("cycle dependency changed dependency edge count");
    }

    return 0;
}

int PackageDependencyValidationRejectsCycleAfterHighFanout() {
    constexpr PackageEntryId START{1U};
    constexpr PackageEntryId TARGET{2U};
    constexpr PackageEntryId BRANCH{32U};

    PackageRegistry registry;
    RegisterManifest(registry);
    for (std::uint32_t index = 1U; index <= MAX_PACKAGE_ENTRY_COUNT; ++index) {
        const std::string suffix = std::to_string(index);
        const std::string logical_key = "entry_" + suffix;
        const std::string source_key = "package/entry_" + suffix + ".bin";
        const PackageRegistrationResult result =
            registry.RegisterEntry(Entry(PACKAGE_A, PackageEntryId{index}, TYPE_TEXTURE, logical_key.c_str(), source_key.c_str()));
        if (!result.Succeeded()) {
            return Fail("high-fanout cycle fixture failed to register entries");
        }
    }

    for (std::uint32_t index = 3U; index <= MAX_PACKAGE_ENTRY_COUNT; ++index) {
        if (registry.AddDependency(PACKAGE_A, START, PackageEntryId{index}) != PackageStatus::Success) {
            return Fail("high-fanout cycle fixture failed to add start edge");
        }
    }

    if (registry.AddDependency(PACKAGE_A, BRANCH, PackageEntryId{3U}) != PackageStatus::Success) {
        return Fail("high-fanout cycle fixture failed to add duplicate pending edge");
    }

    if (registry.AddDependency(PACKAGE_A, BRANCH, PackageEntryId{4U}) != PackageStatus::Success) {
        return Fail("high-fanout cycle fixture failed to add second duplicate pending edge");
    }

    if (registry.AddDependency(PACKAGE_A, BRANCH, PackageEntryId{5U}) != PackageStatus::Success) {
        return Fail("high-fanout cycle fixture failed to add third duplicate pending edge");
    }

    if (registry.AddDependency(PACKAGE_A, BRANCH, TARGET) != PackageStatus::Success) {
        return Fail("high-fanout cycle fixture failed to add target path edge");
    }

    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageStatus cycle_status = registry.AddDependency(PACKAGE_A, TARGET, START);
    if (cycle_status != PackageStatus::DependencyCycle) {
        return Fail("high-fanout dependency path did not return explicit cycle status");
    }

    if (registry.Snapshot().dependency_edge_count != before_snapshot.dependency_edge_count) {
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

    if (result.plan.records[0U].entry.value != ENTRY_AUDIO.value) {
        return Fail("first dependency did not preserve declaration order");
    }

    if (result.plan.records[1U].entry.value != ENTRY_MATERIAL.value) {
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

    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageStatus overflow = registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_AUDIO);
    if (overflow != PackageStatus::DependencyCapacityExceeded) {
        return Fail("dependency capacity overflow did not return explicit status");
    }

    if (registry.Snapshot().dependency_edge_count != before_snapshot.dependency_edge_count) {
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

    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (result.status != PackageStatus::LoadPlanCapacityExceeded) {
        return Fail("load-plan capacity overflow did not return explicit status");
    }

    if (registry.Snapshot().load_plan_resolve_count != before_snapshot.load_plan_resolve_count) {
        return Fail("load-plan capacity overflow changed resolve count");
    }

    if (registry.Snapshot().last_load_plan_record_count != before_snapshot.last_load_plan_record_count) {
        return Fail("load-plan capacity overflow changed plan record count");
    }

    return 0;
}

int PackageDisabledDiagnosticsDoesNotChangeResults() {
    PackageRegistry recording_registry = CreateResolvedRegistry();
    PackageRegistry disabled_registry = CreateResolvedRegistry();

    const PackageLoadPlanResult recording_result =
        recording_registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    const PackageLoadPlanResult disabled_result =
        disabled_registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (recording_result.status != disabled_result.status) {
        return Fail("disabled diagnostics changed resolve status");
    }

    const PackageLoadPlanResult recording_failure =
        recording_registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_AUDIO, ResourceLogicalKey("texture_a"));
    const PackageLoadPlanResult disabled_failure =
        disabled_registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_AUDIO, ResourceLogicalKey("texture_a"));
    if (recording_failure.status != disabled_failure.status) {
        return Fail("disabled diagnostics changed failure status");
    }

    if (!SnapshotsMatch(recording_registry.Snapshot(), disabled_registry.Snapshot())) {
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

    if (result.plan.records[2U].source_key.Value() != std::string_view("textures/texture_a.bin")) {
        return Fail("synthetic source-key metadata was not preserved");
    }

    return 0;
}

int PackageNoHiddenAllocationUsesYuMemorySignal() {
    PackageRegistry registry = CreateResolvedRegistry();
    const PackageSnapshot before_snapshot = registry.Snapshot();
    if (before_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("package registry did not expose YuMemory accounting vocabulary");
    }

    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (!result.Succeeded()) {
        return Fail("resolve failed");
    }

    const PackageSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.manifest_capacity != before_snapshot.manifest_capacity) {
        return Fail("manifest capacity changed during resolve");
    }

    if (after_snapshot.entry_capacity != before_snapshot.entry_capacity) {
        return Fail("entry capacity changed during resolve");
    }

    if (after_snapshot.dependency_edge_capacity != before_snapshot.dependency_edge_capacity) {
        return Fail("dependency capacity changed during resolve");
    }

    if (after_snapshot.load_plan_record_capacity != before_snapshot.load_plan_record_capacity) {
        return Fail("load-plan capacity changed during resolve");
    }

    if (after_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("package registry changed allocation accounting vocabulary");
    }

    return 0;
}

int PackageFileBackedArtifactRoundTripsLoadPlanThroughFileVfs() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_ROUND_TRIP);
    const std::array<PackageEntryDescriptor, 3U> entries{
        Entry(PACKAGE_A, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin", 0U, 16U),
        Entry(PACKAGE_A, ENTRY_AUDIO, TYPE_AUDIO, "audio_a", "audio/audio_a.bin", 16U, 32U),
        Entry(PACKAGE_A, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin", 48U, 24U)};
    const std::array<PackageArtifactDependency, 2U> dependencies{
        PackageArtifactDependency{ENTRY_TEXTURE, ENTRY_AUDIO},
        PackageArtifactDependency{ENTRY_TEXTURE, ENTRY_MATERIAL}};

    PackageArtifactWriteRequest write_request{};
    write_request.mount_table = &table;
    write_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    write_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    write_request.package = PACKAGE_A;
    write_request.entries = entries.data();
    write_request.entry_count = static_cast<std::uint32_t>(entries.size());
    write_request.dependencies = dependencies.data();
    write_request.dependency_count = static_cast<std::uint32_t>(dependencies.size());
    const PackageArtifactResult write_result = WritePackageArtifact(write_request);
    if (write_result.status != PackageStatus::Success ||
        !write_result.wrote_artifact ||
        write_result.artifact_byte_count == 0U) {
        return Fail("package artifact did not write through File/VFS");
    }

    PackageRegistry artifact_registry;
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = &table;
    read_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    read_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    read_request.registry = &artifact_registry;
    const PackageArtifactResult read_result = ReadPackageArtifact(read_request);
    if (read_result.status != PackageStatus::Success ||
        !read_result.read_artifact ||
        !read_result.rebuilt_registry ||
        read_result.entry_count != entries.size() ||
        read_result.dependency_count != dependencies.size()) {
        return Fail("package artifact did not rebuild registry from File/VFS bytes");
    }

    const PackageLoadPlanResult plan =
        artifact_registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (!plan.Succeeded() || plan.plan.record_count != 3U) {
        return Fail("file-backed artifact did not resolve deterministic load plan");
    }

    if (plan.plan.records[0U].entry.value != ENTRY_AUDIO.value ||
        plan.plan.records[1U].entry.value != ENTRY_MATERIAL.value ||
        plan.plan.records[2U].entry.value != ENTRY_TEXTURE.value) {
        return Fail("file-backed artifact load plan did not preserve dependency order");
    }

    if (table.Snapshot().write_byte_count == 0U || table.Snapshot().read_byte_count == 0U) {
        return Fail("package artifact did not exercise MountTable read/write counters");
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_ROUND_TRIP));
    return 0;
}

int PackageFileBackedArtifactRejectsInvalidBytesWithoutMutation() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_INVALID);
    const std::string invalid_artifact = "not-a-package-artifact\n";
    const auto write_result = table.Write({
        MountId(PACKAGE_ARTIFACT_MOUNT),
        VirtualPath(PACKAGE_ARTIFACT_PATH),
        reinterpret_cast<const std::uint8_t *>(invalid_artifact.data()),
        invalid_artifact.size()});
    if (!write_result.Succeeded()) {
        return Fail("invalid artifact fixture write failed");
    }

    PackageRegistry registry;
    if (!RegisterManifest(registry).Succeeded()) {
        return Fail("registry fixture manifest failed");
    }

    const PackageSnapshot before_snapshot = registry.Snapshot();
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = &table;
    read_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    read_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    read_request.registry = &registry;
    const PackageArtifactResult read_result = ReadPackageArtifact(read_request);
    if (read_result.status != PackageStatus::InvalidArtifact ||
        read_result.read_artifact ||
        read_result.rebuilt_registry) {
        return Fail("invalid package artifact did not report explicit parse failure");
    }

    if (!SnapshotsMatch(before_snapshot, registry.Snapshot())) {
        return Fail("invalid package artifact mutated caller registry");
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_INVALID));
    return 0;
}

int PackageFileBackedArtifactReportsMissingFile() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_MISSING);
    PackageRegistry registry;
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = &table;
    read_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    read_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    read_request.registry = &registry;
    const PackageArtifactResult read_result = ReadPackageArtifact(read_request);
    if (read_result.status != PackageStatus::FileReadFailed ||
        read_result.read_artifact ||
        read_result.rebuilt_registry) {
        return Fail("missing package artifact did not report file read failure");
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_MISSING));
    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
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
        {TEST_NO_HIDDEN_ALLOCATION, PackageNoHiddenAllocationUsesYuMemorySignal},
        {TEST_ARTIFACT_ROUND_TRIP, PackageFileBackedArtifactRoundTripsLoadPlanThroughFileVfs},
        {TEST_ARTIFACT_INVALID, PackageFileBackedArtifactRejectsInvalidBytesWithoutMutation},
        {TEST_ARTIFACT_MISSING, PackageFileBackedArtifactReportsMissingFile}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
