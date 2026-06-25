// Module: YuEngine ExternalAuthoring
// File: Src/YuEngine/ExternalAuthoring/Src/ExternalAuthoringBridge.cpp

#include "YuEngine/ExternalAuthoring/ExternalAuthoringBridge.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "YuEngine/File/FileReadRequest.h"
#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/MountTable.h"

namespace yuengine::externalauthoring {
namespace {
constexpr std::uint32_t MAX_MANIFEST_ENTRY_COUNT = 8U;
constexpr std::uint32_t MAX_MANIFEST_DEPENDENCY_COUNT = 8U;
constexpr std::uint32_t RESOURCE_TYPE_MESH = 101U;
constexpr std::uint32_t RESOURCE_TYPE_MATERIAL = 102U;
constexpr std::uint32_t RESOURCE_TYPE_TEXTURE = 103U;
constexpr std::uint32_t RESOURCE_TYPE_SHADER = 104U;
constexpr std::uint32_t RESOURCE_TYPE_SCENE = 105U;
constexpr std::uint32_t RESOURCE_TYPE_ANIMATION = 106U;
constexpr std::uint32_t ASSET_TYPE_MESH = 201U;
constexpr std::uint32_t ASSET_TYPE_MATERIAL = 202U;
constexpr std::uint32_t ASSET_TYPE_TEXTURE = 203U;
constexpr std::uint32_t ASSET_TYPE_SHADER = 204U;
constexpr std::uint32_t ASSET_TYPE_SCENE = 205U;
constexpr std::uint32_t ASSET_TYPE_ANIMATION = 206U;

struct ManifestEntry final {
    std::string target_kind;
    std::string payload_path;
    std::uint64_t stable_id = 0U;
    std::uint64_t content_hash = 0U;
    std::uint32_t dependency_count = 0U;
    std::array<std::uint64_t, MAX_MANIFEST_DEPENDENCY_COUNT> dependencies{};
};

struct ManifestDocument final {
    ExternalAuthoringToolKind tool = ExternalAuthoringToolKind::Unknown;
    std::string export_id;
    std::string unit_scale;
    std::string handedness;
    std::string up_axis;
    std::string transform_bake;
    std::string material_policy;
    std::string animation_policy;
    std::uint32_t unsupported_feature_count = 0U;
    std::uint32_t entry_count = 0U;
    std::array<ManifestEntry, MAX_MANIFEST_ENTRY_COUNT> entries{};
};

bool StartsWith(std::string_view text, std::string_view prefix) {
    if (text.size() < prefix.size()) {
        return false;
    }

    return text.substr(0U, prefix.size()) == prefix;
}

std::string_view StripLine(std::string_view line) {
    if (!line.empty() && line.back() == '\r') {
        line.remove_suffix(1U);
    }

    return line;
}

bool ParseUint32(std::string_view text, std::uint32_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    if (text.empty()) {
        return false;
    }

    std::uint32_t value = 0U;
    const char *first = text.data();
    const char *last = text.data() + text.size();
    const std::from_chars_result result = std::from_chars(first, last, value);
    if (result.ec != std::errc()) {
        return false;
    }

    if (result.ptr != last) {
        return false;
    }

    *out_value = value;
    return true;
}

bool ParseUint64(std::string_view text, std::uint64_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    if (text.empty()) {
        return false;
    }

    std::uint64_t value = 0U;
    const char *first = text.data();
    const char *last = text.data() + text.size();
    const std::from_chars_result result = std::from_chars(first, last, value);
    if (result.ec != std::errc()) {
        return false;
    }

    if (result.ptr != last) {
        return false;
    }

    *out_value = value;
    return true;
}

ExternalAuthoringToolKind ParseToolKind(std::string_view value) {
    if (value == "Unity") {
        return ExternalAuthoringToolKind::Unity;
    }

    if (value == "Unreal") {
        return ExternalAuthoringToolKind::Unreal;
    }

    if (value == "Dcc") {
        return ExternalAuthoringToolKind::Dcc;
    }

    return ExternalAuthoringToolKind::Unknown;
}

yuengine::runtimeasset::RuntimeAssetFileKind ParseRuntimeAssetKind(std::string_view value) {
    if (value == "Mesh") {
        return yuengine::runtimeasset::RuntimeAssetFileKind::Mesh;
    }

    if (value == "Material") {
        return yuengine::runtimeasset::RuntimeAssetFileKind::Material;
    }

    if (value == "Texture") {
        return yuengine::runtimeasset::RuntimeAssetFileKind::Texture;
    }

    if (value == "Shader") {
        return yuengine::runtimeasset::RuntimeAssetFileKind::Shader;
    }

    if (value == "Scene") {
        return yuengine::runtimeasset::RuntimeAssetFileKind::Scene;
    }

    if (value == "Animation") {
        return yuengine::runtimeasset::RuntimeAssetFileKind::Animation;
    }

    return yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
}

yuengine::resource::ResourceTypeId ResourceTypeForKind(
    yuengine::runtimeasset::RuntimeAssetFileKind kind) {
    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Mesh) {
        return yuengine::resource::ResourceTypeId{RESOURCE_TYPE_MESH};
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Material) {
        return yuengine::resource::ResourceTypeId{RESOURCE_TYPE_MATERIAL};
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Texture) {
        return yuengine::resource::ResourceTypeId{RESOURCE_TYPE_TEXTURE};
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Shader) {
        return yuengine::resource::ResourceTypeId{RESOURCE_TYPE_SHADER};
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Scene) {
        return yuengine::resource::ResourceTypeId{RESOURCE_TYPE_SCENE};
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Animation) {
        return yuengine::resource::ResourceTypeId{RESOURCE_TYPE_ANIMATION};
    }

