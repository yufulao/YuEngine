// 模块: Tests RenderScene
// 文件: Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "YuEngine/Asset/AssetDescriptor.h"
#include "YuEngine/Asset/AssetManager.h"
#include "YuEngine/Asset/AssetRecord.h"
#include "YuEngine/Asset/AssetStatus.h"
#include "YuEngine/File/FileWriteResult.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderScene/RenderSceneCameraFrameBinder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h"
#include "YuEngine/RenderScene/RenderSceneThreePrimitiveCaptureRoute.h"
#include "YuEngine/Resource/ResourceCachePayloadRequest.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRequest.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceLoadCommitRequest.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Resource/ResourceResidencyRequest.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridge.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeRequest.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeResult.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"

namespace {
using yuengine::animation::AnimationRuntimeStatus;
using yuengine::asset::AssetDescriptor;
using yuengine::asset::AssetHandle;
using yuengine::asset::AssetManager;
using yuengine::asset::AssetRecord;
using yuengine::asset::AssetRegistrationResult;
using yuengine::asset::AssetSnapshot;
using yuengine::asset::AssetStatus;
using yuengine::asset::AssetTypeId;
using yuengine::file::FileReadResult;
using yuengine::file::FileStatus;
using yuengine::file::FileWriteRequest;
using yuengine::file::MountId;
using yuengine::file::MountTable;
using yuengine::file::VirtualPath;
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
using yuengine::renderscene::RenderSceneRuntimeFrameDrawRecord;
using yuengine::renderscene::RenderSceneRuntimeFrameEntityRequest;
using yuengine::renderscene::RenderSceneRuntimeFrameRequest;
using yuengine::renderscene::RenderSceneRuntimeFrameResult;
using yuengine::renderscene::RenderSceneRuntimeFrameStatus;
using yuengine::renderscene::RenderSceneRuntimeMaterialBuilder;
using yuengine::renderscene::RenderSceneRuntimeMaterialRecord;
using yuengine::renderscene::RenderSceneRuntimeMaterialRequest;
using yuengine::renderscene::RenderSceneRuntimeMaterialStatus;
using yuengine::renderscene::RenderSceneRuntimeMaterialTextureSlot;
using yuengine::renderscene::RenderSceneStatus;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureRequest;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureResult;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureRoute;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureStatus;
using yuengine::renderscene::RenderSceneThreePrimitiveEntityRequest;
using yuengine::resource::ResourceCachePayloadRequest;
using yuengine::resource::ResourceCachePayloadStatus;
using yuengine::resource::ResourceDecodedPayloadRequest;
using yuengine::resource::ResourceDecodedPayloadStatus;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceLoadCommitRequest;
using yuengine::resource::ResourceLoadCommitStatus;
using yuengine::resource::ResourceLoadState;
using yuengine::resource::ResourceLogicalKey;
using yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceRegistrationResult;
using yuengine::resource::ResourceResidencyBudgetDesc;
using yuengine::resource::ResourceResidencyRequest;
using yuengine::resource::ResourceResidencyStatus;
using yuengine::resource::ResourceSnapshot;
using yuengine::resource::ResourceStatus;
using yuengine::resource::ResourceTypeId;
using yuengine::runtimeasset::HashRuntimeAssetDataBytes;
using yuengine::runtimeasset::BuildRuntimeAssetShaderProgramPipeline;
using yuengine::runtimeasset::DecodeRuntimeAssetShaderProgramData;
using yuengine::runtimeasset::LoadRuntimeAssetDataGraph;
using yuengine::runtimeasset::RuntimeAssetArtifactClass;
using yuengine::runtimeasset::RuntimeAssetDataStatus;
using yuengine::runtimeasset::RuntimeAssetFileDesc;
using yuengine::runtimeasset::RuntimeAssetFileKind;
using yuengine::runtimeasset::RuntimeAssetGraphLoadRequest;
using yuengine::runtimeasset::RuntimeAssetGraphLoadResult;
using yuengine::runtimeasset::RuntimeAssetLoadedFile;
using yuengine::runtimeasset::RuntimeAssetLoadedShaderProgramData;
using yuengine::runtimeasset::RuntimeAssetLoadTransactionPhase;
using yuengine::runtimeasset::RuntimeAssetSceneCameraRecord;
using yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using yuengine::runtimeasset::RuntimeAssetSceneLoaderOutput;
using yuengine::runtimeasset::RuntimeAssetSceneResourceRef;
using yuengine::runtimeasset::RuntimeAssetSceneTransformOutputRecord;
using yuengine::runtimeasset::RuntimeAssetShaderProgramPipelineRequest;
using yuengine::runtimeasset::RuntimeAssetShaderProgramPipelineResult;
using yuengine::runtimeasset::RuntimeAssetValidationResult;
using yuengine::runtimeasset::ValidateRuntimeAssetDataBytes;
using yuengine::streaming::ResourceDecodedTextureBridge;
using yuengine::streaming::ResourceDecodedTextureBridgeRequest;
using yuengine::streaming::ResourceDecodedTextureBridgeResult;
using yuengine::streaming::ResourceDecodedTextureBridgeStatus;
using yuengine::rhi::IRhiDevice;
using yuengine::rhi::NullRhiDevice;
using yuengine::rhi::RhiBufferDesc;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiColorTargetDesc;
using yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiFormat;
using yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiInputElementFormat;
using yuengine::rhi::RhiInputElementSemantic;
using yuengine::rhi::RhiInputLayoutDesc;
using yuengine::rhi::RhiPipelineDesc;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveTopology;
using yuengine::rhi::RhiSampledTextureBinding;
using yuengine::rhi::RhiSamplerBinding;
using yuengine::rhi::RhiSamplerDesc;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiShaderModuleDesc;
using yuengine::rhi::RhiShaderModuleHandle;
using yuengine::rhi::RhiShaderStage;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::RhiTextureDesc;
using yuengine::rhi::RhiTextureHandle;
using yuengine::rhi::RhiVertexBufferView;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldTransformState;

constexpr const char *TEST_GENERATOR =
    "RuntimeAssetData_GeneratorWritesDeterministicFilesAndHashes";
constexpr const char *TEST_UNSUPPORTED_VERSION =
    "RuntimeAssetData_FormatHeaderRejectsUnsupportedVersion";
constexpr const char *TEST_INVALID_BOUNDS =
    "RuntimeAssetData_ValidatorRejectsInvalidBoundsWithoutOutputs";
constexpr const char *TEST_TYPED_MESH_MATERIAL_TEXTURE =
    "RuntimeAssetData_MeshMaterialTextureTypedValidatorsAcceptStructuredMetadata";
constexpr const char *TEST_MATERIAL_TYPED_REFS =
    "RuntimeAssetData_MaterialValidatorRejectsMissingDuplicateAndTypeMismatchRefs";
constexpr const char *TEST_TEXTURE_TYPED_METADATA =
    "RuntimeAssetData_TextureValidatorRejectsInvalidFormatExtentPayload";
constexpr const char *TEST_SHADER_SCENE_ANIMATION_SCHEMA =
    "RuntimeAssetData_ShaderSceneAnimationRequireSourceSchema";
constexpr const char *TEST_INVALID_DEPENDENCY =
    "RuntimeAssetData_DependencyGraphRejectsMissingAndDuplicateRefs";
constexpr const char *TEST_SHADER_PROGRAM_PIPELINE_BRIDGE =
    "RuntimeAssetData_ShaderProgramBridgeCreatesRhiPipelineFromLoadedBytecode";
constexpr const char *TEST_SHADER_PROGRAM_PIPELINE_REJECTS =
    "RuntimeAssetData_ShaderProgramBridgeRejectsInvalidProgramDataWithoutRhiMutation";
constexpr const char *TEST_LOADER_FILE_RESOURCE =
    "RuntimeAssetData_LoaderUsesFileResourcePathNotInMemoryStructs";
constexpr const char *TEST_SCENE_REFERENCES =
    "RuntimeAssetData_SceneReferencesMeshMaterialTextureShader";
constexpr const char *TEST_SCENE_FAMILY_PATH_INDEPENDENT =
    "RuntimeAssetData_SceneFamilyDetectionIsPathIndependent";
constexpr const char *TEST_SOURCE_COOKED_METADATA =
    "RuntimeAssetData_SourceCookedParserReportsBoundedMetadata";
constexpr const char *TEST_SOURCE_COOKED_REJECTS =
    "RuntimeAssetData_SourceCookedParserRejectsInvalidTablesHashesAndDependencies";
constexpr const char *TEST_HEADER_REJECTS_PARTIAL_VERSION =
    "RuntimeAssetData_HeaderParserRejectsPartialVersionsAndNoise";
constexpr const char *TEST_LOADER_REJECTS_SCHEMA_KIND_SUFFIX =
    "RuntimeAssetData_LoaderRejectsSchemaKindAndMisleadingSuffixBeforeMutation";
constexpr const char *TEST_LOADER_TRANSACTION_INVALID_SCHEMA =
    "RuntimeAssetData_LoaderRejectsMissingSchemaBeforeMutation";
constexpr const char *TEST_LOADER_TRANSACTION_COMMIT_FAILURE =
    "RuntimeAssetData_LoaderCommitFailureReportsMutatedState";
constexpr const char *TEST_SHADER_PROGRAM_DEPENDENCIES =
    "RuntimeAssetData_ShaderProgramDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs";
constexpr const char *TEST_SCENE_CAMERA_ANIMATION_DEPENDENCIES =
    "RuntimeAssetData_SceneCameraAnimationDependencyValidatorRejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_ANIMATION_DEPENDENCIES =
    "RuntimeAssetData_AnimationDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs";
constexpr const char *TEST_LOADED_RENDER_RECORDS =
    "RuntimeAssetData_LoadCreatesRenderSceneRuntimeRecords";
constexpr const char *TEST_PRODUCTION_SCENE_LOADER_OUTPUT =
    "RuntimeAssetData_ProductionSceneLoaderOutputsDeterministicRecords";
constexpr const char *TEST_DISK_ANIMATION_SAMPLING =
    "RuntimeAssetData_DiskAnimationSamplingFeedsSceneTransforms";
constexpr const char *TEST_SCENE_LOADER_INVALID_ENTITY_NO_MUTATION =
    "RuntimeAssetData_SceneLoaderRejectsInvalidEntityWithoutOutputMutation";
constexpr const char *TEST_SCENE_LOADER_INVALID_KEYFRAME_NO_MUTATION =
    "RuntimeAssetData_SceneLoaderRejectsInvalidKeyframesWithoutOutputMutation";
constexpr const char *TEST_DECODED_PAYLOADS =
    "RuntimeAssetData_CookStoresDecodedPayloadsForMeshMaterialTexture";
constexpr const char *TEST_TEXTURE_MATERIAL_SLOT_BRIDGE =
    "RuntimeAssetData_DecodedTexturePayloadsDriveRhiMaterialSlots";
constexpr const char *TEST_TEXTURE_MATERIAL_SLOT_BRIDGE_FAILURES =
    "RuntimeAssetData_TextureMaterialSlotBridgeFailuresDoNotMutateRenderSceneOutputs";
constexpr const char *TEST_RUNTIME_DEPENDENCIES =
    "RuntimeAssetData_LoadRegistersResourceAndAssetDependencyEdges";
constexpr const char *TEST_LOAD_RENDER =
    "RuntimeAssetData_RenderClosedLoop_CapturesCubeCylinderConeThroughRhi";
constexpr const char *TEST_CPU_ORACLE =
    "RuntimeAssetData_CpuPpmOracleDoesNotBypassRhiRenderCore";
constexpr const char *TEST_NO_UPPER =
    "RuntimeAssetData_DoesNotDependOnEditorWebUiInputOrGdiViewer";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char *MOUNT_ID = "runtime";
constexpr const char *SCENE_PATH = "Scene/CanonicalScene.yuscene";
constexpr std::uint64_t FNV_OFFSET = 14695981039346656037ULL;
constexpr std::uint64_t FNV_PRIME = 1099511628211ULL;
constexpr std::uint32_t MATERIAL_ID = 4101U;
constexpr std::uint32_t FRAME_ID = 9001U;
constexpr std::uint64_t HALF_SECOND_NANOSECONDS = 500000000ULL;
constexpr std::uint32_t SEGMENT_COUNT = 8U;
constexpr std::uint32_t RUNTIME_TEXTURE_WIDTH = 2U;
constexpr std::uint32_t RUNTIME_TEXTURE_HEIGHT = 2U;
constexpr std::uint32_t RUNTIME_TEXTURE_BYTE_COUNT = 16U;
constexpr std::uint32_t RUNTIME_TEXTURE_SLOT_COUNT = 3U;
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
constexpr std::size_t FIXTURE_FILE_COUNT = 9U;
constexpr std::size_t CAPTURE_BYTES_PER_ENTITY = 64U;
constexpr std::size_t TOTAL_CAPTURE_BYTES = CAPTURE_BYTES_PER_ENTITY * 3U;

struct FixtureFile final {
    RuntimeAssetFileDesc desc{};
    const char *bytes = nullptr;
};

struct LoadedGraph final {
    std::array<RuntimeAssetLoadedFile, FIXTURE_FILE_COUNT> assets{};
    std::array<RuntimeAssetSceneResourceRef, FIXTURE_FILE_COUNT> scene_resource_refs{};
    std::array<RuntimeAssetSceneCameraRecord, 1U> scene_cameras{};
    std::array<RuntimeAssetSceneEntityRecord, 3U> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, 3U> scene_transforms{};
    RuntimeAssetLoadedFile scene_asset{};
    RuntimeAssetSceneLoaderOutput scene_output{};
    std::size_t file_read_count = 0U;
    std::size_t resource_payload_count = 0U;
    std::size_t decoded_payload_count = 0U;
    std::size_t dependency_count = 0U;
    std::size_t runtime_texture_upload_count = 0U;
    std::size_t material_texture_slot_count = 0U;
    RenderSceneRuntimeFrameResult frame_result{};
    RenderSceneThreePrimitiveCaptureResult capture_result{};
    bool scene_references_mesh_material_texture_shader = false;
    bool loader_used_file_mount = false;
    bool resource_payloads_stored = false;
    bool material_slots_from_decoded_payloads = false;
    bool render_capture_completed = false;
    bool cpu_oracle_allowed = false;
};

enum class RuntimeTextureMaterialSlotBridgeStatus {
    Success,
    InvalidArgument,
    InvalidLoadedTexture,
    TextureBridgeFailed,
    TextureReadyFailed,
    AssetQueryFailed,
    MaterialBuildFailed
};

struct RuntimeTextureMaterialSlotBridgeResult final {
    RuntimeTextureMaterialSlotBridgeStatus status = RuntimeTextureMaterialSlotBridgeStatus::InvalidArgument;
    ResourceDecodedTextureBridgeStatus texture_status = ResourceDecodedTextureBridgeStatus::Success;
    ResourceDecodedPayloadStatus decoded_payload_status = ResourceDecodedPayloadStatus::Success;
    RhiStatus rhi_status = RhiStatus::Success;
    std::size_t runtime_texture_upload_count = 0U;
    std::size_t material_texture_slot_count = 0U;
};

using TestFunction = int (*)();

class RuntimeAssetRhiDevice final : public IRhiDevice {
public:
    RhiStatus Initialize(const RhiDeviceDesc &desc) override {
        const RhiStatus status = device_.Initialize(desc);
        if (status != RhiStatus::Success) {
            return status;
        }

        RhiColorTargetDesc target_desc{};
        target_desc.format = RhiFormat::Rgba8Unorm;
        target_desc.extent = {2U, 2U};
        const RhiStatus target_status = device_.CreateColorTarget(target_desc, swapchain_target_);
        if (target_status != RhiStatus::Success) {
            return target_status;
        }

        swapchain_extent_ = target_desc.extent;
        swapchain_valid_ = true;
        return RhiStatus::Success;
    }

    RhiStatus CreateColorTarget(const RhiColorTargetDesc &desc, RhiTextureHandle &out_handle) override {
        return device_.CreateColorTarget(desc, out_handle);
    }

    RhiStatus GetSwapchainColorTarget(RhiTextureHandle &out_handle) const override {
        out_handle = RhiTextureHandle{};
        if (!swapchain_valid_) {
            return RhiStatus::UnsupportedBackend;
        }

        out_handle = swapchain_target_;
        return RhiStatus::Success;
    }

    RhiStatus ResizeSwapchain(
        const yuengine::rhi::RhiSwapchainResizeRequest &request,
        yuengine::rhi::RhiSwapchainResizeResult &out_result) override {
        out_result = yuengine::rhi::RhiSwapchainResizeResult{};
        RhiColorTargetDesc target_desc{};
        target_desc.format = RhiFormat::Rgba8Unorm;
        target_desc.extent = request.extent;
        const RhiStatus target_status = device_.CreateColorTarget(target_desc, swapchain_target_);
        out_result.status = target_status;
        if (target_status != RhiStatus::Success) {
            return target_status;
        }

        swapchain_extent_ = target_desc.extent;
        swapchain_valid_ = true;
        out_result.snapshot = Snapshot().swapchain;
        return RhiStatus::Success;
    }

    RhiStatus DestroyTarget(RhiTextureHandle handle) override {
        return device_.DestroyTarget(handle);
    }

    RhiStatus RecordClear(yuengine::rhi::RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) override {
        return device_.RecordClear(command_list, handle, color);
    }

    RhiStatus RecordBindPipeline(
        yuengine::rhi::RhiCommandList &command_list,
        RhiPipelineHandle handle) override {
        return device_.RecordBindPipeline(command_list, handle);
    }

    RhiStatus RecordBindVertexBuffer(
        yuengine::rhi::RhiCommandList &command_list,
        const RhiVertexBufferView &view) override {
        return device_.RecordBindVertexBuffer(command_list, view);
    }

    RhiStatus RecordBindIndexBuffer(
        yuengine::rhi::RhiCommandList &command_list,
        const RhiIndexBufferView &view) override {
        return device_.RecordBindIndexBuffer(command_list, view);
    }

    RhiStatus RecordBindSampledTexture(
        yuengine::rhi::RhiCommandList &command_list,
        const RhiSampledTextureBinding &binding) override {
        return device_.RecordBindSampledTexture(command_list, binding);
    }

    RhiStatus RecordBindSampler(
        yuengine::rhi::RhiCommandList &command_list,
        const RhiSamplerBinding &binding) override {
        return device_.RecordBindSampler(command_list, binding);
    }

    RhiStatus RecordBindBlendState(
        yuengine::rhi::RhiCommandList &command_list,
        const yuengine::rhi::RhiBlendStateDesc &desc) override {
        return device_.RecordBindBlendState(command_list, desc);
    }

    RhiStatus RecordDraw(
        yuengine::rhi::RhiCommandList &command_list,
        const yuengine::rhi::RhiDrawDesc &desc) override {
        return device_.RecordDraw(command_list, desc);
    }

    RhiStatus RecordDrawIndexed(
        yuengine::rhi::RhiCommandList &command_list,
        const yuengine::rhi::RhiDrawIndexedDesc &desc) override {
        return device_.RecordDrawIndexed(command_list, desc);
    }

