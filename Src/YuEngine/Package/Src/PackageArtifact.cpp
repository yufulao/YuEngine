// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Src/PackageArtifact.cpp

#include "YuEngine/Package/PackageArtifact.h"

#include <array>
#include <charconv>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/FileWriteResult.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"

namespace yuengine::package {
namespace {
constexpr std::string_view PACKAGE_ARTIFACT_MAGIC = "YUPACKAGE_ARTIFACT_V1";
constexpr std::uint64_t ARTIFACT_HASH_OFFSET = 14695981039346656037ULL;
constexpr std::uint64_t ARTIFACT_HASH_MULTIPLIER = 1099511628211ULL;

struct ParsedArtifactEntry final {
    PackageEntryDescriptor descriptor;
    std::uint64_t metadata_hash = 0ULL;
    bool has_integrity_hashes = false;
    bool has_payload_metadata = false;
};

struct ArtifactIntegrityRecord final {
    std::uint64_t dependency_table_hash = 0ULL;
    std::uint64_t package_table_hash = 0ULL;
};

PackageArtifactResult PackageArtifactFailure(
    PackageStatus package_status,
    yuengine::file::FileStatus file_status = yuengine::file::FileStatus::Success) {
    PackageArtifactResult result{};
    result.status = package_status;
    result.file_status = file_status;
    return result;
}

PackageStatus ValidateArtifactRecordCounts(std::uint32_t entry_count, std::uint32_t dependency_count) {
    if (entry_count == 0U) {
        return PackageStatus::InvalidArtifactEntryTable;
    }

    if (entry_count > MAX_PACKAGE_ENTRY_COUNT || dependency_count > MAX_PACKAGE_DEPENDENCY_EDGE_COUNT) {
        return PackageStatus::ArtifactCapacityExceeded;
    }

    return PackageStatus::Success;
}

std::uint64_t GetArchiveByteOffset(const PackageEntryDescriptor &entry) {
    if (entry.archive_byte_size > 0ULL) {
        return entry.archive_byte_offset;
    }

    return entry.byte_offset;
}

std::uint64_t GetArchiveByteSize(const PackageEntryDescriptor &entry) {
    if (entry.archive_byte_size > 0ULL) {
        return entry.archive_byte_size;
    }

    return entry.byte_size;
}

bool HasExplicitPayloadMetadata(const PackageEntryDescriptor &entry) {
    if (entry.payload_logical_byte_count != 0ULL) {
        return true;
    }

    if (entry.payload_window_byte_offset != 0ULL) {
        return true;
    }

    return entry.payload_window_byte_size != 0ULL;
}

std::uint64_t GetPayloadLogicalByteCount(const PackageEntryDescriptor &entry) {
    if (HasExplicitPayloadMetadata(entry)) {
        return entry.payload_logical_byte_count;
    }

    return GetArchiveByteSize(entry);
}

std::uint64_t GetPayloadWindowByteOffset(const PackageEntryDescriptor &entry) {
    if (HasExplicitPayloadMetadata(entry)) {
        return entry.payload_window_byte_offset;
    }

    return 0ULL;
}

std::uint64_t GetPayloadWindowByteSize(const PackageEntryDescriptor &entry) {
    if (HasExplicitPayloadMetadata(entry)) {
        return entry.payload_window_byte_size;
    }

    return GetArchiveByteSize(entry);
}

std::uint32_t MakeLegacyByteValue(std::uint64_t value) {
    const std::uint64_t max_legacy_value = std::numeric_limits<std::uint32_t>::max();
    if (value > max_legacy_value) {
        return 0U;
    }

    return static_cast<std::uint32_t>(value);
}

std::uint64_t MakeNonZeroHash(std::uint64_t hash) {
    if (hash == 0ULL) {
        return ARTIFACT_HASH_OFFSET;
    }

    return hash;
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

std::uint64_t GetEntryPayloadHash(const PackageEntryDescriptor &entry) {
    if (entry.payload_hash != 0ULL) {
        return entry.payload_hash;
    }

    std::uint64_t hash = ARTIFACT_HASH_OFFSET;
    hash = HashArtifactText(hash, entry.source_key.Value());
    hash = MixArtifactHash(hash, GetArchiveByteOffset(entry));
    hash = MixArtifactHash(hash, GetArchiveByteSize(entry));
    return MakeNonZeroHash(hash);
}

std::uint64_t ComputeEntryMetadataHash(const PackageEntryDescriptor &entry, bool include_payload_metadata) {
    std::uint64_t hash = ARTIFACT_HASH_OFFSET;
    hash = MixArtifactHash(hash, entry.package.value);
    hash = MixArtifactHash(hash, entry.entry.value);
    hash = MixArtifactHash(hash, entry.type.value);
    hash = HashArtifactText(hash, entry.logical_key.Value());
    hash = HashArtifactText(hash, entry.source_key.Value());
    hash = MixArtifactHash(hash, GetArchiveByteOffset(entry));
    hash = MixArtifactHash(hash, GetArchiveByteSize(entry));
    if (include_payload_metadata) {
        hash = MixArtifactHash(hash, GetPayloadLogicalByteCount(entry));
        hash = MixArtifactHash(hash, GetPayloadWindowByteOffset(entry));
        hash = MixArtifactHash(hash, GetPayloadWindowByteSize(entry));
    }

    hash = MixArtifactHash(hash, GetEntryPayloadHash(entry));
    return MakeNonZeroHash(hash);
}

std::uint64_t ComputeDependencyTableHash(
    const PackageArtifactDependency *dependencies,
    std::uint32_t dependency_count) {
    std::uint64_t hash = ARTIFACT_HASH_OFFSET;
    hash = MixArtifactHash(hash, dependency_count);
    for (std::uint32_t index = 0U; index < dependency_count; ++index) {
        hash = MixArtifactHash(hash, dependencies[index].dependent.value);
        hash = MixArtifactHash(hash, dependencies[index].dependency.value);
    }

    return MakeNonZeroHash(hash);
}

std::uint64_t ComputePackageTableHash(
    PackageId package,
    const PackageEntryDescriptor *entries,
    std::uint32_t entry_count,
    const PackageArtifactDependency *dependencies,
    std::uint32_t dependency_count) {
    std::uint64_t hash = ARTIFACT_HASH_OFFSET;
    hash = MixArtifactHash(hash, package.value);
    hash = MixArtifactHash(hash, entry_count);
    hash = MixArtifactHash(hash, dependency_count);
    for (std::uint32_t index = 0U; index < entry_count; ++index) {
        hash = MixArtifactHash(hash, entries[index].entry.value);
        hash = MixArtifactHash(hash, GetEntryPayloadHash(entries[index]));
        hash = MixArtifactHash(hash, ComputeEntryMetadataHash(entries[index], true));
    }

    hash = MixArtifactHash(hash, ComputeDependencyTableHash(dependencies, dependency_count));
    return MakeNonZeroHash(hash);
}

std::uint64_t ComputeParsedPackageTableHash(
    PackageId package,
    const ParsedArtifactEntry *entries,
    std::uint32_t entry_count,
    const PackageArtifactDependency *dependencies,
    std::uint32_t dependency_count) {
    std::uint64_t hash = ARTIFACT_HASH_OFFSET;
    hash = MixArtifactHash(hash, package.value);
    hash = MixArtifactHash(hash, entry_count);
    hash = MixArtifactHash(hash, dependency_count);
    for (std::uint32_t index = 0U; index < entry_count; ++index) {
        const PackageEntryDescriptor &entry = entries[index].descriptor;
        hash = MixArtifactHash(hash, entry.entry.value);
        hash = MixArtifactHash(hash, GetEntryPayloadHash(entry));
        hash = MixArtifactHash(hash, ComputeEntryMetadataHash(entry, entries[index].has_payload_metadata));
    }

    hash = MixArtifactHash(hash, ComputeDependencyTableHash(dependencies, dependency_count));
    return MakeNonZeroHash(hash);
}

PackageEntryDescriptor NormalizeArtifactEntry(const PackageEntryDescriptor &entry) {
    PackageEntryDescriptor normalized = entry;
    const std::uint64_t archive_byte_offset = GetArchiveByteOffset(entry);
    const std::uint64_t archive_byte_size = GetArchiveByteSize(entry);
    normalized.byte_offset = MakeLegacyByteValue(archive_byte_offset);
    normalized.byte_size = MakeLegacyByteValue(archive_byte_size);
    normalized.archive_byte_offset = archive_byte_offset;
    normalized.archive_byte_size = archive_byte_size;
    normalized.payload_logical_byte_count = GetPayloadLogicalByteCount(entry);
    normalized.payload_window_byte_offset = GetPayloadWindowByteOffset(entry);
    normalized.payload_window_byte_size = GetPayloadWindowByteSize(entry);
    normalized.payload_hash = GetEntryPayloadHash(normalized);
    return normalized;
}

PackageStatus BuildRegistryFromRecords(
    PackageRegistry &registry,
    PackageId package,
    const PackageEntryDescriptor *entries,
    std::uint32_t entry_count,
    const PackageArtifactDependency *dependencies,
    std::uint32_t dependency_count) {
    if (!package.IsValid()) {
        return PackageStatus::InvalidPackageId;
    }

    if (entries == nullptr || entry_count == 0U) {
        return PackageStatus::InvalidArtifactEntryTable;
    }

    if (dependencies == nullptr && dependency_count > 0U) {
        return PackageStatus::InvalidArtifactDependencyTable;
    }

    const PackageStatus count_status = ValidateArtifactRecordCounts(entry_count, dependency_count);
    if (count_status != PackageStatus::Success) {
        return count_status;
    }

    const PackageRegistrationResult manifest_result = registry.RegisterSyntheticManifest({package});
    if (!manifest_result.Succeeded()) {
        return manifest_result.status;
    }

    for (std::uint32_t index = 0U; index < entry_count; ++index) {
        const PackageRegistrationResult entry_result = registry.RegisterEntry(entries[index]);
        if (!entry_result.Succeeded()) {
            return entry_result.status;
        }
    }

    for (std::uint32_t index = 0U; index < dependency_count; ++index) {
        const PackageStatus dependency_status =
            registry.AddDependency(package, dependencies[index].dependent, dependencies[index].dependency);
        if (dependency_status != PackageStatus::Success) {
            return dependency_status;
        }
    }

    return PackageStatus::Success;
}

std::string BuildArtifactText(
    PackageId package,
    const PackageEntryDescriptor *entries,
    std::uint32_t entry_count,
    const PackageArtifactDependency *dependencies,
    std::uint32_t dependency_count) {
    std::array<PackageEntryDescriptor, MAX_PACKAGE_ENTRY_COUNT> normalized_entries{};
    for (std::uint32_t index = 0U; index < entry_count; ++index) {
        normalized_entries[index] = NormalizeArtifactEntry(entries[index]);
    }

    std::ostringstream output;
    output << PACKAGE_ARTIFACT_MAGIC << '\n';
    output << "package|" << package.value << '\n';
    output << "entries|" << entry_count << '\n';
    output << "dependencies|" << dependency_count << '\n';
    for (std::uint32_t index = 0U; index < entry_count; ++index) {
        const PackageEntryDescriptor &entry = normalized_entries[index];
        const std::uint64_t archive_byte_offset = GetArchiveByteOffset(entry);
        const std::uint64_t archive_byte_size = GetArchiveByteSize(entry);
        const std::uint64_t payload_logical_byte_count = GetPayloadLogicalByteCount(entry);
        const std::uint64_t payload_window_byte_offset = GetPayloadWindowByteOffset(entry);
        const std::uint64_t payload_window_byte_size = GetPayloadWindowByteSize(entry);
        const std::uint64_t payload_hash = GetEntryPayloadHash(entry);
        const std::uint64_t metadata_hash = ComputeEntryMetadataHash(entry, true);
        output << "entry|"
               << entry.entry.value << '|'
               << entry.type.value << '|'
               << entry.logical_key.Value() << '|'
               << entry.source_key.Value() << '|'
               << archive_byte_offset << '|'
               << archive_byte_size << '|'
               << payload_logical_byte_count << '|'
               << payload_window_byte_offset << '|'
               << payload_window_byte_size << '|'
               << payload_hash << '|'
               << metadata_hash << '\n';
    }

    for (std::uint32_t index = 0U; index < dependency_count; ++index) {
        output << "dependency|"
               << dependencies[index].dependent.value << '|'
               << dependencies[index].dependency.value << '\n';
    }

    const std::uint64_t dependency_table_hash = ComputeDependencyTableHash(dependencies, dependency_count);
    const std::uint64_t package_table_hash = ComputePackageTableHash(
        package,
        normalized_entries.data(),
        entry_count,
        dependencies,
        dependency_count);
    output << "dependency_table_hash|" << dependency_table_hash << '\n';
    output << "package_table_hash|" << package_table_hash << '\n';
    output << "end\n";
    return output.str();
}

std::vector<std::string> SplitLines(std::string_view text) {
    std::vector<std::string> lines;
    std::string line;
    std::istringstream input{std::string(text)};
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        lines.push_back(line);
    }

    return lines;
}

std::vector<std::string_view> SplitFields(std::string_view line) {
    std::vector<std::string_view> fields;
    std::size_t start = 0U;
    while (start <= line.size()) {
        const std::size_t separator = line.find('|', start);
        if (separator == std::string_view::npos) {
            fields.push_back(line.substr(start));
            return fields;
        }

        fields.push_back(line.substr(start, separator - start));
        start = separator + 1U;
    }

    return fields;
}

bool ParseUInt32(std::string_view text, std::uint32_t *out_value) {
    if (text.empty() || out_value == nullptr) {
        return false;
    }

    std::uint32_t value = 0U;
    const char *first = text.data();
    const char *last = text.data() + text.size();
    const std::from_chars_result result = std::from_chars(first, last, value);
    if (result.ec != std::errc{} || result.ptr != last) {
        return false;
    }

    *out_value = value;
    return true;
}

bool ParseUInt64(std::string_view text, std::uint64_t *out_value) {
    if (text.empty() || out_value == nullptr) {
        return false;
    }

    std::uint64_t value = 0ULL;
    const char *first = text.data();
    const char *last = text.data() + text.size();
    const std::from_chars_result result = std::from_chars(first, last, value);
    if (result.ec != std::errc{} || result.ptr != last) {
        return false;
    }

    *out_value = value;
    return true;
}

PackageStatus ParseHeaderCountLine(
    const std::string &line,
    std::string_view expected_token,
    std::uint32_t *out_value) {
    const std::vector<std::string_view> fields = SplitFields(line);
    if (fields.size() != 2U || fields[0] != expected_token) {
        return PackageStatus::InvalidArtifactManifest;
    }

    if (!ParseUInt32(fields[1], out_value)) {
        return PackageStatus::ArtifactBadCount;
    }

    return PackageStatus::Success;
}

PackageStatus ParsePackageLine(const std::string &line, std::uint32_t *out_value) {
    const std::vector<std::string_view> fields = SplitFields(line);
    if (fields.size() != 2U || fields[0] != "package") {
        return PackageStatus::InvalidArtifactManifest;
    }

    if (!ParseUInt32(fields[1], out_value)) {
        return PackageStatus::ArtifactParseFailure;
    }

    return PackageStatus::Success;
}

PackageStatus ParseEntryLine(
    const std::string &line,
    PackageId package,
    ParsedArtifactEntry *out_entry) {
    if (out_entry == nullptr) {
        return PackageStatus::InvalidArtifactEntryTable;
    }

    const std::vector<std::string_view> fields = SplitFields(line);
    if ((fields.size() != 7U && fields.size() != 9U && fields.size() != 12U) || fields[0] != "entry") {
        return PackageStatus::InvalidArtifactEntryTable;
    }

    std::uint32_t entry_value = 0U;
    std::uint32_t type_value = 0U;
    std::uint64_t byte_offset = 0ULL;
    std::uint64_t byte_size = 0ULL;
    std::uint64_t payload_logical_byte_count = 0ULL;
    std::uint64_t payload_window_byte_offset = 0ULL;
    std::uint64_t payload_window_byte_size = 0ULL;
    std::uint64_t payload_hash = 0ULL;
    std::uint64_t metadata_hash = 0ULL;
    if (!ParseUInt32(fields[1], &entry_value) ||
        !ParseUInt32(fields[2], &type_value) ||
        !ParseUInt64(fields[5], &byte_offset) ||
        !ParseUInt64(fields[6], &byte_size)) {
        return PackageStatus::ArtifactParseFailure;
    }

    bool has_integrity_hashes = false;
    bool has_payload_metadata = false;
    if (fields.size() == 9U) {
        if (!ParseUInt64(fields[7], &payload_hash) || !ParseUInt64(fields[8], &metadata_hash)) {
            return PackageStatus::ArtifactParseFailure;
        }

        has_integrity_hashes = true;
    }

    if (fields.size() == 12U) {
        if (!ParseUInt64(fields[7], &payload_logical_byte_count) ||
            !ParseUInt64(fields[8], &payload_window_byte_offset) ||
            !ParseUInt64(fields[9], &payload_window_byte_size) ||
            !ParseUInt64(fields[10], &payload_hash) ||
            !ParseUInt64(fields[11], &metadata_hash)) {
            return PackageStatus::ArtifactParseFailure;
        }

        has_integrity_hashes = true;
        has_payload_metadata = true;
    }

    const std::uint32_t legacy_byte_offset = MakeLegacyByteValue(byte_offset);
    const std::uint32_t legacy_byte_size = MakeLegacyByteValue(byte_size);
    if (!has_payload_metadata) {
        payload_logical_byte_count = byte_size;
        payload_window_byte_offset = 0ULL;
        payload_window_byte_size = byte_size;
    }

    out_entry->descriptor = PackageEntryDescriptor{
        package,
        PackageEntryId{entry_value},
        yuengine::resource::ResourceTypeId{type_value},
        yuengine::resource::ResourceLogicalKey(fields[3]),
        PackageSourceKey(fields[4]),
        legacy_byte_offset,
        legacy_byte_size,
        byte_offset,
        byte_size,
        payload_hash,
        payload_logical_byte_count,
        payload_window_byte_offset,
        payload_window_byte_size};
    out_entry->metadata_hash = metadata_hash;
    out_entry->has_integrity_hashes = has_integrity_hashes;
    out_entry->has_payload_metadata = has_payload_metadata;
    return PackageStatus::Success;
}

PackageStatus ParseDependencyLine(
    const std::string &line,
    PackageArtifactDependency *out_dependency) {
    if (out_dependency == nullptr) {
        return PackageStatus::InvalidArtifactDependencyTable;
    }

    const std::vector<std::string_view> fields = SplitFields(line);
    if (fields.size() != 3U || fields[0] != "dependency") {
        return PackageStatus::InvalidArtifactDependencyTable;
    }

    std::uint32_t dependent = 0U;
    std::uint32_t dependency = 0U;
    if (!ParseUInt32(fields[1], &dependent) || !ParseUInt32(fields[2], &dependency)) {
        return PackageStatus::ArtifactParseFailure;
    }

    *out_dependency = PackageArtifactDependency{PackageEntryId{dependent}, PackageEntryId{dependency}};
    return PackageStatus::Success;
}

PackageStatus ParseArtifactHashLine(
    const std::string &line,
    std::string_view expected_token,
    std::uint64_t *out_hash) {
    if (out_hash == nullptr) {
        return PackageStatus::InvalidArtifactIntegrityTable;
    }

    const std::vector<std::string_view> fields = SplitFields(line);
    if (fields.size() != 2U || fields[0] != expected_token) {
        return PackageStatus::InvalidArtifactIntegrityTable;
    }

    if (!ParseUInt64(fields[1], out_hash)) {
        return PackageStatus::ArtifactParseFailure;
    }

    if (*out_hash == 0ULL) {
        return PackageStatus::ArtifactHashMismatch;
    }

    return PackageStatus::Success;
}

PackageStatus ParseArtifactIntegrityLines(
    const std::vector<std::string> &lines,
    std::size_t *line_index,
    ArtifactIntegrityRecord *out_integrity) {
    if (line_index == nullptr || out_integrity == nullptr) {
        return PackageStatus::InvalidArtifactIntegrityTable;
    }

    if (*line_index >= lines.size()) {
        return PackageStatus::ArtifactTruncated;
    }

    PackageStatus parse_status = ParseArtifactHashLine(
        lines[*line_index],
        "dependency_table_hash",
        &out_integrity->dependency_table_hash);
    if (parse_status != PackageStatus::Success) {
        return parse_status;
    }

    ++(*line_index);
    if (*line_index >= lines.size()) {
        return PackageStatus::ArtifactTruncated;
    }

    parse_status = ParseArtifactHashLine(lines[*line_index], "package_table_hash", &out_integrity->package_table_hash);
    if (parse_status != PackageStatus::Success) {
        return parse_status;
    }

    ++(*line_index);
    return PackageStatus::Success;
}

PackageStatus ValidateArtifactIntegrity(
    PackageId package,
    const ParsedArtifactEntry *entries,
    std::uint32_t entry_count,
    const PackageArtifactDependency *dependencies,
    std::uint32_t dependency_count,
    const ArtifactIntegrityRecord &integrity) {
    if (entries == nullptr) {
        return PackageStatus::InvalidArtifactEntryTable;
    }

    for (std::uint32_t index = 0U; index < entry_count; ++index) {
        if (!entries[index].has_integrity_hashes) {
            return PackageStatus::InvalidArtifactIntegrityTable;
        }

        if (entries[index].descriptor.payload_hash == 0ULL) {
            return PackageStatus::ArtifactHashMismatch;
        }

        const std::uint64_t metadata_hash = ComputeEntryMetadataHash(
            entries[index].descriptor,
            entries[index].has_payload_metadata);
        if (entries[index].metadata_hash != metadata_hash) {
            return PackageStatus::ArtifactHashMismatch;
        }
    }

    const std::uint64_t dependency_table_hash = ComputeDependencyTableHash(dependencies, dependency_count);
    if (integrity.dependency_table_hash != dependency_table_hash) {
        return PackageStatus::ArtifactHashMismatch;
    }

    const std::uint64_t package_table_hash =
        ComputeParsedPackageTableHash(package, entries, entry_count, dependencies, dependency_count);
    if (integrity.package_table_hash != package_table_hash) {
        return PackageStatus::ArtifactHashMismatch;
    }

    return PackageStatus::Success;
}

bool AllEntriesUseLegacyArtifactFormat(const ParsedArtifactEntry *entries, std::uint32_t entry_count) {
    if (entries == nullptr) {
        return false;
    }

    for (std::uint32_t index = 0U; index < entry_count; ++index) {
        if (entries[index].has_integrity_hashes) {
            return false;
        }

        if (entries[index].has_payload_metadata) {
            return false;
        }
    }

    return true;
}

PackageArtifactResult ReadArtifactText(
    std::string_view text,
    PackageRegistryDesc registry_desc,
    PackageRegistry *registry) {
    if (registry == nullptr) {
        return PackageArtifactFailure(PackageStatus::InvalidArtifact);
    }

    const std::vector<std::string> lines = SplitLines(text);
    if (lines.empty()) {
        return PackageArtifactFailure(PackageStatus::ArtifactTruncated);
    }

    if (lines[0] != PACKAGE_ARTIFACT_MAGIC) {
        return PackageArtifactFailure(PackageStatus::ArtifactParseFailure);
    }

    if (lines.size() < 4U) {
        return PackageArtifactFailure(PackageStatus::ArtifactTruncated);
    }

    std::uint32_t package_value = 0U;
    std::uint32_t entry_count = 0U;
    std::uint32_t dependency_count = 0U;
    PackageStatus parse_status = ParsePackageLine(lines[1], &package_value);
    if (parse_status != PackageStatus::Success) {
        return PackageArtifactFailure(parse_status);
    }

    parse_status = ParseHeaderCountLine(lines[2], "entries", &entry_count);
    if (parse_status != PackageStatus::Success) {
        return PackageArtifactFailure(parse_status);
    }

    parse_status = ParseHeaderCountLine(lines[3], "dependencies", &dependency_count);
    if (parse_status != PackageStatus::Success) {
        return PackageArtifactFailure(parse_status);
    }

    parse_status = ValidateArtifactRecordCounts(entry_count, dependency_count);
    if (parse_status != PackageStatus::Success) {
        return PackageArtifactFailure(parse_status);
    }

    std::array<ParsedArtifactEntry, MAX_PACKAGE_ENTRY_COUNT> parsed_entries{};
    std::array<PackageEntryDescriptor, MAX_PACKAGE_ENTRY_COUNT> entries{};
    std::array<PackageArtifactDependency, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> dependencies{};
    const PackageId package{package_value};
    std::size_t line_index = 4U;
    for (std::uint32_t index = 0U; index < entry_count; ++index) {
        if (line_index >= lines.size()) {
            return PackageArtifactFailure(PackageStatus::ArtifactTruncated);
        }

        parse_status = ParseEntryLine(lines[line_index], package, &parsed_entries[index]);
        if (parse_status != PackageStatus::Success) {
            return PackageArtifactFailure(parse_status);
        }

        entries[index] = parsed_entries[index].descriptor;
        ++line_index;
    }

    for (std::uint32_t index = 0U; index < dependency_count; ++index) {
        if (line_index >= lines.size()) {
            return PackageArtifactFailure(PackageStatus::ArtifactTruncated);
        }

        parse_status = ParseDependencyLine(lines[line_index], &dependencies[index]);
        if (parse_status != PackageStatus::Success) {
            return PackageArtifactFailure(parse_status);
        }

        ++line_index;
    }

    PackageRegistry staged_registry(registry_desc);
    const PackageStatus build_status = BuildRegistryFromRecords(
        staged_registry,
        package,
        entries.data(),
        entry_count,
        dependencies.data(),
        dependency_count);
    if (build_status != PackageStatus::Success) {
        return PackageArtifactFailure(build_status);
    }

    const bool uses_legacy_artifact_format = AllEntriesUseLegacyArtifactFormat(parsed_entries.data(), entry_count);
    if (!uses_legacy_artifact_format || line_index >= lines.size() || lines[line_index] != "end") {
        ArtifactIntegrityRecord integrity{};
        parse_status = ParseArtifactIntegrityLines(lines, &line_index, &integrity);
        if (parse_status != PackageStatus::Success) {
            return PackageArtifactFailure(parse_status);
        }

        parse_status = ValidateArtifactIntegrity(
            package,
            parsed_entries.data(),
            entry_count,
            dependencies.data(),
            dependency_count,
            integrity);
        if (parse_status != PackageStatus::Success) {
            return PackageArtifactFailure(parse_status);
        }
    }

    if (line_index >= lines.size()) {
        return PackageArtifactFailure(PackageStatus::ArtifactTruncated);
    }

    if (lines[line_index] != "end") {
        return PackageArtifactFailure(PackageStatus::ArtifactTruncated);
    }

    ++line_index;
    if (line_index < lines.size()) {
        return PackageArtifactFailure(PackageStatus::ArtifactUnknownSection);
    }

    *registry = staged_registry;
    PackageArtifactResult result{};
    result.status = PackageStatus::Success;
    result.artifact_byte_count = text.size();
    result.manifest_count = 1U;
    result.entry_count = entry_count;
    result.dependency_count = dependency_count;
    result.read_artifact = true;
    result.rebuilt_registry = true;
    return result;
}
}

PackageArtifactResult WritePackageArtifact(const PackageArtifactWriteRequest &request) {
    if (request.mount_table == nullptr) {
        return PackageArtifactFailure(PackageStatus::InvalidArtifact);
    }

    PackageRegistry staged_registry;
    const PackageStatus build_status = BuildRegistryFromRecords(
        staged_registry,
        request.package,
        request.entries,
        request.entry_count,
        request.dependencies,
        request.dependency_count);
    if (build_status != PackageStatus::Success) {
        return PackageArtifactFailure(build_status);
    }

    const std::string artifact_text = BuildArtifactText(
        request.package,
        request.entries,
        request.entry_count,
        request.dependencies,
        request.dependency_count);
    const yuengine::file::FileWriteResult write_result = request.mount_table->Write(
        {request.mount,
         request.artifact_path,
         reinterpret_cast<const std::uint8_t *>(artifact_text.data()),
         artifact_text.size()});
    if (!write_result.Succeeded()) {
        return PackageArtifactFailure(PackageStatus::FileWriteFailed, write_result.status);
    }

    PackageArtifactResult result{};
    result.status = PackageStatus::Success;
    result.file_status = write_result.status;
    result.artifact_byte_count = write_result.byte_count;
    result.manifest_count = 1U;
    result.entry_count = request.entry_count;
    result.dependency_count = request.dependency_count;
    result.wrote_artifact = true;
    return result;
}

PackageArtifactResult ReadPackageArtifact(const PackageArtifactReadRequest &request) {
    if (request.mount_table == nullptr || request.registry == nullptr) {
        return PackageArtifactFailure(PackageStatus::InvalidArtifact);
    }

    const yuengine::file::FileReadResult read_result =
        request.mount_table->Read({request.mount, request.artifact_path});
    if (!read_result.Succeeded()) {
        return PackageArtifactFailure(PackageStatus::FileReadFailed, read_result.status);
    }

    PackageArtifactResult result = ReadArtifactText(
        std::string_view(
            reinterpret_cast<const char *>(read_result.bytes.data()),
            read_result.bytes.size()),
        request.registry_desc,
        request.registry);
    result.file_status = read_result.status;
    result.artifact_byte_count = read_result.bytes.size();
    result.read_artifact = result.status == PackageStatus::Success;
    return result;
}
}