    return yuengine::resource::ResourceTypeId{};
}

yuengine::asset::AssetTypeId AssetTypeForKind(yuengine::runtimeasset::RuntimeAssetFileKind kind) {
    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Mesh) {
        return yuengine::asset::AssetTypeId{ASSET_TYPE_MESH};
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Material) {
        return yuengine::asset::AssetTypeId{ASSET_TYPE_MATERIAL};
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Texture) {
        return yuengine::asset::AssetTypeId{ASSET_TYPE_TEXTURE};
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Shader) {
        return yuengine::asset::AssetTypeId{ASSET_TYPE_SHADER};
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Scene) {
        return yuengine::asset::AssetTypeId{ASSET_TYPE_SCENE};
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Animation) {
        return yuengine::asset::AssetTypeId{ASSET_TYPE_ANIMATION};
    }

    return yuengine::asset::AssetTypeId{};
}

yuengine::resource::ResourceDecodePlanAssetClass DecodePlanClassForKind(
    yuengine::runtimeasset::RuntimeAssetFileKind kind) {
    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Mesh) {
        return yuengine::resource::ResourceDecodePlanAssetClass::Mesh;
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Material) {
        return yuengine::resource::ResourceDecodePlanAssetClass::Material;
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Texture) {
        return yuengine::resource::ResourceDecodePlanAssetClass::Texture;
    }

    return yuengine::resource::ResourceDecodePlanAssetClass::Unknown;
}

yuengine::resource::ResourceDecodeResultClass DecodeResultClassForKind(
    yuengine::runtimeasset::RuntimeAssetFileKind kind) {
    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Mesh) {
        return yuengine::resource::ResourceDecodeResultClass::Mesh;
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Material) {
        return yuengine::resource::ResourceDecodeResultClass::Material;
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Texture) {
        return yuengine::resource::ResourceDecodeResultClass::Texture;
    }

    return yuengine::resource::ResourceDecodeResultClass::Unknown;
}

std::uint32_t DecodedByteCountForKind(yuengine::runtimeasset::RuntimeAssetFileKind kind) {
    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Mesh) {
        return 96U;
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Material) {
        return 128U;
    }

    if (kind == yuengine::runtimeasset::RuntimeAssetFileKind::Texture) {
        return 16U;
    }

    return 0U;
}

