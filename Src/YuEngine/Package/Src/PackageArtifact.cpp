// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Src/PackageArtifact.cpp

#include "YuEngine/Package/PackageArtifact.h"

#include <array>
#include <charconv>
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
    std::ostringstream output;
    output << PACKAGE_ARTIFACT_MAGIC << '\n';
    output << "package|" << package.value << '\n';
    output << "entries|" << entry_count << '\n';
    output << "dependencies|" << dependency_count << '\n';
    for (std::uint32_t index = 0U; index < entry_count; ++index) {
        const PackageEntryDescriptor &entry = entries[index];
        output << "entry|"
               << entry.entry.value << '|'
               << entry.type.value << '|'
               << entry.logical_key.Value() << '|'
               << entry.source_key.Value() << '|'
               << entry.byte_offset << '|'
               << entry.byte_size << '\n';
    }

    for (std::uint32_t index = 0U; index < dependency_count; ++index) {
        output << "dependency|"
               << dependencies[index].dependent.value << '|'
               << dependencies[index].dependency.value << '\n';
    }

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
    PackageEntryDescriptor *out_entry) {
    if (out_entry == nullptr) {
        return PackageStatus::InvalidArtifactEntryTable;
    }

    const std::vector<std::string_view> fields = SplitFields(line);
    if (fields.size() != 7U || fields[0] != "entry") {
        return PackageStatus::InvalidArtifactEntryTable;
    }

    std::uint32_t entry_value = 0U;
    std::uint32_t type_value = 0U;
    std::uint32_t byte_offset = 0U;
    std::uint32_t byte_size = 0U;
    if (!ParseUInt32(fields[1], &entry_value) ||
        !ParseUInt32(fields[2], &type_value) ||
        !ParseUInt32(fields[5], &byte_offset) ||
        !ParseUInt32(fields[6], &byte_size)) {
        return PackageStatus::ArtifactParseFailure;
    }

    *out_entry = PackageEntryDescriptor{
        package,
        PackageEntryId{entry_value},
        yuengine::resource::ResourceTypeId{type_value},
        yuengine::resource::ResourceLogicalKey(fields[3]),
        PackageSourceKey(fields[4]),
        byte_offset,
        byte_size};
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

    const std::size_t required_line_count =
        5U + static_cast<std::size_t>(entry_count) + static_cast<std::size_t>(dependency_count);
    if (lines.size() < required_line_count) {
        return PackageArtifactFailure(PackageStatus::ArtifactTruncated);
    }

    if (lines[required_line_count - 1U] != "end") {
        return PackageArtifactFailure(PackageStatus::ArtifactTruncated);
    }

    if (lines.size() > required_line_count) {
        return PackageArtifactFailure(PackageStatus::ArtifactUnknownSection);
    }

    std::array<PackageEntryDescriptor, MAX_PACKAGE_ENTRY_COUNT> entries{};
    std::array<PackageArtifactDependency, MAX_PACKAGE_DEPENDENCY_EDGE_COUNT> dependencies{};
    const PackageId package{package_value};
    std::size_t line_index = 4U;
    for (std::uint32_t index = 0U; index < entry_count; ++index) {
        parse_status = ParseEntryLine(lines[line_index], package, &entries[index]);
        if (parse_status != PackageStatus::Success) {
            return PackageArtifactFailure(parse_status);
        }

        ++line_index;
    }

    for (std::uint32_t index = 0U; index < dependency_count; ++index) {
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
