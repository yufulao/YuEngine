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
using yuengine::package::PackageLoadPlanRecord;
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
using yuengine::package::MAX_PACKAGE_DEPENDENCY_EDGE_COUNT;
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
constexpr const char* TEST_REGISTRATION_CAPACITY_IDENTITY =
    "Package_RegistrationCapacityFailureIdentityClearsForNonCapacityFailures";
constexpr const char* TEST_OVERSIZED_KEYS = "Package_RegisterEntryRejectsOversizedKeysWithoutMutation";
constexpr const char* TEST_OVERSIZED_BYTE_RANGE = "Package_RegisterEntryRejectsOversizedByteRangeWithoutMutation";
constexpr const char* TEST_LARGE_BYTE_RANGE_INDEX =
    "Package_RegisterEntryAcceptsLargeByteRangeAndResolvePreservesIndexMetadata";
constexpr const char* TEST_PAYLOAD_METADATA_LOAD_PLAN =
    "Package_LoadPlanPreservesPayloadWindowMetadataDistinctFromArchiveRange";
constexpr const char* TEST_RESOLVE = "Package_ResolveEntryByResourceKey_ReturnsDeterministicLoadPlan";
constexpr const char* TEST_RESOLVE_RESOURCE_KEY_TUPLE = "Package_ResolveEntryByResourceKey_UsesTypeAndLogicalKeyTuple";
constexpr const char* TEST_INDEXED_LOOKUPS = "Package_IndexedLookupsPreserveStatusOrderAndCapacities";
constexpr const char* TEST_ACCEPTED_STATUS =
    "Package_AcceptedOperationCountTracksSuccessAfterFailureAndDuplicateDependency";
constexpr const char* TEST_UNKNOWN_KEY = "Package_ResolveRejectsUnknownResourceKey";
constexpr const char* TEST_TYPE_MISMATCH = "Package_ResolveRejectsTypeMismatchWithoutMutation";
constexpr const char* TEST_MISSING_DEPENDENCY = "Package_DependencyValidationRejectsMissingEntry";
constexpr const char* TEST_DEPENDENCY_CYCLE = "Package_DependencyValidationRejectsCycle";
constexpr const char* TEST_DEPENDENCY_CYCLE_HIGH_FANOUT = "Package_DependencyValidationRejectsCycleAfterHighFanout";
constexpr const char* TEST_DEPENDENCY_ORDER = "Package_DependencyPlanPreservesDeclarationOrder";
constexpr const char* TEST_DEPENDENCY_TRANSITIVE_CLOSURE =
    "Package_DependencyClosurePlanIncludesTransitiveDependenciesBeforeRoot";
constexpr const char* TEST_DEPENDENCY_CLOSURE_DEDUP =
    "Package_DependencyClosureDeduplicatesSharedDependenciesAndPreservesDeclarationOrder";
constexpr const char* TEST_DEPENDENCY_CLOSURE_RECORD_BUDGET =
    "Package_DependencyClosureRejectsRecordBudgetWithoutMutation";
constexpr const char* TEST_DEPENDENCY_CAPACITY = "Package_DependencyCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_LOAD_PLAN_CAPACITY = "Package_LoadPlanCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_LOAD_PLAN_BYTE_BUDGET = "Package_LoadPlanRejectsArchiveByteBudgetWithoutMutation";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Package_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_FILE_ORIGINAL = "Package_NoFileReadOriginalPackageOrGameAdapterDependency";
constexpr const char* TEST_NO_HIDDEN_ALLOCATION = "Package_NoHiddenAllocation_UsesYuMemorySignal";
constexpr const char* TEST_ARTIFACT_ROUND_TRIP = "Package_FileBackedArtifactRoundTripsLoadPlanThroughFileVfs";
constexpr const char* TEST_ARTIFACT_LARGE_BYTE_RANGE =
    "Package_FileBackedArtifactRoundTripsLargeByteRangeMetadata";
constexpr const char* TEST_ARTIFACT_PAYLOAD_METADATA =
    "Package_FileBackedArtifactRoundTripsPayloadWindowMetadata";
constexpr const char* TEST_ARTIFACT_LEGACY_SEVEN_FIELD =
    "Package_FileBackedArtifactReadsLegacySevenFieldRowsWithDefaultPayloadMetadata";
constexpr const char* TEST_ARTIFACT_LEGACY_NINE_FIELD =
    "Package_FileBackedArtifactReadsLegacyNineFieldRowsWithDefaultPayloadMetadata";
constexpr const char* TEST_ARTIFACT_HASH_ROUND_TRIP =
    "Package_FileBackedArtifactRoundTripsHashAndDependencyIntegrity";
constexpr const char* TEST_ARTIFACT_HASH_VALIDATION =
    "Package_FileBackedArtifactRejectsHashMismatchesWithoutMutation";
constexpr const char* TEST_ARTIFACT_INVALID = "Package_FileBackedArtifactRejectsInvalidBytesWithoutMutation";
constexpr const char* TEST_ARTIFACT_MANIFEST_VALIDATION =
    "Package_FileBackedArtifactRejectsManifestParseAndSectionErrorsWithoutMutation";
constexpr const char* TEST_ARTIFACT_ENTRY_VALIDATION =
    "Package_FileBackedArtifactRejectsEntryMetadataWithoutMutation";
constexpr const char* TEST_ARTIFACT_DEPENDENCY_VALIDATION =
    "Package_FileBackedArtifactRejectsDependencyMetadataWithoutMutation";
constexpr const char* TEST_ARTIFACT_CAPACITY_ENTRY_IDENTITY =
    "Package_FileBackedArtifactReportsCapacityEntryIdentity";
constexpr const char* TEST_ARTIFACT_WRITE_INVALID =
    "Package_FileBackedArtifactWriteRejectsInvalidMetadataWithoutFileWrite";
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
constexpr const char* PACKAGE_ARTIFACT_MAGIC = "YUPACKAGE_ARTIFACT_V1";
constexpr std::uint64_t ARTIFACT_HASH_OFFSET = 14695981039346656037ULL;
constexpr std::uint64_t ARTIFACT_HASH_MULTIPLIER = 1099511628211ULL;

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
constexpr std::uint64_t LARGE_ARCHIVE_BYTE_OFFSET = 0x100000400ULL;
constexpr std::uint64_t PAYLOAD_LOGICAL_BYTE_COUNT = 0x100000800ULL;
constexpr std::uint64_t PAYLOAD_WINDOW_BYTE_OFFSET = 0x100000040ULL;
constexpr std::uint64_t PAYLOAD_WINDOW_BYTE_SIZE = 128ULL;
constexpr std::uint32_t ARTIFACT_CAPACITY_ENTRY_TEST_COUNT = MAX_PACKAGE_ENTRY_COUNT + 1U;
constexpr std::uint32_t ARTIFACT_CAPACITY_DEPENDENCY_TEST_COUNT = MAX_PACKAGE_DEPENDENCY_EDGE_COUNT + 1U;
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

bool WriteRawPackageArtifact(MountTable& table, std::string_view artifact_text) {
    const auto write_result = table.Write({
        MountId(PACKAGE_ARTIFACT_MOUNT),
        VirtualPath(PACKAGE_ARTIFACT_PATH),
        reinterpret_cast<const std::uint8_t *>(artifact_text.data()),
        artifact_text.size()});
    return write_result.Succeeded();
}

std::string ReadRawPackageArtifact(MountTable& table) {
    const auto read_result = table.Read({MountId(PACKAGE_ARTIFACT_MOUNT), VirtualPath(PACKAGE_ARTIFACT_PATH)});
    if (!read_result.Succeeded()) {
        return std::string();
    }

    return std::string(
        reinterpret_cast<const char *>(read_result.bytes.data()),
        read_result.bytes.size());
}

bool ReplaceHashLineValue(std::string *artifact_text, std::string_view prefix, std::string_view replacement) {
    if (artifact_text == nullptr) {
        return false;
    }

    const std::string search_text = "\n" + std::string(prefix);
    std::size_t line_start = artifact_text->find(search_text);
    if (line_start == std::string::npos) {
        return false;
    }

    line_start += search_text.size();
    const std::size_t line_end = artifact_text->find('\n', line_start);
    if (line_end == std::string::npos) {
        return false;
    }

    artifact_text->replace(line_start, line_end - line_start, replacement);
    return true;
}

std::uint64_t MixArtifactHash(std::uint64_t hash, std::uint64_t value) {
    hash ^= value;
    return hash * ARTIFACT_HASH_MULTIPLIER;
}

std::uint64_t HashArtifactText(std::uint64_t hash, std::string_view text) {
    for (const char character : text) {
        hash = MixArtifactHash(hash, static_cast<std::uint64_t>(static_cast<unsigned char>(character)));
    }

    return hash;
}

std::uint64_t MakeNonZeroArtifactHash(std::uint64_t hash) {
    if (hash == 0ULL) {
        return ARTIFACT_HASH_OFFSET;
    }

    return hash;
}

std::uint64_t LegacyEntryPayloadHash(std::string_view source_key, std::uint64_t byte_offset, std::uint64_t byte_size) {
    std::uint64_t hash = ARTIFACT_HASH_OFFSET;
    hash = HashArtifactText(hash, source_key);
    hash = MixArtifactHash(hash, byte_offset);
    hash = MixArtifactHash(hash, byte_size);
    return MakeNonZeroArtifactHash(hash);
}

std::uint64_t LegacyEntryMetadataHash(
    PackageEntryId entry,
    ResourceTypeId type,
    std::string_view logical_key,
    std::string_view source_key,
    std::uint64_t byte_offset,
    std::uint64_t byte_size,
    std::uint64_t payload_hash) {
    std::uint64_t hash = ARTIFACT_HASH_OFFSET;
    hash = MixArtifactHash(hash, PACKAGE_A.value);
    hash = MixArtifactHash(hash, entry.value);
    hash = MixArtifactHash(hash, type.value);
    hash = HashArtifactText(hash, logical_key);
    hash = HashArtifactText(hash, source_key);
    hash = MixArtifactHash(hash, byte_offset);
    hash = MixArtifactHash(hash, byte_size);
    hash = MixArtifactHash(hash, payload_hash);
    return MakeNonZeroArtifactHash(hash);
}

std::uint64_t LegacyDependencyTableHash(std::uint32_t dependency_count) {
    std::uint64_t hash = ARTIFACT_HASH_OFFSET;
    hash = MixArtifactHash(hash, dependency_count);
    return MakeNonZeroArtifactHash(hash);
}

std::uint64_t LegacyPackageTableHash(
    PackageEntryId entry,
    std::uint64_t payload_hash,
    std::uint64_t metadata_hash,
    std::uint64_t dependency_table_hash) {
    std::uint64_t hash = ARTIFACT_HASH_OFFSET;
    hash = MixArtifactHash(hash, PACKAGE_A.value);
    hash = MixArtifactHash(hash, 1U);
    hash = MixArtifactHash(hash, 0U);
    hash = MixArtifactHash(hash, entry.value);
    hash = MixArtifactHash(hash, payload_hash);
    hash = MixArtifactHash(hash, metadata_hash);
    hash = MixArtifactHash(hash, dependency_table_hash);
    return MakeNonZeroArtifactHash(hash);
}

