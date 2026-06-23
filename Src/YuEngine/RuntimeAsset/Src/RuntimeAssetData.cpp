// 模块: YuEngine RuntimeAsset
// 文件: Src/YuEngine/RuntimeAsset/Src/RuntimeAssetData.cpp

#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/Asset/AssetConstants.h"
#include "YuEngine/Asset/AssetDescriptor.h"
#include "YuEngine/Asset/AssetManager.h"
#include "YuEngine/Asset/AssetRecord.h"
#include "YuEngine/File/FileReadRequest.h"
#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/FileWriteRequest.h"
#include "YuEngine/File/FileWriteResult.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingRequest.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingResult.h"
#include "YuEngine/RenderScene/RenderSceneCameraFrameBinder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialConstants.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRequest.h"
#include "YuEngine/Resource/ResourceCachePayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceCachePayloadRequest.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRecord.h"
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
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferUsage.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiFormat.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiShaderStage.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridge.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeRequest.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeResult.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"
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
using yuengine::asset::AssetRecord;
using yuengine::asset::AssetRegistrationResult;
using yuengine::asset::AssetSnapshot;
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
using yuengine::file::FileWriteRequest;
using yuengine::file::FileWriteResult;
using yuengine::file::MountTable;
using yuengine::resource::ResourceCachePayloadRequest;
using yuengine::resource::ResourceCachePayloadSnapshot;
using yuengine::resource::ResourceCachePayloadStatus;
using yuengine::resource::ResourceDecodedPayloadRequest;
using yuengine::resource::ResourceDecodedPayloadSnapshot;
using yuengine::resource::ResourceDecodedPayloadStatus;
using yuengine::resource::ResourceDecodePlanAssetClass;
using yuengine::resource::ResourceDecodePlanRequest;
using yuengine::resource::ResourceDecodePlanSnapshot;
using yuengine::resource::ResourceDecodePlanStatus;
using yuengine::resource::ResourceDecodeResultClass;
using yuengine::resource::ResourceDecodeResultRequest;
using yuengine::resource::ResourceDecodeResultSnapshot;
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
using yuengine::resource::ResourceSnapshot;
using yuengine::resource::ResourceStatus;
using yuengine::resource::ResourceTypeId;
using yuengine::renderscene::RenderSceneCameraBindingRequest;
using yuengine::renderscene::RenderSceneCameraBindingResult;
using yuengine::renderscene::RenderSceneCameraFrameBinder;
using yuengine::renderscene::RenderScenePrimitiveGeometryBuilder;
using yuengine::renderscene::RenderScenePrimitiveGeometryKind;
using yuengine::renderscene::RenderScenePrimitiveGeometryRecord;
using yuengine::renderscene::RenderScenePrimitiveGeometryRequest;
using yuengine::renderscene::RenderScenePrimitiveGeometryStatus;
using yuengine::renderscene::RenderSceneRuntimeCameraRecord;
using yuengine::renderscene::RenderSceneRuntimeMaterialBuilder;
using yuengine::renderscene::RenderSceneRuntimeMaterialRecord;
using yuengine::renderscene::RenderSceneRuntimeMaterialRequest;
using yuengine::renderscene::RenderSceneRuntimeMaterialStatus;
using yuengine::renderscene::RenderSceneRuntimeMaterialTextureSlot;
using yuengine::renderscene::RenderSceneStatus;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureMissingLayer;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureRequest;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureResult;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureRoute;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureStatus;
using yuengine::renderscene::RenderSceneThreePrimitiveEntityRequest;
using yuengine::rhi::IRhiDevice;
using yuengine::rhi::RhiBufferDesc;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiColorTargetDesc;
using yuengine::rhi::RhiFormat;
using yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiSamplerBinding;
using yuengine::rhi::RhiSamplerDesc;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::RhiTextureHandle;
using yuengine::rhi::RhiVertexBufferView;
using yuengine::streaming::ResourceDecodedTextureBridge;
using yuengine::streaming::ResourceDecodedTextureBridgeRequest;
using yuengine::streaming::ResourceDecodedTextureBridgeResult;
using yuengine::streaming::ResourceDecodedTextureBridgeStatus;
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
constexpr std::uint32_t DEFAULT_PAYLOAD_BYTE_CAPACITY = 8192U;
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
constexpr std::uint32_t RUNTIME_ASSET_MAX_SCENE_ENTITY_COUNT = 64U;
constexpr std::uint32_t RUNTIME_ASSET_MAX_SCENE_CAMERA_COUNT = 4U;
constexpr std::uint32_t RUNTIME_ASSET_MAX_ANIMATION_CLIP_COUNT = 8U;
constexpr std::uint32_t RUNTIME_ASSET_MAX_ANIMATION_TRACK_COUNT = 128U;
constexpr std::uint32_t RUNTIME_ASSET_MAX_ANIMATION_KEYFRAME_COUNT = 512U;
constexpr std::uint32_t RUNTIME_ASSET_MAX_ANIMATION_SAMPLED_VALUE_COUNT = 128U;
constexpr std::uint32_t RUNTIME_ASSET_VISUAL_PROOF_MATERIAL_ID = 4101U;
constexpr std::uint32_t RUNTIME_ASSET_VISUAL_PROOF_PASS_ID = 7201U;
constexpr std::uint32_t RUNTIME_ASSET_VISUAL_PROOF_FIRST_DRAW_ID = 7301U;
constexpr std::uint32_t RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT = 3U;
constexpr std::size_t RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT = 3U;
constexpr std::uint32_t RUNTIME_ASSET_VISUAL_PROOF_TARGET_WIDTH = 2U;
constexpr std::uint32_t RUNTIME_ASSET_VISUAL_PROOF_TARGET_HEIGHT = 2U;

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

    if (!std::isfinite(parsed)) {
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

std::string_view RecordMainValue(std::string_view record) {
    const std::size_t separator = record.find('|');
    if (separator == std::string_view::npos) {
        return record;
    }

    return record.substr(0U, separator);
}

std::string_view FieldValue(std::string_view record, std::string_view field) {
    std::size_t offset = 0U;
    while (offset < record.size()) {
        const std::size_t segment_end = record.find('|', offset);
        const std::size_t length =
            segment_end == std::string_view::npos ? record.size() - offset : segment_end - offset;
        const std::string_view segment = record.substr(offset, length);
        if (StartsWith(segment, field)) {
            return segment.substr(field.size());
        }

        if (segment_end == std::string_view::npos) {
            return {};
        }

        offset = segment_end + 1U;
    }

    return {};
}

bool ParseOptionalU32Field(
    std::string_view record,
    std::string_view field,
    std::uint32_t default_value,
    std::uint32_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    const std::string_view value = FieldValue(record, field);
    if (value.empty()) {
        *out_value = default_value;
        return true;
    }

    return ParseU32(value, out_value);
}

bool ParseOptionalBoolField(
    std::string_view record,
    std::string_view field,
    bool default_value,
    bool *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    const std::string_view value = FieldValue(record, field);
    if (value.empty()) {
        *out_value = default_value;
        return true;
    }

    if (value == "1" || value == "true") {
        *out_value = true;
        return true;
    }

    if (value == "0" || value == "false") {
        *out_value = false;
        return true;
    }

    return false;
}

RuntimeAssetDataStatus ParseDeclaredCount(
    std::string_view text,
    std::string_view token,
    std::uint32_t default_value,
    std::uint32_t max_value,
    std::uint32_t *out_value,
    bool *out_declared) {
    if (out_value == nullptr || out_declared == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const std::string_view value = ValueForToken(text, token);
    if (value.empty()) {
        *out_value = default_value;
        *out_declared = false;
        return RuntimeAssetDataStatus::Success;
    }

    std::uint32_t parsed = 0U;
    if (!ParseU32(value, &parsed) || parsed == 0U) {
        return RuntimeAssetDataStatus::InvalidCount;
    }

    if (parsed > max_value) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    *out_value = parsed;
    *out_declared = true;
    return RuntimeAssetDataStatus::Success;
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

struct RuntimeAssetSceneEntityStageRow final {
    RuntimeAssetSceneEntityRecord entity{};
    std::uint32_t sort_key = 0U;
};

struct RuntimeAssetGraphRequestCounts final {
    std::uint32_t record_count = 0U;
    std::uint32_t dependency_edge_count = 0U;
};

struct RuntimeAssetSceneLoaderStage final {
    std::array<RuntimeAssetSceneCameraRecord, RUNTIME_ASSET_MAX_SCENE_CAMERA_COUNT> cameras{};
    std::array<RuntimeAssetSceneEntityRecord, RUNTIME_ASSET_MAX_SCENE_ENTITY_COUNT> entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, RUNTIME_ASSET_MAX_SCENE_ENTITY_COUNT> transforms{};
    std::uint32_t camera_count = 0U;
    std::uint32_t entity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t animation_sampled_value_count = 0U;
    AnimationRuntimeStatus animation_sample_status = AnimationRuntimeStatus::MissingClip;
    AnimationRuntimeStatus animation_apply_status = AnimationRuntimeStatus::MissingSample;
};

bool SceneStageHasWorldObject(
    const RuntimeAssetSceneLoaderStage &stage,
    WorldObjectId world_object_id) {
    for (std::uint32_t index = 0U; index < stage.entity_count; ++index) {
        if (stage.entities[index].world_object_id.value == world_object_id.value) {
            return true;
        }
    }

    return false;
}

RuntimeAssetDataStatus ResolveSceneEntityRef(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetFileKind kind,
    std::uint32_t ordinal,
    std::uint32_t *out_index) {
    if (!FindRefIndex(request.files, request.file_count, kind, ordinal, out_index)) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ParseSceneCameraRecord(
    std::string_view value,
    RuntimeAssetSceneCameraRecord *out_record) {
    if (out_record == nullptr || value.empty()) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const std::size_t separator = value.find(':');
    if (separator == std::string_view::npos) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    std::uint32_t camera_id = 0U;
    if (!ParseU32(value.substr(0U, separator), &camera_id) || camera_id == 0U) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    const std::string_view active_value = value.substr(separator + 1U);
    RuntimeAssetSceneCameraRecord record{};
    record.camera_id = camera_id;
    if (active_value == "active" || active_value == "1" || active_value == "true") {
        record.is_active = true;
        *out_record = record;
        return RuntimeAssetDataStatus::Success;
    }

    if (active_value == "inactive" || active_value == "0" || active_value == "false") {
        record.is_active = false;
        *out_record = record;
        return RuntimeAssetDataStatus::Success;
    }

    return RuntimeAssetDataStatus::InvalidDependency;
}

RuntimeAssetDataStatus ParseSceneCameras(
    std::string_view scene_text,
    std::uint32_t camera_count,
    bool camera_count_declared,
    RuntimeAssetSceneLoaderStage *stage) {
    if (stage == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (!camera_count_declared) {
        RuntimeAssetSceneCameraRecord camera{};
        camera.camera_id = 1U;
        camera.is_active = true;
        stage->cameras[0U] = camera;
        stage->camera_count = 1U;
        return RuntimeAssetDataStatus::Success;
    }

    std::uint32_t active_count = 0U;
    for (std::uint32_t index = 0U; index < camera_count; ++index) {
        const std::string token = "camera" + std::to_string(index) + "=";
        RuntimeAssetSceneCameraRecord camera{};
        const RuntimeAssetDataStatus status =
            ParseSceneCameraRecord(ValueForToken(scene_text, token), &camera);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }

        if (camera.is_active) {
            ++active_count;
        }

        stage->cameras[index] = camera;
    }

    if (active_count == 0U) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (active_count > 1U) {
        return RuntimeAssetDataStatus::DuplicateDependency;
    }

    stage->camera_count = camera_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ParseSceneEntityStageRow(
    const RuntimeAssetGraphLoadRequest &request,
    std::string_view value,
    std::uint32_t entity_index,
    std::uint32_t camera_count,
    bool bounded_record,
    RuntimeAssetSceneEntityStageRow *out_row) {
    if (out_row == nullptr || value.empty()) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const std::uint32_t default_mesh_ordinal =
        entity_index < RUNTIME_ASSET_SCENE_ENTITY_COUNT ? entity_index : 0U;
    std::uint32_t mesh_ordinal = 0U;
    std::uint32_t material_ordinal = 0U;
    std::uint32_t texture_ordinal = 0U;
    std::uint32_t shader_ordinal = 0U;
    std::uint32_t camera_index = 0U;
    std::uint32_t animation_ordinal = 0U;
    std::uint32_t sort_key = entity_index;
    bool is_visible = true;
    bool is_active = true;
    if (!ParseOptionalU32Field(value, "mesh_ref=", default_mesh_ordinal, &mesh_ordinal) ||
        !ParseOptionalU32Field(value, "material_ref=", 0U, &material_ordinal) ||
        !ParseOptionalU32Field(value, "texture_ref=", 0U, &texture_ordinal) ||
        !ParseOptionalU32Field(value, "shader_ref=", 0U, &shader_ordinal) ||
        !ParseOptionalU32Field(value, "camera=", 0U, &camera_index) ||
        !ParseOptionalU32Field(value, "animation_ref=", 0U, &animation_ordinal) ||
        !ParseOptionalU32Field(value, "sort=", entity_index, &sort_key) ||
        !ParseOptionalBoolField(value, "visible=", true, &is_visible) ||
        !ParseOptionalBoolField(value, "active=", true, &is_active)) {
        return bounded_record ? RuntimeAssetDataStatus::InvalidBounds :
            RuntimeAssetDataStatus::InvalidDependency;
    }

    if (camera_index >= camera_count) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    std::uint32_t mesh_ref = 0U;
    std::uint32_t material_ref = 0U;
    std::uint32_t texture_ref = 0U;
    std::uint32_t shader_ref = 0U;
    std::uint32_t animation_ref = 0U;
    RuntimeAssetDataStatus status =
        ResolveSceneEntityRef(request, RuntimeAssetFileKind::Mesh, mesh_ordinal, &mesh_ref);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ResolveSceneEntityRef(request, RuntimeAssetFileKind::Material, material_ordinal, &material_ref);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ResolveSceneEntityRef(request, RuntimeAssetFileKind::Texture, texture_ordinal, &texture_ref);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ResolveSceneEntityRef(request, RuntimeAssetFileKind::Shader, shader_ordinal, &shader_ref);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ResolveSceneEntityRef(request, RuntimeAssetFileKind::Animation, animation_ordinal, &animation_ref);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    RuntimeAssetSceneEntityRecord entity{};
    if (!ParseSceneEntityValue(
            RecordMainValue(value),
            entity_index + 1U,
            mesh_ref,
            material_ref,
            texture_ref,
            shader_ref,
            camera_index,
            animation_ref,
            &entity)) {
        return bounded_record ? RuntimeAssetDataStatus::InvalidBounds :
            RuntimeAssetDataStatus::InvalidDependency;
    }

    entity.is_visible = is_visible;
    entity.is_active = is_active;
    out_row->entity = entity;
    out_row->sort_key = sort_key;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ParseSceneEntities(
    const RuntimeAssetGraphLoadRequest &request,
    std::string_view scene_text,
    std::uint32_t entity_count,
    bool entity_count_declared,
    RuntimeAssetSceneLoaderStage *stage) {
    if (stage == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    std::array<RuntimeAssetSceneEntityStageRow, RUNTIME_ASSET_MAX_SCENE_ENTITY_COUNT> rows{};
    for (std::uint32_t index = 0U; index < entity_count; ++index) {
        const std::string token = "e" + std::to_string(index) + "=";
        const RuntimeAssetDataStatus status = ParseSceneEntityStageRow(
            request,
            ValueForToken(scene_text, token),
            index,
            stage->camera_count,
            entity_count_declared,
            &rows[index]);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }
    }

    for (std::uint32_t index = 0U; index < entity_count; ++index) {
        for (std::uint32_t other = index + 1U; other < entity_count; ++other) {
            if (rows[index].entity.world_object_id.value == rows[other].entity.world_object_id.value) {
                return RuntimeAssetDataStatus::DuplicateDependency;
            }
        }
    }

    std::sort(
        rows.begin(),
        rows.begin() + entity_count,
        [](const RuntimeAssetSceneEntityStageRow &left, const RuntimeAssetSceneEntityStageRow &right) {
            if (left.sort_key != right.sort_key) {
                return left.sort_key < right.sort_key;
            }

            return left.entity.world_object_id.value < right.entity.world_object_id.value;
        });

    for (std::uint32_t index = 0U; index < entity_count; ++index) {
        stage->entities[index] = rows[index].entity;
    }

    stage->entity_count = entity_count;
    return RuntimeAssetDataStatus::Success;
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

struct RuntimeAssetHeaderParseResult final {
    RuntimeAssetArtifactClass artifact_class = RuntimeAssetArtifactClass::Unknown;
    std::uint32_t version = 0U;
    bool has_runtime_magic = false;
    bool kind_matches_expected = false;
    bool version_parsed = false;
    bool exact_token_count = false;
};

bool IsHeaderWhitespace(char character) {
    return character == ' ' || character == '\t';
}

std::string_view FirstHeaderLine(std::string_view text) {
    const std::size_t line_end = text.find('\n');
    std::string_view line =
        line_end == std::string_view::npos ? text : text.substr(0U, line_end);
    if (!line.empty() && line.back() == '\r') {
        line.remove_suffix(1U);
    }

    return line;
}

bool ReadHeaderToken(
    std::string_view line,
    std::size_t *in_out_offset,
    std::string_view *out_token) {
    if (in_out_offset == nullptr || out_token == nullptr) {
        return false;
    }

    std::size_t offset = *in_out_offset;
    while (offset < line.size() && IsHeaderWhitespace(line[offset])) {
        ++offset;
    }

    if (offset >= line.size()) {
        return false;
    }

    const std::size_t token_start = offset;
    while (offset < line.size() && !IsHeaderWhitespace(line[offset])) {
        ++offset;
    }

    *in_out_offset = offset;
    *out_token = line.substr(token_start, offset - token_start);
    return true;
}

bool HeaderHasRemainingToken(std::string_view line, std::size_t offset) {
    while (offset < line.size()) {
        if (!IsHeaderWhitespace(line[offset])) {
            return true;
        }

        ++offset;
    }

    return false;
}

RuntimeAssetHeaderParseResult ParseRuntimeAssetHeader(
    std::string_view text,
    RuntimeAssetFileKind expected_kind) {
    RuntimeAssetHeaderParseResult result{};
    const std::string_view line = FirstHeaderLine(text);
    std::size_t offset = 0U;
    std::string_view magic{};
    std::string_view kind{};
    std::string_view version{};
    if (!ReadHeaderToken(line, &offset, &magic)) {
        return result;
    }

    if (magic == "YUASSET") {
        result.artifact_class = RuntimeAssetArtifactClass::Source;
        result.has_runtime_magic = true;
    }

    if (magic == "YUCOOKED") {
        result.artifact_class = RuntimeAssetArtifactClass::Cooked;
        result.has_runtime_magic = true;
    }

    if (!result.has_runtime_magic) {
        return result;
    }

    if (!ReadHeaderToken(line, &offset, &kind)) {
        return result;
    }

    result.kind_matches_expected = kind == RuntimeAssetFileKindName(expected_kind);
    if (!ReadHeaderToken(line, &offset, &version)) {
        return result;
    }

    result.version_parsed = ParseU32(version, &result.version);
    result.exact_token_count = !HeaderHasRemainingToken(line, offset);
    return result;
}

std::uint64_t HashRuntimeAssetText(std::string_view text) {
    std::uint64_t hash = FNV_OFFSET;
    for (const char character : text) {
        hash ^= static_cast<std::uint64_t>(static_cast<std::uint8_t>(character));
        hash *= FNV_PRIME;
    }

    return hash;
}

constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_MESH = 101U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_MATERIAL = 102U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_TEXTURE = 103U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_SHADER = 104U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_SCENE = 105U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_ANIMATION = 106U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_MESH = 201U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_MATERIAL = 202U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_TEXTURE = 203U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_SHADER = 204U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_SCENE = 205U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_ANIMATION = 206U;

struct RuntimeAssetFixtureArtifact final {
    RuntimeAssetFileDesc desc{};
    std::string text{};
};

std::string RuntimeAssetFamilyNameLower(RuntimeAssetFileKind kind) {
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
    }

    return "unknown";
}

RuntimeAssetFileDesc RuntimeAssetFixtureDesc(
    const char *path,
    RuntimeAssetFileKind kind,
    std::uint64_t stable_id,
    std::uint32_t decoded_byte_count) {
    RuntimeAssetFileDesc desc{};
    desc.path = path;
    desc.kind = kind;
    desc.stable_id = stable_id;
    desc.decoded_byte_count = decoded_byte_count;
    switch (kind) {
        case RuntimeAssetFileKind::Mesh:
            desc.resource_type = ResourceTypeId{RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_MESH};
            desc.asset_type = yuengine::asset::AssetTypeId{RUNTIME_ASSET_FIXTURE_ASSET_TYPE_MESH};
            desc.decode_asset_class = ResourceDecodePlanAssetClass::Mesh;
            desc.decode_result_class = ResourceDecodeResultClass::Mesh;
            break;
        case RuntimeAssetFileKind::Material:
            desc.resource_type = ResourceTypeId{RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_MATERIAL};
            desc.asset_type = yuengine::asset::AssetTypeId{RUNTIME_ASSET_FIXTURE_ASSET_TYPE_MATERIAL};
            desc.decode_asset_class = ResourceDecodePlanAssetClass::Material;
            desc.decode_result_class = ResourceDecodeResultClass::Material;
            break;
        case RuntimeAssetFileKind::Texture:
            desc.resource_type = ResourceTypeId{RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_TEXTURE};
            desc.asset_type = yuengine::asset::AssetTypeId{RUNTIME_ASSET_FIXTURE_ASSET_TYPE_TEXTURE};
            desc.decode_asset_class = ResourceDecodePlanAssetClass::Texture;
            desc.decode_result_class = ResourceDecodeResultClass::Texture;
            break;
        case RuntimeAssetFileKind::Shader:
            desc.resource_type = ResourceTypeId{RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_SHADER};
            desc.asset_type = yuengine::asset::AssetTypeId{RUNTIME_ASSET_FIXTURE_ASSET_TYPE_SHADER};
            break;
        case RuntimeAssetFileKind::Scene:
            desc.resource_type = ResourceTypeId{RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_SCENE};
            desc.asset_type = yuengine::asset::AssetTypeId{RUNTIME_ASSET_FIXTURE_ASSET_TYPE_SCENE};
            break;
        case RuntimeAssetFileKind::Animation:
            desc.resource_type = ResourceTypeId{RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_ANIMATION};
            desc.asset_type = yuengine::asset::AssetTypeId{RUNTIME_ASSET_FIXTURE_ASSET_TYPE_ANIMATION};
            break;
        case RuntimeAssetFileKind::Unknown:
            break;
    }

    return desc;
}

std::array<RuntimeAssetFixtureArtifact, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT>
RuntimeAssetSourceFixtureArtifacts() {
    return std::array<RuntimeAssetFixtureArtifact, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT>{
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Mesh/Cube.yumesh", RuntimeAssetFileKind::Mesh, 1001U, 96U),
            "YUASSET MESH 1\nschema=rav0-source\nid=cube_mesh\nkind=cube\nvertices=24\nindices=36\nbounds=-1,-1,-1,1,1,1\n"},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Mesh/Cylinder.yumesh", RuntimeAssetFileKind::Mesh, 1002U, 96U),
            "YUASSET MESH 1\nschema=rav0-source\nid=cylinder_mesh\nkind=cylinder\nvertices=18\nindices=96\nbounds=-1,-1,-1,1,1,1\n"},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Mesh/Cone.yumesh", RuntimeAssetFileKind::Mesh, 1003U, 96U),
            "YUASSET MESH 1\nschema=rav0-source\nid=cone_mesh\nkind=cone\nvertices=10\nindices=48\nbounds=-1,-1,-1,1,1,1\n"},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Material/Shared.yumat", RuntimeAssetFileKind::Material, 2001U, 128U),
            "YUASSET MATERIAL 1\nschema=rav0-source\nid=shared_material\nshader=Shader/RuntimeProgram.yuprogram\ntexture0=Texture/Albedo.yutex\ntexture1=Texture/Normal.yutex\ntexture2=Texture/Mask.yutex\n"},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Texture/Albedo.yutex", RuntimeAssetFileKind::Texture, 3001U, 16U),
            "YUASSET TEXTURE 1\nschema=rav0-source\nid=albedo\nformat=rgba8\nextent=2x2\npayload=checker\n"},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Texture/Normal.yutex", RuntimeAssetFileKind::Texture, 3002U, 16U),
            "YUASSET TEXTURE 1\nschema=rav0-source\nid=normal\nformat=rgba8\nextent=2x2\npayload=normal\n"},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Texture/Mask.yutex", RuntimeAssetFileKind::Texture, 3003U, 16U),
            "YUASSET TEXTURE 1\nschema=rav0-source\nid=mask\nformat=rgba8\nextent=2x2\npayload=mask\n"},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Shader/RuntimeProgram.yuprogram", RuntimeAssetFileKind::Shader, 4001U, 0U),
            "YUASSET SHADER 1\nschema=rav0-source\nid=runtime_program\nstage_vs=bytecode:runtime_program_vs\nstage_ps=bytecode:runtime_program_ps\ninput=layout:position,color\ntextures=3\n"},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Animation/Spin.yuanim", RuntimeAssetFileKind::Animation, 5001U, 0U),
            "YUASSET ANIMATION 1\nschema=rav0-source\nid=spin\nclip=1\nduration=1\ntarget=scene_entity:101\ntrack=transform:rotation_y\nkey0=0:0\nkey1=1:1\ntracks=1\nsample_rate=30\n"}};
}