template <std::size_t Size>
bool CopyTextToBuffer(std::string_view text, char (&buffer)[Size]) {
    if (text.size() >= Size) {
        return false;
    }

    std::fill(std::begin(buffer), std::end(buffer), '\0');
    std::copy(text.begin(), text.end(), buffer);
    return true;
}

bool ApplyEntryField(std::string_view key, std::string_view value, ManifestDocument *document) {
    if (document == nullptr) {
        return false;
    }

    if (!StartsWith(key, "entry.")) {
        return false;
    }

    std::string_view rest = key.substr(6U);
    const std::size_t separator = rest.find('.');
    if (separator == std::string_view::npos) {
        return false;
    }

    std::uint32_t entry_index = 0U;
    if (!ParseUint32(rest.substr(0U, separator), &entry_index)) {
        return false;
    }

    if (entry_index >= MAX_MANIFEST_ENTRY_COUNT) {
        return false;
    }

    ManifestEntry &entry = document->entries[entry_index];
    const std::string_view field = rest.substr(separator + 1U);
    if (field == "kind") {
        entry.target_kind = std::string(value);
        return true;
    }

    if (field == "payload") {
        entry.payload_path = std::string(value);
        return true;
    }

    if (field == "stable_id") {
        return ParseUint64(value, &entry.stable_id);
    }

    if (field == "content_hash") {
        return ParseUint64(value, &entry.content_hash);
    }

    if (field == "dependency_count") {
        return ParseUint32(value, &entry.dependency_count);
    }

    if (StartsWith(field, "dependency.")) {
        std::uint32_t dependency_index = 0U;
        if (!ParseUint32(field.substr(11U), &dependency_index)) {
            return false;
        }

        if (dependency_index >= MAX_MANIFEST_DEPENDENCY_COUNT) {
            return false;
        }

        return ParseUint64(value, &entry.dependencies[dependency_index]);
    }

    return false;
}

bool ApplyTopLevelField(std::string_view key, std::string_view value, ManifestDocument *document) {
    if (document == nullptr) {
        return false;
    }

    if (key == "tool") {
        document->tool = ParseToolKind(value);
        return true;
    }

    if (key == "export_id") {
        document->export_id = std::string(value);
        return true;
    }

    if (key == "unit_scale") {
        document->unit_scale = std::string(value);
        return true;
    }

    if (key == "handedness") {
        document->handedness = std::string(value);
        return true;
    }

    if (key == "up_axis") {
        document->up_axis = std::string(value);
        return true;
    }

    if (key == "transform_bake") {
        document->transform_bake = std::string(value);
        return true;
    }

    if (key == "material_policy") {
        document->material_policy = std::string(value);
        return true;
    }

    if (key == "animation_policy") {
        document->animation_policy = std::string(value);
        return true;
    }

    if (key == "unsupported_feature_count") {
        return ParseUint32(value, &document->unsupported_feature_count);
    }

    if (key == "entry_count") {
        return ParseUint32(value, &document->entry_count);
    }

    return ApplyEntryField(key, value, document);
}

bool FinalizeManifestDocument(const ManifestDocument &document) {
    if (document.entry_count == 0U) {
        return false;
    }

    if (document.entry_count > MAX_MANIFEST_ENTRY_COUNT) {
        return false;
    }

    std::uint32_t entry_index = 0U;
    while (entry_index < document.entry_count) {
        const ManifestEntry &entry = document.entries[entry_index];
        if (entry.stable_id == 0U) {
            return false;
        }

        if (entry.payload_path.empty()) {
            return false;
        }

        if (ParseRuntimeAssetKind(entry.target_kind) ==
            yuengine::runtimeasset::RuntimeAssetFileKind::Unknown) {
            return false;
        }

        if (entry.dependency_count > MAX_MANIFEST_DEPENDENCY_COUNT) {
            return false;
        }

        ++entry_index;
    }

    return true;
}