std::string BuildLegacyNineFieldArtifact(
    PackageEntryId entry,
    ResourceTypeId type,
    std::string_view logical_key,
    std::string_view source_key,
    std::uint64_t byte_offset,
    std::uint64_t byte_size) {
    const std::uint64_t payload_hash = LegacyEntryPayloadHash(source_key, byte_offset, byte_size);
    const std::uint64_t metadata_hash =
        LegacyEntryMetadataHash(entry, type, logical_key, source_key, byte_offset, byte_size, payload_hash);
    const std::uint64_t dependency_table_hash = LegacyDependencyTableHash(0U);
    const std::uint64_t package_table_hash =
        LegacyPackageTableHash(entry, payload_hash, metadata_hash, dependency_table_hash);

    std::string artifact_text = std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|" +
        std::to_string(entry.value) + "|" +
        std::to_string(type.value) + "|" +
        std::string(logical_key) + "|" +
        std::string(source_key) + "|" +
        std::to_string(byte_offset) + "|" +
        std::to_string(byte_size) + "|" +
        std::to_string(payload_hash) + "|" +
        std::to_string(metadata_hash) + "\n" +
        "dependency_table_hash|" + std::to_string(dependency_table_hash) + "\n" +
        "package_table_hash|" + std::to_string(package_table_hash) + "\nend\n";
    return artifact_text;
}

bool ReplaceFirstEntryHashField(
    std::string *artifact_text,
    std::uint32_t hash_field_index,
    std::string_view replacement) {
    if (artifact_text == nullptr) {
        return false;
    }

    const std::string entry_prefix = "entry|1|1|texture_a|textures/texture_a.bin|0|16|16|0|16|";
    std::size_t field_start = artifact_text->find(entry_prefix);
    if (field_start == std::string::npos) {
        return false;
    }

    field_start += entry_prefix.size();
    if (hash_field_index > 0U) {
        const std::size_t payload_separator = artifact_text->find('|', field_start);
        if (payload_separator == std::string::npos) {
            return false;
        }

        field_start = payload_separator + 1U;
    }

    const char separator = hash_field_index == 0U ? '|' : '\n';
    const std::size_t field_end = artifact_text->find(separator, field_start);
    if (field_end == std::string::npos) {
        return false;
    }

    artifact_text->replace(field_start, field_end - field_start, replacement);
    return true;
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
    std::uint64_t byte_offset = 0ULL,
    std::uint64_t byte_size = 16ULL) {
    std::uint32_t legacy_byte_offset = 0U;
    std::uint32_t legacy_byte_size = 0U;
    const std::uint64_t max_legacy_value = std::numeric_limits<std::uint32_t>::max();
    if (byte_offset <= max_legacy_value) {
        legacy_byte_offset = static_cast<std::uint32_t>(byte_offset);
    }

    if (byte_size <= max_legacy_value) {
        legacy_byte_size = static_cast<std::uint32_t>(byte_size);
    }

    return PackageEntryDescriptor{
        package,
        entry,
        type,
        ResourceLogicalKey(logical_key),
        PackageSourceKey(source_key),
        legacy_byte_offset,
        legacy_byte_size,
        byte_offset,
        byte_size};
}

void FillArtifactCapacityEntries(
    std::array<PackageEntryDescriptor, ARTIFACT_CAPACITY_ENTRY_TEST_COUNT> *entries) {
    if (entries == nullptr) {
        return;
    }

    std::uint32_t index = 0U;
    while (index < ARTIFACT_CAPACITY_ENTRY_TEST_COUNT) {
        const std::string index_text = std::to_string(index);
        const std::string logical_key = "artifact_capacity_entry_" + index_text;
        const std::string source_key = "artifacts/capacity_entry_" + index_text + ".bin";
        const PackageEntryId entry_id{index + 1U};
        const std::uint64_t byte_offset = static_cast<std::uint64_t>(index) * 16ULL;
        (*entries)[static_cast<std::size_t>(index)] = Entry(
            PACKAGE_A,
            entry_id,
            TYPE_TEXTURE,
            logical_key.c_str(),
            source_key.c_str(),
            byte_offset,
            16ULL);
        ++index;
    }
}

void FillArtifactCapacityDependencies(
    std::array<PackageArtifactDependency, ARTIFACT_CAPACITY_DEPENDENCY_TEST_COUNT> *dependencies,
    PackageEntryId dependent,
    PackageEntryId dependency) {
    if (dependencies == nullptr) {
        return;
    }

    std::uint32_t index = 0U;
    while (index < ARTIFACT_CAPACITY_DEPENDENCY_TEST_COUNT) {
        (*dependencies)[static_cast<std::size_t>(index)] = PackageArtifactDependency{dependent, dependency};
        ++index;
    }
}

std::string BuildEntryCapacityArtifact(PackageEntryId failed_entry) {
    std::string artifact_text = std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|" + std::to_string(ARTIFACT_CAPACITY_ENTRY_TEST_COUNT) + "\ndependencies|0\n";

    std::uint32_t index = 0U;
    while (index < ARTIFACT_CAPACITY_ENTRY_TEST_COUNT) {
        const std::uint32_t entry_value = index == MAX_PACKAGE_ENTRY_COUNT ? failed_entry.value : index + 1U;
        const std::string index_text = std::to_string(index);
        artifact_text += "entry|" + std::to_string(entry_value) + "|1|artifact_read_entry_" + index_text +
            "|artifacts/read_entry_" + index_text + ".bin|0|16\n";
        ++index;
    }

    artifact_text += "end\n";
    return artifact_text;
}

std::string BuildDependencyCapacityArtifact(PackageEntryId dependent, PackageEntryId dependency) {
    std::string artifact_text = std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|2\ndependencies|" +
        std::to_string(ARTIFACT_CAPACITY_DEPENDENCY_TEST_COUNT) +
        "\nentry|1|1|artifact_dependency_a|artifacts/dependency_a.bin|0|16\n"
        "entry|2|2|artifact_dependency_b|artifacts/dependency_b.bin|16|16\n";

    std::uint32_t index = 0U;
    while (index < ARTIFACT_CAPACITY_DEPENDENCY_TEST_COUNT) {
        artifact_text += "dependency|" + std::to_string(dependent.value) + "|" +
            std::to_string(dependency.value) + "\n";
        ++index;
    }

    artifact_text += "end\n";
    return artifact_text;
}

int ExpectArtifactEntryCapacityFields(
    const PackageArtifactResult &result,
    PackageEntryId failed_entry,
    std::uint32_t required_entry_count) {
    if (result.failed_entry_index != MAX_PACKAGE_ENTRY_COUNT) {
        return Fail("artifact entry capacity reported wrong failed index");
    }

    if (result.failed_entry_id.value != failed_entry.value) {
        return Fail("artifact entry capacity reported wrong failed entry");
    }

    if (result.failed_entry_capacity != MAX_PACKAGE_ENTRY_COUNT) {
        return Fail("artifact entry capacity reported wrong capacity");
    }

    if (result.failed_entry_count != MAX_PACKAGE_ENTRY_COUNT) {
        return Fail("artifact entry capacity reported wrong current count");
    }

    if (result.required_entry_count != required_entry_count) {
        return Fail("artifact entry capacity reported wrong required count");
    }

    return 0;
}

int ExpectArtifactDependencyCapacityFields(
    const PackageArtifactResult &result,
    PackageEntryId dependent_entry,
    PackageEntryId dependency_entry,
    std::uint32_t required_dependency_count) {
    if (result.failed_dependency_index != MAX_PACKAGE_DEPENDENCY_EDGE_COUNT) {
        return Fail("artifact dependency capacity reported wrong failed index");
    }

    if (result.failed_dependent_entry_id.value != dependent_entry.value) {
        return Fail("artifact dependency capacity reported wrong dependent entry");
    }

    if (result.failed_dependency_entry_id.value != dependency_entry.value) {
        return Fail("artifact dependency capacity reported wrong dependency entry");
    }

    if (result.failed_dependency_capacity != MAX_PACKAGE_DEPENDENCY_EDGE_COUNT) {
        return Fail("artifact dependency capacity reported wrong capacity");
    }

    if (result.failed_dependency_count != MAX_PACKAGE_DEPENDENCY_EDGE_COUNT) {
        return Fail("artifact dependency capacity reported wrong current count");
    }

    if (result.required_dependency_count != required_dependency_count) {
        return Fail("artifact dependency capacity reported wrong required count");
    }

    return 0;
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
    std::uint64_t byte_offset = 0ULL,
    std::uint64_t byte_size = 16ULL) {
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

    if (left.last_load_plan_record_count != right.last_load_plan_record_count) {
        return false;
    }

    return left.last_load_plan_archive_byte_count == right.last_load_plan_archive_byte_count;
}

bool SnapshotsMatch(const PackageSnapshot& left, const PackageSnapshot& right) {
    if (!CoreCountsMatch(left, right)) {
        return false;
    }

    if (left.rejected_operation_count != right.rejected_operation_count) {
        return false;
    }

    if (left.accepted_operation_count != right.accepted_operation_count) {
        return false;
    }

    return left.allocation_accounting_status == right.allocation_accounting_status;
}

bool LoadPlanCapacityResultMatches(
    const PackageLoadPlanResult& result,
    PackageEntryId expected_entry,
    ResourceTypeId expected_type,
    const char *expected_logical_key,
    std::uint32_t expected_record_capacity,
    std::uint32_t expected_record_count,
    std::uint32_t expected_required_record_count) {
    if (result.required_load_plan_record_count != expected_required_record_count) {
        return false;
    }

    if (result.failed_load_plan_package.value != PACKAGE_A.value) {
        return false;
    }

    if (result.failed_load_plan_entry_id.value != expected_entry.value) {
        return false;
    }

    if (result.failed_load_plan_resource_type.value != expected_type.value) {
        return false;
    }

    if (!result.failed_load_plan_resource_key.Equals(ResourceLogicalKey(expected_logical_key))) {
        return false;
    }

    if (result.failed_load_plan_record_capacity != expected_record_capacity) {
        return false;
    }

    return result.failed_load_plan_record_count == expected_record_count;
}

bool LoadPlanCapacitySnapshotMatches(
    const PackageSnapshot& snapshot,
    PackageEntryId expected_entry,
    ResourceTypeId expected_type,
    const char *expected_logical_key,
    std::uint32_t expected_record_capacity,
    std::uint32_t expected_record_count,
    std::uint32_t expected_required_record_count) {
    if (snapshot.required_load_plan_record_count != expected_required_record_count) {
        return false;
    }

    if (snapshot.last_failed_load_plan_package.value != PACKAGE_A.value) {
        return false;
    }

    if (snapshot.last_failed_load_plan_entry_id.value != expected_entry.value) {
        return false;
    }

    if (snapshot.last_failed_load_plan_resource_type.value != expected_type.value) {
        return false;
    }

    if (!snapshot.last_failed_load_plan_resource_key.Equals(ResourceLogicalKey(expected_logical_key))) {
        return false;
    }

    if (snapshot.last_failed_load_plan_record_capacity != expected_record_capacity) {
        return false;
    }

    return snapshot.last_failed_load_plan_record_count == expected_record_count;
}