    RhiStatus Submit(const yuengine::rhi::RhiCommandList &command_list) override {
        return device_.Submit(command_list);
    }

    RhiStatus Present() override {
        return device_.Present();
    }

    yuengine::rhi::RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination) override {
        return device_.CapturePresentedTarget(destination);
    }

    RhiStatus CreateBuffer(
        const RhiBufferDesc &desc,
        std::span<const std::uint8_t> initial_bytes,
        RhiBufferHandle &out_handle) override {
        return device_.CreateBuffer(desc, initial_bytes, out_handle);
    }

    RhiStatus UpdateBuffer(
        RhiBufferHandle handle,
        std::span<const std::uint8_t> bytes,
        yuengine::rhi::RhiFenceHandle &out_fence) override {
        return device_.UpdateBuffer(handle, bytes, out_fence);
    }

    RhiStatus DestroyBuffer(RhiBufferHandle handle) override {
        return device_.DestroyBuffer(handle);
    }

    RhiStatus CreateTexture(
        const RhiTextureDesc &desc,
        std::span<const std::uint8_t> initial_bytes,
        RhiTextureHandle &out_handle) override {
        return device_.CreateTexture(desc, initial_bytes, out_handle);
    }

    RhiStatus UpdateTexture(
        RhiTextureHandle handle,
        std::span<const std::uint8_t> bytes,
        yuengine::rhi::RhiFenceHandle &out_fence) override {
        return device_.UpdateTexture(handle, bytes, out_fence);
    }

    RhiStatus DestroyTexture(RhiTextureHandle handle) override {
        return device_.DestroyTexture(handle);
    }

    RhiStatus CreateSampler(const RhiSamplerDesc &desc, RhiSamplerHandle &out_handle) override {
        return device_.CreateSampler(desc, out_handle);
    }

    RhiStatus DestroySampler(RhiSamplerHandle handle) override {
        return device_.DestroySampler(handle);
    }

    RhiStatus CreateShaderModule(const RhiShaderModuleDesc &desc, RhiShaderModuleHandle &out_handle) override {
        return device_.CreateShaderModule(desc, out_handle);
    }

    RhiStatus DestroyShaderModule(RhiShaderModuleHandle handle) override {
        return device_.DestroyShaderModule(handle);
    }

    RhiStatus CreatePipeline(const RhiPipelineDesc &desc, RhiPipelineHandle &out_handle) override {
        return device_.CreatePipeline(desc, out_handle);
    }

    RhiStatus DestroyPipeline(RhiPipelineHandle handle) override {
        return device_.DestroyPipeline(handle);
    }

    RhiStatus RequestPrimitiveRetirement(
        const yuengine::rhi::RhiPrimitiveRetirementRequest &request,
        yuengine::rhi::RhiPrimitiveRetirementRecord &out_record) override {
        return device_.RequestPrimitiveRetirement(request, out_record);
    }

    RhiStatus QueryPrimitiveRetirement(
        std::uint64_t retirement_id,
        yuengine::rhi::RhiPrimitiveRetirementRecord &out_record) const override {
        return device_.QueryPrimitiveRetirement(retirement_id, out_record);
    }

    RhiStatus DrainPrimitiveRetirements(
        const yuengine::rhi::RhiPrimitiveRetirementDrainRequest &request,
        yuengine::rhi::RhiPrimitiveRetirementDrainResult &out_result) override {
        return device_.DrainPrimitiveRetirements(request, out_result);
    }

    yuengine::rhi::RhiCapabilities Capabilities() const override {
        return device_.Capabilities();
    }

    yuengine::rhi::RhiDeviceSnapshot Snapshot() const override {
        yuengine::rhi::RhiDeviceSnapshot snapshot = device_.Snapshot();
        snapshot.swapchain.valid = swapchain_valid_;
        snapshot.swapchain.extent = swapchain_extent_;
        snapshot.swapchain.color_target = swapchain_target_;
        return snapshot;
    }

private:
    NullRhiDevice device_{};
    RhiTextureHandle swapchain_target_{};
    yuengine::rhi::RhiExtent2D swapchain_extent_{};
    bool swapchain_valid_ = false;
};

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool FailStep(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return false;
}

const char *StatusName(RuntimeAssetDataStatus status) {
    switch (status) {
        case RuntimeAssetDataStatus::Success:
            return "Success";
        case RuntimeAssetDataStatus::InvalidArgument:
            return "InvalidArgument";
        case RuntimeAssetDataStatus::InvalidHeader:
            return "InvalidHeader";
        case RuntimeAssetDataStatus::UnsupportedVersion:
            return "UnsupportedVersion";
        case RuntimeAssetDataStatus::InvalidKind:
            return "InvalidKind";
        case RuntimeAssetDataStatus::InvalidSchema:
            return "InvalidSchema";
        case RuntimeAssetDataStatus::InvalidCount:
            return "InvalidCount";
        case RuntimeAssetDataStatus::InvalidSize:
            return "InvalidSize";
        case RuntimeAssetDataStatus::InvalidAlignment:
            return "InvalidAlignment";
        case RuntimeAssetDataStatus::InvalidBounds:
            return "InvalidBounds";
        case RuntimeAssetDataStatus::InvalidDependency:
            return "InvalidDependency";
        case RuntimeAssetDataStatus::MissingDependency:
            return "MissingDependency";
        case RuntimeAssetDataStatus::DuplicateDependency:
            return "DuplicateDependency";
        case RuntimeAssetDataStatus::TypeMismatch:
            return "TypeMismatch";
        case RuntimeAssetDataStatus::HashMismatch:
            return "HashMismatch";
        case RuntimeAssetDataStatus::UnsupportedFieldValue:
            return "UnsupportedFieldValue";
        case RuntimeAssetDataStatus::CapacityExceeded:
            return "CapacityExceeded";
        case RuntimeAssetDataStatus::BudgetExceeded:
            return "BudgetExceeded";
        case RuntimeAssetDataStatus::FileReadFailed:
            return "FileReadFailed";
        case RuntimeAssetDataStatus::ResourceRegistrationFailed:
            return "ResourceRegistrationFailed";
        case RuntimeAssetDataStatus::ResourceLoadCommitFailed:
            return "ResourceLoadCommitFailed";
        case RuntimeAssetDataStatus::ResourceResidencyFailed:
            return "ResourceResidencyFailed";
        case RuntimeAssetDataStatus::CachePayloadStoreFailed:
            return "CachePayloadStoreFailed";
        case RuntimeAssetDataStatus::DecodePlanFailed:
            return "DecodePlanFailed";
        case RuntimeAssetDataStatus::DecodeResultFailed:
            return "DecodeResultFailed";
        case RuntimeAssetDataStatus::DecodedPayloadStoreFailed:
            return "DecodedPayloadStoreFailed";
        case RuntimeAssetDataStatus::AssetRegistrationFailed:
            return "AssetRegistrationFailed";
        case RuntimeAssetDataStatus::ResourceDependencyFailed:
            return "ResourceDependencyFailed";
        case RuntimeAssetDataStatus::AssetDependencyFailed:
            return "AssetDependencyFailed";
        case RuntimeAssetDataStatus::InvalidInputLayout:
            return "InvalidInputLayout";
        case RuntimeAssetDataStatus::RhiShaderModuleFailed:
            return "RhiShaderModuleFailed";
        case RuntimeAssetDataStatus::RhiPipelineFailed:
            return "RhiPipelineFailed";
        default:
            break;
    }

    return "Unknown";
}

std::filesystem::path TestRoot(std::string_view test_name) {
    std::filesystem::path root = std::filesystem::temp_directory_path();
    root /= "YuEngineRuntimeAssetDataTests";
    root /= std::string(test_name);
    return root;
}

std::array<FixtureFile, FIXTURE_FILE_COUNT> CanonicalFiles() {
    return std::array<FixtureFile, FIXTURE_FILE_COUNT>{
        FixtureFile{
            RuntimeAssetFileDesc{
                "Mesh/Cube.yumesh",
                RuntimeAssetFileKind::Mesh,
                ResourceTypeId{RESOURCE_TYPE_MESH},
                AssetTypeId{ASSET_TYPE_MESH},
                1001U,
                yuengine::resource::ResourceDecodePlanAssetClass::Mesh,
                yuengine::resource::ResourceDecodeResultClass::Mesh,
                96U},
            "YUASSET MESH 1\nschema=rav0-source\nid=cube_mesh\nkind=cube\nvertices=24\nindices=36\nbounds=-1,-1,-1,1,1,1\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Mesh/Cylinder.yumesh",
                RuntimeAssetFileKind::Mesh,
                ResourceTypeId{RESOURCE_TYPE_MESH},
                AssetTypeId{ASSET_TYPE_MESH},
                1002U,
                yuengine::resource::ResourceDecodePlanAssetClass::Mesh,
                yuengine::resource::ResourceDecodeResultClass::Mesh,
                96U},
            "YUASSET MESH 1\nschema=rav0-source\nid=cylinder_mesh\nkind=cylinder\nvertices=18\nindices=96\nbounds=-1,-1,-1,1,1,1\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Mesh/Cone.yumesh",
                RuntimeAssetFileKind::Mesh,
                ResourceTypeId{RESOURCE_TYPE_MESH},
                AssetTypeId{ASSET_TYPE_MESH},
                1003U,
                yuengine::resource::ResourceDecodePlanAssetClass::Mesh,
                yuengine::resource::ResourceDecodeResultClass::Mesh,
                96U},
            "YUASSET MESH 1\nschema=rav0-source\nid=cone_mesh\nkind=cone\nvertices=10\nindices=48\nbounds=-1,-1,-1,1,1,1\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Material/Shared.yumat",
                RuntimeAssetFileKind::Material,
                ResourceTypeId{RESOURCE_TYPE_MATERIAL},
                AssetTypeId{ASSET_TYPE_MATERIAL},
                2001U,
                yuengine::resource::ResourceDecodePlanAssetClass::Material,
                yuengine::resource::ResourceDecodeResultClass::Material,
                128U},
            "YUASSET MATERIAL 1\nschema=rav0-source\nid=shared_material\nshader=Shader/RuntimeProgram.yuprogram\ntexture0=Texture/Albedo.yutex\ntexture1=Texture/Normal.yutex\ntexture2=Texture/Mask.yutex\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Texture/Albedo.yutex",
                RuntimeAssetFileKind::Texture,
                ResourceTypeId{RESOURCE_TYPE_TEXTURE},
                AssetTypeId{ASSET_TYPE_TEXTURE},
                3001U,
                yuengine::resource::ResourceDecodePlanAssetClass::Texture,
                yuengine::resource::ResourceDecodeResultClass::Texture,
                16U},
            "YUASSET TEXTURE 1\nschema=rav0-source\nid=albedo\nformat=rgba8\nextent=2x2\npayload=checker\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Texture/Normal.yutex",
                RuntimeAssetFileKind::Texture,
                ResourceTypeId{RESOURCE_TYPE_TEXTURE},
                AssetTypeId{ASSET_TYPE_TEXTURE},
                3002U,
                yuengine::resource::ResourceDecodePlanAssetClass::Texture,
                yuengine::resource::ResourceDecodeResultClass::Texture,
                16U},
            "YUASSET TEXTURE 1\nschema=rav0-source\nid=normal\nformat=rgba8\nextent=2x2\npayload=normal\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Texture/Mask.yutex",
                RuntimeAssetFileKind::Texture,
                ResourceTypeId{RESOURCE_TYPE_TEXTURE},
                AssetTypeId{ASSET_TYPE_TEXTURE},
                3003U,
                yuengine::resource::ResourceDecodePlanAssetClass::Texture,
                yuengine::resource::ResourceDecodeResultClass::Texture,
                16U},
            "YUASSET TEXTURE 1\nschema=rav0-source\nid=mask\nformat=rgba8\nextent=2x2\npayload=mask\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Shader/RuntimeProgram.yuprogram",
                RuntimeAssetFileKind::Shader,
                ResourceTypeId{RESOURCE_TYPE_SHADER},
                AssetTypeId{ASSET_TYPE_SHADER},
                4001U},
            "YUASSET SHADER 1\nschema=rav0-source\nid=runtime_program\nstage_vs=bytecode:runtime_program_vs\nstage_ps=bytecode:runtime_program_ps\ninput=layout:position,color\ntextures=3\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Animation/Spin.yuanim",
                RuntimeAssetFileKind::Animation,
                ResourceTypeId{RESOURCE_TYPE_ANIMATION},
                AssetTypeId{ASSET_TYPE_ANIMATION},
                5001U},
            "YUASSET ANIMATION 1\nschema=rav0-source\nid=spin\nclip=1\nduration=1\ntarget=scene_entity:101\ntrack=transform:rotation_y\nkey0=0:0\nkey1=1:1\ntracks=1\nsample_rate=30\n"}};
}

std::string SceneBytes() {
    return std::string(
        "YUASSET SCENE 1\n"
        "schema=rav0-source\n"
        "id=scene\n"
        "m0=Mesh/C.yumesh\n"
        "m1=Mesh/Y.yumesh\n"
        "m2=Mesh/N.yumesh\n"
        "mat=Material/M.yumat\n"
        "t0=Texture/A.yutex\n"
        "prog=Shader/P.yuprogram\n"
        "anim=Animation/S.yuanim\n"
        "cam=camera:orbit\n"
        "e0=101:-2,0,0\n"
        "e1=102:0,0,0\n"
        "e2=103:2,0,0\n");
}

std::string AlternateRuntimeFamilySceneBytes() {
    return std::string(
        "YUASSET SCENE 1\n"
        "schema=rav0-source\n"
        "id=alt_scene\n"
        "m0=Mesh/C.alt\n"
        "m1=Mesh/Y.alt\n"
        "m2=Mesh/N.alt\n"
        "mat=Material/M.alt\n"
        "t0=Texture/T.alt\n"
        "prog=Shader/P.alt\n"
        "anim=Animation/A.alt\n"
        "cam=camera:orbit\n"
        "e0=101:-2,0,0\n"
        "e1=102:0,0,0\n"
        "e2=103:2,0,0\n");
}

std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> AlternateRuntimeFamilyFileDescs() {
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        descs[index] = files[index].desc;
    }

    descs[0U].path = "Mesh/C.alt";
    descs[1U].path = "Mesh/Y.alt";
    descs[2U].path = "Mesh/N.alt";
    descs[3U].path = "Material/M.alt";
    descs[4U].path = "Texture/T.alt";
    descs[5U].path = "Texture/N.alt";
    descs[6U].path = "Texture/K.alt";
    descs[7U].path = "Shader/P.alt";
    descs[8U].path = "Animation/A.alt";
    return descs;
}

std::vector<std::uint8_t> BytesFromString(const std::string &text) {
    return std::vector<std::uint8_t>(text.begin(), text.end());
}

std::uint64_t HashText(std::string_view text) {
    std::uint64_t hash = FNV_OFFSET;
    for (const char character : text) {
        hash ^= static_cast<std::uint64_t>(static_cast<std::uint8_t>(character));
        hash *= FNV_PRIME;
    }

    return hash;
}

std::string SourceMeshText(std::string_view header_line) {
    std::string text(header_line);
    text +=
        "\n"
        "schema=rav0-source\n"
        "id=cube_mesh\n"
        "kind=cube\n"
        "vertices=24\n"
        "indices=36\n"
        "bounds=-1,-1,-1,1,1,1\n";
    return text;
}

std::string CookedTextureTextWithHeader(
    std::string_view header_line,
    std::string_view payload,
    std::uint32_t dependency_table_count,
    std::uint32_t record_table_count,
    std::uint32_t record_byte_count,
    std::uint32_t payload_alignment,
    std::uint64_t payload_hash) {
    std::string text(header_line);
    text +=
        "\n"
        "schema=rav1-cooked\n"
        "id=albedo_cooked\n"
        "kind=texture\n";
    text += "sourceHash=" + std::to_string(HashText("albedo_source")) + "\n";
    text += "payloadHash=" + std::to_string(payload_hash) + "\n";
    text += "dependencyTable=" + std::to_string(dependency_table_count) + "\n";
    text += "recordTable=" + std::to_string(record_table_count) + "\n";
    text += "recordBytes=" + std::to_string(record_byte_count) + "\n";
    text += "payloadBytes=" + std::to_string(payload.size()) + "\n";
    text += "payloadAlign=" + std::to_string(payload_alignment) + "\n";
    text +=
        "format=rgba8\n"
        "extent=2x2\n";
    if (dependency_table_count > 0U) {
        text += "dep0=material:shared_material:" + std::to_string(HashText("shared_material")) + "\n";
    }

    text += "payload=";
    text += payload;
    text += "\n";
    return text;
}

std::string CookedTextureText(
    std::string_view payload,
    std::uint32_t dependency_table_count,
    std::uint32_t record_table_count,
    std::uint32_t record_byte_count,
    std::uint32_t payload_alignment,
    std::uint64_t payload_hash) {
    return CookedTextureTextWithHeader(
        "YUCOOKED TEXTURE 1",
        payload,
        dependency_table_count,
        record_table_count,
        record_byte_count,
        payload_alignment,
        payload_hash);
}

std::string ValidCookedTextureText() {
    constexpr std::string_view payload = "checker";
    return CookedTextureText(payload, 1U, 1U, 64U, 4U, HashText(payload));
}

bool Contains(std::string_view text, std::string_view token) {
    return text.find(token) != std::string_view::npos;
}

bool Approx(float actual, float expected) {
    return std::fabs(actual - expected) < 0.001F;
}

bool WriteBytes(MountTable &table, const char *path, const std::vector<std::uint8_t> &bytes) {
    FileWriteRequest request{};
    request.mount = MountId(MOUNT_ID);
    request.path = VirtualPath(path);
    request.bytes = bytes.data();
    request.byte_count = bytes.size();
    return table.Write(request).Succeeded();
}

bool WriteCanonicalFixture(MountTable &table) {
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    for (const FixtureFile &file : files) {
        const std::string text(file.bytes);
        const std::vector<std::uint8_t> bytes = BytesFromString(text);
        if (!WriteBytes(table, file.desc.path, bytes)) {
            return false;
        }
    }

    const std::vector<std::uint8_t> scene_bytes = BytesFromString(SceneBytes());
    return WriteBytes(table, SCENE_PATH, scene_bytes);
}

bool WriteAlternateRuntimeFamilyFixture(MountTable &table, const char *scene_path) {
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs = AlternateRuntimeFamilyFileDescs();
    for (std::size_t index = 0U; index < files.size(); ++index) {
        const std::string text(files[index].bytes);
        const std::vector<std::uint8_t> bytes = BytesFromString(text);
        if (!WriteBytes(table, descs[index].path, bytes)) {
            return false;
        }
    }

    const std::vector<std::uint8_t> scene_bytes = BytesFromString(AlternateRuntimeFamilySceneBytes());
    return WriteBytes(table, scene_path, scene_bytes);
}

bool CreateMountedTable(const std::filesystem::path &root, MountTable *out_table) {
    if (out_table == nullptr) {
        return FailStep("null mount table output");
    }

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    MountTable table;
    const FileStatus mount_status = table.RegisterLooseMount(MountId(MOUNT_ID), root);
    if (mount_status != FileStatus::Success) {
        return FailStep("register loose mount failed");
    }

    *out_table = table;
    return true;
}

