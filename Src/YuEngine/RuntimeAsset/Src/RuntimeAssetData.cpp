// 模块: YuEngine RuntimeAsset
// 文件: Src/YuEngine/RuntimeAsset/Src/RuntimeAssetData.cpp

#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/Asset/AssetConstants.h"
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
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiShaderStage.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectDesc.h"
#include "YuEngine/World/WorldRegistrationResult.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformBridgeDesc.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::runtimeasset {
namespace {
using yuengine::asset::AssetDescriptor;
using yuengine::asset::AssetManager;
using yuengine::asset::AssetRegistrationResult;
using yuengine::asset::AssetStatus;
using yuengine::animation::AnimationRuntimeChannel;
using yuengine::animation::AnimationRuntimeClipRecord;
using yuengine::animation::AnimationRuntimeInterpolation;
using yuengine::animation::AnimationRuntimeKeyframeRecord;
using yuengine::animation::AnimationRuntimeSampleRequest;
using yuengine::animation::AnimationRuntimeSampleResult;
using yuengine::animation::AnimationRuntimeSampledValue;
using yuengine::animation::AnimationRuntimeSampler;
using yuengine::animation::AnimationRuntimeStatus;
using yuengine::animation::AnimationRuntimeTrackRecord;
using yuengine::animation::AnimationRuntimeTransformApplyRequest;
using yuengine::animation::AnimationRuntimeTransformApplyResult;
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
using yuengine::world::WorldInstance;
using yuengine::world::WorldObjectDesc;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldRegistrationResult;
using yuengine::world::WorldTransformBridge;
using yuengine::world::WorldTransformBridgeDesc;
using yuengine::world::WorldTransformResult;
using yuengine::world::WorldTransformState;
using yuengine::world::WorldTransformStatus;

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
constexpr std::string_view RUNTIME_ASSET_SOURCE_SCHEMA = "rav0-source";
constexpr std::string_view RUNTIME_ASSET_COOKED_SCHEMA = "rav1-cooked";
constexpr std::string_view SHADER_BYTECODE_PREFIX = "bytecode:";
constexpr std::string_view SHADER_LAYOUT_PREFIX = "layout:";
constexpr std::uint32_t RUNTIME_ASSET_SCENE_ENTITY_COUNT = 3U;
constexpr std::uint32_t RUNTIME_ASSET_SCENE_CAMERA_COUNT = 1U;
constexpr std::uint32_t RUNTIME_ASSET_SCENE_TRANSFORM_COUNT = 3U;
constexpr std::uint32_t MAX_RUNTIME_ASSET_DEPENDENCY_ROWS = 64U;
constexpr std::uint32_t MAX_RUNTIME_ASSET_RECORD_TABLES = 16U;
constexpr std::uint32_t MAX_RUNTIME_ASSET_RECORD_BYTES = 4096U;
constexpr std::uint32_t MAX_RUNTIME_ASSET_PAYLOAD_BYTES = 4096U;

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

bool ParseU32(std::string_view text, std::uint32_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    if (text.empty()) {
        return false;
    }

    std::uint64_t value = 0U;
    for (const char character : text) {
        if (character < '0' || character > '9') {
            return false;
        }

        value *= 10U;
        value += static_cast<std::uint64_t>(character - '0');
        if (value > std::numeric_limits<std::uint32_t>::max()) {
            return false;
        }
    }

    *out_value = static_cast<std::uint32_t>(value);
    return true;
}

bool ParseFloat(std::string_view text, float *out_value) {
    if (out_value == nullptr || text.empty()) {
        return false;
    }

    const std::string copy(text);
    char *end = nullptr;
    const float parsed = std::strtof(copy.c_str(), &end);
    if (end != copy.c_str() + copy.size()) {
        return false;
    }

    *out_value = parsed;
    return true;
}

bool ParseVec3(std::string_view text, float *out_x, float *out_y, float *out_z) {
    const std::size_t first_comma = text.find(',');
    if (first_comma == std::string_view::npos) {
        return false;
    }

    const std::size_t second_comma = text.find(',', first_comma + 1U);
    if (second_comma == std::string_view::npos) {
        return false;
    }

    return ParseFloat(text.substr(0U, first_comma), out_x) &&
        ParseFloat(text.substr(first_comma + 1U, second_comma - first_comma - 1U), out_y) &&
        ParseFloat(text.substr(second_comma + 1U), out_z);
}

bool ParseSceneEntityValue(
    std::string_view value,
    std::uint32_t entity_id,
    std::uint32_t mesh_ref_index,
    std::uint32_t material_ref_index,
    std::uint32_t texture_ref_index,
    std::uint32_t shader_ref_index,
    std::uint32_t camera_index,
    std::uint32_t animation_ref_index,
    RuntimeAssetSceneEntityRecord *out_record) {
    if (out_record == nullptr) {
        return false;
    }

    std::uint32_t world_id = 0U;
    std::string_view transform_components{};
    constexpr std::string_view entity_prefix = "scene_entity:";
    constexpr std::string_view transform_prefix = "transform:";
    const bool uses_scene_entity_prefix = StartsWith(value, entity_prefix);
    if (uses_scene_entity_prefix) {
        const std::size_t comma = value.find(',');
        if (comma == std::string_view::npos) {
            return false;
        }

        if (!ParseU32(value.substr(entity_prefix.size(), comma - entity_prefix.size()), &world_id)) {
            return false;
        }

        const std::string_view transform_value = value.substr(comma + 1U);
        if (!StartsWith(transform_value, transform_prefix)) {
            return false;
        }

        transform_components = transform_value.substr(transform_prefix.size());
    }

    if (!uses_scene_entity_prefix) {
        const std::size_t colon = value.find(':');
        if (colon == std::string_view::npos) {
            return false;
        }

        if (!ParseU32(value.substr(0U, colon), &world_id)) {
            return false;
        }

        transform_components = value.substr(colon + 1U);
    }

    WorldTransformState transform{};
    if (!ParseVec3(
            transform_components,
            &transform.translation_x,
            &transform.translation_y,
            &transform.translation_z)) {
        return false;
    }

    out_record->entity_id = entity_id;
    out_record->world_object_id = WorldObjectId{world_id};
    out_record->transform = transform;
    out_record->mesh_ref_index = mesh_ref_index;
    out_record->material_ref_index = material_ref_index;
    out_record->texture_ref_index = texture_ref_index;
    out_record->shader_ref_index = shader_ref_index;
    out_record->camera_index = camera_index;
    out_record->animation_ref_index = animation_ref_index;
    out_record->is_visible = true;
    out_record->is_active = true;
    return true;
}

bool ParseAnimationChannel(std::string_view value, AnimationRuntimeChannel *out_channel) {
    if (out_channel == nullptr) {
        return false;
    }

    if (value == "transform:translation_x") {
        *out_channel = AnimationRuntimeChannel::TranslationX;
        return true;
    }

    if (value == "transform:translation_y") {
        *out_channel = AnimationRuntimeChannel::TranslationY;
        return true;
    }

    if (value == "transform:translation_z") {
        *out_channel = AnimationRuntimeChannel::TranslationZ;
        return true;
    }

    if (value == "transform:rotation_x") {
        *out_channel = AnimationRuntimeChannel::RotationX;
        return true;
    }

    if (value == "transform:rotation_y") {
        *out_channel = AnimationRuntimeChannel::RotationY;
        return true;
    }

    if (value == "transform:rotation_z") {
        *out_channel = AnimationRuntimeChannel::RotationZ;
        return true;
    }

    if (value == "transform:rotation_w") {
        *out_channel = AnimationRuntimeChannel::RotationW;
        return true;
    }

    if (value == "transform:scale_x") {
        *out_channel = AnimationRuntimeChannel::ScaleX;
        return true;
    }

    if (value == "transform:scale_y") {
        *out_channel = AnimationRuntimeChannel::ScaleY;
        return true;
    }

    if (value == "transform:scale_z") {
        *out_channel = AnimationRuntimeChannel::ScaleZ;
        return true;
    }

    return false;
}

bool ParseAnimationTarget(std::string_view value, WorldObjectId *out_target) {
    if (out_target == nullptr) {
        return false;
    }

    constexpr std::string_view prefix = "scene_entity:";
    if (!StartsWith(value, prefix)) {
        return false;
    }

    std::uint32_t id = 0U;
    if (!ParseU32(value.substr(prefix.size()), &id)) {
        return false;
    }

    *out_target = WorldObjectId{id};
    return true;
}

bool ParseKeyframe(std::string_view value, AnimationRuntimeKeyframeRecord *out_keyframe) {
    if (out_keyframe == nullptr) {
        return false;
    }

    const std::size_t colon = value.find(':');
    if (colon == std::string_view::npos) {
        return false;
    }

    AnimationRuntimeKeyframeRecord keyframe{};
    if (!ParseFloat(value.substr(0U, colon), &keyframe.time_seconds) ||
        !ParseFloat(value.substr(colon + 1U), &keyframe.value)) {
        return false;
    }

    keyframe.is_valid = true;
    *out_keyframe = keyframe;
    return true;
}

bool FindRefIndex(
    const RuntimeAssetFileDesc *files,
    std::uint32_t file_count,
    RuntimeAssetFileKind kind,
    std::uint32_t ordinal,
    std::uint32_t *out_index) {
    if (files == nullptr || out_index == nullptr) {
        return false;
    }

    std::uint32_t found_count = 0U;
    for (std::uint32_t index = 0U; index < file_count; ++index) {
        if (files[index].kind != kind) {
            continue;
        }

        if (found_count == ordinal) {
            *out_index = index;
            return true;
        }

        ++found_count;
    }

    return false;
}

std::string HeaderFor(
    RuntimeAssetArtifactClass artifact_class,
    RuntimeAssetFileKind expected_kind,
    std::uint32_t version) {
    const char *prefix = "YUASSET ";
    if (artifact_class == RuntimeAssetArtifactClass::Cooked) {
        prefix = "YUCOOKED ";
    }

    return std::string(prefix) + std::string(RuntimeAssetFileKindName(expected_kind)) + " " +
        std::to_string(version);
}

bool HasSupportedHeader(
    std::string_view text,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetArtifactClass artifact_class) {
    const std::string header =
        HeaderFor(artifact_class, expected_kind, 1U);
    return Contains(text, header);
}

bool HasUnsupportedHeader(
    std::string_view text,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetArtifactClass artifact_class) {
    const std::string header =
        HeaderFor(artifact_class, expected_kind, 2U);
    return Contains(text, header);
}

RuntimeAssetArtifactClass DetectSupportedArtifactClass(
    std::string_view text,
    RuntimeAssetFileKind expected_kind) {
    if (HasSupportedHeader(text, expected_kind, RuntimeAssetArtifactClass::Source)) {
        return RuntimeAssetArtifactClass::Source;
    }

    if (HasSupportedHeader(text, expected_kind, RuntimeAssetArtifactClass::Cooked)) {
        return RuntimeAssetArtifactClass::Cooked;
    }

    return RuntimeAssetArtifactClass::Unknown;
}

RuntimeAssetArtifactClass DetectUnsupportedArtifactClass(
    std::string_view text,
    RuntimeAssetFileKind expected_kind) {
    if (HasUnsupportedHeader(text, expected_kind, RuntimeAssetArtifactClass::Source)) {
        return RuntimeAssetArtifactClass::Source;
    }

    if (HasUnsupportedHeader(text, expected_kind, RuntimeAssetArtifactClass::Cooked)) {
        return RuntimeAssetArtifactClass::Cooked;
    }

    return RuntimeAssetArtifactClass::Unknown;
}

std::string_view RuntimeAssetFamilyToken(RuntimeAssetFileKind kind) {
    switch (kind) {
        case RuntimeAssetFileKind::Mesh:
            return "mesh";
        case RuntimeAssetFileKind::Material:
            return "material";
        case RuntimeAssetFileKind::Texture:
            return "texture";
        case RuntimeAssetFileKind::Shader:
            return "shader";
        case RuntimeAssetFileKind::Scene:
            return "scene";
        case RuntimeAssetFileKind::Animation:
            return "animation";
        case RuntimeAssetFileKind::Unknown:
            break;
        default:
            break;
    }

    return {};
}

bool IsSupportedPayloadAlignment(std::uint32_t alignment) {
    return alignment == 1U || alignment == 2U || alignment == 4U ||
        alignment == 8U || alignment == 16U;
}

std::uint64_t HashRuntimeAssetText(std::string_view text) {
    std::uint64_t hash = FNV_OFFSET;
    for (const char character : text) {
        hash ^= static_cast<std::uint64_t>(static_cast<std::uint8_t>(character));
        hash *= FNV_PRIME;
    }

    return hash;
}

bool ParseU64(std::string_view text, std::uint64_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    if (text.empty()) {
        return false;
    }

    std::uint64_t value = 0U;
    for (const char character : text) {
        if (character < '0' || character > '9') {
            return false;
        }

        const std::uint64_t digit = static_cast<std::uint64_t>(character - '0');
        if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10U) {
            return false;
        }

        value *= 10U;
        value += digit;
    }