bool LoadPlanCapacityResultCleared(const PackageLoadPlanResult& result) {
    if (result.failed_load_plan_package.value != 0U) {
        return false;
    }

    if (result.failed_load_plan_entry_id.value != 0U) {
        return false;
    }

    if (result.failed_load_plan_resource_type.value != 0U) {
        return false;
    }

    if (result.failed_load_plan_resource_key.IsValid()) {
        return false;
    }

    if (result.failed_load_plan_record_capacity != 0U) {
        return false;
    }

    return result.failed_load_plan_record_count == 0U;
}

bool LoadPlanCapacitySnapshotCleared(const PackageSnapshot& snapshot) {
    if (snapshot.last_failed_load_plan_package.value != 0U) {
        return false;
    }

    if (snapshot.last_failed_load_plan_entry_id.value != 0U) {
        return false;
    }

    if (snapshot.last_failed_load_plan_resource_type.value != 0U) {
        return false;
    }

    if (snapshot.last_failed_load_plan_resource_key.IsValid()) {
        return false;
    }

    if (snapshot.last_failed_load_plan_record_capacity != 0U) {
        return false;
    }

    return snapshot.last_failed_load_plan_record_count == 0U;
}

int ExpectInvalidArtifactRead(
    MountTable& table,
    std::string_view artifact_text,
    PackageStatus expected_status,
    const char* failure_message,
    PackageRegistryDesc registry_desc = PackageRegistryDesc{}) {
    if (!WriteRawPackageArtifact(table, artifact_text)) {
        return Fail("artifact validation fixture write failed");
    }

    PackageRegistry registry;
    if (!RegisterManifest(registry).Succeeded()) {
        return Fail("artifact validation sentinel manifest failed");
    }

    const PackageSnapshot before_snapshot = registry.Snapshot();
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = &table;
    read_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    read_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    read_request.registry = &registry;
    read_request.registry_desc = registry_desc;
    const PackageArtifactResult read_result = ReadPackageArtifact(read_request);
    if (read_result.status != expected_status || read_result.read_artifact || read_result.rebuilt_registry) {
        return Fail(failure_message);
    }

    if (!SnapshotsMatch(before_snapshot, registry.Snapshot())) {
        return Fail("invalid artifact read mutated caller registry");
    }

    return 0;
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
    constexpr std::uint32_t EXPECTED_REQUIRED_MANIFEST_COUNT = 2U;
    constexpr std::uint32_t EXPECTED_FAILED_MANIFEST_INDEX = 1U;

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

    const PackageSnapshot after_snapshot = registry.Snapshot();
    if (overflow.required_manifest_record_count != EXPECTED_REQUIRED_MANIFEST_COUNT) {
        return Fail("manifest capacity overflow did not report required manifest count");
    }

    if (overflow.capacity_failure_kind != PackageStatus::ManifestCapacityExceeded) {
        return Fail("manifest capacity overflow did not report failure kind");
    }

    if (overflow.manifest_capacity != before_snapshot.manifest_capacity ||
        overflow.current_manifest_count != before_snapshot.manifest_count ||
        overflow.current_entry_count != before_snapshot.entry_count ||
        overflow.current_dependency_edge_count != before_snapshot.dependency_edge_count) {
        return Fail("manifest capacity overflow did not freeze current counts");
    }

    if (overflow.failed_manifest_index != EXPECTED_FAILED_MANIFEST_INDEX) {
        return Fail("manifest capacity overflow did not report failed manifest index");
    }

    if (overflow.failed_package.value != PACKAGE_B.value) {
        return Fail("manifest capacity overflow did not report failed package");
    }

    if (after_snapshot.required_manifest_record_count != EXPECTED_REQUIRED_MANIFEST_COUNT) {
        return Fail("manifest capacity overflow did not snapshot required manifest count");
    }

    if (after_snapshot.last_registration_capacity_failure_kind != PackageStatus::ManifestCapacityExceeded) {
        return Fail("manifest capacity overflow did not snapshot failure kind");
    }

    if (after_snapshot.last_failed_manifest_capacity != before_snapshot.manifest_capacity ||
        after_snapshot.last_failed_manifest_count != before_snapshot.manifest_count ||
        after_snapshot.last_failed_entry_count != before_snapshot.entry_count ||
        after_snapshot.last_failed_dependency_edge_count != before_snapshot.dependency_edge_count) {
        return Fail("manifest capacity overflow did not snapshot current counts");
    }

    if (after_snapshot.last_failed_manifest_index != EXPECTED_FAILED_MANIFEST_INDEX) {
        return Fail("manifest capacity overflow did not snapshot failed manifest index");
    }

    if (after_snapshot.last_failed_package.value != PACKAGE_B.value) {
        return Fail("manifest capacity overflow did not snapshot failed package");
    }

    return 0;
}

int PackageEntryCapacityOverflowDoesNotMutate() {
    constexpr std::uint32_t EXPECTED_REQUIRED_ENTRY_COUNT = 2U;
    constexpr std::uint32_t EXPECTED_FAILED_ENTRY_INDEX = 1U;

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

    const PackageSnapshot after_snapshot = registry.Snapshot();
    if (overflow.required_entry_record_count != EXPECTED_REQUIRED_ENTRY_COUNT) {
        return Fail("entry capacity overflow did not report required entry count");
    }

    if (overflow.capacity_failure_kind != PackageStatus::EntryCapacityExceeded) {
        return Fail("entry capacity overflow did not report failure kind");
    }

    if (overflow.entry_capacity != before_snapshot.entry_capacity ||
        overflow.current_manifest_count != before_snapshot.manifest_count ||
        overflow.current_entry_count != before_snapshot.entry_count ||
        overflow.current_dependency_edge_count != before_snapshot.dependency_edge_count) {
        return Fail("entry capacity overflow did not freeze current counts");
    }

    if (overflow.failed_package.value != PACKAGE_A.value) {
        return Fail("entry capacity overflow did not report failed package");
    }

    if (overflow.failed_entry_index != EXPECTED_FAILED_ENTRY_INDEX) {
        return Fail("entry capacity overflow did not report failed entry index");
    }

    if (overflow.failed_entry.value != ENTRY_MATERIAL.value) {
        return Fail("entry capacity overflow did not report failed entry");
    }

    if (after_snapshot.required_entry_record_count != EXPECTED_REQUIRED_ENTRY_COUNT) {
        return Fail("entry capacity overflow did not snapshot required entry count");
    }

    if (after_snapshot.last_registration_capacity_failure_kind != PackageStatus::EntryCapacityExceeded) {
        return Fail("entry capacity overflow did not snapshot failure kind");
    }

    if (after_snapshot.last_failed_entry_capacity != before_snapshot.entry_capacity ||
        after_snapshot.last_failed_manifest_count != before_snapshot.manifest_count ||
        after_snapshot.last_failed_entry_count != before_snapshot.entry_count ||
        after_snapshot.last_failed_dependency_edge_count != before_snapshot.dependency_edge_count) {
        return Fail("entry capacity overflow did not snapshot current counts");
    }

    if (after_snapshot.last_failed_package.value != PACKAGE_A.value) {
        return Fail("entry capacity overflow did not snapshot failed package");
    }

    if (after_snapshot.last_failed_entry_index != EXPECTED_FAILED_ENTRY_INDEX) {
        return Fail("entry capacity overflow did not snapshot failed entry index");
    }

    if (after_snapshot.last_failed_entry.value != ENTRY_MATERIAL.value) {
        return Fail("entry capacity overflow did not snapshot failed entry");
    }

    return 0;
}