bool ReadFile(MountTable &table, const char *path, std::vector<std::uint8_t> *out_bytes) {
    if (out_bytes == nullptr) {
        return FailStep("null file output");
    }

    const FileReadResult read_result = table.Read({MountId(MOUNT_ID), VirtualPath(path)});
    if (!read_result.Succeeded()) {
        return FailStep("read file failed");
    }

    *out_bytes = read_result.bytes;
    return true;
}

bool SceneReferencesRequiredAssets(const std::vector<std::uint8_t> &scene_bytes) {
    const std::string scene(scene_bytes.begin(), scene_bytes.end());
    if (!Contains(scene, "m0=Mesh/C.yumesh")) {
        return FailStep("missing cube mesh dependency");
    }

    if (!Contains(scene, "m1=Mesh/Y.yumesh")) {
        return FailStep("missing cylinder mesh dependency");
    }

    if (!Contains(scene, "m2=Mesh/N.yumesh")) {
        return FailStep("missing cone mesh dependency");
    }

    if (!Contains(scene, "mat=Material/M.yumat")) {
        return FailStep("missing material dependency");
    }

    if (!Contains(scene, "t0=Texture/A.yutex")) {
        return FailStep("missing texture dependency");
    }

    if (!Contains(scene, "prog=Shader/P.yuprogram")) {
        return FailStep("missing shader dependency");
    }

    if (!Contains(scene, "cam=camera:orbit")) {
        return FailStep("missing camera dependency");
    }

    if (scene_bytes.size() > yuengine::resource::MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD) {
        return FailStep("scene source payload exceeded cache record capacity");
    }

    if (!Contains(scene, "e0=101:-2,0,0") ||
        !Contains(scene, "e1=102:0,0,0") ||
        !Contains(scene, "e2=103:2,0,0")) {
        return FailStep("missing scene entity transform records");
    }

    return Contains(scene, "anim=Animation/S.yuanim");
}

bool CreateShaderModule(IRhiDevice &device, RhiShaderStage stage, RhiShaderModuleHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    const std::array<std::uint8_t, 4U> bytes{1U, 3U, 5U, 7U};
    const RhiShaderModuleDesc desc{stage, std::span<const std::uint8_t>(bytes.data(), bytes.size())};
    return device.CreateShaderModule(desc, *out_handle) == RhiStatus::Success;
}

RhiInputLayoutDesc RuntimeInputLayout() {
    RhiInputLayoutDesc desc{};
    desc.elements[0U].semantic = RhiInputElementSemantic::Position;
    desc.elements[0U].format = RhiInputElementFormat::Float32x2;
    desc.elements[0U].offset_bytes = 0U;
    desc.element_count = 1U;
    desc.stride_bytes = sizeof(float) * 2U;
    return desc;
}

bool CreatePipeline(IRhiDevice &device, RhiPipelineHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    RhiShaderModuleHandle vertex_shader{};
    if (!CreateShaderModule(device, RhiShaderStage::Vertex, &vertex_shader)) {
        return false;
    }

    RhiShaderModuleHandle pixel_shader{};
    if (!CreateShaderModule(device, RhiShaderStage::Pixel, &pixel_shader)) {
        return false;
    }

    RhiPipelineDesc desc{};
    desc.vertex_shader = vertex_shader;
    desc.pixel_shader = pixel_shader;
    desc.input_layout = RuntimeInputLayout();
    return device.CreatePipeline(desc, *out_handle) == RhiStatus::Success;
}

bool CreateBuffer(
    IRhiDevice &device,
    RhiBufferUsage usage,
    std::size_t byte_count,
    RhiBufferHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    RhiBufferDesc desc{};
    desc.usage = usage;
    desc.size_bytes = byte_count;
    const std::span<const std::uint8_t> empty_bytes{};
    return device.CreateBuffer(desc, empty_bytes, *out_handle) == RhiStatus::Success;
}

bool CreateSampler(IRhiDevice &device, RhiSamplerHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    RhiSamplerDesc desc{};
    desc.linear_filter = false;
    desc.clamp_to_edge = true;
    return device.CreateSampler(desc, *out_handle) == RhiStatus::Success;
}

RhiTextureDesc RuntimeTextureDesc() {
    RhiTextureDesc desc{};
    desc.format = RhiFormat::Rgba8Unorm;
    desc.extent = {RUNTIME_TEXTURE_WIDTH, RUNTIME_TEXTURE_HEIGHT};
    return desc;
}

bool IsLoadedRuntimeTexture(const RuntimeAssetLoadedFile &file) {
    if (file.kind != RuntimeAssetFileKind::Texture) {
        return false;
    }

    if (!file.resource.IsValid() || !file.asset.IsValid()) {
        return false;
    }

    if (file.resource_type.value != RESOURCE_TYPE_TEXTURE || file.asset_type.value != ASSET_TYPE_TEXTURE) {
        return false;
    }

    if (!file.decode_plan_created || !file.decode_result_committed || !file.decoded_payload_stored) {
        return false;
    }

    if (file.decode_plan_payload_id == 0U || file.decode_plan_id == 0U ||
        file.decode_result_id == 0U || file.decoded_payload_id == 0U) {
        return false;
    }

    if (file.decode_asset_class != yuengine::resource::ResourceDecodePlanAssetClass::Texture) {
        return false;
    }

    if (file.decode_result_class != yuengine::resource::ResourceDecodeResultClass::Texture) {
        return false;
    }

    return file.decoded_byte_count == RUNTIME_TEXTURE_BYTE_COUNT;
}

ResourceDecodedPayloadRequest DecodedPayloadRequestFor(const RuntimeAssetLoadedFile &texture_file) {
    ResourceDecodedPayloadRequest request{};
    request.resource = texture_file.resource;
    request.expected_type = texture_file.resource_type;
    request.payload_id = texture_file.decode_plan_payload_id;
    request.decode_plan_id = texture_file.decode_plan_id;
    request.decode_result_id = texture_file.decode_result_id;
    request.decoded_payload_id = texture_file.decoded_payload_id;
    request.asset_class = texture_file.decode_asset_class;
    request.result_class = texture_file.decode_result_class;
    request.decoded_byte_count = texture_file.decoded_byte_count;
    return request;
}

RhiVertexBufferView VertexView(RhiBufferHandle handle, std::size_t vertex_count) {
    RhiVertexBufferView view{};
    view.buffer = handle;
    view.offset_bytes = 0U;
    view.stride_bytes = sizeof(float) * 2U;
    view.size_bytes = view.stride_bytes * vertex_count;
    return view;
}

RhiIndexBufferView IndexView(RhiBufferHandle handle, std::size_t index_count) {
    RhiIndexBufferView view{};
    view.buffer = handle;
    view.offset_bytes = 0U;
    view.size_bytes = sizeof(std::uint16_t) * index_count;
    view.format = RhiIndexFormat::Uint16;
    return view;
}

bool BuildGeometry(
    RenderScenePrimitiveGeometryKind kind,
    AssetHandle asset,
    std::uint32_t draw_id,
    RhiVertexBufferView vertex_view,
    RhiIndexBufferView index_view,
    RenderScenePrimitiveGeometryRecord *out_record) {
    if (out_record == nullptr) {
        return false;
    }

    RenderScenePrimitiveGeometryRequest request{};
    request.geometry_asset = asset;
    request.kind = kind;
    request.segment_count = SEGMENT_COUNT;
    request.draw_id = draw_id;
    request.pass_id = draw_id + 100U;
    request.material_id = MATERIAL_ID;
    request.vertex_buffer = vertex_view;
    request.index_buffer = index_view;

    RenderScenePrimitiveGeometryBuilder builder;
    const RenderScenePrimitiveGeometryStatus status = builder.Build(request, out_record);
    return status == RenderScenePrimitiveGeometryStatus::Success;
}

RuntimeTextureMaterialSlotBridgeResult BuildMaterial(
    IRhiDevice &device,
    ResourceRegistry &registry,
    AssetManager &manager,
    AssetHandle material_asset,
    std::span<const RuntimeAssetLoadedFile> texture_assets,
    RhiTextureDesc texture_desc,
    RenderSceneRuntimeMaterialRecord *out_material) {
    RuntimeTextureMaterialSlotBridgeResult result{};
    if (out_material == nullptr) {
        return result;
    }

    if (texture_assets.size() < RUNTIME_TEXTURE_SLOT_COUNT) {
        return result;
    }

    RhiPipelineHandle pipeline{};
    if (!CreatePipeline(device, &pipeline)) {
        result.status = RuntimeTextureMaterialSlotBridgeStatus::InvalidArgument;
        return result;
    }

    ResourceDecodedTextureBridge texture_bridge;
    std::array<RenderSceneRuntimeMaterialTextureSlot, RUNTIME_TEXTURE_SLOT_COUNT> slots{};
    for (std::size_t index = 0U; index < slots.size(); ++index) {
        const RuntimeAssetLoadedFile &texture_asset = texture_assets[index];
        if (!IsLoadedRuntimeTexture(texture_asset)) {
            result.status = RuntimeTextureMaterialSlotBridgeStatus::InvalidLoadedTexture;
            return result;
        }

        std::array<std::uint8_t, RUNTIME_TEXTURE_BYTE_COUNT> scratch_bytes{};
        RhiTextureHandle texture{};
        ResourceDecodedTextureBridgeRequest bridge_request{};
        bridge_request.resource_registry = &registry;
        bridge_request.rhi_device = &device;
        bridge_request.decoded_payload = DecodedPayloadRequestFor(texture_asset);
        bridge_request.scratch_bytes = std::span<std::uint8_t>(scratch_bytes.data(), scratch_bytes.size());
        bridge_request.texture_desc = texture_desc;
        bridge_request.output_texture_handle = &texture;
        bridge_request.staging_request_id = texture_asset.stable_id + 300000U;
        bridge_request.upload_id = texture_asset.stable_id + 400000U;
        bridge_request.sampled_texture_slot = static_cast<std::uint32_t>(index);
        const ResourceDecodedTextureBridgeResult texture_result = texture_bridge.UploadTexture(bridge_request);
        if (texture_result.status != ResourceDecodedTextureBridgeStatus::Success) {
            result.status = RuntimeTextureMaterialSlotBridgeStatus::TextureBridgeFailed;
            result.texture_status = texture_result.status;
            result.decoded_payload_status = texture_result.decoded_payload_status;
            result.rhi_status = texture_result.rhi_status;
            return result;
        }

        if (manager.MarkTextureReady(texture_asset.asset, texture_result) != AssetStatus::Success) {
            result.status = RuntimeTextureMaterialSlotBridgeStatus::TextureReadyFailed;
            result.texture_status = texture_result.status;
            result.decoded_payload_status = texture_result.decoded_payload_status;
            result.rhi_status = texture_result.rhi_status;
            return result;
        }

        AssetRecord texture_record{};
        if (manager.QueryAsset(texture_asset.asset, &texture_record) != AssetStatus::Success) {
            result.status = RuntimeTextureMaterialSlotBridgeStatus::AssetQueryFailed;
            return result;
        }

        if (!texture_record.texture_ready.is_ready) {
            result.status = RuntimeTextureMaterialSlotBridgeStatus::TextureReadyFailed;
            return result;
        }

        RhiSamplerHandle sampler{};
        if (!CreateSampler(device, &sampler)) {
            result.status = RuntimeTextureMaterialSlotBridgeStatus::InvalidArgument;
            return result;
        }

        slots[index].slot = static_cast<std::uint32_t>(index);
        slots[index].texture_asset = texture_asset.asset;
        slots[index].sampled_texture = texture_record.texture_ready.sampled_texture;
        slots[index].sampler = RhiSamplerBinding{sampler, static_cast<std::uint32_t>(index)};
        ++result.runtime_texture_upload_count;
    }

    RenderSceneRuntimeMaterialRequest request{};
    request.material_asset = material_asset;
    request.material_id = MATERIAL_ID;
    request.pipeline = pipeline;
    request.texture_slots = std::span<const RenderSceneRuntimeMaterialTextureSlot>(slots.data(), slots.size());

    RenderSceneRuntimeMaterialBuilder builder;
    const RenderSceneRuntimeMaterialStatus status = builder.Build(request, out_material);
    if (status != RenderSceneRuntimeMaterialStatus::Success) {
        result.status = RuntimeTextureMaterialSlotBridgeStatus::MaterialBuildFailed;
        return result;
    }

    result.material_texture_slot_count = out_material->texture_slot_count;
    result.status = RuntimeTextureMaterialSlotBridgeStatus::Success;
    return result;
}

bool BuildCamera(IRhiDevice &device, RenderSceneCameraBindingResult *out_camera) {
    if (out_camera == nullptr) {
        return false;
    }

    RhiTextureHandle target{};
    RhiColorTargetDesc target_desc{};
    target_desc.format = RhiFormat::Rgba8Unorm;
    target_desc.extent = {2U, 2U};
    if (device.CreateColorTarget(target_desc, target) != RhiStatus::Success) {
        return false;
    }

    RenderSceneRuntimeCameraRecord camera{};
    camera.camera_id = 1U;
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
    RenderSceneCameraBindingRequest request{};
    request.frame_id = FRAME_ID;
    request.active_camera_id = camera.camera_id;
    request.cameras = std::span<const RenderSceneRuntimeCameraRecord>(cameras.data(), cameras.size());
    request.capture_byte_budget = TOTAL_CAPTURE_BYTES;
    request.capture_requested = true;

    RenderSceneCameraFrameBinder binder;
    const RenderSceneStatus status = binder.BuildActiveCameraFrame(request, out_camera);
    return status == RenderSceneStatus::Success;
}

WorldTransformState Transform(float x, float y, float z) {
    WorldTransformState transform{};
    transform.translation_x = x;
    transform.translation_y = y;
    transform.translation_z = z;
    return transform;
}

bool ExecuteLoadedRenderPath(
    IRhiDevice &device,
    ResourceRegistry &registry,
    AssetManager &manager,
    const LoadedGraph &graph,
    RenderSceneRuntimeFrameResult *out_frame_result,
    RenderSceneThreePrimitiveCaptureResult *out_capture_result,
    std::size_t *out_runtime_texture_upload_count,
    std::size_t *out_material_texture_slot_count) {
    if (out_frame_result == nullptr) {
        return FailStep("null frame result output");
    }

    if (out_capture_result == nullptr) {
        return FailStep("null capture result output");
    }

    if (out_runtime_texture_upload_count == nullptr || out_material_texture_slot_count == nullptr) {
        return FailStep("null material texture bridge count output");
    }

    RhiBufferHandle buffer_slot_guard{};
    if (!CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U, &buffer_slot_guard)) {
        return FailStep("create buffer slot guard failed");
    }

    RhiBufferHandle cube_vertex{};
    if (!CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U * 24U, &cube_vertex)) {
        return FailStep("create cube vertex buffer failed");
    }

    RhiBufferHandle cube_index{};
    if (!CreateBuffer(device, RhiBufferUsage::Index, sizeof(std::uint16_t) * 36U, &cube_index)) {
        return FailStep("create cube index buffer failed");
    }

    RhiBufferHandle cylinder_vertex{};
    if (!CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U * 18U, &cylinder_vertex)) {
        return FailStep("create cylinder vertex buffer failed");
    }

    RhiBufferHandle cylinder_index{};
    if (!CreateBuffer(device, RhiBufferUsage::Index, sizeof(std::uint16_t) * 96U, &cylinder_index)) {
        return FailStep("create cylinder index buffer failed");
    }

    RhiBufferHandle cone_vertex{};
    if (!CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U * 10U, &cone_vertex)) {
        return FailStep("create cone vertex buffer failed");
    }

    RhiBufferHandle cone_index{};
    if (!CreateBuffer(device, RhiBufferUsage::Index, sizeof(std::uint16_t) * 48U, &cone_index)) {
        return FailStep("create cone index buffer failed");
    }

    if (graph.scene_output.status != RuntimeAssetDataStatus::Success ||
        graph.scene_output.entity_count != graph.scene_entities.size()) {
        return FailStep("production scene loader output is unavailable");
    }

    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    if (!BuildGeometry(
            RenderScenePrimitiveGeometryKind::Cube,
            graph.assets[0U].asset,
            11U,
            VertexView(cube_vertex, 24U),
            IndexView(cube_index, 36U),
            &geometry[0U])) {
        return FailStep("build cube geometry failed");
    }

    if (!BuildGeometry(
            RenderScenePrimitiveGeometryKind::Cylinder,
            graph.assets[1U].asset,
            12U,
            VertexView(cylinder_vertex, 18U),
            IndexView(cylinder_index, 96U),
            &geometry[1U])) {
        return FailStep("build cylinder geometry failed");
    }

    if (!BuildGeometry(
            RenderScenePrimitiveGeometryKind::Cone,
            graph.assets[2U].asset,
            13U,
            VertexView(cone_vertex, 10U),
            IndexView(cone_index, 48U),
            &geometry[2U])) {
        return FailStep("build cone geometry failed");
    }

    const std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{
        graph.assets[4U],
        graph.assets[5U],
        graph.assets[6U]};
    RenderSceneRuntimeMaterialRecord material{};
    const RuntimeTextureMaterialSlotBridgeResult material_result = BuildMaterial(
        device,
        registry,
        manager,
        graph.assets[3U].asset,
        std::span<const RuntimeAssetLoadedFile>(texture_assets.data(), texture_assets.size()),
        RuntimeTextureDesc(),
        &material);
    if (material_result.status != RuntimeTextureMaterialSlotBridgeStatus::Success) {
        return FailStep("build material failed");
    }

    *out_runtime_texture_upload_count = material_result.runtime_texture_upload_count;
    *out_material_texture_slot_count = material_result.material_texture_slot_count;

    RenderSceneCameraBindingResult camera{};
    if (!BuildCamera(device, &camera)) {
        return FailStep("build camera failed");
    }

    std::array<RenderSceneRuntimeFrameEntityRequest, 3U> frame_entities{};
    for (std::size_t index = 0U; index < frame_entities.size(); ++index) {
        const RuntimeAssetSceneEntityRecord &scene_entity = graph.scene_entities[index];
        frame_entities[index].world_object_id = scene_entity.world_object_id;
        frame_entities[index].transform = scene_entity.transform;
        frame_entities[index].geometry = geometry[index];
        frame_entities[index].is_visible = scene_entity.is_visible;
        frame_entities[index].is_active = scene_entity.is_active;
    }

    std::array<RenderSceneRuntimeFrameDrawRecord, 3U> draws{};
    RenderSceneRuntimeFrameRequest frame_request{};
    frame_request.frame_id = FRAME_ID;
    frame_request.camera = camera;
    frame_request.material = material;
    frame_request.entities = std::span<const RenderSceneRuntimeFrameEntityRequest>(
        frame_entities.data(),
        frame_entities.size());
    RenderSceneRuntimeFrameBuilder frame_builder;
    const RenderSceneRuntimeFrameStatus frame_status =
        frame_builder.Build(frame_request, std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()), out_frame_result);
    if (frame_status != RenderSceneRuntimeFrameStatus::Success) {
        return FailStep("build frame failed");
    }

    std::array<RenderSceneThreePrimitiveEntityRequest, 3U> capture_entities{};
    capture_entities[0U].world_object_id = graph.scene_entities[0U].world_object_id;
    capture_entities[0U].object_name = "Cube";
    capture_entities[0U].object_name_byte_count = 4U;
    capture_entities[0U].transform = graph.scene_entities[0U].transform;
    capture_entities[0U].geometry = geometry[0U];
    capture_entities[0U].is_visible = graph.scene_entities[0U].is_visible;
    capture_entities[0U].is_active = graph.scene_entities[0U].is_active;
    capture_entities[1U].world_object_id = graph.scene_entities[1U].world_object_id;
    capture_entities[1U].object_name = "Cylinder";
    capture_entities[1U].object_name_byte_count = 8U;
    capture_entities[1U].transform = graph.scene_entities[1U].transform;
    capture_entities[1U].geometry = geometry[1U];
    capture_entities[1U].is_visible = graph.scene_entities[1U].is_visible;
    capture_entities[1U].is_active = graph.scene_entities[1U].is_active;
    capture_entities[2U].world_object_id = graph.scene_entities[2U].world_object_id;
    capture_entities[2U].object_name = "Cone";
    capture_entities[2U].object_name_byte_count = 4U;
    capture_entities[2U].transform = graph.scene_entities[2U].transform;
    capture_entities[2U].geometry = geometry[2U];
    capture_entities[2U].is_visible = graph.scene_entities[2U].is_visible;
    capture_entities[2U].is_active = graph.scene_entities[2U].is_active;

    std::array<std::uint8_t, TOTAL_CAPTURE_BYTES> capture_bytes{};
    RenderSceneThreePrimitiveCaptureRequest capture_request{};
    capture_request.frame_id = FRAME_ID;
    capture_request.camera = camera;
    capture_request.material = material;
    capture_request.entities = std::span<const RenderSceneThreePrimitiveEntityRequest>(
        capture_entities.data(),
        capture_entities.size());
    capture_request.rhi_device = &device;
    capture_request.output_path = "Artifacts/RuntimeAssetData/Canonical.ppm";
    capture_request.output_path_byte_count = 39U;
    capture_request.capture_output = std::span<std::uint8_t>(capture_bytes.data(), capture_bytes.size());
    capture_request.capture_byte_budget_per_entity = CAPTURE_BYTES_PER_ENTITY;

    RenderSceneThreePrimitiveCaptureRoute capture_route;
    const RenderSceneThreePrimitiveCaptureStatus capture_status =
        capture_route.Execute(capture_request, out_capture_result);
    if (capture_status != RenderSceneThreePrimitiveCaptureStatus::Success) {
        return FailStep("capture route failed");
    }

    return true;
}