RuntimeAssetFixtureArtifact RuntimeAssetSourceSceneArtifact() {
    return RuntimeAssetFixtureArtifact{
        RuntimeAssetFixtureDesc("Scene/CanonicalScene.yuscene", RuntimeAssetFileKind::Scene, 6001U, 0U),
        "YUASSET SCENE 1\n"
        "schema=rav0-source\n"
        "id=scene\n"
        "m0=Mesh/Cube.yumesh\n"
        "m1=Mesh/Cylinder.yumesh\n"
        "m2=Mesh/Cone.yumesh\n"
        "mat=Material/Shared.yumat\n"
        "t0=Texture/Albedo.yutex\n"
        "prog=Shader/RuntimeProgram.yuprogram\n"
        "anim=Animation/Spin.yuanim\n"
        "cam=camera:orbit\n"
        "e0=101:-2,0,0\n"
        "e1=102:0,0,0\n"
        "e2=103:2,0,0\n"};
}

std::string RuntimeAssetCookedText(
    RuntimeAssetFileKind kind,
    std::string_view id,
    std::string_view payload,
    std::string_view family_fields,
    std::span<const std::string_view> dependency_rows,
    std::uint32_t record_byte_count,
    std::uint32_t payload_alignment) {
    std::string text("YUCOOKED ");
    text += RuntimeAssetFileKindName(kind);
    text += " 1\nschema=rav1-cooked\nid=";
    text += id;
    text += "\nkind=";
    text += RuntimeAssetFamilyNameLower(kind);
    text += "\nsourceHash=";
    text += std::to_string(HashRuntimeAssetText(id));
    text += "\npayloadHash=";
    text += std::to_string(HashRuntimeAssetText(payload));
    text += "\ndependencyTable=";
    text += std::to_string(dependency_rows.size());
    text += "\nrecordTable=1\nrecordBytes=";
    text += std::to_string(record_byte_count);
    text += "\npayloadBytes=";
    text += std::to_string(payload.size());
    text += "\npayloadAlign=";
    text += std::to_string(payload_alignment);
    text += "\n";
    for (std::size_t index = 0U; index < dependency_rows.size(); ++index) {
        text += "dep";
        text += std::to_string(index);
        text += "=";
        text += dependency_rows[index];
        text += "\n";
    }

    text += family_fields;
    text += "payload=";
    text += payload;
    text += "\n";
    return text;
}

std::array<RuntimeAssetFixtureArtifact, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT>
RuntimeAssetCookedFixtureArtifacts() {
    const std::array<std::string_view, 0U> no_deps{};
    const std::array<std::string_view, 4U> material_deps{{
        "shader:runtime_program:4001",
        "texture:albedo:3001",
        "texture:normal:3002",
        "texture:mask:3003",
    }};

    return std::array<RuntimeAssetFixtureArtifact, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT>{
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Mesh/Cube.racooked", RuntimeAssetFileKind::Mesh, 11001U, 96U),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Mesh,
                "cube_mesh_cooked",
                "mesh-cube-payload",
                "shape=cube\nvertices=24\nindices=36\nbounds=-1,-1,-1,1,1,1\n",
                std::span<const std::string_view>(no_deps.data(), no_deps.size()),
                96U,
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Mesh/Cylinder.racooked", RuntimeAssetFileKind::Mesh, 11002U, 96U),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Mesh,
                "cylinder_mesh_cooked",
                "mesh-cylinder-payload",
                "shape=cylinder\nvertices=18\nindices=96\nbounds=-1,-1,-1,1,1,1\n",
                std::span<const std::string_view>(no_deps.data(), no_deps.size()),
                96U,
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Mesh/Cone.racooked", RuntimeAssetFileKind::Mesh, 11003U, 96U),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Mesh,
                "cone_mesh_cooked",
                "mesh-cone-payload",
                "shape=cone\nvertices=10\nindices=48\nbounds=-1,-1,-1,1,1,1\n",
                std::span<const std::string_view>(no_deps.data(), no_deps.size()),
                96U,
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Material/Shared.racooked", RuntimeAssetFileKind::Material, 12001U, 128U),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Material,
                "shared_material_cooked",
                "material-shared-payload",
                "shader=Shader/RuntimeProgram.racooked\ntexture0=Texture/Albedo.racooked\ntexture1=Texture/Normal.racooked\ntexture2=Texture/Mask.racooked\n",
                std::span<const std::string_view>(material_deps.data(), material_deps.size()),
                128U,
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Texture/Albedo.racooked", RuntimeAssetFileKind::Texture, 13001U, 16U),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Texture,
                "albedo_cooked",
                "checker",
                "format=rgba8\nextent=2x2\n",
                std::span<const std::string_view>(no_deps.data(), no_deps.size()),
                64U,
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Texture/Normal.racooked", RuntimeAssetFileKind::Texture, 13002U, 16U),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Texture,
                "normal_cooked",
                "normal",
                "format=rgba8\nextent=2x2\n",
                std::span<const std::string_view>(no_deps.data(), no_deps.size()),
                64U,
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Texture/Mask.racooked", RuntimeAssetFileKind::Texture, 13003U, 16U),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Texture,
                "mask_cooked",
                "mask",
                "format=rgba8\nextent=2x2\n",
                std::span<const std::string_view>(no_deps.data(), no_deps.size()),
                64U,
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Shader/RuntimeProgram.racooked", RuntimeAssetFileKind::Shader, 14001U, 0U),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Shader,
                "runtime_program_cooked",
                "shader-program-payload",
                "stage_vs=bytecode:runtime_program_vs\nstage_ps=bytecode:runtime_program_ps\ninput=layout:position,color\ntextures=3\n",
                std::span<const std::string_view>(no_deps.data(), no_deps.size()),
                128U,
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Animation/Spin.racooked", RuntimeAssetFileKind::Animation, 15001U, 0U),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Animation,
                "spin_cooked",
                "animation-spin-payload",
                "target=scene_entity:101\ntrack=transform:rotation_y\nclip=1\nduration=1\nkey0=0:0\nkey1=1:1\ntracks=1\nsample_rate=30\n",
                std::span<const std::string_view>(no_deps.data(), no_deps.size()),
                128U,
                4U)}};
}

RuntimeAssetFixtureArtifact RuntimeAssetCookedSceneArtifact() {
    const std::array<std::string_view, 8U> scene_deps{{
        "mesh:cube_mesh_cooked:11001",
        "mesh:cylinder_mesh_cooked:11002",
        "mesh:cone_mesh_cooked:11003",
        "material:shared_material_cooked:12001",
        "texture:albedo_cooked:13001",
        "shader:runtime_program_cooked:14001",
        "animation:spin_cooked:15001",
        "camera:orbit:6001",
    }};

    return RuntimeAssetFixtureArtifact{
        RuntimeAssetFixtureDesc("Scene/CanonicalScene.racooked", RuntimeAssetFileKind::Scene, 16001U, 0U),
        RuntimeAssetCookedText(
            RuntimeAssetFileKind::Scene,
            "scene_cooked",
            "scene-runtime-payload",
            "m0=Mesh/Cube.racooked\n"
            "m1=Mesh/Cylinder.racooked\n"
            "m2=Mesh/Cone.racooked\n"
            "mat=Material/Shared.racooked\n"
            "t0=Texture/Albedo.racooked\n"
            "prog=Shader/RuntimeProgram.racooked\n"
            "anim=Animation/Spin.racooked\n"
            "cam=camera:orbit\n"
            "e0=101:-2,0,0\n"
            "e1=102:0,0,0\n"
            "e2=103:2,0,0\n",
            std::span<const std::string_view>(scene_deps.data(), scene_deps.size()),
            256U,
            4U)};
}