int PackageRegistrationCapacityFailureIdentityClearsForNonCapacityFailures() {
    PackageRegistry manifest_registry(PackageRegistryDesc{1U, 4U, 4U, 4U});
    RegisterManifest(manifest_registry, PACKAGE_A);
    RegisterManifest(manifest_registry, PACKAGE_B);

    const PackageRegistrationResult duplicate_manifest = RegisterManifest(manifest_registry, PACKAGE_A);
    if (duplicate_manifest.status != PackageStatus::DuplicateManifest) {
        return Fail("duplicate manifest did not return duplicate status after capacity failure");
    }

    if (duplicate_manifest.failed_manifest_index != 0U) {
        return Fail("duplicate manifest reported stale failed manifest index");
    }

    if (duplicate_manifest.failed_package.IsValid()) {
        return Fail("duplicate manifest reported stale failed package");
    }

    const PackageSnapshot duplicate_manifest_snapshot = manifest_registry.Snapshot();
    if (duplicate_manifest_snapshot.last_failed_manifest_index != 0U) {
        return Fail("duplicate manifest snapshot kept stale failed manifest index");
    }

    if (duplicate_manifest_snapshot.last_failed_package.IsValid()) {
        return Fail("duplicate manifest snapshot kept stale failed package");
    }

    RegisterManifest(manifest_registry, PACKAGE_B);
    const PackageRegistrationResult invalid_manifest = RegisterManifest(manifest_registry, PackageId{});
    if (invalid_manifest.status != PackageStatus::InvalidPackageId) {
        return Fail("invalid manifest did not return invalid package status after capacity failure");
    }

    if (invalid_manifest.failed_manifest_index != 0U) {
        return Fail("invalid manifest reported stale failed manifest index");
    }

    if (invalid_manifest.failed_package.IsValid()) {
        return Fail("invalid manifest reported stale failed package");
    }

    const PackageSnapshot invalid_manifest_snapshot = manifest_registry.Snapshot();
    if (invalid_manifest_snapshot.last_failed_manifest_index != 0U) {
        return Fail("invalid manifest snapshot kept stale failed manifest index");
    }

    if (invalid_manifest_snapshot.last_failed_package.IsValid()) {
        return Fail("invalid manifest snapshot kept stale failed package");
    }

    PackageRegistry entry_registry(PackageRegistryDesc{1U, 1U, 4U, 4U});
    RegisterManifest(entry_registry);
    RegisterEntry(entry_registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");
    RegisterEntry(entry_registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin");

    const PackageRegistrationResult duplicate_entry =
        RegisterEntry(entry_registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");
    if (duplicate_entry.status != PackageStatus::DuplicateEntry) {
        return Fail("duplicate entry did not return duplicate status after capacity failure");
    }

    if (duplicate_entry.failed_package.IsValid()) {
        return Fail("duplicate entry reported stale failed package");
    }

    if (duplicate_entry.failed_entry_index != 0U) {
        return Fail("duplicate entry reported stale failed entry index");
    }

    if (duplicate_entry.failed_entry.IsValid()) {
        return Fail("duplicate entry reported stale failed entry");
    }

    const PackageSnapshot duplicate_entry_snapshot = entry_registry.Snapshot();
    if (duplicate_entry_snapshot.last_failed_package.IsValid()) {
        return Fail("duplicate entry snapshot kept stale failed package");
    }

    if (duplicate_entry_snapshot.last_failed_entry_index != 0U) {
        return Fail("duplicate entry snapshot kept stale failed entry index");
    }

    if (duplicate_entry_snapshot.last_failed_entry.IsValid()) {
        return Fail("duplicate entry snapshot kept stale failed entry");
    }

    RegisterEntry(entry_registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_b", "materials/material_b.bin");
    const PackageRegistrationResult invalid_entry =
        entry_registry.RegisterEntry(Entry(PACKAGE_A, PackageEntryId{}, TYPE_TEXTURE, "invalid", "invalid.bin"));
    if (invalid_entry.status != PackageStatus::InvalidEntryId) {
        return Fail("invalid entry did not return invalid entry status after capacity failure");
    }

    if (invalid_entry.failed_package.IsValid()) {
        return Fail("invalid entry reported stale failed package");
    }

    if (invalid_entry.failed_entry_index != 0U) {
        return Fail("invalid entry reported stale failed entry index");
    }

    if (invalid_entry.failed_entry.IsValid()) {
        return Fail("invalid entry reported stale failed entry");
    }

    const PackageSnapshot invalid_entry_snapshot = entry_registry.Snapshot();
    if (invalid_entry_snapshot.last_failed_package.IsValid()) {
        return Fail("invalid entry snapshot kept stale failed package");
    }

    if (invalid_entry_snapshot.last_failed_entry_index != 0U) {
        return Fail("invalid entry snapshot kept stale failed entry index");
    }

    if (invalid_entry_snapshot.last_failed_entry.IsValid()) {
        return Fail("invalid entry snapshot kept stale failed entry");
    }

    PackageRegistry dependency_registry(PackageRegistryDesc{1U, 4U, 1U, 4U});
    RegisterManifest(dependency_registry);
    RegisterEntry(dependency_registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");
    RegisterEntry(dependency_registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin");
    RegisterEntry(dependency_registry, ENTRY_AUDIO, TYPE_AUDIO, "audio_a", "audio/audio_a.bin");
    dependency_registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);
    dependency_registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_AUDIO);

    const PackageStatus duplicate_dependency =
        dependency_registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);
    if (duplicate_dependency != PackageStatus::Success) {
        return Fail("duplicate dependency did not return success after capacity failure");
    }

    const PackageSnapshot duplicate_dependency_snapshot = dependency_registry.Snapshot();
    if (duplicate_dependency_snapshot.last_registration_capacity_failure_kind != PackageStatus::Success) {
        return Fail("duplicate dependency snapshot kept stale failure kind");
    }

    if (duplicate_dependency_snapshot.last_failed_dependency.IsValid()) {
        return Fail("duplicate dependency snapshot kept stale failed dependency");
    }

    dependency_registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_AUDIO);
    const PackageStatus invalid_dependency =
        dependency_registry.AddDependency(PACKAGE_A, PackageEntryId{}, ENTRY_AUDIO);
    if (invalid_dependency != PackageStatus::InvalidEntryId) {
        return Fail("invalid dependency did not return invalid entry status after capacity failure");
    }

    const PackageSnapshot invalid_dependency_snapshot = dependency_registry.Snapshot();
    if (invalid_dependency_snapshot.last_registration_capacity_failure_kind != PackageStatus::Success) {
        return Fail("invalid dependency snapshot kept stale failure kind");
    }

    if (invalid_dependency_snapshot.last_failed_dependency.IsValid()) {
        return Fail("invalid dependency snapshot kept stale failed dependency");
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
        MAX_DECLARED_ENTRY_SIZE + 1ULL);
    if (too_large.status != PackageStatus::ByteRangeOutOfBounds) {
        return Fail("oversized byte range did not return explicit status");
    }

    const PackageRegistrationResult zero_size = RegisterEntry(
        registry,
        ENTRY_TEXTURE,
        TYPE_TEXTURE,
        "texture_a",
        "textures/texture_a.bin",
        0U,
        0U);
    if (zero_size.status != PackageStatus::ByteRangeOutOfBounds) {
        return Fail("zero-size byte range did not return explicit status");
    }

    const PackageRegistrationResult overflow = RegisterEntry(
        registry,
        ENTRY_TEXTURE,
        TYPE_TEXTURE,
        "texture_a",
        "textures/texture_a.bin",
        std::numeric_limits<std::uint64_t>::max(),
        1U);
    if (overflow.status != PackageStatus::ByteRangeOutOfBounds) {
        return Fail("overflow byte range did not return explicit status");
    }

    if (registry.Snapshot().entry_count != before_snapshot.entry_count) {
        return Fail("oversized byte range changed entry count");
    }

    return 0;
}

int PackageRegisterEntryAcceptsLargeByteRangeAndResolvePreservesIndexMetadata() {
    PackageRegistry registry;
    RegisterManifest(registry);

    const PackageRegistrationResult entry_result = RegisterEntry(
        registry,
        ENTRY_TEXTURE,
        TYPE_TEXTURE,
        "texture_large_range",
        "textures/texture_large_range.bin",
        LARGE_ARCHIVE_BYTE_OFFSET,
        MAX_DECLARED_ENTRY_SIZE);
    if (!entry_result.Succeeded()) {
        return Fail("large byte-range entry did not register");
    }

    const PackageLoadPlanResult plan =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_large_range"));
    if (!plan.Succeeded()) {
        return Fail("large byte-range entry did not resolve through package index");
    }

    if (plan.plan.record_count != 1U) {
        return Fail("large byte-range load plan returned unexpected record count");
    }

    const PackageLoadPlanRecord& record = plan.plan.records[0U];
    if (record.archive_byte_offset != LARGE_ARCHIVE_BYTE_OFFSET) {
        return Fail("large byte-range offset was not preserved");
    }

    if (record.archive_byte_size != MAX_DECLARED_ENTRY_SIZE) {
        return Fail("large byte-range size was not preserved");
    }

    if (record.payload_logical_byte_count != MAX_DECLARED_ENTRY_SIZE ||
        record.payload_window_byte_offset != 0ULL ||
        record.payload_window_byte_size != MAX_DECLARED_ENTRY_SIZE) {
        return Fail("large byte-range default payload metadata changed");
    }

    return 0;
}

int PackageLoadPlanPreservesPayloadWindowMetadataDistinctFromArchiveRange() {
    PackageRegistry registry;
    RegisterManifest(registry);

    PackageEntryDescriptor descriptor = Entry(
        PACKAGE_A,
        ENTRY_TEXTURE,
        TYPE_TEXTURE,
        "texture_payload_window",
        "textures/texture_payload_window.bin",
        LARGE_ARCHIVE_BYTE_OFFSET,
        PAYLOAD_WINDOW_BYTE_SIZE);
    descriptor.payload_logical_byte_count = PAYLOAD_LOGICAL_BYTE_COUNT;
    descriptor.payload_window_byte_offset = PAYLOAD_WINDOW_BYTE_OFFSET;
    descriptor.payload_window_byte_size = PAYLOAD_WINDOW_BYTE_SIZE;

    const PackageRegistrationResult entry_result = registry.RegisterEntry(descriptor);
    if (!entry_result.Succeeded()) {
        return Fail("payload metadata entry did not register");
    }

    const PackageLoadPlanResult plan =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_payload_window"));
    if (!plan.Succeeded() || plan.plan.record_count != 1U) {
        return Fail("payload metadata entry did not resolve");
    }

    const PackageLoadPlanRecord& record = plan.plan.records[0U];
    if (record.archive_byte_offset != LARGE_ARCHIVE_BYTE_OFFSET ||
        record.archive_byte_size != PAYLOAD_WINDOW_BYTE_SIZE) {
        return Fail("payload metadata changed archive range");
    }

    if (record.payload_logical_byte_count != PAYLOAD_LOGICAL_BYTE_COUNT ||
        record.payload_window_byte_offset != PAYLOAD_WINDOW_BYTE_OFFSET ||
        record.payload_window_byte_size != PAYLOAD_WINDOW_BYTE_SIZE) {
        return Fail("payload metadata was not preserved");
    }

    if (record.payload_window_byte_offset == record.archive_byte_offset) {
        return Fail("payload metadata reused archive offset");
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

PackageRegistry CreateTransitiveClosureRegistry() {
    PackageRegistry registry;
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin", 64U, 32U);
    RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin", 8U, 16U);
    RegisterEntry(registry, ENTRY_AUDIO, TYPE_AUDIO, "audio_a", "audio/audio_a.bin", 24U, 8U);
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);
    registry.AddDependency(PACKAGE_A, ENTRY_MATERIAL, ENTRY_AUDIO);
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

    if (registry.Snapshot().last_load_plan_archive_byte_count != 64ULL) {
        return Fail("load plan archive byte count was not recorded");
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

int PackageAcceptedOperationCountTracksSuccessAfterFailureAndDuplicateDependency() {
    PackageRegistry registry;
    const PackageSnapshot initial_snapshot = registry.Snapshot();
    if (initial_snapshot.accepted_operation_count != 0U) {
        return Fail("initial accepted operation count changed");
    }

    if (initial_snapshot.rejected_operation_count != 0U) {
        return Fail("initial rejected operation count changed");
    }

    const PackageRegistrationResult invalid_manifest = registry.RegisterSyntheticManifest({PackageId{}});
    if (invalid_manifest.status != PackageStatus::InvalidPackageId) {
        return Fail("invalid manifest status changed");
    }

    const PackageSnapshot after_invalid_manifest = registry.Snapshot();
    if (after_invalid_manifest.accepted_operation_count != 0U) {
        return Fail("invalid manifest changed accepted operation count");
    }

    if (after_invalid_manifest.rejected_operation_count != 1U) {
        return Fail("invalid manifest did not increment rejected operation count");
    }

    if (after_invalid_manifest.last_status != PackageStatus::InvalidPackageId) {
        return Fail("invalid manifest did not record last status");
    }

    if (!RegisterManifest(registry).Succeeded()) {
        return Fail("accepted count fixture manifest failed");
    }

    const PackageSnapshot after_manifest = registry.Snapshot();
    if (after_manifest.accepted_operation_count != 1U) {
        return Fail("manifest success did not increment accepted operation count");
    }

    if (after_manifest.rejected_operation_count != 1U) {
        return Fail("manifest success changed rejected operation count");
    }

    if (after_manifest.last_status != PackageStatus::Success) {
        return Fail("manifest success did not clear last status");
    }

    if (!RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin").Succeeded()) {
        return Fail("accepted count texture entry failed");
    }

    if (!RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin").Succeeded()) {
        return Fail("accepted count material entry failed");
    }

    const PackageSnapshot after_entries = registry.Snapshot();
    if (after_entries.accepted_operation_count != 3U) {
        return Fail("entry success did not increment accepted operation count");
    }

    const PackageStatus dependency_status = registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);
    if (dependency_status != PackageStatus::Success) {
        return Fail("accepted count dependency edge failed");
    }

    const PackageSnapshot after_dependency = registry.Snapshot();
    if (after_dependency.accepted_operation_count != 4U) {
        return Fail("dependency success did not increment accepted operation count");
    }

    if (after_dependency.dependency_edge_count != 1U) {
        return Fail("dependency success changed edge count unexpectedly");
    }

    const PackageStatus duplicate_dependency_status = registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);
    if (duplicate_dependency_status != PackageStatus::Success) {
        return Fail("duplicate dependency success path failed");
    }

    const PackageSnapshot after_duplicate_dependency = registry.Snapshot();
    if (after_duplicate_dependency.accepted_operation_count != 5U) {
        return Fail("duplicate dependency success did not increment accepted operation count");
    }

    if (after_duplicate_dependency.dependency_edge_count != after_dependency.dependency_edge_count) {
        return Fail("duplicate dependency success changed edge count");
    }

    if (after_duplicate_dependency.rejected_operation_count != after_manifest.rejected_operation_count) {
        return Fail("duplicate dependency success changed rejected operation count");
    }

    const PackageRegistrationResult duplicate_entry =
        RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_duplicate", "textures/texture_duplicate.bin");
    if (duplicate_entry.status != PackageStatus::DuplicateEntry) {
        return Fail("duplicate entry failure status changed");
    }

    const PackageSnapshot after_duplicate_entry = registry.Snapshot();
    if (after_duplicate_entry.accepted_operation_count != after_duplicate_dependency.accepted_operation_count) {
        return Fail("duplicate entry changed accepted operation count");
    }

    if (after_duplicate_entry.rejected_operation_count != 2U) {
        return Fail("duplicate entry did not increment rejected operation count");
    }

    if (after_duplicate_entry.last_status != PackageStatus::DuplicateEntry) {
        return Fail("duplicate entry did not record last status");
    }

    const PackageLoadPlanResult plan =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (!plan.Succeeded()) {
        return Fail("accepted count resolve failed");
    }

    const PackageSnapshot after_resolve = registry.Snapshot();
    if (after_resolve.accepted_operation_count != 6U) {
        return Fail("resolve success did not increment accepted operation count");
    }

    if (after_resolve.rejected_operation_count != after_duplicate_entry.rejected_operation_count) {
        return Fail("resolve success changed rejected operation count");
    }

    if (after_resolve.last_status != PackageStatus::Success) {
        return Fail("resolve success did not clear last status");
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

int PackageDependencyClosurePlanIncludesTransitiveDependenciesBeforeRoot() {
    constexpr std::uint64_t EXPECTED_ARCHIVE_BYTE_COUNT = 56ULL;

    PackageRegistry registry = CreateTransitiveClosureRegistry();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (!result.Succeeded()) {
        return Fail("transitive closure resolve failed");
    }

    if (result.plan.record_count != 3U) {
        return Fail("transitive closure record count was not dependency chain plus root");
    }

    if (result.plan.records[0U].entry.value != ENTRY_AUDIO.value) {
        return Fail("transitive dependency was not emitted before direct dependency");
    }

    if (result.plan.records[1U].entry.value != ENTRY_MATERIAL.value) {
        return Fail("direct dependency was not emitted before root");
    }

    if (result.plan.records[2U].entry.value != ENTRY_TEXTURE.value) {
        return Fail("root was not emitted last after transitive closure");
    }

    if (result.plan.archive_byte_count != EXPECTED_ARCHIVE_BYTE_COUNT) {
        return Fail("transitive closure archive byte count was not accumulated");
    }

    const PackageSnapshot snapshot = registry.Snapshot();
    if (snapshot.last_load_plan_archive_byte_count != EXPECTED_ARCHIVE_BYTE_COUNT) {
        return Fail("transitive closure archive byte count was not recorded");
    }

    return 0;
}

int PackageDependencyClosureDeduplicatesSharedDependenciesAndPreservesDeclarationOrder() {
    constexpr std::uint64_t EXPECTED_ARCHIVE_BYTE_COUNT = 60ULL;

    PackageRegistry registry;
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin", 64U, 32U);
    RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin", 8U, 16U);
    RegisterEntry(registry, ENTRY_AUDIO, TYPE_AUDIO, "audio_a", "audio/audio_a.bin", 24U, 8U);
    RegisterEntry(registry, ENTRY_EFFECT, TYPE_EFFECT, "effect_a", "effects/effect_a.bin", 96U, 4U);
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_EFFECT);
    registry.AddDependency(PACKAGE_A, ENTRY_MATERIAL, ENTRY_AUDIO);
    registry.AddDependency(PACKAGE_A, ENTRY_EFFECT, ENTRY_AUDIO);

    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (!result.Succeeded()) {
        return Fail("shared dependency closure resolve failed");
    }

    if (result.plan.record_count != 4U) {
        return Fail("shared dependency closure did not deduplicate records");
    }

    if (result.plan.records[0U].entry.value != ENTRY_AUDIO.value) {
        return Fail("shared dependency was not emitted before first dependent");
    }

    if (result.plan.records[1U].entry.value != ENTRY_MATERIAL.value) {
        return Fail("first direct dependency did not preserve declaration order");
    }

    if (result.plan.records[2U].entry.value != ENTRY_EFFECT.value) {
        return Fail("second direct dependency did not preserve declaration order");
    }

    if (result.plan.records[3U].entry.value != ENTRY_TEXTURE.value) {
        return Fail("shared dependency closure root was not last");
    }

    if (result.plan.archive_byte_count != EXPECTED_ARCHIVE_BYTE_COUNT) {
        return Fail("shared dependency closure archive byte count was not deduplicated");
    }

    return 0;
}

int PackageDependencyClosureRejectsRecordBudgetWithoutMutation() {
    constexpr std::uint32_t EXPECTED_REQUIRED_RECORD_COUNT = 3U;

    PackageRegistryDesc desc{};
    desc.manifest_capacity = 1U;
    desc.entry_capacity = 4U;
    desc.dependency_edge_capacity = 4U;
    desc.load_plan_record_capacity = 2U;

    PackageRegistry registry(desc);
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin", 64U, 32U);
    RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin", 8U, 16U);
    RegisterEntry(registry, ENTRY_AUDIO, TYPE_AUDIO, "audio_a", "audio/audio_a.bin", 24U, 8U);
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);
    registry.AddDependency(PACKAGE_A, ENTRY_MATERIAL, ENTRY_AUDIO);

    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (result.status != PackageStatus::LoadPlanCapacityExceeded) {
        return Fail("dependency closure record budget did not return explicit status");
    }

    const PackageSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.load_plan_resolve_count != before_snapshot.load_plan_resolve_count) {
        return Fail("dependency closure record budget changed resolve count");
    }

    if (after_snapshot.last_load_plan_record_count != before_snapshot.last_load_plan_record_count) {
        return Fail("dependency closure record budget changed last record count");
    }

    if (after_snapshot.last_load_plan_archive_byte_count != before_snapshot.last_load_plan_archive_byte_count) {
        return Fail("dependency closure record budget changed last archive byte count");
    }

    if (result.required_load_plan_record_count != EXPECTED_REQUIRED_RECORD_COUNT) {
        return Fail("dependency closure record budget did not report required record count");
    }

    if (after_snapshot.required_load_plan_record_count != EXPECTED_REQUIRED_RECORD_COUNT) {
        return Fail("dependency closure record budget did not snapshot required record count");
    }

    if (!LoadPlanCapacityResultMatches(
            result,
            ENTRY_TEXTURE,
            TYPE_TEXTURE,
            "texture_a",
            desc.load_plan_record_capacity,
            desc.load_plan_record_capacity,
            EXPECTED_REQUIRED_RECORD_COUNT)) {
        return Fail("dependency closure record budget did not report rejected entry identity");
    }

    if (!LoadPlanCapacitySnapshotMatches(
            after_snapshot,
            ENTRY_TEXTURE,
            TYPE_TEXTURE,
            "texture_a",
            desc.load_plan_record_capacity,
            desc.load_plan_record_capacity,
            EXPECTED_REQUIRED_RECORD_COUNT)) {
        return Fail("dependency closure record budget did not snapshot rejected entry identity");
    }

    if (result.plan.record_count != 0U) {
        return Fail("dependency closure record budget published partial plan");
    }

    return 0;
}

int PackageDependencyCapacityOverflowDoesNotMutate() {
    constexpr std::uint32_t EXPECTED_REQUIRED_DEPENDENCY_COUNT = 2U;
    constexpr std::uint32_t EXPECTED_FAILED_DEPENDENCY_INDEX = 1U;

    PackageRegistry registry(PackageRegistryDesc{1U, 4U, 1U, 1U});
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin");
    RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin");
    RegisterEntry(registry, ENTRY_AUDIO, TYPE_AUDIO, "audio_a", "audio/audio_a.bin");
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);
    const PackageLoadPlanResult capacity_result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (capacity_result.status != PackageStatus::LoadPlanCapacityExceeded) {
        return Fail("dependency capacity fixture did not seed load-plan capacity failure");
    }

    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageStatus overflow = registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_AUDIO);
    if (overflow != PackageStatus::DependencyCapacityExceeded) {
        return Fail("dependency capacity overflow did not return explicit status");
    }

    if (registry.Snapshot().dependency_edge_count != before_snapshot.dependency_edge_count) {
        return Fail("dependency capacity overflow changed edge count");
    }

    if (!LoadPlanCapacitySnapshotCleared(registry.Snapshot())) {
        return Fail("dependency capacity overflow did not clear load-plan capacity entry");
    }

    const PackageSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.last_registration_capacity_failure_kind != PackageStatus::DependencyCapacityExceeded) {
        return Fail("dependency capacity overflow did not snapshot failure kind");
    }

    if (after_snapshot.last_failed_package.value != PACKAGE_A.value ||
        after_snapshot.last_failed_entry.value != ENTRY_TEXTURE.value ||
        after_snapshot.last_failed_dependency.value != ENTRY_AUDIO.value) {
        return Fail("dependency capacity overflow did not snapshot rejected identity");
    }

    if (after_snapshot.last_failed_dependency_edge_index != EXPECTED_FAILED_DEPENDENCY_INDEX) {
        return Fail("dependency capacity overflow did not snapshot failed dependency index");
    }

    if (after_snapshot.last_required_dependency_edge_count != EXPECTED_REQUIRED_DEPENDENCY_COUNT) {
        return Fail("dependency capacity overflow did not snapshot required dependency count");
    }

    if (after_snapshot.last_failed_dependency_edge_capacity != before_snapshot.dependency_edge_capacity ||
        after_snapshot.last_failed_manifest_count != before_snapshot.manifest_count ||
        after_snapshot.last_failed_entry_count != before_snapshot.entry_count ||
        after_snapshot.last_failed_dependency_edge_count != before_snapshot.dependency_edge_count) {
        return Fail("dependency capacity overflow did not snapshot current counts");
    }

    return 0;
}