bool LoadRuntimeAssetRecords(
    MountTable &table,
    ResourceRegistry &registry,
    AssetManager &manager,
    LoadedGraph *out_graph) {
    if (out_graph == nullptr) {
        return FailStep("read scene failed");
    }

    LoadedGraph graph{};
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> file_descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        file_descs[index] = files[index].desc;
    }

    RuntimeAssetGraphLoadRequest load_request{};
    load_request.mount_table = &table;
    load_request.mount = MountId(MOUNT_ID);
    load_request.scene_path = VirtualPath(SCENE_PATH);
    load_request.scene_resource_type = ResourceTypeId{RESOURCE_TYPE_SCENE};
    load_request.scene_asset_type = AssetTypeId{ASSET_TYPE_SCENE};
    load_request.scene_stable_id = 6001U;
    load_request.files = file_descs.data();
    load_request.file_count = static_cast<std::uint32_t>(file_descs.size());
    load_request.resource_registry = &registry;
    load_request.asset_manager = &manager;
    load_request.loaded_files = graph.assets.data();
    load_request.loaded_file_capacity = static_cast<std::uint32_t>(graph.assets.size());
    load_request.scene_resource_refs = graph.scene_resource_refs.data();
    load_request.scene_resource_ref_capacity = static_cast<std::uint32_t>(graph.scene_resource_refs.size());
    load_request.scene_cameras = graph.scene_cameras.data();
    load_request.scene_camera_capacity = static_cast<std::uint32_t>(graph.scene_cameras.size());
    load_request.scene_entities = graph.scene_entities.data();
    load_request.scene_entity_capacity = static_cast<std::uint32_t>(graph.scene_entities.size());
    load_request.scene_transforms = graph.scene_transforms.data();
    load_request.scene_transform_capacity = static_cast<std::uint32_t>(graph.scene_transforms.size());
    load_request.scene_output = &graph.scene_output;
    load_request.animation_frame_context.frame_index = 1U;
    load_request.animation_frame_context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    load_request.animation_frame_context.fixed_time_nanoseconds = HALF_SECOND_NANOSECONDS;

    RuntimeAssetGraphLoadResult load_result{};
    const RuntimeAssetDataStatus load_status = LoadRuntimeAssetDataGraph(load_request, &load_result);
    if (load_status != RuntimeAssetDataStatus::Success) {
        std::fwrite(StatusName(load_status), sizeof(char), std::string_view(StatusName(load_status)).size(), stderr);
        std::fputc('\n', stderr);
        return FailStep("runtime asset graph load failed");
    }

    if (load_result.transaction_plan.status != RuntimeAssetDataStatus::Success ||
        load_result.transaction_plan.phase != RuntimeAssetLoadTransactionPhase::PreflightCommit ||
        load_result.transaction_plan.record_count != FIXTURE_FILE_COUNT + 1U ||
        load_result.transaction_plan.resource_commit_count != FIXTURE_FILE_COUNT + 1U ||
        load_result.transaction_plan.asset_commit_count != FIXTURE_FILE_COUNT + 1U ||
        load_result.transaction_plan.dependency_edge_commit_count != FIXTURE_FILE_COUNT * 2U ||
        load_result.transaction_result.status != RuntimeAssetDataStatus::Success ||
        load_result.transaction_result.phase != RuntimeAssetLoadTransactionPhase::CommitSceneOutput ||
        !load_result.transaction_result.mutated_state ||
        load_result.transaction_result.committed_resource_count != FIXTURE_FILE_COUNT + 1U ||
        load_result.transaction_result.committed_asset_count != FIXTURE_FILE_COUNT + 1U ||
        load_result.transaction_result.committed_dependency_edge_count != FIXTURE_FILE_COUNT * 2U) {
        return FailStep("runtime asset graph transaction diagnostics changed");
    }

    graph.scene_asset = load_result.scene;
    graph.loader_used_file_mount = load_result.file_read_count == FIXTURE_FILE_COUNT + 1U;
    graph.file_read_count = load_result.file_read_count;
    graph.resource_payload_count = load_result.cache_payload_count;
    graph.decoded_payload_count = load_result.decoded_payload_count;
    graph.resource_payloads_stored = load_result.cache_payload_count > FIXTURE_FILE_COUNT;
    graph.dependency_count = load_result.resource_dependency_count + load_result.asset_dependency_count;
    graph.scene_references_mesh_material_texture_shader = load_result.scene_references_runtime_asset_families;

    *out_graph = graph;
    return true;
}

void SeedSceneLoaderFailureSentinels(
    std::array<RuntimeAssetSceneResourceRef, FIXTURE_FILE_COUNT> &refs,
    std::array<RuntimeAssetSceneCameraRecord, 1U> &cameras,
    std::array<RuntimeAssetSceneEntityRecord, 3U> &entities,
    std::array<RuntimeAssetSceneTransformOutputRecord, 3U> &transforms,
    RuntimeAssetSceneLoaderOutput *output) {
    for (std::uint32_t index = 0U; index < refs.size(); ++index) {
        refs[index].kind = RuntimeAssetFileKind::Shader;
        refs[index].stable_id = 7000U + index;
        refs[index].loaded_file_index = 90U + index;
        refs[index].resource = ResourceHandle{10U + index, 20U + index};
        refs[index].asset = AssetHandle{30U + index, 40U + index};
    }

    cameras[0U].camera_id = 77U;
    cameras[0U].is_active = true;

    for (std::uint32_t index = 0U; index < entities.size(); ++index) {
        entities[index].entity_id = 80U + index;
        entities[index].world_object_id = WorldObjectId{800U + index};
        entities[index].transform.translation_x = 81.0F + static_cast<float>(index);
        entities[index].transform.rotation_y = 82.0F + static_cast<float>(index);
        entities[index].mesh_ref_index = 83U + index;
        entities[index].material_ref_index = 84U + index;
        entities[index].texture_ref_index = 85U + index;
        entities[index].shader_ref_index = 86U + index;
        entities[index].camera_index = 87U + index;
        entities[index].animation_ref_index = 88U + index;
        entities[index].is_visible = false;
        entities[index].is_active = false;
    }

    for (std::uint32_t index = 0U; index < transforms.size(); ++index) {
        transforms[index].world_object_id = WorldObjectId{900U + index};
        transforms[index].transform.translation_x = 91.0F + static_cast<float>(index);
        transforms[index].transform.rotation_y = 92.0F + static_cast<float>(index);
    }

    if (output != nullptr) {
        output->status = RuntimeAssetDataStatus::BudgetExceeded;
        output->scene_id = 777U;
        output->scene_hash = 778U;
        output->entity_count = 779U;
        output->transform_count = 780U;
        output->resource_ref_count = 781U;
        output->camera_count = 782U;
        output->animation_sampled_value_count = 783U;
        output->animation_sample_status = AnimationRuntimeStatus::InvalidClip;
        output->animation_apply_status = AnimationRuntimeStatus::InvalidTarget;
    }
}

bool SceneLoaderFailureSentinelsUnchanged(
    const std::array<RuntimeAssetSceneResourceRef, FIXTURE_FILE_COUNT> &refs,
    const std::array<RuntimeAssetSceneCameraRecord, 1U> &cameras,
    const std::array<RuntimeAssetSceneEntityRecord, 3U> &entities,
    const std::array<RuntimeAssetSceneTransformOutputRecord, 3U> &transforms,
    const RuntimeAssetSceneLoaderOutput &output) {
    for (std::uint32_t index = 0U; index < refs.size(); ++index) {
        if (refs[index].kind != RuntimeAssetFileKind::Shader ||
            refs[index].stable_id != 7000U + index ||
            refs[index].loaded_file_index != 90U + index ||
            refs[index].resource.slot != 10U + index ||
            refs[index].resource.generation != 20U + index ||
            refs[index].asset.slot != 30U + index ||
            refs[index].asset.generation != 40U + index) {
            return FailStep("scene loader failure mutated resource refs");
        }
    }

    if (cameras[0U].camera_id != 77U || !cameras[0U].is_active) {
        return FailStep("scene loader failure mutated camera output");
    }

    for (std::uint32_t index = 0U; index < entities.size(); ++index) {
        if (entities[index].entity_id != 80U + index ||
            entities[index].world_object_id.value != 800U + index ||
            !Approx(entities[index].transform.translation_x, 81.0F + static_cast<float>(index)) ||
            !Approx(entities[index].transform.rotation_y, 82.0F + static_cast<float>(index)) ||
            entities[index].mesh_ref_index != 83U + index ||
            entities[index].material_ref_index != 84U + index ||
            entities[index].texture_ref_index != 85U + index ||
            entities[index].shader_ref_index != 86U + index ||
            entities[index].camera_index != 87U + index ||
            entities[index].animation_ref_index != 88U + index ||
            entities[index].is_visible ||
            entities[index].is_active) {
            return FailStep("scene loader failure mutated entity output");
        }
    }

    for (std::uint32_t index = 0U; index < transforms.size(); ++index) {
        if (transforms[index].world_object_id.value != 900U + index ||
            !Approx(transforms[index].transform.translation_x, 91.0F + static_cast<float>(index)) ||
            !Approx(transforms[index].transform.rotation_y, 92.0F + static_cast<float>(index))) {
            return FailStep("scene loader failure mutated transform output");
        }
    }

    if (output.status != RuntimeAssetDataStatus::BudgetExceeded ||
        output.scene_id != 777U ||
        output.scene_hash != 778U ||
        output.entity_count != 779U ||
        output.transform_count != 780U ||
        output.resource_ref_count != 781U ||
        output.camera_count != 782U ||
        output.animation_sampled_value_count != 783U ||
        output.animation_sample_status != AnimationRuntimeStatus::InvalidClip ||
        output.animation_apply_status != AnimationRuntimeStatus::InvalidTarget) {
        return FailStep("scene loader failure mutated loader output");
    }

    return true;
}

bool ProbeSceneLoaderFailureWithoutOutputMutation(
    MountTable &table,
    RuntimeAssetDataStatus expected_status,
    RuntimeAssetLoadTransactionPhase expected_phase) {
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> file_descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        file_descs[index] = files[index].desc;
    }

    ResourceRegistry registry;
    AssetManager manager;
    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const auto before_cache_snapshot = registry.CachePayloadSnapshot();
    const auto before_decoded_snapshot = registry.DecodedPayloadSnapshot();
    const AssetSnapshot before_asset_snapshot = manager.Snapshot();
    RuntimeAssetRhiDevice rhi_device;
    if (rhi_device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return FailStep("scene loader failure rhi sentinel init failed");
    }

    RhiTextureHandle before_rhi_target{};
    if (rhi_device.GetSwapchainColorTarget(before_rhi_target) != RhiStatus::Success) {
        return FailStep("scene loader failure rhi sentinel target failed");
    }

    const auto before_rhi_snapshot = rhi_device.Snapshot();
    std::array<RuntimeAssetLoadedFile, FIXTURE_FILE_COUNT> loaded_files{};
    std::array<RuntimeAssetSceneResourceRef, FIXTURE_FILE_COUNT> scene_resource_refs{};
    std::array<RuntimeAssetSceneCameraRecord, 1U> scene_cameras{};
    std::array<RuntimeAssetSceneEntityRecord, 3U> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, 3U> scene_transforms{};
    RuntimeAssetSceneLoaderOutput scene_output{};
    SeedSceneLoaderFailureSentinels(
        scene_resource_refs,
        scene_cameras,
        scene_entities,
        scene_transforms,
        &scene_output);

    RuntimeAssetGraphLoadRequest load_request{};
    load_request.mount_table = &table;
    load_request.mount = MountId(MOUNT_ID);
    load_request.scene_path = VirtualPath(SCENE_PATH);
    load_request.scene_resource_type = ResourceTypeId{RESOURCE_TYPE_SCENE};
    load_request.scene_asset_type = AssetTypeId{ASSET_TYPE_SCENE};
    load_request.scene_stable_id = 6001U;
    load_request.files = file_descs.data();
    load_request.file_count = static_cast<std::uint32_t>(file_descs.size());
    load_request.resource_registry = &registry;
    load_request.asset_manager = &manager;
    load_request.loaded_files = loaded_files.data();
    load_request.loaded_file_capacity = static_cast<std::uint32_t>(loaded_files.size());
    load_request.scene_resource_refs = scene_resource_refs.data();
    load_request.scene_resource_ref_capacity = static_cast<std::uint32_t>(scene_resource_refs.size());
    load_request.scene_cameras = scene_cameras.data();
    load_request.scene_camera_capacity = static_cast<std::uint32_t>(scene_cameras.size());
    load_request.scene_entities = scene_entities.data();
    load_request.scene_entity_capacity = static_cast<std::uint32_t>(scene_entities.size());
    load_request.scene_transforms = scene_transforms.data();
    load_request.scene_transform_capacity = static_cast<std::uint32_t>(scene_transforms.size());
    load_request.scene_output = &scene_output;
    load_request.animation_frame_context.frame_index = 1U;
    load_request.animation_frame_context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    load_request.animation_frame_context.fixed_time_nanoseconds = HALF_SECOND_NANOSECONDS;

    RuntimeAssetGraphLoadResult load_result{};
    const RuntimeAssetDataStatus load_status = LoadRuntimeAssetDataGraph(load_request, &load_result);
    if (load_status != expected_status || load_result.status != expected_status) {
        return FailStep("scene loader failure did not return expected status");
    }

    if (load_result.transaction_result.status != expected_status ||
        load_result.transaction_plan.status != expected_status ||
        load_result.transaction_plan.phase != expected_phase ||
        load_result.transaction_result.phase != expected_phase ||
        load_result.transaction_result.mutated_state ||
        load_result.transaction_result.committed_resource_count != 0U ||
        load_result.transaction_result.committed_asset_count != 0U ||
        load_result.transaction_result.committed_cache_payload_count != 0U ||
        load_result.transaction_result.committed_decoded_payload_count != 0U ||
        load_result.transaction_result.committed_dependency_edge_count != 0U) {
        return FailStep("scene loader failure transaction mutated before commit");
    }

    if (!SceneLoaderFailureSentinelsUnchanged(
            scene_resource_refs,
            scene_cameras,
            scene_entities,
            scene_transforms,
            scene_output)) {
        return false;
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count ||
        after_resource_snapshot.dependency_edge_count != before_resource_snapshot.dependency_edge_count ||
        after_resource_snapshot.load_commit_record_count != before_resource_snapshot.load_commit_record_count ||
        after_resource_snapshot.loaded_resource_count != before_resource_snapshot.loaded_resource_count) {
        return FailStep("scene loader failure mutated Resource registry state");
    }

    const auto after_cache_snapshot = registry.CachePayloadSnapshot();
    if (after_cache_snapshot.cached_byte_count != before_cache_snapshot.cached_byte_count ||
        after_cache_snapshot.cached_payload_count != before_cache_snapshot.cached_payload_count ||
        after_cache_snapshot.cache_payload_record_count != before_cache_snapshot.cache_payload_record_count ||
        after_cache_snapshot.stored_payload_count != before_cache_snapshot.stored_payload_count) {
        return FailStep("scene loader failure mutated Resource cache payload state");
    }

    const auto after_decoded_snapshot = registry.DecodedPayloadSnapshot();
    if (after_decoded_snapshot.stored_decoded_byte_count != before_decoded_snapshot.stored_decoded_byte_count ||
        after_decoded_snapshot.active_payload_count != before_decoded_snapshot.active_payload_count ||
        after_decoded_snapshot.decoded_payload_record_count != before_decoded_snapshot.decoded_payload_record_count ||
        after_decoded_snapshot.stored_payload_count != before_decoded_snapshot.stored_payload_count) {
        return FailStep("scene loader failure mutated Resource decoded payload state");
    }

    const AssetSnapshot after_asset_snapshot = manager.Snapshot();
    if (after_asset_snapshot.active_asset_count != before_asset_snapshot.active_asset_count ||
        after_asset_snapshot.active_dependency_edge_count != before_asset_snapshot.active_dependency_edge_count ||
        after_asset_snapshot.registered_asset_count != before_asset_snapshot.registered_asset_count ||
        after_asset_snapshot.referenced_asset_count != before_asset_snapshot.referenced_asset_count) {
        return FailStep("scene loader failure mutated Asset manager state");
    }

    RhiTextureHandle after_rhi_target{};
    if (rhi_device.GetSwapchainColorTarget(after_rhi_target) != RhiStatus::Success) {
        return FailStep("scene loader failure rhi sentinel target query failed");
    }

    const auto after_rhi_snapshot = rhi_device.Snapshot();
    if (after_rhi_target.slot != before_rhi_target.slot ||
        after_rhi_target.generation != before_rhi_target.generation ||
        after_rhi_snapshot.color_target_count != before_rhi_snapshot.color_target_count ||
        after_rhi_snapshot.created_target_count != before_rhi_snapshot.created_target_count ||
        after_rhi_snapshot.resources.buffer_count != before_rhi_snapshot.resources.buffer_count ||
        after_rhi_snapshot.resources.texture_count != before_rhi_snapshot.resources.texture_count ||
        after_rhi_snapshot.resources.sampler_count != before_rhi_snapshot.resources.sampler_count ||
        after_rhi_snapshot.resources.shader_module_count != before_rhi_snapshot.resources.shader_module_count ||
        after_rhi_snapshot.resources.pipeline_count != before_rhi_snapshot.resources.pipeline_count) {
        return FailStep("scene loader failure mutated RHI handle state");
    }

    for (const RuntimeAssetLoadedFile &loaded_file : loaded_files) {
        if (loaded_file.stable_id != 0U ||
            loaded_file.resource.IsValid() ||
            loaded_file.asset.IsValid()) {
            return FailStep("scene loader failure mutated caller loaded file outputs");
        }
    }

    return true;
}