bool ParseManifestBytes(std::span<const std::uint8_t> bytes, ManifestDocument *out_document) {
    if (out_document == nullptr) {
        return false;
    }

    if (bytes.empty()) {
        return false;
    }

    ManifestDocument document{};
    const std::string text(bytes.begin(), bytes.end());
    std::size_t cursor = 0U;
    bool read_header = false;
    while (cursor <= text.size()) {
        std::size_t end = text.find('\n', cursor);
        if (end == std::string::npos) {
            end = text.size();
        }

        const std::string_view line = StripLine(std::string_view(text.data() + cursor, end - cursor));
        cursor = end + 1U;
        if (line.empty()) {
            if (end == text.size()) {
                break;
            }

            continue;
        }

        if (!read_header) {
            if (line != "YuExternalAuthoringExport v1") {
                return false;
            }

            read_header = true;
            if (end == text.size()) {
                break;
            }

            continue;
        }

        const std::size_t separator = line.find('=');
        if (separator == std::string_view::npos) {
            return false;
        }

        const std::string_view key = line.substr(0U, separator);
        const std::string_view value = line.substr(separator + 1U);
        if (!ApplyTopLevelField(key, value, &document)) {
            return false;
        }

        if (end == text.size()) {
            break;
        }
    }

    if (!read_header) {
        return false;
    }

    if (!FinalizeManifestDocument(document)) {
        return false;
    }

    *out_document = document;
    return true;
}

bool SupportsCoordinatePolicy(const ManifestDocument &document) {
    if (document.unit_scale != "1") {
        return false;
    }

    if (document.handedness != "right") {
        return false;
    }

    if (document.up_axis != "y") {
        return false;
    }

    return document.transform_bake == "world";
}

bool SupportsMaterialPolicy(const ManifestDocument &document) {
    return document.material_policy == "yu_material_v0";
}

bool SupportsAnimationPolicy(const ManifestDocument &document) {
    return document.animation_policy == "sampled_clip_v0";
}

bool ContainsStableId(const ManifestDocument &document, std::uint64_t stable_id) {
    std::uint32_t entry_index = 0U;
    while (entry_index < document.entry_count) {
        if (document.entries[entry_index].stable_id == stable_id) {
            return true;
        }

        ++entry_index;
    }

    return false;
}

bool ValidateDependencyGraph(const ManifestDocument &document, std::uint32_t *out_dependency_count) {
    if (out_dependency_count == nullptr) {
        return false;
    }

    std::uint32_t total_dependency_count = 0U;
    std::uint32_t entry_index = 0U;
    while (entry_index < document.entry_count) {
        const ManifestEntry &entry = document.entries[entry_index];
        std::uint32_t dependency_index = 0U;
        while (dependency_index < entry.dependency_count) {
            const std::uint64_t dependency_stable_id = entry.dependencies[dependency_index];
            if (dependency_stable_id == 0U) {
                return false;
            }

            if (!ContainsStableId(document, dependency_stable_id)) {
                return false;
            }

            ++total_dependency_count;
            ++dependency_index;
        }

        ++entry_index;
    }

    *out_dependency_count = total_dependency_count;
    return true;
}

ExternalAuthoringBridgeStatus CommitResult(
    const ExternalAuthoringBridgeResult &result,
    ExternalAuthoringBridgeResult *out_result) {
    if (out_result != nullptr) {
        *out_result = result;
    }

    return result.status;
}

bool ConfigureRuntimeAssetRow(
    const ManifestDocument &document,
    const ManifestEntry &entry,
    std::string_view manifest_path,
    std::uint64_t payload_hash,
    ExternalAuthoringRuntimeAssetInputRow *out_row) {
    if (out_row == nullptr) {
        return false;
    }

    ExternalAuthoringRuntimeAssetInputRow row{};
    if (!CopyTextToBuffer(manifest_path, row.manifest_path)) {
        return false;
    }

    if (!CopyTextToBuffer(entry.payload_path, row.payload_path)) {
        return false;
    }

    const yuengine::runtimeasset::RuntimeAssetFileKind kind =
        ParseRuntimeAssetKind(entry.target_kind);
    row.tool = document.tool;
    row.target_kind = kind;
    row.resource_type = ResourceTypeForKind(kind);
    row.asset_type = AssetTypeForKind(kind);
    row.stable_id = entry.stable_id;
    row.content_hash = payload_hash;
    row.dependency_count = entry.dependency_count;
    row.unsupported_feature_count = document.unsupported_feature_count;
    row.manifest_readable = true;
    row.payload_available = true;
    row.dependencies_valid = true;
    row.coordinate_policy_supported = true;
    row.material_policy_supported = true;
    row.animation_policy_supported = true;
    row.runtime_asset_descriptor_ready = true;
    row.manifest_ready = true;
    row.preview_supported = true;
    row.selected = true;
    row.runtime_asset_source.path = row.payload_path;
    row.runtime_asset_source.kind = kind;
    row.runtime_asset_source.resource_type = row.resource_type;
    row.runtime_asset_source.asset_type = row.asset_type;
    row.runtime_asset_source.stable_id = row.stable_id;
    row.runtime_asset_source.decode_asset_class = DecodePlanClassForKind(kind);
    row.runtime_asset_source.decode_result_class = DecodeResultClassForKind(kind);
    row.runtime_asset_source.decoded_byte_count = DecodedByteCountForKind(kind);
    *out_row = row;
    return true;
}