int PackageLoadPlanCapacityOverflowDoesNotMutate() {
    constexpr std::uint32_t EXPECTED_REQUIRED_RECORD_COUNT = 2U;

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

    const PackageSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.load_plan_resolve_count != before_snapshot.load_plan_resolve_count) {
        return Fail("load-plan capacity overflow changed resolve count");
    }

    if (after_snapshot.last_load_plan_record_count != before_snapshot.last_load_plan_record_count) {
        return Fail("load-plan capacity overflow changed plan record count");
    }

    if (after_snapshot.last_load_plan_archive_byte_count != before_snapshot.last_load_plan_archive_byte_count) {
        return Fail("load-plan capacity overflow changed plan archive byte count");
    }

    if (result.required_load_plan_record_count != EXPECTED_REQUIRED_RECORD_COUNT) {
        return Fail("load-plan capacity overflow did not report required record count");
    }

    if (after_snapshot.required_load_plan_record_count != EXPECTED_REQUIRED_RECORD_COUNT) {
        return Fail("load-plan capacity overflow did not snapshot required record count");
    }

    if (!LoadPlanCapacityResultMatches(
            result,
            ENTRY_TEXTURE,
            TYPE_TEXTURE,
            "texture_a",
            1U,
            1U,
            EXPECTED_REQUIRED_RECORD_COUNT)) {
        return Fail("load-plan capacity overflow did not report rejected entry identity");
    }

    if (!LoadPlanCapacitySnapshotMatches(
            after_snapshot,
            ENTRY_TEXTURE,
            TYPE_TEXTURE,
            "texture_a",
            1U,
            1U,
            EXPECTED_REQUIRED_RECORD_COUNT)) {
        return Fail("load-plan capacity overflow did not snapshot rejected entry identity");
    }

    const PackageLoadPlanResult success_result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_MATERIAL, ResourceLogicalKey("material_a"));
    if (!success_result.Succeeded()) {
        return Fail("load-plan capacity follow-up success failed");
    }

    if (!LoadPlanCapacityResultCleared(success_result)) {
        return Fail("load-plan capacity follow-up success did not clear result entry");
    }

    if (!LoadPlanCapacitySnapshotCleared(registry.Snapshot())) {
        return Fail("load-plan capacity follow-up success did not clear snapshot entry");
    }

    return 0;
}