bool LoadGraph(MountTable &table, LoadedGraph *out_graph) {
    if (out_graph == nullptr) {
        return FailStep("read scene failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return false;
    }

    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return FailStep("initialize rhi failed");
    }

    if (!ExecuteLoadedRenderPath(
            device,
            registry,
            manager,
            graph,
            &graph.frame_result,
            &graph.capture_result,
            &graph.runtime_texture_upload_count,
            &graph.material_texture_slot_count)) {
        return FailStep("execute loaded render path failed");
    }

    graph.material_slots_from_decoded_payloads =
        graph.runtime_texture_upload_count == RUNTIME_TEXTURE_SLOT_COUNT &&
        graph.material_texture_slot_count == RUNTIME_TEXTURE_SLOT_COUNT &&
        graph.capture_result.material_texture_slot_report_count == RUNTIME_TEXTURE_SLOT_COUNT;
    graph.render_capture_completed = graph.capture_result.capture_bytes_written > 0U;
    graph.cpu_oracle_allowed = graph.render_capture_completed;
    *out_graph = graph;
    return true;
}

int RuntimeAssetDataGeneratorWritesDeterministicFilesAndHashes() {
    MountTable first_table;
    if (!CreateMountedTable(TestRoot("GeneratorA"), &first_table)) {
        return Fail("first mount setup failed");
    }

    MountTable second_table;
    if (!CreateMountedTable(TestRoot("GeneratorB"), &second_table)) {
        return Fail("second mount setup failed");
    }

    if (!WriteCanonicalFixture(first_table)) {
        return Fail("first generator write failed");
    }

    if (!WriteCanonicalFixture(second_table)) {
        return Fail("second generator write failed");
    }

    std::vector<std::uint8_t> first_scene{};
    std::vector<std::uint8_t> second_scene{};
    if (!ReadFile(first_table, SCENE_PATH, &first_scene)) {
        return Fail("first scene read failed");
    }

    if (!ReadFile(second_table, SCENE_PATH, &second_scene)) {
        return Fail("second scene read failed");
    }

    if (first_scene.size() != second_scene.size()) {
        return Fail("deterministic scene size changed");
    }

    const std::uint64_t first_hash = HashRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(first_scene.data(), first_scene.size()));
    const std::uint64_t second_hash = HashRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(second_scene.data(), second_scene.size()));
    if (first_hash != second_hash) {
        return Fail("deterministic scene hash changed");
    }

    if (!SceneReferencesRequiredAssets(first_scene)) {
        return Fail("scene did not reference required asset families");
    }

    return 0;
}

int RuntimeAssetDataFormatHeaderRejectsUnsupportedVersion() {
    LoadedGraph graph{};
    graph.resource_payload_count = 99U;
    graph.render_capture_completed = true;

    const std::vector<std::uint8_t> bytes = BytesFromString(
        "YUASSET MESH 2\n"
        "id=bad_mesh\n"
        "kind=cube\n"
        "vertices=24\n"
        "indices=36\n");
    RuntimeAssetValidationResult result{};
    const RuntimeAssetDataStatus status = ValidateRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(bytes.data(), bytes.size()),
        RuntimeAssetFileKind::Mesh,
        &result);
    if (status != RuntimeAssetDataStatus::UnsupportedVersion) {
        return Fail("unsupported version was not rejected");
    }

    if (graph.resource_payload_count != 99U) {
        return Fail("validator mutated resource output state");
    }

    if (!graph.render_capture_completed) {
        return Fail("validator mutated render output state");
    }

    return 0;
}

int RuntimeAssetDataValidatorRejectsInvalidBoundsWithoutOutputs() {
    LoadedGraph graph{};
    graph.file_read_count = 77U;
    graph.resource_payload_count = 88U;

    const std::vector<std::uint8_t> bytes = BytesFromString(
        "YUASSET MESH 1\n"
        "schema=rav0-source\n"
        "id=bad_bounds\n"
        "kind=cube\n"
        "vertices=0\n"
        "indices=36\n"
        "bounds=-1,-1,-1,1,1,1\n");
    RuntimeAssetValidationResult result{};
    const RuntimeAssetDataStatus status = ValidateRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(bytes.data(), bytes.size()),
        RuntimeAssetFileKind::Mesh,
        &result);
    if (status != RuntimeAssetDataStatus::InvalidBounds) {
        return Fail("invalid mesh bounds were not rejected");
    }

    if (graph.file_read_count != 77U) {
        return Fail("invalid bounds mutated file read count");
    }

    if (graph.resource_payload_count != 88U) {
        return Fail("invalid bounds mutated resource outputs");
    }

    if (graph.frame_result.output_draw_count != 0U) {
        return Fail("invalid bounds produced frame draws");
    }

    return 0;
}

int RuntimeAssetDataDependencyGraphRejectsMissingAndDuplicateRefs() {
    const std::vector<std::uint8_t> missing_bytes = BytesFromString(
        "YUASSET SCENE 1\n"
        "schema=rav0-source\n"
        "id=missing_scene_ref\n"
        "m1=Mesh/Cylinder.yumesh\n"
        "m2=Mesh/Cone.yumesh\n"
        "mat=Material/Shared.yumat\n"
        "t0=Texture/Albedo.yutex\n"
        "prog=Shader/RuntimeProgram.yuprogram\n"
        "cam=perspective\n"
        "anim=Animation/Spin.yuanim\n");
    RuntimeAssetValidationResult missing_result{};
    const RuntimeAssetDataStatus missing_status =
        ValidateRuntimeAssetDataBytes(
            std::span<const std::uint8_t>(missing_bytes.data(), missing_bytes.size()),
            RuntimeAssetFileKind::Scene,
            &missing_result);
    if (missing_status != RuntimeAssetDataStatus::MissingDependency) {
        return Fail("missing dependency was not rejected");
    }

    const std::vector<std::uint8_t> duplicate_bytes = BytesFromString(
        "YUASSET SCENE 1\n"
        "schema=rav0-source\n"
        "id=duplicate_scene_ref\n"
        "m0=Mesh/Cube.yumesh\n"
        "m0=Mesh/Cube.yumesh\n"
        "m1=Mesh/Cylinder.yumesh\n"
        "m2=Mesh/Cone.yumesh\n"
        "mat=Material/Shared.yumat\n"
        "t0=Texture/Albedo.yutex\n"
        "prog=Shader/RuntimeProgram.yuprogram\n"
        "cam=perspective\n"
        "anim=Animation/Spin.yuanim\n");
    RuntimeAssetValidationResult duplicate_result{};
    const RuntimeAssetDataStatus duplicate_status =
        ValidateRuntimeAssetDataBytes(
            std::span<const std::uint8_t>(duplicate_bytes.data(), duplicate_bytes.size()),
            RuntimeAssetFileKind::Scene,
            &duplicate_result);
    if (duplicate_status != RuntimeAssetDataStatus::DuplicateDependency) {
        return Fail("duplicate dependency was not rejected");
    }

    return 0;
}

bool ExpectValidationStatus(
    std::string_view text,
    RuntimeAssetFileKind kind,
    RuntimeAssetDataStatus expected_status);
bool ValidateText(
    std::string_view text,
    RuntimeAssetFileKind kind,
    RuntimeAssetValidationResult *out_result);

int RuntimeAssetDataShaderSceneAnimationRequireSourceSchema() {
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();

    RuntimeAssetValidationResult shader_result{};
    if (!ValidateText(files[7U].bytes, RuntimeAssetFileKind::Shader, &shader_result)) {
        return Fail("shader schema validator rejected canonical shader");
    }

    if (shader_result.schema_version != 1U || shader_result.identity_hash == 0U) {
        return Fail("shader schema metadata was not recorded");
    }

    RuntimeAssetValidationResult scene_result{};
    if (!ValidateText(SceneBytes(), RuntimeAssetFileKind::Scene, &scene_result)) {
        return Fail("scene schema validator rejected canonical scene");
    }

    if (scene_result.schema_version != 1U || scene_result.identity_hash == 0U) {
        return Fail("scene schema metadata was not recorded");
    }

    RuntimeAssetValidationResult animation_result{};
    if (!ValidateText(files[8U].bytes, RuntimeAssetFileKind::Animation, &animation_result)) {
        return Fail("animation schema validator rejected canonical animation");
    }

    if (animation_result.schema_version != 1U || animation_result.identity_hash == 0U) {
        return Fail("animation schema metadata was not recorded");
    }

    if (!ExpectValidationStatus(
            "YUASSET SHADER 1\n"
            "id=runtime_program\n"
            "stage_vs=bytecode:runtime_program_vs\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:position,color\n"
            "textures=3\n",
            RuntimeAssetFileKind::Shader,
            RuntimeAssetDataStatus::InvalidSchema)) {
        return Fail("shader without source schema was not rejected");
    }

    if (!ExpectValidationStatus(
            "YUASSET SCENE 1\n"
            "id=canonical_scene\n"
            "m0=Mesh/Cube.yumesh\n"
            "m1=Mesh/Cylinder.yumesh\n"
            "m2=Mesh/Cone.yumesh\n"
            "mat=Material/Shared.yumat\n"
            "t0=Texture/Albedo.yutex\n"
            "prog=Shader/RuntimeProgram.yuprogram\n"
            "anim=Animation/Spin.yuanim\n"
            "cam=camera:orbit\n",
            RuntimeAssetFileKind::Scene,
            RuntimeAssetDataStatus::InvalidSchema)) {
        return Fail("scene without source schema was not rejected");
    }

    if (!ExpectValidationStatus(
            "YUASSET ANIMATION 1\n"
            "id=spin\n"
            "target=scene_entity:101\n"
            "track=transform:rotation_y\n"
            "tracks=1\n"
            "sample_rate=30\n",
            RuntimeAssetFileKind::Animation,
            RuntimeAssetDataStatus::InvalidSchema)) {
        return Fail("animation without source schema was not rejected");
    }

    return 0;
}

int RuntimeAssetDataLoaderUsesFileResourcePathNotInMemoryStructs() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("LoaderPath"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("loaded graph failed");
    }

    if (!graph.loader_used_file_mount) {
        return Fail("loader did not use file mount");
    }

    if (graph.file_read_count != FIXTURE_FILE_COUNT + 1U) {
        return Fail("loader did not read expected file graph");
    }

    if (!graph.resource_payloads_stored) {
        return Fail("resource payloads were not stored");
    }

    return 0;
}

int RuntimeAssetDataSceneReferencesMeshMaterialTextureShader() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("SceneRefs"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    std::vector<std::uint8_t> scene_bytes{};
    if (!ReadFile(table, SCENE_PATH, &scene_bytes)) {
        return Fail("scene read failed");
    }

    RuntimeAssetValidationResult result{};
    const RuntimeAssetDataStatus status = ValidateRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(scene_bytes.data(), scene_bytes.size()),
        RuntimeAssetFileKind::Scene,
        &result);
    if (status != RuntimeAssetDataStatus::Success) {
        return Fail("scene dependency validator failed");
    }

    if (result.dependency_count != 8U) {
        return Fail("scene dependency count changed");
    }

    if (!SceneReferencesRequiredAssets(scene_bytes)) {
        return Fail("scene did not reference required asset families");
    }

    return 0;
}