    *out_value = value;
    return true;
}

bool ParseExtent(std::string_view text, std::uint32_t *out_width, std::uint32_t *out_height) {
    if (out_width == nullptr) {
        return false;
    }

    if (out_height == nullptr) {
        return false;
    }

    const std::size_t separator = text.find('x');
    if (separator == std::string_view::npos) {
        return false;
    }

    const std::string_view width_text = text.substr(0U, separator);
    const std::string_view height_text = text.substr(separator + 1U);
    std::uint32_t width = 0U;
    std::uint32_t height = 0U;
    if (!ParseU32(width_text, &width)) {
        return false;
    }

    if (!ParseU32(height_text, &height)) {
        return false;
    }

    *out_width = width;
    *out_height = height;
    return true;
}

RuntimeAssetDataStatus ValidateCookedDependencyRows(
    std::string_view text,
    std::uint32_t dependency_count,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (dependency_count > MAX_RUNTIME_ASSET_DEPENDENCY_ROWS) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    std::uint32_t index = 0U;
    while (index < dependency_count) {
        const std::string token = "dep" + std::to_string(index) + "=";
        const std::size_t token_count = CountToken(text, token);
        if (token_count == 0U) {
            return RuntimeAssetDataStatus::MissingDependency;
        }

        if (token_count > 1U) {
            return RuntimeAssetDataStatus::DuplicateDependency;
        }

        const std::string_view row = ValueForToken(text, token);
        const std::size_t first_separator = row.find(':');
        const std::size_t second_separator =
            first_separator == std::string_view::npos
                ? std::string_view::npos
                : row.find(':', first_separator + 1U);
        if (first_separator == std::string_view::npos ||
            second_separator == std::string_view::npos ||
            first_separator == 0U ||
            second_separator + 1U >= row.size()) {
            return RuntimeAssetDataStatus::InvalidDependency;
        }

        std::uint64_t dependency_hash = 0U;
        if (!ParseU64(row.substr(second_separator + 1U), &dependency_hash) ||
            dependency_hash == 0U) {
            return RuntimeAssetDataStatus::HashMismatch;
        }

        ++index;
    }

    out_result->dependency_count = dependency_count;
    out_result->dependency_table_count = dependency_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateCookedMetadata(
    std::string_view text,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (ValueForToken(text, "schema=") != RUNTIME_ASSET_COOKED_SCHEMA) {
        return RuntimeAssetDataStatus::InvalidSchema;
    }

    const std::string_view family = RuntimeAssetFamilyToken(expected_kind);
    if (family.empty() || ValueForToken(text, "kind=") != family) {
        return RuntimeAssetDataStatus::InvalidKind;
    }

    const std::string_view id = ValueForToken(text, "id=");
    if (id.empty() || Contains(id, " ")) {
        return RuntimeAssetDataStatus::InvalidSchema;
    }

    std::uint32_t dependency_count = 0U;
    if (!ParseU32(ValueForToken(text, "dependencyTable="), &dependency_count)) {
        return RuntimeAssetDataStatus::InvalidCount;
    }

    std::uint32_t record_table_count = 0U;
    if (!ParseU32(ValueForToken(text, "recordTable="), &record_table_count) ||
        record_table_count == 0U) {
        return RuntimeAssetDataStatus::InvalidCount;
    }

    if (record_table_count > MAX_RUNTIME_ASSET_RECORD_TABLES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    std::uint32_t record_byte_count = 0U;
    if (!ParseU32(ValueForToken(text, "recordBytes="), &record_byte_count) ||
        record_byte_count == 0U) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (record_byte_count > MAX_RUNTIME_ASSET_RECORD_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    std::uint32_t payload_byte_count = 0U;
    if (!ParseU32(ValueForToken(text, "payloadBytes="), &payload_byte_count)) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (payload_byte_count > MAX_RUNTIME_ASSET_PAYLOAD_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    std::uint32_t payload_alignment = 0U;
    if (!ParseU32(ValueForToken(text, "payloadAlign="), &payload_alignment) ||
        !IsSupportedPayloadAlignment(payload_alignment)) {
        return RuntimeAssetDataStatus::InvalidAlignment;
    }

    std::uint64_t source_hash = 0U;
    if (!ParseU64(ValueForToken(text, "sourceHash="), &source_hash) ||
        source_hash == 0U) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    std::uint64_t payload_hash = 0U;
    if (!ParseU64(ValueForToken(text, "payloadHash="), &payload_hash)) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    const std::string_view payload = ValueForToken(text, "payload=");
    if (payload_byte_count > 0U) {
        if (payload.empty() || payload.size() != payload_byte_count) {
            return RuntimeAssetDataStatus::InvalidSize;
        }

        if (HashRuntimeAssetText(payload) != payload_hash) {
            return RuntimeAssetDataStatus::HashMismatch;
        }
    }

    if (payload_byte_count == 0U && payload_hash != 0U) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    RuntimeAssetDataStatus status = ValidateCookedDependencyRows(text, dependency_count, out_result);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    out_result->artifact_class = RuntimeAssetArtifactClass::Cooked;
    out_result->schema_version = 1U;
    out_result->identity_hash = HashRuntimeAssetText(id);
    out_result->source_hash = source_hash;
    out_result->payload_hash = payload_hash;
    out_result->record_table_count = record_table_count;
    out_result->record_table_byte_count = record_byte_count;
    out_result->payload_byte_count = payload_byte_count;
    out_result->payload_alignment = payload_alignment;
    return RuntimeAssetDataStatus::Success;
}

bool RequiresSourceSchema(RuntimeAssetFileKind kind) {
    return kind != RuntimeAssetFileKind::Unknown;
}

RuntimeAssetDataStatus ValidateCommonMetadata(
    std::string_view text,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const RuntimeAssetArtifactClass unsupported_artifact =
        DetectUnsupportedArtifactClass(text, expected_kind);
    if (unsupported_artifact != RuntimeAssetArtifactClass::Unknown) {
        out_result->artifact_class = unsupported_artifact;
        out_result->version = 2U;
        return RuntimeAssetDataStatus::UnsupportedVersion;
    }

    const RuntimeAssetArtifactClass artifact_class =
        DetectSupportedArtifactClass(text, expected_kind);
    if (artifact_class == RuntimeAssetArtifactClass::Unknown) {
        if (Contains(text, "YUASSET ") || Contains(text, "YUCOOKED ")) {
            return RuntimeAssetDataStatus::InvalidKind;
        }

        return RuntimeAssetDataStatus::InvalidHeader;
    }

    out_result->artifact_class = artifact_class;
    out_result->version = 1U;
    if (artifact_class == RuntimeAssetArtifactClass::Cooked) {
        return ValidateCookedMetadata(text, expected_kind, out_result);
    }

    const std::string_view schema = ValueForToken(text, "schema=");
    if (!RequiresSourceSchema(expected_kind) && schema.empty()) {
        const std::string_view id = ValueForToken(text, "id=");
        if (!id.empty() && Contains(id, " ")) {
            return RuntimeAssetDataStatus::InvalidSchema;
        }

        if (!id.empty()) {
            out_result->identity_hash = HashRuntimeAssetText(id);
        }

        out_result->artifact_class = RuntimeAssetArtifactClass::Source;
        out_result->source_hash = out_result->hash;
        out_result->record_table_count = 1U;
        out_result->record_table_byte_count = static_cast<std::uint32_t>(out_result->byte_count);
        return RuntimeAssetDataStatus::Success;
    }

    if (schema != RUNTIME_ASSET_SOURCE_SCHEMA) {
        return RuntimeAssetDataStatus::InvalidSchema;
    }

    out_result->schema_version = 1U;
    const std::string_view id = ValueForToken(text, "id=");
    if (id.empty()) {
        return RuntimeAssetDataStatus::InvalidSchema;
    }

    if (Contains(id, " ")) {
        return RuntimeAssetDataStatus::InvalidSchema;
    }

    out_result->identity_hash = HashRuntimeAssetText(id);
    out_result->artifact_class = RuntimeAssetArtifactClass::Source;
    out_result->source_hash = out_result->hash;
    out_result->record_table_count = 1U;
    if (out_result->byte_count > MAX_RUNTIME_ASSET_RECORD_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    out_result->record_table_byte_count = static_cast<std::uint32_t>(out_result->byte_count);
    return RuntimeAssetDataStatus::Success;
}

struct DependencyRule final {
    std::string_view token;
    std::string_view expected_prefix;
};

struct RuntimeAssetSceneLoaderStage final {
    std::array<RuntimeAssetSceneCameraRecord, RUNTIME_ASSET_SCENE_CAMERA_COUNT> cameras{};
    std::array<RuntimeAssetSceneEntityRecord, RUNTIME_ASSET_SCENE_ENTITY_COUNT> entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, RUNTIME_ASSET_SCENE_TRANSFORM_COUNT> transforms{};
    std::uint32_t camera_count = 0U;
    std::uint32_t entity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t animation_sampled_value_count = 0U;
    AnimationRuntimeStatus animation_sample_status = AnimationRuntimeStatus::MissingClip;
    AnimationRuntimeStatus animation_apply_status = AnimationRuntimeStatus::MissingSample;
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

    out_result->dependency_table_count = static_cast<std::uint32_t>(out_result->dependency_count);
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

RuntimeAssetDataStatus ValidateMeshMetadata(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    std::string_view mesh_kind = ValueForToken(text, "shape=");
    if (mesh_kind.empty()) {
        mesh_kind = ValueForToken(text, "kind=");
    }

    if (mesh_kind.empty()) {
        return RuntimeAssetDataStatus::InvalidKind;
    }

    if (mesh_kind != "cube" && mesh_kind != "cylinder" && mesh_kind != "cone") {
        return RuntimeAssetDataStatus::InvalidKind;
    }

    std::uint32_t vertex_count = 0U;
    if (!ParseU32(ValueForToken(text, "vertices="), &vertex_count)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (vertex_count == 0U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    std::uint32_t index_count = 0U;
    if (!ParseU32(ValueForToken(text, "indices="), &index_count)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (index_count == 0U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    const std::string_view bounds = ValueForToken(text, "bounds=");
    if (CountToken(bounds, ",") != 5U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    out_result->vertex_count = vertex_count;
    out_result->index_count = index_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateMaterialMetadata(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    constexpr std::array<DependencyRule, 4U> rules{{
        {"shader=", "Shader/"},
        {"texture0=", "Texture/"},
        {"texture1=", "Texture/"},
        {"texture2=", "Texture/"},
    }};

    RuntimeAssetDataStatus status =
        ValidateDependencyRules(text, std::span<const DependencyRule>(rules.data(), rules.size()), out_result);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    const std::string_view texture0 = ValueForToken(text, "texture0=");
    const std::string_view texture1 = ValueForToken(text, "texture1=");
    const std::string_view texture2 = ValueForToken(text, "texture2=");
    if (texture0 == texture1 || texture0 == texture2 || texture1 == texture2) {
        return RuntimeAssetDataStatus::DuplicateDependency;
    }

    out_result->texture_slot_count = 3U;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateTextureMetadata(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const std::string_view format = ValueForToken(text, "format=");
    if (format != "rgba8") {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    std::uint32_t width = 0U;
    std::uint32_t height = 0U;
    if (!ParseExtent(ValueForToken(text, "extent="), &width, &height)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (width == 0U || height == 0U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (width > 4096U || height > 4096U) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    const std::string_view payload = ValueForToken(text, "payload=");
    if (payload.empty()) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    out_result->texture_width = width;
    out_result->texture_height = height;
    return RuntimeAssetDataStatus::Success;
}

std::size_t ShaderInputElementByteSize(yuengine::rhi::RhiInputElementFormat format) {
    switch (format) {
        case yuengine::rhi::RhiInputElementFormat::Float32x2:
            return 8U;
        case yuengine::rhi::RhiInputElementFormat::Float32x3:
            return 12U;
        case yuengine::rhi::RhiInputElementFormat::Float32x4:
            return 16U;
        case yuengine::rhi::RhiInputElementFormat::Unsupported:
            break;
        default:
            break;
    }

    return 0U;
}

RuntimeAssetDataStatus DecodeShaderInputElement(
    std::string_view semantic,
    std::size_t offset_bytes,
    yuengine::rhi::RhiInputElementDesc *out_element,
    std::size_t *out_byte_count) {
    if (out_element == nullptr || out_byte_count == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    yuengine::rhi::RhiInputElementDesc element{};
    if (semantic == "position") {
        element.semantic = yuengine::rhi::RhiInputElementSemantic::Position;
        element.format = yuengine::rhi::RhiInputElementFormat::Float32x2;
    }

    if (semantic == "color") {
        element.semantic = yuengine::rhi::RhiInputElementSemantic::Color;
        element.format = yuengine::rhi::RhiInputElementFormat::Float32x4;
    }

    if (semantic == "texcoord") {
        element.semantic = yuengine::rhi::RhiInputElementSemantic::TexCoord;
        element.format = yuengine::rhi::RhiInputElementFormat::Float32x2;
    }

    if (element.semantic == yuengine::rhi::RhiInputElementSemantic::Unsupported) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    const std::size_t byte_count = ShaderInputElementByteSize(element.format);
    if (byte_count == 0U) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    element.offset_bytes = offset_bytes;
    *out_element = element;
    *out_byte_count = byte_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus DecodeShaderInputLayout(
    std::string_view input,
    yuengine::rhi::RhiInputLayoutDesc *out_layout) {
    if (out_layout == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (!StartsWith(input, SHADER_LAYOUT_PREFIX)) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    const std::string_view semantic_list = input.substr(SHADER_LAYOUT_PREFIX.size());
    if (semantic_list.empty()) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    yuengine::rhi::RhiInputLayoutDesc layout{};
    std::size_t cursor = 0U;
    std::size_t stride_bytes = 0U;
    bool has_position = false;
    while (cursor <= semantic_list.size()) {
        const std::size_t separator = semantic_list.find(',', cursor);
        std::string_view semantic{};
        if (separator == std::string_view::npos) {
            semantic = semantic_list.substr(cursor);
        }

        if (separator != std::string_view::npos) {
            semantic = semantic_list.substr(cursor, separator - cursor);
        }

        if (semantic.empty()) {
            return RuntimeAssetDataStatus::InvalidInputLayout;
        }

        if (layout.element_count >= yuengine::rhi::MAX_RHI_INPUT_ELEMENTS) {
            return RuntimeAssetDataStatus::CapacityExceeded;
        }

        yuengine::rhi::RhiInputElementDesc element{};
        std::size_t element_byte_count = 0U;
        const RuntimeAssetDataStatus status =
            DecodeShaderInputElement(semantic, stride_bytes, &element, &element_byte_count);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }

        if (element.semantic == yuengine::rhi::RhiInputElementSemantic::Position) {
            has_position = true;
        }

        layout.elements[layout.element_count] = element;
        ++layout.element_count;
        stride_bytes += element_byte_count;
        if (separator == std::string_view::npos) {
            break;
        }

        cursor = separator + 1U;
    }

    if (!has_position) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    layout.stride_bytes = stride_bytes;
    *out_layout = layout;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ShaderBytecodePayloadForValue(
    std::string_view value,
    std::string_view *out_payload) {
    if (out_payload == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (!StartsWith(value, SHADER_BYTECODE_PREFIX)) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    const std::string_view payload = value.substr(SHADER_BYTECODE_PREFIX.size());
    if (payload.empty()) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (payload.size() > yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    *out_payload = payload;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus CopyShaderBytecode(
    std::string_view text,
    std::string_view token,
    std::array<std::uint8_t, yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES> *destination,
    std::size_t *out_size,
    std::uint64_t *out_hash) {
    if (destination == nullptr || out_size == nullptr || out_hash == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    std::string_view payload{};
    const RuntimeAssetDataStatus status =
        ShaderBytecodePayloadForValue(ValueForToken(text, token), &payload);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    std::fill(destination->begin(), destination->end(), std::uint8_t{0U});
    std::transform(
        payload.begin(),
        payload.end(),
        destination->begin(),
        [](char character) {
            return static_cast<std::uint8_t>(character);
        });
    *out_size = payload.size();
    *out_hash = HashRuntimeAssetText(payload);
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateOptionalShaderHash(
    std::string_view text,
    std::string_view token,
    std::uint64_t actual_hash) {
    const std::string_view hash_text = ValueForToken(text, token);
    if (hash_text.empty()) {
        return RuntimeAssetDataStatus::Success;
    }

    std::uint64_t expected_hash = 0U;
    if (!ParseU64(hash_text, &expected_hash)) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    if (expected_hash != actual_hash) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateShaderProgramMetadata(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    RuntimeAssetDataStatus status = ValidateShaderProgramDependencies(text, out_result);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    std::uint32_t texture_slot_count = 0U;
    if (!ParseU32(ValueForToken(text, "textures="), &texture_slot_count)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (texture_slot_count > yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    out_result->shader_stage_count = 2U;
    out_result->texture_slot_count = texture_slot_count;
    const std::string_view vertex_stage = ValueForToken(text, "stage_vs=");
    const std::string_view pixel_stage = ValueForToken(text, "stage_ps=");
    std::string_view vertex_payload{};
    status = ShaderBytecodePayloadForValue(vertex_stage, &vertex_payload);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    std::string_view pixel_payload{};
    status = ShaderBytecodePayloadForValue(pixel_stage, &pixel_payload);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    yuengine::rhi::RhiInputLayoutDesc input_layout{};
    status = DecodeShaderInputLayout(ValueForToken(text, "input="), &input_layout);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    out_result->shader_bytecode_byte_count =
        static_cast<std::uint32_t>(vertex_payload.size() + pixel_payload.size());
    return RuntimeAssetDataStatus::Success;
}

bool TokenValueStartsWith(std::string_view text, std::string_view token, std::string_view prefix) {
    const std::string_view value = ValueForToken(text, token);
    return !value.empty() && StartsWith(value, prefix);
}

bool SceneReferencesRuntimeFamilies(std::string_view scene_text) {
    RuntimeAssetValidationResult validation{};
    if (ValidateCommonMetadata(scene_text, RuntimeAssetFileKind::Scene, &validation) !=
        RuntimeAssetDataStatus::Success) {
        return false;
    }

    if (!TokenValueStartsWith(scene_text, "m0=", "Mesh/")) {
        return false;
    }

    if (!TokenValueStartsWith(scene_text, "m1=", "Mesh/")) {
        return false;
    }

    if (!TokenValueStartsWith(scene_text, "m2=", "Mesh/")) {
        return false;
    }

    if (!TokenValueStartsWith(scene_text, "mat=", "Material/")) {
        return false;
    }

    if (!TokenValueStartsWith(scene_text, "t0=", "Texture/")) {
        return false;
    }

    if (!TokenValueStartsWith(scene_text, "prog=", "Shader/")) {
        return false;
    }

    if (!TokenValueStartsWith(scene_text, "cam=", "camera:")) {
        return false;
    }

    return TokenValueStartsWith(scene_text, "anim=", "Animation/");
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

    out_record->decode_plan_payload_id = plan_payload_id;
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
    record.resource_type = desc.resource_type;
    record.asset_type = desc.asset_type;
    record.stable_id = desc.stable_id;
    record.hash = HashRuntimeAssetDataBytes(bytes);
    record.byte_count = static_cast<std::uint32_t>(bytes.size());
    record.decode_asset_class = desc.decode_asset_class;
    record.decode_result_class = desc.decode_result_class;
    record.decoded_byte_count = desc.decoded_byte_count;

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

    if (request.scene_output != nullptr) {
        if (request.scene_resource_refs == nullptr ||
            request.scene_cameras == nullptr ||
            request.scene_entities == nullptr ||
            request.scene_transforms == nullptr) {
            return RuntimeAssetDataStatus::InvalidArgument;
        }

        if (request.scene_resource_ref_capacity < request.file_count ||
            request.scene_camera_capacity < RUNTIME_ASSET_SCENE_CAMERA_COUNT ||
            request.scene_entity_capacity < RUNTIME_ASSET_SCENE_ENTITY_COUNT ||
            request.scene_transform_capacity < RUNTIME_ASSET_SCENE_TRANSFORM_COUNT) {
            return RuntimeAssetDataStatus::CapacityExceeded;
        }
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

std::size_t GetInputElementByteSize(yuengine::rhi::RhiInputElementFormat format) {
    return ShaderInputElementByteSize(format);
}

bool IsInputElementValid(
    const yuengine::rhi::RhiInputElementDesc &element,
    std::size_t stride_bytes) {
    if (element.semantic == yuengine::rhi::RhiInputElementSemantic::Unsupported) {
        return false;
    }

    const std::size_t element_size = GetInputElementByteSize(element.format);
    if (element_size == 0U) {
        return false;
    }

    if (stride_bytes < element_size) {
        return false;
    }

    const std::size_t max_offset = stride_bytes - element_size;
    return element.offset_bytes <= max_offset;
}

bool IsInputLayoutValid(const yuengine::rhi::RhiInputLayoutDesc &layout) {
    if (layout.element_count == 0U) {
        return false;
    }

    if (layout.element_count > yuengine::rhi::MAX_RHI_INPUT_ELEMENTS) {
        return false;
    }

    if (layout.stride_bytes == 0U) {
        return false;
    }

    for (std::size_t index = 0U; index < layout.element_count; ++index) {
        if (!IsInputElementValid(layout.elements[index], layout.stride_bytes)) {
            return false;
        }
    }

    return true;
}

std::span<const std::uint8_t> VertexShaderBytecodeSpan(
    const RuntimeAssetLoadedShaderProgramData &program) {
    return std::span<const std::uint8_t>(
        program.vertex_bytecode.data(),
        program.vertex_bytecode_size);
}

std::span<const std::uint8_t> PixelShaderBytecodeSpan(
    const RuntimeAssetLoadedShaderProgramData &program) {
    return std::span<const std::uint8_t>(
        program.pixel_bytecode.data(),
        program.pixel_bytecode_size);
}

RuntimeAssetDataStatus ValidateShaderProgramPipelineRequest(
    const RuntimeAssetShaderProgramPipelineRequest &request) {
    if (request.device == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.program == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const RuntimeAssetLoadedShaderProgramData &program = *request.program;
    if (program.status != RuntimeAssetDataStatus::Success) {
        return program.status;
    }

    if (program.program_id == 0U) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const std::span<const std::uint8_t> vertex_bytecode = VertexShaderBytecodeSpan(program);
    const std::span<const std::uint8_t> pixel_bytecode = PixelShaderBytecodeSpan(program);
    if (vertex_bytecode.empty()) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (pixel_bytecode.empty()) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (vertex_bytecode.size() > yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    if (pixel_bytecode.size() > yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    if (HashRuntimeAssetDataBytes(vertex_bytecode) != program.vertex_bytecode_hash) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    if (HashRuntimeAssetDataBytes(pixel_bytecode) != program.pixel_bytecode_hash) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    if (!IsInputLayoutValid(program.input_layout)) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    if (program.texture_slot_count > yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus BuildSceneLoaderStage(
    const RuntimeAssetGraphLoadRequest &request,
    std::string_view scene_text,
    std::string_view animation_text,
    RuntimeAssetSceneLoaderStage *out_stage) {
    if (out_stage == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetSceneLoaderStage stage{};
    std::uint32_t cube_ref = 0U;
    std::uint32_t cylinder_ref = 0U;
    std::uint32_t cone_ref = 0U;
    std::uint32_t material_ref = 0U;
    std::uint32_t texture_ref = 0U;
    std::uint32_t shader_ref = 0U;
    std::uint32_t animation_ref = 0U;
    if (!FindRefIndex(request.files, request.file_count, RuntimeAssetFileKind::Mesh, 0U, &cube_ref) ||
        !FindRefIndex(request.files, request.file_count, RuntimeAssetFileKind::Mesh, 1U, &cylinder_ref) ||
        !FindRefIndex(request.files, request.file_count, RuntimeAssetFileKind::Mesh, 2U, &cone_ref) ||
        !FindRefIndex(request.files, request.file_count, RuntimeAssetFileKind::Material, 0U, &material_ref) ||
        !FindRefIndex(request.files, request.file_count, RuntimeAssetFileKind::Texture, 0U, &texture_ref) ||
        !FindRefIndex(request.files, request.file_count, RuntimeAssetFileKind::Shader, 0U, &shader_ref) ||
        !FindRefIndex(request.files, request.file_count, RuntimeAssetFileKind::Animation, 0U, &animation_ref)) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    RuntimeAssetSceneCameraRecord camera{};
    camera.camera_id = 1U;
    camera.is_active = true;
    stage.cameras[0U] = camera;
    stage.camera_count = 1U;

    if (!ParseSceneEntityValue(
            ValueForToken(scene_text, "e0="),
            1U,
            cube_ref,
            material_ref,
            texture_ref,
            shader_ref,
            0U,
            animation_ref,
            &stage.entities[0U]) ||
        !ParseSceneEntityValue(
            ValueForToken(scene_text, "e1="),
            2U,
            cylinder_ref,
            material_ref,
            texture_ref,
            shader_ref,
            0U,
            animation_ref,
            &stage.entities[1U]) ||
        !ParseSceneEntityValue(
            ValueForToken(scene_text, "e2="),
            3U,
            cone_ref,
            material_ref,
            texture_ref,
            shader_ref,
            0U,
            animation_ref,
            &stage.entities[2U])) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }
    stage.entity_count = RUNTIME_ASSET_SCENE_ENTITY_COUNT;

    if (animation_text.empty()) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    std::uint32_t clip_id = 0U;
    float duration_seconds = 0.0F;
    WorldObjectId animation_target{};
    AnimationRuntimeChannel animation_channel = AnimationRuntimeChannel::TranslationX;
    std::array<AnimationRuntimeKeyframeRecord, 2U> keyframes{};
    if (!ParseU32(ValueForToken(animation_text, "clip="), &clip_id) ||
        !ParseFloat(ValueForToken(animation_text, "duration="), &duration_seconds) ||
        !ParseAnimationTarget(ValueForToken(animation_text, "target="), &animation_target) ||
        !ParseAnimationChannel(ValueForToken(animation_text, "track="), &animation_channel) ||
        !ParseKeyframe(ValueForToken(animation_text, "key0="), &keyframes[0U]) ||
        !ParseKeyframe(ValueForToken(animation_text, "key1="), &keyframes[1U])) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    WorldInstance world;
    WorldTransformBridge bridge(world, WorldTransformBridgeDesc{RUNTIME_ASSET_SCENE_ENTITY_COUNT});
    for (std::uint32_t index = 0U; index < RUNTIME_ASSET_SCENE_ENTITY_COUNT; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = stage.entities[index];
        const WorldRegistrationResult registration =
            world.RegisterObject(WorldObjectDesc{entity.world_object_id, entity.is_active});
        if (!registration.Succeeded()) {
            return RuntimeAssetDataStatus::InvalidDependency;
        }

        if (bridge.Register(entity.world_object_id, entity.transform).status != WorldTransformStatus::Success) {
            return RuntimeAssetDataStatus::InvalidDependency;
        }
    }

    AnimationRuntimeClipRecord clip{};
    clip.clip_id = clip_id;
    clip.duration_seconds = duration_seconds;
    clip.first_track_index = 0U;
    clip.track_count = 1U;
    clip.layer_count = 1U;
    clip.is_valid = true;

    AnimationRuntimeTrackRecord track{};
    track.track_id = 1U;
    track.target = animation_target;
    track.channel = animation_channel;
    track.interpolation = AnimationRuntimeInterpolation::Linear;
    track.first_keyframe_index = 0U;
    track.keyframe_count = keyframes.size();
    track.is_valid = true;

    const std::array<AnimationRuntimeClipRecord, 1U> clips{clip};
    const std::array<AnimationRuntimeTrackRecord, 1U> tracks{track};
    std::array<AnimationRuntimeSampledValue, 1U> sampled_values{};

    AnimationRuntimeSampleRequest sample_request{};
    sample_request.clip_id = clip_id;
    sample_request.clips = std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size());
    sample_request.tracks = std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size());
    sample_request.keyframes = std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size());
    sample_request.frame_context = request.animation_frame_context;
    sample_request.clip_start_time_nanoseconds = request.animation_clip_start_time_nanoseconds;

    AnimationRuntimeSampleResult sample_result{};
    const AnimationRuntimeSampler sampler;
    stage.animation_sample_status = sampler.Sample(
        sample_request,
        std::span<AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
        &sample_result);
    stage.animation_sampled_value_count = static_cast<std::uint32_t>(sample_result.sampled_value_count);
    if (stage.animation_sample_status != AnimationRuntimeStatus::Success) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    AnimationRuntimeTransformApplyResult apply_result{};
    stage.animation_apply_status = sampler.ApplySampledTransform(
        AnimationRuntimeTransformApplyRequest{&bridge, std::span<const AnimationRuntimeSampledValue>(
            sampled_values.data(),
            sample_result.sampled_value_count)},
        &apply_result);
    if (stage.animation_apply_status != AnimationRuntimeStatus::Success) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    for (std::uint32_t index = 0U; index < RUNTIME_ASSET_SCENE_ENTITY_COUNT; ++index) {
        const WorldTransformResult transform_result =
            bridge.Query(stage.entities[index].world_object_id);
        if (transform_result.status != WorldTransformStatus::Success) {
            return RuntimeAssetDataStatus::InvalidDependency;
        }

        stage.entities[index].transform = transform_result.transform_state;
        stage.transforms[index].world_object_id = stage.entities[index].world_object_id;
        stage.transforms[index].transform = transform_result.transform_state;
    }
    stage.transform_count = RUNTIME_ASSET_SCENE_TRANSFORM_COUNT;

    *out_stage = stage;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus CommitSceneLoaderOutput(
    const RuntimeAssetGraphLoadRequest &request,
    const RuntimeAssetGraphLoadResult &load_result,
    const RuntimeAssetSceneLoaderStage &stage) {
    if (request.scene_output == nullptr) {
        return RuntimeAssetDataStatus::Success;
    }

    RuntimeAssetSceneLoaderOutput output{};
    output.scene_id = request.scene_stable_id;
    output.scene_hash = load_result.scene.hash;
    output.entity_count = stage.entity_count;
    output.transform_count = stage.transform_count;
    output.resource_ref_count = request.file_count;
    output.camera_count = stage.camera_count;
    output.animation_sampled_value_count = stage.animation_sampled_value_count;
    output.entity_capacity = request.scene_entity_capacity;
    output.transform_capacity = request.scene_transform_capacity;
    output.resource_ref_capacity = request.scene_resource_ref_capacity;
    output.camera_capacity = request.scene_camera_capacity;
    output.file_read_count = load_result.file_read_count;
    output.dependency_count = load_result.resource_dependency_count + load_result.asset_dependency_count;
    output.cache_payload_count = load_result.cache_payload_count;
    output.decoded_payload_count = load_result.decoded_payload_count;
    output.animation_sample_status = stage.animation_sample_status;
    output.animation_apply_status = stage.animation_apply_status;

    std::vector<RuntimeAssetSceneResourceRef> staged_resource_refs{};
    staged_resource_refs.resize(request.file_count);
    for (std::uint32_t index = 0U; index < request.file_count; ++index) {
        RuntimeAssetSceneResourceRef ref{};
        ref.kind = request.files[index].kind;
        ref.stable_id = request.files[index].stable_id;
        ref.loaded_file_index = index;
        ref.resource = request.loaded_files[index].resource;
        ref.asset = request.loaded_files[index].asset;
        staged_resource_refs[index] = ref;
    }

    for (std::uint32_t index = 0U; index < request.file_count; ++index) {
        request.scene_resource_refs[index] = staged_resource_refs[index];
    }

    for (std::uint32_t index = 0U; index < RUNTIME_ASSET_SCENE_CAMERA_COUNT; ++index) {
        request.scene_cameras[index] = stage.cameras[index];
    }

    for (std::uint32_t index = 0U; index < RUNTIME_ASSET_SCENE_ENTITY_COUNT; ++index) {
        request.scene_entities[index] = stage.entities[index];
    }

    for (std::uint32_t index = 0U; index < RUNTIME_ASSET_SCENE_TRANSFORM_COUNT; ++index) {
        request.scene_transforms[index] = stage.transforms[index];
    }

    output.status = RuntimeAssetDataStatus::Success;
    *request.scene_output = output;
    return RuntimeAssetDataStatus::Success;
}

struct RuntimeAssetGraphTransactionData final {
    RuntimeAssetLoadTransactionPlan plan;
    RuntimeAssetGraphLoadResult result;
    std::vector<std::uint8_t> scene_bytes;
    std::vector<std::vector<std::uint8_t>> file_bytes;
    std::string scene_text;
    std::string animation_text;
    RuntimeAssetSceneLoaderStage scene_stage;
};

void SetTransactionFailure(
    RuntimeAssetGraphTransactionData *transaction,
    RuntimeAssetDataStatus status,
    RuntimeAssetLoadTransactionPhase phase,
    std::uint32_t first_failed_record_index = 0U,
    std::uint32_t first_failed_dependency_index = 0U) {
    transaction->plan.status = status;
    transaction->plan.phase = phase;
    transaction->result.status = status;
    transaction->result.transaction_plan = transaction->plan;
    transaction->result.transaction_result.status = status;
    transaction->result.transaction_result.phase = phase;
    transaction->result.transaction_result.first_failed_record_index = first_failed_record_index;
    transaction->result.transaction_result.first_failed_dependency_index = first_failed_dependency_index;
}

void SetTransactionPhase(
    RuntimeAssetGraphTransactionData *transaction,
    RuntimeAssetLoadTransactionPhase phase) {
    transaction->plan.phase = phase;
    transaction->result.transaction_plan = transaction->plan;
    transaction->result.transaction_result.phase = phase;
}

void SetCommitPhase(
    RuntimeAssetGraphTransactionData *transaction,
    RuntimeAssetLoadTransactionPhase phase) {
    transaction->result.transaction_result.phase = phase;
}

bool HasDuplicateStableId(
    const RuntimeAssetGraphLoadRequest &request,
    std::uint64_t stable_id,
    std::uint32_t before_file_index) {
    if (stable_id == request.scene_stable_id) {
        return true;
    }

    for (std::uint32_t index = 0U; index < before_file_index; ++index) {
        if (request.files[index].stable_id == stable_id) {
            return true;
        }
    }

    return false;
}

RuntimeAssetDataStatus PreflightRuntimeAssetCommitIntents(
    const RuntimeAssetGraphLoadRequest &request,
    const RuntimeAssetGraphTransactionData &transaction) {
    const std::uint32_t record_count = request.file_count + 1U;
    if (record_count > yuengine::resource::MAX_RESOURCE_COUNT ||
        record_count > yuengine::asset::MAX_ASSET_COUNT) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    if ((request.file_count * 2U) > yuengine::resource::MAX_DEPENDENCY_EDGE_COUNT ||
        (request.file_count * 2U) > yuengine::asset::MAX_ASSET_DEPENDENCY_EDGE_COUNT) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    std::uint32_t cache_payload_bytes = static_cast<std::uint32_t>(transaction.scene_bytes.size());
    if (cache_payload_bytes > yuengine::resource::MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    std::uint32_t decoded_payload_bytes = 0U;
    for (std::uint32_t index = 0U; index < request.file_count; ++index) {
        const RuntimeAssetFileDesc &file = request.files[index];
        if (file.path == nullptr) {
            return RuntimeAssetDataStatus::InvalidArgument;
        }

        if (file.kind == RuntimeAssetFileKind::Unknown ||
            file.stable_id == 0U ||
            !file.resource_type.IsValid() ||
            !file.asset_type.IsValid()) {
            return RuntimeAssetDataStatus::InvalidArgument;
        }

        if (HasDuplicateStableId(request, file.stable_id, index)) {
            return RuntimeAssetDataStatus::DuplicateDependency;
        }

        const std::uint32_t source_byte_count =
            static_cast<std::uint32_t>(transaction.file_bytes[index].size());
        if (source_byte_count > yuengine::resource::MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD) {
            return RuntimeAssetDataStatus::CapacityExceeded;
        }

        cache_payload_bytes += source_byte_count;
        if (file.decode_asset_class != ResourceDecodePlanAssetClass::Unknown) {
            if (file.decode_result_class == ResourceDecodeResultClass::Unknown ||
                file.decoded_byte_count == 0U) {
                return RuntimeAssetDataStatus::InvalidArgument;
            }

            if (file.decoded_byte_count > yuengine::resource::MAX_RESOURCE_DECODED_PAYLOAD_BYTES_PER_RECORD) {
                return RuntimeAssetDataStatus::CapacityExceeded;
            }

            cache_payload_bytes += yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT;
            decoded_payload_bytes += file.decoded_byte_count;
        }
    }

    if (cache_payload_bytes > DEFAULT_PAYLOAD_BYTE_CAPACITY ||
        decoded_payload_bytes > yuengine::resource::MAX_RESOURCE_DECODED_PAYLOAD_TOTAL_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    return RuntimeAssetDataStatus::Success;
}

void PlanRuntimeAssetCommitIntents(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetGraphTransactionData *transaction) {
    transaction->plan.record_count = request.file_count + 1U;
    transaction->plan.resource_commit_count = request.file_count + 1U;
    transaction->plan.asset_commit_count = request.file_count + 1U;
    transaction->plan.cache_payload_commit_count = request.file_count + 1U;
    transaction->plan.dependency_edge_commit_count = request.file_count * 2U;
    for (std::uint32_t index = 0U; index < request.file_count; ++index) {
        if (request.files[index].decode_asset_class != ResourceDecodePlanAssetClass::Unknown) {
            ++transaction->plan.cache_payload_commit_count;
            ++transaction->plan.decoded_payload_commit_count;
        }
    }
}

RuntimeAssetDataStatus BuildRuntimeAssetGraphTransactionPlan(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetGraphTransactionData *transaction) {
    if (transaction == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    *transaction = RuntimeAssetGraphTransactionData{};
    SetTransactionPhase(transaction, RuntimeAssetLoadTransactionPhase::Preflight);
    RuntimeAssetDataStatus status = ValidateGraphRequest(request);
    if (status != RuntimeAssetDataStatus::Success) {
        SetTransactionFailure(transaction, status, RuntimeAssetLoadTransactionPhase::Preflight);
        return status;
    }

    PlanRuntimeAssetCommitIntents(request, transaction);
    SetTransactionPhase(transaction, RuntimeAssetLoadTransactionPhase::ReadBytes);
    status = ReadRuntimeAssetFile(request, request.scene_path, &transaction->scene_bytes);
    if (status != RuntimeAssetDataStatus::Success) {
        SetTransactionFailure(transaction, status, RuntimeAssetLoadTransactionPhase::ReadBytes);
        return status;
    }

    ++transaction->result.file_read_count;
    RuntimeAssetValidationResult scene_validation{};
    SetTransactionPhase(transaction, RuntimeAssetLoadTransactionPhase::ValidateRecord);
    status = ValidateRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(transaction->scene_bytes.data(), transaction->scene_bytes.size()),
        RuntimeAssetFileKind::Scene,
        &scene_validation);
    if (status != RuntimeAssetDataStatus::Success) {
        SetTransactionFailure(transaction, status, RuntimeAssetLoadTransactionPhase::ValidateRecord);
        return status;
    }

    transaction->scene_text = std::string(transaction->scene_bytes.begin(), transaction->scene_bytes.end());
    transaction->result.scene_dependency_count = scene_validation.dependency_count;
    transaction->plan.dependency_count = static_cast<std::uint32_t>(scene_validation.dependency_count);
    transaction->result.scene_references_runtime_asset_families =
        SceneReferencesRuntimeFamilies(transaction->scene_text);
    if (!transaction->result.scene_references_runtime_asset_families) {
        status = RuntimeAssetDataStatus::MissingDependency;
        SetTransactionFailure(transaction, status, RuntimeAssetLoadTransactionPhase::ValidateDependencies);
        return status;
    }

    transaction->file_bytes.resize(request.file_count);
    std::uint32_t file_index = 0U;
    while (file_index < request.file_count) {
        const RuntimeAssetFileDesc &file = request.files[file_index];
        if (file.path == nullptr) {
            status = RuntimeAssetDataStatus::InvalidArgument;
            SetTransactionFailure(
                transaction,
                status,
                RuntimeAssetLoadTransactionPhase::Preflight,
                file_index + 1U);
            return status;
        }

        SetTransactionPhase(transaction, RuntimeAssetLoadTransactionPhase::ReadBytes);
        const yuengine::file::VirtualPath path(file.path);
        status = ReadRuntimeAssetFile(request, path, &transaction->file_bytes[file_index]);
        if (status != RuntimeAssetDataStatus::Success) {
            SetTransactionFailure(
                transaction,
                status,
                RuntimeAssetLoadTransactionPhase::ReadBytes,
                file_index + 1U);
            return status;
        }

        ++transaction->result.file_read_count;
        RuntimeAssetValidationResult validation{};
        SetTransactionPhase(transaction, RuntimeAssetLoadTransactionPhase::ValidateRecord);
        status = ValidateRuntimeAssetDataBytes(
            std::span<const std::uint8_t>(
                transaction->file_bytes[file_index].data(),
                transaction->file_bytes[file_index].size()),
            file.kind,
            &validation);
        if (status != RuntimeAssetDataStatus::Success) {
            SetTransactionFailure(
                transaction,
                status,
                RuntimeAssetLoadTransactionPhase::ValidateRecord,
                file_index + 1U);
            return status;
        }

        if (file.kind == RuntimeAssetFileKind::Animation) {
            transaction->animation_text = std::string(
                transaction->file_bytes[file_index].begin(),
                transaction->file_bytes[file_index].end());
        }

        ++file_index;
    }

    if (request.scene_output != nullptr) {
        SetTransactionPhase(transaction, RuntimeAssetLoadTransactionPhase::StageSceneOutput);
        status = BuildSceneLoaderStage(
            request,
            transaction->scene_text,
            transaction->animation_text,
            &transaction->scene_stage);
        if (status != RuntimeAssetDataStatus::Success) {
            SetTransactionFailure(transaction, status, RuntimeAssetLoadTransactionPhase::StageSceneOutput);
            return status;
        }
    }

    SetTransactionPhase(transaction, RuntimeAssetLoadTransactionPhase::PreflightCommit);
    status = PreflightRuntimeAssetCommitIntents(request, *transaction);
    if (status != RuntimeAssetDataStatus::Success) {
        SetTransactionFailure(transaction, status, RuntimeAssetLoadTransactionPhase::PreflightCommit);
        return status;
    }

    transaction->plan.status = RuntimeAssetDataStatus::Success;
    transaction->plan.phase = RuntimeAssetLoadTransactionPhase::PreflightCommit;
    transaction->result.transaction_plan = transaction->plan;
    transaction->result.transaction_result.status = RuntimeAssetDataStatus::Success;
    transaction->result.transaction_result.phase = RuntimeAssetLoadTransactionPhase::PreflightCommit;
    return RuntimeAssetDataStatus::Success;
}

void RecordCommittedRuntimeAssetFile(
    const RuntimeAssetLoadedFile &file,
    RuntimeAssetGraphLoadResult *result) {
    ++result->transaction_result.committed_resource_count;
    ++result->transaction_result.committed_asset_count;
    ++result->transaction_result.committed_cache_payload_count;
    ++result->cache_payload_count;
    if (file.decode_plan_created) {
        ++result->transaction_result.committed_cache_payload_count;
        ++result->cache_payload_count;
    }

    if (file.decoded_payload_stored) {
        ++result->transaction_result.committed_decoded_payload_count;
        ++result->decoded_payload_count;
    }
}

void SetCommitFailure(
    RuntimeAssetGraphTransactionData *transaction,
    RuntimeAssetDataStatus status,
    RuntimeAssetLoadTransactionPhase phase) {
    transaction->result.status = status;
    transaction->result.transaction_result.status = status;
    transaction->result.transaction_result.phase = phase;
}

RuntimeAssetDataStatus CommitRuntimeAssetGraphTransaction(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetGraphTransactionData *transaction) {
    SetCommitPhase(transaction, RuntimeAssetLoadTransactionPhase::CommitResources);
    transaction->result.transaction_result.mutated_state = true;
    if (!ConfigureRuntimeAssetResourceBudgets(*request.resource_registry)) {
        SetCommitFailure(
            transaction,
            RuntimeAssetDataStatus::ResourceResidencyFailed,
            RuntimeAssetLoadTransactionPhase::CommitResources);
        return transaction->result.status;
    }

    RuntimeAssetFileDesc scene_desc{};
    scene_desc.path = request.scene_path.Value().data();
    scene_desc.kind = RuntimeAssetFileKind::Scene;
    scene_desc.resource_type = request.scene_resource_type;
    scene_desc.asset_type = request.scene_asset_type;
    scene_desc.stable_id = request.scene_stable_id;
    RuntimeAssetDataStatus status = RegisterLoadedFile(
        *request.resource_registry,
        *request.asset_manager,
        scene_desc,
        std::span<const std::uint8_t>(transaction->scene_bytes.data(), transaction->scene_bytes.size()),
        &transaction->result.scene);
    if (status != RuntimeAssetDataStatus::Success) {
        SetCommitFailure(transaction, status, RuntimeAssetLoadTransactionPhase::CommitResources);
        return status;
    }

    transaction->result.scene_registered = true;
    RecordCommittedRuntimeAssetFile(transaction->result.scene, &transaction->result);

    std::uint32_t file_index = 0U;
    while (file_index < request.file_count) {
        const RuntimeAssetFileDesc &file = request.files[file_index];
        status = RegisterLoadedFile(
            *request.resource_registry,
            *request.asset_manager,
            file,
            std::span<const std::uint8_t>(
                transaction->file_bytes[file_index].data(),
                transaction->file_bytes[file_index].size()),
            &request.loaded_files[file_index]);
        if (status != RuntimeAssetDataStatus::Success) {
            SetCommitFailure(transaction, status, RuntimeAssetLoadTransactionPhase::CommitResources);
            return status;
        }

        ++transaction->result.loaded_file_count;
        RecordCommittedRuntimeAssetFile(request.loaded_files[file_index], &transaction->result);
        ++file_index;
    }

    SetCommitPhase(transaction, RuntimeAssetLoadTransactionPhase::CommitDependencies);
    file_index = 0U;
    while (file_index < request.file_count) {
        status = AddLoadedDependency(
            *request.resource_registry,
            *request.asset_manager,
            transaction->result.scene,
            request.loaded_files[file_index]);
        if (status != RuntimeAssetDataStatus::Success) {
            SetCommitFailure(transaction, status, RuntimeAssetLoadTransactionPhase::CommitDependencies);
            return status;
        }

        ++transaction->result.resource_dependency_count;
        ++transaction->result.asset_dependency_count;
        transaction->result.transaction_result.committed_dependency_edge_count += 2U;
        ++file_index;
    }

    SetCommitPhase(transaction, RuntimeAssetLoadTransactionPhase::CommitSceneOutput);
    status = CommitSceneLoaderOutput(request, transaction->result, transaction->scene_stage);
    if (status != RuntimeAssetDataStatus::Success) {
        SetCommitFailure(transaction, status, RuntimeAssetLoadTransactionPhase::CommitSceneOutput);
        return status;
    }

    transaction->result.status = RuntimeAssetDataStatus::Success;
    transaction->result.transaction_result.status = RuntimeAssetDataStatus::Success;
    transaction->result.transaction_result.phase = RuntimeAssetLoadTransactionPhase::CommitSceneOutput;
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
    result.status = ValidateCommonMetadata(text, expected_kind, &result);
    if (result.status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return result.status;
    }

    if (expected_kind == RuntimeAssetFileKind::Mesh) {
        result.status = ValidateMeshMetadata(text, &result);
        *out_result = result;
        return result.status;
    }

    if (expected_kind == RuntimeAssetFileKind::Material) {
        result.status = ValidateMaterialMetadata(text, &result);
        *out_result = result;
        return result.status;
    }

    if (expected_kind == RuntimeAssetFileKind::Texture) {
        result.status = ValidateTextureMetadata(text, &result);
        *out_result = result;
        return result.status;
    }

    if (expected_kind == RuntimeAssetFileKind::Scene) {
        result.status = ValidateSceneDependencies(text, &result);
        *out_result = result;
        return result.status;
    }

    if (expected_kind == RuntimeAssetFileKind::Shader) {
        result.status = ValidateShaderProgramMetadata(text, &result);
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

RuntimeAssetDataStatus DecodeRuntimeAssetShaderProgramData(
    std::span<const std::uint8_t> bytes,
    std::uint32_t program_id,
    RuntimeAssetLoadedShaderProgramData *out_data) {
    if (out_data == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetLoadedShaderProgramData data{};
    data.program_id = program_id;
    if (program_id == 0U) {
        data.status = RuntimeAssetDataStatus::InvalidArgument;
        *out_data = data;
        return data.status;
    }

    RuntimeAssetValidationResult validation{};
    RuntimeAssetDataStatus status =
        ValidateRuntimeAssetDataBytes(bytes, RuntimeAssetFileKind::Shader, &validation);
    data.validation = validation;
    if (status != RuntimeAssetDataStatus::Success) {
        data.status = status;
        *out_data = data;
        return data.status;
    }

    const std::string text(bytes.begin(), bytes.end());
    status = CopyShaderBytecode(
        text,
        "stage_vs=",
        &data.vertex_bytecode,
        &data.vertex_bytecode_size,
        &data.vertex_bytecode_hash);
    if (status != RuntimeAssetDataStatus::Success) {
        data.status = status;
        *out_data = data;
        return data.status;
    }

    status = CopyShaderBytecode(
        text,
        "stage_ps=",
        &data.pixel_bytecode,
        &data.pixel_bytecode_size,
        &data.pixel_bytecode_hash);
    if (status != RuntimeAssetDataStatus::Success) {
        data.status = status;
        *out_data = data;
        return data.status;
    }

    status = ValidateOptionalShaderHash(text, "stage_vs_hash=", data.vertex_bytecode_hash);
    if (status != RuntimeAssetDataStatus::Success) {
        data.status = status;
        *out_data = data;
        return data.status;
    }

    status = ValidateOptionalShaderHash(text, "stage_ps_hash=", data.pixel_bytecode_hash);
    if (status != RuntimeAssetDataStatus::Success) {
        data.status = status;
        *out_data = data;
        return data.status;
    }

    status = DecodeShaderInputLayout(ValueForToken(text, "input="), &data.input_layout);
    if (status != RuntimeAssetDataStatus::Success) {
        data.status = status;
        *out_data = data;
        return data.status;
    }

    data.texture_slot_count = validation.texture_slot_count;
    data.status = RuntimeAssetDataStatus::Success;
    *out_data = data;
    return data.status;
}

RuntimeAssetDataStatus LoadRuntimeAssetDataGraph(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetGraphLoadResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetGraphTransactionData transaction{};
    RuntimeAssetDataStatus status = BuildRuntimeAssetGraphTransactionPlan(request, &transaction);
    if (status != RuntimeAssetDataStatus::Success) {
        *out_result = transaction.result;
        return status;
    }

    status = CommitRuntimeAssetGraphTransaction(request, &transaction);
    *out_result = transaction.result;
    return status;
}

RuntimeAssetDataStatus BuildRuntimeAssetShaderProgramPipeline(
    const RuntimeAssetShaderProgramPipelineRequest &request,
    RuntimeAssetShaderProgramPipelineResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetShaderProgramPipelineResult result{};
    if (request.program != nullptr) {
        result.program_id = request.program->program_id;
        result.texture_slot_count = request.program->texture_slot_count;
        result.vertex_bytecode_hash = request.program->vertex_bytecode_hash;
        result.pixel_bytecode_hash = request.program->pixel_bytecode_hash;
    }

    result.status = ValidateShaderProgramPipelineRequest(request);
    if (result.status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return result.status;
    }

    const RuntimeAssetLoadedShaderProgramData &program = *request.program;
    const std::span<const std::uint8_t> vertex_bytecode = VertexShaderBytecodeSpan(program);
    const std::span<const std::uint8_t> pixel_bytecode = PixelShaderBytecodeSpan(program);

    yuengine::rhi::RhiShaderModuleDesc vertex_desc{};
    vertex_desc.stage = yuengine::rhi::RhiShaderStage::Vertex;
    vertex_desc.bytecode = vertex_bytecode;
    const yuengine::rhi::RhiStatus vertex_status =
        request.device->CreateShaderModule(vertex_desc, result.vertex_shader);
    if (vertex_status != yuengine::rhi::RhiStatus::Success) {
        result.status = RuntimeAssetDataStatus::RhiShaderModuleFailed;
        *out_result = result;
        return result.status;
    }

    yuengine::rhi::RhiShaderModuleDesc pixel_desc{};
    pixel_desc.stage = yuengine::rhi::RhiShaderStage::Pixel;
    pixel_desc.bytecode = pixel_bytecode;
    const yuengine::rhi::RhiStatus pixel_status =
        request.device->CreateShaderModule(pixel_desc, result.pixel_shader);
    if (pixel_status != yuengine::rhi::RhiStatus::Success) {
        request.device->DestroyShaderModule(result.vertex_shader);
        result.vertex_shader = {};
        result.status = RuntimeAssetDataStatus::RhiShaderModuleFailed;
        *out_result = result;
        return result.status;
    }

    yuengine::rhi::RhiPipelineDesc pipeline_desc{};
    pipeline_desc.vertex_shader = result.vertex_shader;
    pipeline_desc.pixel_shader = result.pixel_shader;
    pipeline_desc.input_layout = program.input_layout;
    const yuengine::rhi::RhiStatus pipeline_status =
        request.device->CreatePipeline(pipeline_desc, result.pipeline);
    if (pipeline_status != yuengine::rhi::RhiStatus::Success) {
        request.device->DestroyShaderModule(result.pixel_shader);
        request.device->DestroyShaderModule(result.vertex_shader);
        result.pixel_shader = {};
        result.vertex_shader = {};
        result.status = RuntimeAssetDataStatus::RhiPipelineFailed;
        *out_result = result;
        return result.status;
    }

    result.pipeline_desc = pipeline_desc;
    result.status = RuntimeAssetDataStatus::Success;
    *out_result = result;
    return result.status;
}

}