int PackageLoadPlanRejectsArchiveByteBudgetWithoutMutation() {
    PackageRegistryDesc desc{};
    desc.manifest_capacity = 1U;
    desc.entry_capacity = 4U;
    desc.dependency_edge_capacity = 4U;
    desc.load_plan_record_capacity = 1U;
    desc.load_plan_archive_byte_budget = 40ULL;

    PackageRegistry registry(desc);
    RegisterManifest(registry);
    RegisterEntry(registry, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin", 64U, 32U);
    RegisterEntry(registry, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin", 8U, 16U);
    RegisterEntry(registry, ENTRY_AUDIO, TYPE_AUDIO, "audio_a", "audio/audio_a.bin", 24U, 8U);
    RegisterEntry(registry, ENTRY_EFFECT, TYPE_EFFECT, "effect_a", "effects/effect_a.bin", 128U, 48U);
    registry.AddDependency(PACKAGE_A, ENTRY_TEXTURE, ENTRY_MATERIAL);
    const PackageLoadPlanResult capacity_result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (capacity_result.status != PackageStatus::LoadPlanCapacityExceeded) {
        return Fail("load-plan archive byte budget fixture did not seed capacity entry");
    }

    const PackageSnapshot before_snapshot = registry.Snapshot();
    const PackageLoadPlanResult result =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_EFFECT, ResourceLogicalKey("effect_a"));
    if (result.status != PackageStatus::LoadPlanByteBudgetExceeded) {
        return Fail("load-plan archive byte budget did not return explicit status");
    }

    const PackageSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.load_plan_resolve_count != before_snapshot.load_plan_resolve_count) {
        return Fail("load-plan archive byte budget changed resolve count");
    }

    if (after_snapshot.last_load_plan_record_count != before_snapshot.last_load_plan_record_count) {
        return Fail("load-plan archive byte budget changed last record count");
    }

    if (after_snapshot.last_load_plan_archive_byte_count != before_snapshot.last_load_plan_archive_byte_count) {
        return Fail("load-plan archive byte budget changed last archive byte count");
    }

    if (result.plan.record_count != 0U) {
        return Fail("load-plan archive byte budget published partial plan");
    }

    if (!LoadPlanCapacityResultCleared(result)) {
        return Fail("load-plan archive byte budget did not clear result capacity entry");
    }

    if (!LoadPlanCapacitySnapshotCleared(after_snapshot)) {
        return Fail("load-plan archive byte budget did not clear snapshot capacity entry");
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

int PackageFileBackedArtifactRoundTripsLargeByteRangeMetadata() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_LARGE_BYTE_RANGE);
    const std::array<PackageEntryDescriptor, 1U> entries{
        Entry(
            PACKAGE_A,
            ENTRY_TEXTURE,
            TYPE_TEXTURE,
            "texture_large_range",
            "textures/texture_large_range.bin",
            LARGE_ARCHIVE_BYTE_OFFSET,
            128U)};

    PackageArtifactWriteRequest write_request{};
    write_request.mount_table = &table;
    write_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    write_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    write_request.package = PACKAGE_A;
    write_request.entries = entries.data();
    write_request.entry_count = static_cast<std::uint32_t>(entries.size());
    const PackageArtifactResult write_result = WritePackageArtifact(write_request);
    if (write_result.status != PackageStatus::Success || !write_result.wrote_artifact) {
        return Fail("large byte-range artifact did not write");
    }

    PackageRegistry artifact_registry;
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = &table;
    read_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    read_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    read_request.registry = &artifact_registry;
    const PackageArtifactResult read_result = ReadPackageArtifact(read_request);
    if (read_result.status != PackageStatus::Success || !read_result.rebuilt_registry) {
        return Fail("large byte-range artifact did not rebuild registry");
    }

    const PackageLoadPlanResult plan =
        artifact_registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_large_range"));
    if (!plan.Succeeded() || plan.plan.record_count != 1U) {
        return Fail("large byte-range artifact did not resolve");
    }

    const PackageLoadPlanRecord& record = plan.plan.records[0U];
    if (record.archive_byte_offset != LARGE_ARCHIVE_BYTE_OFFSET || record.archive_byte_size != 128ULL) {
        return Fail("large byte-range artifact metadata changed");
    }

    if (record.payload_logical_byte_count != 128ULL ||
        record.payload_window_byte_offset != 0ULL ||
        record.payload_window_byte_size != 128ULL) {
        return Fail("large byte-range artifact default payload metadata changed");
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_LARGE_BYTE_RANGE));
    return 0;
}

int PackageFileBackedArtifactRoundTripsPayloadWindowMetadata() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_PAYLOAD_METADATA);
    PackageEntryDescriptor descriptor = Entry(
        PACKAGE_A,
        ENTRY_TEXTURE,
        TYPE_TEXTURE,
        "texture_payload_window",
        "textures/texture_payload_window.bin",
        LARGE_ARCHIVE_BYTE_OFFSET,
        PAYLOAD_WINDOW_BYTE_SIZE);
    descriptor.payload_logical_byte_count = PAYLOAD_LOGICAL_BYTE_COUNT;
    descriptor.payload_window_byte_offset = PAYLOAD_WINDOW_BYTE_OFFSET;
    descriptor.payload_window_byte_size = PAYLOAD_WINDOW_BYTE_SIZE;
    const std::array<PackageEntryDescriptor, 1U> entries{descriptor};

    PackageArtifactWriteRequest write_request{};
    write_request.mount_table = &table;
    write_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    write_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    write_request.package = PACKAGE_A;
    write_request.entries = entries.data();
    write_request.entry_count = static_cast<std::uint32_t>(entries.size());
    const PackageArtifactResult write_result = WritePackageArtifact(write_request);
    if (write_result.status != PackageStatus::Success || !write_result.wrote_artifact) {
        return Fail("payload metadata artifact did not write");
    }

    PackageRegistry artifact_registry;
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = &table;
    read_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    read_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    read_request.registry = &artifact_registry;
    const PackageArtifactResult read_result = ReadPackageArtifact(read_request);
    if (read_result.status != PackageStatus::Success || !read_result.rebuilt_registry) {
        return Fail("payload metadata artifact did not rebuild registry");
    }

    const PackageLoadPlanResult plan =
        artifact_registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_payload_window"));
    if (!plan.Succeeded() || plan.plan.record_count != 1U) {
        return Fail("payload metadata artifact did not resolve");
    }

    const PackageLoadPlanRecord& record = plan.plan.records[0U];
    if (record.archive_byte_offset != LARGE_ARCHIVE_BYTE_OFFSET ||
        record.archive_byte_size != PAYLOAD_WINDOW_BYTE_SIZE) {
        return Fail("payload metadata artifact changed archive range");
    }

    if (record.payload_logical_byte_count != PAYLOAD_LOGICAL_BYTE_COUNT ||
        record.payload_window_byte_offset != PAYLOAD_WINDOW_BYTE_OFFSET ||
        record.payload_window_byte_size != PAYLOAD_WINDOW_BYTE_SIZE) {
        return Fail("payload metadata artifact changed payload window");
    }

    if (record.payload_window_byte_offset == record.archive_byte_offset) {
        return Fail("payload metadata artifact reused archive offset");
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_PAYLOAD_METADATA));
    return 0;
}

int PackageFileBackedArtifactReadsLegacySevenFieldRowsWithDefaultPayloadMetadata() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_LEGACY_SEVEN_FIELD);
    const std::string artifact_text =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|1|1|texture_legacy_7|textures/legacy_7.bin|64|32\nend\n";
    if (!WriteRawPackageArtifact(table, artifact_text)) {
        return Fail("legacy seven-field artifact fixture write failed");
    }

    PackageRegistry artifact_registry;
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = &table;
    read_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    read_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    read_request.registry = &artifact_registry;
    const PackageArtifactResult read_result = ReadPackageArtifact(read_request);
    if (read_result.status != PackageStatus::Success || !read_result.rebuilt_registry) {
        return Fail("legacy seven-field artifact did not rebuild registry");
    }

    const PackageLoadPlanResult plan =
        artifact_registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_legacy_7"));
    if (!plan.Succeeded() || plan.plan.record_count != 1U) {
        return Fail("legacy seven-field artifact did not resolve");
    }

    const PackageLoadPlanRecord& record = plan.plan.records[0U];
    if (record.archive_byte_offset != 64ULL || record.archive_byte_size != 32ULL) {
        return Fail("legacy seven-field artifact changed archive range");
    }

    if (record.payload_logical_byte_count != 32ULL ||
        record.payload_window_byte_offset != 0ULL ||
        record.payload_window_byte_size != 32ULL) {
        return Fail("legacy seven-field artifact did not default payload metadata");
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_LEGACY_SEVEN_FIELD));
    return 0;
}

int PackageFileBackedArtifactReadsLegacyNineFieldRowsWithDefaultPayloadMetadata() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_LEGACY_NINE_FIELD);
    const std::string artifact_text = BuildLegacyNineFieldArtifact(
        ENTRY_TEXTURE,
        TYPE_TEXTURE,
        "texture_legacy_9",
        "textures/legacy_9.bin",
        96ULL,
        24ULL);
    if (!WriteRawPackageArtifact(table, artifact_text)) {
        return Fail("legacy nine-field artifact fixture write failed");
    }

    PackageRegistry artifact_registry;
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = &table;
    read_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    read_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    read_request.registry = &artifact_registry;
    const PackageArtifactResult read_result = ReadPackageArtifact(read_request);
    if (read_result.status != PackageStatus::Success || !read_result.rebuilt_registry) {
        return Fail("legacy nine-field artifact did not rebuild registry");
    }

    const PackageLoadPlanResult plan =
        artifact_registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_legacy_9"));
    if (!plan.Succeeded() || plan.plan.record_count != 1U) {
        return Fail("legacy nine-field artifact did not resolve");
    }

    const PackageLoadPlanRecord& record = plan.plan.records[0U];
    if (record.archive_byte_offset != 96ULL || record.archive_byte_size != 24ULL) {
        return Fail("legacy nine-field artifact changed archive range");
    }

    if (record.payload_logical_byte_count != 24ULL ||
        record.payload_window_byte_offset != 0ULL ||
        record.payload_window_byte_size != 24ULL) {
        return Fail("legacy nine-field artifact did not default payload metadata");
    }

    if (record.payload_hash == 0ULL) {
        return Fail("legacy nine-field artifact did not preserve payload hash");
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_LEGACY_NINE_FIELD));
    return 0;
}