bool HasImportCookBuffers(const ExternalAuthoringBridgeRequest &request) {
    if (request.import_cook_request == nullptr) {
        return false;
    }

    if (request.import_cook_source_files == nullptr) {
        return false;
    }

    if (request.import_cook_cooked_files == nullptr) {
        return false;
    }

    if (request.import_cook_source_file_capacity <
        yuengine::runtimeasset::RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT) {
        return false;
    }

    return request.import_cook_cooked_file_capacity >=
        yuengine::runtimeasset::RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT;
}

yuengine::runtimeasset::RuntimeAssetImportCookCommandRequest BuildImportCookRequest(
    const ExternalAuthoringBridgeRequest &request) {
    yuengine::runtimeasset::RuntimeAssetImportCookCommandRequest command{};
    command.command =
        yuengine::runtimeasset::RuntimeAssetImportCookCommandKind::GenerateDeterministicDiskFixture;
    command.fixture.mount_table = request.mount_table;
    command.fixture.mount = request.mount;
    command.fixture.source_files = request.import_cook_source_files;
    command.fixture.source_file_capacity = request.import_cook_source_file_capacity;
    command.fixture.cooked_files = request.import_cook_cooked_files;
    command.fixture.cooked_file_capacity = request.import_cook_cooked_file_capacity;
    return command;
}
}

