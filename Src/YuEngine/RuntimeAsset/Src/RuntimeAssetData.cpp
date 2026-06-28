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
#include "YuEngine/Kernel/EngineKernel.h"
#include "YuEngine/Kernel/IModule.h"
#include "YuEngine/Kernel/KernelResult.h"
#include "YuEngine/Kernel/KernelStatus.h"
#include "YuEngine/Kernel/RuntimeApp.h"
#include "YuEngine/Package/PackageLoadPlanResult.h"
#include "YuEngine/Package/PackageRegistry.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingRequest.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingResult.h"
#include "YuEngine/RenderScene/RenderSceneCameraFrameBinder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameBuilder.h"
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
using yuengine::kernel::EngineKernel;
using yuengine::kernel::IModule;
using yuengine::kernel::KernelResult;
using yuengine::kernel::KernelStatus;
using yuengine::kernel::RuntimeApp;
using yuengine::kernel::RuntimeAppRunResult;
using yuengine::kernel::RuntimeFramePhase;
using yuengine::package::PackageArtifactReadRequest;
using yuengine::package::PackageArtifactResult;
using yuengine::package::PackageLoadPlanRecord;
using yuengine::package::PackageLoadPlanResult;
using yuengine::package::PackageRegistry;
using yuengine::package::PackageStatus;
using yuengine::package::ReadPackageArtifact;
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
using yuengine::renderscene::RenderSceneRuntimeFrameBuilder;
using yuengine::renderscene::RenderSceneRuntimeFrameEntityRequest;
using yuengine::renderscene::RenderSceneRuntimeFrameRequest;
using yuengine::renderscene::RenderSceneRuntimeFrameStatus;
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
using yuengine::rhi::RhiBlendMode;
using yuengine::rhi::RhiBlendStateDesc;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiSamplerBinding;
using yuengine::rhi::RhiSamplerDesc;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiSampledTextureBinding;
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
constexpr std::uint32_t DEFAULT_RESIDENCY_BYTE_CAPACITY = 16384U;
constexpr std::uint32_t DEFAULT_PAYLOAD_BYTE_CAPACITY = 8192U;
constexpr std::uint32_t RUNTIME_ASSET_MATERIAL_PARAMETER_COUNT = 5U;
constexpr std::uint32_t RUNTIME_ASSET_SHADER_IMPORT_POLICY_FIELD_COUNT = 7U;
constexpr std::uint32_t RUNTIME_ASSET_MESH_VERTEX_STRIDE_BYTES = 16U;
constexpr std::uint32_t RUNTIME_ASSET_MESH_INDEX_STRIDE_BYTES = 2U;
constexpr std::string_view RUNTIME_ASSET_SOURCE_SCHEMA = "rav0-source";
constexpr std::string_view RUNTIME_ASSET_COOKED_SCHEMA = "rav1-cooked";
constexpr std::string_view SHADER_BYTECODE_PREFIX = "bytecode:";
constexpr std::string_view SHADER_BYTECODE_BASE64_PREFIX = "b64:";
constexpr std::string_view SHADER_BYTECODE_HEX_PREFIX = "hex:";
constexpr std::string_view SHADER_LAYOUT_PREFIX = "layout:";
constexpr std::string_view SHADER_LAYOUT_NONE = "none";
constexpr std::string_view RUNTIME_ASSET_D3D11_VISUAL_PROOF_VERTEX_SHADER_BASE64 =
    "RFhCQwc2QLD2f/IqqN7bCwxpOPUBAAAA3AAAAAMAAAAsAAAAYAAAAJQAAABJU0dOLAAAAAEAAAAIAAAAIAAAAAAAAA"
    "AGAAAAAQAAAAAAAAABAAAAU1ZfVmVydGV4SUQAT1NHTiwAAAABAAAACAAAACAAAAAAAAAAAQAAAAMAAAAAAAAADwAA"
    "AFNWX1Bvc2l0aW9uAFNIRVhAAAAAUAABABAAAABqCAABZwAABPIgEAAAAAAAAQAAADYAAAjyIBAAAAAAAAJAAAAAAA"
    "AAAAAAAAAAAAAAAIA/PgAAAQ==";
constexpr std::string_view RUNTIME_ASSET_D3D11_VISUAL_PROOF_PIXEL_SHADER_BASE64 =
    "RFhCQ+5NC5Qp7gIBY6Rd809W4bEBAAAAtAAAAAMAAAAsAAAAPAAAAHAAAABJU0dOCAAAAAAAAAAIAAAAT1NHTiwAAA"
    "ABAAAACAAAACAAAAAAAAAAAAAAAAMAAAAAAAAADwAAAFNWX1RhcmdldACrq1NIRVg8AAAAUAAAAA8AAABqCAABZQAA"
    "A/IgEAAAAAAANgAACPIgEAAAAAAAAkAAAAAAgD8AAAAAAAAAAAAAgD8+AAAB";
constexpr std::uint32_t RUNTIME_ASSET_SCENE_ENTITY_COUNT = 3U;
constexpr std::uint32_t RUNTIME_ASSET_SCENE_CAMERA_COUNT = 1U;
constexpr std::uint32_t RUNTIME_ASSET_SCENE_TRANSFORM_COUNT = 3U;
constexpr std::uint32_t MAX_RUNTIME_ASSET_DEPENDENCY_ROWS = 64U;
constexpr std::uint32_t MAX_RUNTIME_ASSET_RECORD_TABLES = 16U;
constexpr std::uint32_t MAX_RUNTIME_ASSET_RECORD_BYTES = 4096U;
constexpr std::uint32_t MAX_RUNTIME_ASSET_PAYLOAD_BYTES = 4096U;
constexpr std::uint32_t RUNTIME_ASSET_MAX_SCENE_ENTITY_COUNT = 64U;
constexpr std::uint32_t RUNTIME_ASSET_MAX_SCENE_CAMERA_COUNT = 4U;
constexpr std::uint32_t RUNTIME_ASSET_MAX_TARGET_IDENTITY_COUNT = 256U;
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

bool ParseAnimationInterpolation(
    std::string_view value,
    AnimationRuntimeInterpolation *out_interpolation) {
    if (out_interpolation == nullptr) {
        return false;
    }

    if (value.empty() || value == "linear") {
        *out_interpolation = AnimationRuntimeInterpolation::Linear;
        return true;
    }

    if (value == "step") {
        *out_interpolation = AnimationRuntimeInterpolation::Step;
        return true;
    }

    return false;
}

bool ParseRuntimeAssetAnimationTargetProperty(
    std::string_view value,
    RuntimeAssetAnimationTargetProperty *out_property) {
    if (out_property == nullptr) {
        return false;
    }

    if (value == "transform:translation_x") {
        *out_property = RuntimeAssetAnimationTargetProperty::TransformTranslationX;
        return true;
    }

    if (value == "transform:translation_y") {
        *out_property = RuntimeAssetAnimationTargetProperty::TransformTranslationY;
        return true;
    }

    if (value == "transform:translation_z") {
        *out_property = RuntimeAssetAnimationTargetProperty::TransformTranslationZ;
        return true;
    }

    if (value == "transform:rotation_x") {
        *out_property = RuntimeAssetAnimationTargetProperty::TransformRotationX;
        return true;
    }

    if (value == "transform:rotation_y") {
        *out_property = RuntimeAssetAnimationTargetProperty::TransformRotationY;
        return true;
    }

    if (value == "transform:rotation_z") {
        *out_property = RuntimeAssetAnimationTargetProperty::TransformRotationZ;
        return true;
    }

    if (value == "transform:rotation_w") {
        *out_property = RuntimeAssetAnimationTargetProperty::TransformRotationW;
        return true;
    }

    if (value == "transform:scale_x") {
        *out_property = RuntimeAssetAnimationTargetProperty::TransformScaleX;
        return true;
    }

    if (value == "transform:scale_y") {
        *out_property = RuntimeAssetAnimationTargetProperty::TransformScaleY;
        return true;
    }

    if (value == "transform:scale_z") {
        *out_property = RuntimeAssetAnimationTargetProperty::TransformScaleZ;
        return true;
    }

    return false;
}

bool RuntimeAssetAnimationTargetPropertyToChannel(
    RuntimeAssetAnimationTargetProperty property,
    AnimationRuntimeChannel *out_channel) {
    if (out_channel == nullptr) {
        return false;
    }

    switch (property) {
        case RuntimeAssetAnimationTargetProperty::TransformTranslationX:
            *out_channel = AnimationRuntimeChannel::TranslationX;
            return true;
        case RuntimeAssetAnimationTargetProperty::TransformTranslationY:
            *out_channel = AnimationRuntimeChannel::TranslationY;
            return true;
        case RuntimeAssetAnimationTargetProperty::TransformTranslationZ:
            *out_channel = AnimationRuntimeChannel::TranslationZ;
            return true;
        case RuntimeAssetAnimationTargetProperty::TransformRotationX:
            *out_channel = AnimationRuntimeChannel::RotationX;
            return true;
        case RuntimeAssetAnimationTargetProperty::TransformRotationY:
            *out_channel = AnimationRuntimeChannel::RotationY;
            return true;
        case RuntimeAssetAnimationTargetProperty::TransformRotationZ:
            *out_channel = AnimationRuntimeChannel::RotationZ;
            return true;
        case RuntimeAssetAnimationTargetProperty::TransformRotationW:
            *out_channel = AnimationRuntimeChannel::RotationW;
            return true;
        case RuntimeAssetAnimationTargetProperty::TransformScaleX:
            *out_channel = AnimationRuntimeChannel::ScaleX;
            return true;
        case RuntimeAssetAnimationTargetProperty::TransformScaleY:
            *out_channel = AnimationRuntimeChannel::ScaleY;
            return true;
        case RuntimeAssetAnimationTargetProperty::TransformScaleZ:
            *out_channel = AnimationRuntimeChannel::ScaleZ;
            return true;
        case RuntimeAssetAnimationTargetProperty::Unknown:
        default:
            break;
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
    std::array<RuntimeAssetTargetIdentityRecord, RUNTIME_ASSET_MAX_TARGET_IDENTITY_COUNT> target_identities{};
    std::array<RuntimeAssetRuntimeInstanceMappingRecord, RUNTIME_ASSET_MAX_TARGET_IDENTITY_COUNT>
        runtime_instance_mappings{};
    std::array<AnimationRuntimeClipRecord, RUNTIME_ASSET_MAX_ANIMATION_CLIP_COUNT> animation_clips{};
    std::array<AnimationRuntimeTrackRecord, RUNTIME_ASSET_MAX_ANIMATION_TRACK_COUNT> animation_tracks{};
    std::array<RuntimeAssetAnimationTrackTargetBindingRecord, RUNTIME_ASSET_MAX_ANIMATION_TRACK_COUNT>
        animation_target_bindings{};
    std::array<AnimationRuntimeKeyframeRecord, RUNTIME_ASSET_MAX_ANIMATION_KEYFRAME_COUNT> animation_keyframes{};
    std::uint32_t camera_count = 0U;
    std::uint32_t entity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t target_identity_count = 0U;
    std::uint32_t runtime_instance_mapping_count = 0U;
    std::uint32_t animation_clip_count = 0U;
    std::uint32_t animation_track_count = 0U;
    std::uint32_t animation_target_binding_count = 0U;
    std::uint32_t animation_keyframe_count = 0U;
    std::uint32_t animation_sampled_value_count = 0U;
    std::uint32_t selected_animation_clip_id = 0U;
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

bool SceneStageHasRuntimeAnimationTarget(
    const RuntimeAssetSceneLoaderStage &stage,
    std::uint32_t target_value) {
    for (std::uint32_t index = 0U; index < stage.entity_count; ++index) {
        if (stage.entities[index].world_object_id.value == target_value) {
            return true;
        }
    }

    return false;
}

bool HasTargetIdentityOutputRequest(const RuntimeAssetGraphLoadRequest &request) {
    if (request.target_identities != nullptr) {
        return true;
    }

    return request.target_identity_capacity != 0U;
}

bool HasRuntimeInstanceMappingOutputRequest(const RuntimeAssetGraphLoadRequest &request) {
    if (request.runtime_instance_mappings != nullptr) {
        return true;
    }

    return request.runtime_instance_mapping_capacity != 0U;
}

RuntimeAssetDataStatus ValidateTargetIdentityOutputRequest(
    const RuntimeAssetGraphLoadRequest &request,
    std::uint32_t target_identity_count) {
    if (!HasTargetIdentityOutputRequest(request)) {
        return RuntimeAssetDataStatus::Success;
    }

    if (request.target_identities == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.target_identity_capacity < target_identity_count) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateRuntimeInstanceMappingOutputRequest(
    const RuntimeAssetGraphLoadRequest &request,
    std::uint32_t mapping_count) {
    if (!HasRuntimeInstanceMappingOutputRequest(request)) {
        return RuntimeAssetDataStatus::Success;
    }

    if (request.runtime_instance_mappings == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.runtime_instance_mapping_capacity < mapping_count) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    return RuntimeAssetDataStatus::Success;
}

bool HasAnimationRuntimeOutputRequest(const RuntimeAssetGraphLoadRequest &request) {
    if (request.animation_clips != nullptr) {
        return true;
    }

    if (request.animation_tracks != nullptr) {
        return true;
    }

    if (request.animation_keyframes != nullptr) {
        return true;
    }

    if (request.animation_clip_capacity != 0U) {
        return true;
    }

    if (request.animation_track_capacity != 0U) {
        return true;
    }

    return request.animation_keyframe_capacity != 0U;
}

bool HasAnimationTargetBindingOutputRequest(const RuntimeAssetGraphLoadRequest &request) {
    if (request.animation_target_bindings != nullptr) {
        return true;
    }

    return request.animation_target_binding_capacity != 0U;
}

RuntimeAssetDataStatus ValidateAnimationRuntimeOutputRequest(
    const RuntimeAssetGraphLoadRequest &request,
    std::uint32_t clip_count,
    std::uint32_t track_count,
    std::uint32_t keyframe_count) {
    if (!HasAnimationRuntimeOutputRequest(request)) {
        return RuntimeAssetDataStatus::Success;
    }

    if (request.animation_clips == nullptr ||
        request.animation_tracks == nullptr ||
        request.animation_keyframes == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.animation_clip_capacity < clip_count ||
        request.animation_track_capacity < track_count ||
        request.animation_keyframe_capacity < keyframe_count) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateAnimationTargetBindingOutputRequest(
    const RuntimeAssetGraphLoadRequest &request,
    std::uint32_t binding_count) {
    if (!HasAnimationTargetBindingOutputRequest(request)) {
        return RuntimeAssetDataStatus::Success;
    }

    if (request.animation_target_bindings == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.animation_target_binding_capacity < binding_count) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    return RuntimeAssetDataStatus::Success;
}

void CommitTargetIdentityRecords(
    const RuntimeAssetGraphLoadRequest &request,
    const RuntimeAssetSceneLoaderStage &stage) {
    if (!HasTargetIdentityOutputRequest(request)) {
        return;
    }

    for (std::uint32_t index = 0U; index < stage.target_identity_count; ++index) {
        request.target_identities[index] = stage.target_identities[index];
    }
}

void CommitRuntimeInstanceMappingRecords(
    const RuntimeAssetGraphLoadRequest &request,
    const RuntimeAssetSceneLoaderStage &stage) {
    if (!HasRuntimeInstanceMappingOutputRequest(request)) {
        return;
    }

    for (std::uint32_t index = 0U; index < stage.runtime_instance_mapping_count; ++index) {
        request.runtime_instance_mappings[index] = stage.runtime_instance_mappings[index];
    }
}

void CommitAnimationRuntimeTables(
    const RuntimeAssetGraphLoadRequest &request,
    const RuntimeAssetSceneLoaderStage &stage) {
    if (!HasAnimationRuntimeOutputRequest(request)) {
        return;
    }

    for (std::uint32_t index = 0U; index < stage.animation_clip_count; ++index) {
        request.animation_clips[index] = stage.animation_clips[index];
    }

    for (std::uint32_t index = 0U; index < stage.animation_track_count; ++index) {
        request.animation_tracks[index] = stage.animation_tracks[index];
    }

    for (std::uint32_t index = 0U; index < stage.animation_keyframe_count; ++index) {
        request.animation_keyframes[index] = stage.animation_keyframes[index];
    }
}

void CommitAnimationTargetBindingRecords(
    const RuntimeAssetGraphLoadRequest &request,
    const RuntimeAssetSceneLoaderStage &stage) {
    if (!HasAnimationTargetBindingOutputRequest(request)) {
        return;
    }

    for (std::uint32_t index = 0U; index < stage.animation_target_binding_count; ++index) {
        request.animation_target_bindings[index] = stage.animation_target_bindings[index];
    }
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
        case RuntimeAssetFileKind::Camera:
            return "camera";
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
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_CAMERA = 107U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_MESH = 201U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_MATERIAL = 202U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_TEXTURE = 203U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_SHADER = 204U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_SCENE = 205U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_ANIMATION = 206U;
constexpr std::uint32_t RUNTIME_ASSET_FIXTURE_ASSET_TYPE_CAMERA = 207U;

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
        case RuntimeAssetFileKind::Camera:
            return "camera";
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
        case RuntimeAssetFileKind::Camera:
            desc.resource_type = ResourceTypeId{RUNTIME_ASSET_FIXTURE_RESOURCE_TYPE_CAMERA};
            desc.asset_type = yuengine::asset::AssetTypeId{RUNTIME_ASSET_FIXTURE_ASSET_TYPE_CAMERA};
            break;
        case RuntimeAssetFileKind::Unknown:
            break;
    }

    return desc;
}

std::string RuntimeAssetMeshPayload(char seed, std::uint32_t byte_count) {
    std::string payload{};
    payload.reserve(byte_count);
    for (std::uint32_t index = 0U; index < byte_count; ++index) {
        const int payload_value = static_cast<int>(seed) + static_cast<int>(index % 10U);
        const char value = static_cast<char>(payload_value);
        payload.push_back(value);
    }

    return payload;
}

constexpr std::uint32_t RuntimeAssetMeshVertexPayloadByteCount(std::uint32_t vertex_count) {
    return vertex_count * RUNTIME_ASSET_MESH_VERTEX_STRIDE_BYTES;
}

constexpr std::uint32_t RuntimeAssetMeshIndexPayloadByteCount(std::uint32_t index_count) {
    return index_count * RUNTIME_ASSET_MESH_INDEX_STRIDE_BYTES;
}

constexpr std::uint32_t RuntimeAssetMeshPayloadByteCount(
    std::uint32_t vertex_count,
    std::uint32_t index_count) {
    return RuntimeAssetMeshVertexPayloadByteCount(vertex_count) +
           RuntimeAssetMeshIndexPayloadByteCount(index_count);
}

std::string RuntimeAssetMeshLayoutFields() {
    std::string text(
        "input=layout:position,texcoord\n"
        "vertexStrideBytes=");
    text += std::to_string(RUNTIME_ASSET_MESH_VERTEX_STRIDE_BYTES);
    text += "\nindexFormat=uint16\nindexStrideBytes=";
    text += std::to_string(RUNTIME_ASSET_MESH_INDEX_STRIDE_BYTES);
    text += "\ntopology=triangle_list\n";
    return text;
}

std::string RuntimeAssetSourceMeshText(
    std::string_view id,
    std::string_view shape,
    std::uint32_t vertex_count,
    std::uint32_t index_count,
    std::uint32_t vertex_payload_byte_count,
    std::uint32_t index_payload_byte_count,
    std::string_view payload) {
    std::string text("YUASSET MESH 1\nschema=rav0-source\nid=");
    text += id;
    text += "\nkind=";
    text += shape;
    text += "\nvertices=";
    text += std::to_string(vertex_count);
    text += "\nindices=";
    text += std::to_string(index_count);
    text += "\nbounds=-1,-1,-1,1,1,1\nvertexPayloadBytes=";
    text += std::to_string(vertex_payload_byte_count);
    text += "\nindexPayloadBytes=";
    text += std::to_string(index_payload_byte_count);
    text += "\n";
    text += RuntimeAssetMeshLayoutFields();
    text += "payloadBytes=";
    text += std::to_string(payload.size());
    text += "\npayloadAlign=4\npayloadHash=";
    text += std::to_string(HashRuntimeAssetText(payload));
    text += "\npayload=";
    text += payload;
    text += "\n";
    return text;
}

std::string RuntimeAssetMaterialParameterFields() {
    return std::string(
        "parameterCount=5\n"
        "baseColorRgba=32,48,64,192\n"
        "emissiveStrength=64\n"
        "metallic=128\n"
        "roughness=96\n"
        "opacity=192\n"
        "alphaMode=blend\n");
}

std::string RuntimeAssetSourceMaterialText() {
    std::string text(
        "YUASSET MATERIAL 1\n"
        "schema=rav0-source\n"
        "id=shared_material\n"
        "shader=Shader/RuntimeProgram.yuprogram\n"
        "texture0=Texture/Albedo.yutex\n"
        "texture1=Texture/Normal.yutex\n"
        "texture2=Texture/Mask.yutex\n");
    text += RuntimeAssetMaterialParameterFields();
    return text;
}

std::string RuntimeAssetShaderImportPolicyFields() {
    return std::string(
        "importLanguage=hlsl\n"
        "importTarget=d3d11\n"
        "entry_vs=VSMain\n"
        "entry_ps=PSMain\n"
        "profile_vs=vs_5_0\n"
        "profile_ps=ps_5_0\n"
        "compileFlags=deterministic\n");
}

std::string RuntimeAssetSourceShaderText() {
    std::string text(
        "YUASSET SHADER 1\n"
        "schema=rav0-source\n"
        "id=runtime_program\n");
    text += RuntimeAssetShaderImportPolicyFields();
    text +=
        "stage_vs=bytecode:runtime_program_vs\n"
        "stage_ps=bytecode:runtime_program_ps\n"
        "input=layout:position,color\n"
        "textures=3\n";
    return text;
}

std::array<RuntimeAssetFixtureArtifact, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT>
RuntimeAssetSourceFixtureArtifacts() {
    constexpr std::uint32_t cube_vertex_count = 24U;
    constexpr std::uint32_t cube_index_count = 36U;
    constexpr std::uint32_t cylinder_vertex_count = 18U;
    constexpr std::uint32_t cylinder_index_count = 96U;
    constexpr std::uint32_t cone_vertex_count = 10U;
    constexpr std::uint32_t cone_index_count = 48U;
    constexpr std::uint32_t cube_vertex_payload_byte_count =
        RuntimeAssetMeshVertexPayloadByteCount(cube_vertex_count);
    constexpr std::uint32_t cube_index_payload_byte_count =
        RuntimeAssetMeshIndexPayloadByteCount(cube_index_count);
    constexpr std::uint32_t cylinder_vertex_payload_byte_count =
        RuntimeAssetMeshVertexPayloadByteCount(cylinder_vertex_count);
    constexpr std::uint32_t cylinder_index_payload_byte_count =
        RuntimeAssetMeshIndexPayloadByteCount(cylinder_index_count);
    constexpr std::uint32_t cone_vertex_payload_byte_count =
        RuntimeAssetMeshVertexPayloadByteCount(cone_vertex_count);
    constexpr std::uint32_t cone_index_payload_byte_count =
        RuntimeAssetMeshIndexPayloadByteCount(cone_index_count);
    constexpr std::uint32_t cube_payload_byte_count =
        RuntimeAssetMeshPayloadByteCount(cube_vertex_count, cube_index_count);
    constexpr std::uint32_t cylinder_payload_byte_count =
        RuntimeAssetMeshPayloadByteCount(cylinder_vertex_count, cylinder_index_count);
    constexpr std::uint32_t cone_payload_byte_count =
        RuntimeAssetMeshPayloadByteCount(cone_vertex_count, cone_index_count);
    const std::string cube_payload =
        RuntimeAssetMeshPayload('A', cube_payload_byte_count);
    const std::string cylinder_payload =
        RuntimeAssetMeshPayload('K', cylinder_payload_byte_count);
    const std::string cone_payload =
        RuntimeAssetMeshPayload('U', cone_payload_byte_count);
    return std::array<RuntimeAssetFixtureArtifact, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT>{
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc(
                "Mesh/Cube.yumesh",
                RuntimeAssetFileKind::Mesh,
                1001U,
                cube_payload_byte_count),
            RuntimeAssetSourceMeshText(
                "cube_mesh",
                "cube",
                cube_vertex_count,
                cube_index_count,
                cube_vertex_payload_byte_count,
                cube_index_payload_byte_count,
                cube_payload)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc(
                "Mesh/Cylinder.yumesh",
                RuntimeAssetFileKind::Mesh,
                1002U,
                cylinder_payload_byte_count),
            RuntimeAssetSourceMeshText(
                "cylinder_mesh",
                "cylinder",
                cylinder_vertex_count,
                cylinder_index_count,
                cylinder_vertex_payload_byte_count,
                cylinder_index_payload_byte_count,
                cylinder_payload)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc(
                "Mesh/Cone.yumesh",
                RuntimeAssetFileKind::Mesh,
                1003U,
                cone_payload_byte_count),
            RuntimeAssetSourceMeshText(
                "cone_mesh",
                "cone",
                cone_vertex_count,
                cone_index_count,
                cone_vertex_payload_byte_count,
                cone_index_payload_byte_count,
                cone_payload)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Material/Shared.yumat", RuntimeAssetFileKind::Material, 2001U, 128U),
            RuntimeAssetSourceMaterialText()},
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
            RuntimeAssetSourceShaderText()},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Animation/Spin.yuanim", RuntimeAssetFileKind::Animation, 5001U, 0U),
            "YUASSET ANIMATION 1\nschema=rav0-source\nid=spin\nclip=1\nduration=1\ntarget=scene_entity:101\ntrack=transform:rotation_y\nkey0=0:0\nkey1=1:1\ntracks=1\nsample_rate=30\n"},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Camera/Main.yucamera", RuntimeAssetFileKind::Camera, 7001U, 0U),
            "YUASSET CAMERA 1\nschema=rav0-source\nid=main_camera\nprojection=perspective\nfov_degrees=55\nnear=0.1\nfar=100\nkeyframes=3\nkey0=0:-4,2,-6:0,0,0\nkey1=0.5:0,3,-5:0,0,0\nkey2=1:4,2,-6:0,0,0\n"}};
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
        "cam=Camera/Main.yucamera\n"
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