int PackageFileBackedArtifactRoundTripsHashAndDependencyIntegrity() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_HASH_ROUND_TRIP);
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
    if (write_result.status != PackageStatus::Success || !write_result.wrote_artifact) {
        return Fail("hash integrity artifact did not write");
    }

    const std::string artifact_text = ReadRawPackageArtifact(table);
    if (artifact_text.find("dependency_table_hash|") == std::string::npos ||
        artifact_text.find("package_table_hash|") == std::string::npos) {
        return Fail("hash integrity artifact did not include table hashes");
    }

    PackageRegistry artifact_registry;
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = &table;
    read_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    read_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    read_request.registry = &artifact_registry;
    const PackageArtifactResult read_result = ReadPackageArtifact(read_request);
    if (read_result.status != PackageStatus::Success || !read_result.rebuilt_registry) {
        return Fail("hash integrity artifact did not rebuild registry");
    }

    const PackageLoadPlanResult plan =
        artifact_registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey("texture_a"));
    if (!plan.Succeeded() || plan.plan.record_count != 3U) {
        return Fail("hash integrity artifact did not resolve dependency plan");
    }

    if (plan.plan.records[0U].payload_hash == 0ULL ||
        plan.plan.records[1U].payload_hash == 0ULL ||
        plan.plan.records[2U].payload_hash == 0ULL) {
        return Fail("hash integrity artifact did not preserve payload hashes");
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_HASH_ROUND_TRIP));
    return 0;
}

int PackageFileBackedArtifactRejectsHashMismatchesWithoutMutation() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_HASH_VALIDATION);
    const std::array<PackageEntryDescriptor, 2U> entries{
        Entry(PACKAGE_A, ENTRY_TEXTURE, TYPE_TEXTURE, "texture_a", "textures/texture_a.bin", 0U, 16U),
        Entry(PACKAGE_A, ENTRY_MATERIAL, TYPE_MATERIAL, "material_a", "materials/material_a.bin", 16U, 24U)};
    const std::array<PackageArtifactDependency, 1U> dependencies{
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
    if (write_result.status != PackageStatus::Success || !write_result.wrote_artifact) {
        return Fail("hash validation artifact did not write");
    }

    const std::string artifact_text = ReadRawPackageArtifact(table);
    if (artifact_text.empty()) {
        return Fail("hash validation fixture read failed");
    }

    std::string payload_hash_mismatch = artifact_text;
    if (!ReplaceFirstEntryHashField(&payload_hash_mismatch, 0U, "0")) {
        return Fail("hash validation fixture payload edit failed");
    }

    std::string metadata_hash_mismatch = artifact_text;
    if (!ReplaceFirstEntryHashField(&metadata_hash_mismatch, 1U, "0")) {
        return Fail("hash validation fixture metadata edit failed");
    }

    std::string dependency_hash_mismatch = artifact_text;
    if (!ReplaceHashLineValue(&dependency_hash_mismatch, "dependency_table_hash|", "0")) {
        return Fail("hash validation fixture dependency edit failed");
    }

    std::string package_hash_mismatch = artifact_text;
    if (!ReplaceHashLineValue(&package_hash_mismatch, "package_table_hash|", "0")) {
        return Fail("hash validation fixture package edit failed");
    }

    if (ExpectInvalidArtifactRead(
            table,
            payload_hash_mismatch,
            PackageStatus::ArtifactHashMismatch,
            "artifact payload hash mismatch did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            metadata_hash_mismatch,
            PackageStatus::ArtifactHashMismatch,
            "artifact metadata hash mismatch did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            dependency_hash_mismatch,
            PackageStatus::ArtifactHashMismatch,
            "artifact dependency hash mismatch did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            package_hash_mismatch,
            PackageStatus::ArtifactHashMismatch,
            "artifact package hash mismatch did not return explicit status") != 0) {
        return 1;
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_HASH_VALIDATION));
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
    if (read_result.status != PackageStatus::ArtifactParseFailure ||
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

int PackageFileBackedArtifactRejectsManifestParseAndSectionErrorsWithoutMutation() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_MANIFEST_VALIDATION);

    const std::string invalid_package =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|0\nentries|1\ndependencies|0\nentry|1|1|texture_a|textures/texture_a.bin|0|16\nend\n";
    const std::string invalid_manifest_header =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\nmanifest|1\nentries|1\ndependencies|0\nentry|1|1|texture_a|textures/texture_a.bin|0|16\nend\n";
    const std::string bad_count =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|many\ndependencies|0\nentry|1|1|texture_a|textures/texture_a.bin|0|16\nend\n";
    const std::string truncated =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|1|1|texture_a|textures/texture_a.bin|0|16\n";
    const std::string extra_section =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|1|1|texture_a|textures/texture_a.bin|0|16\nend\nextra|1\n";

    if (ExpectInvalidArtifactRead(
            table,
            "not-a-package-artifact\n",
            PackageStatus::ArtifactParseFailure,
            "artifact bad magic did not return parse failure") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            invalid_package,
            PackageStatus::InvalidPackageId,
            "artifact invalid package id did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            invalid_manifest_header,
            PackageStatus::InvalidArtifactManifest,
            "artifact invalid manifest header did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            bad_count,
            PackageStatus::ArtifactBadCount,
            "artifact bad count did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            truncated,
            PackageStatus::ArtifactTruncated,
            "artifact truncated bytes did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            extra_section,
            PackageStatus::ArtifactUnknownSection,
            "artifact extra section did not return explicit status") != 0) {
        return 1;
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_MANIFEST_VALIDATION));
    return 0;
}

int PackageFileBackedArtifactRejectsEntryMetadataWithoutMutation() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_ENTRY_VALIDATION);

    const std::string wrong_table =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nrecord|1|1|texture_a|textures/texture_a.bin|0|16\nend\n";
    const std::string parse_failure =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|entry|1|texture_a|textures/texture_a.bin|0|16\nend\n";
    const std::string invalid_type =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|1|0|texture_a|textures/texture_a.bin|0|16\nend\n";
    const std::string invalid_logical =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|1|1|Texture_A|textures/texture_a.bin|0|16\nend\n";
    const std::string invalid_source =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|1|1|texture_a|Textures/texture_a.bin|0|16\nend\n";
    const std::string zero_range =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|1|1|texture_a|textures/texture_a.bin|0|0\nend\n";
    const std::string overflow_range =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|1|1|texture_a|textures/texture_a.bin|"
        "18446744073709551615|1\nend\n";
    const std::string payload_window_size_mismatch =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|1|1|texture_a|textures/texture_a.bin|"
        "0|16|32|0|15|1|1\nend\n";
    const std::string payload_window_out_of_bounds =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|0\nentry|1|1|texture_a|textures/texture_a.bin|"
        "0|16|16|1|16|1|1\nend\n";
    const std::string duplicate_entry =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|2\ndependencies|0\nentry|1|1|texture_a|textures/texture_a.bin|0|16\n"
        "entry|1|2|material_a|materials/material_a.bin|16|16\nend\n";
    const std::string duplicate_resource_key =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|2\ndependencies|0\nentry|1|1|texture_a|textures/texture_a.bin|0|16\n"
        "entry|2|1|texture_a|textures/texture_a_copy.bin|16|16\nend\n";
    const std::string entry_capacity =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|" + std::to_string(MAX_PACKAGE_ENTRY_COUNT + 1U) + "\ndependencies|0\nend\n";

    if (ExpectInvalidArtifactRead(
            table,
            wrong_table,
            PackageStatus::InvalidArtifactEntryTable,
            "artifact entry table did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            parse_failure,
            PackageStatus::ArtifactParseFailure,
            "artifact entry parse failure did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            invalid_type,
            PackageStatus::InvalidResourceType,
            "artifact invalid resource type did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            invalid_logical,
            PackageStatus::InvalidLogicalKey,
            "artifact invalid logical key did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            invalid_source,
            PackageStatus::InvalidSourceKey,
            "artifact invalid source key did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            zero_range,
            PackageStatus::ByteRangeOutOfBounds,
            "artifact zero byte range did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            overflow_range,
            PackageStatus::ByteRangeOutOfBounds,
            "artifact overflow byte range did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            payload_window_size_mismatch,
            PackageStatus::ByteRangeOutOfBounds,
            "artifact payload window size mismatch did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            payload_window_out_of_bounds,
            PackageStatus::ByteRangeOutOfBounds,
            "artifact payload window out-of-bounds did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            duplicate_entry,
            PackageStatus::DuplicateEntry,
            "artifact duplicate entry did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            duplicate_resource_key,
            PackageStatus::DuplicateResourceKey,
            "artifact duplicate resource key did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            entry_capacity,
            PackageStatus::ArtifactCapacityExceeded,
            "artifact entry capacity did not return explicit status") != 0) {
        return 1;
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_ENTRY_VALIDATION));
    return 0;
}

int PackageFileBackedArtifactRejectsDependencyMetadataWithoutMutation() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_DEPENDENCY_VALIDATION);

    const std::string wrong_table =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|1\nentry|1|1|texture_a|textures/texture_a.bin|0|16\n"
        "edge|1|2\nend\n";
    const std::string parse_failure =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|1\nentry|1|1|texture_a|textures/texture_a.bin|0|16\n"
        "dependency|1|dependency\nend\n";
    const std::string missing_entry =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|1\nentry|1|1|texture_a|textures/texture_a.bin|0|16\n"
        "dependency|1|2\nend\n";
    const std::string cycle =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|1\nentry|1|1|texture_a|textures/texture_a.bin|0|16\n"
        "dependency|1|1\nend\n";
    const std::string dependency_capacity =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|1\ndependencies|" +
        std::to_string(MAX_PACKAGE_DEPENDENCY_EDGE_COUNT + 1U) +
        "\nentry|1|1|texture_a|textures/texture_a.bin|0|16\nend\n";
    const std::string valid_dependency_zero_capacity =
        std::string(PACKAGE_ARTIFACT_MAGIC) +
        "\npackage|1\nentries|2\ndependencies|1\nentry|1|1|texture_a|textures/texture_a.bin|0|16\n"
        "entry|2|2|material_a|materials/material_a.bin|16|16\n"
        "dependency|1|2\nend\n";

    if (ExpectInvalidArtifactRead(
            table,
            wrong_table,
            PackageStatus::InvalidArtifactDependencyTable,
            "artifact dependency table did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            parse_failure,
            PackageStatus::ArtifactParseFailure,
            "artifact dependency parse failure did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            missing_entry,
            PackageStatus::DependencyMissing,
            "artifact missing dependency did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            cycle,
            PackageStatus::DependencyCycle,
            "artifact dependency cycle did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            dependency_capacity,
            PackageStatus::ArtifactCapacityExceeded,
            "artifact dependency capacity did not return explicit status") != 0) {
        return 1;
    }

    if (ExpectInvalidArtifactRead(
            table,
            valid_dependency_zero_capacity,
            PackageStatus::DependencyCapacityExceeded,
            "artifact registry dependency capacity did not return explicit status",
            PackageRegistryDesc{1U, 2U, 0U, 2U}) != 0) {
        return 1;
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_DEPENDENCY_VALIDATION));
    return 0;
}