int RuntimeAssetDataSceneFamilyDetectionIsPathIndependent() {
    constexpr const char *alternate_scene_path = "Scene/AlternateScene.sceneasset";
    MountTable table;
    if (!CreateMountedTable(TestRoot("SceneFamilyPathIndependent"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteAlternateRuntimeFamilyFixture(table, alternate_scene_path)) {
        return Fail("alternate family fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> file_descs = AlternateRuntimeFamilyFileDescs();

    RuntimeAssetGraphLoadRequest load_request{};
    load_request.mount_table = &table;
    load_request.mount = MountId(MOUNT_ID);
    load_request.scene_path = VirtualPath(alternate_scene_path);
    load_request.scene_resource_type = ResourceTypeId{RESOURCE_TYPE_SCENE};
    load_request.scene_asset_type = AssetTypeId{ASSET_TYPE_SCENE};
    load_request.scene_stable_id = 6002U;
    load_request.files = file_descs.data();
    load_request.file_count = static_cast<std::uint32_t>(file_descs.size());
    load_request.resource_registry = &registry;
    load_request.asset_manager = &manager;
    load_request.loaded_files = graph.assets.data();
    load_request.loaded_file_capacity = static_cast<std::uint32_t>(graph.assets.size());
    load_request.scene_resource_refs = graph.scene_resource_refs.data();
    load_request.scene_resource_ref_capacity = static_cast<std::uint32_t>(graph.scene_resource_refs.size());
    load_request.scene_cameras = graph.scene_cameras.data();
    load_request.scene_camera_capacity = static_cast<std::uint32_t>(graph.scene_cameras.size());
    load_request.scene_entities = graph.scene_entities.data();
    load_request.scene_entity_capacity = static_cast<std::uint32_t>(graph.scene_entities.size());
    load_request.scene_transforms = graph.scene_transforms.data();
    load_request.scene_transform_capacity = static_cast<std::uint32_t>(graph.scene_transforms.size());
    load_request.scene_output = &graph.scene_output;
    load_request.animation_frame_context.frame_index = 1U;
    load_request.animation_frame_context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    load_request.animation_frame_context.fixed_time_nanoseconds = HALF_SECOND_NANOSECONDS;

    RuntimeAssetGraphLoadResult load_result{};
    const RuntimeAssetDataStatus load_status = LoadRuntimeAssetDataGraph(load_request, &load_result);
    if (load_status != RuntimeAssetDataStatus::Success) {
        return Fail("path-independent scene family load failed");
    }

    if (!load_result.scene_references_runtime_asset_families) {
        return Fail("scene family detection depended on exact smoke fixture paths");
    }

    if (load_result.file_read_count != FIXTURE_FILE_COUNT + 1U) {
        return Fail("path-independent scene family load read unexpected file count");
    }

    return 0;
}

int RuntimeAssetDataLoaderRejectsMissingSchemaBeforeMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("LoaderMissingSchemaNoMutation"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    const std::string invalid_texture =
        "YUASSET TEXTURE 1\n"
        "id=albedo\n"
        "format=rgba8\n"
        "extent=2x2\n"
        "payload=checker\n";
    if (!WriteBytes(table, "Texture/Albedo.yutex", BytesFromString(invalid_texture))) {
        return Fail("invalid texture write failed");
    }

    if (!ProbeSceneLoaderFailureWithoutOutputMutation(
            table,
            RuntimeAssetDataStatus::InvalidSchema,
            RuntimeAssetLoadTransactionPhase::ValidateRecord)) {
        return Fail("missing schema loader failure mutated pre-commit state");
    }

    return 0;
}

int RuntimeAssetDataLoaderCommitFailureReportsMutatedState() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("LoaderCommitFailureMutatedState"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    ResourceRegistry registry;
    ResourceDescriptor duplicate_scene{};
    duplicate_scene.type = ResourceTypeId{RESOURCE_TYPE_SCENE};
    duplicate_scene.logical_key = ResourceLogicalKey("radc.6001");
    if (!registry.RegisterSyntheticDescriptor(duplicate_scene).Succeeded()) {
        return Fail("duplicate scene resource seed failed");
    }

    AssetManager manager;
    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const AssetSnapshot before_asset_snapshot = manager.Snapshot();

    LoadedGraph graph{};
    SeedSceneLoaderFailureSentinels(
        graph.scene_resource_refs,
        graph.scene_cameras,
        graph.scene_entities,
        graph.scene_transforms,
        &graph.scene_output);

    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> file_descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        file_descs[index] = files[index].desc;
    }

    RuntimeAssetGraphLoadRequest load_request{};
    load_request.mount_table = &table;
    load_request.mount = MountId(MOUNT_ID);
    load_request.scene_path = VirtualPath(SCENE_PATH);
    load_request.scene_resource_type = ResourceTypeId{RESOURCE_TYPE_SCENE};
    load_request.scene_asset_type = AssetTypeId{ASSET_TYPE_SCENE};
    load_request.scene_stable_id = 6001U;
    load_request.files = file_descs.data();
    load_request.file_count = static_cast<std::uint32_t>(file_descs.size());
    load_request.resource_registry = &registry;
    load_request.asset_manager = &manager;
    load_request.loaded_files = graph.assets.data();
    load_request.loaded_file_capacity = static_cast<std::uint32_t>(graph.assets.size());
    load_request.scene_resource_refs = graph.scene_resource_refs.data();
    load_request.scene_resource_ref_capacity = static_cast<std::uint32_t>(graph.scene_resource_refs.size());
    load_request.scene_cameras = graph.scene_cameras.data();
    load_request.scene_camera_capacity = static_cast<std::uint32_t>(graph.scene_cameras.size());
    load_request.scene_entities = graph.scene_entities.data();
    load_request.scene_entity_capacity = static_cast<std::uint32_t>(graph.scene_entities.size());
    load_request.scene_transforms = graph.scene_transforms.data();
    load_request.scene_transform_capacity = static_cast<std::uint32_t>(graph.scene_transforms.size());
    load_request.scene_output = &graph.scene_output;
    load_request.animation_frame_context.frame_index = 1U;
    load_request.animation_frame_context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    load_request.animation_frame_context.fixed_time_nanoseconds = HALF_SECOND_NANOSECONDS;

    RuntimeAssetGraphLoadResult load_result{};
    const RuntimeAssetDataStatus status = LoadRuntimeAssetDataGraph(load_request, &load_result);
    if (status != RuntimeAssetDataStatus::ResourceRegistrationFailed ||
        load_result.status != RuntimeAssetDataStatus::ResourceRegistrationFailed) {
        return Fail("commit failure did not return duplicate resource registration status");
    }

    if (load_result.transaction_plan.status != RuntimeAssetDataStatus::Success ||
        load_result.transaction_plan.phase != RuntimeAssetLoadTransactionPhase::PreflightCommit ||
        load_result.transaction_result.status != RuntimeAssetDataStatus::ResourceRegistrationFailed ||
        load_result.transaction_result.phase != RuntimeAssetLoadTransactionPhase::CommitResources ||
        !load_result.transaction_result.mutated_state) {
        return Fail("commit failure did not report post-commit mutation diagnostics");
    }

    if (load_result.scene_registered ||
        load_result.loaded_file_count != 0U ||
        load_result.resource_dependency_count != 0U ||
        load_result.asset_dependency_count != 0U ||
        load_result.transaction_result.committed_resource_count != 0U ||
        load_result.transaction_result.committed_asset_count != 0U ||
        load_result.transaction_result.committed_dependency_edge_count != 0U) {
        return Fail("commit failure recorded committed graph outputs");
    }

    if (!SceneLoaderFailureSentinelsUnchanged(
            graph.scene_resource_refs,
            graph.scene_cameras,
            graph.scene_entities,
            graph.scene_transforms,
            graph.scene_output)) {
        return Fail("commit failure mutated scene loader outputs");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    const AssetSnapshot after_asset_snapshot = manager.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count ||
        after_resource_snapshot.dependency_edge_count != before_resource_snapshot.dependency_edge_count ||
        after_asset_snapshot.active_asset_count != before_asset_snapshot.active_asset_count ||
        after_asset_snapshot.active_dependency_edge_count != before_asset_snapshot.active_dependency_edge_count) {
        return Fail("commit failure registered graph records before reporting failure");
    }

    return 0;
}

bool ExpectValidationStatus(
    std::string_view text,
    RuntimeAssetFileKind kind,
    RuntimeAssetDataStatus expected_status) {
    const std::vector<std::uint8_t> bytes = BytesFromString(std::string(text));
    RuntimeAssetValidationResult result{};
    const RuntimeAssetDataStatus status = ValidateRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(bytes.data(), bytes.size()),
        kind,
        &result);
    if (status != expected_status) {
        std::fwrite(StatusName(status), sizeof(char), std::string_view(StatusName(status)).size(), stderr);
        std::fputc('\n', stderr);
        return false;
    }

    return true;
}

bool ValidateText(
    std::string_view text,
    RuntimeAssetFileKind kind,
    RuntimeAssetValidationResult *out_result) {
    if (out_result == nullptr) {
        return false;
    }

    const std::vector<std::uint8_t> bytes = BytesFromString(std::string(text));
    const RuntimeAssetDataStatus status = ValidateRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(bytes.data(), bytes.size()),
        kind,
        out_result);
    if (status != RuntimeAssetDataStatus::Success) {
        std::fwrite(StatusName(status), sizeof(char), std::string_view(StatusName(status)).size(), stderr);
        std::fputc('\n', stderr);
        return false;
    }

    return true;
}

bool ExpectLoaderRejectsAlbedoTextureWithoutMutation(
    std::string_view texture_text,
    RuntimeAssetDataStatus expected_status) {
    MountTable table;
    if (!CreateMountedTable(TestRoot("CookedTextureRejectsWithoutMutation"), &table)) {
        return false;
    }

    if (!WriteCanonicalFixture(table)) {
        return FailStep("canonical fixture write failed");
    }

    const std::vector<std::uint8_t> texture_bytes = BytesFromString(std::string(texture_text));
    if (!WriteBytes(table, "Texture/Albedo.yutex", texture_bytes)) {
        return FailStep("invalid cooked texture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    graph.scene_output.status = RuntimeAssetDataStatus::BudgetExceeded;
    graph.assets[0U].stable_id = 7777U;
    graph.assets[0U].cache_payload_stored = true;
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> file_descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        file_descs[index] = files[index].desc;
    }

    RuntimeAssetGraphLoadRequest load_request{};
    load_request.mount_table = &table;
    load_request.mount = MountId(MOUNT_ID);
    load_request.scene_path = VirtualPath(SCENE_PATH);
    load_request.scene_resource_type = ResourceTypeId{RESOURCE_TYPE_SCENE};
    load_request.scene_asset_type = AssetTypeId{ASSET_TYPE_SCENE};
    load_request.scene_stable_id = 6004U;
    load_request.files = file_descs.data();
    load_request.file_count = static_cast<std::uint32_t>(file_descs.size());
    load_request.resource_registry = &registry;
    load_request.asset_manager = &manager;
    load_request.loaded_files = graph.assets.data();
    load_request.loaded_file_capacity = static_cast<std::uint32_t>(graph.assets.size());
    load_request.scene_resource_refs = graph.scene_resource_refs.data();
    load_request.scene_resource_ref_capacity = static_cast<std::uint32_t>(graph.scene_resource_refs.size());
    load_request.scene_cameras = graph.scene_cameras.data();
    load_request.scene_camera_capacity = static_cast<std::uint32_t>(graph.scene_cameras.size());
    load_request.scene_entities = graph.scene_entities.data();
    load_request.scene_entity_capacity = static_cast<std::uint32_t>(graph.scene_entities.size());
    load_request.scene_transforms = graph.scene_transforms.data();
    load_request.scene_transform_capacity = static_cast<std::uint32_t>(graph.scene_transforms.size());
    load_request.scene_output = &graph.scene_output;

    RuntimeAssetGraphLoadResult load_result{};
    const RuntimeAssetDataStatus status = LoadRuntimeAssetDataGraph(load_request, &load_result);
    if (status != expected_status) {
        std::fwrite(StatusName(status), sizeof(char), std::string_view(StatusName(status)).size(), stderr);
        std::fputc('\n', stderr);
        return FailStep("loader returned unexpected invalid cooked texture status");
    }

    if (load_result.scene_registered ||
        load_result.loaded_file_count != 0U ||
        load_result.cache_payload_count != 0U ||
        load_result.resource_dependency_count != 0U ||
        load_result.asset_dependency_count != 0U ||
        graph.scene_output.status != RuntimeAssetDataStatus::BudgetExceeded ||
        graph.assets[0U].stable_id != 7777U ||
        !graph.assets[0U].cache_payload_stored) {
        return FailStep("invalid cooked texture mutated runtime outputs");
    }

    return true;
}

int RuntimeAssetDataSourceCookedParserReportsBoundedMetadata() {
    RuntimeAssetValidationResult source_result{};
    const std::string source_text =
        "YUASSET MESH 1\n"
        "schema=rav0-source\n"
        "id=cube_mesh\n"
        "kind=cube\n"
        "vertices=24\n"
        "indices=36\n"
        "bounds=-1,-1,-1,1,1,1\n";
    if (!ValidateText(source_text, RuntimeAssetFileKind::Mesh, &source_result)) {
        return Fail("source parser rejected canonical mesh");
    }

    if (source_result.artifact_class != RuntimeAssetArtifactClass::Source ||
        source_result.source_hash != source_result.hash ||
        source_result.record_table_count != 1U ||
        source_result.record_table_byte_count != source_text.size() ||
        source_result.identity_hash == 0U) {
        return Fail("source parser did not report bounded source metadata");
    }

    RuntimeAssetValidationResult cooked_result{};
    const std::string cooked_text = ValidCookedTextureText();
    if (!ValidateText(cooked_text, RuntimeAssetFileKind::Texture, &cooked_result)) {
        return Fail("cooked parser rejected valid cooked texture");
    }

    if (cooked_result.artifact_class != RuntimeAssetArtifactClass::Cooked ||
        cooked_result.schema_version != 1U ||
        cooked_result.source_hash != HashText("albedo_source") ||
        cooked_result.payload_hash != HashText("checker") ||
        cooked_result.dependency_table_count != 1U ||
        cooked_result.dependency_count != 1U ||
        cooked_result.record_table_count != 1U ||
        cooked_result.record_table_byte_count != 64U ||
        cooked_result.payload_byte_count != 7U ||
        cooked_result.payload_alignment != 4U) {
        return Fail("cooked parser did not report bounded table/hash metadata");
    }

    if (cooked_result.texture_width != 2U || cooked_result.texture_height != 2U) {
        return Fail("cooked texture family metadata was not validated");
    }

    return 0;
}

int RuntimeAssetDataSourceCookedParserRejectsInvalidTablesHashesAndDependencies() {
    const std::string zero_record_table =
        CookedTextureText("checker", 1U, 0U, 64U, 4U, HashText("checker"));
    if (!ExpectValidationStatus(
            zero_record_table,
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::InvalidCount) ||
        !ExpectLoaderRejectsAlbedoTextureWithoutMutation(
            zero_record_table,
            RuntimeAssetDataStatus::InvalidCount)) {
        return Fail("cooked zero record table was not rejected");
    }

    const std::string zero_record_bytes =
        CookedTextureText("checker", 1U, 1U, 0U, 4U, HashText("checker"));
    if (!ExpectValidationStatus(
            zero_record_bytes,
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::InvalidSize) ||
        !ExpectLoaderRejectsAlbedoTextureWithoutMutation(
            zero_record_bytes,
            RuntimeAssetDataStatus::InvalidSize)) {
        return Fail("cooked zero record bytes were not rejected");
    }

    const std::string invalid_alignment =
        CookedTextureText("checker", 1U, 1U, 64U, 3U, HashText("checker"));
    if (!ExpectValidationStatus(
            invalid_alignment,
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::InvalidAlignment) ||
        !ExpectLoaderRejectsAlbedoTextureWithoutMutation(
            invalid_alignment,
            RuntimeAssetDataStatus::InvalidAlignment)) {
        return Fail("cooked invalid alignment was not rejected");
    }

    const std::string wrong_payload_hash =
        CookedTextureText("checker", 1U, 1U, 64U, 4U, HashText("wrong_payload"));
    if (!ExpectValidationStatus(
            wrong_payload_hash,
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::HashMismatch) ||
        !ExpectLoaderRejectsAlbedoTextureWithoutMutation(
            wrong_payload_hash,
            RuntimeAssetDataStatus::HashMismatch)) {
        return Fail("cooked payload hash mismatch was not rejected");
    }

    const std::string missing_dependency =
        CookedTextureText("checker", 2U, 1U, 64U, 4U, HashText("checker"));
    if (!ExpectValidationStatus(
            missing_dependency,
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::MissingDependency) ||
        !ExpectLoaderRejectsAlbedoTextureWithoutMutation(
            missing_dependency,
            RuntimeAssetDataStatus::MissingDependency)) {
        return Fail("cooked missing dependency row was not rejected");
    }

    return 0;
}

int RuntimeAssetDataHeaderParserRejectsPartialVersionsAndNoise() {
    constexpr std::string_view payload = "checker";

    if (!ExpectValidationStatus(
            SourceMeshText("YUASSET MESH 10"),
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::UnsupportedVersion)) {
        return Fail("source version 10 was treated as supported version 1");
    }

    if (!ExpectValidationStatus(
            CookedTextureTextWithHeader(
                "YUCOOKED TEXTURE 10",
                payload,
                1U,
                1U,
                64U,
                4U,
                HashText(payload)),
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::UnsupportedVersion)) {
        return Fail("cooked version 10 was treated as supported version 1");
    }

    if (!ExpectValidationStatus(
            SourceMeshText("YUASSET MESH 3"),
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::UnsupportedVersion)) {
        return Fail("source version 3 did not return unsupported version");
    }

    if (!ExpectValidationStatus(
            CookedTextureTextWithHeader(
                "YUCOOKED TEXTURE 3",
                payload,
                1U,
                1U,
                64U,
                4U,
                HashText(payload)),
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::UnsupportedVersion)) {
        return Fail("cooked version 3 did not return unsupported version");
    }

    if (!ExpectValidationStatus(
            SourceMeshText("prefix YUASSET MESH 1"),
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::InvalidHeader)) {
        return Fail("source header accepted prefix noise by substring");
    }

    if (!ExpectValidationStatus(
            CookedTextureTextWithHeader(
                "YUCOOKED TEXTURE 1 suffix",
                payload,
                1U,
                1U,
                64U,
                4U,
                HashText(payload)),
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::InvalidHeader)) {
        return Fail("cooked header accepted suffix noise by substring");
    }

    if (!ExpectValidationStatus(
            std::string("noise\n") + SourceMeshText("YUASSET MESH 1"),
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::InvalidHeader)) {
        return Fail("source parser accepted valid header from non-header line");
    }

    return 0;
}

int RuntimeAssetDataLoaderRejectsSchemaKindAndMisleadingSuffixBeforeMutation() {
    constexpr const char *misleading_scene_path = "Scene/MisleadingScene.yuscene";

    auto probe = [&](std::string_view scene_text, RuntimeAssetDataStatus expected_status) -> bool {
        MountTable table;
        if (!CreateMountedTable(TestRoot("LoaderRejectsMetadata"), &table)) {
            return false;
        }

        if (!WriteCanonicalFixture(table)) {
            return FailStep("canonical fixture write failed");
        }

        const std::vector<std::uint8_t> scene_bytes = BytesFromString(std::string(scene_text));
        if (!WriteBytes(table, misleading_scene_path, scene_bytes)) {
            return FailStep("misleading scene write failed");
        }

        ResourceRegistry registry;
        AssetManager manager;
        LoadedGraph graph{};
        graph.scene_output.status = RuntimeAssetDataStatus::BudgetExceeded;
        const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
        std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> file_descs{};
        for (std::size_t index = 0U; index < files.size(); ++index) {
            file_descs[index] = files[index].desc;
        }

        RuntimeAssetGraphLoadRequest load_request{};
        load_request.mount_table = &table;
        load_request.mount = MountId(MOUNT_ID);
        load_request.scene_path = VirtualPath(misleading_scene_path);
        load_request.scene_resource_type = ResourceTypeId{RESOURCE_TYPE_SCENE};
        load_request.scene_asset_type = AssetTypeId{ASSET_TYPE_SCENE};
        load_request.scene_stable_id = 6003U;
        load_request.files = file_descs.data();
        load_request.file_count = static_cast<std::uint32_t>(file_descs.size());
        load_request.resource_registry = &registry;
        load_request.asset_manager = &manager;
        load_request.loaded_files = graph.assets.data();
        load_request.loaded_file_capacity = static_cast<std::uint32_t>(graph.assets.size());
        load_request.scene_resource_refs = graph.scene_resource_refs.data();
        load_request.scene_resource_ref_capacity = static_cast<std::uint32_t>(graph.scene_resource_refs.size());
        load_request.scene_cameras = graph.scene_cameras.data();
        load_request.scene_camera_capacity = static_cast<std::uint32_t>(graph.scene_cameras.size());
        load_request.scene_entities = graph.scene_entities.data();
        load_request.scene_entity_capacity = static_cast<std::uint32_t>(graph.scene_entities.size());
        load_request.scene_transforms = graph.scene_transforms.data();
        load_request.scene_transform_capacity = static_cast<std::uint32_t>(graph.scene_transforms.size());
        load_request.scene_output = &graph.scene_output;

        RuntimeAssetGraphLoadResult load_result{};
        const RuntimeAssetDataStatus status = LoadRuntimeAssetDataGraph(load_request, &load_result);
        if (status != expected_status) {
            std::fwrite(StatusName(status), sizeof(char), std::string_view(StatusName(status)).size(), stderr);
            std::fputc('\n', stderr);
            return FailStep("loader metadata rejection returned unexpected status");
        }

        if (load_result.scene_registered ||
            load_result.loaded_file_count != 0U ||
            load_result.cache_payload_count != 0U ||
            graph.scene_output.status != RuntimeAssetDataStatus::BudgetExceeded ||
            graph.assets[0U].cache_payload_stored) {
            return FailStep("loader metadata rejection mutated runtime outputs");
        }

        return true;
    };

    if (!probe(
            "YUASSET SCENE 1\n"
            "id=missing_schema_scene\n"
            "m0=Mesh/Cube.yumesh\n"
            "m1=Mesh/Cylinder.yumesh\n"
            "m2=Mesh/Cone.yumesh\n"
            "mat=Material/Shared.yumat\n"
            "t0=Texture/Albedo.yutex\n"
            "prog=Shader/RuntimeProgram.yuprogram\n"
            "cam=camera:orbit\n"
            "anim=Animation/Spin.yuanim\n",
            RuntimeAssetDataStatus::InvalidSchema)) {
        return Fail("loader did not reject missing scene schema before mutation");
    }

    if (!probe(
            "YUASSET MESH 1\n"
            "schema=rav0-source\n"
            "id=mesh_inside_yuscene_path\n"
            "kind=cube\n"
            "vertices=24\n"
            "indices=36\n"
            "bounds=-1,-1,-1,1,1,1\n",
            RuntimeAssetDataStatus::InvalidKind)) {
        return Fail("loader trusted misleading .yuscene suffix over internal kind");
    }

    return 0;
}

int RuntimeAssetDataMeshMaterialTextureTypedValidatorsAcceptStructuredMetadata() {
    RuntimeAssetValidationResult mesh_result{};
    if (!ValidateText(
            "YUASSET MESH 1\n"
            "schema=rav0-source\n"
            "id=cube_mesh\n"
            "kind=cube\n"
            "vertices=24\n"
            "indices=36\n"
            "bounds=-1,-1,-1,1,1,1\n",
            RuntimeAssetFileKind::Mesh,
            &mesh_result)) {
        return Fail("mesh metadata validator rejected valid mesh");
    }

    if (mesh_result.version != 1U || mesh_result.schema_version != 1U ||
        mesh_result.identity_hash == 0U) {
        return Fail("mesh metadata did not report version schema identity");
    }

    if (mesh_result.vertex_count != 24U || mesh_result.index_count != 36U) {
        return Fail("mesh metadata counts were not parsed");
    }

    RuntimeAssetValidationResult material_result{};
    if (!ValidateText(
            "YUASSET MATERIAL 1\n"
            "schema=rav0-source\n"
            "id=shared_material\n"
            "shader=Shader/RuntimeProgram.yuprogram\n"
            "texture0=Texture/Albedo.yutex\n"
            "texture1=Texture/Normal.yutex\n"
            "texture2=Texture/Mask.yutex\n",
            RuntimeAssetFileKind::Material,
            &material_result)) {
        return Fail("material metadata validator rejected valid material");
    }

    if (material_result.dependency_count != 4U || material_result.texture_slot_count != 3U) {
        return Fail("material dependency metadata was not parsed");
    }

    RuntimeAssetValidationResult texture_result{};
    if (!ValidateText(
            "YUASSET TEXTURE 1\n"
            "schema=rav0-source\n"
            "id=albedo\n"
            "format=rgba8\n"
            "extent=2x2\n"
            "payload=checker\n",
            RuntimeAssetFileKind::Texture,
            &texture_result)) {
        return Fail("texture metadata validator rejected valid texture");
    }

    if (texture_result.texture_width != 2U || texture_result.texture_height != 2U) {
        return Fail("texture extent metadata was not parsed");
    }

    return 0;
}

int RuntimeAssetDataMaterialValidatorRejectsMissingDuplicateAndTypeMismatchRefs() {
    LoadedGraph graph{};
    graph.file_read_count = 31U;
    graph.resource_payload_count = 32U;
    graph.render_capture_completed = true;

    if (!ExpectValidationStatus(
            "YUASSET MATERIAL 1\n"
            "schema=rav0-source\n"
            "id=shared_material\n"
            "shader=Shader/RuntimeProgram.yuprogram\n"
            "texture0=Texture/Albedo.yutex\n"
            "texture1=Texture/Normal.yutex\n",
            RuntimeAssetFileKind::Material,
            RuntimeAssetDataStatus::MissingDependency)) {
        return Fail("material missing texture dependency was not rejected");
    }

    if (!ExpectValidationStatus(
            "YUASSET MATERIAL 1\n"
            "schema=rav0-source\n"
            "id=shared_material\n"
            "shader=Shader/RuntimeProgram.yuprogram\n"
            "texture0=Texture/Albedo.yutex\n"
            "texture1=Texture/Albedo.yutex\n"
            "texture2=Texture/Mask.yutex\n",
            RuntimeAssetFileKind::Material,
            RuntimeAssetDataStatus::DuplicateDependency)) {
        return Fail("material duplicate texture dependency was not rejected");
    }

    if (!ExpectValidationStatus(
            "YUASSET MATERIAL 1\n"
            "schema=rav0-source\n"
            "id=shared_material\n"
            "shader=Texture/Albedo.yutex\n"
            "texture0=Texture/Albedo.yutex\n"
            "texture1=Texture/Normal.yutex\n"
            "texture2=Texture/Mask.yutex\n",
            RuntimeAssetFileKind::Material,
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("material shader type mismatch was not rejected");
    }

    if (graph.file_read_count != 31U || graph.resource_payload_count != 32U ||
        !graph.render_capture_completed) {
        return Fail("material validator mutated output state");
    }

    if (graph.frame_result.output_draw_count != 0U) {
        return Fail("material validator produced frame draws");
    }

    return 0;
}

int RuntimeAssetDataTextureValidatorRejectsInvalidFormatExtentPayload() {
    LoadedGraph graph{};
    graph.file_read_count = 41U;
    graph.resource_payload_count = 42U;
    graph.render_capture_completed = true;

    if (!ExpectValidationStatus(
            "YUASSET TEXTURE 1\n"
            "schema=rav0-source\n"
            "id=albedo\n"
            "format=bc7\n"
            "extent=2x2\n"
            "payload=checker\n",
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("texture format mismatch was not rejected");
    }

    if (!ExpectValidationStatus(
            "YUASSET TEXTURE 1\n"
            "schema=rav0-source\n"
            "id=albedo\n"
            "format=rgba8\n"
            "extent=0x2\n"
            "payload=checker\n",
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::InvalidBounds)) {
        return Fail("texture invalid extent was not rejected");
    }

    if (!ExpectValidationStatus(
            "YUASSET TEXTURE 1\n"
            "schema=rav0-source\n"
            "id=albedo\n"
            "format=rgba8\n"
            "extent=2x2\n"
            "payload=\n",
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::InvalidSize)) {
        return Fail("texture empty payload was not rejected");
    }

    if (graph.file_read_count != 41U || graph.resource_payload_count != 42U ||
        !graph.render_capture_completed) {
        return Fail("texture validator mutated output state");
    }

    if (graph.frame_result.output_draw_count != 0U) {
        return Fail("texture validator produced frame draws");
    }

    return 0;
}

std::string CanonicalShaderProgramText() {
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    for (const FixtureFile &file : files) {
        if (file.desc.kind == RuntimeAssetFileKind::Shader) {
            return std::string(file.bytes);
        }
    }

    return {};
}

RuntimeAssetDataStatus DecodeShaderProgramText(
    std::string_view text,
    RuntimeAssetLoadedShaderProgramData *out_program) {
    if (out_program == nullptr) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    const std::vector<std::uint8_t> bytes = BytesFromString(std::string(text));
    return DecodeRuntimeAssetShaderProgramData(
        std::span<const std::uint8_t>(bytes.data(), bytes.size()),
        4001U,
        out_program);
}

RuntimeAssetShaderProgramPipelineRequest ProgramPipelineRequest(
    RuntimeAssetRhiDevice *device,
    const RuntimeAssetLoadedShaderProgramData *program) {
    RuntimeAssetShaderProgramPipelineRequest request{};
    request.device = device;
    request.program = program;
    return request;
}

int RuntimeAssetDataShaderProgramBridgeCreatesRhiPipelineFromLoadedBytecode() {
    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("rhi init failed");
    }

    RuntimeAssetLoadedShaderProgramData program{};
    if (DecodeShaderProgramText(CanonicalShaderProgramText(), &program) != RuntimeAssetDataStatus::Success) {
        return Fail("runtime asset shader program decode failed");
    }

    const RuntimeAssetShaderProgramPipelineRequest request = ProgramPipelineRequest(&device, &program);
    RuntimeAssetShaderProgramPipelineResult result{};
    const RuntimeAssetDataStatus status = BuildRuntimeAssetShaderProgramPipeline(request, &result);
    if (status != RuntimeAssetDataStatus::Success) {
        return Fail("runtime asset shader bridge rejected valid bytecode");
    }

    if (result.vertex_shader.generation == 0U || result.pixel_shader.generation == 0U ||
        result.pipeline.generation == 0U) {
        return Fail("runtime asset shader bridge did not create RHI primitives");
    }

    if (result.vertex_bytecode_hash == result.pixel_bytecode_hash) {
        return Fail("runtime asset shader bridge did not track distinct bytecode hashes");
    }

    if (result.vertex_bytecode_hash != program.vertex_bytecode_hash ||
        result.pixel_bytecode_hash != program.pixel_bytecode_hash) {
        return Fail("runtime asset shader bridge did not use decoded program hashes");
    }

    if (result.pipeline_desc.input_layout.element_count != program.input_layout.element_count ||
        result.texture_slot_count != program.texture_slot_count) {
        return Fail("runtime asset shader bridge did not preserve layout or texture slots");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.shader_module_count != 2U || snapshot.resources.pipeline_count != 1U) {
        return Fail("runtime asset shader bridge did not update RHI ownership counts");
    }

    return 0;
}

bool ExpectShaderBridgeRejectedWithoutRhiMutation(
    std::string_view text,
    RuntimeAssetDataStatus expected_status) {
    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return FailStep("rhi init failed");
    }

    const auto before = device.Snapshot();
    RuntimeAssetLoadedShaderProgramData program{};
    const RuntimeAssetDataStatus decode_status = DecodeShaderProgramText(text, &program);
    if (decode_status != expected_status) {
        return FailStep("shader program decode returned unexpected status");
    }

    const RuntimeAssetShaderProgramPipelineRequest request = ProgramPipelineRequest(&device, &program);
    RuntimeAssetShaderProgramPipelineResult result{};
    const RuntimeAssetDataStatus status = BuildRuntimeAssetShaderProgramPipeline(request, &result);
    if (status != expected_status) {
        return FailStep("runtime asset shader bridge returned unexpected rejection status");
    }

    const auto after = device.Snapshot();
    if (before.resources.shader_module_count != after.resources.shader_module_count ||
        before.resources.pipeline_count != after.resources.pipeline_count ||
        before.resources.created_primitive_count != after.resources.created_primitive_count ||
        before.failed_operation_count != after.failed_operation_count) {
        return FailStep("runtime asset shader bridge mutated RHI on invalid program data");
    }

    if (result.vertex_shader.generation != 0U || result.pixel_shader.generation != 0U ||
        result.pipeline.generation != 0U) {
        return FailStep("runtime asset shader bridge returned handles on invalid program data");
    }

    return true;
}

int RuntimeAssetDataShaderProgramBridgeRejectsInvalidProgramDataWithoutRhiMutation() {
    if (!ExpectShaderBridgeRejectedWithoutRhiMutation(
            "YUASSET SHADER 1\n"
            "schema=rav0-source\n"
            "id=runtime_program\n"
            "stage_vs=Texture/Albedo.yutex\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:position,color\n"
            "textures=3\n",
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("runtime asset shader bridge accepted invalid stage refs");
    }

    if (!ExpectShaderBridgeRejectedWithoutRhiMutation(
            "YUASSET SHADER 1\n"
            "schema=rav0-source\n"
            "id=runtime_program\n"
            "stage_vs=bytecode:\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:position,color\n"
            "textures=3\n",
            RuntimeAssetDataStatus::InvalidSize)) {
        return Fail("runtime asset shader bridge accepted missing bytecode");
    }

    if (!ExpectShaderBridgeRejectedWithoutRhiMutation(
            "YUASSET SHADER 1\n"
            "schema=rav0-source\n"
            "id=runtime_program\n"
            "stage_vs=bytecode:runtime_program_vs\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "stage_vs_hash=1\n"
            "input=layout:position,color\n"
            "textures=3\n",
            RuntimeAssetDataStatus::HashMismatch)) {
        return Fail("runtime asset shader bridge accepted hash mismatch");
    }

    if (!ExpectShaderBridgeRejectedWithoutRhiMutation(
            "YUASSET SHADER 1\n"
            "schema=rav0-source\n"
            "id=runtime_program\n"
            "stage_vs=bytecode:runtime_program_vs\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:color\n"
            "textures=3\n",
            RuntimeAssetDataStatus::InvalidInputLayout)) {
        return Fail("runtime asset shader bridge accepted input-layout mismatch");
    }

    if (!ExpectShaderBridgeRejectedWithoutRhiMutation(
            "YUASSET SHADER 1\n"
            "schema=rav0-source\n"
            "id=runtime_program\n"
            "stage_vs=bytecode:runtime_program_vs\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:position,normal\n"
            "textures=3\n",
            RuntimeAssetDataStatus::UnsupportedFieldValue)) {
        return Fail("runtime asset shader bridge accepted unsupported semantic");
    }

    if (!ExpectShaderBridgeRejectedWithoutRhiMutation(
            "YUASSET SHADER 1\n"
            "schema=rav0-source\n"
            "id=runtime_program\n"
            "stage_vs=bytecode:runtime_program_vs\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:position,color,texcoord\n"
            "textures=3\n",
            RuntimeAssetDataStatus::CapacityExceeded)) {
        return Fail("runtime asset shader bridge accepted layout capacity overflow");
    }

    return 0;
}

int RuntimeAssetDataShaderProgramDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs() {
    LoadedGraph graph{};
    graph.file_read_count = 15U;
    graph.resource_payload_count = 16U;
    graph.render_capture_completed = true;

    if (!ExpectValidationStatus(
            "YUASSET SHADER 1\n"
            "schema=rav0-source\n"
            "id=runtime_program\n"
            "stage_vs=bytecode:runtime_program_vs\n"
            "input=layout:position,color\n"
            "textures=3\n",
            RuntimeAssetFileKind::Shader,
            RuntimeAssetDataStatus::MissingDependency)) {
        return Fail("missing shader stage dependency was not rejected");
    }

    if (!ExpectValidationStatus(
            "YUASSET SHADER 1\n"
            "schema=rav0-source\n"
            "id=runtime_program\n"
            "stage_vs=bytecode:runtime_program_vs\n"
            "stage_vs=bytecode:runtime_program_vs\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:position,color\n"
            "textures=3\n",
            RuntimeAssetFileKind::Shader,
            RuntimeAssetDataStatus::DuplicateDependency)) {
        return Fail("duplicate shader stage dependency was not rejected");
    }

    if (!ExpectValidationStatus(
            "YUASSET SHADER 1\n"
            "schema=rav0-source\n"
            "id=runtime_program\n"
            "stage_vs=Texture/Albedo.yutex\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:position,color\n"
            "textures=3\n",
            RuntimeAssetFileKind::Shader,
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("shader stage type mismatch was not rejected");
    }

    if (graph.file_read_count != 15U || graph.resource_payload_count != 16U ||
        !graph.render_capture_completed) {
        return Fail("shader validator mutated output state");
    }

    return 0;
}

int RuntimeAssetDataSceneCameraAnimationDependencyValidatorRejectsTypeMismatchWithoutMutation() {
    LoadedGraph graph{};
    graph.dependency_count = 17U;
    graph.render_capture_completed = true;

    if (!ExpectValidationStatus(
            "YUASSET SCENE 1\n"
            "schema=rav0-source\n"
            "id=camera_type_mismatch_scene\n"
            "m0=Mesh/Cube.yumesh\n"
            "m1=Mesh/Cylinder.yumesh\n"
            "m2=Mesh/Cone.yumesh\n"
            "mat=Material/Shared.yumat\n"
            "t0=Texture/Albedo.yutex\n"
            "prog=Shader/RuntimeProgram.yuprogram\n"
            "anim=Animation/Spin.yuanim\n"
            "cam=Animation/Spin.yuanim\n",
            RuntimeAssetFileKind::Scene,
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("scene camera type mismatch was not rejected");
    }

    if (!ExpectValidationStatus(
            "YUASSET SCENE 1\n"
            "schema=rav0-source\n"
            "id=animation_type_mismatch_scene\n"
            "m0=Mesh/Cube.yumesh\n"
            "m1=Mesh/Cylinder.yumesh\n"
            "m2=Mesh/Cone.yumesh\n"
            "mat=Material/Shared.yumat\n"
            "t0=Texture/Albedo.yutex\n"
            "prog=Shader/RuntimeProgram.yuprogram\n"
            "anim=Texture/Albedo.yutex\n"
            "cam=camera:orbit\n",
            RuntimeAssetFileKind::Scene,
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("scene animation type mismatch was not rejected");
    }

    if (graph.dependency_count != 17U || !graph.render_capture_completed) {
        return Fail("scene typed dependency validator mutated output state");
    }

    return 0;
}

int RuntimeAssetDataAnimationDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs() {
    LoadedGraph graph{};
    graph.file_read_count = 19U;
    graph.resource_payload_count = 23U;

    if (!ExpectValidationStatus(
            "YUASSET ANIMATION 1\n"
            "schema=rav0-source\n"
            "id=spin\n"
            "track=transform:rotation_y\n"
            "tracks=3\n"
            "sample_rate=30\n",
            RuntimeAssetFileKind::Animation,
            RuntimeAssetDataStatus::MissingDependency)) {
        return Fail("missing animation target dependency was not rejected");
    }

    if (!ExpectValidationStatus(
            "YUASSET ANIMATION 1\n"
            "schema=rav0-source\n"
            "id=spin\n"
            "target=scene_entity:101\n"
            "target=scene_entity:102\n"
            "track=transform:rotation_y\n"
            "tracks=3\n"
            "sample_rate=30\n",
            RuntimeAssetFileKind::Animation,
            RuntimeAssetDataStatus::DuplicateDependency)) {
        return Fail("duplicate animation target dependency was not rejected");
    }

    if (!ExpectValidationStatus(
            "YUASSET ANIMATION 1\n"
            "schema=rav0-source\n"
            "id=spin\n"
            "target=Mesh/Cube.yumesh\n"
            "track=transform:rotation_y\n"
            "tracks=3\n"
            "sample_rate=30\n",
            RuntimeAssetFileKind::Animation,
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("animation target type mismatch was not rejected");
    }

    if (graph.file_read_count != 19U || graph.resource_payload_count != 23U) {
        return Fail("animation dependency validator mutated output state");
    }

    return 0;
}

int RuntimeAssetDataLoadCreatesRenderSceneRuntimeRecords() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("RuntimeRecords"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("loaded graph failed");
    }

    if (graph.frame_result.output_draw_count != 3U) {
        return Fail("RenderScene runtime draw count changed");
    }

    if (graph.capture_result.entity_report_count != 3U) {
        return Fail("RenderScene entity report count changed");
    }

    if (graph.capture_result.material_texture_slot_report_count != 3U) {
        return Fail("RenderScene material texture slot count changed");
    }

    if (!graph.material_slots_from_decoded_payloads) {
        return Fail("RenderScene material slots bypassed decoded texture payloads");
    }

    return 0;
}

int RuntimeAssetDataProductionSceneLoaderOutputsDeterministicRecords() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("ProductionSceneLoaderOutput"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("loaded graph failed");
    }

    if (graph.scene_output.status != RuntimeAssetDataStatus::Success) {
        return Fail("production scene loader output status failed");
    }

    if (graph.scene_output.scene_id != 6001U || graph.scene_output.scene_hash == 0U) {
        return Fail("production scene loader scene identity changed");
    }

    if (graph.scene_output.resource_ref_count != FIXTURE_FILE_COUNT ||
        graph.scene_output.entity_count != graph.scene_entities.size() ||
        graph.scene_output.transform_count != graph.scene_transforms.size() ||
        graph.scene_output.camera_count != graph.scene_cameras.size()) {
        return Fail("production scene loader output counts changed");
    }

    if (graph.scene_output.resource_ref_capacity != graph.scene_resource_refs.size() ||
        graph.scene_output.entity_capacity != graph.scene_entities.size() ||
        graph.scene_output.transform_capacity != graph.scene_transforms.size() ||
        graph.scene_output.camera_capacity != graph.scene_cameras.size()) {
        return Fail("production scene loader capacity counts changed");
    }

    if (graph.scene_output.file_read_count != FIXTURE_FILE_COUNT + 1U ||
        graph.scene_output.dependency_count != FIXTURE_FILE_COUNT * 2U ||
        graph.scene_output.cache_payload_count != graph.resource_payload_count ||
        graph.scene_output.decoded_payload_count != graph.decoded_payload_count) {
        return Fail("production scene loader diagnostics changed");
    }

    if (graph.scene_resource_refs[0U].kind != RuntimeAssetFileKind::Mesh ||
        graph.scene_resource_refs[3U].kind != RuntimeAssetFileKind::Material ||
        graph.scene_resource_refs[4U].kind != RuntimeAssetFileKind::Texture ||
        graph.scene_resource_refs[7U].kind != RuntimeAssetFileKind::Shader ||
        graph.scene_resource_refs[8U].kind != RuntimeAssetFileKind::Animation) {
        return Fail("production scene loader resource refs changed");
    }

    if (!graph.scene_resource_refs[0U].resource.IsValid() ||
        !graph.scene_resource_refs[0U].asset.IsValid() ||
        graph.scene_resource_refs[0U].stable_id != 1001U ||
        graph.scene_resource_refs[8U].stable_id != 5001U) {
        return Fail("production scene loader resource ref identity changed");
    }

    if (graph.scene_cameras[0U].camera_id != 1U || !graph.scene_cameras[0U].is_active) {
        return Fail("production scene loader camera record changed");
    }

    if (graph.scene_entities[0U].world_object_id.value != 101U ||
        graph.scene_entities[1U].world_object_id.value != 102U ||
        graph.scene_entities[2U].world_object_id.value != 103U) {
        return Fail("production scene loader entity ids changed");
    }

    if (graph.scene_entities[0U].mesh_ref_index != 0U ||
        graph.scene_entities[1U].mesh_ref_index != 1U ||
        graph.scene_entities[2U].mesh_ref_index != 2U ||
        graph.scene_entities[0U].material_ref_index != 3U ||
        graph.scene_entities[0U].texture_ref_index != 4U ||
        graph.scene_entities[0U].shader_ref_index != 7U ||
        graph.scene_entities[0U].animation_ref_index != 8U) {
        return Fail("production scene loader entity refs changed");
    }

    if (!Approx(graph.scene_entities[0U].transform.translation_x, -2.0F) ||
        !Approx(graph.scene_entities[1U].transform.translation_x, 0.0F) ||
        !Approx(graph.scene_entities[2U].transform.translation_x, 2.0F)) {
        return Fail("production scene loader transforms changed");
    }

    return 0;
}

int RuntimeAssetDataDiskAnimationSamplingFeedsSceneTransforms() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("DiskAnimationSampling"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("loaded graph failed");
    }

    if (graph.scene_output.animation_sample_status != AnimationRuntimeStatus::Success ||
        graph.scene_output.animation_apply_status != AnimationRuntimeStatus::Success ||
        graph.scene_output.animation_sampled_value_count != 1U) {
        return Fail("disk animation sampler diagnostics changed");
    }

    if (!Approx(graph.scene_entities[0U].transform.rotation_y, 0.5F) ||
        !Approx(graph.scene_transforms[0U].transform.rotation_y, 0.5F)) {
        return Fail("disk animation did not feed scene transform output");
    }

    if (!Approx(graph.scene_entities[1U].transform.rotation_y, 0.0F) ||
        !Approx(graph.scene_entities[2U].transform.rotation_y, 0.0F)) {
        return Fail("disk animation mutated unrelated scene transforms");
    }

    if (graph.capture_result.entity_report_count != 3U ||
        !Approx(graph.capture_result.entity_reports[0U].transform.rotation_y, 0.5F) ||
        !Approx(graph.capture_result.entity_reports[0U].draw_record.transform.rotation_y, 0.5F)) {
        return Fail("RenderScene did not consume production loader transform output");
    }

    return 0;
}

int RuntimeAssetDataSceneLoaderRejectsInvalidEntityWithoutOutputMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("SceneLoaderInvalidEntityNoMutation"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    const std::string invalid_scene =
        "YUASSET SCENE 1\n"
        "schema=rav0-source\n"
        "id=invalid_scene\n"
        "m0=Mesh/C.yumesh\n"
        "m1=Mesh/Y.yumesh\n"
        "m2=Mesh/N.yumesh\n"
        "mat=Material/M.yumat\n"
        "t0=Texture/A.yutex\n"
        "prog=Shader/P.yuprogram\n"
        "anim=Animation/S.yuanim\n"
        "cam=camera:orbit\n"
        "e0=101:-2,0,0\n"
        "e1=102:bad,0,0\n"
        "e2=103:2,0,0\n";
    if (!WriteBytes(table, SCENE_PATH, BytesFromString(invalid_scene))) {
        return Fail("invalid scene write failed");
    }

    if (!ProbeSceneLoaderFailureWithoutOutputMutation(
            table,
            RuntimeAssetDataStatus::InvalidDependency,
            RuntimeAssetLoadTransactionPhase::StageSceneOutput)) {
        return Fail("invalid scene entity failure mutated scene loader outputs");
    }

    return 0;
}

int RuntimeAssetDataSceneLoaderRejectsInvalidKeyframesWithoutOutputMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("SceneLoaderInvalidKeyframeNoMutation"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    const std::string invalid_animation =
        "YUASSET ANIMATION 1\n"
        "schema=rav0-source\n"
        "id=spin\n"
        "clip=1\n"
        "duration=1\n"
        "target=scene_entity:101\n"
        "track=transform:rotation_y\n"
        "key0=0:0\n"
        "key1=1:bad\n"
        "tracks=1\n"
        "sample_rate=30\n";
    if (!WriteBytes(table, "Animation/Spin.yuanim", BytesFromString(invalid_animation))) {
        return Fail("invalid animation write failed");
    }

    if (!ProbeSceneLoaderFailureWithoutOutputMutation(
            table,
            RuntimeAssetDataStatus::InvalidDependency,
            RuntimeAssetLoadTransactionPhase::StageSceneOutput)) {
        return Fail("invalid keyframe failure mutated scene loader outputs");
    }

    return 0;
}

int RuntimeAssetDataCookStoresDecodedPayloadsForMeshMaterialTexture() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("DecodedPayloads"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("loaded graph failed");
    }

    if (graph.decoded_payload_count != 7U) {
        return Fail("decoded payload count changed");
    }

    std::size_t index = 0U;
    while (index < 7U) {
        if (!graph.assets[index].decoded_payload_stored) {
            return Fail("decoded runtime asset payload was not stored");
        }

        ++index;
    }

    if (graph.assets[7U].decoded_payload_stored) {
        return Fail("shader payload was incorrectly marked decoded");
    }

    if (graph.assets[8U].decoded_payload_stored) {
        return Fail("animation payload was incorrectly marked decoded");
    }

    return 0;
}

int RuntimeAssetDataDecodedTexturePayloadsDriveRhiMaterialSlots() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("DecodedTextureMaterialSlots"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("loaded graph failed");
    }

    if (!graph.material_slots_from_decoded_payloads) {
        return Fail("material slots did not come from decoded texture payload uploads");
    }

    if (graph.runtime_texture_upload_count != RUNTIME_TEXTURE_SLOT_COUNT) {
        return Fail("decoded texture payload upload count changed");
    }

    if (graph.material_texture_slot_count != RUNTIME_TEXTURE_SLOT_COUNT) {
        return Fail("runtime material texture slot count changed");
    }

    if (graph.capture_result.material_texture_slot_report_count != RUNTIME_TEXTURE_SLOT_COUNT) {
        return Fail("capture route did not receive runtime material texture slots");
    }

    return 0;
}

int RuntimeAssetDataTextureMaterialSlotBridgeFailuresDoNotMutateRenderSceneOutputs() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("DecodedTextureMaterialSlotFailures"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("runtime asset records failed");
    }

    const std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{
        graph.assets[4U],
        graph.assets[5U],
        graph.assets[6U]};

    {
        RuntimeAssetRhiDevice device;
        if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
            return Fail("initialize damaged texture metadata rhi failed");
        }

        std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> damaged_assets = texture_assets;
        damaged_assets[0U].decoded_byte_count = RUNTIME_TEXTURE_BYTE_COUNT - 4U;

        RenderSceneRuntimeMaterialRecord material{};
        const RuntimeTextureMaterialSlotBridgeResult result = BuildMaterial(
            device,
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetLoadedFile>(damaged_assets.data(), damaged_assets.size()),
            RuntimeTextureDesc(),
            &material);
        if (result.status != RuntimeTextureMaterialSlotBridgeStatus::InvalidLoadedTexture) {
            return Fail("damaged texture metadata did not return InvalidLoadedTexture");
        }

        if (material.is_resolved || material.texture_slot_count != 0U) {
            return Fail("damaged texture metadata mutated RenderScene material output");
        }
    }

    {
        RuntimeAssetRhiDevice device;
        if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
            return Fail("initialize missing decoded payload rhi failed");
        }

        std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> missing_assets = texture_assets;
        missing_assets[0U].decoded_payload_id += 700000U;

        RenderSceneRuntimeMaterialRecord material{};
        const RuntimeTextureMaterialSlotBridgeResult result = BuildMaterial(
            device,
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetLoadedFile>(missing_assets.data(), missing_assets.size()),
            RuntimeTextureDesc(),
            &material);
        if (result.status != RuntimeTextureMaterialSlotBridgeStatus::TextureBridgeFailed) {
            return Fail("missing decoded payload did not fail through texture bridge");
        }

        if (result.texture_status != ResourceDecodedTextureBridgeStatus::ResourceQueryFailed) {
            return Fail("missing decoded payload did not expose bridge query failure");
        }

        if (result.decoded_payload_status != ResourceDecodedPayloadStatus::MissingDecodedPayload) {
            return Fail("missing decoded payload did not expose decoded payload status");
        }

        if (material.is_resolved || material.texture_slot_count != 0U) {
            return Fail("missing decoded payload mutated RenderScene material output");
        }
    }

    {
        RuntimeAssetRhiDevice device;
        if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
            return Fail("initialize decoded size mismatch rhi failed");
        }

        RhiTextureDesc size_mismatch_desc = RuntimeTextureDesc();
        size_mismatch_desc.extent = {1U, 2U};

        RenderSceneRuntimeMaterialRecord material{};
        const RuntimeTextureMaterialSlotBridgeResult result = BuildMaterial(
            device,
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetLoadedFile>(texture_assets.data(), texture_assets.size()),
            size_mismatch_desc,
            &material);
        if (result.status != RuntimeTextureMaterialSlotBridgeStatus::TextureBridgeFailed) {
            return Fail("decoded texture byte mismatch did not fail through texture bridge");
        }

        if (result.texture_status != ResourceDecodedTextureBridgeStatus::TextureByteCountMismatch) {
            return Fail("decoded texture byte mismatch did not expose bridge byte-count status");
        }

        if (material.is_resolved || material.texture_slot_count != 0U) {
            return Fail("decoded texture byte mismatch mutated RenderScene material output");
        }
    }

    {
        RuntimeAssetRhiDevice device;
        if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
            return Fail("initialize unsupported texture format rhi failed");
        }

        RhiTextureDesc unsupported_format_desc = RuntimeTextureDesc();
        unsupported_format_desc.format = RhiFormat::Unsupported;

        RenderSceneRuntimeMaterialRecord material{};
        const RuntimeTextureMaterialSlotBridgeResult result = BuildMaterial(
            device,
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetLoadedFile>(texture_assets.data(), texture_assets.size()),
            unsupported_format_desc,
            &material);
        if (result.status != RuntimeTextureMaterialSlotBridgeStatus::TextureBridgeFailed) {
            return Fail("unsupported texture format did not fail through texture bridge");
        }

        if (result.texture_status != ResourceDecodedTextureBridgeStatus::InvalidArgument) {
            return Fail("unsupported texture format did not expose bridge invalid-argument status");
        }

        if (material.is_resolved || material.texture_slot_count != 0U) {
            return Fail("unsupported texture format mutated RenderScene material output");
        }
    }

    {
        RuntimeAssetRhiDevice device;
        if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
            return Fail("initialize capacity rhi failed");
        }

        std::array<std::uint8_t, RUNTIME_TEXTURE_BYTE_COUNT> texture_bytes{};
        for (std::size_t index = 0U; index < yuengine::rhi::MAX_RHI_TEXTURES; ++index) {
            RhiTextureHandle texture{};
            const RhiStatus status = device.CreateTexture(
                RuntimeTextureDesc(),
                std::span<const std::uint8_t>(texture_bytes.data(), texture_bytes.size()),
                texture);
            if (status != RhiStatus::Success) {
                return Fail("failed to fill rhi texture capacity");
            }
        }

        RenderSceneRuntimeMaterialRecord material{};
        const RuntimeTextureMaterialSlotBridgeResult result = BuildMaterial(
            device,
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetLoadedFile>(texture_assets.data(), texture_assets.size()),
            RuntimeTextureDesc(),
            &material);
        if (result.status != RuntimeTextureMaterialSlotBridgeStatus::TextureBridgeFailed) {
            return Fail("rhi texture capacity did not fail through texture bridge");
        }

        if (result.texture_status != ResourceDecodedTextureBridgeStatus::UploadProcessFailed) {
            return Fail("rhi texture capacity did not expose bridge upload failure");
        }

        if (result.rhi_status != RhiStatus::CapacityExceeded) {
            return Fail("rhi texture capacity did not expose capacity status");
        }

        if (material.is_resolved || material.texture_slot_count != 0U) {
            return Fail("rhi texture capacity mutated RenderScene material output");
        }
    }

    if (graph.frame_result.output_draw_count != 0U) {
        return Fail("failure probes mutated RenderScene frame output");
    }

    if (graph.capture_result.capture_bytes_written != 0U) {
        return Fail("failure probes mutated RenderScene capture output");
    }

    return 0;
}