std::string RuntimeAssetCookedVisualProofShaderFields() {
    std::string text = RuntimeAssetShaderImportPolicyFields();
    text += "stage_vs=bytecode:b64:";
    text += RUNTIME_ASSET_D3D11_VISUAL_PROOF_VERTEX_SHADER_BASE64;
    text += "\nstage_ps=bytecode:b64:";
    text += RUNTIME_ASSET_D3D11_VISUAL_PROOF_PIXEL_SHADER_BASE64;
    text += "\ninput=layout:position,texcoord\ntextures=3\n";
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
    constexpr std::uint32_t cube_vertex_count = 24U;
    constexpr std::uint32_t cube_index_count = 36U;
    constexpr std::uint32_t cylinder_vertex_count = 18U;
    constexpr std::uint32_t cylinder_index_count = 96U;
    constexpr std::uint32_t cone_vertex_count = 10U;
    constexpr std::uint32_t cone_index_count = 48U;
    constexpr std::uint32_t cube_vertex_payload_byte_count =
        RuntimeAssetMeshVertexPayloadByteCount(cube_vertex_count);
    constexpr std::uint32_t cube_index_payload_byte_count =
        RuntimeAssetMeshIndexPayloadByteCount(cube_index_count);
    constexpr std::uint32_t cylinder_vertex_payload_byte_count =
        RuntimeAssetMeshVertexPayloadByteCount(cylinder_vertex_count);
    constexpr std::uint32_t cylinder_index_payload_byte_count =
        RuntimeAssetMeshIndexPayloadByteCount(cylinder_index_count);
    constexpr std::uint32_t cone_vertex_payload_byte_count =
        RuntimeAssetMeshVertexPayloadByteCount(cone_vertex_count);
    constexpr std::uint32_t cone_index_payload_byte_count =
        RuntimeAssetMeshIndexPayloadByteCount(cone_index_count);
    constexpr std::uint32_t cube_payload_byte_count =
        RuntimeAssetMeshPayloadByteCount(cube_vertex_count, cube_index_count);
    constexpr std::uint32_t cylinder_payload_byte_count =
        RuntimeAssetMeshPayloadByteCount(cylinder_vertex_count, cylinder_index_count);
    constexpr std::uint32_t cone_payload_byte_count =
        RuntimeAssetMeshPayloadByteCount(cone_vertex_count, cone_index_count);
    const std::string cube_payload =
        RuntimeAssetMeshPayload('A', cube_payload_byte_count);
    const std::string cylinder_payload =
        RuntimeAssetMeshPayload('K', cylinder_payload_byte_count);
    const std::string cone_payload =
        RuntimeAssetMeshPayload('U', cone_payload_byte_count);
    std::string cube_mesh_fields(
        "shape=cube\nvertices=24\nindices=36\nbounds=-1,-1,-1,1,1,1\nvertexPayloadBytes=");
    cube_mesh_fields += std::to_string(cube_vertex_payload_byte_count);
    cube_mesh_fields += "\nindexPayloadBytes=";
    cube_mesh_fields += std::to_string(cube_index_payload_byte_count);
    cube_mesh_fields += "\n";
    cube_mesh_fields += RuntimeAssetMeshLayoutFields();
    std::string cylinder_mesh_fields(
        "shape=cylinder\nvertices=18\nindices=96\nbounds=-1,-1,-1,1,1,1\nvertexPayloadBytes=");
    cylinder_mesh_fields += std::to_string(cylinder_vertex_payload_byte_count);
    cylinder_mesh_fields += "\nindexPayloadBytes=";
    cylinder_mesh_fields += std::to_string(cylinder_index_payload_byte_count);
    cylinder_mesh_fields += "\n";
    cylinder_mesh_fields += RuntimeAssetMeshLayoutFields();
    std::string cone_mesh_fields(
        "shape=cone\nvertices=10\nindices=48\nbounds=-1,-1,-1,1,1,1\nvertexPayloadBytes=");
    cone_mesh_fields += std::to_string(cone_vertex_payload_byte_count);
    cone_mesh_fields += "\nindexPayloadBytes=";
    cone_mesh_fields += std::to_string(cone_index_payload_byte_count);
    cone_mesh_fields += "\n";
    cone_mesh_fields += RuntimeAssetMeshLayoutFields();
    std::string material_fields(
        "shader=Shader/RuntimeProgram.racooked\n"
        "texture0=Texture/Albedo.racooked\n"
        "texture1=Texture/Normal.racooked\n"
        "texture2=Texture/Mask.racooked\n");
    material_fields += RuntimeAssetMaterialParameterFields();
    const std::string visual_proof_shader_fields = RuntimeAssetCookedVisualProofShaderFields();

    return std::array<RuntimeAssetFixtureArtifact, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT>{
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc(
                "Mesh/Cube.racooked",
                RuntimeAssetFileKind::Mesh,
                11001U,
                cube_payload_byte_count),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Mesh,
                "cube_mesh_cooked",
                cube_payload,
                cube_mesh_fields,
                std::span<const std::string_view>(no_deps.data(), no_deps.size()),
                cube_payload_byte_count,
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc(
                "Mesh/Cylinder.racooked",
                RuntimeAssetFileKind::Mesh,
                11002U,
                cylinder_payload_byte_count),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Mesh,
                "cylinder_mesh_cooked",
                cylinder_payload,
                cylinder_mesh_fields,
                std::span<const std::string_view>(no_deps.data(), no_deps.size()),
                cylinder_payload_byte_count,
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc(
                "Mesh/Cone.racooked",
                RuntimeAssetFileKind::Mesh,
                11003U,
                cone_payload_byte_count),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Mesh,
                "cone_mesh_cooked",
                cone_payload,
                cone_mesh_fields,
                std::span<const std::string_view>(no_deps.data(), no_deps.size()),
                cone_payload_byte_count,
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Material/Shared.racooked", RuntimeAssetFileKind::Material, 12001U, 128U),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Material,
                "shared_material_cooked",
                "material-shared-payload",
                material_fields,
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
                visual_proof_shader_fields,
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
                4U)},
        RuntimeAssetFixtureArtifact{
            RuntimeAssetFixtureDesc("Camera/Main.racooked", RuntimeAssetFileKind::Camera, 17001U, 0U),
            RuntimeAssetCookedText(
                RuntimeAssetFileKind::Camera,
                "main_camera_cooked",
                "camera-tween-payload",
                "projection=perspective\nfov_degrees=55\nnear=0.1\nfar=100\nkeyframes=3\nkey0=0:-4,2,-6:0,0,0\nkey1=0.5:0,3,-5:0,0,0\nkey2=1:4,2,-6:0,0,0\n",
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
        "camera:main_camera_cooked:17001",
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
            "cam=Camera/Main.racooked\n"
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

bool ParseRuntimeAssetTargetIdentityKind(
    std::string_view value,
    RuntimeAssetTargetIdentityKind *out_kind) {
    if (out_kind == nullptr) {
        return false;
    }

    if (value == "scene_node") {
        *out_kind = RuntimeAssetTargetIdentityKind::SceneNode;
        return true;
    }

    if (value == "model_node") {
        *out_kind = RuntimeAssetTargetIdentityKind::ModelNode;
        return true;
    }

    if (value == "skeleton_joint") {
        *out_kind = RuntimeAssetTargetIdentityKind::SkeletonJoint;
        return true;
    }

    return false;
}

bool SceneStageHasEntityId(
    const RuntimeAssetSceneLoaderStage &stage,
    std::uint32_t scene_entity_id) {
    for (std::uint32_t index = 0U; index < stage.entity_count; ++index) {
        if (stage.entities[index].entity_id == scene_entity_id) {
            return true;
        }
    }

    return false;
}

bool FindSceneStageEntityIndexById(
    const RuntimeAssetSceneLoaderStage &stage,
    std::uint32_t scene_entity_id,
    std::uint32_t *out_index) {
    if (out_index == nullptr) {
        return false;
    }

    for (std::uint32_t index = 0U; index < stage.entity_count; ++index) {
        if (stage.entities[index].entity_id == scene_entity_id) {
            *out_index = index;
            return true;
        }
    }

    return false;
}

const RuntimeAssetTargetIdentityRecord *FindTargetIdentityRecord(
    std::span<const RuntimeAssetTargetIdentityRecord> identities,
    std::uint64_t target_id) {
    for (const RuntimeAssetTargetIdentityRecord &identity : identities) {
        if (identity.target_id == target_id) {
            return &identity;
        }
    }

    return nullptr;
}

const RuntimeAssetRuntimeInstanceMappingRecord *FindRuntimeInstanceMappingRecord(
    std::span<const RuntimeAssetRuntimeInstanceMappingRecord> mappings,
    std::uint64_t target_id) {
    for (const RuntimeAssetRuntimeInstanceMappingRecord &mapping : mappings) {
        if (mapping.target_id == target_id) {
            return &mapping;
        }
    }

    return nullptr;
}

RuntimeAssetDataStatus BuildRuntimeInstanceMappings(RuntimeAssetSceneLoaderStage *stage) {
    if (stage == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    for (std::uint32_t index = 0U; index < stage->target_identity_count; ++index) {
        const RuntimeAssetTargetIdentityRecord &identity = stage->target_identities[index];
        RuntimeAssetRuntimeInstanceMappingRecord mapping{};
        mapping.target_kind = identity.kind;
        mapping.target_id = identity.target_id;
        mapping.scene_entity_id = identity.scene_entity_id;

        if (identity.kind == RuntimeAssetTargetIdentityKind::SceneNode) {
            std::uint32_t scene_entity_index = 0U;
            if (!FindSceneStageEntityIndexById(*stage, identity.scene_entity_id, &scene_entity_index)) {
                return RuntimeAssetDataStatus::MissingDependency;
            }

            mapping.scene_entity_index = scene_entity_index;
            mapping.scene_transform_index = scene_entity_index;
            mapping.is_valid = true;
        }

        stage->runtime_instance_mappings[index] = mapping;
    }

    stage->runtime_instance_mapping_count = stage->target_identity_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ResolveAnimationTrackTargetBinding(
    const RuntimeAssetSceneLoaderStage &stage,
    std::uint64_t target_id,
    RuntimeAssetAnimationTargetProperty property,
    const RuntimeAssetSceneEntityRecord **out_entity,
    AnimationRuntimeChannel *out_channel) {
    if (out_entity == nullptr || out_channel == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const RuntimeAssetRuntimeInstanceMappingRecord *mapping = FindRuntimeInstanceMappingRecord(
        std::span<const RuntimeAssetRuntimeInstanceMappingRecord>(
            stage.runtime_instance_mappings.data(),
            stage.runtime_instance_mapping_count),
        target_id);
    if (mapping == nullptr) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (mapping->target_kind != RuntimeAssetTargetIdentityKind::SceneNode || !mapping->is_valid) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (mapping->scene_entity_index >= stage.entity_count) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    const RuntimeAssetSceneEntityRecord *entity = &stage.entities[mapping->scene_entity_index];
    AnimationRuntimeChannel channel = AnimationRuntimeChannel::TranslationX;
    if (!RuntimeAssetAnimationTargetPropertyToChannel(property, &channel)) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    *out_entity = entity;
    *out_channel = channel;
    return RuntimeAssetDataStatus::Success;
}

bool TargetIdentityParentKindMatches(
    RuntimeAssetTargetIdentityKind kind,
    RuntimeAssetTargetIdentityKind parent_kind) {
    switch (kind) {
        case RuntimeAssetTargetIdentityKind::SceneNode:
            return parent_kind == RuntimeAssetTargetIdentityKind::SceneNode;
        case RuntimeAssetTargetIdentityKind::ModelNode:
            return parent_kind == RuntimeAssetTargetIdentityKind::SceneNode ||
                parent_kind == RuntimeAssetTargetIdentityKind::ModelNode;
        case RuntimeAssetTargetIdentityKind::SkeletonJoint:
            return parent_kind == RuntimeAssetTargetIdentityKind::ModelNode ||
                parent_kind == RuntimeAssetTargetIdentityKind::SkeletonJoint;
        case RuntimeAssetTargetIdentityKind::Unknown:
        default:
            break;
    }

    return false;
}

RuntimeAssetDataStatus ParseTargetIdentityRecord(
    std::string_view value,
    std::uint32_t target_index,
    RuntimeAssetTargetIdentityRecord *out_record) {
    if (out_record == nullptr || value.empty()) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    RuntimeAssetTargetIdentityKind kind = RuntimeAssetTargetIdentityKind::Unknown;
    std::uint64_t target_id = 0U;
    std::uint64_t parent_target_id = 0U;
    std::uint32_t scene_entity_id = 0U;
    std::uint32_t ordinal = target_index;
    if (!ParseRuntimeAssetTargetIdentityKind(FieldValue(value, "kind="), &kind) ||
        !ParseU64(FieldValue(value, "id="), &target_id) ||
        !ParseOptionalU32Field(value, "scene_entity=", 0U, &scene_entity_id) ||
        !ParseOptionalU32Field(value, "ordinal=", target_index, &ordinal)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    const std::string_view parent_value = FieldValue(value, "parent=");
    if (!parent_value.empty() && !ParseU64(parent_value, &parent_target_id)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (target_id == 0U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    RuntimeAssetTargetIdentityRecord record{};
    record.kind = kind;
    record.target_id = target_id;
    record.parent_target_id = parent_target_id;
    record.scene_entity_id = scene_entity_id;
    record.ordinal = ordinal;
    record.is_valid = true;
    *out_record = record;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateTargetIdentityRecords(
    const RuntimeAssetSceneLoaderStage &stage,
    std::span<const RuntimeAssetTargetIdentityRecord> identities) {
    for (std::size_t index = 0U; index < identities.size(); ++index) {
        const RuntimeAssetTargetIdentityRecord &identity = identities[index];
        if (!identity.is_valid || identity.kind == RuntimeAssetTargetIdentityKind::Unknown) {
            return RuntimeAssetDataStatus::InvalidBounds;
        }

        for (std::size_t other = index + 1U; other < identities.size(); ++other) {
            if (identity.target_id == identities[other].target_id) {
                return RuntimeAssetDataStatus::DuplicateDependency;
            }
        }

        if (identity.scene_entity_id != 0U && !SceneStageHasEntityId(stage, identity.scene_entity_id)) {
            return RuntimeAssetDataStatus::MissingDependency;
        }

        if (identity.kind == RuntimeAssetTargetIdentityKind::SceneNode && identity.scene_entity_id == 0U) {
            return RuntimeAssetDataStatus::MissingDependency;
        }

        if (identity.parent_target_id == 0U) {
            if (identity.kind == RuntimeAssetTargetIdentityKind::SceneNode) {
                continue;
            }

            return RuntimeAssetDataStatus::MissingDependency;
        }

        if (identity.parent_target_id == identity.target_id) {
            return RuntimeAssetDataStatus::InvalidDependency;
        }

        const RuntimeAssetTargetIdentityRecord *parent =
            FindTargetIdentityRecord(identities, identity.parent_target_id);
        if (parent == nullptr) {
            return RuntimeAssetDataStatus::MissingDependency;
        }

        if (!TargetIdentityParentKindMatches(identity.kind, parent->kind)) {
            return RuntimeAssetDataStatus::TypeMismatch;
        }
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ParseTargetIdentities(
    std::string_view scene_text,
    RuntimeAssetSceneLoaderStage *stage) {
    if (stage == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    bool target_count_declared = false;
    std::uint32_t target_count = 0U;
    RuntimeAssetDataStatus status = ParseDeclaredCount(
        scene_text,
        "targets=",
        0U,
        RUNTIME_ASSET_MAX_TARGET_IDENTITY_COUNT,
        &target_count,
        &target_count_declared);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    if (!target_count_declared) {
        stage->target_identity_count = 0U;
        return RuntimeAssetDataStatus::Success;
    }

    for (std::uint32_t index = 0U; index < target_count; ++index) {
        const std::string token = "target_identity" + std::to_string(index) + "=";
        status = ParseTargetIdentityRecord(
            ValueForToken(scene_text, token),
            index,
            &stage->target_identities[index]);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }
    }

    status = ValidateTargetIdentityRecords(
        *stage,
        std::span<const RuntimeAssetTargetIdentityRecord>(
            stage->target_identities.data(),
            target_count));
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    stage->target_identity_count = target_count;
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

RuntimeAssetFileKind DependencyKindForValue(std::string_view value);

void SetValidationDependencyFailure(
    RuntimeAssetValidationResult *out_result,
    RuntimeAssetDataStatus status,
    std::uint32_t dependency_index,
    std::uint32_t token_index,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetFileKind actual_kind);

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
            const RuntimeAssetDataStatus status = RuntimeAssetDataStatus::MissingDependency;
            SetValidationDependencyFailure(
                out_result,
                status,
                index,
                index,
                RuntimeAssetFileKind::Unknown,
                RuntimeAssetFileKind::Unknown);
            return status;
        }

        if (token_count > 1U) {
            const RuntimeAssetDataStatus status = RuntimeAssetDataStatus::DuplicateDependency;
            SetValidationDependencyFailure(
                out_result,
                status,
                index,
                index,
                RuntimeAssetFileKind::Unknown,
                DependencyKindForValue(ValueForToken(text, token)));
            return status;
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
            const RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidDependency;
            SetValidationDependencyFailure(
                out_result,
                status,
                index,
                index,
                RuntimeAssetFileKind::Unknown,
                DependencyKindForValue(row));
            return status;
        }

        std::uint64_t dependency_hash = 0U;
        if (!ParseU64(row.substr(second_separator + 1U), &dependency_hash) ||
            dependency_hash == 0U) {
            const RuntimeAssetDataStatus status = RuntimeAssetDataStatus::HashMismatch;
            SetValidationDependencyFailure(
                out_result,
                status,
                index,
                index,
                RuntimeAssetFileKind::Unknown,
                DependencyKindForValue(row));
            return status;
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

RuntimeAssetFileKind DependencyKindForPrefix(std::string_view prefix) {
    if (prefix == "Mesh/" || prefix == "mesh:") {
        return RuntimeAssetFileKind::Mesh;
    }

    if (prefix == "Material/" || prefix == "material:") {
        return RuntimeAssetFileKind::Material;
    }

    if (prefix == "Texture/" || prefix == "texture:") {
        return RuntimeAssetFileKind::Texture;
    }

    if (prefix == "Shader/" || prefix == "shader:") {
        return RuntimeAssetFileKind::Shader;
    }

    if (prefix == "Scene/" || prefix == "scene:") {
        return RuntimeAssetFileKind::Scene;
    }

    if (prefix == "Animation/" || prefix == "animation:") {
        return RuntimeAssetFileKind::Animation;
    }

    if (prefix == "Camera/" || prefix == "camera:") {
        return RuntimeAssetFileKind::Camera;
    }

    return RuntimeAssetFileKind::Unknown;
}

RuntimeAssetFileKind DependencyKindForValue(std::string_view value) {
    constexpr std::array<std::string_view, 14U> prefixes{{
        "Mesh/",
        "mesh:",
        "Material/",
        "material:",
        "Texture/",
        "texture:",
        "Shader/",
        "shader:",
        "Scene/",
        "scene:",
        "Animation/",
        "animation:",
        "Camera/",
        "camera:",
    }};

    for (const std::string_view prefix : prefixes) {
        if (StartsWith(value, prefix)) {
            return DependencyKindForPrefix(prefix);
        }
    }

    return RuntimeAssetFileKind::Unknown;
}

void SetValidationDependencyFailure(
    RuntimeAssetValidationResult *out_result,
    RuntimeAssetDataStatus status,
    std::uint32_t dependency_index,
    std::uint32_t token_index,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetFileKind actual_kind) {
    out_result->first_failed_dependency_status = status;
    out_result->first_failed_dependency_index = dependency_index;
    out_result->first_failed_dependency_token_index = token_index;
    out_result->first_failed_expected_kind = expected_kind;
    out_result->first_failed_actual_kind = actual_kind;
}

RuntimeAssetDataStatus ValidateDependencyRules(
    std::string_view text,
    std::span<const DependencyRule> rules,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    for (std::uint32_t rule_index = 0U; rule_index < rules.size(); ++rule_index) {
        const DependencyRule &rule = rules[rule_index];
        const RuntimeAssetFileKind expected_kind = DependencyKindForPrefix(rule.expected_prefix);
        const std::size_t count = CountToken(text, rule.token);
        if (count == 0U) {
            const RuntimeAssetDataStatus status = RuntimeAssetDataStatus::MissingDependency;
            SetValidationDependencyFailure(
                out_result,
                status,
                rule_index,
                rule_index,
                expected_kind,
                RuntimeAssetFileKind::Unknown);
            return status;
        }

        if (count > 1U) {
            const RuntimeAssetDataStatus status = RuntimeAssetDataStatus::DuplicateDependency;
            SetValidationDependencyFailure(
                out_result,
                status,
                rule_index,
                rule_index,
                expected_kind,
                DependencyKindForValue(ValueForToken(text, rule.token)));
            return status;
        }

        if (!rule.expected_prefix.empty()) {
            const std::string_view value = ValueForToken(text, rule.token);
            if (!StartsWith(value, rule.expected_prefix)) {
                const RuntimeAssetDataStatus status = RuntimeAssetDataStatus::TypeMismatch;
                SetValidationDependencyFailure(
                    out_result,
                    status,
                    rule_index,
                    rule_index,
                    expected_kind,
                    DependencyKindForValue(value));
                return status;
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
        {"cam=", "Camera/"},
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
        {"textures=", ""},
    }};

    return ValidateDependencyRules(text, std::span<const DependencyRule>(rules.data(), rules.size()), out_result);
}

std::uint64_t RuntimeAssetShaderImportPolicyHash() {
    const std::string fields = RuntimeAssetShaderImportPolicyFields();
    return HashRuntimeAssetText(fields);
}

RuntimeAssetDataStatus ValidateShaderImportPolicyMetadata(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (ValueForToken(text, "importLanguage=") != "hlsl") {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    if (ValueForToken(text, "importTarget=") != "d3d11") {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    if (ValueForToken(text, "entry_vs=") != "VSMain") {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (ValueForToken(text, "entry_ps=") != "PSMain") {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (ValueForToken(text, "profile_vs=") != "vs_5_0") {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (ValueForToken(text, "profile_ps=") != "ps_5_0") {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (ValueForToken(text, "compileFlags=") != "deterministic") {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    out_result->shader_import_policy_count = RUNTIME_ASSET_SHADER_IMPORT_POLICY_FIELD_COUNT;
    out_result->shader_import_policy_hash = RuntimeAssetShaderImportPolicyHash();
    return RuntimeAssetDataStatus::Success;
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

RuntimeAssetDataStatus ValidateCameraTweenKeyframe(std::string_view value) {
    const std::size_t first_separator = value.find(':');
    if (first_separator == std::string_view::npos) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    const std::size_t second_separator = value.find(':', first_separator + 1U);
    if (second_separator == std::string_view::npos) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    if (value.find(':', second_separator + 1U) != std::string_view::npos) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    float time = 0.0F;
    if (!ParseFloat(value.substr(0U, first_separator), &time)) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    if (time < 0.0F || time > 1.0F) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    float eye_x = 0.0F;
    float eye_y = 0.0F;
    float eye_z = 0.0F;
    if (!ParseVec3(value.substr(first_separator + 1U, second_separator - first_separator - 1U), &eye_x, &eye_y, &eye_z)) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    float target_x = 0.0F;
    float target_y = 0.0F;
    float target_z = 0.0F;
    if (!ParseVec3(value.substr(second_separator + 1U), &target_x, &target_y, &target_z)) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateCameraMetadata(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (ValueForToken(text, "projection=") != "perspective") {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    float fov_degrees = 0.0F;
    if (!ParseFloat(ValueForToken(text, "fov_degrees="), &fov_degrees)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (fov_degrees < 1.0F || fov_degrees > 170.0F) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    float near_plane = 0.0F;
    float far_plane = 0.0F;
    if (!ParseFloat(ValueForToken(text, "near="), &near_plane) ||
        !ParseFloat(ValueForToken(text, "far="), &far_plane)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (near_plane <= 0.0F || far_plane <= near_plane) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    std::uint32_t keyframe_count = 0U;
    if (!ParseU32(ValueForToken(text, "keyframes="), &keyframe_count)) {
        return RuntimeAssetDataStatus::InvalidCount;
    }

    if (keyframe_count < 2U || keyframe_count > 16U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    std::uint32_t index = 0U;
    while (index < keyframe_count) {
        const std::string token = "key" + std::to_string(index) + "=";
        const std::string_view keyframe = ValueForToken(text, token);
        const RuntimeAssetDataStatus status = ValidateCameraTweenKeyframe(keyframe);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }

        ++index;
    }

    out_result->dependency_count = keyframe_count;
    out_result->dependency_table_count = keyframe_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateSourcePayloadPolicy(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    std::uint32_t payload_byte_count = 0U;
    if (!ParseU32(ValueForToken(text, "payloadBytes="), &payload_byte_count)) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (payload_byte_count == 0U || payload_byte_count > MAX_RUNTIME_ASSET_PAYLOAD_BYTES) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    std::uint32_t payload_alignment = 0U;
    if (!ParseU32(ValueForToken(text, "payloadAlign="), &payload_alignment) ||
        !IsSupportedPayloadAlignment(payload_alignment)) {
        return RuntimeAssetDataStatus::InvalidAlignment;
    }

    std::uint64_t payload_hash = 0U;
    if (!ParseU64(ValueForToken(text, "payloadHash="), &payload_hash)) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    const std::string_view payload = ValueForToken(text, "payload=");
    if (payload.empty() || payload.size() != payload_byte_count) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (HashRuntimeAssetText(payload) != payload_hash) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    out_result->payload_byte_count = payload_byte_count;
    out_result->payload_alignment = payload_alignment;
    out_result->payload_hash = payload_hash;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus DecodeRuntimeInputLayout(
    std::string_view input,
    yuengine::rhi::RhiInputLayoutDesc *out_layout);

RuntimeAssetDataStatus DecodeMeshIndexFormat(
    std::string_view text,
    yuengine::rhi::RhiIndexFormat *out_format,
    std::uint32_t *out_stride_bytes) {
    if (out_format == nullptr || out_stride_bytes == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (text == "uint16") {
        *out_format = yuengine::rhi::RhiIndexFormat::Uint16;
        *out_stride_bytes = 2U;
        return RuntimeAssetDataStatus::Success;
    }

    if (text == "uint32") {
        *out_format = yuengine::rhi::RhiIndexFormat::Uint32;
        *out_stride_bytes = 4U;
        return RuntimeAssetDataStatus::Success;
    }

    return RuntimeAssetDataStatus::UnsupportedFieldValue;
}

RuntimeAssetDataStatus DecodeMeshTopology(
    std::string_view text,
    yuengine::rhi::RhiPrimitiveTopology *out_topology) {
    if (out_topology == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (text == "triangle_list") {
        *out_topology = yuengine::rhi::RhiPrimitiveTopology::TriangleList;
        return RuntimeAssetDataStatus::Success;
    }

    return RuntimeAssetDataStatus::UnsupportedFieldValue;
}

RuntimeAssetDataStatus ValidateMeshPayloadPolicy(
    std::string_view text,
    std::uint32_t vertex_count,
    std::uint32_t index_count,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    yuengine::rhi::RhiInputLayoutDesc input_layout{};
    RuntimeAssetDataStatus layout_status =
        DecodeRuntimeInputLayout(ValueForToken(text, "input="), &input_layout);
    if (layout_status != RuntimeAssetDataStatus::Success) {
        return layout_status;
    }

    if (input_layout.element_count == 0U || input_layout.stride_bytes == 0U) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    std::uint32_t vertex_stride_bytes = 0U;
    if (!ParseU32(ValueForToken(text, "vertexStrideBytes="), &vertex_stride_bytes)) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    if (static_cast<std::size_t>(vertex_stride_bytes) != input_layout.stride_bytes) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    yuengine::rhi::RhiIndexFormat index_format = yuengine::rhi::RhiIndexFormat::Unsupported;
    std::uint32_t index_stride_from_format = 0U;
    layout_status =
        DecodeMeshIndexFormat(ValueForToken(text, "indexFormat="), &index_format, &index_stride_from_format);
    if (layout_status != RuntimeAssetDataStatus::Success) {
        return layout_status;
    }

    std::uint32_t index_stride_bytes = 0U;
    if (!ParseU32(ValueForToken(text, "indexStrideBytes="), &index_stride_bytes)) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    if (index_stride_bytes != index_stride_from_format) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    yuengine::rhi::RhiPrimitiveTopology topology = yuengine::rhi::RhiPrimitiveTopology::Unsupported;
    layout_status = DecodeMeshTopology(ValueForToken(text, "topology="), &topology);
    if (layout_status != RuntimeAssetDataStatus::Success) {
        return layout_status;
    }

    std::uint32_t vertex_payload_byte_count = 0U;
    std::uint32_t index_payload_byte_count = 0U;
    if (!ParseU32(ValueForToken(text, "vertexPayloadBytes="), &vertex_payload_byte_count) ||
        !ParseU32(ValueForToken(text, "indexPayloadBytes="), &index_payload_byte_count)) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (vertex_payload_byte_count == 0U ||
        index_payload_byte_count == 0U ||
        vertex_payload_byte_count > MAX_RUNTIME_ASSET_PAYLOAD_BYTES ||
        index_payload_byte_count > MAX_RUNTIME_ASSET_PAYLOAD_BYTES) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    const std::uint64_t expected_vertex_payload_byte_count =
        static_cast<std::uint64_t>(vertex_count) *
        static_cast<std::uint64_t>(vertex_stride_bytes);
    const std::uint64_t expected_index_payload_byte_count =
        static_cast<std::uint64_t>(index_count) *
        static_cast<std::uint64_t>(index_stride_bytes);
    if (expected_vertex_payload_byte_count != vertex_payload_byte_count ||
        expected_index_payload_byte_count != index_payload_byte_count) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    const std::uint64_t expected_payload_byte_count =
        static_cast<std::uint64_t>(vertex_payload_byte_count) +
        static_cast<std::uint64_t>(index_payload_byte_count);
    if (expected_payload_byte_count > MAX_RUNTIME_ASSET_PAYLOAD_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    if (out_result->artifact_class == RuntimeAssetArtifactClass::Source) {
        const RuntimeAssetDataStatus status = ValidateSourcePayloadPolicy(text, out_result);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }
    }

    if (out_result->payload_byte_count != expected_payload_byte_count) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (out_result->payload_alignment == 0U ||
        !IsSupportedPayloadAlignment(out_result->payload_alignment)) {
        return RuntimeAssetDataStatus::InvalidAlignment;
    }

    const std::string_view payload = ValueForToken(text, "payload=");
    if (payload.empty() || payload.size() != out_result->payload_byte_count) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (HashRuntimeAssetText(payload) != out_result->payload_hash) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    out_result->mesh_input_layout = input_layout;
    out_result->mesh_index_format = index_format;
    out_result->mesh_topology = topology;
    out_result->mesh_vertex_stride_bytes = vertex_stride_bytes;
    out_result->mesh_index_stride_bytes = index_stride_bytes;
    return RuntimeAssetDataStatus::Success;
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

    const RuntimeAssetDataStatus payload_status =
        ValidateMeshPayloadPolicy(text, vertex_count, index_count, out_result);
    if (payload_status != RuntimeAssetDataStatus::Success) {
        return payload_status;
    }

    out_result->vertex_count = vertex_count;
    out_result->index_count = index_count;
    out_result->mesh_geometry_kind = geometry_kind;
    return RuntimeAssetDataStatus::Success;
}

bool ParseMaterialU8(std::string_view text, std::uint32_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    std::uint32_t value = 0U;
    if (!ParseU32(text, &value)) {
        return false;
    }

    if (value > 255U) {
        return false;
    }

    *out_value = value;
    return true;
}

bool ParseMaterialRgba(std::string_view text, std::uint32_t *out_rgba) {
    if (out_rgba == nullptr) {
        return false;
    }

    const std::size_t first_comma = text.find(',');
    if (first_comma == std::string_view::npos) {
        return false;
    }

    const std::size_t second_comma = text.find(',', first_comma + 1U);
    if (second_comma == std::string_view::npos) {
        return false;
    }

    const std::size_t third_comma = text.find(',', second_comma + 1U);
    if (third_comma == std::string_view::npos) {
        return false;
    }

    if (text.find(',', third_comma + 1U) != std::string_view::npos) {
        return false;
    }

    std::uint32_t red = 0U;
    std::uint32_t green = 0U;
    std::uint32_t blue = 0U;
    std::uint32_t alpha = 0U;
    const std::string_view red_text = text.substr(0U, first_comma);
    const std::string_view green_text = text.substr(first_comma + 1U, second_comma - first_comma - 1U);
    const std::string_view blue_text = text.substr(second_comma + 1U, third_comma - second_comma - 1U);
    const std::string_view alpha_text = text.substr(third_comma + 1U);
    if (!ParseMaterialU8(red_text, &red) ||
        !ParseMaterialU8(green_text, &green) ||
        !ParseMaterialU8(blue_text, &blue) ||
        !ParseMaterialU8(alpha_text, &alpha)) {
        return false;
    }

    const std::uint32_t packed_red = red << 24U;
    const std::uint32_t packed_green = green << 16U;
    const std::uint32_t packed_blue = blue << 8U;
    *out_rgba = packed_red | packed_green | packed_blue | alpha;
    return true;
}

RuntimeAssetDataStatus ParseMaterialAlphaMode(
    std::string_view text,
    RuntimeAssetMaterialAlphaMode *out_mode) {
    if (out_mode == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (text == "opaque") {
        *out_mode = RuntimeAssetMaterialAlphaMode::Opaque;
        return RuntimeAssetDataStatus::Success;
    }

    if (text == "blend") {
        *out_mode = RuntimeAssetMaterialAlphaMode::Blend;
        return RuntimeAssetDataStatus::Success;
    }

    return RuntimeAssetDataStatus::UnsupportedFieldValue;
}

RuntimeAssetDataStatus ValidateMaterialParameterMetadata(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    std::uint32_t parameter_count = 0U;
    if (!ParseU32(ValueForToken(text, "parameterCount="), &parameter_count)) {
        return RuntimeAssetDataStatus::InvalidCount;
    }

    if (parameter_count != RUNTIME_ASSET_MATERIAL_PARAMETER_COUNT) {
        return RuntimeAssetDataStatus::InvalidCount;
    }

    std::uint32_t base_color_rgba = 0U;
    if (!ParseMaterialRgba(ValueForToken(text, "baseColorRgba="), &base_color_rgba)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    std::uint32_t emissive_strength = 0U;
    std::uint32_t metallic = 0U;
    std::uint32_t roughness = 0U;
    std::uint32_t opacity = 0U;
    if (!ParseMaterialU8(ValueForToken(text, "emissiveStrength="), &emissive_strength) ||
        !ParseMaterialU8(ValueForToken(text, "metallic="), &metallic) ||
        !ParseMaterialU8(ValueForToken(text, "roughness="), &roughness) ||
        !ParseMaterialU8(ValueForToken(text, "opacity="), &opacity)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    RuntimeAssetMaterialAlphaMode alpha_mode = RuntimeAssetMaterialAlphaMode::Unknown;
    const RuntimeAssetDataStatus alpha_status =
        ParseMaterialAlphaMode(ValueForToken(text, "alphaMode="), &alpha_mode);
    if (alpha_status != RuntimeAssetDataStatus::Success) {
        return alpha_status;
    }

    if (opacity < 255U && alpha_mode != RuntimeAssetMaterialAlphaMode::Blend) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    out_result->material_parameter_count = parameter_count;
    out_result->material_base_color_rgba = base_color_rgba;
    out_result->material_emissive_strength = emissive_strength;
    out_result->material_metallic = metallic;
    out_result->material_roughness = roughness;
    out_result->material_opacity = opacity;
    out_result->material_alpha_mode = alpha_mode;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateMaterialMetadata(
    std::string_view text,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

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

    status = ValidateMaterialParameterMetadata(text, out_result);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
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

bool HexNibbleValue(char character, std::uint8_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    if (character >= '0' && character <= '9') {
        *out_value = static_cast<std::uint8_t>(character - '0');
        return true;
    }

    if (character >= 'A' && character <= 'F') {
        *out_value = static_cast<std::uint8_t>((character - 'A') + 10);
        return true;
    }

    if (character >= 'a' && character <= 'f') {
        *out_value = static_cast<std::uint8_t>((character - 'a') + 10);
        return true;
    }

    return false;
}

bool Base64Value(char character, std::uint8_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    if (character >= 'A' && character <= 'Z') {
        *out_value = static_cast<std::uint8_t>(character - 'A');
        return true;
    }

    if (character >= 'a' && character <= 'z') {
        *out_value = static_cast<std::uint8_t>((character - 'a') + 26);
        return true;
    }

    if (character >= '0' && character <= '9') {
        *out_value = static_cast<std::uint8_t>((character - '0') + 52);
        return true;
    }

    if (character == '+') {
        *out_value = 62U;
        return true;
    }

    if (character == '/') {
        *out_value = 63U;
        return true;
    }

    return false;
}

RuntimeAssetDataStatus ValidateBase64ShaderBytecodePayload(
    std::string_view base64_payload,
    std::size_t *out_byte_count) {
    if (out_byte_count == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    *out_byte_count = 0U;
    if (base64_payload.empty() || (base64_payload.size() % 4U) != 0U) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    std::size_t padding_count = 0U;
    if (base64_payload[base64_payload.size() - 1U] == '=') {
        padding_count = 1U;
    }

    if (base64_payload[base64_payload.size() - 2U] == '=') {
        padding_count = 2U;
    }

    const std::size_t data_character_count = base64_payload.size() - padding_count;
    for (std::size_t index = 0U; index < data_character_count; ++index) {
        std::uint8_t value = 0U;
        if (!Base64Value(base64_payload[index], &value)) {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }
    }

    for (std::size_t index = data_character_count; index < base64_payload.size(); ++index) {
        if (base64_payload[index] != '=') {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }
    }

    const std::size_t byte_count = ((base64_payload.size() / 4U) * 3U) - padding_count;
    if (byte_count == 0U) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (byte_count > yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    *out_byte_count = byte_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateHexShaderBytecodePayload(
    std::string_view hex_payload,
    std::size_t *out_byte_count) {
    if (out_byte_count == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    *out_byte_count = 0U;
    if (hex_payload.empty()) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if ((hex_payload.size() % 2U) != 0U) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    const std::size_t byte_count = hex_payload.size() / 2U;
    if (byte_count > yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    for (const char character : hex_payload) {
        std::uint8_t value = 0U;
        if (!HexNibbleValue(character, &value)) {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }
    }

    *out_byte_count = byte_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ShaderBytecodePayloadByteCount(
    std::string_view payload,
    std::size_t *out_byte_count) {
    if (out_byte_count == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (StartsWith(payload, SHADER_BYTECODE_BASE64_PREFIX)) {
        return ValidateBase64ShaderBytecodePayload(
            payload.substr(SHADER_BYTECODE_BASE64_PREFIX.size()),
            out_byte_count);
    }

    if (StartsWith(payload, SHADER_BYTECODE_HEX_PREFIX)) {
        return ValidateHexShaderBytecodePayload(
            payload.substr(SHADER_BYTECODE_HEX_PREFIX.size()),
            out_byte_count);
    }

    if (payload.empty()) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    if (payload.size() > yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    *out_byte_count = payload.size();
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus CopyBase64ShaderBytecodePayload(
    std::string_view base64_payload,
    std::array<std::uint8_t, yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES> *destination,
    std::size_t *out_size,
    std::uint64_t *out_hash) {
    if (destination == nullptr || out_size == nullptr || out_hash == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    std::size_t byte_count = 0U;
    RuntimeAssetDataStatus status = ValidateBase64ShaderBytecodePayload(base64_payload, &byte_count);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    std::size_t write_index = 0U;
    for (std::size_t index = 0U; index < base64_payload.size(); index += 4U) {
        std::uint8_t first = 0U;
        std::uint8_t second = 0U;
        std::uint8_t third = 0U;
        std::uint8_t fourth = 0U;
        if (!Base64Value(base64_payload[index], &first) ||
            !Base64Value(base64_payload[index + 1U], &second)) {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }

        (*destination)[write_index] = static_cast<std::uint8_t>((first << 2U) | (second >> 4U));
        ++write_index;
        if (base64_payload[index + 2U] == '=') {
            continue;
        }

        if (!Base64Value(base64_payload[index + 2U], &third)) {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }

        (*destination)[write_index] = static_cast<std::uint8_t>((second << 4U) | (third >> 2U));
        ++write_index;
        if (base64_payload[index + 3U] == '=') {
            continue;
        }

        if (!Base64Value(base64_payload[index + 3U], &fourth)) {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }

        (*destination)[write_index] = static_cast<std::uint8_t>((third << 6U) | fourth);
        ++write_index;
    }

    if (write_index != byte_count) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    *out_size = byte_count;
    *out_hash = HashRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(destination->data(), byte_count));
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus CopyHexShaderBytecodePayload(
    std::string_view hex_payload,
    std::array<std::uint8_t, yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES> *destination,
    std::size_t *out_size,
    std::uint64_t *out_hash) {
    if (destination == nullptr || out_size == nullptr || out_hash == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    std::size_t byte_count = 0U;
    RuntimeAssetDataStatus status = ValidateHexShaderBytecodePayload(hex_payload, &byte_count);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    for (std::size_t byte_index = 0U; byte_index < byte_count; ++byte_index) {
        std::uint8_t high = 0U;
        std::uint8_t low = 0U;
        if (!HexNibbleValue(hex_payload[(byte_index * 2U)], &high)) {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }

        if (!HexNibbleValue(hex_payload[(byte_index * 2U) + 1U], &low)) {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }

        (*destination)[byte_index] = static_cast<std::uint8_t>((high << 4U) | low);
    }

    *out_size = byte_count;
    *out_hash = HashRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(destination->data(), byte_count));
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus DecodeRuntimeInputLayout(
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
    if (semantic_list == SHADER_LAYOUT_NONE) {
        *out_layout = layout;
        return RuntimeAssetDataStatus::Success;
    }

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

    std::size_t byte_count = 0U;
    const RuntimeAssetDataStatus status = ShaderBytecodePayloadByteCount(payload, &byte_count);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
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
    if (StartsWith(payload, SHADER_BYTECODE_BASE64_PREFIX)) {
        return CopyBase64ShaderBytecodePayload(
            payload.substr(SHADER_BYTECODE_BASE64_PREFIX.size()),
            destination,
            out_size,
            out_hash);
    }

    if (StartsWith(payload, SHADER_BYTECODE_HEX_PREFIX)) {
        return CopyHexShaderBytecodePayload(
            payload.substr(SHADER_BYTECODE_HEX_PREFIX.size()),
            destination,
            out_size,
            out_hash);
    }

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

    status = ValidateShaderImportPolicyMetadata(text, out_result);
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

    std::size_t vertex_byte_count = 0U;
    status = ShaderBytecodePayloadByteCount(vertex_payload, &vertex_byte_count);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    std::size_t pixel_byte_count = 0U;
    status = ShaderBytecodePayloadByteCount(pixel_payload, &pixel_byte_count);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    yuengine::rhi::RhiInputLayoutDesc input_layout{};
    status = DecodeRuntimeInputLayout(ValueForToken(text, "input="), &input_layout);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    if (input_layout.element_count == 0U) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    out_result->shader_bytecode_byte_count =
        static_cast<std::uint32_t>(vertex_byte_count + pixel_byte_count);
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

    if (!TokenValueStartsWith(scene_text, "cam=", "Camera/")) {
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

std::vector<std::uint8_t> BuildDecodedBytesForAsset(
    RuntimeAssetFileKind kind,
    std::span<const std::uint8_t> source_bytes,
    std::uint32_t decoded_byte_count) {
    if (kind != RuntimeAssetFileKind::Mesh) {
        return BuildDecodedBytes(source_bytes, decoded_byte_count);
    }

    const char *source_text_data = reinterpret_cast<const char *>(source_bytes.data());
    const std::string_view source_text(source_text_data, source_bytes.size());
    const std::string_view payload = ValueForToken(source_text, "payload=");
    if (payload.size() != decoded_byte_count) {
        return BuildDecodedBytes(source_bytes, decoded_byte_count);
    }

    return std::vector<std::uint8_t>(payload.begin(), payload.end());
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
    out_record->mesh_input_layout = validation.mesh_input_layout;
    out_record->mesh_index_format = validation.mesh_index_format;
    out_record->mesh_topology = validation.mesh_topology;
    out_record->mesh_vertex_stride_bytes = validation.mesh_vertex_stride_bytes;
    out_record->mesh_index_stride_bytes = validation.mesh_index_stride_bytes;
    out_record->texture_width = validation.texture_width;
    out_record->texture_height = validation.texture_height;
    out_record->texture_slot_count = validation.texture_slot_count;
    out_record->material_parameter_count = validation.material_parameter_count;
    out_record->material_base_color_rgba = validation.material_base_color_rgba;
    out_record->material_emissive_strength = validation.material_emissive_strength;
    out_record->material_metallic = validation.material_metallic;
    out_record->material_roughness = validation.material_roughness;
    out_record->material_opacity = validation.material_opacity;
    out_record->material_alpha_mode = validation.material_alpha_mode;
    out_record->shader_stage_count = validation.shader_stage_count;
    out_record->shader_bytecode_byte_count = validation.shader_bytecode_byte_count;
    out_record->shader_import_policy_count = validation.shader_import_policy_count;
    out_record->shader_import_policy_hash = validation.shader_import_policy_hash;
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

    const std::vector<std::uint8_t> decoded_bytes =
        BuildDecodedBytesForAsset(desc.kind, source_bytes, desc.decoded_byte_count);
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
        return layout.stride_bytes == 0U;
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
    AnimationRuntimeTrackRecord *out_track,
    RuntimeAssetAnimationTrackTargetBindingRecord *out_binding,
    bool *out_has_binding) {
    if (out_track == nullptr || out_has_binding == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (value.empty()) {
        return RuntimeAssetDataStatus::InvalidCount;
    }

    *out_has_binding = false;
    std::uint32_t track_id = 0U;
    std::uint32_t first_key = 0U;
    std::uint32_t track_key_count = 0U;
    WorldObjectId target{};
    AnimationRuntimeChannel channel = AnimationRuntimeChannel::TranslationX;
    if (!ParseU32(FieldValue(value, "id="), &track_id)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (track_id == 0U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (!ParseU32(FieldValue(value, "first_key="), &first_key)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (!ParseU32(FieldValue(value, "key_count="), &track_key_count)) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (track_key_count == 0U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (first_key >= keyframe_count) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    if (track_key_count > keyframe_count - first_key) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    const std::string_view target_id_value = FieldValue(value, "target_id=");
    const std::string_view property_value = FieldValue(value, "property=");
    const bool uses_target_binding = !target_id_value.empty() || !property_value.empty();
    if (uses_target_binding) {
        if (out_binding == nullptr) {
            return RuntimeAssetDataStatus::InvalidArgument;
        }

        if (target_id_value.empty() || property_value.empty()) {
            return RuntimeAssetDataStatus::InvalidBounds;
        }

        std::uint64_t target_id = 0U;
        if (!ParseU64(target_id_value, &target_id)) {
            return RuntimeAssetDataStatus::InvalidBounds;
        }

        if (target_id == 0U) {
            return RuntimeAssetDataStatus::InvalidBounds;
        }

        RuntimeAssetAnimationTargetProperty property = RuntimeAssetAnimationTargetProperty::Unknown;
        if (!ParseRuntimeAssetAnimationTargetProperty(property_value, &property)) {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }

        const RuntimeAssetSceneEntityRecord *target_entity = nullptr;
        const RuntimeAssetDataStatus binding_status =
            ResolveAnimationTrackTargetBinding(stage, target_id, property, &target_entity, &channel);
        if (binding_status != RuntimeAssetDataStatus::Success) {
            return binding_status;
        }

        target = target_entity->world_object_id;
        RuntimeAssetAnimationTrackTargetBindingRecord binding{};
        binding.track_id = track_id;
        binding.target_id = target_id;
        binding.property = property;
        binding.is_valid = true;
        *out_binding = binding;
        *out_has_binding = true;
    }

    if (!uses_target_binding) {
        if (!ParseAnimationTarget(FieldValue(value, "target_ref="), &target)) {
            return RuntimeAssetDataStatus::TypeMismatch;
        }

        if (!SceneStageHasRuntimeAnimationTarget(stage, target.value)) {
            return RuntimeAssetDataStatus::InvalidDependency;
        }

        if (!ParseAnimationChannel(FieldValue(value, "channel="), &channel)) {
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }
    }

    AnimationRuntimeInterpolation interpolation = AnimationRuntimeInterpolation::Linear;
    if (!ParseAnimationInterpolation(FieldValue(value, "interp="), &interpolation)) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    AnimationRuntimeTrackRecord track{};
    track.track_id = track_id;
    track.target = target;
    track.channel = channel;
    track.interpolation = interpolation;
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
    std::array<RuntimeAssetAnimationTrackTargetBindingRecord, RUNTIME_ASSET_MAX_ANIMATION_TRACK_COUNT> *out_bindings,
    std::uint32_t *out_binding_count,
    std::array<AnimationRuntimeKeyframeRecord, RUNTIME_ASSET_MAX_ANIMATION_KEYFRAME_COUNT> *out_keyframes,
    std::uint32_t *out_keyframe_count) {
    if (out_clips == nullptr || out_clip_count == nullptr ||
        out_tracks == nullptr || out_track_count == nullptr ||
        out_bindings == nullptr || out_binding_count == nullptr ||
        out_keyframes == nullptr || out_keyframe_count == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    *out_binding_count = 0U;
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

    std::uint32_t binding_count = 0U;
    for (std::uint32_t index = 0U; index < track_count; ++index) {
        const std::string token = "track" + std::to_string(index) + "=";
        RuntimeAssetAnimationTrackTargetBindingRecord binding{};
        bool has_binding = false;
        status = ParseAnimationTrackRecord(
            ValueForToken(animation_text, token),
            keyframe_count,
            stage,
            &(*out_tracks)[index],
            &binding,
            &has_binding);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }

        if (has_binding) {
            (*out_bindings)[binding_count] = binding;
            ++binding_count;
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
    *out_binding_count = binding_count;
    *out_keyframe_count = keyframe_count;
    return RuntimeAssetDataStatus::Success;
}

std::uint32_t ResolveAnimationSampleClipId(
    const RuntimeAssetGraphLoadRequest &request,
    std::span<const AnimationRuntimeClipRecord> clips) {
    if (request.selected_animation_clip_id != 0U) {
        return request.selected_animation_clip_id;
    }

    if (clips.empty()) {
        return 0U;
    }

    return clips[0U].clip_id;
}

RuntimeAssetDataStatus MapAnimationRuntimeStatusToRuntimeAssetDataStatus(AnimationRuntimeStatus status) {
    switch (status) {
        case AnimationRuntimeStatus::Success:
            return RuntimeAssetDataStatus::Success;
        case AnimationRuntimeStatus::NullPointer:
            return RuntimeAssetDataStatus::InvalidArgument;
        case AnimationRuntimeStatus::UnsupportedInterpolation:
        case AnimationRuntimeStatus::UnsupportedChannel:
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        case AnimationRuntimeStatus::OutputCapacityExceeded:
        case AnimationRuntimeStatus::LayerCapacityExceeded:
            return RuntimeAssetDataStatus::CapacityExceeded;
        case AnimationRuntimeStatus::MissingClip:
        case AnimationRuntimeStatus::InvalidClip:
        case AnimationRuntimeStatus::MissingTrack:
        case AnimationRuntimeStatus::InvalidTrack:
        case AnimationRuntimeStatus::MissingKeyframe:
        case AnimationRuntimeStatus::InvalidKeyframe:
        case AnimationRuntimeStatus::InvalidTime:
        case AnimationRuntimeStatus::TimeOutOfRange:
        case AnimationRuntimeStatus::MissingSample:
        case AnimationRuntimeStatus::InvalidTarget:
        case AnimationRuntimeStatus::TargetNotFound:
        case AnimationRuntimeStatus::TransformApplyFailed:
        default:
            break;
    }

    return RuntimeAssetDataStatus::InvalidDependency;
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

    const std::uint32_t clip_id = ResolveAnimationSampleClipId(request, clips);
    stage->selected_animation_clip_id = clip_id;

    std::array<AnimationRuntimeSampledValue, RUNTIME_ASSET_MAX_ANIMATION_SAMPLED_VALUE_COUNT>
        sampled_values{};
    AnimationRuntimeSampleRequest sample_request{};
    sample_request.clip_id = clip_id;
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
        return MapAnimationRuntimeStatusToRuntimeAssetDataStatus(stage->animation_sample_status);
    }

    AnimationRuntimeTransformApplyResult apply_result{};
    stage->animation_apply_status = sampler.ApplySampledTransform(
        AnimationRuntimeTransformApplyRequest{bridge, std::span<const AnimationRuntimeSampledValue>(
            sampled_values.data(),
            sample_result.sampled_value_count)},
        &apply_result);
    if (stage->animation_apply_status != AnimationRuntimeStatus::Success) {
        return MapAnimationRuntimeStatusToRuntimeAssetDataStatus(stage->animation_apply_status);
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

    status = ParseTargetIdentities(scene_text, &stage);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = BuildRuntimeInstanceMappings(&stage);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ValidateTargetIdentityOutputRequest(request, stage.target_identity_count);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ValidateRuntimeInstanceMappingOutputRequest(
        request,
        stage.runtime_instance_mapping_count);
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

    status = ParseBoundedAnimationTables(
        animation_text,
        stage,
        &stage.animation_clips,
        &stage.animation_clip_count,
        &stage.animation_tracks,
        &stage.animation_track_count,
        &stage.animation_target_bindings,
        &stage.animation_target_binding_count,
        &stage.animation_keyframes,
        &stage.animation_keyframe_count);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ValidateAnimationRuntimeOutputRequest(
        request,
        stage.animation_clip_count,
        stage.animation_track_count,
        stage.animation_keyframe_count);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ValidateAnimationTargetBindingOutputRequest(
        request,
        stage.animation_target_binding_count);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = SampleAndApplyAnimationStage(
        request,
        &stage,
        &bridge,
        std::span<const AnimationRuntimeClipRecord>(
            stage.animation_clips.data(),
            stage.animation_clip_count),
        std::span<const AnimationRuntimeTrackRecord>(
            stage.animation_tracks.data(),
            stage.animation_track_count),
        std::span<const AnimationRuntimeKeyframeRecord>(
            stage.animation_keyframes.data(),
            stage.animation_keyframe_count));
    if (status != RuntimeAssetDataStatus::Success) {
        *out_stage = stage;
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
    output.target_identity_count = stage.target_identity_count;
    output.runtime_instance_mapping_count = stage.runtime_instance_mapping_count;
    output.animation_clip_count = stage.animation_clip_count;
    output.animation_track_count = stage.animation_track_count;
    output.animation_target_binding_count = stage.animation_target_binding_count;
    output.animation_keyframe_count = stage.animation_keyframe_count;
    output.animation_sampled_value_count = stage.animation_sampled_value_count;
    output.selected_animation_clip_id = stage.selected_animation_clip_id;
    output.entity_capacity = request.scene_entity_capacity;
    output.transform_capacity = request.scene_transform_capacity;
    output.resource_ref_capacity = request.scene_resource_ref_capacity;
    output.camera_capacity = request.scene_camera_capacity;
    output.target_identity_capacity = request.target_identity_capacity;
    output.runtime_instance_mapping_capacity = request.runtime_instance_mapping_capacity;
    output.animation_clip_capacity = request.animation_clip_capacity;
    output.animation_track_capacity = request.animation_track_capacity;
    output.animation_target_binding_capacity = request.animation_target_binding_capacity;
    output.animation_keyframe_capacity = request.animation_keyframe_capacity;
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

    CommitTargetIdentityRecords(request, stage);
    CommitRuntimeInstanceMappingRecords(request, stage);
    CommitAnimationRuntimeTables(request, stage);
    CommitAnimationTargetBindingRecords(request, stage);

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

void RecordSceneLoaderAnimationDiagnostics(
    const RuntimeAssetSceneLoaderStage &stage,
    RuntimeAssetGraphLoadResult *result) {
    if (result == nullptr) {
        return;
    }

    result->selected_animation_clip_id = stage.selected_animation_clip_id;
    result->animation_sampled_value_count = stage.animation_sampled_value_count;
    result->animation_sample_status = stage.animation_sample_status;
    result->animation_apply_status = stage.animation_apply_status;
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
        SetTransactionFailure(
            transaction,
            status,
            RuntimeAssetLoadTransactionPhase::ValidateRecord,
            0U,
            scene_validation.first_failed_dependency_index);
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
                file_index + 1U,
                validation.first_failed_dependency_index);
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
        RecordSceneLoaderAnimationDiagnostics(transaction->scene_stage, &transaction->result);
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

bool IsShaderModuleHandleSet(yuengine::rhi::RhiShaderModuleHandle handle) {
    return handle.generation != 0U;
}

bool IsBufferHandleSet(RhiBufferHandle handle) {
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

RuntimeAssetDataStatus BuildRuntimeAssetMaterialBlendState(
    const RuntimeAssetLoadedFile &material,
    RhiBlendStateDesc *out_blend_state) {
    if (out_blend_state == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (material.kind != RuntimeAssetFileKind::Material) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (material.material_opacity > 255U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    RhiBlendStateDesc blend_state{};
    if (material.material_alpha_mode == RuntimeAssetMaterialAlphaMode::Opaque) {
        blend_state.mode = RhiBlendMode::Opaque;
        blend_state.constant_alpha = static_cast<std::uint8_t>(255U);
        *out_blend_state = blend_state;
        return RuntimeAssetDataStatus::Success;
    }

    if (material.material_alpha_mode == RuntimeAssetMaterialAlphaMode::Blend) {
        blend_state.mode = RhiBlendMode::AlphaOver;
        blend_state.constant_alpha = static_cast<std::uint8_t>(material.material_opacity);
        *out_blend_state = blend_state;
        return RuntimeAssetDataStatus::Success;
    }

    return RuntimeAssetDataStatus::UnsupportedFieldValue;
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

    if (request.loaded_material != nullptr) {
        if (request.loaded_material->asset.slot != request.material_asset.slot ||
            request.loaded_material->asset.generation != request.material_asset.generation) {
            return RuntimeAssetDataStatus::TypeMismatch;
        }

        RuntimeAssetPackedMaterialConstants material_constants{};
        const RuntimeAssetDataStatus material_status =
            PackRuntimeAssetMaterialConstants(*request.loaded_material, &material_constants);
        if (material_status != RuntimeAssetDataStatus::Success) {
            return material_status;
        }

        RhiBlendStateDesc blend_state{};
        const RuntimeAssetDataStatus blend_status =
            BuildRuntimeAssetMaterialBlendState(*request.loaded_material, &blend_state);
        if (blend_status != RuntimeAssetDataStatus::Success) {
            return blend_status;
        }
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
    RuntimeAssetPackedMaterialConstants material_constants{};
    RhiBlendStateDesc material_blend_state{};
    if (request.loaded_material != nullptr) {
        const RuntimeAssetDataStatus material_status =
            PackRuntimeAssetMaterialConstants(*request.loaded_material, &material_constants);
        if (material_status != RuntimeAssetDataStatus::Success) {
            result->status = material_status;
            return result->status;
        }

        const RuntimeAssetDataStatus blend_status =
            BuildRuntimeAssetMaterialBlendState(*request.loaded_material, &material_blend_state);
        if (blend_status != RuntimeAssetDataStatus::Success) {
            result->status = blend_status;
            return result->status;
        }
    }

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
    material_request.blend_state = material_blend_state;
    material_request.texture_slots = std::span<const RenderSceneRuntimeMaterialTextureSlot>(
        render_slots.data(),
        request.material_slots.size());
    material_request.material_constant_bytes = std::span<const std::uint8_t>(
        material_constants.bytes.data(),
        material_constants.byte_count);

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
    result->material_constant_byte_count = material_constants.byte_count;
    result->material_constant_hash = material_constants.hash;
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
    std::span<const std::uint8_t> initial_bytes,
    RhiBufferHandle *out_handle) {
    if (out_handle == nullptr || byte_count == 0U) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RhiBufferDesc desc{};
    desc.usage = usage;
    desc.size_bytes = byte_count;
    const RhiStatus status = device.CreateBuffer(desc, initial_bytes, *out_handle);
    return status == RhiStatus::Success ? RuntimeAssetDataStatus::Success : RuntimeAssetDataStatus::RhiCaptureFailed;
}

struct RuntimeAssetVisualProofMeshPayload final {
    std::vector<std::uint8_t> bytes{};
    std::uint32_t vertex_byte_count = 0U;
    std::uint32_t index_byte_count = 0U;
};

RuntimeAssetDataStatus ResolveVisualProofMeshPayloadByteCounts(
    const RuntimeAssetLoadedFile &mesh,
    std::uint32_t *out_vertex_byte_count,
    std::uint32_t *out_index_byte_count) {
    if (out_vertex_byte_count == nullptr || out_index_byte_count == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (mesh.vertex_count == 0U ||
        mesh.index_count == 0U ||
        mesh.mesh_vertex_stride_bytes == 0U ||
        mesh.mesh_index_stride_bytes == 0U ||
        mesh.decoded_byte_count == 0U ||
        mesh.payload_byte_count == 0U) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    const std::uint64_t vertex_byte_count =
        static_cast<std::uint64_t>(mesh.vertex_count) *
        static_cast<std::uint64_t>(mesh.mesh_vertex_stride_bytes);
    const std::uint64_t index_byte_count =
        static_cast<std::uint64_t>(mesh.index_count) *
        static_cast<std::uint64_t>(mesh.mesh_index_stride_bytes);
    const std::uint64_t payload_byte_count = vertex_byte_count + index_byte_count;
    const std::uint64_t max_u32 = static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max());
    if (vertex_byte_count > max_u32 ||
        index_byte_count > max_u32 ||
        payload_byte_count > max_u32) {
        return RuntimeAssetDataStatus::BudgetExceeded;
    }

    if (payload_byte_count != static_cast<std::uint64_t>(mesh.payload_byte_count) ||
        payload_byte_count != static_cast<std::uint64_t>(mesh.decoded_byte_count)) {
        return RuntimeAssetDataStatus::InvalidSize;
    }

    *out_vertex_byte_count = static_cast<std::uint32_t>(vertex_byte_count);
    *out_index_byte_count = static_cast<std::uint32_t>(index_byte_count);
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ReadVisualProofDecodedMeshPayload(
    ResourceRegistry &registry,
    const RuntimeAssetLoadedFile &mesh,
    RuntimeAssetVisualProofMeshPayload *out_payload) {
    if (out_payload == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    *out_payload = RuntimeAssetVisualProofMeshPayload{};
    if (mesh.kind != RuntimeAssetFileKind::Mesh ||
        mesh.payload_hash == 0U ||
        !mesh.decoded_payload_stored ||
        mesh.decode_plan_payload_id == 0U ||
        mesh.decode_plan_id == 0U ||
        mesh.decode_result_id == 0U ||
        mesh.decoded_payload_id == 0U ||
        mesh.decode_asset_class != ResourceDecodePlanAssetClass::Mesh ||
        mesh.decode_result_class != ResourceDecodeResultClass::Mesh) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    std::uint32_t vertex_byte_count = 0U;
    std::uint32_t index_byte_count = 0U;
    RuntimeAssetDataStatus status = ResolveVisualProofMeshPayloadByteCounts(
        mesh,
        &vertex_byte_count,
        &index_byte_count);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    std::vector<std::uint8_t> bytes{};
    bytes.assign(mesh.decoded_byte_count, 0U);

    ResourceDecodedPayloadRequest request{};
    request.resource = mesh.resource;
    request.expected_type = mesh.resource_type;
    request.payload_id = mesh.decode_plan_payload_id;
    request.decode_plan_id = mesh.decode_plan_id;
    request.decode_result_id = mesh.decode_result_id;
    request.decoded_payload_id = mesh.decoded_payload_id;
    request.asset_class = mesh.decode_asset_class;
    request.result_class = mesh.decode_result_class;
    request.decoded_byte_count = mesh.decoded_byte_count;

    std::uint32_t read_byte_count = 0U;
    const std::uint32_t byte_capacity = static_cast<std::uint32_t>(bytes.size());
    const ResourceDecodedPayloadStatus payload_status = registry.ReadDecodedPayload(
        request,
        bytes.data(),
        byte_capacity,
        &read_byte_count);
    if (payload_status != ResourceDecodedPayloadStatus::Success ||
        read_byte_count != mesh.decoded_byte_count) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    const std::span<const std::uint8_t> decoded_bytes(bytes.data(), bytes.size());
    const std::uint64_t payload_hash = HashRuntimeAssetDataBytes(decoded_bytes);
    if (payload_hash != mesh.payload_hash) {
        return RuntimeAssetDataStatus::HashMismatch;
    }

    out_payload->bytes = std::move(bytes);
    out_payload->vertex_byte_count = vertex_byte_count;
    out_payload->index_byte_count = index_byte_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus BuildVisualProofGeometry(
    ResourceRegistry &registry,
    const RuntimeAssetLoadedFile &mesh,
    std::uint32_t index,
    IRhiDevice &device,
    RenderScenePrimitiveGeometryRecord *out_geometry,
    RuntimeAssetVisualProofResult *result) {
    if (out_geometry == nullptr) {
        return FailVisualProof(
            RuntimeAssetDataStatus::InvalidArgument,
            RuntimeAssetVisualProofMissingLayer::Model,
            result);
    }

    if (!IsInputLayoutValid(mesh.mesh_input_layout) ||
        mesh.mesh_input_layout.stride_bytes == 0U ||
        mesh.mesh_topology != yuengine::rhi::RhiPrimitiveTopology::TriangleList ||
        mesh.mesh_index_format != RhiIndexFormat::Uint16 ||
        static_cast<std::size_t>(mesh.mesh_vertex_stride_bytes) != mesh.mesh_input_layout.stride_bytes ||
        mesh.mesh_index_stride_bytes != sizeof(std::uint16_t)) {
        return FailVisualProof(
            RuntimeAssetDataStatus::InvalidInputLayout,
            RuntimeAssetVisualProofMissingLayer::Model,
            result);
    }

    const std::size_t vertex_stride_bytes = mesh.mesh_input_layout.stride_bytes;
    std::uint32_t segment_count = 0U;
    std::uint32_t vertex_count = 0U;
    std::uint32_t index_count = 0U;
    if (!ResolveVisualProofMeshCounts(mesh, &segment_count, &vertex_count, &index_count)) {
        return FailVisualProof(
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetVisualProofMissingLayer::Model,
            result);
    }

    RuntimeAssetVisualProofMeshPayload mesh_payload{};
    RuntimeAssetDataStatus status = ReadVisualProofDecodedMeshPayload(
        registry,
        mesh,
        &mesh_payload);
    if (status != RuntimeAssetDataStatus::Success) {
        return FailVisualProof(status, RuntimeAssetVisualProofMissingLayer::Model, result);
    }

    const std::size_t vertex_byte_count = static_cast<std::size_t>(mesh_payload.vertex_byte_count);
    const std::size_t index_byte_count = static_cast<std::size_t>(mesh_payload.index_byte_count);
    const std::uint8_t *payload_data = mesh_payload.bytes.data();
    const std::uint8_t *index_payload_data = payload_data + vertex_byte_count;
    const std::span<const std::uint8_t> vertex_initial_bytes(payload_data, vertex_byte_count);
    const std::span<const std::uint8_t> index_initial_bytes(index_payload_data, index_byte_count);

    RhiBufferHandle vertex_buffer{};
    status = CreateVisualProofBuffer(
        device,
        RhiBufferUsage::Vertex,
        vertex_byte_count,
        vertex_initial_bytes,
        &vertex_buffer);
    if (status != RuntimeAssetDataStatus::Success) {
        return FailVisualProof(status, RuntimeAssetVisualProofMissingLayer::RhiCapture, result);
    }

    RhiBufferHandle index_buffer{};
    status = CreateVisualProofBuffer(
        device,
        RhiBufferUsage::Index,
        index_byte_count,
        index_initial_bytes,
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
        vertex_byte_count};
    request.index_buffer = RhiIndexBufferView{
        index_buffer,
        0U,
        index_byte_count,
        RhiIndexFormat::Uint16};

    RenderScenePrimitiveGeometryBuilder builder;
    const RenderScenePrimitiveGeometryStatus geometry_status = builder.Build(request, out_geometry);
    if (geometry_status != RenderScenePrimitiveGeometryStatus::Success) {
        return FailVisualProof(
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetVisualProofMissingLayer::Model,
            result);
    }

    if (result != nullptr) {
        ++result->mesh_decoded_payload_count;
        result->mesh_vertex_payload_byte_count += vertex_byte_count;
        result->mesh_index_payload_byte_count += index_byte_count;
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

    if (request.shader_program != nullptr &&
        request.shader_program->status != RuntimeAssetDataStatus::Success) {
        return FailVisualProof(
            request.shader_program->status,
            RuntimeAssetVisualProofMissingLayer::ShaderPipeline,
            result);
    }

    if (shader == nullptr ||
        shader->shader_stage_count < 2U ||
        shader->shader_bytecode_byte_count == 0U ||
        request.shader_program == nullptr ||
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

RuntimeAssetDataStatus ResolveVisualProofCameraTarget(
    IRhiDevice &device,
    RhiTextureHandle *out_target) {
    if (out_target == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RhiTextureHandle target{};
    if (device.GetSwapchainColorTarget(target) == RhiStatus::Success) {
        *out_target = target;
        return RuntimeAssetDataStatus::Success;
    }

    RhiColorTargetDesc target_desc{};
    target_desc.format = RhiFormat::Rgba8Unorm;
    target_desc.extent = {
        static_cast<std::uint16_t>(RUNTIME_ASSET_VISUAL_PROOF_TARGET_WIDTH),
        static_cast<std::uint16_t>(RUNTIME_ASSET_VISUAL_PROOF_TARGET_HEIGHT)};
    if (device.CreateColorTarget(target_desc, target) == RhiStatus::Success) {
        *out_target = target;
        return RuntimeAssetDataStatus::Success;
    }

    return RuntimeAssetDataStatus::RhiCaptureFailed;
}

RuntimeAssetDataStatus BuildVisualProofMaterial(
    const RuntimeAssetVisualProofRequest &request,
    const RuntimeAssetLoadedFile &material,
    RenderSceneRuntimeMaterialRecord *out_material,
    RuntimeAssetVisualProofResult *result) {
    if (material.texture_slot_count < RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT) {
        return FailVisualProof(
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetVisualProofMissingLayer::MaterialSlot,
            result);
    }

    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT> textures{};
    for (std::uint32_t index = 0U; index < textures.size(); ++index) {
        const RuntimeAssetLoadedFile *texture = nullptr;
        RuntimeAssetDataStatus status = RequireVisualProofRecord(
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
    material_request.loaded_material = &material;
    material_request.material_asset = material.asset;
    material_request.material_id = RUNTIME_ASSET_VISUAL_PROOF_MATERIAL_ID;
    material_request.pipeline = result->shader_pipeline_result.pipeline;
    material_request.textures = std::span<const RuntimeAssetCookedTexturePayloadDesc>(textures.data(), textures.size());
    material_request.material_slots = std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size());
    material_request.scratch_bytes = request.scratch_bytes;
    material_request.out_material = out_material;

    RuntimeAssetDataStatus status = BuildRuntimeAssetCookedTextureMaterialBridge(material_request, &result->material_result);
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

    if (material == nullptr) {
        return FailVisualProof(
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetVisualProofMissingLayer::MaterialSlot,
            result);
    }

    return BuildVisualProofMaterial(request, *material, out_material, result);
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
    if (ResolveVisualProofCameraTarget(*request.rhi_device, &target) != RuntimeAssetDataStatus::Success) {
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

void CleanupVisualProofFailureResources(
    const RuntimeAssetVisualProofRequest &request,
    RhiBufferHandle buffer_slot_guard,
    RuntimeAssetVisualProofResult *result) {
    if (request.rhi_device == nullptr || result == nullptr) {
        return;
    }

    if (IsBufferHandleSet(buffer_slot_guard)) {
        request.rhi_device->DestroyBuffer(buffer_slot_guard);
    }

    RuntimeAssetShaderProgramPipelineResult *pipeline_result = &result->shader_pipeline_result;
    if (IsPipelineHandleSet(pipeline_result->pipeline)) {
        request.rhi_device->DestroyPipeline(pipeline_result->pipeline);
        pipeline_result->pipeline = {};
    }

    if (IsShaderModuleHandleSet(pipeline_result->pixel_shader)) {
        request.rhi_device->DestroyShaderModule(pipeline_result->pixel_shader);
        pipeline_result->pixel_shader = {};
    }

    if (IsShaderModuleHandleSet(pipeline_result->vertex_shader)) {
        request.rhi_device->DestroyShaderModule(pipeline_result->vertex_shader);
        pipeline_result->vertex_shader = {};
    }
}

RuntimeAssetDataStatus PublishVisualProofRouteResult(
    const RuntimeAssetVisualProofRequest &request,
    RhiBufferHandle buffer_slot_guard,
    RuntimeAssetVisualProofResult *result,
    RuntimeAssetVisualProofResult *out_result) {
    if (result == nullptr || out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (result->status != RuntimeAssetDataStatus::Success) {
        CleanupVisualProofFailureResources(request, buffer_slot_guard, result);
    }

    *out_result = *result;
    return result->status;
}

constexpr std::uint32_t RUNTIME_ASSET_SUBMISSION_INVALID_INDEX = 0xFFFFFFFFU;

struct RuntimeAssetRenderSceneSubmissionValidation final {
    const RenderSceneRuntimeMaterialRecord *shared_material = nullptr;
    std::uint32_t shared_material_ref_index = RUNTIME_ASSET_SUBMISSION_INVALID_INDEX;
    std::uint32_t submitted_entity_count = 0U;
    std::uint32_t skipped_entity_count = 0U;
    std::uint32_t resolved_geometry_count = 0U;
    std::uint32_t resolved_material_count = 0U;
    std::uint32_t material_variant_count = 0U;
    std::uint32_t material_table_count = 0U;
};

bool IsRenderSceneSubmissionEntityActive(const RuntimeAssetSceneEntityRecord &entity) {
    if (!entity.is_active) {
        return false;
    }

    return entity.is_visible;
}

RenderSceneRuntimeFrameStatus MapSubmissionGeometryStatus(
    RenderScenePrimitiveGeometryStatus status) {
    if (status == RenderScenePrimitiveGeometryStatus::MissingGeometryRecord) {
        return RenderSceneRuntimeFrameStatus::MissingGeometryRecord;
    }

    return RenderSceneRuntimeFrameStatus::InvalidGeometryRecord;
}

RenderSceneRuntimeFrameStatus MapSubmissionMaterialStatus(
    RenderSceneRuntimeMaterialStatus status) {
    if (status == RenderSceneRuntimeMaterialStatus::MissingMaterialRecord) {
        return RenderSceneRuntimeFrameStatus::MissingMaterialRecord;
    }

    return RenderSceneRuntimeFrameStatus::InvalidMaterialRecord;
}

RuntimeAssetDataStatus MapSubmissionFrameStatus(
    RenderSceneRuntimeFrameStatus status) {
    if (status == RenderSceneRuntimeFrameStatus::Success) {
        return RuntimeAssetDataStatus::Success;
    }

    if (status == RenderSceneRuntimeFrameStatus::OutputCapacityExceeded) {
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    if (status == RenderSceneRuntimeFrameStatus::DuplicateWorldObject ||
        status == RenderSceneRuntimeFrameStatus::DuplicateTransform ||
        status == RenderSceneRuntimeFrameStatus::DuplicateMaterialRecord) {
        return RuntimeAssetDataStatus::DuplicateDependency;
    }

    if (status == RenderSceneRuntimeFrameStatus::MissingCamera ||
        status == RenderSceneRuntimeFrameStatus::MissingEntity ||
        status == RenderSceneRuntimeFrameStatus::MissingGeometryRecord ||
        status == RenderSceneRuntimeFrameStatus::MissingMaterialRecord) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    return RuntimeAssetDataStatus::InvalidDependency;
}

void SetRenderSceneSubmissionFailure(
    RuntimeAssetRenderSceneSubmissionResult *result,
    RuntimeAssetDataStatus status,
    RenderSceneRuntimeFrameStatus frame_status,
    std::uint32_t entity_index,
    std::uint32_t resource_ref_index) {
    if (result == nullptr) {
        return;
    }

    result->status = status;
    result->frame_status = frame_status;
    result->first_failed_entity_index = entity_index;
    result->first_missing_resource_ref_index = resource_ref_index;
}

RuntimeAssetDataStatus ValidateRenderSceneSubmissionCamera(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    RuntimeAssetRenderSceneSubmissionResult *result) {
    if (request.camera.status != RenderSceneStatus::Success) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::MissingCamera,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (!request.camera.camera.is_active || request.camera.camera.camera_id == 0U) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::MissingCamera,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::MissingDependency;
    }

    return RuntimeAssetDataStatus::Success;
}

bool IsSameSubmissionTransform(
    const WorldTransformState &left,
    const WorldTransformState &right) {
    if (left.translation_x != right.translation_x) {
        return false;
    }

    if (left.translation_y != right.translation_y) {
        return false;
    }

    if (left.translation_z != right.translation_z) {
        return false;
    }

    if (left.rotation_x != right.rotation_x) {
        return false;
    }

    if (left.rotation_y != right.rotation_y) {
        return false;
    }

    if (left.rotation_z != right.rotation_z) {
        return false;
    }

    if (left.rotation_w != right.rotation_w) {
        return false;
    }

    if (left.scale_x != right.scale_x) {
        return false;
    }

    if (left.scale_y != right.scale_y) {
        return false;
    }

    return left.scale_z == right.scale_z;
}

const RuntimeAssetSceneTransformOutputRecord *FindRenderSceneSubmissionTransform(
    std::span<const RuntimeAssetSceneTransformOutputRecord> transforms,
    std::uint32_t transform_count,
    WorldObjectId world_object_id) {
    for (std::uint32_t index = 0U; index < transform_count; ++index) {
        const RuntimeAssetSceneTransformOutputRecord &transform = transforms[index];
        if (transform.world_object_id.value == world_object_id.value) {
            return &transform;
        }
    }

    return nullptr;
}

const RuntimeAssetRenderSceneGeometryBinding *FindRenderSceneSubmissionGeometry(
    std::span<const RuntimeAssetRenderSceneGeometryBinding> bindings,
    std::uint32_t resource_ref_index) {
    for (const RuntimeAssetRenderSceneGeometryBinding &binding : bindings) {
        if (binding.resource_ref_index == resource_ref_index) {
            return &binding;
        }
    }

    return nullptr;
}

const RuntimeAssetRenderSceneMaterialBinding *FindRenderSceneSubmissionMaterial(
    std::span<const RuntimeAssetRenderSceneMaterialBinding> bindings,
    std::uint32_t resource_ref_index) {
    for (const RuntimeAssetRenderSceneMaterialBinding &binding : bindings) {
        if (binding.resource_ref_index == resource_ref_index) {
            return &binding;
        }
    }

    return nullptr;
}

bool HasDuplicateRenderSceneSubmissionGeometryBinding(
    std::span<const RuntimeAssetRenderSceneGeometryBinding> bindings,
    std::uint32_t *out_resource_ref_index) {
    if (out_resource_ref_index == nullptr) {
        return false;
    }

    for (std::size_t left = 0U; left < bindings.size(); ++left) {
        const std::uint32_t ref_index = bindings[left].resource_ref_index;
        for (std::size_t right = left + 1U; right < bindings.size(); ++right) {
            if (bindings[right].resource_ref_index == ref_index) {
                *out_resource_ref_index = ref_index;
                return true;
            }
        }
    }

    return false;
}

bool HasDuplicateRenderSceneSubmissionMaterialBinding(
    std::span<const RuntimeAssetRenderSceneMaterialBinding> bindings,
    std::uint32_t *out_resource_ref_index) {
    if (out_resource_ref_index == nullptr) {
        return false;
    }

    for (std::size_t left = 0U; left < bindings.size(); ++left) {
        const std::uint32_t ref_index = bindings[left].resource_ref_index;
        for (std::size_t right = left + 1U; right < bindings.size(); ++right) {
            if (bindings[right].resource_ref_index == ref_index) {
                *out_resource_ref_index = ref_index;
                return true;
            }
        }
    }

    return false;
}

bool HasPriorRenderSceneSubmissionWorldObject(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    std::uint32_t current_index,
    WorldObjectId world_object_id) {
    for (std::uint32_t index = 0U; index < current_index; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = request.scene_entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        if (entity.world_object_id.value == world_object_id.value) {
            return true;
        }
    }

    return false;
}

bool HasPriorRenderSceneSubmissionTransform(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    std::uint32_t current_index,
    const WorldTransformState &transform) {
    const std::uint32_t transform_count = request.scene_output->transform_count;
    for (std::uint32_t index = 0U; index < current_index; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = request.scene_entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        const RuntimeAssetSceneTransformOutputRecord *prior_transform =
            FindRenderSceneSubmissionTransform(
                request.scene_transforms,
                transform_count,
                entity.world_object_id);
        if (prior_transform == nullptr) {
            continue;
        }

        if (IsSameSubmissionTransform(prior_transform->transform, transform)) {
            return true;
        }
    }

    return false;
}

bool HasPriorRenderSceneSubmissionMaterialRef(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    std::uint32_t current_index,
    std::uint32_t material_ref_index) {
    for (std::uint32_t index = 0U; index < current_index; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = request.scene_entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        if (entity.material_ref_index == material_ref_index) {
            return true;
        }
    }

    return false;
}

bool HasPriorRenderSceneSubmissionMaterialId(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    std::uint32_t current_index,
    std::uint32_t material_id) {
    for (std::uint32_t index = 0U; index < current_index; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = request.scene_entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        if (HasPriorRenderSceneSubmissionMaterialRef(request, index, entity.material_ref_index)) {
            continue;
        }

        const RuntimeAssetRenderSceneMaterialBinding *material =
            FindRenderSceneSubmissionMaterial(request.material_bindings, entity.material_ref_index);
        if (material == nullptr) {
            continue;
        }

        if (material->material.material_id == material_id) {
            return true;
        }
    }

    return false;
}

std::uint32_t FindRenderSceneSubmissionMaterialTableIndex(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    std::uint32_t current_index) {
    const RuntimeAssetSceneEntityRecord &current_entity = request.scene_entities[current_index];
    bool found_current_ref = false;
    std::uint32_t table_index = 0U;
    for (std::uint32_t index = 0U; index < request.scene_output->entity_count; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = request.scene_entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        if (HasPriorRenderSceneSubmissionMaterialRef(request, index, entity.material_ref_index)) {
            continue;
        }

        if (entity.material_ref_index < current_entity.material_ref_index) {
            ++table_index;
            continue;
        }

        if (entity.material_ref_index == current_entity.material_ref_index) {
            found_current_ref = true;
        }
    }

    if (found_current_ref) {
        return table_index;
    }

    return RUNTIME_ASSET_SUBMISSION_INVALID_INDEX;
}

bool FindNextRenderSceneSubmissionMaterialRef(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    bool has_lower_bound,
    std::uint32_t lower_bound_ref_index,
    std::uint32_t *out_ref_index) {
    if (out_ref_index == nullptr) {
        return false;
    }

    bool found = false;
    std::uint32_t best_ref_index = RUNTIME_ASSET_SUBMISSION_INVALID_INDEX;
    for (std::uint32_t index = 0U; index < request.scene_output->entity_count; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = request.scene_entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        if (has_lower_bound && entity.material_ref_index <= lower_bound_ref_index) {
            continue;
        }

        if (found && entity.material_ref_index >= best_ref_index) {
            continue;
        }

        best_ref_index = entity.material_ref_index;
        found = true;
    }

    if (!found) {
        return false;
    }

    *out_ref_index = best_ref_index;
    return true;
}

RuntimeAssetDataStatus ValidateRenderSceneSubmissionStorage(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    RuntimeAssetRenderSceneSubmissionResult *result) {
    if (request.scene_output == nullptr) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::InvalidArgument,
            RenderSceneRuntimeFrameStatus::NullPointer,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.scene_output->status != RuntimeAssetDataStatus::Success) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::MissingEntity,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::MissingDependency;
    }

    const std::size_t entity_count = static_cast<std::size_t>(request.scene_output->entity_count);
    const std::size_t transform_count = static_cast<std::size_t>(request.scene_output->transform_count);
    if (entity_count == 0U ||
        request.scene_entities.size() < entity_count ||
        request.scene_transforms.size() < transform_count ||
        request.scene_entities.data() == nullptr ||
        request.scene_transforms.data() == nullptr) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::MissingEntity,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (request.frame_id == 0U) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::InvalidArgument,
            RenderSceneRuntimeFrameStatus::InvalidFrameId,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (request.geometry_bindings.empty() ||
        request.material_bindings.empty() ||
        request.geometry_bindings.data() == nullptr ||
        request.material_bindings.data() == nullptr) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::MissingGeometryRecord,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (request.out_frame_entities.data() == nullptr || request.out_draws.data() == nullptr) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::InvalidArgument,
            RenderSceneRuntimeFrameStatus::NullPointer,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (!request.require_shared_material && request.out_frame_materials.data() == nullptr) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::CapacityExceeded,
            RenderSceneRuntimeFrameStatus::OutputCapacityExceeded,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    std::uint32_t duplicate_ref_index = 0U;
    if (HasDuplicateRenderSceneSubmissionGeometryBinding(request.geometry_bindings, &duplicate_ref_index)) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::DuplicateDependency,
            RenderSceneRuntimeFrameStatus::InvalidGeometryRecord,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            duplicate_ref_index);
        return RuntimeAssetDataStatus::DuplicateDependency;
    }

    if (HasDuplicateRenderSceneSubmissionMaterialBinding(request.material_bindings, &duplicate_ref_index)) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::DuplicateDependency,
            RenderSceneRuntimeFrameStatus::InvalidMaterialRecord,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            duplicate_ref_index);
        return RuntimeAssetDataStatus::DuplicateDependency;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateRenderSceneSubmissionEntity(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    std::uint32_t entity_index,
    RuntimeAssetRenderSceneSubmissionValidation *validation,
    RuntimeAssetRenderSceneSubmissionResult *result) {
    if (validation == nullptr || result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const RuntimeAssetSceneEntityRecord &entity = request.scene_entities[entity_index];
    if (!IsRenderSceneSubmissionEntityActive(entity)) {
        ++validation->skipped_entity_count;
        return RuntimeAssetDataStatus::Success;
    }

    if (!entity.world_object_id.IsValid()) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::InvalidDependency,
            RenderSceneRuntimeFrameStatus::InvalidEntityRecord,
            entity_index,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    if (HasPriorRenderSceneSubmissionWorldObject(request, entity_index, entity.world_object_id)) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::DuplicateDependency,
            RenderSceneRuntimeFrameStatus::DuplicateWorldObject,
            entity_index,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::DuplicateDependency;
    }

    const RuntimeAssetSceneTransformOutputRecord *transform =
        FindRenderSceneSubmissionTransform(
            request.scene_transforms,
            request.scene_output->transform_count,
            entity.world_object_id);
    if (transform == nullptr) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::MissingEntity,
            entity_index,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (HasPriorRenderSceneSubmissionTransform(request, entity_index, transform->transform)) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::DuplicateDependency,
            RenderSceneRuntimeFrameStatus::DuplicateTransform,
            entity_index,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::DuplicateDependency;
    }

    const RuntimeAssetRenderSceneGeometryBinding *geometry =
        FindRenderSceneSubmissionGeometry(request.geometry_bindings, entity.mesh_ref_index);
    if (geometry == nullptr) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::MissingGeometryRecord,
            entity_index,
            entity.mesh_ref_index);
        return RuntimeAssetDataStatus::MissingDependency;
    }

    RenderScenePrimitiveGeometryBuilder geometry_builder;
    const RenderScenePrimitiveGeometryStatus geometry_status =
        geometry_builder.Validate(geometry->geometry);
    if (geometry_status != RenderScenePrimitiveGeometryStatus::Success) {
        const RenderSceneRuntimeFrameStatus frame_status =
            MapSubmissionGeometryStatus(geometry_status);
        SetRenderSceneSubmissionFailure(
            result,
            MapSubmissionFrameStatus(frame_status),
            frame_status,
            entity_index,
            entity.mesh_ref_index);
        return result->status;
    }

    const RuntimeAssetRenderSceneMaterialBinding *material =
        FindRenderSceneSubmissionMaterial(request.material_bindings, entity.material_ref_index);
    if (material == nullptr) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::MissingMaterialRecord,
            entity_index,
            entity.material_ref_index);
        return RuntimeAssetDataStatus::MissingDependency;
    }

    RenderSceneRuntimeMaterialBuilder material_builder;
    const RenderSceneRuntimeMaterialStatus material_status =
        material_builder.Validate(material->material);
    if (material_status != RenderSceneRuntimeMaterialStatus::Success) {
        const RenderSceneRuntimeFrameStatus frame_status =
            MapSubmissionMaterialStatus(material_status);
        SetRenderSceneSubmissionFailure(
            result,
            MapSubmissionFrameStatus(frame_status),
            frame_status,
            entity_index,
            entity.material_ref_index);
        return result->status;
    }

    if (validation->shared_material == nullptr) {
        validation->shared_material = &material->material;
        validation->shared_material_ref_index = entity.material_ref_index;
        if (request.require_shared_material) {
            validation->material_variant_count = 1U;
        }
    }

    if (request.require_shared_material) {
        if (validation->shared_material_ref_index != entity.material_ref_index) {
            validation->material_variant_count = 2U;
            result->material_variant_count = validation->material_variant_count;
            SetRenderSceneSubmissionFailure(
                result,
                RuntimeAssetDataStatus::UnsupportedFieldValue,
                RenderSceneRuntimeFrameStatus::InvalidMaterialRecord,
                entity_index,
                entity.material_ref_index);
            return RuntimeAssetDataStatus::UnsupportedFieldValue;
        }

        ++validation->submitted_entity_count;
        ++validation->resolved_geometry_count;
        validation->resolved_material_count = validation->material_variant_count;
        validation->material_table_count = validation->material_variant_count;
        return RuntimeAssetDataStatus::Success;
    }

    if (!HasPriorRenderSceneSubmissionMaterialRef(request, entity_index, entity.material_ref_index)) {
        if (HasPriorRenderSceneSubmissionMaterialId(request, entity_index, material->material.material_id)) {
            result->material_variant_count = validation->material_variant_count + 1U;
            result->material_table_count = result->material_variant_count;
            SetRenderSceneSubmissionFailure(
                result,
                RuntimeAssetDataStatus::DuplicateDependency,
                RenderSceneRuntimeFrameStatus::DuplicateMaterialRecord,
                entity_index,
                entity.material_ref_index);
            return RuntimeAssetDataStatus::DuplicateDependency;
        }

        ++validation->material_variant_count;
        validation->material_table_count = validation->material_variant_count;
        if (static_cast<std::size_t>(validation->material_table_count) > request.out_frame_materials.size()) {
            result->material_variant_count = validation->material_variant_count;
            SetRenderSceneSubmissionFailure(
                result,
                RuntimeAssetDataStatus::CapacityExceeded,
                RenderSceneRuntimeFrameStatus::OutputCapacityExceeded,
                entity_index,
                entity.material_ref_index);
            return RuntimeAssetDataStatus::CapacityExceeded;
        }
    }

    ++validation->submitted_entity_count;
    ++validation->resolved_geometry_count;
    validation->resolved_material_count = validation->material_table_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateRenderSceneSubmissionRequest(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    RuntimeAssetRenderSceneSubmissionValidation *validation,
    RuntimeAssetRenderSceneSubmissionResult *result) {
    if (validation == nullptr || result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetDataStatus status = ValidateRenderSceneSubmissionStorage(request, result);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = ValidateRenderSceneSubmissionCamera(request, result);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    const std::uint32_t entity_count = request.scene_output->entity_count;
    for (std::uint32_t index = 0U; index < entity_count; ++index) {
        status = ValidateRenderSceneSubmissionEntity(request, index, validation, result);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }
    }

    if (validation->submitted_entity_count == 0U) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::MissingEntity,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (validation->submitted_entity_count > request.out_frame_entities.size() ||
        validation->submitted_entity_count > request.out_draws.size()) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::CapacityExceeded,
            RenderSceneRuntimeFrameStatus::OutputCapacityExceeded,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    result->submitted_entity_count = validation->submitted_entity_count;
    result->skipped_entity_count = validation->skipped_entity_count;
    result->resolved_geometry_count = validation->resolved_geometry_count;
    result->resolved_material_count = validation->resolved_material_count;
    result->material_variant_count = validation->material_variant_count;
    result->material_table_count = validation->material_table_count;
    result->shared_material_ref_index = validation->shared_material_ref_index;
    return RuntimeAssetDataStatus::Success;
}

void FillRenderSceneSubmissionEntities(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    std::uint32_t submitted_entity_count) {
    std::uint32_t output_index = 0U;
    for (std::uint32_t index = 0U; index < request.scene_output->entity_count; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = request.scene_entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        const RuntimeAssetSceneTransformOutputRecord *transform =
            FindRenderSceneSubmissionTransform(
                request.scene_transforms,
                request.scene_output->transform_count,
                entity.world_object_id);
        const RuntimeAssetRenderSceneGeometryBinding *geometry =
            FindRenderSceneSubmissionGeometry(request.geometry_bindings, entity.mesh_ref_index);
        if (transform == nullptr || geometry == nullptr) {
            return;
        }

        RenderSceneRuntimeFrameEntityRequest &out_entity = request.out_frame_entities[output_index];
        out_entity.world_object_id = entity.world_object_id;
        out_entity.transform = transform->transform;
        out_entity.geometry = geometry->geometry;
        out_entity.material_table_index = 0U;
        if (!request.require_shared_material) {
            out_entity.material_table_index =
                FindRenderSceneSubmissionMaterialTableIndex(request, index);
        }
        out_entity.is_visible = entity.is_visible;
        out_entity.is_active = entity.is_active;
        ++output_index;
        if (output_index == submitted_entity_count) {
            return;
        }
    }
}

void FillRenderSceneSubmissionMaterials(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    std::uint32_t material_table_count) {
    bool has_lower_bound = false;
    std::uint32_t lower_bound_ref_index = 0U;
    for (std::uint32_t output_index = 0U; output_index < material_table_count; ++output_index) {
        std::uint32_t material_ref_index = 0U;
        if (!FindNextRenderSceneSubmissionMaterialRef(
                request,
                has_lower_bound,
                lower_bound_ref_index,
                &material_ref_index)) {
            return;
        }

        const RuntimeAssetRenderSceneMaterialBinding *material =
            FindRenderSceneSubmissionMaterial(request.material_bindings, material_ref_index);
        if (material == nullptr) {
            return;
        }

        request.out_frame_materials[output_index] = material->material;
        lower_bound_ref_index = material_ref_index;
        has_lower_bound = true;
    }
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
        case RuntimeAssetFileKind::Camera:
            return "CAMERA";
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

RuntimeAssetDataStatus PackRuntimeAssetMaterialConstants(
    const RuntimeAssetLoadedFile &material,
    RuntimeAssetPackedMaterialConstants *out_constants) {
    if (out_constants == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetPackedMaterialConstants constants{};
    if (material.kind != RuntimeAssetFileKind::Material) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    if (!material.asset.IsValid()) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (material.material_parameter_count != RUNTIME_ASSET_MATERIAL_PARAMETER_COUNT) {
        return RuntimeAssetDataStatus::InvalidCount;
    }

    if (material.material_alpha_mode != RuntimeAssetMaterialAlphaMode::Opaque &&
        material.material_alpha_mode != RuntimeAssetMaterialAlphaMode::Blend) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    if (material.material_emissive_strength > 255U ||
        material.material_metallic > 255U ||
        material.material_roughness > 255U ||
        material.material_opacity > 255U) {
        return RuntimeAssetDataStatus::InvalidBounds;
    }

    static_assert(
        RUNTIME_ASSET_PACKED_MATERIAL_CONSTANT_BYTES <=
        yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_CONSTANT_BYTES,
        "packed material constants must fit render scene material constants");
    constexpr std::uint32_t BYTE_MASK = 0xFFU;
    constants.byte_count = RUNTIME_ASSET_PACKED_MATERIAL_CONSTANT_BYTES;
    constants.bytes[0U] = static_cast<std::uint8_t>((material.material_base_color_rgba >> 24U) & BYTE_MASK);
    constants.bytes[1U] = static_cast<std::uint8_t>((material.material_base_color_rgba >> 16U) & BYTE_MASK);
    constants.bytes[2U] = static_cast<std::uint8_t>((material.material_base_color_rgba >> 8U) & BYTE_MASK);
    constants.bytes[3U] = static_cast<std::uint8_t>(material.material_base_color_rgba & BYTE_MASK);
    constants.bytes[4U] = static_cast<std::uint8_t>(material.material_emissive_strength);
    constants.bytes[5U] = static_cast<std::uint8_t>(material.material_metallic);
    constants.bytes[6U] = static_cast<std::uint8_t>(material.material_roughness);
    constants.bytes[7U] = static_cast<std::uint8_t>(material.material_opacity);
    constants.bytes[8U] = static_cast<std::uint8_t>(material.material_alpha_mode);
    constants.hash = HashRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(constants.bytes.data(), constants.byte_count));

    *out_constants = constants;
    return RuntimeAssetDataStatus::Success;
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

    if (expected_kind == RuntimeAssetFileKind::Camera) {
        result.status = ValidateCameraMetadata(text, &result);
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

    status = DecodeRuntimeInputLayout(ValueForToken(text, "input="), &data.input_layout);
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

RuntimeAssetDataStatus CompileRuntimeAssetShaderProgram(
    const RuntimeAssetShaderCompilerBackendRequest &request,
    RuntimeAssetShaderCompilerBackendResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetShaderCompilerBackendResult result{};
    result.backend_kind = request.backend_kind;
    if (request.backend_kind != RuntimeAssetShaderCompilerBackendKind::DeterministicFixture) {
        result.status = RuntimeAssetDataStatus::UnsupportedFieldValue;
        *out_result = result;
        return result.status;
    }

    if (request.source_bytes.empty()) {
        result.status = RuntimeAssetDataStatus::InvalidSize;
        *out_result = result;
        return result.status;
    }

    if (request.program_id == 0U) {
        result.status = RuntimeAssetDataStatus::InvalidArgument;
        *out_result = result;
        return result.status;
    }

    RuntimeAssetLoadedShaderProgramData program{};
    RuntimeAssetDataStatus status = DecodeRuntimeAssetShaderProgramData(
        request.source_bytes,
        request.program_id,
        &program);
    result.program = program;
    result.import_policy_hash = program.validation.shader_import_policy_hash;
    result.compiled_shader_stage_count = program.validation.shader_stage_count;
    result.reflection_input_element_count =
        static_cast<std::uint32_t>(program.input_layout.element_count);
    result.reflection_texture_slot_count = program.texture_slot_count;
    result.vertex_bytecode_hash = program.vertex_bytecode_hash;
    result.pixel_bytecode_hash = program.pixel_bytecode_hash;
    if (status != RuntimeAssetDataStatus::Success) {
        result.status = status;
        *out_result = result;
        return result.status;
    }

    if (request.expected_import_policy_hash != 0U &&
        request.expected_import_policy_hash != result.import_policy_hash) {
        result.status = RuntimeAssetDataStatus::HashMismatch;
        result.program.status = result.status;
        *out_result = result;
        return result.status;
    }

    result.compiled_program = true;
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
    RhiBufferHandle buffer_slot_guard{};
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
        return PublishVisualProofRouteResult(request, buffer_slot_guard, &result, out_result);
    }

    if (request.rhi_device == nullptr) {
        result.status = RuntimeAssetDataStatus::RhiCaptureFailed;
        result.first_missing_layer = RuntimeAssetVisualProofMissingLayer::RhiCapture;
        return PublishVisualProofRouteResult(request, buffer_slot_guard, &result, out_result);
    }

    RuntimeAssetDataStatus status = ValidateVisualProofSceneOutputs(request, &result);
    if (status != RuntimeAssetDataStatus::Success) {
        return PublishVisualProofRouteResult(request, buffer_slot_guard, &result, out_result);
    }

    status = BuildVisualProofShaderPipeline(request, &result);
    if (status != RuntimeAssetDataStatus::Success) {
        return PublishVisualProofRouteResult(request, buffer_slot_guard, &result, out_result);
    }

    status = CreateVisualProofBuffer(
        *request.rhi_device,
        RhiBufferUsage::Vertex,
        sizeof(float) * 2U,
        std::span<const std::uint8_t>{},
        &buffer_slot_guard);
    if (status != RuntimeAssetDataStatus::Success) {
        FailVisualProof(status, RuntimeAssetVisualProofMissingLayer::RhiCapture, &result);
        return PublishVisualProofRouteResult(request, buffer_slot_guard, &result, out_result);
    }

    std::array<RenderScenePrimitiveGeometryRecord, RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT> geometry{};
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
            return PublishVisualProofRouteResult(request, buffer_slot_guard, &result, out_result);
        }

        status = BuildVisualProofGeometry(
            *request.resource_registry,
            *mesh,
            index,
            *request.rhi_device,
            &geometry[index],
            &result);
        if (status != RuntimeAssetDataStatus::Success) {
            return PublishVisualProofRouteResult(request, buffer_slot_guard, &result, out_result);
        }
    }

    const bool mesh_payload_count_ready =
        result.mesh_decoded_payload_count == RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT;
    const bool mesh_payload_bytes_ready =
        result.mesh_vertex_payload_byte_count > 0U &&
        result.mesh_index_payload_byte_count > 0U;
    result.mesh_buffers_from_decoded_payloads =
        mesh_payload_count_ready &&
        mesh_payload_bytes_ready;

    RenderSceneRuntimeMaterialRecord material{};
    status = BuildVisualProofMaterial(request, &material, &result);
    if (status != RuntimeAssetDataStatus::Success) {
        return PublishVisualProofRouteResult(request, buffer_slot_guard, &result, out_result);
    }

    std::array<RenderSceneThreePrimitiveEntityRequest, RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT> entities{};
    status = BuildVisualProofEntities(
        request,
        std::span<const RenderScenePrimitiveGeometryRecord>(geometry.data(), geometry.size()),
        &entities,
        &result);
    if (status != RuntimeAssetDataStatus::Success) {
        return PublishVisualProofRouteResult(request, buffer_slot_guard, &result, out_result);
    }

    status = ExecuteVisualProofCaptureFrames(
        request,
        material,
        std::span<const RenderSceneThreePrimitiveEntityRequest>(entities.data(), entities.size()),
        &result);
    if (status != RuntimeAssetDataStatus::Success) {
        return PublishVisualProofRouteResult(request, buffer_slot_guard, &result, out_result);
    }

    result.loaded_records_verified =
        (!request.require_cooked_records || result.source_record_count == 0U) &&
        result.cooked_record_count >= (request.loaded_files.size() + 1U) &&
        result.mesh_record_count >= RUNTIME_ASSET_VISUAL_PROOF_ENTITY_COUNT &&
        result.texture_record_count >= RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT;
    result.status = RuntimeAssetDataStatus::Success;
    result.first_missing_layer = RuntimeAssetVisualProofMissingLayer::None;
    return PublishVisualProofRouteResult(request, buffer_slot_guard, &result, out_result);
}

RuntimeAssetDataStatus BuildRuntimeAssetRenderSceneSubmission(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    RuntimeAssetRenderSceneSubmissionResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetRenderSceneSubmissionResult result{};
    result.frame_id = request.frame_id;
    RuntimeAssetRenderSceneSubmissionValidation validation{};
    result.status = ValidateRenderSceneSubmissionRequest(request, &validation, &result);
    if (result.status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return result.status;
    }

    if (validation.shared_material == nullptr) {
        result.status = RuntimeAssetDataStatus::MissingDependency;
        result.frame_status = RenderSceneRuntimeFrameStatus::MissingMaterialRecord;
        *out_result = result;
        return result.status;
    }

    FillRenderSceneSubmissionEntities(request, validation.submitted_entity_count);

    RenderSceneRuntimeFrameRequest frame_request{};
    frame_request.frame_id = request.frame_id;
    frame_request.camera = request.camera;
    frame_request.material = *validation.shared_material;
    frame_request.entities = request.out_frame_entities.subspan(
        0U,
        static_cast<std::size_t>(validation.submitted_entity_count));
    if (!request.require_shared_material) {
        FillRenderSceneSubmissionMaterials(request, validation.material_table_count);
        frame_request.materials = request.out_frame_materials.subspan(
            0U,
            static_cast<std::size_t>(validation.material_table_count));
        if (!frame_request.materials.empty()) {
            frame_request.material = frame_request.materials[0U];
        }
    }

    RenderSceneRuntimeFrameBuilder frame_builder;
    result.frame_status = frame_builder.Build(
        frame_request,
        request.out_draws,
        &result.frame_result);
    result.output_draw_count =
        static_cast<std::uint32_t>(result.frame_result.output_draw_count);
    if (result.frame_status != RenderSceneRuntimeFrameStatus::Success) {
        result.status = MapSubmissionFrameStatus(result.frame_status);
        *out_result = result;
        return result.status;
    }

    result.status = RuntimeAssetDataStatus::Success;
    result.submitted_entity_count =
        static_cast<std::uint32_t>(result.frame_result.submitted_entity_count);
    result.skipped_entity_count +=
        static_cast<std::uint32_t>(result.frame_result.skipped_entity_count);
    *out_result = result;
    return result.status;
}

namespace {
constexpr const char *RUNTIME_ASSET_PACKAGED_RUN_MODULE = "RuntimeAssetPackagedRun";
constexpr const char *RUNTIME_ASSET_PACKAGED_RUN_UPDATE_FAILURE =
    "runtime asset packaged run module failed";
constexpr const char *RUNTIME_ASSET_PACKAGE_LOGICAL_KEY_PREFIX = "runtime_asset_";
constexpr const char *RUNTIME_ASSET_PACKAGE_SOURCE_KEY_PREFIX = "cooked_record_";

std::string RuntimeAssetPackageLogicalKeyForStableId(std::uint64_t stable_id) {
    return std::string(RUNTIME_ASSET_PACKAGE_LOGICAL_KEY_PREFIX) + std::to_string(stable_id);
}

std::string RuntimeAssetPackageSourceKeyForStableId(std::uint64_t stable_id) {
    return std::string(RUNTIME_ASSET_PACKAGE_SOURCE_KEY_PREFIX) + std::to_string(stable_id);
}

std::uint64_t MixRuntimeAssetPackagePayloadHash(std::uint64_t hash, std::uint64_t value) {
    hash ^= value;
    return hash * FNV_PRIME;
}

std::uint64_t HashRuntimeAssetPackagePayloadText(std::uint64_t hash, std::string_view text) {
    for (const char character : text) {
        hash = MixRuntimeAssetPackagePayloadHash(
            hash,
            static_cast<std::uint64_t>(static_cast<unsigned char>(character)));
    }

    return hash;
}

std::uint64_t MakeRuntimeAssetPackagePayloadHash(const PackageLoadPlanRecord &record) {
    std::uint64_t hash = FNV_OFFSET;
    hash = HashRuntimeAssetPackagePayloadText(hash, record.source_key.Value());
    hash = MixRuntimeAssetPackagePayloadHash(hash, record.archive_byte_offset);
    hash = MixRuntimeAssetPackagePayloadHash(hash, record.archive_byte_size);
    if (hash == 0ULL) {
        return FNV_OFFSET;
    }

    return hash;
}

bool RuntimeAssetPackageRecordRangeIsValid(const PackageLoadPlanRecord &record) {
    if (record.archive_byte_size == 0ULL) {
        return false;
    }

    const std::uint64_t max_value = std::numeric_limits<std::uint64_t>::max();
    return record.archive_byte_offset <= max_value - record.archive_byte_size;
}

bool RuntimeAssetPackageRecordsHaveSameIdentity(
    const PackageLoadPlanRecord &left,
    const PackageLoadPlanRecord &right) {
    if (left.package.value == right.package.value &&
        left.entry.value == right.entry.value) {
        return true;
    }

    if (left.type.value != right.type.value) {
        return false;
    }

    if (left.logical_key.Value() != right.logical_key.Value()) {
        return false;
    }

    return left.source_key.Value() == right.source_key.Value();
}

RuntimeAssetDataStatus FailRuntimeAssetPackagedValidation(
    RuntimeAssetPackagedRunResult *result,
    PackageStatus package_status,
    RuntimeAssetDataStatus status,
    std::uint32_t first_failed_record_index) {
    result->package_status = package_status;
    result->packaged_validation.status = status;
    result->packaged_validation.package_status = package_status;
    result->packaged_validation.first_failed_record_index = first_failed_record_index;
    return status;
}

bool RuntimeAssetPackageRecordMatchesDesc(
    const PackageLoadPlanRecord &record,
    const RuntimeAssetFileDesc &desc) {
    if (record.type.value != desc.resource_type.value) {
        return false;
    }

    if (record.archive_byte_size == 0ULL) {
        return false;
    }

    const std::string logical_key = RuntimeAssetPackageLogicalKeyForStableId(desc.stable_id);
    if (record.logical_key.Value() != std::string_view(logical_key)) {
        return false;
    }

    const std::string source_key = RuntimeAssetPackageSourceKeyForStableId(desc.stable_id);
    return record.source_key.Value() == std::string_view(source_key);
}

bool FindRuntimeAssetPackageRecordIndexForDesc(
    const yuengine::package::PackageLoadPlan &plan,
    const RuntimeAssetFileDesc &desc,
    std::uint32_t *out_index) {
    if (out_index == nullptr) {
        return false;
    }

    for (std::uint32_t index = 0U; index < plan.record_count; ++index) {
        if (RuntimeAssetPackageRecordMatchesDesc(plan.records[index], desc)) {
            *out_index = index;
            return true;
        }
    }

    return false;
}

RuntimeAssetDataStatus ValidateRuntimeAssetPackageRecordUniqueness(
    const yuengine::package::PackageLoadPlan &plan,
    RuntimeAssetPackagedRunResult *result) {
    for (std::uint32_t left_index = 0U; left_index < plan.record_count; ++left_index) {
        for (std::uint32_t right_index = left_index + 1U; right_index < plan.record_count; ++right_index) {
            if (RuntimeAssetPackageRecordsHaveSameIdentity(plan.records[left_index], plan.records[right_index])) {
                return FailRuntimeAssetPackagedValidation(
                    result,
                    PackageStatus::DuplicateResourceKey,
                    RuntimeAssetDataStatus::DuplicateDependency,
                    right_index);
            }
        }
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateRuntimeAssetPackageRecordPayload(
    const RuntimeAssetPackagedRunRequest &request,
    const PackageLoadPlanRecord &record,
    const RuntimeAssetFileDesc &desc,
    std::uint32_t record_index,
    RuntimeAssetPackagedRunResult *result) {
    if (request.mount_table == nullptr || desc.path == nullptr) {
        return FailRuntimeAssetPackagedValidation(
            result,
            PackageStatus::NotFound,
            RuntimeAssetDataStatus::InvalidArgument,
            record_index);
    }

    if (!RuntimeAssetPackageRecordRangeIsValid(record)) {
        const RuntimeAssetDataStatus status =
            record.archive_byte_size == 0ULL
                ? RuntimeAssetDataStatus::InvalidSize
                : RuntimeAssetDataStatus::InvalidBounds;
        return FailRuntimeAssetPackagedValidation(
            result,
            PackageStatus::ByteRangeOutOfBounds,
            status,
            record_index);
    }

    const std::uint64_t expected_payload_hash = MakeRuntimeAssetPackagePayloadHash(record);
    if (record.payload_hash == 0ULL || record.payload_hash != expected_payload_hash) {
        return FailRuntimeAssetPackagedValidation(
            result,
            PackageStatus::ArtifactHashMismatch,
            RuntimeAssetDataStatus::HashMismatch,
            record_index);
    }

    FileReadRequest read_request{};
    read_request.mount = request.mount;
    read_request.path = yuengine::file::VirtualPath(desc.path);
    read_request.use_range = true;
    read_request.range_byte_offset = record.archive_byte_offset;
    read_request.range_byte_size = record.archive_byte_size;
    const FileReadResult read_result = request.mount_table->Read(read_request);
    if (!read_result.Succeeded()) {
        return FailRuntimeAssetPackagedValidation(
            result,
            PackageStatus::FileReadFailed,
            RuntimeAssetDataStatus::FileReadFailed,
            record_index);
    }

    const std::uint64_t read_byte_count = static_cast<std::uint64_t>(read_result.bytes.size());
    if (read_byte_count != record.archive_byte_size) {
        return FailRuntimeAssetPackagedValidation(
            result,
            PackageStatus::ByteRangeOutOfBounds,
            RuntimeAssetDataStatus::InvalidSize,
            record_index);
    }

    RuntimeAssetValidationResult validation{};
    RuntimeAssetDataStatus status = ValidateRuntimeAssetDataBytes(read_result.bytes, desc.kind, &validation);
    if (status != RuntimeAssetDataStatus::Success) {
        return FailRuntimeAssetPackagedValidation(
            result,
            PackageStatus::InvalidArtifact,
            status,
            record_index);
    }

    result->packaged_validation.validated_archive_byte_count += record.archive_byte_size;
    ++result->packaged_validation.validated_record_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ValidateRuntimeAssetPackageLoadPlan(
    const RuntimeAssetPackagedRunRequest &request,
    RuntimeAssetPackagedRunResult *result) {
    if (request.package_load_plan == nullptr || result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const yuengine::package::PackageLoadPlan &plan = *request.package_load_plan;
    result->package_load_plan_record_count = plan.record_count;
    result->packaged_validation.record_count = plan.record_count;
    result->packaged_validation.archive_byte_count = plan.archive_byte_count;
    if (plan.record_count == 0U) {
        return FailRuntimeAssetPackagedValidation(
            result,
            PackageStatus::NotFound,
            RuntimeAssetDataStatus::InvalidCount,
            0U);
    }

    if (request.files == nullptr || request.file_count == 0U) {
        return FailRuntimeAssetPackagedValidation(
            result,
            PackageStatus::NotFound,
            RuntimeAssetDataStatus::InvalidArgument,
            0U);
    }

    if (plan.record_count != request.file_count + 1U) {
        return FailRuntimeAssetPackagedValidation(
            result,
            PackageStatus::LoadPlanCapacityExceeded,
            RuntimeAssetDataStatus::InvalidCount,
            plan.record_count);
    }

    RuntimeAssetDataStatus status = ValidateRuntimeAssetPackageRecordUniqueness(plan, result);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    const PackageLoadPlanRecord &scene_record = plan.records[plan.record_count - 1U];
    if (!RuntimeAssetPackageRecordMatchesDesc(scene_record, request.scene)) {
        return FailRuntimeAssetPackagedValidation(
            result,
            PackageStatus::TypeMismatch,
            RuntimeAssetDataStatus::TypeMismatch,
            plan.record_count - 1U);
    }

    status = ValidateRuntimeAssetPackageRecordPayload(
        request,
        scene_record,
        request.scene,
        plan.record_count - 1U,
        result);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    for (std::uint32_t index = 0U; index < request.file_count; ++index) {
        std::uint32_t record_index = 0U;
        if (!FindRuntimeAssetPackageRecordIndexForDesc(plan, request.files[index], &record_index)) {
            return FailRuntimeAssetPackagedValidation(
                result,
                PackageStatus::NotFound,
                RuntimeAssetDataStatus::MissingDependency,
                index);
        }

        status = ValidateRuntimeAssetPackageRecordPayload(
            request,
            plan.records[record_index],
            request.files[index],
            record_index,
            result);
        if (status != RuntimeAssetDataStatus::Success) {
            return status;
        }
    }

    if (result->packaged_validation.validated_archive_byte_count != plan.archive_byte_count) {
        return FailRuntimeAssetPackagedValidation(
            result,
            PackageStatus::LoadPlanByteBudgetExceeded,
            RuntimeAssetDataStatus::BudgetExceeded,
            plan.record_count);
    }

    result->package_status = PackageStatus::Success;
    result->packaged_validation.status = RuntimeAssetDataStatus::Success;
    result->packaged_validation.package_status = PackageStatus::Success;
    result->packaged_validation.dependency_records_validated = true;
    result->packaged_validation.archive_ranges_validated = true;
    result->packaged_validation.payload_hashes_validated = true;
    result->packaged_validation.runtime_asset_payloads_validated = true;
    result->package_load_plan_consumed = true;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus ReadRuntimeAssetPackagedBytes(
    MountTable &mount_table,
    yuengine::file::MountId mount,
    const char *path,
    std::vector<std::uint8_t> *out_bytes) {
    if (path == nullptr || out_bytes == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const FileReadResult read_result = mount_table.Read({mount, yuengine::file::VirtualPath(path)});
    if (!read_result.Succeeded()) {
        return RuntimeAssetDataStatus::FileReadFailed;
    }

    *out_bytes = read_result.bytes;
    return RuntimeAssetDataStatus::Success;
}

const RuntimeAssetFileDesc *FindRuntimeAssetPackagedFile(
    const RuntimeAssetPackagedRunRequest &request,
    RuntimeAssetFileKind kind) {
    if (request.files == nullptr) {
        return nullptr;
    }

    for (std::uint32_t index = 0U; index < request.file_count; ++index) {
        if (request.files[index].kind == kind) {
            return &request.files[index];
        }
    }

    return nullptr;
}

RuntimeAssetDataStatus DecodeRuntimeAssetPackagedShader(
    const RuntimeAssetPackagedRunRequest &request,
    RuntimeAssetPackagedRunResult *result) {
    if (request.mount_table == nullptr ||
        request.shader_program == nullptr ||
        result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const RuntimeAssetFileDesc *shader_desc =
        FindRuntimeAssetPackagedFile(request, RuntimeAssetFileKind::Shader);
    if (shader_desc == nullptr) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    std::vector<std::uint8_t> bytes{};
    RuntimeAssetDataStatus status = ReadRuntimeAssetPackagedBytes(
        *request.mount_table,
        request.mount,
        shader_desc->path,
        &bytes);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    status = DecodeRuntimeAssetShaderProgramData(
        std::span<const std::uint8_t>(bytes.data(), bytes.size()),
        static_cast<std::uint32_t>(shader_desc->stable_id),
        request.shader_program);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    if (request.shader_program->validation.artifact_class != RuntimeAssetArtifactClass::Cooked) {
        return RuntimeAssetDataStatus::InvalidSchema;
    }

    result->shader_program_decoded = true;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetGraphLoadRequest BuildRuntimeAssetPackagedGraphLoadRequest(
    const RuntimeAssetPackagedRunRequest &request) {
    RuntimeAssetGraphLoadRequest load_request{};
    load_request.mount_table = request.mount_table;
    load_request.mount = request.mount;
    load_request.scene_path = yuengine::file::VirtualPath(request.scene.path);
    load_request.scene_resource_type = request.scene.resource_type;
    load_request.scene_asset_type = request.scene.asset_type;
    load_request.scene_stable_id = request.scene.stable_id;
    load_request.files = request.files;
    load_request.file_count = request.file_count;
    load_request.resource_registry = request.resource_registry;
    load_request.asset_manager = request.asset_manager;
    load_request.loaded_files = request.loaded_files;
    load_request.loaded_file_capacity = request.loaded_file_capacity;
    load_request.scene_resource_refs = request.scene_resource_refs;
    load_request.scene_resource_ref_capacity = request.scene_resource_ref_capacity;
    load_request.scene_cameras = request.scene_cameras;
    load_request.scene_camera_capacity = request.scene_camera_capacity;
    load_request.scene_entities = request.scene_entities;
    load_request.scene_entity_capacity = request.scene_entity_capacity;
    load_request.scene_transforms = request.scene_transforms;
    load_request.scene_transform_capacity = request.scene_transform_capacity;
    load_request.target_identities = request.target_identities;
    load_request.target_identity_capacity = request.target_identity_capacity;
    load_request.runtime_instance_mappings = request.runtime_instance_mappings;
    load_request.runtime_instance_mapping_capacity = request.runtime_instance_mapping_capacity;
    load_request.animation_clips = request.animation_clips;
    load_request.animation_clip_capacity = request.animation_clip_capacity;
    load_request.animation_tracks = request.animation_tracks;
    load_request.animation_track_capacity = request.animation_track_capacity;
    load_request.animation_target_bindings = request.animation_target_bindings;
    load_request.animation_target_binding_capacity = request.animation_target_binding_capacity;
    load_request.animation_keyframes = request.animation_keyframes;
    load_request.animation_keyframe_capacity = request.animation_keyframe_capacity;
    load_request.scene_output = request.scene_output;
    load_request.animation_frame_context = request.animation_frame_context;
    load_request.selected_animation_clip_id = request.selected_animation_clip_id;
    load_request.animation_clip_start_time_nanoseconds =
        request.animation_clip_start_time_nanoseconds;
    return load_request;
}

RuntimeAssetVisualProofRequest BuildRuntimeAssetPackagedVisualProofRequest(
    const RuntimeAssetPackagedRunRequest &request,
    const RuntimeAssetGraphLoadResult &graph_load_result) {
    RuntimeAssetVisualProofRequest proof_request{};
    proof_request.resource_registry = request.resource_registry;
    proof_request.asset_manager = request.asset_manager;
    proof_request.rhi_device = request.rhi_device;
    proof_request.scene = &graph_load_result.scene;
    proof_request.loaded_files =
        std::span<const RuntimeAssetLoadedFile>(request.loaded_files, graph_load_result.loaded_file_count);
    proof_request.scene_cameras =
        std::span<const RuntimeAssetSceneCameraRecord>(request.scene_cameras, request.scene_output->camera_count);
    proof_request.scene_entities =
        std::span<const RuntimeAssetSceneEntityRecord>(request.scene_entities, request.scene_output->entity_count);
    proof_request.scene_transforms =
        std::span<const RuntimeAssetSceneTransformOutputRecord>(request.scene_transforms, request.scene_output->transform_count);
    proof_request.scene_output = request.scene_output;
    proof_request.shader_program = request.shader_program;
    proof_request.scratch_bytes = request.scratch_bytes;
    proof_request.capture_output = request.capture_output;
    proof_request.capture_byte_budget_per_entity = request.capture_byte_budget_per_entity;
    proof_request.first_frame_id = request.first_frame_id;
    proof_request.frame_count = request.visual_frame_count;
    proof_request.output_path = request.output_path;
    proof_request.output_path_byte_count = request.output_path_byte_count;
    proof_request.require_cooked_records = true;
    return proof_request;
}

bool HasPriorRuntimeAssetPackagedGenericMeshRef(
    std::span<const RuntimeAssetSceneEntityRecord> entities,
    std::uint32_t current_index,
    std::uint32_t mesh_ref_index) {
    for (std::uint32_t index = 0U; index < current_index; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        if (entity.mesh_ref_index == mesh_ref_index) {
            return true;
        }
    }

    return false;
}

bool HasPriorRuntimeAssetPackagedGenericMaterialRef(
    std::span<const RuntimeAssetSceneEntityRecord> entities,
    std::uint32_t current_index,
    std::uint32_t material_ref_index) {
    for (std::uint32_t index = 0U; index < current_index; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        if (entity.material_ref_index == material_ref_index) {
            return true;
        }
    }

    return false;
}

RuntimeAssetDataStatus FindRuntimeAssetPackagedGenericLoadedFile(
    const RuntimeAssetPackagedRunRequest &request,
    const RuntimeAssetGraphLoadResult &graph_load_result,
    RuntimeAssetFileKind kind,
    std::uint32_t resource_ref_index,
    const RuntimeAssetLoadedFile **out_file) {
    if (out_file == nullptr ||
        request.scene_output == nullptr ||
        request.scene_resource_refs == nullptr ||
        request.loaded_files == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (resource_ref_index >= request.scene_output->resource_ref_count ||
        resource_ref_index >= request.scene_resource_ref_capacity) {
        *out_file = nullptr;
        return RuntimeAssetDataStatus::MissingDependency;
    }

    const RuntimeAssetSceneResourceRef &resource_ref =
        request.scene_resource_refs[resource_ref_index];
    if (resource_ref.kind != kind ||
        resource_ref.loaded_file_index >= graph_load_result.loaded_file_count ||
        resource_ref.loaded_file_index >= request.loaded_file_capacity) {
        *out_file = nullptr;
        return RuntimeAssetDataStatus::MissingDependency;
    }

    const RuntimeAssetLoadedFile &file = request.loaded_files[resource_ref.loaded_file_index];
    if (!IsLoadedRuntimeAssetRecordUsable(file, kind, true)) {
        *out_file = nullptr;
        return RuntimeAssetDataStatus::MissingDependency;
    }

    *out_file = &file;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus CountRuntimeAssetPackagedGenericSubmissionBindings(
    const RuntimeAssetPackagedRunRequest &request,
    const RuntimeAssetGraphLoadResult &graph_load_result,
    RuntimeAssetRenderSceneSubmissionResult *result,
    std::uint32_t *out_geometry_count,
    std::uint32_t *out_material_count) {
    if (result == nullptr || out_geometry_count == nullptr || out_material_count == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const std::uint32_t entity_count = request.scene_output->entity_count;
    const std::span<const RuntimeAssetSceneEntityRecord> entities(request.scene_entities, entity_count);
    std::uint32_t geometry_count = 0U;
    std::uint32_t material_count = 0U;
    for (std::uint32_t index = 0U; index < entity_count; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        const RuntimeAssetLoadedFile *mesh = nullptr;
        if (!HasPriorRuntimeAssetPackagedGenericMeshRef(entities, index, entity.mesh_ref_index)) {
            const RuntimeAssetDataStatus status = FindRuntimeAssetPackagedGenericLoadedFile(
                request,
                graph_load_result,
                RuntimeAssetFileKind::Mesh,
                entity.mesh_ref_index,
                &mesh);
            if (status != RuntimeAssetDataStatus::Success) {
                SetRenderSceneSubmissionFailure(
                    result,
                    status,
                    RenderSceneRuntimeFrameStatus::MissingGeometryRecord,
                    index,
                    entity.mesh_ref_index);
                return status;
            }

            ++geometry_count;
        }

        const RuntimeAssetLoadedFile *material = nullptr;
        if (!HasPriorRuntimeAssetPackagedGenericMaterialRef(entities, index, entity.material_ref_index)) {
            const RuntimeAssetDataStatus status = FindRuntimeAssetPackagedGenericLoadedFile(
                request,
                graph_load_result,
                RuntimeAssetFileKind::Material,
                entity.material_ref_index,
                &material);
            if (status != RuntimeAssetDataStatus::Success) {
                SetRenderSceneSubmissionFailure(
                    result,
                    status,
                    RenderSceneRuntimeFrameStatus::MissingMaterialRecord,
                    index,
                    entity.material_ref_index);
                return status;
            }

            ++material_count;
        }
    }

    if (geometry_count > request.generic_geometry_bindings.size() ||
        material_count > request.generic_material_bindings.size()) {
        SetRenderSceneSubmissionFailure(
            result,
            RuntimeAssetDataStatus::CapacityExceeded,
            RenderSceneRuntimeFrameStatus::OutputCapacityExceeded,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        return RuntimeAssetDataStatus::CapacityExceeded;
    }

    *out_geometry_count = geometry_count;
    *out_material_count = material_count;
    return RuntimeAssetDataStatus::Success;
}

RhiBufferHandle BuildRuntimeAssetPackagedGenericBufferHandle(std::uint32_t slot) {
    return RhiBufferHandle{50000U + slot, 1U};
}

RhiTextureHandle BuildRuntimeAssetPackagedGenericTextureHandle(std::uint32_t slot) {
    return RhiTextureHandle{51000U + slot, 1U};
}

RhiSamplerHandle BuildRuntimeAssetPackagedGenericSamplerHandle(std::uint32_t slot) {
    return RhiSamplerHandle{52000U + slot, 1U};
}

RuntimeAssetDataStatus BuildRuntimeAssetPackagedGenericGeometryRecord(
    const RuntimeAssetLoadedFile &mesh,
    std::uint32_t geometry_index,
    RenderScenePrimitiveGeometryRecord *out_geometry) {
    if (out_geometry == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (!IsInputLayoutValid(mesh.mesh_input_layout) ||
        mesh.mesh_input_layout.stride_bytes == 0U ||
        mesh.mesh_topology != yuengine::rhi::RhiPrimitiveTopology::TriangleList ||
        mesh.mesh_index_format != RhiIndexFormat::Uint16 ||
        static_cast<std::size_t>(mesh.mesh_vertex_stride_bytes) != mesh.mesh_input_layout.stride_bytes ||
        mesh.mesh_index_stride_bytes != sizeof(std::uint16_t)) {
        return RuntimeAssetDataStatus::InvalidInputLayout;
    }

    std::uint32_t segment_count = 0U;
    std::uint32_t vertex_count = 0U;
    std::uint32_t index_count = 0U;
    if (!ResolveVisualProofMeshCounts(mesh, &segment_count, &vertex_count, &index_count)) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    const std::size_t vertex_stride_bytes = mesh.mesh_input_layout.stride_bytes;
    const std::size_t vertex_byte_count = vertex_stride_bytes * vertex_count;
    const std::size_t index_byte_count =
        static_cast<std::size_t>(mesh.mesh_index_stride_bytes) * index_count;

    RhiVertexBufferView vertex_buffer{};
    vertex_buffer.buffer = BuildRuntimeAssetPackagedGenericBufferHandle((geometry_index * 2U) + 1U);
    vertex_buffer.stride_bytes = vertex_stride_bytes;
    vertex_buffer.size_bytes = vertex_byte_count;

    RhiIndexBufferView index_buffer{};
    index_buffer.buffer = BuildRuntimeAssetPackagedGenericBufferHandle((geometry_index * 2U) + 2U);
    index_buffer.size_bytes = index_byte_count;
    index_buffer.format = RhiIndexFormat::Uint16;

    RenderScenePrimitiveGeometryRequest geometry_request{};
    geometry_request.geometry_asset = mesh.asset;
    geometry_request.kind = ToRenderScenePrimitiveKind(mesh.mesh_geometry_kind);
    geometry_request.segment_count = segment_count;
    geometry_request.draw_id = RUNTIME_ASSET_VISUAL_PROOF_FIRST_DRAW_ID + geometry_index;
    geometry_request.pass_id = RUNTIME_ASSET_VISUAL_PROOF_PASS_ID;
    geometry_request.material_id = RUNTIME_ASSET_VISUAL_PROOF_MATERIAL_ID;
    geometry_request.vertex_buffer = vertex_buffer;
    geometry_request.index_buffer = index_buffer;

    RenderScenePrimitiveGeometryBuilder builder;
    const RenderScenePrimitiveGeometryStatus status = builder.Build(geometry_request, out_geometry);
    if (status != RenderScenePrimitiveGeometryStatus::Success) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus BuildRuntimeAssetPackagedGenericMaterialRecord(
    const RuntimeAssetVisualProofRequest &proof_request,
    const RuntimeAssetLoadedFile &material,
    std::uint32_t material_index,
    RenderSceneRuntimeMaterialRecord *out_material) {
    if (out_material == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetPackedMaterialConstants material_constants{};
    RuntimeAssetDataStatus status = PackRuntimeAssetMaterialConstants(material, &material_constants);
    if (status != RuntimeAssetDataStatus::Success) {
        return status;
    }

    std::array<RenderSceneRuntimeMaterialTextureSlot, RUNTIME_ASSET_VISUAL_PROOF_TEXTURE_SLOT_COUNT>
        texture_slots{};
    for (std::uint32_t index = 0U; index < texture_slots.size(); ++index) {
        const RuntimeAssetLoadedFile *texture = nullptr;
        status = RequireVisualProofRecord(
            proof_request,
            RuntimeAssetFileKind::Texture,
            index,
            RuntimeAssetVisualProofMissingLayer::MaterialSlot,
            &texture,
            nullptr);
        if (status != RuntimeAssetDataStatus::Success || texture == nullptr) {
            return RuntimeAssetDataStatus::MissingDependency;
        }

        RenderSceneRuntimeMaterialTextureSlot &slot = texture_slots[index];
        slot.slot = index;
        slot.texture_asset = texture->asset;
        slot.sampled_texture = RhiSampledTextureBinding{
            BuildRuntimeAssetPackagedGenericTextureHandle(index + 1U),
            index};
        slot.sampler = RhiSamplerBinding{
            BuildRuntimeAssetPackagedGenericSamplerHandle(index + 1U),
            index};
    }

    RenderSceneRuntimeMaterialRequest material_request{};
    material_request.material_asset = material.asset;
    material_request.material_id = RUNTIME_ASSET_VISUAL_PROOF_MATERIAL_ID + material_index;
    material_request.pipeline = RhiPipelineHandle{53000U + material_index, 1U};
    material_request.texture_slots = std::span<const RenderSceneRuntimeMaterialTextureSlot>(
        texture_slots.data(),
        texture_slots.size());
    material_request.material_constant_bytes = std::span<const std::uint8_t>(
        material_constants.bytes.data(),
        material_constants.byte_count);

    RenderSceneRuntimeMaterialBuilder builder;
    const RenderSceneRuntimeMaterialStatus material_status =
        builder.Build(material_request, out_material);
    if (material_status != RenderSceneRuntimeMaterialStatus::Success) {
        return RuntimeAssetDataStatus::RenderSceneMaterialFailed;
    }

    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus BuildRuntimeAssetPackagedGenericGeometryBindings(
    const RuntimeAssetPackagedRunRequest &request,
    const RuntimeAssetGraphLoadResult &graph_load_result,
    RuntimeAssetRenderSceneSubmissionResult *result,
    std::uint32_t *out_geometry_count) {
    if (result == nullptr || out_geometry_count == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const std::uint32_t entity_count = request.scene_output->entity_count;
    const std::span<const RuntimeAssetSceneEntityRecord> entities(request.scene_entities, entity_count);
    std::uint32_t geometry_count = 0U;
    for (std::uint32_t index = 0U; index < entity_count; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        if (HasPriorRuntimeAssetPackagedGenericMeshRef(entities, index, entity.mesh_ref_index)) {
            continue;
        }

        const RuntimeAssetLoadedFile *mesh = nullptr;
        RuntimeAssetDataStatus status = FindRuntimeAssetPackagedGenericLoadedFile(
            request,
            graph_load_result,
            RuntimeAssetFileKind::Mesh,
            entity.mesh_ref_index,
            &mesh);
        if (status != RuntimeAssetDataStatus::Success || mesh == nullptr) {
            SetRenderSceneSubmissionFailure(
                result,
                status,
                RenderSceneRuntimeFrameStatus::MissingGeometryRecord,
                index,
                entity.mesh_ref_index);
            return status;
        }

        RuntimeAssetRenderSceneGeometryBinding &binding =
            request.generic_geometry_bindings[geometry_count];
        status = BuildRuntimeAssetPackagedGenericGeometryRecord(
            *mesh,
            geometry_count,
            &binding.geometry);
        if (status != RuntimeAssetDataStatus::Success) {
            SetRenderSceneSubmissionFailure(
                result,
                status,
                RenderSceneRuntimeFrameStatus::MissingGeometryRecord,
                index,
                entity.mesh_ref_index);
            return status;
        }

        binding.resource_ref_index = entity.mesh_ref_index;
        ++geometry_count;
    }

    *out_geometry_count = geometry_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus BuildRuntimeAssetPackagedGenericMaterialBindings(
    const RuntimeAssetPackagedRunRequest &request,
    const RuntimeAssetGraphLoadResult &graph_load_result,
    const RuntimeAssetVisualProofRequest &proof_request,
    RuntimeAssetRenderSceneSubmissionResult *result,
    std::uint32_t *out_material_count) {
    if (result == nullptr || out_material_count == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const std::uint32_t entity_count = request.scene_output->entity_count;
    const std::span<const RuntimeAssetSceneEntityRecord> entities(request.scene_entities, entity_count);
    std::uint32_t material_count = 0U;
    for (std::uint32_t index = 0U; index < entity_count; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = entities[index];
        if (!IsRenderSceneSubmissionEntityActive(entity)) {
            continue;
        }

        if (HasPriorRuntimeAssetPackagedGenericMaterialRef(entities, index, entity.material_ref_index)) {
            continue;
        }

        const RuntimeAssetLoadedFile *material = nullptr;
        RuntimeAssetDataStatus status = FindRuntimeAssetPackagedGenericLoadedFile(
            request,
            graph_load_result,
            RuntimeAssetFileKind::Material,
            entity.material_ref_index,
            &material);
        if (status != RuntimeAssetDataStatus::Success || material == nullptr) {
            SetRenderSceneSubmissionFailure(
                result,
                status,
                RenderSceneRuntimeFrameStatus::MissingMaterialRecord,
                index,
                entity.material_ref_index);
            return status;
        }

        RuntimeAssetRenderSceneMaterialBinding &binding =
            request.generic_material_bindings[material_count];
        status = BuildRuntimeAssetPackagedGenericMaterialRecord(
            proof_request,
            *material,
            material_count,
            &binding.material);
        if (status != RuntimeAssetDataStatus::Success) {
            SetRenderSceneSubmissionFailure(
                result,
                status,
                RenderSceneRuntimeFrameStatus::MissingMaterialRecord,
                index,
                entity.material_ref_index);
            return status;
        }

        binding.resource_ref_index = entity.material_ref_index;
        ++material_count;
    }

    *out_material_count = material_count;
    return RuntimeAssetDataStatus::Success;
}

RuntimeAssetDataStatus BuildRuntimeAssetPackagedGenericSubmission(
    const RuntimeAssetPackagedRunRequest &request,
    RuntimeAssetPackagedRunResult *result) {
    if (result == nullptr || request.scene_output == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetRenderSceneSubmissionResult submission_result{};
    submission_result.frame_id = request.first_frame_id;
    result->generic_submission_result = submission_result;
    result->generic_render_scene_submission_success = false;

    std::uint32_t expected_geometry_count = 0U;
    std::uint32_t expected_material_count = 0U;
    RuntimeAssetDataStatus status = CountRuntimeAssetPackagedGenericSubmissionBindings(
        request,
        result->graph_load_result,
        &submission_result,
        &expected_geometry_count,
        &expected_material_count);
    if (status != RuntimeAssetDataStatus::Success) {
        result->generic_submission_result = submission_result;
        return status;
    }

    RuntimeAssetVisualProofRequest proof_request =
        BuildRuntimeAssetPackagedVisualProofRequest(request, result->graph_load_result);
    RuntimeAssetVisualProofResult proof_result = result->visual_proof_result;
    std::uint32_t geometry_count = 0U;
    status = BuildRuntimeAssetPackagedGenericGeometryBindings(
        request,
        result->graph_load_result,
        &submission_result,
        &geometry_count);
    if (status != RuntimeAssetDataStatus::Success) {
        result->generic_submission_result = submission_result;
        return status;
    }

    std::uint32_t material_count = 0U;
    status = BuildRuntimeAssetPackagedGenericMaterialBindings(
        request,
        result->graph_load_result,
        proof_request,
        &submission_result,
        &material_count);
    if (status != RuntimeAssetDataStatus::Success) {
        result->generic_submission_result = submission_result;
        return status;
    }

    RenderSceneCameraBindingResult camera{};
    status = BuildVisualProofCamera(
        proof_request,
        request.first_frame_id,
        &camera,
        &proof_result);
    if (status != RuntimeAssetDataStatus::Success) {
        SetRenderSceneSubmissionFailure(
            &submission_result,
            status,
            RenderSceneRuntimeFrameStatus::MissingCamera,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX,
            RUNTIME_ASSET_SUBMISSION_INVALID_INDEX);
        result->generic_submission_result = submission_result;
        return status;
    }

    RuntimeAssetRenderSceneSubmissionRequest submission_request{};
    submission_request.scene_output = request.scene_output;
    submission_request.scene_entities = std::span<const RuntimeAssetSceneEntityRecord>(
        request.scene_entities,
        request.scene_output->entity_count);
    submission_request.scene_transforms = std::span<const RuntimeAssetSceneTransformOutputRecord>(
        request.scene_transforms,
        request.scene_output->transform_count);
    submission_request.geometry_bindings = request.generic_geometry_bindings.subspan(
        0U,
        static_cast<std::size_t>(geometry_count));
    submission_request.material_bindings = request.generic_material_bindings.subspan(
        0U,
        static_cast<std::size_t>(material_count));
    submission_request.camera = camera;
    submission_request.out_frame_entities = request.generic_frame_entities;
    submission_request.out_frame_materials = request.generic_frame_materials;
    submission_request.out_draws = request.generic_draws;
    submission_request.frame_id = request.first_frame_id;
    submission_request.require_shared_material = false;

    status = BuildRuntimeAssetRenderSceneSubmission(
        submission_request,
        &result->generic_submission_result);
    result->generic_render_scene_submission_success =
        status == RuntimeAssetDataStatus::Success &&
        result->generic_submission_result.status == RuntimeAssetDataStatus::Success &&
        result->generic_submission_result.frame_status == RenderSceneRuntimeFrameStatus::Success;
    return status;
}

bool RuntimeAssetPackagedRunRequestHasRequiredPointers(
    const RuntimeAssetPackagedRunRequest &request) {
    return request.mount_table != nullptr &&
        request.resource_registry != nullptr &&
        request.asset_manager != nullptr &&
        request.rhi_device != nullptr &&
        request.loaded_files != nullptr &&
        request.scene_resource_refs != nullptr &&
        request.scene_cameras != nullptr &&
        request.scene_entities != nullptr &&
        request.scene_transforms != nullptr &&
        request.scene_output != nullptr &&
        request.shader_program != nullptr &&
        request.scratch_bytes.data() != nullptr &&
        request.capture_output.data() != nullptr &&
        request.generic_geometry_bindings.data() != nullptr &&
        request.generic_material_bindings.data() != nullptr &&
        request.generic_frame_entities.data() != nullptr &&
        request.generic_draws.data() != nullptr;
}

class RuntimeAssetPackagedRunModule final : public IModule {
public:
    RuntimeAssetPackagedRunModule(
        const RuntimeAssetPackagedRunRequest &request,
        RuntimeAssetPackagedRunResult *result)
        : request_(request),
          result_(result) {
    }

    std::string_view Name() const override {
        return RUNTIME_ASSET_PACKAGED_RUN_MODULE;
    }

    std::vector<std::string_view> Dependencies() const override {
        return std::vector<std::string_view>{};
    }

    std::vector<std::string_view> RequiredServices() const override {
        return std::vector<std::string_view>{};
    }

    std::vector<std::string_view> PublishedServices() const override {
        return std::vector<std::string_view>{};
    }

    KernelResult Start(
        yuengine::kernel::ServiceRegistry &service_registry,
        std::vector<std::string> &lifecycle_trace) override {
        (void)service_registry;
        lifecycle_trace.push_back("module.start.RuntimeAssetPackagedRun");
        return KernelResult::Success();
    }

    KernelResult Update(
        std::uint32_t frame_index,
        std::uint64_t tick_time_nanoseconds,
        std::vector<std::string> &lifecycle_trace) override {
        lifecycle_trace.push_back("module.update.RuntimeAssetPackagedRun");
        if (executed_) {
            return KernelResult::Success();
        }

        executed_ = true;
        RuntimeAssetDataStatus status = Execute(frame_index, tick_time_nanoseconds);
        if (status != RuntimeAssetDataStatus::Success) {
            return KernelResult::Failure(
                KernelStatus::UpdateFailure,
                RUNTIME_ASSET_PACKAGED_RUN_UPDATE_FAILURE);
        }

        return KernelResult::Success();
    }

    KernelResult Shutdown(std::vector<std::string> &lifecycle_trace) override {
        lifecycle_trace.push_back("module.shutdown.RuntimeAssetPackagedRun");
        return KernelResult::Success();
    }

private:
    RuntimeAssetDataStatus Execute(
        std::uint32_t frame_index,
        std::uint64_t tick_time_nanoseconds) {
        if (result_ == nullptr) {
            return RuntimeAssetDataStatus::InvalidArgument;
        }

        RuntimeAssetDataStatus status = ValidateRuntimeAssetPackageLoadPlan(request_, result_);
        if (status != RuntimeAssetDataStatus::Success) {
            result_->status = status;
            result_->blocked_layer = RuntimeAssetPackagedRunBlockedLayer::PackageLoadPlan;
            return status;
        }

        result_->blocked_layer = RuntimeAssetPackagedRunBlockedLayer::RuntimeAssetData;
        RuntimeAssetGraphLoadRequest load_request = BuildRuntimeAssetPackagedGraphLoadRequest(request_);
        if (load_request.animation_frame_context.delta_time_nanoseconds == 0U) {
            load_request.animation_frame_context.frame_index = frame_index;
            load_request.animation_frame_context.delta_time_nanoseconds = tick_time_nanoseconds;
            load_request.animation_frame_context.fixed_time_nanoseconds = tick_time_nanoseconds;
            load_request.animation_frame_context.phase = RuntimeFramePhase::LoadOrCommitResources;
        }

        status = LoadRuntimeAssetDataGraph(load_request, &result_->graph_load_result);
        result_->loaded_file_count = result_->graph_load_result.loaded_file_count;
        result_->resource_dependency_count = result_->graph_load_result.resource_dependency_count;
        result_->asset_dependency_count = result_->graph_load_result.asset_dependency_count;
        result_->runtime_asset_validation_load_success =
            status == RuntimeAssetDataStatus::Success &&
            result_->graph_load_result.status == RuntimeAssetDataStatus::Success &&
            result_->graph_load_result.scene.artifact_class == RuntimeAssetArtifactClass::Cooked &&
            request_.scene_output->status == RuntimeAssetDataStatus::Success;
        if (!result_->runtime_asset_validation_load_success) {
            result_->status = status;
            return status;
        }

        result_->blocked_layer = RuntimeAssetPackagedRunBlockedLayer::ResourceAsset;
        result_->resource_asset_registration_success =
            result_->graph_load_result.scene_registered &&
            result_->graph_load_result.resource_dependency_count == request_.file_count &&
            result_->graph_load_result.asset_dependency_count == request_.file_count;
        if (!result_->resource_asset_registration_success) {
            result_->status = RuntimeAssetDataStatus::ResourceDependencyFailed;
            return result_->status;
        }

        result_->blocked_layer = RuntimeAssetPackagedRunBlockedLayer::ShaderProgram;
        status = DecodeRuntimeAssetPackagedShader(request_, result_);
        if (status != RuntimeAssetDataStatus::Success) {
            result_->status = status;
            return status;
        }

        result_->blocked_layer = RuntimeAssetPackagedRunBlockedLayer::RenderSceneRenderCoreRhi;
        RuntimeAssetVisualProofRequest proof_request =
            BuildRuntimeAssetPackagedVisualProofRequest(request_, result_->graph_load_result);
        status = BuildRuntimeAssetCookedVisualProofRoute(
            proof_request,
            &result_->visual_proof_result);
        result_->render_scene_render_core_rhi_success =
            status == RuntimeAssetDataStatus::Success &&
            result_->visual_proof_result.status == RuntimeAssetDataStatus::Success &&
            result_->visual_proof_result.first_missing_layer == RuntimeAssetVisualProofMissingLayer::None &&
            result_->visual_proof_result.render_scene_routed &&
            result_->visual_proof_result.render_core_rhi_capture_routed;
        if (!result_->render_scene_render_core_rhi_success) {
            result_->status = status;
            return status;
        }

        status = BuildRuntimeAssetPackagedGenericSubmission(request_, result_);
        if (status != RuntimeAssetDataStatus::Success) {
            result_->status = status;
            return status;
        }

        result_->status = RuntimeAssetDataStatus::Success;
        return result_->status;
    }

    const RuntimeAssetPackagedRunRequest &request_;
    RuntimeAssetPackagedRunResult *result_ = nullptr;
    bool executed_ = false;
};
}

RuntimeAssetDataStatus RunRuntimeAssetPackagedEntryPoint(
    const RuntimeAssetPackagedRunRequest &request,
    RuntimeAssetPackagedRunResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetPackagedRunResult result{};
    if (!RuntimeAssetPackagedRunRequestHasRequiredPointers(request) ||
        request.scene.path == nullptr ||
        request.capture_byte_budget_per_entity == 0U ||
        request.visual_frame_count == 0U) {
        result.status = RuntimeAssetDataStatus::InvalidArgument;
        *out_result = result;
        return result.status;
    }

    EngineKernel kernel{};
    RuntimeAssetPackagedRunModule module(request, &result);
    if (!kernel.RegisterModule(module)) {
        result.status = RuntimeAssetDataStatus::InvalidArgument;
        result.blocked_layer = RuntimeAssetPackagedRunBlockedLayer::RuntimeAppFrameLoop;
        *out_result = result;
        return result.status;
    }

    RuntimeApp runtime_app{};
    if (!runtime_app.Initialize(&kernel, request.runtime_app)) {
        result.status = RuntimeAssetDataStatus::InvalidArgument;
        result.blocked_layer = RuntimeAssetPackagedRunBlockedLayer::RuntimeAppFrameLoop;
        *out_result = result;
        return result.status;
    }

    std::vector<std::string> lifecycle_trace{};
    std::vector<RuntimeFramePhase> phase_trace{};
    const RuntimeAppRunResult run_result =
        runtime_app.RunFixedFrames(&lifecycle_trace, &phase_trace);
    result.runtime_app_result = run_result;
    result.runtime_app_completed_frame_count = run_result.completed_frame_count;
    result.runtime_app_frame_loop_success =
        run_result.succeeded &&
        run_result.completed_frame_count == request.runtime_app.frame_count &&
        run_result.last_frame_context.phase == RuntimeFramePhase::EndFrame;
    if (!result.runtime_app_frame_loop_success) {
        if (result.status == RuntimeAssetDataStatus::Success) {
            result.status = RuntimeAssetDataStatus::InvalidArgument;
            result.blocked_layer = RuntimeAssetPackagedRunBlockedLayer::RuntimeAppFrameLoop;
        }

        *out_result = result;
        return result.status;
    }

    if (result.status != RuntimeAssetDataStatus::Success) {
        *out_result = result;
        return result.status;
    }

    result.blocked_layer = RuntimeAssetPackagedRunBlockedLayer::None;
    result.packaged_runtime_entrypoint_available = true;
    *out_result = result;
    return result.status;
}

namespace {
bool RuntimeAssetPackageArtifactProductRunRequestHasRequiredInputs(
    const RuntimeAssetPackageArtifactProductRunRequest &request) {
    return request.mount_table != nullptr &&
        request.package_artifact_path.ByteLength() > 0U &&
        request.package.IsValid() &&
        request.scene_resource_type.IsValid() &&
        request.scene_logical_key.IsValid();
}

RuntimeAssetDataStatus RuntimeAssetPackageArtifactFailureStatus(
    const PackageArtifactResult &artifact_result) {
    if (artifact_result.status == PackageStatus::FileReadFailed) {
        return RuntimeAssetDataStatus::FileReadFailed;
    }

    if (artifact_result.status == PackageStatus::TypeMismatch) {
        return RuntimeAssetDataStatus::TypeMismatch;
    }

    return RuntimeAssetDataStatus::InvalidSchema;
}

RuntimeAssetPackageArtifactProductRunMissingLayer ProductRunLayerForPackagedRun(
    RuntimeAssetPackagedRunBlockedLayer layer) {
    if (layer == RuntimeAssetPackagedRunBlockedLayer::PackageLoadPlan) {
        return RuntimeAssetPackageArtifactProductRunMissingLayer::PackageLoadPlan;
    }

    return RuntimeAssetPackageArtifactProductRunMissingLayer::PackagedRuntimeEntryPoint;
}
}

RuntimeAssetDataStatus RunRuntimeAssetPackageArtifactProductCommand(
    const RuntimeAssetPackageArtifactProductRunRequest &request,
    RuntimeAssetPackageArtifactProductRunResult *out_result) {
    if (out_result == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    RuntimeAssetPackageArtifactProductRunResult result{};
    if (!RuntimeAssetPackageArtifactProductRunRequestHasRequiredInputs(request)) {
        result.status = RuntimeAssetDataStatus::InvalidArgument;
        result.missing_layer = RuntimeAssetPackageArtifactProductRunMissingLayer::Command;
        *out_result = result;
        return result.status;
    }

    PackageRegistry artifact_registry(request.package_registry);
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = request.mount_table;
    read_request.mount = request.mount;
    read_request.artifact_path = request.package_artifact_path;
    read_request.registry = &artifact_registry;
    read_request.registry_desc = request.package_registry;
    result.package_artifact = ReadPackageArtifact(read_request);
    result.file_status = result.package_artifact.file_status;
    result.package_status = result.package_artifact.status;
    result.package_artifact_read = result.package_artifact.read_artifact;
    result.package_registry_rebuilt = result.package_artifact.rebuilt_registry;
    if (result.package_artifact.status != PackageStatus::Success ||
        !result.package_artifact.read_artifact ||
        !result.package_artifact.rebuilt_registry) {
        result.status = RuntimeAssetPackageArtifactFailureStatus(result.package_artifact);
        result.missing_layer =
            result.package_artifact.status == PackageStatus::FileReadFailed
                ? RuntimeAssetPackageArtifactProductRunMissingLayer::FileVfs
                : RuntimeAssetPackageArtifactProductRunMissingLayer::PackageArtifact;
        *out_result = result;
        return result.status;
    }

    const PackageLoadPlanResult load_plan_result = artifact_registry.ResolveEntryByResourceKey(
        request.package,
        request.scene_resource_type,
        request.scene_logical_key);
    result.package_status = load_plan_result.status;
    result.package_load_plan_record_count = load_plan_result.plan.record_count;
    if (!load_plan_result.Succeeded()) {
        result.status =
            load_plan_result.status == PackageStatus::TypeMismatch
                ? RuntimeAssetDataStatus::TypeMismatch
                : RuntimeAssetDataStatus::MissingDependency;
        result.missing_layer = RuntimeAssetPackageArtifactProductRunMissingLayer::PackageLoadPlan;
        *out_result = result;
        return result.status;
    }

    result.package_load_plan_resolved = true;
    RuntimeAssetPackagedRunRequest packaged_request = request.packaged_run;
    packaged_request.package_load_plan = &load_plan_result.plan;
    result.packaged_run_executed = true;
    result.status = RunRuntimeAssetPackagedEntryPoint(packaged_request, &result.packaged_run);
    result.package_status = result.packaged_run.package_status;
    result.package_load_plan_record_count = result.packaged_run.package_load_plan_record_count;
    if (result.status != RuntimeAssetDataStatus::Success ||
        result.packaged_run.status != RuntimeAssetDataStatus::Success ||
        result.packaged_run.blocked_layer != RuntimeAssetPackagedRunBlockedLayer::None) {
        result.missing_layer = ProductRunLayerForPackagedRun(result.packaged_run.blocked_layer);
        *out_result = result;
        return result.status;
    }

    result.missing_layer = RuntimeAssetPackageArtifactProductRunMissingLayer::None;
    *out_result = result;
    return result.status;
}

}