int PackageFileBackedArtifactReportsCapacityEntryIdentity() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_CAPACITY_ENTRY_IDENTITY);
    std::array<PackageEntryDescriptor, ARTIFACT_CAPACITY_ENTRY_TEST_COUNT> entries{};
    FillArtifactCapacityEntries(&entries);

    PackageArtifactWriteRequest write_request{};
    write_request.mount_table = &table;
    write_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    write_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    write_request.package = PACKAGE_A;
    write_request.entries = entries.data();
    write_request.entry_count = ARTIFACT_CAPACITY_ENTRY_TEST_COUNT;
    PackageArtifactResult write_result = WritePackageArtifact(write_request);
    if (write_result.status != PackageStatus::ArtifactCapacityExceeded || write_result.wrote_artifact) {
        return Fail("artifact write entry capacity did not reject");
    }

    int capacity_entry_status = ExpectArtifactEntryCapacityFields(
        write_result,
        entries[MAX_PACKAGE_ENTRY_COUNT].entry,
        ARTIFACT_CAPACITY_ENTRY_TEST_COUNT);
    if (capacity_entry_status != 0) {
        return capacity_entry_status;
    }

    const std::array<PackageEntryDescriptor, 2U> dependency_entries{
        Entry(PACKAGE_A, ENTRY_TEXTURE, TYPE_TEXTURE, "artifact_dependency_a", "artifacts/dependency_a.bin", 0U, 16U),
        Entry(PACKAGE_A, ENTRY_MATERIAL, TYPE_MATERIAL, "artifact_dependency_b", "artifacts/dependency_b.bin", 16U, 16U)};
    std::array<PackageArtifactDependency, ARTIFACT_CAPACITY_DEPENDENCY_TEST_COUNT> dependencies{};
    FillArtifactCapacityDependencies(&dependencies, ENTRY_TEXTURE, ENTRY_MATERIAL);
    write_request.entries = dependency_entries.data();
    write_request.entry_count = static_cast<std::uint32_t>(dependency_entries.size());
    write_request.dependencies = dependencies.data();
    write_request.dependency_count = ARTIFACT_CAPACITY_DEPENDENCY_TEST_COUNT;
    write_result = WritePackageArtifact(write_request);
    if (write_result.status != PackageStatus::ArtifactCapacityExceeded || write_result.wrote_artifact) {
        return Fail("artifact write dependency capacity did not reject");
    }

    int capacity_dependency_status = ExpectArtifactDependencyCapacityFields(
        write_result,
        ENTRY_TEXTURE,
        ENTRY_MATERIAL,
        ARTIFACT_CAPACITY_DEPENDENCY_TEST_COUNT);
    if (capacity_dependency_status != 0) {
        return capacity_dependency_status;
    }

    const PackageEntryId read_failed_entry{91U};
    const std::string entry_capacity_artifact = BuildEntryCapacityArtifact(read_failed_entry);
    if (!WriteRawPackageArtifact(table, entry_capacity_artifact)) {
        return Fail("artifact read entry capacity fixture write failed");
    }

    PackageRegistry registry;
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = &table;
    read_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    read_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    read_request.registry = &registry;
    PackageArtifactResult read_result = ReadPackageArtifact(read_request);
    if (read_result.status != PackageStatus::ArtifactCapacityExceeded ||
        read_result.read_artifact ||
        read_result.rebuilt_registry) {
        return Fail("artifact read entry capacity did not reject");
    }

    capacity_entry_status = ExpectArtifactEntryCapacityFields(
        read_result,
        read_failed_entry,
        ARTIFACT_CAPACITY_ENTRY_TEST_COUNT);
    if (capacity_entry_status != 0) {
        return capacity_entry_status;
    }

    const std::string dependency_capacity_artifact =
        BuildDependencyCapacityArtifact(ENTRY_TEXTURE, ENTRY_MATERIAL);
    if (!WriteRawPackageArtifact(table, dependency_capacity_artifact)) {
        return Fail("artifact read dependency capacity fixture write failed");
    }

    read_result = ReadPackageArtifact(read_request);
    if (read_result.status != PackageStatus::ArtifactCapacityExceeded ||
        read_result.read_artifact ||
        read_result.rebuilt_registry) {
        return Fail("artifact read dependency capacity did not reject");
    }

    capacity_dependency_status = ExpectArtifactDependencyCapacityFields(
        read_result,
        ENTRY_TEXTURE,
        ENTRY_MATERIAL,
        ARTIFACT_CAPACITY_DEPENDENCY_TEST_COUNT);
    if (capacity_dependency_status != 0) {
        return capacity_dependency_status;
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_CAPACITY_ENTRY_IDENTITY));
    return 0;
}

int PackageFileBackedArtifactWriteRejectsInvalidMetadataWithoutFileWrite() {
    MountTable table = CreatePackageArtifactTable(TEST_ARTIFACT_WRITE_INVALID);
    const std::array<PackageEntryDescriptor, 1U> entries{
        Entry(PACKAGE_A, ENTRY_TEXTURE, TYPE_TEXTURE, "Texture_A", "textures/texture_a.bin", 0U, 16U)};

    PackageArtifactWriteRequest write_request{};
    write_request.mount_table = &table;
    write_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    write_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    write_request.package = PACKAGE_A;
    write_request.entries = entries.data();
    write_request.entry_count = static_cast<std::uint32_t>(entries.size());
    const PackageArtifactResult write_result = WritePackageArtifact(write_request);
    if (write_result.status != PackageStatus::InvalidLogicalKey || write_result.wrote_artifact) {
        return Fail("invalid artifact write metadata did not return explicit status");
    }

    if (table.Snapshot().write_byte_count != 0U) {
        return Fail("invalid artifact write mutated File/VFS bytes");
    }

    std::filesystem::remove_all(PackageArtifactRoot(TEST_ARTIFACT_WRITE_INVALID));
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
        {TEST_REGISTRATION_CAPACITY_IDENTITY, PackageRegistrationCapacityFailureIdentityClearsForNonCapacityFailures},
        {TEST_OVERSIZED_KEYS, PackageRegisterEntryRejectsOversizedKeysWithoutMutation},
        {TEST_OVERSIZED_BYTE_RANGE, PackageRegisterEntryRejectsOversizedByteRangeWithoutMutation},
        {TEST_LARGE_BYTE_RANGE_INDEX, PackageRegisterEntryAcceptsLargeByteRangeAndResolvePreservesIndexMetadata},
        {TEST_PAYLOAD_METADATA_LOAD_PLAN, PackageLoadPlanPreservesPayloadWindowMetadataDistinctFromArchiveRange},
        {TEST_RESOLVE, PackageResolveEntryByResourceKeyReturnsDeterministicLoadPlan},
        {TEST_RESOLVE_RESOURCE_KEY_TUPLE, PackageResolveEntryByResourceKeyUsesTypeAndLogicalKeyTuple},
        {TEST_INDEXED_LOOKUPS, PackageIndexedLookupsPreserveStatusOrderAndCapacities},
        {TEST_ACCEPTED_STATUS, PackageAcceptedOperationCountTracksSuccessAfterFailureAndDuplicateDependency},
        {TEST_UNKNOWN_KEY, PackageResolveRejectsUnknownResourceKey},
        {TEST_TYPE_MISMATCH, PackageResolveRejectsTypeMismatchWithoutMutation},
        {TEST_MISSING_DEPENDENCY, PackageDependencyValidationRejectsMissingEntry},
        {TEST_DEPENDENCY_CYCLE, PackageDependencyValidationRejectsCycle},
        {TEST_DEPENDENCY_CYCLE_HIGH_FANOUT, PackageDependencyValidationRejectsCycleAfterHighFanout},
        {TEST_DEPENDENCY_ORDER, PackageDependencyPlanPreservesDeclarationOrder},
        {TEST_DEPENDENCY_TRANSITIVE_CLOSURE, PackageDependencyClosurePlanIncludesTransitiveDependenciesBeforeRoot},
        {TEST_DEPENDENCY_CLOSURE_DEDUP,
         PackageDependencyClosureDeduplicatesSharedDependenciesAndPreservesDeclarationOrder},
        {TEST_DEPENDENCY_CLOSURE_RECORD_BUDGET, PackageDependencyClosureRejectsRecordBudgetWithoutMutation},
        {TEST_DEPENDENCY_CAPACITY, PackageDependencyCapacityOverflowDoesNotMutate},
        {TEST_LOAD_PLAN_CAPACITY, PackageLoadPlanCapacityOverflowDoesNotMutate},
        {TEST_LOAD_PLAN_BYTE_BUDGET, PackageLoadPlanRejectsArchiveByteBudgetWithoutMutation},
        {TEST_DISABLED_DIAGNOSTICS, PackageDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_FILE_ORIGINAL, PackageNoFileReadOriginalPackageOrGameAdapterDependency},
        {TEST_NO_HIDDEN_ALLOCATION, PackageNoHiddenAllocationUsesYuMemorySignal},
        {TEST_ARTIFACT_ROUND_TRIP, PackageFileBackedArtifactRoundTripsLoadPlanThroughFileVfs},
        {TEST_ARTIFACT_LARGE_BYTE_RANGE, PackageFileBackedArtifactRoundTripsLargeByteRangeMetadata},
        {TEST_ARTIFACT_PAYLOAD_METADATA, PackageFileBackedArtifactRoundTripsPayloadWindowMetadata},
        {TEST_ARTIFACT_LEGACY_SEVEN_FIELD, PackageFileBackedArtifactReadsLegacySevenFieldRowsWithDefaultPayloadMetadata},
        {TEST_ARTIFACT_LEGACY_NINE_FIELD, PackageFileBackedArtifactReadsLegacyNineFieldRowsWithDefaultPayloadMetadata},
        {TEST_ARTIFACT_HASH_ROUND_TRIP, PackageFileBackedArtifactRoundTripsHashAndDependencyIntegrity},
        {TEST_ARTIFACT_HASH_VALIDATION, PackageFileBackedArtifactRejectsHashMismatchesWithoutMutation},
        {TEST_ARTIFACT_INVALID, PackageFileBackedArtifactRejectsInvalidBytesWithoutMutation},
        {TEST_ARTIFACT_MANIFEST_VALIDATION,
         PackageFileBackedArtifactRejectsManifestParseAndSectionErrorsWithoutMutation},
        {TEST_ARTIFACT_ENTRY_VALIDATION, PackageFileBackedArtifactRejectsEntryMetadataWithoutMutation},
        {TEST_ARTIFACT_DEPENDENCY_VALIDATION, PackageFileBackedArtifactRejectsDependencyMetadataWithoutMutation},
        {TEST_ARTIFACT_CAPACITY_ENTRY_IDENTITY, PackageFileBackedArtifactReportsCapacityEntryIdentity},
        {TEST_ARTIFACT_WRITE_INVALID, PackageFileBackedArtifactWriteRejectsInvalidMetadataWithoutFileWrite},
        {TEST_ARTIFACT_MISSING, PackageFileBackedArtifactReportsMissingFile}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