void AppendRuntimeAssetGraphHash(std::uint64_t *hash, std::string_view text) {
    if (hash == nullptr) {
        return;
    }

    for (const char character : text) {
        *hash ^= static_cast<std::uint64_t>(static_cast<std::uint8_t>(character));
        *hash *= FNV_PRIME;
    }
}

RuntimeAssetDataStatus WriteAndValidateFixtureArtifact(
    const RuntimeAssetDeterministicDiskFixtureRequest &request,
    const RuntimeAssetFixtureArtifact &artifact,
    std::uint32_t artifact_index,
    std::uint64_t *graph_hash,
    std::uint64_t *out_artifact_hash,
    RuntimeAssetDeterministicDiskFixtureResult *result) {
    if (result == nullptr || request.mount_table == nullptr || artifact.desc.path == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const std::span<const std::uint8_t> bytes(
        reinterpret_cast<const std::uint8_t *>(artifact.text.data()),
        artifact.text.size());
    FileWriteRequest write_request{};
    write_request.mount = request.mount;
    write_request.path = yuengine::file::VirtualPath(artifact.desc.path);
    write_request.bytes = bytes.data();
    write_request.byte_count = bytes.size();
    const FileWriteResult write_result = request.mount_table->Write(write_request);
    if (!write_result.Succeeded()) {
        result->status = RuntimeAssetDataStatus::FileWriteFailed;
        result->missing_layer = RuntimeAssetImportCookMissingLayer::FileVfs;
        result->file_status = write_result.status;
        result->first_failed_kind = artifact.desc.kind;
        result->first_failed_artifact_index = artifact_index;
        return result->status;
    }

    RuntimeAssetValidationResult validation{};
    const RuntimeAssetDataStatus validation_status =
        ValidateRuntimeAssetDataBytes(bytes, artifact.desc.kind, &validation);
    if (validation_status != RuntimeAssetDataStatus::Success) {
        result->status = validation_status;
        result->missing_layer = RuntimeAssetImportCookMissingLayer::RuntimeAssetData;
        result->validation_status = validation_status;
        result->first_failed_kind = artifact.desc.kind;
        result->first_failed_artifact_index = artifact_index;
        return validation_status;
    }

    ++result->validation_count;
    if (out_artifact_hash != nullptr) {
        *out_artifact_hash = validation.hash;
    }

    if (graph_hash != nullptr) {
        AppendRuntimeAssetGraphHash(graph_hash, artifact.desc.path);
        AppendRuntimeAssetGraphHash(graph_hash, "\n");
        AppendRuntimeAssetGraphHash(graph_hash, artifact.text);
        AppendRuntimeAssetGraphHash(graph_hash, "\n");
    }

    return RuntimeAssetDataStatus::Success;
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

RuntimeAssetDataStatus ValidateOptionalDeclaredHash(std::string_view text) {
    const std::string_view expected_hash_text = ValueForToken(text, "expected_hash=");
    if (expected_hash_text.empty()) {
        return RuntimeAssetDataStatus::Success;
    }

    std::uint64_t expected_hash = 0U;
    if (!ParseU64(expected_hash_text, &expected_hash)) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    if (expected_hash != HashRuntimeAssetText(text)) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    return RuntimeAssetDataStatus::Success;
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

    const RuntimeAssetHeaderParseResult header = ParseRuntimeAssetHeader(text, expected_kind);
    if (!header.has_runtime_magic) {
        return RuntimeAssetDataStatus::InvalidHeader;
    }

    out_result->artifact_class = header.artifact_class;
    if (!header.kind_matches_expected) {
        return RuntimeAssetDataStatus::InvalidKind;
    }

    if (!header.version_parsed || !header.exact_token_count) {
        return RuntimeAssetDataStatus::InvalidHeader;
    }

    out_result->version = header.version;
    if (header.version != 1U) {
        return RuntimeAssetDataStatus::UnsupportedVersion;
    }

    if (header.artifact_class == RuntimeAssetArtifactClass::Cooked) {
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

    RuntimeAssetMeshGeometryKind geometry_kind = RuntimeAssetMeshGeometryKind::Unknown;
    if (mesh_kind == "cube") {
        geometry_kind = RuntimeAssetMeshGeometryKind::Cube;
    }

    if (mesh_kind == "cylinder") {
        geometry_kind = RuntimeAssetMeshGeometryKind::Cylinder;
    }

    if (mesh_kind == "cone") {
        geometry_kind = RuntimeAssetMeshGeometryKind::Cone;
    }

    if (geometry_kind == RuntimeAssetMeshGeometryKind::Unknown) {
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
    out_result->mesh_geometry_kind = geometry_kind;
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

void CopyValidationMetadataToLoadedFile(
    const RuntimeAssetValidationResult &validation,
    RuntimeAssetLoadedFile *out_record) {
    if (out_record == nullptr) {
        return;
    }

    out_record->hash = validation.hash;
    out_record->artifact_class = validation.artifact_class;
    out_record->schema_version = validation.schema_version;
    out_record->identity_hash = validation.identity_hash;
    out_record->source_hash = validation.source_hash;
    out_record->payload_hash = validation.payload_hash;
    out_record->byte_count = static_cast<std::uint32_t>(validation.byte_count);
    out_record->record_table_count = validation.record_table_count;
    out_record->record_table_byte_count = validation.record_table_byte_count;
    out_record->payload_byte_count = validation.payload_byte_count;
    out_record->payload_alignment = validation.payload_alignment;
    out_record->vertex_count = validation.vertex_count;
    out_record->index_count = validation.index_count;
    out_record->mesh_geometry_kind = validation.mesh_geometry_kind;
    out_record->texture_width = validation.texture_width;
    out_record->texture_height = validation.texture_height;
    out_record->texture_slot_count = validation.texture_slot_count;
    out_record->shader_stage_count = validation.shader_stage_count;
    out_record->shader_bytecode_byte_count = validation.shader_bytecode_byte_count;
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

    RuntimeAssetValidationResult validation{};
    RuntimeAssetDataStatus status = ValidateRuntimeAssetDataBytes(bytes, desc.kind, &validation);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    RuntimeAssetLoadedFile record{};
    record.kind = desc.kind;
    record.resource_type = desc.resource_type;
    record.asset_type = desc.asset_type;
    record.stable_id = desc.stable_id;
    record.decode_asset_class = desc.decode_asset_class;
    record.decode_result_class = desc.decode_result_class;
    record.decoded_byte_count = desc.decoded_byte_count;
    CopyValidationMetadataToLoadedFile(validation, &record);

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

    status = StoreSourcePayload(
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

bool CountExceedsCapacity(
    std::uint32_t current_count,
    std::uint32_t additional_count,
    std::uint32_t capacity) {
    if (current_count > capacity) {
        return true;
    }

    return additional_count > (capacity - current_count);
}

RuntimeAssetDataStatus BuildGraphRequestCounts(
    std::uint32_t file_count,
    RuntimeAssetGraphRequestCounts *out_counts) {
    if (out_counts == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (file_count == 0U) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (file_count > (std::numeric_limits<std::uint32_t>::max() - 1U)) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    if (file_count > (std::numeric_limits<std::uint32_t>::max() / 2U)) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    RuntimeAssetGraphRequestCounts counts{};
    counts.record_count = file_count + 1U;
    counts.dependency_edge_count = file_count * 2U;
    *out_counts = counts;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateGraphRequestCounts(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetGraphRequestCounts *out_counts) {
    RuntimeAssetGraphRequestCounts counts{};
    RuntimeAssetDataStatus status = BuildGraphRequestCounts(request.file_count, &counts);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    if (request.file_count > request.loaded_file_capacity) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    if (counts.record_count > yuengine::resource::MAX_RESOURCE_COUNT ||
        counts.record_count > yuengine::resource::MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT ||
        counts.record_count > yuengine::asset::MAX_ASSET_COUNT) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    if (request.file_count > yuengine::resource::MAX_DEPENDENCY_EDGE_COUNT ||
        request.file_count > yuengine::asset::MAX_ASSET_DEPENDENCY_EDGE_COUNT) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    if (out_counts != nullptr) {
        *out_counts = counts;
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

    RuntimeAssetGraphRequestCounts counts{};
    const RuntimeAssetDataStatus count_status = ValidateGraphRequestCounts(request, &counts);
    if (count_status != RuntimeAssetDataStatus::Success) {
        return count_status;
    }

    if (request.scene_output != nullptr) {
        if (request.scene_resource_refs == nullptr ||
            request.scene_cameras == nullptr ||
            request.scene_entities == nullptr ||
            request.scene_transforms == nullptr) {
            return RuntimeAssetDataStatus::InvalidArgument;
        }

        if (request.scene_resource_ref_capacity < request.file_count ||
            request.scene_camera_capacity == 0U ||
            request.scene_entity_capacity == 0U ||
            request.scene_transform_capacity == 0U) {
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

bool IsCookedShaderPowerOfTwo(std::uint32_t value) {
    return value != 0U && (value & (value - 1U)) == 0U;
}

bool IsCStringEmpty(const char *value) {
    return value == nullptr || value[0U] == '\0';
}

bool IsCookedShaderStageProfile(
    yuengine::rhi::RhiShaderStage stage,
    const char *profile) {
    if (profile == nullptr) {
        return false;
    }

    const std::string_view text(profile);
    if (stage == yuengine::rhi::RhiShaderStage::Vertex) {
        return text == "vs_5_0";
    }

    if (stage == yuengine::rhi::RhiShaderStage::Pixel) {
        return text == "ps_5_0";
    }

    return false;
}

std::span<const std::uint8_t> CookedShaderStageBytecodeSpan(
    const RuntimeAssetCookedShaderStagePayloadDesc &stage) {
    return std::span<const std::uint8_t>(
        stage.payload_bytes + stage.bytecode_offset,
        stage.bytecode_byte_count);
}

RuntimeAssetDataStatus ValidateCookedShaderStagePayload(
    const RuntimeAssetCookedShaderStagePayloadDesc &stage) {
    if (stage.stage != yuengine::rhi::RhiShaderStage::Vertex &&
        stage.stage != yuengine::rhi::RhiShaderStage::Pixel) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (IsCStringEmpty(stage.entry_point)) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (IsCStringEmpty(stage.bytecode_profile)) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (!IsCookedShaderStageProfile(stage.stage, stage.bytecode_profile)) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (stage.bytecode_format != RuntimeAssetCookedShaderBytecodeFormat::OpaqueBytecode) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    if (stage.payload_id == 0U) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (stage.payload_bytes == nullptr || stage.payload_byte_count == 0U ||
        stage.bytecode_byte_count == 0U) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (stage.bytecode_offset > stage.payload_byte_count ||
        stage.bytecode_byte_count > stage.payload_byte_count - stage.bytecode_offset) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (!IsCookedShaderPowerOfTwo(stage.bytecode_alignment) ||
        (stage.bytecode_offset % stage.bytecode_alignment) != 0U) {
        return RuntimeAssetDataStatus::InvalidAlignment;
    }

    if (stage.bytecode_byte_count > yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    const std::span<const std::uint8_t> bytecode = CookedShaderStageBytecodeSpan(stage);
    const std::uint64_t bytecode_hash = HashRuntimeAssetDataBytes(bytecode);
    if (bytecode_hash != stage.bytecode_hash) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    if (stage.expected_stage_hash == 0U || stage.expected_stage_hash != bytecode_hash) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateCookedInputElement(
    const yuengine::rhi::RhiInputElementDesc &element,
    std::size_t stride_bytes) {
    if (element.semantic == yuengine::rhi::RhiInputElementSemantic::Unsupported) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    const std::size_t element_size = GetInputElementByteSize(element.format);
    if (element_size == 0U) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    if (stride_bytes < element_size) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    const std::size_t max_offset = stride_bytes - element_size;
    if (element.offset_bytes > max_offset) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    return RuntimeAssetDataStatus::Success;
}

bool CookedInputLayoutHasSemantic(
    const yuengine::rhi::RhiInputLayoutDesc &layout,
    yuengine::rhi::RhiInputElementSemantic semantic) {
    for (std::size_t index = 0U; index < layout.element_count; ++index) {
        if (layout.elements[index].semantic == semantic) {
            return true;
        }
    }

    return false;
}

RuntimeAssetDataStatus ValidateCookedProgramInputLayout(
    const RuntimeAssetCookedProgramDesc &program) {
    const yuengine::rhi::RhiInputLayoutDesc &layout = program.input_layout;
    if (layout.element_count == 0U) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    if (layout.element_count > yuengine::rhi::MAX_RHI_INPUT_ELEMENTS) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    if (layout.stride_bytes == 0U || program.vertex_stride_bytes == 0U ||
        program.vertex_stride_bytes != layout.stride_bytes) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    bool has_position = false;
    for (std::size_t index = 0U; index < layout.element_count; ++index) {
        const RuntimeAssetDataStatus status =
            ValidateCookedInputElement(layout.elements[index], layout.stride_bytes);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }

        if (layout.elements[index].semantic == yuengine::rhi::RhiInputElementSemantic::Position) {
            has_position = true;
        }
    }

    if (!has_position) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    if (program.required_input_semantic_count > yuengine::rhi::MAX_RHI_INPUT_ELEMENTS) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    for (std::uint32_t index = 0U; index < program.required_input_semantic_count; ++index) {
        const yuengine::rhi::RhiInputElementSemantic semantic =
            program.required_input_semantics[index];
        if (semantic == yuengine::rhi::RhiInputElementSemantic::Unsupported) {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }

        if (!CookedInputLayoutHasSemantic(layout, semantic)) {
            return RuntimeAssetDataStatus::InvalidInputLayout;
        }
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateCookedShaderProgramRequest(
    const RuntimeAssetCookedShaderProgramPipelineRequest &request,
    const RuntimeAssetCookedShaderStagePayloadDesc **out_vertex_stage,
    const RuntimeAssetCookedShaderStagePayloadDesc **out_pixel_stage) {
    if (out_vertex_stage == nullptr || out_pixel_stage == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    *out_vertex_stage = nullptr;
    *out_pixel_stage = nullptr;

    if (request.device == nullptr || request.program == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const RuntimeAssetCookedProgramDesc &program = *request.program;
    if (program.program_id == 0U) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (program.pipeline_class != RuntimeAssetCookedProgramPipelineClass::Graphics) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    if (program.stages == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (program.stage_count == 0U) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (program.stage_count > 2U) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    for (std::uint32_t index = 0U; index < program.stage_count; ++index) {
        const RuntimeAssetCookedShaderStagePayloadDesc &stage = program.stages[index];
        const RuntimeAssetDataStatus stage_status = ValidateCookedShaderStagePayload(stage);
        if (stage_status != RuntimeAssetDataStatus::Success) {
            return stage_status;
        }

        if (stage.stage == yuengine::rhi::RhiShaderStage::Vertex) {
            if (*out_vertex_stage != nullptr) {
                return RuntimeAssetDataStatus::DuplicateDependency;
            }

            *out_vertex_stage = &stage;
        }

        if (stage.stage == yuengine::rhi::RhiShaderStage::Pixel) {
            if (*out_pixel_stage != nullptr) {
                return RuntimeAssetDataStatus::DuplicateDependency;
            }

            *out_pixel_stage = &stage;
        }
    }

    if (*out_vertex_stage == nullptr || *out_pixel_stage == nullptr) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    const RuntimeAssetDataStatus layout_status = ValidateCookedProgramInputLayout(program);
    if (layout_status != RuntimeAssetDataStatus::Success) {
        return layout_status;
    }

    if (program.texture_slot_count > yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS ||
        program.sampler_slot_count > yuengine::rhi::MAX_RHI_SAMPLER_SLOTS) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    if (program.constant_range_count > 8U) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ParseAnimationClipRecord(
    std::string_view value,
    std::uint32_t track_count,
    AnimationRuntimeClipRecord *out_clip) {
    if (out_clip == nullptr || value.empty()) {
        return RuntimeAssetDataStatus::InvalidCount;
    }

    std::uint32_t clip_id = 0U;
    std::uint32_t first_track = 0U;
    std::uint32_t clip_track_count = 0U;
    float duration_seconds = 0.0F;
    if (!ParseU32(FieldValue(value, "id="), &clip_id) || clip_id == 0U ||
        !ParseU32(FieldValue(value, "first_track_index="), &first_track) ||
        !ParseU32(FieldValue(value, "track_count="), &clip_track_count) ||
        !ParseFloat(FieldValue(value, "duration="), &duration_seconds) ||
        duration_seconds <= 0.0F ||
        clip_track_count == 0U ||
        first_track >= track_count ||
        clip_track_count > track_count - first_track) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    AnimationRuntimeClipRecord clip{};
    clip.clip_id = clip_id;
    clip.duration_seconds = duration_seconds;
    clip.first_track_index = first_track;
    clip.track_count = clip_track_count;
    clip.layer_count = 1U;
    clip.is_valid = true;
    *out_clip = clip;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ParseAnimationTrackRecord(
    std::string_view value,
    std::uint32_t keyframe_count,
    const RuntimeAssetSceneLoaderStage &stage,
    AnimationRuntimeTrackRecord *out_track) {
    if (out_track == nullptr || value.empty()) {
        return RuntimeAssetDataStatus::InvalidCount;
    }

    std::uint32_t track_id = 0U;
    std::uint32_t first_key = 0U;
    std::uint32_t track_key_count = 0U;
    WorldObjectId target{};
    AnimationRuntimeChannel channel = AnimationRuntimeChannel::TranslationX;
    if (!ParseU32(FieldValue(value, "id="), &track_id) || track_id == 0U ||
        !ParseU32(FieldValue(value, "first_key="), &first_key) ||
        !ParseU32(FieldValue(value, "key_count="), &track_key_count) ||
        track_key_count == 0U ||
        first_key >= keyframe_count ||
        track_key_count > keyframe_count - first_key) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (!ParseAnimationTarget(FieldValue(value, "target_ref="), &target)) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (!SceneStageHasWorldObject(stage, target)) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    if (!ParseAnimationChannel(FieldValue(value, "channel="), &channel)) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    const std::string_view interpolation = FieldValue(value, "interp=");
    if (!interpolation.empty() && interpolation != "linear") {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    AnimationRuntimeTrackRecord track{};
    track.track_id = track_id;
    track.target = target;
    track.channel = channel;
    track.interpolation = AnimationRuntimeInterpolation::Linear;
    track.first_keyframe_index = first_key;
    track.keyframe_count = track_key_count;
    track.is_valid = true;
    *out_track = track;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ParseAnimationKeyframeRecord(
    std::string_view value,
    AnimationRuntimeKeyframeRecord *out_keyframe) {
    if (!ParseKeyframe(value, out_keyframe)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateAnimationTrackKeyframes(
    const AnimationRuntimeClipRecord &clip,
    const AnimationRuntimeTrackRecord &track,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes) {
    if (track.keyframe_count == 0U ||
        track.first_keyframe_index >= keyframes.size() ||
        track.keyframe_count > keyframes.size() - track.first_keyframe_index) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    float previous_time = -1.0F;
    for (std::uint32_t index = 0U; index < track.keyframe_count; ++index) {
        const AnimationRuntimeKeyframeRecord &keyframe =
            keyframes[track.first_keyframe_index + index];
        if (!keyframe.is_valid ||
            !std::isfinite(keyframe.time_seconds) ||
            !std::isfinite(keyframe.value) ||
            keyframe.time_seconds < 0.0F ||
            keyframe.time_seconds > clip.duration_seconds) {
            return RuntimeAssetDataStatus::InvalidBounds;
        }

        if (index > 0U && keyframe.time_seconds <= previous_time) {
            return RuntimeAssetDataStatus::InvalidBounds;
        }

        previous_time = keyframe.time_seconds;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ParseBoundedAnimationTables(
    std::string_view animation_text,
    const RuntimeAssetSceneLoaderStage &stage,
    std::array<AnimationRuntimeClipRecord, RUNTIME_ASSET_MAX_ANIMATION_CLIP_COUNT> *out_clips,
    std::uint32_t *out_clip_count,
    std::array<AnimationRuntimeTrackRecord, RUNTIME_ASSET_MAX_ANIMATION_TRACK_COUNT> *out_tracks,
    std::uint32_t *out_track_count,
    std::array<AnimationRuntimeKeyframeRecord, RUNTIME_ASSET_MAX_ANIMATION_KEYFRAME_COUNT> *out_keyframes,
    std::uint32_t *out_keyframe_count) {
    if (out_clips == nullptr || out_clip_count == nullptr ||
        out_tracks == nullptr || out_track_count == nullptr ||
        out_keyframes == nullptr || out_keyframe_count == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const bool legacy_animation_record =
        ValueForToken(animation_text, "clips=").empty() &&
        ValueForToken(animation_text, "keyframes=").empty() &&
        !ValueForToken(animation_text, "clip=").empty();
    if (legacy_animation_record) {
        std::uint32_t clip_id = 0U;
        float duration_seconds = 0.0F;
        WorldObjectId animation_target{};
        AnimationRuntimeChannel animation_channel = AnimationRuntimeChannel::TranslationX;
        if (!ParseU32(ValueForToken(animation_text, "clip="), &clip_id) ||
            !ParseFloat(ValueForToken(animation_text, "duration="), &duration_seconds) ||
            !ParseAnimationTarget(ValueForToken(animation_text, "target="), &animation_target) ||
            !ParseAnimationChannel(ValueForToken(animation_text, "track="), &animation_channel) ||
            !ParseKeyframe(ValueForToken(animation_text, "key0="), &(*out_keyframes)[0U]) ||
            !ParseKeyframe(ValueForToken(animation_text, "key1="), &(*out_keyframes)[1U])) {
            return RuntimeAssetDataStatus::InvalidDependency;
        }

        if (!SceneStageHasWorldObject(stage, animation_target)) {
            return RuntimeAssetDataStatus::InvalidDependency;
        }

        AnimationRuntimeClipRecord clip{};
        clip.clip_id = clip_id;
        clip.duration_seconds = duration_seconds;
        clip.first_track_index = 0U;
        clip.track_count = 1U;
        clip.layer_count = 1U;
        clip.is_valid = true;
        (*out_clips)[0U] = clip;

        AnimationRuntimeTrackRecord track{};
        track.track_id = 1U;
        track.target = animation_target;
        track.channel = animation_channel;
        track.interpolation = AnimationRuntimeInterpolation::Linear;
        track.first_keyframe_index = 0U;
        track.keyframe_count = 2U;
        track.is_valid = true;
        (*out_tracks)[0U] = track;

        *out_clip_count = 1U;
        *out_track_count = 1U;
        *out_keyframe_count = 2U;
        return RuntimeAssetDataStatus::Success;
    }

    bool clip_count_declared = false;
    bool track_count_declared = false;
    bool keyframe_count_declared = false;
    std::uint32_t clip_count = 0U;
    std::uint32_t track_count = 0U;
    std::uint32_t keyframe_count = 0U;
    RuntimeAssetDataStatus status = ParseDeclaredCount(
        animation_text,
        "clips=",
        0U,
        RUNTIME_ASSET_MAX_ANIMATION_CLIP_COUNT,
        &clip_count,
        &clip_count_declared);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ParseDeclaredCount(
        animation_text,
        "tracks=",
        0U,
        RUNTIME_ASSET_MAX_ANIMATION_TRACK_COUNT,
        &track_count,
        &track_count_declared);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ParseDeclaredCount(
        animation_text,
        "keyframes=",
        0U,
        RUNTIME_ASSET_MAX_ANIMATION_KEYFRAME_COUNT,
        &keyframe_count,
        &keyframe_count_declared);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    if (!clip_count_declared || !track_count_declared || !keyframe_count_declared) {
        return RuntimeAssetDataStatus::InvalidCount;
    }

    for (std::uint32_t index = 0U; index < clip_count; ++index) {
        const std::string token = "clip" + std::to_string(index) + "=";
        status = ParseAnimationClipRecord(
            ValueForToken(animation_text, token),
            track_count,
            &(*out_clips)[index]);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }
    }

    for (std::uint32_t index = 0U; index < track_count; ++index) {
        const std::string token = "track" + std::to_string(index) + "=";
        status = ParseAnimationTrackRecord(
            ValueForToken(animation_text, token),
            keyframe_count,
            stage,
            &(*out_tracks)[index]);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }
    }

    for (std::uint32_t index = 0U; index < keyframe_count; ++index) {
        const std::string token = "key" + std::to_string(index) + "=";
        status = ParseAnimationKeyframeRecord(ValueForToken(animation_text, token), &(*out_keyframes)[index]);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }
    }

    for (std::uint32_t clip_index = 0U; clip_index < clip_count; ++clip_index) {
        const AnimationRuntimeClipRecord &clip = (*out_clips)[clip_index];
        for (std::uint32_t track_offset = 0U; track_offset < clip.track_count; ++track_offset) {
            const AnimationRuntimeTrackRecord &track = (*out_tracks)[clip.first_track_index + track_offset];
            status = ValidateAnimationTrackKeyframes(
                clip,
                track,
                std::span<const AnimationRuntimeKeyframeRecord>(out_keyframes->data(), keyframe_count));
            if (status != RuntimeAssetDataStatus::Success) {
                return status;
            }
        }
    }

    *out_clip_count = clip_count;
    *out_track_count = track_count;
    *out_keyframe_count = keyframe_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus SampleAndApplyAnimationStage(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetSceneLoaderStage *stage,
    WorldTransformBridge *bridge,
    std::span<const AnimationRuntimeClipRecord> clips,
    std::span<const AnimationRuntimeTrackRecord> tracks,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes) {
    if (stage == nullptr || bridge == nullptr || clips.empty()) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (tracks.size() > RUNTIME_ASSET_MAX_ANIMATION_SAMPLED_VALUE_COUNT) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    std::array<AnimationRuntimeSampledValue, RUNTIME_ASSET_MAX_ANIMATION_SAMPLED_VALUE_COUNT>
        sampled_values{};
    AnimationRuntimeSampleRequest sample_request{};
    sample_request.clip_id = clips[0U].clip_id;
    sample_request.clips = clips;
    sample_request.tracks = tracks;
    sample_request.keyframes = keyframes;
    sample_request.frame_context = request.animation_frame_context;
    sample_request.clip_start_time_nanoseconds = request.animation_clip_start_time_nanoseconds;

    AnimationRuntimeSampleResult sample_result{};
    const AnimationRuntimeSampler sampler;
    stage->animation_sample_status = sampler.Sample(
        sample_request,
        std::span<AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
        &sample_result);
    stage->animation_sampled_value_count = static_cast<std::uint32_t>(sample_result.sampled_value_count);
    if (stage->animation_sample_status != AnimationRuntimeStatus::Success) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    AnimationRuntimeTransformApplyResult apply_result{};
    stage->animation_apply_status = sampler.ApplySampledTransform(
        AnimationRuntimeTransformApplyRequest{bridge, std::span<const AnimationRuntimeSampledValue>(
            sampled_values.data(),
            sample_result.sampled_value_count)},
        &apply_result);
    if (stage->animation_apply_status != AnimationRuntimeStatus::Success) {
        return RuntimeAssetDataStatus::InvalidDependency;
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
    RuntimeAssetDataStatus status = ValidateOptionalDeclaredHash(scene_text);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    bool entity_count_declared = false;
    bool camera_count_declared = false;
    std::uint32_t entity_count = 0U;
    std::uint32_t camera_count = 0U;
    status = ParseDeclaredCount(
        scene_text,
        "entities=",
        RUNTIME_ASSET_SCENE_ENTITY_COUNT,
        RUNTIME_ASSET_MAX_SCENE_ENTITY_COUNT,
        &entity_count,
        &entity_count_declared);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ParseDeclaredCount(
        scene_text,
        "cameras=",
        RUNTIME_ASSET_SCENE_CAMERA_COUNT,
        RUNTIME_ASSET_MAX_SCENE_CAMERA_COUNT,
        &camera_count,
        &camera_count_declared);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    if (request.scene_entity_capacity < entity_count ||
        request.scene_transform_capacity < entity_count ||
        request.scene_camera_capacity < camera_count) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    status = ParseSceneCameras(scene_text, camera_count, camera_count_declared, &stage);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ParseSceneEntities(request, scene_text, entity_count, entity_count_declared, &stage);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    if (animation_text.empty()) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    status = ValidateOptionalDeclaredHash(animation_text);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    WorldInstance world;
    WorldTransformBridge bridge(world, WorldTransformBridgeDesc{entity_count});
    for (std::uint32_t index = 0U; index < entity_count; ++index) {
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

    std::array<AnimationRuntimeClipRecord, RUNTIME_ASSET_MAX_ANIMATION_CLIP_COUNT> clips{};
    std::array<AnimationRuntimeTrackRecord, RUNTIME_ASSET_MAX_ANIMATION_TRACK_COUNT> tracks{};
    std::array<AnimationRuntimeKeyframeRecord, RUNTIME_ASSET_MAX_ANIMATION_KEYFRAME_COUNT> keyframes{};
    std::uint32_t clip_count = 0U;
    std::uint32_t track_count = 0U;
    std::uint32_t keyframe_count = 0U;
    status = ParseBoundedAnimationTables(
        animation_text,
        stage,
        &clips,
        &clip_count,
        &tracks,
        &track_count,
        &keyframes,
        &keyframe_count);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = SampleAndApplyAnimationStage(
        request,
        &stage,
        &bridge,
        std::span<const AnimationRuntimeClipRecord>(clips.data(), clip_count),
        std::span<const AnimationRuntimeTrackRecord>(tracks.data(), track_count),
        std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframe_count));
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    for (std::uint32_t index = 0U; index < entity_count; ++index) {
        const WorldTransformResult transform_result =
            bridge.Query(stage.entities[index].world_object_id);
        if (transform_result.status != WorldTransformStatus::Success) {
            return RuntimeAssetDataStatus::InvalidDependency;
        }

        stage.entities[index].transform = transform_result.transform_state;
        stage.transforms[index].world_object_id = stage.entities[index].world_object_id;
        stage.transforms[index].transform = transform_result.transform_state;
    }
    stage.transform_count = entity_count;

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

    for (std::uint32_t index = 0U; index < stage.camera_count; ++index) {
        request.scene_cameras[index] = stage.cameras[index];
    }

    for (std::uint32_t index = 0U; index < stage.entity_count; ++index) {
        request.scene_entities[index] = stage.entities[index];
    }

    for (std::uint32_t index = 0U; index < stage.transform_count; ++index) {
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
    RuntimeAssetGraphRequestCounts counts{};
    RuntimeAssetDataStatus status = BuildGraphRequestCounts(request.file_count, &counts);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    const ResourceSnapshot resource_snapshot = request.resource_registry->Snapshot();
    const AssetSnapshot asset_snapshot = request.asset_manager->Snapshot();
    if (CountExceedsCapacity(
            resource_snapshot.registered_resource_count,
            counts.record_count,
            yuengine::resource::MAX_RESOURCE_COUNT) ||
        CountExceedsCapacity(
            resource_snapshot.load_commit_record_count,
            counts.record_count,
            yuengine::resource::MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT) ||
        CountExceedsCapacity(
            asset_snapshot.active_asset_count,
            counts.record_count,
            yuengine::asset::MAX_ASSET_COUNT)) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    if (CountExceedsCapacity(
            resource_snapshot.dependency_edge_count,
            request.file_count,
            yuengine::resource::MAX_DEPENDENCY_EDGE_COUNT) ||
        CountExceedsCapacity(
            asset_snapshot.active_dependency_edge_count,
            request.file_count,
            yuengine::asset::MAX_ASSET_DEPENDENCY_EDGE_COUNT)) {
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

    const ResourceCachePayloadSnapshot cache_snapshot = request.resource_registry->CachePayloadSnapshot();
    const ResourceDecodePlanSnapshot decode_plan_snapshot = request.resource_registry->DecodePlanSnapshot();
    const ResourceDecodeResultSnapshot decode_result_snapshot = request.resource_registry->DecodeResultSnapshot();
    const ResourceDecodedPayloadSnapshot decoded_snapshot = request.resource_registry->DecodedPayloadSnapshot();
    if ((cache_snapshot.cache_payload_record_count + transaction.plan.cache_payload_commit_count) >
            yuengine::resource::MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT ||
        (decode_plan_snapshot.decode_plan_record_count + transaction.plan.decoded_payload_commit_count) >
            yuengine::resource::MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT ||
        (decode_result_snapshot.decode_result_record_count + transaction.plan.decoded_payload_commit_count) >
            yuengine::resource::MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT ||
        (decoded_snapshot.decoded_payload_record_count + transaction.plan.decoded_payload_commit_count) >
            yuengine::resource::MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    if ((cache_snapshot.cached_byte_count + cache_payload_bytes) > DEFAULT_PAYLOAD_BYTE_CAPACITY ||
        (decode_plan_snapshot.planned_decoded_byte_count + decoded_payload_bytes) >
            yuengine::resource::MAX_RESOURCE_DECODE_PLAN_TOTAL_DECODED_BYTES ||
        (decode_result_snapshot.committed_decoded_byte_count + decoded_payload_bytes) >
            yuengine::resource::MAX_RESOURCE_DECODE_RESULT_TOTAL_DECODED_BYTES ||
        (decoded_snapshot.stored_decoded_byte_count + decoded_payload_bytes) >
            yuengine::resource::MAX_RESOURCE_DECODED_PAYLOAD_TOTAL_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus PlanRuntimeAssetCommitIntents(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetGraphTransactionData *transaction) {
    RuntimeAssetGraphRequestCounts counts{};
    const RuntimeAssetDataStatus status = BuildGraphRequestCounts(request.file_count, &counts);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    transaction->plan.record_count = counts.record_count;
    transaction->plan.resource_commit_count = counts.record_count;
    transaction->plan.asset_commit_count = counts.record_count;
    transaction->plan.cache_payload_commit_count = counts.record_count;
    transaction->plan.dependency_edge_commit_count = counts.dependency_edge_count;
    for (std::uint32_t index = 0U; index < request.file_count; ++index) {
        if (request.files[index].decode_asset_class != ResourceDecodePlanAssetClass::Unknown) {
            ++transaction->plan.cache_payload_commit_count;
            ++transaction->plan.decoded_payload_commit_count;
        }
    }

    return RuntimeAssetDataStatus::Success;
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

    status = PlanRuntimeAssetCommitIntents(request, transaction);
    if (status != RuntimeAssetDataStatus::Success) {
        SetTransactionFailure(transaction, status, RuntimeAssetLoadTransactionPhase::Preflight);
        return status;
    }

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

bool IsPipelineHandleSet(yuengine::rhi::RhiPipelineHandle handle) {
    return handle.generation != 0U;
}

bool IsTextureHandleSet(RhiTextureHandle handle) {
    return handle.generation != 0U;
}

bool IsSamplerHandleSet(RhiSamplerHandle handle) {
    return handle.generation != 0U;
}

bool IsPowerOfTwo(std::uint32_t value) {
    return value != 0U && (value & (value - 1U)) == 0U;
}

bool IsCookedTextureColorSpaceSupported(RuntimeAssetCookedTextureColorSpace color_space) {
    return color_space == RuntimeAssetCookedTextureColorSpace::Linear ||
        color_space == RuntimeAssetCookedTextureColorSpace::Srgb;
}

std::uint32_t RuntimeAssetRgba8RowPitch(const yuengine::rhi::RhiTextureDesc &desc) {
    return static_cast<std::uint32_t>(desc.extent.width) *
        static_cast<std::uint32_t>(yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
}

std::uint32_t RuntimeAssetRgba8SlicePitch(const yuengine::rhi::RhiTextureDesc &desc) {
    return RuntimeAssetRgba8RowPitch(desc) * static_cast<std::uint32_t>(desc.extent.height);
}

ResourceDecodedPayloadRequest DecodedPayloadRequestForCookedTexture(
    const RuntimeAssetCookedTexturePayloadDesc &texture) {
    ResourceDecodedPayloadRequest request{};
    if (texture.loaded_texture == nullptr) {
        return request;
    }

    const RuntimeAssetLoadedFile &loaded_texture = *texture.loaded_texture;
    request.resource = loaded_texture.resource;
    request.expected_type = loaded_texture.resource_type;
    request.payload_id = loaded_texture.decode_plan_payload_id;
    request.decode_plan_id = loaded_texture.decode_plan_id;
    request.decode_result_id = loaded_texture.decode_result_id;
    request.decoded_payload_id = texture.decoded_payload_id;
    request.asset_class = loaded_texture.decode_asset_class;
    request.result_class = loaded_texture.decode_result_class;
    request.decoded_byte_count = texture.payload_byte_count;
    return request;
}

RuntimeAssetDataStatus MapDecodedPayloadStatus(ResourceDecodedPayloadStatus status) {
    switch (status) {
        case ResourceDecodedPayloadStatus::Success:
            return RuntimeAssetDataStatus::Success;
        case ResourceDecodedPayloadStatus::MissingDecodedPayload:
        case ResourceDecodedPayloadStatus::MissingDecodeResult:
        case ResourceDecodedPayloadStatus::MissingDecodePlan:
        case ResourceDecodedPayloadStatus::MissingCachePayload:
            return RuntimeAssetDataStatus::MissingDependency;
        case ResourceDecodedPayloadStatus::TypeMismatch:
        case ResourceDecodedPayloadStatus::AssetClassMismatch:
        case ResourceDecodedPayloadStatus::ResultClassMismatch:
            return RuntimeAssetDataStatus::TypeMismatch;
        case ResourceDecodedPayloadStatus::DecodedByteCountMismatch:
        case ResourceDecodedPayloadStatus::OutputBufferTooSmall:
        case ResourceDecodedPayloadStatus::EmptyPayload:
            return RuntimeAssetDataStatus::InvalidSize;
        case ResourceDecodedPayloadStatus::CapacityExceeded:
            return RuntimeAssetDataStatus::CapacityExceeded;
        case ResourceDecodedPayloadStatus::BudgetExceeded:
            return RuntimeAssetDataStatus::BudgetExceeded;
        default:
            break;
    }

    return RuntimeAssetDataStatus::InvalidDependency;
}

RuntimeAssetDataStatus MapTextureBridgeStatus(
    const ResourceDecodedTextureBridgeResult &texture_result) {
    switch (texture_result.status) {
        case ResourceDecodedTextureBridgeStatus::Success:
            return RuntimeAssetDataStatus::Success;
        case ResourceDecodedTextureBridgeStatus::ResourceQueryFailed:
        case ResourceDecodedTextureBridgeStatus::ResourceReadFailed:
            return MapDecodedPayloadStatus(texture_result.decoded_payload_status);
        case ResourceDecodedTextureBridgeStatus::ScratchBufferTooSmall:
        case ResourceDecodedTextureBridgeStatus::TextureByteCountMismatch:
            return RuntimeAssetDataStatus::InvalidSize;
        case ResourceDecodedTextureBridgeStatus::SampledTextureSlotOutOfRange:
            return RuntimeAssetDataStatus::CapacityExceeded;
        case ResourceDecodedTextureBridgeStatus::UploadSubmitFailed:
        case ResourceDecodedTextureBridgeStatus::UploadCompletionMissing:
            return RuntimeAssetDataStatus::CapacityExceeded;
        case ResourceDecodedTextureBridgeStatus::UploadProcessFailed:
            return RuntimeAssetDataStatus::RhiTextureFailed;
        case ResourceDecodedTextureBridgeStatus::InvalidArgument:
            return RuntimeAssetDataStatus::InvalidArgument;
        default:
            break;
    }

    return RuntimeAssetDataStatus::RhiTextureFailed;
}

RuntimeAssetDataStatus ValidateLoadedCookedTexture(
    AssetManager &manager,
    const RuntimeAssetCookedTexturePayloadDesc &texture,
    RuntimeAssetCookedTextureMaterialBridgeResult *result) {
    if (texture.loaded_texture == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const RuntimeAssetLoadedFile &loaded_texture = *texture.loaded_texture;
    if (loaded_texture.kind != RuntimeAssetFileKind::Texture ||
        loaded_texture.decode_asset_class != ResourceDecodePlanAssetClass::Texture ||
        loaded_texture.decode_result_class != ResourceDecodeResultClass::Texture) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (!loaded_texture.resource.IsValid() ||
        !loaded_texture.asset.IsValid() ||
        !loaded_texture.decode_plan_created ||
        !loaded_texture.decode_result_committed ||
        !loaded_texture.decoded_payload_stored ||
        loaded_texture.decode_plan_payload_id == 0U ||
        loaded_texture.decode_plan_id == 0U ||
        loaded_texture.decode_result_id == 0U ||
        loaded_texture.decoded_payload_id == 0U ||
        texture.decoded_payload_id == 0U) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (loaded_texture.decoded_byte_count != texture.payload_byte_count) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    AssetRecord texture_record{};
    const AssetStatus asset_status = manager.QueryAsset(loaded_texture.asset, &texture_record);
    if (asset_status != AssetStatus::Success) {
        if (result != nullptr) {
            result->asset_status = asset_status;
        }
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (texture_record.resource.slot != loaded_texture.resource.slot ||
        texture_record.resource.generation != loaded_texture.resource.generation ||
        texture_record.resource_type.value != loaded_texture.resource_type.value ||
        texture_record.asset_type.value != loaded_texture.asset_type.value) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateCookedTextureLayout(
    const RuntimeAssetCookedTexturePayloadDesc &texture) {
    if (texture.texture_desc.format != RhiFormat::Rgba8Unorm) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    if (texture.texture_desc.extent.width == 0U || texture.texture_desc.extent.height == 0U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (!IsCookedTextureColorSpaceSupported(texture.color_space)) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    if (!IsPowerOfTwo(texture.payload_alignment_bytes)) {
        return RuntimeAssetDataStatus::InvalidAlignment;
    }

    if ((texture.payload_offset_bytes % texture.payload_alignment_bytes) != 0U) {
        return RuntimeAssetDataStatus::InvalidAlignment;
    }

    if (texture.payload_offset_bytes != 0U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    const std::uint32_t expected_row_pitch = RuntimeAssetRgba8RowPitch(texture.texture_desc);
    const std::uint32_t expected_slice_pitch = RuntimeAssetRgba8SlicePitch(texture.texture_desc);
    if (texture.row_pitch_bytes != expected_row_pitch ||
        texture.slice_pitch_bytes != expected_slice_pitch ||
        texture.payload_byte_count != expected_slice_pitch) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (texture.payload_hash == 0U) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    if (texture.staging_request_id == 0U || texture.upload_id == 0U) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateCookedTexturePayloadBytes(
    const RuntimeAssetCookedTextureMaterialBridgeRequest &request,
    const RuntimeAssetCookedTexturePayloadDesc &texture,
    RuntimeAssetCookedTextureMaterialBridgeResult *result) {
    if (request.scratch_bytes.size() < texture.payload_byte_count) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    const ResourceDecodedPayloadRequest payload_request = DecodedPayloadRequestForCookedTexture(texture);
    yuengine::resource::ResourceDecodedPayloadRecord decoded_record{};
    ResourceDecodedPayloadStatus decoded_status =
        request.resource_registry->QueryDecodedPayload(payload_request, &decoded_record);
    if (decoded_status != ResourceDecodedPayloadStatus::Success) {
        if (result != nullptr) {
            result->decoded_payload_status = decoded_status;
        }
        return MapDecodedPayloadStatus(decoded_status);
    }

    if (decoded_record.decoded_byte_count != texture.payload_byte_count) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    std::uint32_t read_byte_count = 0U;
    decoded_status = request.resource_registry->ReadDecodedPayload(
        payload_request,
        request.scratch_bytes.data(),
        static_cast<std::uint32_t>(request.scratch_bytes.size()),
        &read_byte_count);
    if (decoded_status != ResourceDecodedPayloadStatus::Success) {
        if (result != nullptr) {
            result->decoded_payload_status = decoded_status;
        }
        return MapDecodedPayloadStatus(decoded_status);
    }

    if (read_byte_count != texture.payload_byte_count) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    const std::uint64_t actual_hash = HashRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(request.scratch_bytes.data(), texture.payload_byte_count));
    if (actual_hash != texture.payload_hash) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateCookedMaterialSlots(
    const RuntimeAssetCookedTextureMaterialBridgeRequest &request) {
    if (request.material_slots.size() < yuengine::renderscene::MIN_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (request.material_slots.size() > yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    std::array<bool, yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS> material_slots{};
    std::array<bool, yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS> texture_binding_slots{};
    std::array<bool, yuengine::rhi::MAX_RHI_SAMPLER_SLOTS> sampler_binding_slots{};
    std::array<bool, yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS> texture_payload_refs{};

    for (const RuntimeAssetCookedMaterialSlotDesc &slot : request.material_slots) {
        if (slot.texture_payload_index >= request.textures.size()) {
            return RuntimeAssetDataStatus::MissingDependency;
        }

        if (slot.material_slot >= yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS ||
            slot.texture_binding_slot >= yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS ||
            slot.sampler_binding_slot >= yuengine::rhi::MAX_RHI_SAMPLER_SLOTS) {
            return RuntimeAssetDataStatus::CapacityExceeded;
        }

        if (material_slots[slot.material_slot] ||
            texture_binding_slots[slot.texture_binding_slot] ||
            sampler_binding_slots[slot.sampler_binding_slot] ||
            texture_payload_refs[slot.texture_payload_index]) {
            return RuntimeAssetDataStatus::DuplicateDependency;
        }

        material_slots[slot.material_slot] = true;
        texture_binding_slots[slot.texture_binding_slot] = true;
        sampler_binding_slots[slot.sampler_binding_slot] = true;
        texture_payload_refs[slot.texture_payload_index] = true;

        if (slot.texture_binding_slot != slot.material_slot ||
            slot.sampler_binding_slot != slot.material_slot) {
            return RuntimeAssetDataStatus::TypeMismatch;
        }

        if (!IsCookedTextureColorSpaceSupported(slot.expected_color_space)) {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }

        const RuntimeAssetCookedTexturePayloadDesc &texture = request.textures[slot.texture_payload_index];
        if (slot.expected_format != texture.texture_desc.format ||
            slot.expected_color_space != texture.color_space) {
            return RuntimeAssetDataStatus::TypeMismatch;
        }
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateCookedTextureMaterialBridgeRequest(
    const RuntimeAssetCookedTextureMaterialBridgeRequest &request,
    RuntimeAssetCookedTextureMaterialBridgeResult *result) {
    if (request.resource_registry == nullptr ||
        request.asset_manager == nullptr ||
        request.rhi_device == nullptr ||
        request.out_material == nullptr ||
        request.textures.data() == nullptr ||
        request.material_slots.data() == nullptr ||
        request.scratch_bytes.data() == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (!request.material_asset.IsValid() || request.material_id == 0U || !IsPipelineHandleSet(request.pipeline)) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.scratch_bytes.size() > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.textures.empty()) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (request.textures.size() > yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    AssetRecord material_record{};
    const AssetStatus material_asset_status = request.asset_manager->QueryAsset(
        request.material_asset,
        &material_record);
    if (material_asset_status != AssetStatus::Success) {
        if (result != nullptr) {
            result->asset_status = material_asset_status;
        }
        return RuntimeAssetDataStatus::MissingDependency;
    }

    for (const RuntimeAssetCookedTexturePayloadDesc &texture : request.textures) {
        RuntimeAssetDataStatus status = ValidateLoadedCookedTexture(*request.asset_manager, texture, result);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }

        status = ValidateCookedTextureLayout(texture);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }

        status = ValidateCookedTexturePayloadBytes(request, texture, result);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }
    }

    return ValidateCookedMaterialSlots(request);
}

void CleanupCookedTextureMaterialBridge(
    const RuntimeAssetCookedTextureMaterialBridgeRequest &request,
    std::span<const RhiTextureHandle> texture_handles,
    std::span<const RhiSamplerHandle> sampler_handles,
    RuntimeAssetCookedTextureMaterialBridgeResult *result) {
    if (request.rhi_device == nullptr || result == nullptr) {
        return;
    }

    for (const RhiSamplerHandle sampler : sampler_handles) {
        if (!IsSamplerHandleSet(sampler)) {
            continue;
        }

        const RhiStatus status = request.rhi_device->DestroySampler(sampler);
        if (status == RhiStatus::Success) {
            ++result->cleanup_sampler_count;
            continue;
        }

        if (result->rhi_status == RhiStatus::Success) {
            result->rhi_status = status;
        }
    }

    for (const RhiTextureHandle texture : texture_handles) {
        if (!IsTextureHandleSet(texture)) {
            continue;
        }

        const RhiStatus status = request.rhi_device->DestroyTexture(texture);
        if (status == RhiStatus::Success) {
            ++result->cleanup_texture_count;
            continue;
        }

        if (result->rhi_status == RhiStatus::Success) {
            result->rhi_status = status;
        }
    }
}

ResourceDecodedTextureBridgeRequest BuildCookedTextureBridgeRequest(
    const RuntimeAssetCookedTextureMaterialBridgeRequest &request,
    const RuntimeAssetCookedTexturePayloadDesc &texture,
    const RuntimeAssetCookedMaterialSlotDesc &slot,
    RhiTextureHandle *out_texture_handle) {
    ResourceDecodedTextureBridgeRequest bridge_request{};
    bridge_request.resource_registry = request.resource_registry;
    bridge_request.rhi_device = request.rhi_device;
    bridge_request.decoded_payload = DecodedPayloadRequestForCookedTexture(texture);
    bridge_request.scratch_bytes = request.scratch_bytes;
    bridge_request.texture_desc = texture.texture_desc;
    bridge_request.output_texture_handle = out_texture_handle;
    bridge_request.staging_request_id = texture.staging_request_id;
    bridge_request.upload_id = texture.upload_id;
    bridge_request.sampled_texture_slot = slot.texture_binding_slot;
    return bridge_request;
}

RuntimeAssetDataStatus BuildCookedTextureMaterialCommit(
    const RuntimeAssetCookedTextureMaterialBridgeRequest &request,
    RuntimeAssetCookedTextureMaterialBridgeResult *result) {
    std::array<RhiTextureHandle, yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS>
        transient_textures{};
    std::array<RhiSamplerHandle, yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS>
        transient_samplers{};
    std::array<ResourceDecodedTextureBridgeResult, yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS>
        texture_results{};
    std::array<RenderSceneRuntimeMaterialTextureSlot, yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS>
        render_slots{};

    ResourceDecodedTextureBridge texture_bridge;
    std::uint32_t uploaded_count = 0U;
    std::uint32_t sampler_count = 0U;
    for (std::size_t index = 0U; index < request.material_slots.size(); ++index) {
        const RuntimeAssetCookedMaterialSlotDesc &slot = request.material_slots[index];
        const RuntimeAssetCookedTexturePayloadDesc &texture = request.textures[slot.texture_payload_index];

        RhiTextureHandle texture_handle{};
        const ResourceDecodedTextureBridgeRequest bridge_request =
            BuildCookedTextureBridgeRequest(request, texture, slot, &texture_handle);
        const ResourceDecodedTextureBridgeResult texture_result =
            texture_bridge.UploadTexture(bridge_request);
        result->texture_bridge_status = texture_result.status;
        result->decoded_payload_status = texture_result.decoded_payload_status;
        result->rhi_status = texture_result.rhi_status;
        if (texture_result.status != ResourceDecodedTextureBridgeStatus::Success) {
            result->status = MapTextureBridgeStatus(texture_result);
            CleanupCookedTextureMaterialBridge(
                request,
                std::span<const RhiTextureHandle>(transient_textures.data(), uploaded_count),
                std::span<const RhiSamplerHandle>(transient_samplers.data(), sampler_count),
                result);
            return result->status;
        }

        result->mutated_state = true;
        transient_textures[uploaded_count] = texture_result.texture_handle;
        texture_results[uploaded_count] = texture_result;
        ++uploaded_count;

        RhiSamplerHandle sampler{};
        const RhiStatus sampler_status = request.rhi_device->CreateSampler(slot.sampler_desc, sampler);
        result->rhi_status = sampler_status;
        if (sampler_status != RhiStatus::Success) {
            result->status = RuntimeAssetDataStatus::RhiSamplerFailed;
            CleanupCookedTextureMaterialBridge(
                request,
                std::span<const RhiTextureHandle>(transient_textures.data(), uploaded_count),
                std::span<const RhiSamplerHandle>(transient_samplers.data(), sampler_count),
                result);
            return result->status;
        }

        result->mutated_state = true;
        transient_samplers[sampler_count] = sampler;
        ++sampler_count;

        render_slots[index].slot = slot.material_slot;
        render_slots[index].texture_asset = texture.loaded_texture->asset;
        render_slots[index].sampled_texture = texture_result.sampled_texture;
        render_slots[index].sampler = RhiSamplerBinding{sampler, slot.sampler_binding_slot};
    }

    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneRuntimeMaterialRequest material_request{};
    material_request.material_asset = request.material_asset;
    material_request.material_id = request.material_id;
    material_request.pipeline = request.pipeline;
    material_request.texture_slots = std::span<const RenderSceneRuntimeMaterialTextureSlot>(
        render_slots.data(),
        request.material_slots.size());

    RenderSceneRuntimeMaterialBuilder builder;
    const RenderSceneRuntimeMaterialStatus material_status =
        builder.Build(material_request, &material);
    result->material_status = material_status;
    if (material_status != RenderSceneRuntimeMaterialStatus::Success) {
        result->status = RuntimeAssetDataStatus::RenderSceneMaterialFailed;
        CleanupCookedTextureMaterialBridge(
            request,
            std::span<const RhiTextureHandle>(transient_textures.data(), uploaded_count),
            std::span<const RhiSamplerHandle>(transient_samplers.data(), sampler_count),
            result);
        return result->status;
    }

    for (std::size_t index = 0U; index < request.material_slots.size(); ++index) {
        const RuntimeAssetCookedMaterialSlotDesc &slot = request.material_slots[index];
        const RuntimeAssetCookedTexturePayloadDesc &texture = request.textures[slot.texture_payload_index];
        const AssetStatus asset_status =
            request.asset_manager->MarkTextureReady(texture.loaded_texture->asset, texture_results[index]);
        result->asset_status = asset_status;
        if (asset_status != AssetStatus::Success) {
            result->status = RuntimeAssetDataStatus::AssetRegistrationFailed;
            CleanupCookedTextureMaterialBridge(
                request,
                std::span<const RhiTextureHandle>(transient_textures.data(), uploaded_count),
                std::span<const RhiSamplerHandle>(transient_samplers.data(), sampler_count),
                result);
            return result->status;
        }
    }

    *request.out_material = material;
    result->runtime_texture_upload_count = uploaded_count;
    result->material_texture_slot_count = static_cast<std::uint32_t>(material.texture_slot_count);
    result->published_material = true;
    result->status = RuntimeAssetDataStatus::Success;
    return result->status;
}

void CountVisualProofLoadedRecord(
    const RuntimeAssetLoadedFile &file,
    RuntimeAssetVisualProofResult *result) {
    if (result == nullptr) {
        return;
    }

    if (file.artifact_class == RuntimeAssetArtifactClass::Cooked) {
        ++result->cooked_record_count;
    }

    if (file.artifact_class == RuntimeAssetArtifactClass::Source) {
        ++result->source_record_count;
    }

    if (file.kind == RuntimeAssetFileKind::Mesh) {
        ++result->mesh_record_count;
    }

    if (file.kind == RuntimeAssetFileKind::Texture) {
        ++result->texture_record_count;
    }
}

void SeedVisualProofLedger(
    const RuntimeAssetVisualProofRequest &request,
    RuntimeAssetVisualProofResult *result) {
    if (result == nullptr) {
        return;
    }

    if (request.scene != nullptr) {
        CountVisualProofLoadedRecord(*request.scene, result);
    }

    for (const RuntimeAssetLoadedFile &file : request.loaded_files) {
        CountVisualProofLoadedRecord(file, result);
    }

    result->scene_entity_count = request.scene_output != nullptr ?
        request.scene_output->entity_count :
        static_cast<std::uint32_t>(request.scene_entities.size());
    result->scene_transform_count = request.scene_output != nullptr ?
        request.scene_output->transform_count :
        static_cast<std::uint32_t>(request.scene_transforms.size());
    result->scene_camera_count = request.scene_output != nullptr ?
        request.scene_output->camera_count :
        static_cast<std::uint32_t>(request.scene_cameras.size());
    result->animation_sampled_value_count = request.scene_output != nullptr ?
        request.scene_output->animation_sampled_value_count :
        0U;
}

RuntimeAssetDataStatus FailVisualProof(
    RuntimeAssetDataStatus status,
    RuntimeAssetVisualProofMissingLayer layer,
    RuntimeAssetVisualProofResult *result) {
    if (result != nullptr) {
        result->status = status;
        result->first_missing_layer = layer;
    }

    return status;
}

bool IsLoadedRuntimeAssetRecordUsable(
    const RuntimeAssetLoadedFile &file,
    RuntimeAssetFileKind expected_kind,
    bool require_cooked) {
    if (file.kind != expected_kind) {
        return false;
    }

    if (!file.resource.IsValid() || !file.asset.IsValid()) {
        return false;
    }

    if (file.stable_id == 0U || file.hash == 0U || !file.cache_payload_stored) {
        return false;
    }

    if (require_cooked && file.artifact_class != RuntimeAssetArtifactClass::Cooked) {
        return false;
    }

    return file.artifact_class != RuntimeAssetArtifactClass::Unknown;
}

const RuntimeAssetLoadedFile *FindVisualProofLoadedFile(
    const RuntimeAssetVisualProofRequest &request,
    RuntimeAssetFileKind kind,
    std::uint32_t ordinal) {
    std::uint32_t matched_count = 0U;
    for (const RuntimeAssetLoadedFile &file : request.loaded_files) {
        if (file.kind != kind) {
            continue;
        }

        if (matched_count == ordinal) {
            return &file;
        }

        ++matched_count;
    }

    return nullptr;
}

RuntimeAssetDataStatus RequireVisualProofRecord(
    const RuntimeAssetVisualProofRequest &request,
    RuntimeAssetFileKind kind,
    std::uint32_t ordinal,
    RuntimeAssetVisualProofMissingLayer layer,
    const RuntimeAssetLoadedFile **out_file,
    RuntimeAssetVisualProofResult *result) {
    if (out_file == nullptr) {
        return FailVisualProof(RuntimeAssetDataStatus::InvalidArgument, layer, result);
    }

    const RuntimeAssetLoadedFile *file = FindVisualProofLoadedFile(request, kind, ordinal);
    if (file == nullptr || !IsLoadedRuntimeAssetRecordUsable(*file, kind, request.require_cooked_records)) {
        *out_file = nullptr;
        return FailVisualProof(RuntimeAssetDataStatus::MissingDependency, layer, result);
    }

    *out_file = file;
    return RuntimeAssetDataStatus::Success;
}

RenderScenePrimitiveGeometryKind ToRenderScenePrimitiveKind(RuntimeAssetMeshGeometryKind kind) {
    if (kind == RuntimeAssetMeshGeometryKind::Cylinder) {
        return RenderScenePrimitiveGeometryKind::Cylinder;
    }

    if (kind == RuntimeAssetMeshGeometryKind::Cone) {
        return RenderScenePrimitiveGeometryKind::Cone;
    }

    return RenderScenePrimitiveGeometryKind::Cube;
}

bool ResolveVisualProofMeshCounts(
    const RuntimeAssetLoadedFile &mesh,
    std::uint32_t *out_segment_count,
    std::uint32_t *out_vertex_count,
    std::uint32_t *out_index_count) {
    if (out_segment_count == nullptr || out_vertex_count == nullptr || out_index_count == nullptr) {
        return false;
    }

    if (mesh.vertex_count == 0U || mesh.index_count == 0U) {
        return false;
    }

    if (mesh.mesh_geometry_kind == RuntimeAssetMeshGeometryKind::Cube) {
        if (mesh.vertex_count != 24U || mesh.index_count != 36U) {
            return false;
        }

        *out_segment_count = 0U;
        *out_vertex_count = mesh.vertex_count;
        *out_index_count = mesh.index_count;
        return true;
    }

    if (mesh.mesh_geometry_kind == RuntimeAssetMeshGeometryKind::Cylinder) {
        if ((mesh.index_count % 12U) != 0U) {
            return false;
        }

        const std::uint32_t segment_count = mesh.index_count / 12U;
        if (mesh.vertex_count != ((segment_count * 2U) + 2U)) {
            return false;
        }

        *out_segment_count = segment_count;
        *out_vertex_count = mesh.vertex_count;
        *out_index_count = mesh.index_count;
        return true;
    }

    if (mesh.mesh_geometry_kind == RuntimeAssetMeshGeometryKind::Cone) {
        if ((mesh.index_count % 6U) != 0U) {
            return false;
        }

        const std::uint32_t segment_count = mesh.index_count / 6U;
        if (mesh.vertex_count != (segment_count + 2U)) {
            return false;
        }

        *out_segment_count = segment_count;
        *out_vertex_count = mesh.vertex_count;
        *out_index_count = mesh.index_count;
        return true;
    }

    return false;
}

RuntimeAssetDataStatus CreateVisualProofBuffer(
    IRhiDevice &device,
    RhiBufferUsage usage,
    std::size_t byte_count,
    RhiBufferHandle *out_handle) {
    if (out_handle == nullptr || byte_count == 0U) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RhiBufferDesc desc{};
    desc.usage = usage;
    desc.size_bytes = byte_count;
    const std::span<const std::uint8_t> initial_bytes{};
    const RhiStatus status = device.CreateBuffer(desc, initial_bytes, *out_handle);
    return status == RhiStatus::Success ? RuntimeAssetDataStatus::Success : RuntimeAssetDataStatus::RhiCaptureFailed;
}

RuntimeAssetDataStatus BuildVisualProofGeometry(
    const RuntimeAssetLoadedFile &mesh,
    std::uint32_t index,
    std::size_t vertex_stride_bytes,
    IRhiDevice &device,
    RenderScenePrimitiveGeometryRecord *out_geometry,
    RuntimeAssetVisualProofResult *result) {
    if (out_geometry == nullptr || vertex_stride_bytes == 0U) {
        return FailVisualProof(
            RuntimeAssetDataStatus::InvalidArgument,
            RuntimeAssetVisualProofMissingLayer::Model,
            result);
    }

    std::uint32_t segment_count = 0U;
    std::uint32_t vertex_count = 0U;
    std::uint32_t index_count = 0U;
    if (!ResolveVisualProofMeshCounts(mesh, &segment_count, &vertex_count, &index_count)) {
        return FailVisualProof(
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetVisualProofMissingLayer::Model,
            result);
    }

    RhiBufferHandle vertex_buffer{};
    RuntimeAssetDataStatus status = CreateVisualProofBuffer(
        device,
        RhiBufferUsage::Vertex,
        vertex_stride_bytes * vertex_count,
        &vertex_buffer);
    if (status != RuntimeAssetDataStatus::Success) {
        return FailVisualProof(status, RuntimeAssetVisualProofMissingLayer::RhiCapture, result);
    }

    RhiBufferHandle index_buffer{};
    status = CreateVisualProofBuffer(
        device,
        RhiBufferUsage::Index,
        sizeof(std::uint16_t) * index_count,
        &index_buffer);
    if (status != RuntimeAssetDataStatus::Success) {
        return FailVisualProof(status, RuntimeAssetVisualProofMissingLayer::RhiCapture, result);
    }

    RenderScenePrimitiveGeometryRequest request{};
    request.geometry_asset = mesh.asset;
    request.kind = ToRenderScenePrimitiveKind(mesh.mesh_geometry_kind);
    request.segment_count = segment_count;
    request.draw_id = RUNTIME_ASSET_VISUAL_PROOF_FIRST_DRAW_ID + index;
    request.pass_id = RUNTIME_ASSET_VISUAL_PROOF_PASS_ID;
    request.material_id = RUNTIME_ASSET_VISUAL_PROOF_MATERIAL_ID;
    request.vertex_buffer = RhiVertexBufferView{
        vertex_buffer,
        0U,
        vertex_stride_bytes,
        vertex_stride_bytes * vertex_count};
    request.index_buffer = RhiIndexBufferView{
        index_buffer,
        0U,
        sizeof(std::uint16_t) * index_count,
        RhiIndexFormat::Uint16};

    RenderScenePrimitiveGeometryBuilder builder;
    const RenderScenePrimitiveGeometryStatus geometry_status = builder.Build(request, out_geometry);
    if (geometry_status != RenderScenePrimitiveGeometryStatus::Success) {
        return FailVisualProof(
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetVisualProofMissingLayer::Model,
            result);
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus BuildVisualProofShaderPipeline(
    const RuntimeAssetVisualProofRequest &request,
    RuntimeAssetVisualProofResult *result) {
    const RuntimeAssetLoadedFile *shader = nullptr;
    RuntimeAssetDataStatus status = RequireVisualProofRecord(
        request,
        RuntimeAssetFileKind::Shader,
        0U,
        RuntimeAssetVisualProofMissingLayer::ShaderPipeline,
        &shader,
        result);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    if (shader == nullptr ||
        shader->shader_stage_count < 2U ||
        shader->shader_bytecode_byte_count == 0U ||
        request.shader_program == nullptr ||
        request.shader_program->status != RuntimeAssetDataStatus::Success ||
        request.shader_program->validation.artifact_class == RuntimeAssetArtifactClass::Unknown ||
        (request.require_cooked_records &&
            request.shader_program->validation.artifact_class != RuntimeAssetArtifactClass::Cooked) ||
        request.shader_program->texture_slot_count < RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT) {
        return FailVisualProof(
            RuntimeAssetDataStatus::RhiPipelineFailed,
            RuntimeAssetVisualProofMissingLayer::ShaderPipeline,
            result);
    }

    RuntimeAssetShaderProgramPipelineRequest shader_request{};
    shader_request.device = request.rhi_device;
    shader_request.program = request.shader_program;
    status = BuildRuntimeAssetShaderProgramPipeline(shader_request, &result->shader_pipeline_result);
    if (status != RuntimeAssetDataStatus::Success) {
        return FailVisualProof(
            RuntimeAssetDataStatus::RhiPipelineFailed,
            RuntimeAssetVisualProofMissingLayer::ShaderPipeline,
            result);
    }

    result->shader_pipeline_from_runtime_asset = true;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ReadVisualProofDecodedTexturePayload(
    ResourceRegistry &registry,
    const RuntimeAssetLoadedFile &texture,
    std::vector<std::uint8_t> *out_bytes) {
    if (out_bytes == nullptr || texture.decoded_byte_count == 0U) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    out_bytes->assign(texture.decoded_byte_count, 0U);
    ResourceDecodedPayloadRequest request{};
    request.resource = texture.resource;
    request.expected_type = texture.resource_type;
    request.payload_id = texture.decode_plan_payload_id;
    request.decode_plan_id = texture.decode_plan_id;
    request.decode_result_id = texture.decode_result_id;
    request.decoded_payload_id = texture.decoded_payload_id;
    request.asset_class = texture.decode_asset_class;
    request.result_class = texture.decode_result_class;
    request.decoded_byte_count = texture.decoded_byte_count;

    std::uint32_t read_byte_count = 0U;
    const ResourceDecodedPayloadStatus status = registry.ReadDecodedPayload(
        request,
        out_bytes->data(),
        static_cast<std::uint32_t>(out_bytes->size()),
        &read_byte_count);
    if (status != ResourceDecodedPayloadStatus::Success || read_byte_count != texture.decoded_byte_count) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus BuildVisualProofTexturePayloadDesc(
    ResourceRegistry &registry,
    const RuntimeAssetLoadedFile &texture,
    std::uint32_t texture_index,
    RuntimeAssetCookedTexturePayloadDesc *out_desc) {
    if (out_desc == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (texture.kind != RuntimeAssetFileKind::Texture ||
        texture.texture_width == 0U ||
        texture.texture_height == 0U ||
        texture.payload_hash == 0U ||
        !texture.decoded_payload_stored ||
        texture.decoded_payload_id == 0U ||
        texture.decode_asset_class != ResourceDecodePlanAssetClass::Texture ||
        texture.decode_result_class != ResourceDecodeResultClass::Texture) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    std::vector<std::uint8_t> decoded_bytes{};
    RuntimeAssetDataStatus status =
        ReadVisualProofDecodedTexturePayload(registry, texture, &decoded_bytes);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    RuntimeAssetCookedTexturePayloadDesc desc{};
    desc.loaded_texture = &texture;
    desc.texture_desc.format = RhiFormat::Rgba8Unorm;
    desc.texture_desc.extent = {
        static_cast<std::uint16_t>(texture.texture_width),
        static_cast<std::uint16_t>(texture.texture_height)};
    desc.color_space = RuntimeAssetCookedTextureColorSpace::Linear;
    desc.row_pitch_bytes = RuntimeAssetRgba8RowPitch(desc.texture_desc);
    desc.slice_pitch_bytes = RuntimeAssetRgba8SlicePitch(desc.texture_desc);
    desc.payload_offset_bytes = 0U;
    desc.payload_byte_count = texture.decoded_byte_count;
    desc.payload_alignment_bytes = static_cast<std::uint32_t>(yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    desc.payload_hash = HashRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(decoded_bytes.data(), decoded_bytes.size()));
    desc.decoded_payload_id = texture.decoded_payload_id;
    desc.staging_request_id = texture.stable_id + STAGING_ID_OFFSET + texture_index + 1U;
    desc.upload_id = texture.stable_id + UPLOAD_ID_OFFSET + texture_index + 1U;
    *out_desc = desc;
    return RuntimeAssetDataStatus::Success;
}

std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT>
BuildVisualProofMaterialSlots() {
    std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT> slots{};
    for (std::uint32_t index = 0U; index < slots.size(); ++index) {
        slots[index].material_slot = index;
        slots[index].texture_payload_index = index;
        slots[index].expected_format = RhiFormat::Rgba8Unorm;
        slots[index].expected_color_space = RuntimeAssetCookedTextureColorSpace::Linear;
        slots[index].texture_binding_slot = index;
        slots[index].sampler_binding_slot = index;
        slots[index].sampler_desc = RhiSamplerDesc{};
    }

    return slots;
}

RuntimeAssetDataStatus BuildVisualProofMaterial(
    const RuntimeAssetVisualProofRequest &request,
    RenderSceneRuntimeMaterialRecord *out_material,
    RuntimeAssetVisualProofResult *result) {
    const RuntimeAssetLoadedFile *material = nullptr;
    RuntimeAssetDataStatus status = RequireVisualProofRecord(
        request,
        RuntimeAssetFileKind::Material,
        0U,
        RuntimeAssetVisualProofMissingLayer::MaterialSlot,
        &material,
        result);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    if (material == nullptr || material->texture_slot_count < RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT) {
        return FailVisualProof(
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetVisualProofMissingLayer::MaterialSlot,
            result);
    }

    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT> textures{};
    for (std::uint32_t index = 0U; index < textures.size(); ++index) {
        const RuntimeAssetLoadedFile *texture = nullptr;
        status = RequireVisualProofRecord(
            request,
            RuntimeAssetFileKind::Texture,
            index,
            RuntimeAssetVisualProofMissingLayer::MaterialSlot,
            &texture,
            result);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }

        status = BuildVisualProofTexturePayloadDesc(
            *request.resource_registry,
            *texture,
            index,
            &textures[index]);
        if (status != RuntimeAssetDataStatus::Success) {
            return FailVisualProof(status, RuntimeAssetVisualProofMissingLayer::MaterialSlot, result);
        }
    }

    const std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT> slots =
        BuildVisualProofMaterialSlots();

    RuntimeAssetCookedTextureMaterialBridgeRequest material_request{};
    material_request.resource_registry = request.resource_registry;
    material_request.asset_manager = request.asset_manager;
    material_request.rhi_device = request.rhi_device;
    material_request.material_asset = material->asset;
    material_request.material_id = RUNTIME_ASSET_VISUAL_PROOF_MATERIAL_ID;
    material_request.pipeline = result->shader_pipeline_result.pipeline;
    material_request.textures = std::span<const RuntimeAssetCookedTexturePayloadDesc>(textures.data(), textures.size());
    material_request.material_slots = std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size());
    material_request.scratch_bytes = request.scratch_bytes;
    material_request.out_material = out_material;

    status = BuildRuntimeAssetCookedTextureMaterialBridge(material_request, &result->material_result);
    if (status != RuntimeAssetDataStatus::Success) {
        return FailVisualProof(status, RuntimeAssetVisualProofMissingLayer::MaterialSlot, result);
    }

    result->runtime_texture_upload_count = result->material_result.runtime_texture_upload_count;
    result->material_texture_slot_count = result->material_result.material_texture_slot_count;
    result->material_slots_from_cooked_payloads =
        result->runtime_texture_upload_count == RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT &&
        result->material_texture_slot_count == RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus BuildVisualProofCamera(
    const RuntimeAssetVisualProofRequest &request,
    std::uint32_t frame_id,
    RenderSceneCameraBindingResult *out_camera,
    RuntimeAssetVisualProofResult *result) {
    if (out_camera == nullptr || request.scene_output == nullptr || request.scene_output->camera_count == 0U) {
        return FailVisualProof(
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetVisualProofMissingLayer::Camera,
            result);
    }

    const RuntimeAssetSceneCameraRecord *active_camera = nullptr;
    for (const RuntimeAssetSceneCameraRecord &camera : request.scene_cameras) {
        if (camera.is_active && camera.camera_id != 0U) {
            active_camera = &camera;
            break;
        }
    }

    if (active_camera == nullptr) {
        return FailVisualProof(
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetVisualProofMissingLayer::Camera,
            result);
    }

    RhiTextureHandle target{};
    RhiColorTargetDesc target_desc{};
    target_desc.format = RhiFormat::Rgba8Unorm;
    target_desc.extent = {
        static_cast<std::uint16_t>(RUNTIME_ASSET_VISUAL_PROOF_TARGET_WIDTH),
        static_cast<std::uint16_t>(RUNTIME_ASSET_VISUAL_PROOF_TARGET_HEIGHT)};
    if (request.rhi_device->CreateColorTarget(target_desc, target) != RhiStatus::Success) {
        return FailVisualProof(
            RuntimeAssetDataStatus::RhiCaptureFailed,
            RuntimeAssetVisualProofMissingLayer::RhiCapture,
            result);
    }

    RenderSceneRuntimeCameraRecord camera{};
    camera.camera_id = active_camera->camera_id;
    camera.pose.position = {-4.0F, 2.0F, -6.0F};
    camera.pose.target = {0.0F, 0.0F, 0.0F};
    camera.pose.up = {0.0F, 1.0F, 0.0F};
    camera.projection.kind = yuengine::rendercore::RenderCameraProjectionKind::Perspective;
    camera.projection.aspect_ratio = 1.0F;
    camera.projection.vertical_fov_radians = 1.0471975512F;
    camera.projection.near_z = 0.1F;
    camera.projection.far_z = 100.0F;
    camera.target = target;
    camera.clear_color = RhiColor{16U, 24U, 40U, 255U};
    camera.is_active = true;

    const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{camera};
    RenderSceneCameraBindingRequest camera_request{};
    camera_request.frame_id = frame_id;
    camera_request.active_camera_id = camera.camera_id;
    camera_request.cameras = std::span<const RenderSceneRuntimeCameraRecord>(cameras.data(), cameras.size());
    camera_request.capture_byte_budget =
        request.capture_byte_budget_per_entity * RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT;
    camera_request.capture_requested = true;

    RenderSceneCameraFrameBinder binder;
    const RenderSceneStatus status = binder.BuildActiveCameraFrame(camera_request, out_camera);
    if (status != RenderSceneStatus::Success) {
        return FailVisualProof(
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetVisualProofMissingLayer::Camera,
            result);
    }

    return RuntimeAssetDataStatus::Success;
}

const RuntimeAssetSceneTransformOutputRecord *FindSceneTransformOutput(
    const RuntimeAssetVisualProofRequest &request,
    WorldObjectId world_object_id) {
    for (const RuntimeAssetSceneTransformOutputRecord &transform : request.scene_transforms) {
        if (transform.world_object_id.value == world_object_id.value) {
            return &transform;
        }
    }

    return nullptr;
}

RuntimeAssetDataStatus ValidateVisualProofSceneOutputs(
    const RuntimeAssetVisualProofRequest &request,
    RuntimeAssetVisualProofResult *result) {
    if (request.scene == nullptr ||
        !IsLoadedRuntimeAssetRecordUsable(
            *request.scene,
            RuntimeAssetFileKind::Scene,
            request.require_cooked_records) ||
        request.scene_output == nullptr ||
        request.scene_output->status != RuntimeAssetDataStatus::Success ||
        request.scene_output->entity_count < RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT ||
        request.scene_output->transform_count < RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT ||
        request.scene_output->animation_sampled_value_count == 0U ||
        request.scene_output->animation_sample_status != AnimationRuntimeStatus::Success ||
        request.scene_output->animation_apply_status != AnimationRuntimeStatus::Success ||
        request.scene_entities.size() < RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT ||
        request.scene_transforms.size() < RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT) {
        return FailVisualProof(
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetVisualProofMissingLayer::SceneTransform,
            result);
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus BuildVisualProofEntities(
    const RuntimeAssetVisualProofRequest &request,
    std::span<const RenderScenePrimitiveGeometryRecord> geometry,
    std::array<RenderSceneThreePrimitiveEntityRequest, RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT> *out_entities,
    RuntimeAssetVisualProofResult *result) {
    if (out_entities == nullptr || geometry.size() < RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT) {
        return FailVisualProof(
            RuntimeAssetDataStatus::InvalidArgument,
            RuntimeAssetVisualProofMissingLayer::SceneTransform,
            result);
    }

    RuntimeAssetDataStatus status = ValidateVisualProofSceneOutputs(request, result);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    constexpr std::array<const char *, RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT> names{
        "Cube",
        "Cylinder",
        "Cone"};
    constexpr std::array<std::size_t, RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT> name_sizes{4U, 8U, 4U};

    for (std::size_t index = 0U; index < RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT; ++index) {
        const RuntimeAssetSceneEntityRecord &scene_entity = request.scene_entities[index];
        const RuntimeAssetSceneTransformOutputRecord *transform =
            FindSceneTransformOutput(request, scene_entity.world_object_id);
        if (transform == nullptr ||
            !scene_entity.world_object_id.IsValid() ||
            !scene_entity.is_visible ||
            !scene_entity.is_active) {
            return FailVisualProof(
                RuntimeAssetDataStatus::MissingDependency,
                RuntimeAssetVisualProofMissingLayer::SceneTransform,
                result);
        }

        RuntimeAssetSceneEntityRecord entity = scene_entity;
        entity.transform = transform->transform;
        (*out_entities)[index].world_object_id = entity.world_object_id;
        (*out_entities)[index].object_name = names[index];
        (*out_entities)[index].object_name_byte_count = name_sizes[index];
        (*out_entities)[index].transform = entity.transform;
        (*out_entities)[index].geometry = geometry[index];
        (*out_entities)[index].is_visible = entity.is_visible;
        (*out_entities)[index].is_active = entity.is_active;
    }

    result->scene_transforms_from_animation_sampling = true;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetVisualProofMissingLayer MapVisualProofCaptureLayer(
    RenderSceneThreePrimitiveCaptureMissingLayer layer) {
    switch (layer) {
        case RenderSceneThreePrimitiveCaptureMissingLayer::None:
            return RuntimeAssetVisualProofMissingLayer::None;
        case RenderSceneThreePrimitiveCaptureMissingLayer::Camera:
            return RuntimeAssetVisualProofMissingLayer::Camera;
        case RenderSceneThreePrimitiveCaptureMissingLayer::GeometryModel:
            return RuntimeAssetVisualProofMissingLayer::Model;
        case RenderSceneThreePrimitiveCaptureMissingLayer::MaterialTextureSlots:
            return RuntimeAssetVisualProofMissingLayer::MaterialSlot;
        case RenderSceneThreePrimitiveCaptureMissingLayer::ShaderPipeline:
            return RuntimeAssetVisualProofMissingLayer::ShaderPipeline;
        case RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement:
        case RenderSceneThreePrimitiveCaptureMissingLayer::RenderSceneSubmission:
            return RuntimeAssetVisualProofMissingLayer::SceneTransform;
        case RenderSceneThreePrimitiveCaptureMissingLayer::RenderCoreRhiDrawCapture:
        case RenderSceneThreePrimitiveCaptureMissingLayer::RhiCaptureTarget:
        case RenderSceneThreePrimitiveCaptureMissingLayer::OutputPath:
            return RuntimeAssetVisualProofMissingLayer::RhiCapture;
        default:
            break;
    }

    return RuntimeAssetVisualProofMissingLayer::RhiCapture;
}

RuntimeAssetDataStatus MapVisualProofCaptureStatus(
    const RenderSceneThreePrimitiveCaptureResult &capture_result) {
    const RuntimeAssetVisualProofMissingLayer layer =
        MapVisualProofCaptureLayer(capture_result.first_missing_layer);
    if (layer == RuntimeAssetVisualProofMissingLayer::Model ||
        layer == RuntimeAssetVisualProofMissingLayer::MaterialSlot ||
        layer == RuntimeAssetVisualProofMissingLayer::SceneTransform ||
        layer == RuntimeAssetVisualProofMissingLayer::Camera) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (layer == RuntimeAssetVisualProofMissingLayer::ShaderPipeline) {
        return RuntimeAssetDataStatus::RhiPipelineFailed;
    }

    return RuntimeAssetDataStatus::RhiCaptureFailed;
}

RuntimeAssetDataStatus ExecuteVisualProofCaptureFrames(
    const RuntimeAssetVisualProofRequest &request,
    const RenderSceneRuntimeMaterialRecord &material,
    std::span<const RenderSceneThreePrimitiveEntityRequest> entities,
    RuntimeAssetVisualProofResult *result) {
    if (request.frame_count == 0U || request.first_frame_id == 0U ||
        request.capture_byte_budget_per_entity == 0U ||
        request.capture_output.data() == nullptr ||
        request.output_path == nullptr ||
        request.output_path_byte_count == 0U) {
        return FailVisualProof(
            RuntimeAssetDataStatus::InvalidArgument,
            RuntimeAssetVisualProofMissingLayer::RhiCapture,
            result);
    }

    const std::size_t frame_capture_bytes =
        request.capture_byte_budget_per_entity * RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT;
    if (request.capture_output.size() < (frame_capture_bytes * request.frame_count)) {
        return FailVisualProof(
            RuntimeAssetDataStatus::InvalidArgument,
            RuntimeAssetVisualProofMissingLayer::RhiCapture,
            result);
    }

    RenderSceneThreePrimitiveCaptureRoute capture_route;
    for (std::uint32_t frame_index = 0U; frame_index < request.frame_count; ++frame_index) {
        RenderSceneCameraBindingResult camera{};
        RuntimeAssetDataStatus status = BuildVisualProofCamera(
            request,
            request.first_frame_id + frame_index,
            &camera,
            result);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }

        const std::size_t capture_offset = frame_capture_bytes * frame_index;
        RenderSceneThreePrimitiveCaptureRequest capture_request{};
        capture_request.frame_id = request.first_frame_id + frame_index;
        capture_request.camera = camera;
        capture_request.material = material;
        capture_request.entities = entities;
        capture_request.rhi_device = request.rhi_device;
        capture_request.output_path = request.output_path;
        capture_request.output_path_byte_count = request.output_path_byte_count;
        capture_request.capture_output =
            request.capture_output.subspan(capture_offset, frame_capture_bytes);
        capture_request.capture_byte_budget_per_entity = request.capture_byte_budget_per_entity;

        RenderSceneThreePrimitiveCaptureResult capture_result{};
        const RenderSceneThreePrimitiveCaptureStatus capture_status =
            capture_route.Execute(capture_request, &capture_result);
        result->capture_result = capture_result;
        if (capture_status != RenderSceneThreePrimitiveCaptureStatus::Success) {
            const RuntimeAssetVisualProofMissingLayer layer =
                MapVisualProofCaptureLayer(capture_result.first_missing_layer);
            return FailVisualProof(MapVisualProofCaptureStatus(capture_result), layer, result);
        }

        result->capture_bytes_written += capture_result.capture_bytes_written;
        result->submitted_draw_count += static_cast<std::uint32_t>(capture_result.frame_result.output_draw_count);
        ++result->completed_frame_count;
    }

    result->render_scene_routed =
        result->submitted_draw_count >=
        (RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT * result->completed_frame_count);
    result->render_core_rhi_capture_routed = result->capture_bytes_written > 0U;
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

RuntimeAssetDataStatus GenerateRuntimeAssetDeterministicDiskFixture(
    const RuntimeAssetDeterministicDiskFixtureRequest &request,
    RuntimeAssetDeterministicDiskFixtureResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetDeterministicDiskFixtureResult result{};
    result.source_file_count = RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT;
    result.cooked_file_count = RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT;
    result.source_scene = RuntimeAssetSourceSceneArtifact().desc;
    result.cooked_scene = RuntimeAssetCookedSceneArtifact().desc;
    result.source_graph_hash = FNV_OFFSET;
    result.cooked_graph_hash = FNV_OFFSET;

    if (request.mount_table == nullptr) {
        result.status = RuntimeAssetDataStatus::InvalidArgument;
        result.missing_layer = RuntimeAssetImportCookMissingLayer::FileVfs;
        *out_result = result;
        return result.status;
    }

    if (request.source_files == nullptr || request.cooked_files == nullptr) {
        result.status = RuntimeAssetDataStatus::InvalidArgument;
        result.missing_layer = RuntimeAssetImportCookMissingLayer::RuntimeAssetData;
        *out_result = result;
        return result.status;
    }

    if (request.source_file_capacity < RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        request.cooked_file_capacity < RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT) {
        result.status = RuntimeAssetDataStatus::CapacityExceeded;
        result.missing_layer = RuntimeAssetImportCookMissingLayer::RuntimeAssetData;
        *out_result = result;
        return result.status;
    }

    const std::array<RuntimeAssetFixtureArtifact, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> source_artifacts =
        RuntimeAssetSourceFixtureArtifacts();
    for (std::size_t index = 0U; index < source_artifacts.size(); ++index) {
        request.source_files[index] = source_artifacts[index].desc;
        const RuntimeAssetDataStatus status = WriteAndValidateFixtureArtifact(
            request,
            source_artifacts[index],
            static_cast<std::uint32_t>(index),
            &result.source_graph_hash,
            nullptr,
            &result);
        if (status != RuntimeAssetDataStatus::Success) {
            *out_result = result;
            return status;
        }

        ++result.source_artifact_write_count;
    }

    const RuntimeAssetFixtureArtifact source_scene = RuntimeAssetSourceSceneArtifact();
    result.source_scene = source_scene.desc;
    RuntimeAssetDataStatus status = WriteAndValidateFixtureArtifact(
        request,
        source_scene,
        RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT,
        &result.source_graph_hash,
        &result.source_scene_hash,
        &result);
    if (status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return status;
    }

    ++result.source_artifact_write_count;
    result.validated_source_files = true;

    const std::array<RuntimeAssetFixtureArtifact, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> cooked_artifacts =
        RuntimeAssetCookedFixtureArtifacts();
    for (std::size_t index = 0U; index < cooked_artifacts.size(); ++index) {
        request.cooked_files[index] = cooked_artifacts[index].desc;
        const RuntimeAssetDataStatus cooked_status = WriteAndValidateFixtureArtifact(
            request,
            cooked_artifacts[index],
            static_cast<std::uint32_t>(
                RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U + index),
            &result.cooked_graph_hash,
            nullptr,
            &result);
        if (cooked_status != RuntimeAssetDataStatus::Success) {
            *out_result = result;
            return cooked_status;
        }

        ++result.cooked_artifact_write_count;
    }

    const RuntimeAssetFixtureArtifact cooked_scene = RuntimeAssetCookedSceneArtifact();
    result.cooked_scene = cooked_scene.desc;
    status = WriteAndValidateFixtureArtifact(
        request,
        cooked_scene,
        (RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT * 2U) + 1U,
        &result.cooked_graph_hash,
        &result.cooked_scene_hash,
        &result);
    if (status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return status;
    }

    ++result.cooked_artifact_write_count;
    result.validated_cooked_files = true;
    result.wrote_to_disk = true;
    result.status = RuntimeAssetDataStatus::Success;
    result.missing_layer = RuntimeAssetImportCookMissingLayer::None;
    result.validation_status = RuntimeAssetDataStatus::Success;
    *out_result = result;
    return result.status;
}

RuntimeAssetDataStatus ExecuteRuntimeAssetImportCookCommand(
    const RuntimeAssetImportCookCommandRequest &request,
    RuntimeAssetImportCookCommandResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetImportCookCommandResult result{};
    if (request.command != RuntimeAssetImportCookCommandKind::GenerateDeterministicDiskFixture) {
        result.status = RuntimeAssetDataStatus::InvalidArgument;
        result.missing_layer = RuntimeAssetImportCookMissingLayer::Command;
        *out_result = result;
        return result.status;
    }

    const RuntimeAssetDataStatus status =
        GenerateRuntimeAssetDeterministicDiskFixture(request.fixture, &result.fixture);
    result.status = status;
    result.missing_layer = result.fixture.missing_layer;
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

RuntimeAssetDataStatus BuildRuntimeAssetCookedTextureMaterialBridge(
    const RuntimeAssetCookedTextureMaterialBridgeRequest &request,
    RuntimeAssetCookedTextureMaterialBridgeResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetCookedTextureMaterialBridgeResult result{};
    result.status = ValidateCookedTextureMaterialBridgeRequest(request, &result);
    if (result.status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return result.status;
    }

    result.status = BuildCookedTextureMaterialCommit(request, &result);
    *out_result = result;
    return result.status;
}

RuntimeAssetDataStatus BuildRuntimeAssetCookedShaderProgramPipeline(
    const RuntimeAssetCookedShaderProgramPipelineRequest &request,
    RuntimeAssetCookedShaderProgramPipelineResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetCookedShaderProgramPipelineResult result{};
    if (request.program != nullptr) {
        result.program_id = request.program->program_id;
        result.texture_slot_count = request.program->texture_slot_count;
        result.sampler_slot_count = request.program->sampler_slot_count;
        result.preflight_stage_count = request.program->stage_count;
    }

    const RuntimeAssetCookedShaderStagePayloadDesc *vertex_stage = nullptr;
    const RuntimeAssetCookedShaderStagePayloadDesc *pixel_stage = nullptr;
    result.status = ValidateCookedShaderProgramRequest(request, &vertex_stage, &pixel_stage);
    if (result.status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return result.status;
    }

    const std::span<const std::uint8_t> vertex_bytecode = CookedShaderStageBytecodeSpan(*vertex_stage);
    const std::span<const std::uint8_t> pixel_bytecode = CookedShaderStageBytecodeSpan(*pixel_stage);
    result.vertex_bytecode_hash = vertex_stage->bytecode_hash;
    result.pixel_bytecode_hash = pixel_stage->bytecode_hash;

    yuengine::rhi::RhiShaderModuleDesc vertex_desc{};
    vertex_desc.stage = yuengine::rhi::RhiShaderStage::Vertex;
    vertex_desc.bytecode = vertex_bytecode;
    yuengine::rhi::RhiShaderModuleHandle vertex_shader{};
    const yuengine::rhi::RhiStatus vertex_status =
        request.device->CreateShaderModule(vertex_desc, vertex_shader);
    if (vertex_status != yuengine::rhi::RhiStatus::Success) {
        result.status = RuntimeAssetDataStatus::RhiShaderModuleFailed;
        *out_result = result;
        return result.status;
    }
    ++result.created_shader_module_count;

    yuengine::rhi::RhiShaderModuleDesc pixel_desc{};
    pixel_desc.stage = yuengine::rhi::RhiShaderStage::Pixel;
    pixel_desc.bytecode = pixel_bytecode;
    yuengine::rhi::RhiShaderModuleHandle pixel_shader{};
    const yuengine::rhi::RhiStatus pixel_status =
        request.device->CreateShaderModule(pixel_desc, pixel_shader);
    if (pixel_status != yuengine::rhi::RhiStatus::Success) {
        request.device->DestroyShaderModule(vertex_shader);
        ++result.destroyed_shader_module_count;
        result.status = RuntimeAssetDataStatus::RhiShaderModuleFailed;
        *out_result = result;
        return result.status;
    }
    ++result.created_shader_module_count;

    yuengine::rhi::RhiPipelineDesc pipeline_desc{};
    pipeline_desc.vertex_shader = vertex_shader;
    pipeline_desc.pixel_shader = pixel_shader;
    pipeline_desc.input_layout = request.program->input_layout;
    yuengine::rhi::RhiPipelineHandle pipeline{};
    const yuengine::rhi::RhiStatus pipeline_status =
        request.device->CreatePipeline(pipeline_desc, pipeline);
    if (pipeline_status != yuengine::rhi::RhiStatus::Success) {
        request.device->DestroyShaderModule(pixel_shader);
        request.device->DestroyShaderModule(vertex_shader);
        result.destroyed_shader_module_count += 2U;
        result.status = RuntimeAssetDataStatus::RhiPipelineFailed;
        *out_result = result;
        return result.status;
    }

    result.vertex_shader = vertex_shader;
    result.pixel_shader = pixel_shader;
    result.pipeline = pipeline;
    result.pipeline_desc = pipeline_desc;
    result.published_handles = true;
    result.status = RuntimeAssetDataStatus::Success;
    *out_result = result;
    return result.status;
}

RuntimeAssetDataStatus BuildRuntimeAssetCookedVisualProofRoute(
    const RuntimeAssetVisualProofRequest &request,
    RuntimeAssetVisualProofResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetVisualProofResult result{};
    SeedVisualProofLedger(request, &result);

    if (request.resource_registry == nullptr ||
        request.asset_manager == nullptr ||
        request.loaded_files.data() == nullptr ||
        request.loaded_files.empty() ||
        request.scene_entities.data() == nullptr ||
        request.scene_transforms.data() == nullptr ||
        request.scene_cameras.data() == nullptr ||
        request.scratch_bytes.data() == nullptr) {
        result.status = RuntimeAssetDataStatus::InvalidArgument;
        *out_result = result;
        return result.status;
    }

    if (request.rhi_device == nullptr) {
        result.status = RuntimeAssetDataStatus::RhiCaptureFailed;
        result.first_missing_layer = RuntimeAssetVisualProofMissingLayer::RhiCapture;
        *out_result = result;
        return result.status;
    }

    RuntimeAssetDataStatus status = ValidateVisualProofSceneOutputs(request, &result);
    if (status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return status;
    }

    status = BuildVisualProofShaderPipeline(request, &result);
    if (status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return status;
    }

    RhiBufferHandle buffer_slot_guard{};
    status = CreateVisualProofBuffer(
        *request.rhi_device,
        RhiBufferUsage::Vertex,
        sizeof(float) * 2U,
        &buffer_slot_guard);
    if (status != RuntimeAssetDataStatus::Success) {
        FailVisualProof(status, RuntimeAssetVisualProofMissingLayer::RhiCapture, &result);
        *out_result = result;
        return status;
    }

    std::array<RenderScenePrimitiveGeometryRecord, RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT> geometry{};
    const std::size_t vertex_stride_bytes = sizeof(float) * 2U;
    for (std::uint32_t index = 0U; index < geometry.size(); ++index) {
        const RuntimeAssetLoadedFile *mesh = nullptr;
        status = RequireVisualProofRecord(
            request,
            RuntimeAssetFileKind::Mesh,
            index,
            RuntimeAssetVisualProofMissingLayer::Model,
            &mesh,
            &result);
        if (status != RuntimeAssetDataStatus::Success) {
            *out_result = result;
            return status;
        }

        status = BuildVisualProofGeometry(
            *mesh,
            index,
            vertex_stride_bytes,
            *request.rhi_device,
            &geometry[index],
            &result);
        if (status != RuntimeAssetDataStatus::Success) {
            *out_result = result;
            return status;
        }
    }

    RenderSceneRuntimeMaterialRecord material{};
    status = BuildVisualProofMaterial(request, &material, &result);
    if (status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return status;
    }

    std::array<RenderSceneThreePrimitiveEntityRequest, RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT> entities{};
    status = BuildVisualProofEntities(
        request,
        std::span<const RenderScenePrimitiveGeometryRecord>(geometry.data(), geometry.size()),
        &entities,
        &result);
    if (status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return status;
    }

    status = ExecuteVisualProofCaptureFrames(
        request,
        material,
        std::span<const RenderSceneThreePrimitiveEntityRequest>(entities.data(), entities.size()),
        &result);
    if (status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return status;
    }

    result.loaded_records_verified =
        (!request.require_cooked_records || result.source_record_count == 0U) &&
        result.cooked_record_count >= (request.loaded_files.size() + 1U) &&
        result.mesh_record_count >= RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT &&
        result.texture_record_count >= RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT;
    result.status = RuntimeAssetDataStatus::Success;
    result.first_missing_layer = RuntimeAssetVisualProofMissingLayer::None;
    *out_result = result;
    return result.status;
}

}