int RuntimeAssetDataLoadRegistersResourceAndAssetDependencyEdges() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("DependencyEdges"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("loaded graph failed");
    }

    if (!graph.scene_asset.resource.IsValid()) {
        return Fail("scene resource handle was not registered");
    }

    if (!graph.scene_asset.asset.IsValid()) {
        return Fail("scene asset handle was not registered");
    }

    if (graph.dependency_count != FIXTURE_FILE_COUNT * 2U) {
        return Fail("resource and asset dependency edge count changed");
    }

    return 0;
}

int RuntimeAssetDataRenderClosedLoopCapturesCubeCylinderConeThroughRhi() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("LoadRender"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("loaded graph failed");
    }

    if (!graph.loader_used_file_mount) {
        return Fail("loader did not use mount table");
    }

    if (!graph.resource_payloads_stored) {
        return Fail("resource payloads were not stored");
    }

    if (graph.frame_result.output_draw_count != 3U) {
        return Fail("RenderScene did not submit three draw records");
    }

    if (graph.capture_result.status != RenderSceneThreePrimitiveCaptureStatus::Success) {
        return Fail("RenderCore/RHI capture did not succeed");
    }

    if (!graph.render_capture_completed) {
        return Fail("capture bytes were not produced");
    }

    if (!graph.material_slots_from_decoded_payloads) {
        return Fail("runtime render path did not use decoded texture payload material slots");
    }

    return 0;
}