ExternalAuthoringBridgeStatus BuildExternalAuthoringRuntimeAssetImportBridge(
    const ExternalAuthoringBridgeRequest &request,
    ExternalAuthoringBridgeResult *out_result) {
    ExternalAuthoringBridgeResult result{};
    if (request.mount_table == nullptr) {
        result.status = ExternalAuthoringBridgeStatus::InvalidArgument;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::FileVfs;
        return CommitResult(result, out_result);
    }

    if (request.runtime_asset_inputs.empty()) {
        result.status = ExternalAuthoringBridgeStatus::OutputCapacityExceeded;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::RuntimeAssetInput;
        return CommitResult(result, out_result);
    }

    if (!HasImportCookBuffers(request)) {
        result.status = ExternalAuthoringBridgeStatus::ImportCookInputMissing;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::ImportCookCommand;
        return CommitResult(result, out_result);
    }

    const std::string manifest_path(request.manifest_path.Value());
    const yuengine::file::FileReadResult manifest_read =
        request.mount_table->Read({request.mount, request.manifest_path});
    result.file_status = manifest_read.status;
    if (!manifest_read.Succeeded()) {
        result.status = ExternalAuthoringBridgeStatus::FileReadFailed;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::FileVfs;
        return CommitResult(result, out_result);
    }

    result.manifest_read = true;
    ManifestDocument document{};
    if (!ParseManifestBytes(manifest_read.bytes, &document)) {
        result.status = ExternalAuthoringBridgeStatus::ManifestParseFailed;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::Manifest;
        return CommitResult(result, out_result);
    }

    result.parsed_manifest = true;
    result.tool = document.tool;
    result.manifest_entry_count = document.entry_count;
    result.unsupported_feature_count = document.unsupported_feature_count;
    if (document.tool == ExternalAuthoringToolKind::Unknown) {
        result.status = ExternalAuthoringBridgeStatus::UnsupportedTool;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::MappingPolicy;
        return CommitResult(result, out_result);
    }

    if (!SupportsCoordinatePolicy(document)) {
        result.status = ExternalAuthoringBridgeStatus::UnsupportedCoordinatePolicy;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::MappingPolicy;
        return CommitResult(result, out_result);
    }

    result.validated_coordinate_policy = true;
    if (!SupportsMaterialPolicy(document)) {
        result.status = ExternalAuthoringBridgeStatus::UnsupportedMaterialPolicy;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::MappingPolicy;
        return CommitResult(result, out_result);
    }

    result.validated_material_policy = true;
    if (!SupportsAnimationPolicy(document)) {
        result.status = ExternalAuthoringBridgeStatus::UnsupportedAnimationPolicy;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::MappingPolicy;
        return CommitResult(result, out_result);
    }

    result.validated_animation_policy = true;
    if (document.unsupported_feature_count != 0U) {
        result.status = ExternalAuthoringBridgeStatus::UnsupportedFeature;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::MappingPolicy;
        return CommitResult(result, out_result);
    }

    std::uint32_t dependency_count = 0U;
    if (!ValidateDependencyGraph(document, &dependency_count)) {
        result.status = ExternalAuthoringBridgeStatus::InvalidDependencyGraph;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::DependencyGraph;
        return CommitResult(result, out_result);
    }

    result.validated_dependency_graph = true;
    result.dependency_count = dependency_count;
    if (request.runtime_asset_inputs.size() < document.entry_count) {
        result.status = ExternalAuthoringBridgeStatus::OutputCapacityExceeded;
        result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::RuntimeAssetInput;
        return CommitResult(result, out_result);
    }

    std::array<ExternalAuthoringRuntimeAssetInputRow, MAX_MANIFEST_ENTRY_COUNT> staged_rows{};
    std::uint32_t entry_index = 0U;
    while (entry_index < document.entry_count) {
        const ManifestEntry &entry = document.entries[entry_index];
        const yuengine::file::FileReadResult payload_read =
            request.mount_table->Read({request.mount, yuengine::file::VirtualPath(entry.payload_path)});
        result.file_status = payload_read.status;
        if (!payload_read.Succeeded()) {
            result.status = ExternalAuthoringBridgeStatus::MissingPayload;
            result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::Payload;
            return CommitResult(result, out_result);
        }

        const std::uint64_t payload_hash =
            yuengine::runtimeasset::HashRuntimeAssetDataBytes(payload_read.bytes);
        if (entry.content_hash != 0U && entry.content_hash != payload_hash) {
            result.status = ExternalAuthoringBridgeStatus::PayloadHashMismatch;
            result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::Payload;
            return CommitResult(result, out_result);
        }

        if (!ConfigureRuntimeAssetRow(
                document,
                entry,
                manifest_path,
                payload_hash,
                &staged_rows[entry_index])) {
            result.status = ExternalAuthoringBridgeStatus::OutputCapacityExceeded;
            result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::RuntimeAssetInput;
            return CommitResult(result, out_result);
        }

        ++result.payload_read_count;
        ++entry_index;
    }

    const yuengine::runtimeasset::RuntimeAssetImportCookCommandRequest staged_command =
        BuildImportCookRequest(request);
    entry_index = 0U;
    while (entry_index < document.entry_count) {
        request.runtime_asset_inputs[entry_index] = staged_rows[entry_index];
        request.runtime_asset_inputs[entry_index].runtime_asset_source.path =
            request.runtime_asset_inputs[entry_index].payload_path;
        ++entry_index;
    }

    *request.import_cook_request = staged_command;
    result.status = ExternalAuthoringBridgeStatus::Success;
    result.blocked_layer = ExternalAuthoringBridgeBlockedLayer::None;
    result.runtime_asset_input_count = document.entry_count;
    result.emitted_runtime_asset_input = true;
    result.emitted_import_cook_request = true;
    return CommitResult(result, out_result);
}

}
