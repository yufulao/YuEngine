// 模块: YuEngine RuntimeAsset
// 文件: Src/YuEngine/RuntimeAsset/Src/RuntimeAssetData.cpp

#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"

#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "YuEngine/Asset/AssetDescriptor.h"
#include "YuEngine/Asset/AssetManager.h"
#include "YuEngine/File/FileReadRequest.h"
#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/Resource/ResourceCachePayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceCachePayloadRequest.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodedPayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRequest.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Resource/ResourceDecodePlanBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodePlanRequest.h"
#include "YuEngine/Resource/ResourceDecodePlanStatus.h"
#include "YuEngine/Resource/ResourceDecodeResultBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodeResultRequest.h"
#include "YuEngine/Resource/ResourceDecodeResultStatus.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceLoadCommitRequest.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Resource/ResourceResidencyBudgetDesc.h"
#include "YuEngine/Resource/ResourceResidencyRequest.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"

namespace yuengine::runtimeasset {
namespace {
using yuengine::asset::AssetDescriptor;
using yuengine::asset::AssetManager;
using yuengine::asset::AssetRegistrationResult;
using yuengine::asset::AssetStatus;
using yuengine::file::FileReadRequest;
using yuengine::file::FileReadResult;
using yuengine::file::MountTable;
using yuengine::resource::ResourceCachePayloadRequest;
using yuengine::resource::ResourceCachePayloadStatus;
using yuengine::resource::ResourceDecodedPayloadRequest;
using yuengine::resource::ResourceDecodedPayloadStatus;
using yuengine::resource::ResourceDecodePlanAssetClass;
using yuengine::resource::ResourceDecodePlanRequest;
using yuengine::resource::ResourceDecodePlanStatus;
using yuengine::resource::ResourceDecodeResultClass;
using yuengine::resource::ResourceDecodeResultRequest;
using yuengine::resource::ResourceDecodeResultStatus;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceLoadCommitRequest;
using yuengine::resource::ResourceLoadCommitStatus;
using yuengine::resource::ResourceLoadState;
using yuengine::resource::ResourceLogicalKey;
using yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceRegistrationResult;
using yuengine::resource::ResourceResidencyRequest;
using yuengine::resource::ResourceResidencyStatus;
using yuengine::resource::ResourceStatus;

constexpr std::uint64_t FNV_OFFSET = 14695981039346656037ULL;
constexpr std::uint64_t FNV_PRIME = 1099511628211ULL;
constexpr std::uint64_t COMMIT_ID_OFFSET = 10000U;
constexpr std::uint64_t UPLOAD_ID_OFFSET = 20000U;
constexpr std::uint64_t STAGING_ID_OFFSET = 30000U;
constexpr std::uint64_t SOURCE_PAYLOAD_ID_OFFSET = 40000U;
constexpr std::uint64_t PLAN_PAYLOAD_ID_OFFSET = 80000U;
constexpr std::uint64_t DECODE_PLAN_ID_OFFSET = 90000U;
constexpr std::uint64_t DECODE_RESULT_ID_OFFSET = 100000U;
constexpr std::uint64_t DECODED_PAYLOAD_ID_OFFSET = 110000U;
constexpr std::uint32_t DEFAULT_RESIDENCY_BYTE_CAPACITY = 4096U;
constexpr std::uint32_t DEFAULT_PAYLOAD_BYTE_CAPACITY = 4096U;

bool Contains(std::string_view text, std::string_view token) {
    return text.find(token) != std::string_view::npos;
}

bool StartsWith(std::string_view text, std::string_view prefix) {
    return text.size() >= prefix.size() && text.substr(0U, prefix.size()) == prefix;
}

std::size_t CountToken(std::string_view text, std::string_view token) {
    std::size_t count = 0U;
    std::size_t offset = 0U;
    while (offset < text.size()) {
        const std::size_t found = text.find(token, offset);
        if (found == std::string_view::npos) {
            return count;
        }

        ++count;
        offset = found + token.size();
    }

    return count;
}

std::string_view ValueForToken(std::string_view text, std::string_view token) {
    const std::size_t found = text.find(token);
    if (found == std::string_view::npos) {
        return {};
    }

    const std::size_t value_offset = found + token.size();
    const std::size_t line_end = text.find('\n', value_offset);
    if (line_end == std::string_view::npos) {
        return text.substr(value_offset);
    }

    return text.substr(value_offset, line_end - value_offset);
}

bool HasSupportedHeader(std::string_view text, RuntimeAssetFileKind expected_kind) {
    const std::string header =
        "YUASSET " + std::string(RuntimeAssetFileKindName(expected_kind)) + " 1";
    return Contains(text, header);
}

bool HasUnsupportedHeader(std::string_view text, RuntimeAssetFileKind expected_kind) {
    const std::string header =
        "YUASSET " + std::string(RuntimeAssetFileKindName(expected_kind)) + " 2";
    return Contains(text, header);
}

struct DependencyRule final {
    std::string_view token;
    std::string_view expected_prefix;
};

RuntimeAssetDataStatus ValidateDependencyRules(
    std::string_view text,
    std::span<const DependencyRule> rules,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    for (const DependencyRule &rule : rules) {
        const std::size_t count = CountToken(text, rule.token);
        if (count == 0U) {
            return RuntimeAssetDataStatus::MissingDependency;
        }

        if (count > 1U) {
            return RuntimeAssetDataStatus::DuplicateDependency;
        }

        if (!rule.expected_prefix.empty()) {
            const std::string_view value = ValueForToken(text, rule.token);
            if (!StartsWith(value, rule.expected_prefix)) {
                return RuntimeAssetDataStatus::TypeMismatch;
            }
        }

        ++out_result->dependency_count;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateSceneDependencies(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    constexpr std::array<DependencyRule, 8U> rules{{
        {"m0=", "Mesh/"},
        {"m1=", "Mesh/"},
        {"m2=", "Mesh/"},
        {"mat=", "Material/"},
        {"t0=", "Texture/"},
        {"prog=", "Shader/"},
        {"cam=", "camera:"},
        {"anim=", "Animation/"},
    }};

    return ValidateDependencyRules(text, std::span<const DependencyRule>(rules.data(), rules.size()), out_result);
}

RuntimeAssetDataStatus ValidateShaderProgramDependencies(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    constexpr std::array<DependencyRule, 4U> rules{{
        {"stage_vs=", "bytecode:"},
        {"stage_ps=", "bytecode:"},
        {"input=", "layout:"},
        {"textures=", "3"},
    }};

    return ValidateDependencyRules(text, std::span<const DependencyRule>(rules.data(), rules.size()), out_result);
}

RuntimeAssetDataStatus ValidateAnimationDependencies(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    constexpr std::array<DependencyRule, 2U> rules{{
        {"target=", "scene_entity:"},
        {"track=", "transform:"},
    }};

    return ValidateDependencyRules(text, std::span<const DependencyRule>(rules.data(), rules.size()), out_result);
}

bool SceneReferencesRuntimeFamilies(std::string_view scene_text) {
    if (!Contains(scene_text, "m0=Mesh/Cube.yumesh")) {
        return false;
    }

    if (!Contains(scene_text, "m1=Mesh/Cylinder.yumesh")) {
        return false;
    }

    if (!Contains(scene_text, "m2=Mesh/Cone.yumesh")) {
        return false;
    }

    if (!Contains(scene_text, "mat=Material/Shared.yumat")) {
        return false;
    }

    if (!Contains(scene_text, "t0=Texture/Albedo.yutex")) {
        return false;
    }

    if (!Contains(scene_text, "prog=Shader/RuntimeProgram.yuprogram")) {
        return false;
    }

    if (!Contains(scene_text, "cam=camera:orbit")) {
        return false;
    }

    return Contains(scene_text, "anim=Animation/Spin.yuanim");
}

bool ConfigureRuntimeAssetResourceBudgets(ResourceRegistry &registry) {
    yuengine::resource::ResourceResidencyBudgetDesc residency_budget{};
    residency_budget.byte_capacity = DEFAULT_RESIDENCY_BYTE_CAPACITY;
    if (registry.SetResidencyBudget(residency_budget) != ResourceResidencyStatus::Success) {
        return false;
    }

    yuengine::resource::ResourceCachePayloadBudgetDesc cache_budget{};
    cache_budget.byte_capacity = DEFAULT_PAYLOAD_BYTE_CAPACITY;
    if (registry.SetCachePayloadBudget(cache_budget) != ResourceCachePayloadStatus::Success) {
        return false;
    }

    yuengine::resource::ResourceDecodePlanBudgetDesc plan_budget{};
    if (registry.SetDecodePlanBudget(plan_budget) != ResourceDecodePlanStatus::Success) {
        return false;
    }

    yuengine::resource::ResourceDecodeResultBudgetDesc result_budget{};
    if (registry.SetDecodeResultBudget(result_budget) != ResourceDecodeResultStatus::Success) {
        return false;
    }

    yuengine::resource::ResourceDecodedPayloadBudgetDesc decoded_budget{};
    return registry.SetDecodedPayloadBudget(decoded_budget) == ResourceDecodedPayloadStatus::Success;
}

RuntimeAssetDataStatus ReadRuntimeAssetFile(
    const RuntimeAssetGraphLoadRequest &request,
    const yuengine::file::VirtualPath &path,
    std::vector<std::uint8_t> *out_bytes) {
    if (out_bytes == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    FileReadRequest read_request{};
    read_request.mount = request.mount;
    read_request.path = path;
    const FileReadResult read_result = request.mount_table->Read(read_request);
    if (!read_result.Succeeded()) {
        return RuntimeAssetDataStatus::FileReadFailed;
    }

    *out_bytes = read_result.bytes;
    return RuntimeAssetDataStatus::Success;
}

void WriteU32LittleEndian(std::array<std::uint8_t, yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT> &bytes,
    std::uint32_t offset,
    std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

std::array<std::uint8_t, yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT> DecodePlanPayload(
    ResourceDecodePlanAssetClass asset_class,
    std::uint32_t decoded_byte_count) {
    std::array<std::uint8_t, yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT> payload{};
    payload[0U] = yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_0;
    payload[1U] = yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_1;
    payload[2U] = yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_2;
    payload[3U] = yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_3;
    WriteU32LittleEndian(payload, 4U, yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_VERSION);
    WriteU32LittleEndian(payload, 8U, static_cast<std::uint32_t>(asset_class));
    WriteU32LittleEndian(payload, 12U, static_cast<std::uint32_t>(payload.size()));
    WriteU32LittleEndian(payload, 16U, decoded_byte_count);
    return payload;
}

std::vector<std::uint8_t> BuildDecodedBytes(
    std::span<const std::uint8_t> source_bytes,
    std::uint32_t decoded_byte_count) {
    std::vector<std::uint8_t> decoded_bytes{};
    decoded_bytes.resize(decoded_byte_count);
    std::uint32_t index = 0U;
    while (index < decoded_byte_count) {
        const std::size_t source_index = static_cast<std::size_t>(index) % source_bytes.size();
        const std::uint8_t source_byte = source_bytes[source_index];
        decoded_bytes[index] = static_cast<std::uint8_t>(source_byte ^ static_cast<std::uint8_t>(index));
        ++index;
    }

    return decoded_bytes;
}

RuntimeAssetDataStatus StoreSourcePayload(
    ResourceRegistry &registry,
    ResourceHandle resource,
    const RuntimeAssetFileDesc &desc,
    std::span<const std::uint8_t> bytes,
    RuntimeAssetLoadedFile *out_record) {
    ResourceCachePayloadRequest payload_request{};
    payload_request.resource = resource;
    payload_request.expected_type = desc.resource_type;
    payload_request.payload_id = desc.stable_id + SOURCE_PAYLOAD_ID_OFFSET;
    payload_request.payload_bytes = bytes.data();
    payload_request.payload_byte_count = static_cast<std::uint32_t>(bytes.size());
    if (registry.StoreCachePayload(payload_request) != ResourceCachePayloadStatus::Success) {
        return RuntimeAssetDataStatus::CachePayloadStoreFailed;
    }

    out_record->cache_payload_id = payload_request.payload_id;
    out_record->cache_payload_stored = true;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus StoreDecodedPayload(
    ResourceRegistry &registry,
    ResourceHandle resource,
    const RuntimeAssetFileDesc &desc,
    std::span<const std::uint8_t> source_bytes,
    RuntimeAssetLoadedFile *out_record) {
    if (desc.decode_asset_class == ResourceDecodePlanAssetClass::Unknown) {
        return RuntimeAssetDataStatus::Success;
    }

    if (desc.decode_result_class == ResourceDecodeResultClass::Unknown) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (desc.decoded_byte_count == 0U) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const std::uint64_t plan_payload_id = desc.stable_id + PLAN_PAYLOAD_ID_OFFSET;
    const std::array<std::uint8_t, yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT> plan_payload =
        DecodePlanPayload(desc.decode_asset_class, desc.decoded_byte_count);

    ResourceCachePayloadRequest plan_payload_request{};
    plan_payload_request.resource = resource;
    plan_payload_request.expected_type = desc.resource_type;
    plan_payload_request.payload_id = plan_payload_id;
    plan_payload_request.payload_bytes = plan_payload.data();
    plan_payload_request.payload_byte_count = static_cast<std::uint32_t>(plan_payload.size());
    if (registry.StoreCachePayload(plan_payload_request) != ResourceCachePayloadStatus::Success) {
        return RuntimeAssetDataStatus::CachePayloadStoreFailed;
    }

    ResourceDecodePlanRequest plan_request{};
    plan_request.resource = resource;
    plan_request.expected_type = desc.resource_type;
    plan_request.payload_id = plan_payload_id;
    plan_request.decode_plan_id = desc.stable_id + DECODE_PLAN_ID_OFFSET;
    plan_request.asset_class = desc.decode_asset_class;
    plan_request.source_byte_count = static_cast<std::uint32_t>(plan_payload.size());
    plan_request.expected_decoded_byte_count = desc.decoded_byte_count;
    if (registry.CreateDecodePlan(plan_request) != ResourceDecodePlanStatus::Success) {
        return RuntimeAssetDataStatus::DecodePlanFailed;
    }

    ResourceDecodeResultRequest result_request{};
    result_request.resource = resource;
    result_request.expected_type = desc.resource_type;
    result_request.payload_id = plan_payload_id;
    result_request.decode_plan_id = plan_request.decode_plan_id;
    result_request.decode_result_id = desc.stable_id + DECODE_RESULT_ID_OFFSET;
    result_request.asset_class = desc.decode_asset_class;
    result_request.result_class = desc.decode_result_class;
    result_request.decoded_byte_count = desc.decoded_byte_count;
    if (registry.CommitDecodeResult(result_request) != ResourceDecodeResultStatus::Success) {
        return RuntimeAssetDataStatus::DecodeResultFailed;
    }

    const std::vector<std::uint8_t> decoded_bytes = BuildDecodedBytes(source_bytes, desc.decoded_byte_count);
    ResourceDecodedPayloadRequest decoded_request{};
    decoded_request.resource = resource;
    decoded_request.expected_type = desc.resource_type;
    decoded_request.payload_id = plan_payload_id;
    decoded_request.decode_plan_id = plan_request.decode_plan_id;
    decoded_request.decode_result_id = result_request.decode_result_id;
    decoded_request.decoded_payload_id = desc.stable_id + DECODED_PAYLOAD_ID_OFFSET;
    decoded_request.asset_class = desc.decode_asset_class;
    decoded_request.result_class = desc.decode_result_class;
    decoded_request.decoded_bytes = decoded_bytes.data();
    decoded_request.decoded_byte_count = static_cast<std::uint32_t>(decoded_bytes.size());
    if (registry.StoreDecodedPayload(decoded_request) != ResourceDecodedPayloadStatus::Success) {
        return RuntimeAssetDataStatus::DecodedPayloadStoreFailed;
    }

    out_record->decode_plan_id = plan_request.decode_plan_id;
    out_record->decode_result_id = result_request.decode_result_id;
    out_record->decoded_payload_id = decoded_request.decoded_payload_id;
    out_record->decode_plan_created = true;
    out_record->decode_result_committed = true;
    out_record->decoded_payload_stored = true;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus RegisterLoadedFile(
    ResourceRegistry &registry,
    AssetManager &manager,
    const RuntimeAssetFileDesc &desc,
    std::span<const std::uint8_t> bytes,
    RuntimeAssetLoadedFile *out_record) {
    if (out_record == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (desc.path == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetLoadedFile record{};
    record.kind = desc.kind;
    record.stable_id = desc.stable_id;
    record.hash = HashRuntimeAssetDataBytes(bytes);
    record.byte_count = static_cast<std::uint32_t>(bytes.size());

    ResourceDescriptor resource_descriptor{};
    resource_descriptor.type = desc.resource_type;
    const std::string logical_key = "radc." + std::to_string(desc.stable_id);
    resource_descriptor.logical_key = ResourceLogicalKey(logical_key);
    const ResourceRegistrationResult resource_result =
        registry.RegisterSyntheticDescriptor(resource_descriptor);
    if (!resource_result.Succeeded()) {
        return RuntimeAssetDataStatus::ResourceRegistrationFailed;
    }

    ResourceLoadCommitRequest commit_request{};
    commit_request.resource = resource_result.handle;
    commit_request.expected_type = desc.resource_type;
    commit_request.load_state = ResourceLoadState::Uploaded;
    commit_request.commit_id = desc.stable_id + COMMIT_ID_OFFSET;
    commit_request.upload_id = desc.stable_id + UPLOAD_ID_OFFSET;
    commit_request.staging_request_id = desc.stable_id + STAGING_ID_OFFSET;
    commit_request.upload_byte_count = static_cast<std::uint32_t>(bytes.size());
    if (registry.CommitUploadCompletion(commit_request) != ResourceLoadCommitStatus::Success) {
        return RuntimeAssetDataStatus::ResourceLoadCommitFailed;
    }

    ResourceResidencyRequest residency_request{};
    residency_request.resource = resource_result.handle;
    residency_request.expected_type = desc.resource_type;
    if (registry.AdmitResident(residency_request) != ResourceResidencyStatus::Success) {
        return RuntimeAssetDataStatus::ResourceResidencyFailed;
    }

    RuntimeAssetDataStatus status = StoreSourcePayload(
        registry,
        resource_result.handle,
        desc,
        bytes,
        &record);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = StoreDecodedPayload(registry, resource_result.handle, desc, bytes, &record);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    AssetDescriptor asset_descriptor{};
    asset_descriptor.stable_id = desc.stable_id;
    asset_descriptor.asset_type = desc.asset_type;
    asset_descriptor.resource = resource_result.handle;
    asset_descriptor.resource_type = desc.resource_type;
    const AssetRegistrationResult asset_result = manager.RegisterRuntimeAsset(&registry, asset_descriptor);
    if (!asset_result.Succeeded()) {
        return RuntimeAssetDataStatus::AssetRegistrationFailed;
    }

    record.resource = resource_result.handle;
    record.asset = asset_result.handle;
    *out_record = record;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus AddLoadedDependency(
    ResourceRegistry &resource_registry,
    AssetManager &asset_manager,
    const RuntimeAssetLoadedFile &scene,
    const RuntimeAssetLoadedFile &dependency) {
    const ResourceStatus resource_status = resource_registry.AddDependency(scene.resource, dependency.resource);
    if (resource_status != ResourceStatus::Success) {
        return RuntimeAssetDataStatus::ResourceDependencyFailed;
    }

    const AssetStatus asset_status = asset_manager.AddDependency(scene.asset, dependency.asset);
    if (asset_status != AssetStatus::Success) {
        return RuntimeAssetDataStatus::AssetDependencyFailed;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateGraphRequest(const RuntimeAssetGraphLoadRequest &request) {
    if (request.mount_table == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.resource_registry == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.asset_manager == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.files == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.loaded_files == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.file_count == 0U) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.file_count > request.loaded_file_capacity) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    if (request.scene_stable_id == 0U) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (!request.scene_resource_type.IsValid()) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (!request.scene_asset_type.IsValid()) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    return RuntimeAssetDataStatus::Success;
}
}

const char *RuntimeAssetFileKindName(RuntimeAssetFileKind kind) {
    switch (kind) {
        case RuntimeAssetFileKind::Mesh:
            return "MESH";
        case RuntimeAssetFileKind::Material:
            return "MATERIAL";
        case RuntimeAssetFileKind::Texture:
            return "TEXTURE";
        case RuntimeAssetFileKind::Shader:
            return "SHADER";
        case RuntimeAssetFileKind::Scene:
            return "SCENE";
        case RuntimeAssetFileKind::Animation:
            return "ANIMATION";
        case RuntimeAssetFileKind::Unknown:
        default:
            break;
    }

    return "UNKNOWN";
}

std::uint64_t HashRuntimeAssetDataBytes(std::span<const std::uint8_t> bytes) {
    std::uint64_t hash = FNV_OFFSET;
    for (const std::uint8_t byte : bytes) {
        hash ^= static_cast<std::uint64_t>(byte);
        hash *= FNV_PRIME;
    }

    return hash;
}

RuntimeAssetDataStatus ValidateRuntimeAssetDataBytes(
    std::span<const std::uint8_t> bytes,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetValidationResult result{};
    result.kind = expected_kind;
    result.byte_count = bytes.size();
    result.hash = HashRuntimeAssetDataBytes(bytes);

    const std::string text(bytes.begin(), bytes.end());
    if (HasUnsupportedHeader(text, expected_kind)) {
        result.status = RuntimeAssetDataStatus::UnsupportedVersion;
        *out_result = result;
        return result.status;
    }

    if (!HasSupportedHeader(text, expected_kind)) {
        result.status = RuntimeAssetDataStatus::InvalidHeader;
        *out_result = result;
        return result.status;
    }

    if (expected_kind == RuntimeAssetFileKind::Mesh) {
        if (Contains(text, "vertices=0") || Contains(text, "indices=0")) {
            result.status = RuntimeAssetDataStatus::InvalidBounds;
            *out_result = result;
            return result.status;
        }
    }

    if (expected_kind == RuntimeAssetFileKind::Scene) {
        result.status = ValidateSceneDependencies(text, &result);
        *out_result = result;
        return result.status;
    }

    if (expected_kind == RuntimeAssetFileKind::Shader) {
        result.status = ValidateShaderProgramDependencies(text, &result);
        *out_result = result;
        return result.status;
    }

    if (expected_kind == RuntimeAssetFileKind::Animation) {
        result.status = ValidateAnimationDependencies(text, &result);
        *out_result = result;
        return result.status;
    }

    result.status = RuntimeAssetDataStatus::Success;
    *out_result = result;
    return result.status;
}

RuntimeAssetDataStatus LoadRuntimeAssetDataGraph(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetGraphLoadResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetGraphLoadResult result{};
    RuntimeAssetDataStatus status = ValidateGraphRequest(request);
    if (status != RuntimeAssetDataStatus::Success) {
        result.status = status;
        *out_result = result;
        return status;
    }

    if (!ConfigureRuntimeAssetResourceBudgets(*request.resource_registry)) {
        result.status = RuntimeAssetDataStatus::ResourceResidencyFailed;
        *out_result = result;
        return result.status;
    }

    std::vector<std::uint8_t> scene_bytes{};
    status = ReadRuntimeAssetFile(request, request.scene_path, &scene_bytes);
    if (status != RuntimeAssetDataStatus::Success) {
        result.status = status;
        *out_result = result;
        return status;
    }

    ++result.file_read_count;
    RuntimeAssetValidationResult scene_validation{};
    status = ValidateRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(scene_bytes.data(), scene_bytes.size()),
        RuntimeAssetFileKind::Scene,
        &scene_validation);
    if (status != RuntimeAssetDataStatus::Success) {
        result.status = status;
        *out_result = result;
        return status;
    }

    const std::string scene_text(scene_bytes.begin(), scene_bytes.end());
    result.scene_dependency_count = scene_validation.dependency_count;
    result.scene_references_runtime_asset_families = SceneReferencesRuntimeFamilies(scene_text);
    if (!result.scene_references_runtime_asset_families) {
        result.status = RuntimeAssetDataStatus::MissingDependency;
        *out_result = result;
        return result.status;
    }

    RuntimeAssetFileDesc scene_desc{};
    scene_desc.path = request.scene_path.Value().data();
    scene_desc.kind = RuntimeAssetFileKind::Scene;
    scene_desc.resource_type = request.scene_resource_type;
    scene_desc.asset_type = request.scene_asset_type;
    scene_desc.stable_id = request.scene_stable_id;
    status = RegisterLoadedFile(
        *request.resource_registry,
        *request.asset_manager,
        scene_desc,
        std::span<const std::uint8_t>(scene_bytes.data(), scene_bytes.size()),
        &result.scene);
    if (status != RuntimeAssetDataStatus::Success) {
        result.status = status;
        *out_result = result;
        return status;
    }

    result.scene_registered = true;
    ++result.cache_payload_count;

    std::uint32_t file_index = 0U;
    while (file_index < request.file_count) {
        const RuntimeAssetFileDesc &file = request.files[file_index];
        if (file.path == nullptr) {
            result.status = RuntimeAssetDataStatus::InvalidArgument;
            *out_result = result;
            return result.status;
        }

        std::vector<std::uint8_t> bytes{};
        const yuengine::file::VirtualPath path(file.path);
        status = ReadRuntimeAssetFile(request, path, &bytes);
        if (status != RuntimeAssetDataStatus::Success) {
            result.status = status;
            *out_result = result;
            return status;
        }

        ++result.file_read_count;
        RuntimeAssetValidationResult validation{};
        status = ValidateRuntimeAssetDataBytes(
            std::span<const std::uint8_t>(bytes.data(), bytes.size()),
            file.kind,
            &validation);
        if (status != RuntimeAssetDataStatus::Success) {
            result.status = status;
            *out_result = result;
            return status;
        }

        status = RegisterLoadedFile(
            *request.resource_registry,
            *request.asset_manager,
            file,
            std::span<const std::uint8_t>(bytes.data(), bytes.size()),
            &request.loaded_files[file_index]);
        if (status != RuntimeAssetDataStatus::Success) {
            result.status = status;
            *out_result = result;
            return status;
        }

        ++result.loaded_file_count;
        ++result.cache_payload_count;
        if (request.loaded_files[file_index].decode_plan_created) {
            ++result.cache_payload_count;
        }

        if (request.loaded_files[file_index].decoded_payload_stored) {
            ++result.decoded_payload_count;
        }

        status = AddLoadedDependency(
            *request.resource_registry,
            *request.asset_manager,
            result.scene,
            request.loaded_files[file_index]);
        if (status != RuntimeAssetDataStatus::Success) {
            result.status = status;
            *out_result = result;
            return status;
        }

        ++result.resource_dependency_count;
        ++result.asset_dependency_count;
        ++file_index;
    }

    result.status = RuntimeAssetDataStatus::Success;
    *out_result = result;
    return result.status;
}

}