int RuntimeAssetDataCpuPpmOracleDoesNotBypassRhiRenderCore() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("OracleGuard"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("loaded graph failed");
    }

    if (!graph.render_capture_completed) {
        return Fail("capture did not complete before oracle");
    }

    if (!graph.cpu_oracle_allowed) {
        return Fail("oracle guard did not wait for capture");
    }

    return 0;
}

int RuntimeAssetDataDoesNotDependOnEditorWebUiInputOrGdiViewer() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("NoUpper"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("loaded graph failed");
    }

    if (graph.file_read_count != FIXTURE_FILE_COUNT + 1U) {
        return Fail("unexpected file read count");
    }

    if (graph.dependency_count != FIXTURE_FILE_COUNT * 2U) {
        return Fail("dependency graph count was not recorded");
    }

    if (!graph.scene_references_mesh_material_texture_shader) {
        return Fail("scene reference proof failed");
    }

    return 0;
}

const std::unordered_map<std::string_view, TestFunction> TESTS = {
    {TEST_GENERATOR, RuntimeAssetDataGeneratorWritesDeterministicFilesAndHashes},
    {TEST_UNSUPPORTED_VERSION, RuntimeAssetDataFormatHeaderRejectsUnsupportedVersion},
    {TEST_INVALID_BOUNDS, RuntimeAssetDataValidatorRejectsInvalidBoundsWithoutOutputs},
    {TEST_TYPED_MESH_MATERIAL_TEXTURE, RuntimeAssetDataMeshMaterialTextureTypedValidatorsAcceptStructuredMetadata},
    {TEST_MATERIAL_TYPED_REFS, RuntimeAssetDataMaterialValidatorRejectsMissingDuplicateAndTypeMismatchRefs},
    {TEST_TEXTURE_TYPED_METADATA, RuntimeAssetDataTextureValidatorRejectsInvalidFormatExtentPayload},
    {TEST_SHADER_SCENE_ANIMATION_SCHEMA, RuntimeAssetDataShaderSceneAnimationRequireSourceSchema},
    {TEST_INVALID_DEPENDENCY, RuntimeAssetDataDependencyGraphRejectsMissingAndDuplicateRefs},
    {TEST_SHADER_PROGRAM_PIPELINE_BRIDGE, RuntimeAssetDataShaderProgramBridgeCreatesRhiPipelineFromLoadedBytecode},
    {TEST_SHADER_PROGRAM_PIPELINE_REJECTS, RuntimeAssetDataShaderProgramBridgeRejectsInvalidProgramDataWithoutRhiMutation},
    {TEST_LOADER_FILE_RESOURCE, RuntimeAssetDataLoaderUsesFileResourcePathNotInMemoryStructs},
    {TEST_SCENE_REFERENCES, RuntimeAssetDataSceneReferencesMeshMaterialTextureShader},
    {TEST_SCENE_FAMILY_PATH_INDEPENDENT, RuntimeAssetDataSceneFamilyDetectionIsPathIndependent},
    {TEST_SOURCE_COOKED_METADATA, RuntimeAssetDataSourceCookedParserReportsBoundedMetadata},
    {TEST_SOURCE_COOKED_REJECTS, RuntimeAssetDataSourceCookedParserRejectsInvalidTablesHashesAndDependencies},
    {TEST_HEADER_REJECTS_PARTIAL_VERSION, RuntimeAssetDataHeaderParserRejectsPartialVersionsAndNoise},
    {TEST_LOADER_REJECTS_SCHEMA_KIND_SUFFIX,
     RuntimeAssetDataLoaderRejectsSchemaKindAndMisleadingSuffixBeforeMutation},
    {TEST_LOADER_TRANSACTION_INVALID_SCHEMA, RuntimeAssetDataLoaderRejectsMissingSchemaBeforeMutation},
    {TEST_LOADER_TRANSACTION_COMMIT_FAILURE, RuntimeAssetDataLoaderCommitFailureReportsMutatedState},
    {TEST_SHADER_PROGRAM_DEPENDENCIES, RuntimeAssetDataShaderProgramDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs},
    {TEST_SCENE_CAMERA_ANIMATION_DEPENDENCIES, RuntimeAssetDataSceneCameraAnimationDependencyValidatorRejectsTypeMismatchWithoutMutation},
    {TEST_ANIMATION_DEPENDENCIES, RuntimeAssetDataAnimationDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs},
    {TEST_LOADED_RENDER_RECORDS, RuntimeAssetDataLoadCreatesRenderSceneRuntimeRecords},
    {TEST_PRODUCTION_SCENE_LOADER_OUTPUT, RuntimeAssetDataProductionSceneLoaderOutputsDeterministicRecords},
    {TEST_DISK_ANIMATION_SAMPLING, RuntimeAssetDataDiskAnimationSamplingFeedsSceneTransforms},
    {TEST_SCENE_LOADER_INVALID_ENTITY_NO_MUTATION,
     RuntimeAssetDataSceneLoaderRejectsInvalidEntityWithoutOutputMutation},
    {TEST_SCENE_LOADER_INVALID_KEYFRAME_NO_MUTATION,
     RuntimeAssetDataSceneLoaderRejectsInvalidKeyframesWithoutOutputMutation},
    {TEST_DECODED_PAYLOADS, RuntimeAssetDataCookStoresDecodedPayloadsForMeshMaterialTexture},
    {TEST_TEXTURE_MATERIAL_SLOT_BRIDGE, RuntimeAssetDataDecodedTexturePayloadsDriveRhiMaterialSlots},
    {TEST_TEXTURE_MATERIAL_SLOT_BRIDGE_FAILURES,
     RuntimeAssetDataTextureMaterialSlotBridgeFailuresDoNotMutateRenderSceneOutputs},
    {TEST_RUNTIME_DEPENDENCIES, RuntimeAssetDataLoadRegistersResourceAndAssetDependencyEdges},
    {TEST_LOAD_RENDER, RuntimeAssetDataRenderClosedLoopCapturesCubeCylinderConeThroughRhi},
    {TEST_CPU_ORACLE, RuntimeAssetDataCpuPpmOracleDoesNotBypassRhiRenderCore},
    {TEST_NO_UPPER, RuntimeAssetDataDoesNotDependOnEditorWebUiInputOrGdiViewer},
};
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::string_view test_name(argv[1]);
    const auto test = TESTS.find(test_name);
    if (test == TESTS.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test->second();
}
