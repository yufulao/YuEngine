// 模块: Tests RenderScene
// 文件: Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp

#include <array>
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
using yuengine::asset::AssetDescriptor;
using yuengine::asset::AssetHandle;
using yuengine::asset::AssetManager;
using yuengine::asset::AssetRecord;
using yuengine::asset::AssetRegistrationResult;
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
using yuengine::resource::ResourceStatus;
using yuengine::resource::ResourceTypeId;
using yuengine::runtimeasset::HashRuntimeAssetDataBytes;
using yuengine::runtimeasset::BuildRuntimeAssetShaderProgramPipeline;
using yuengine::runtimeasset::DecodeRuntimeAssetShaderProgramData;
using yuengine::runtimeasset::LoadRuntimeAssetDataGraph;
using yuengine::runtimeasset::RuntimeAssetDataStatus;
using yuengine::runtimeasset::RuntimeAssetFileDesc;
using yuengine::runtimeasset::RuntimeAssetFileKind;
using yuengine::runtimeasset::RuntimeAssetGraphLoadRequest;
using yuengine::runtimeasset::RuntimeAssetGraphLoadResult;
using yuengine::runtimeasset::RuntimeAssetLoadedFile;
using yuengine::runtimeasset::RuntimeAssetLoadedShaderProgramData;
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
constexpr const char *TEST_SHADER_PROGRAM_DEPENDENCIES =
    "RuntimeAssetData_ShaderProgramDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs";
constexpr const char *TEST_SCENE_CAMERA_ANIMATION_DEPENDENCIES =
    "RuntimeAssetData_SceneCameraAnimationDependencyValidatorRejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_ANIMATION_DEPENDENCIES =
    "RuntimeAssetData_AnimationDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs";
constexpr const char *TEST_LOADED_RENDER_RECORDS =
    "RuntimeAssetData_LoadCreatesRenderSceneRuntimeRecords";
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
    RuntimeAssetLoadedFile scene_asset{};
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
            "YUASSET SHADER 1\nid=runtime_program\nstage_vs=bytecode:runtime_program_vs\nstage_ps=bytecode:runtime_program_ps\ninput=layout:position,color\ntextures=3\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Animation/Spin.yuanim",
                RuntimeAssetFileKind::Animation,
                ResourceTypeId{RESOURCE_TYPE_ANIMATION},
                AssetTypeId{ASSET_TYPE_ANIMATION},
                5001U},
            "YUASSET ANIMATION 1\nid=spin\ntarget=scene_entity:101\ntrack=transform:rotation_y\ntracks=3\nsample_rate=30\n"}};
}

std::string SceneBytes() {
    return std::string(
        "YUASSET SCENE 1\n"
        "m0=Mesh/Cube.yumesh\n"
        "m1=Mesh/Cylinder.yumesh\n"
        "m2=Mesh/Cone.yumesh\n"
        "mat=Material/Shared.yumat\n"
        "t0=Texture/Albedo.yutex\n"
        "prog=Shader/RuntimeProgram.yuprogram\n"
        "anim=Animation/Spin.yuanim\n"
        "cam=camera:orbit\n"
        "entities=3\n");
}

std::vector<std::uint8_t> BytesFromString(const std::string &text) {
    return std::vector<std::uint8_t>(text.begin(), text.end());
}

bool Contains(std::string_view text, std::string_view token) {
    return text.find(token) != std::string_view::npos;
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
    if (!Contains(scene, "m0=Mesh/Cube.yumesh")) {
        return FailStep("missing cube mesh dependency");
    }

    if (!Contains(scene, "m1=Mesh/Cylinder.yumesh")) {
        return FailStep("missing cylinder mesh dependency");
    }

    if (!Contains(scene, "m2=Mesh/Cone.yumesh")) {
        return FailStep("missing cone mesh dependency");
    }

    if (!Contains(scene, "mat=Material/Shared.yumat")) {
        return FailStep("missing material dependency");
    }

    if (!Contains(scene, "t0=Texture/Albedo.yutex")) {
        return FailStep("missing texture dependency");
    }

    if (!Contains(scene, "prog=Shader/RuntimeProgram.yuprogram")) {
        return FailStep("missing shader dependency");
    }

    if (!Contains(scene, "cam=camera:orbit")) {
        return FailStep("missing camera dependency");
    }

    return Contains(scene, "anim=Animation/Spin.yuanim");
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
    const std::array<RuntimeAssetLoadedFile, FIXTURE_FILE_COUNT> &assets,
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

    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    if (!BuildGeometry(
            RenderScenePrimitiveGeometryKind::Cube,
            assets[0U].asset,
            11U,
            VertexView(cube_vertex, 24U),
            IndexView(cube_index, 36U),
            &geometry[0U])) {
        return FailStep("build cube geometry failed");
    }

    if (!BuildGeometry(
            RenderScenePrimitiveGeometryKind::Cylinder,
            assets[1U].asset,
            12U,
            VertexView(cylinder_vertex, 18U),
            IndexView(cylinder_index, 96U),
            &geometry[1U])) {
        return FailStep("build cylinder geometry failed");
    }

    if (!BuildGeometry(
            RenderScenePrimitiveGeometryKind::Cone,
            assets[2U].asset,
            13U,
            VertexView(cone_vertex, 10U),
            IndexView(cone_index, 48U),
            &geometry[2U])) {
        return FailStep("build cone geometry failed");
    }

    const std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{
        assets[4U],
        assets[5U],
        assets[6U]};
    RenderSceneRuntimeMaterialRecord material{};
    const RuntimeTextureMaterialSlotBridgeResult material_result = BuildMaterial(
        device,
        registry,
        manager,
        assets[3U].asset,
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
    frame_entities[0U].world_object_id = WorldObjectId{101U};
    frame_entities[0U].transform = Transform(-2.0F, 0.0F, 0.0F);
    frame_entities[0U].geometry = geometry[0U];
    frame_entities[0U].is_visible = true;
    frame_entities[0U].is_active = true;
    frame_entities[1U].world_object_id = WorldObjectId{102U};
    frame_entities[1U].transform = Transform(0.0F, 0.0F, 0.0F);
    frame_entities[1U].geometry = geometry[1U];
    frame_entities[1U].is_visible = true;
    frame_entities[1U].is_active = true;
    frame_entities[2U].world_object_id = WorldObjectId{103U};
    frame_entities[2U].transform = Transform(2.0F, 0.0F, 0.0F);
    frame_entities[2U].geometry = geometry[2U];
    frame_entities[2U].is_visible = true;
    frame_entities[2U].is_active = true;

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
    capture_entities[0U].world_object_id = WorldObjectId{101U};
    capture_entities[0U].object_name = "Cube";
    capture_entities[0U].object_name_byte_count = 4U;
    capture_entities[0U].transform = Transform(-2.0F, 0.0F, 0.0F);
    capture_entities[0U].geometry = geometry[0U];
    capture_entities[0U].is_visible = true;
    capture_entities[0U].is_active = true;
    capture_entities[1U].world_object_id = WorldObjectId{102U};
    capture_entities[1U].object_name = "Cylinder";
    capture_entities[1U].object_name_byte_count = 8U;
    capture_entities[1U].transform = Transform(0.0F, 0.0F, 0.0F);
    capture_entities[1U].geometry = geometry[1U];
    capture_entities[1U].is_visible = true;
    capture_entities[1U].is_active = true;
    capture_entities[2U].world_object_id = WorldObjectId{103U};
    capture_entities[2U].object_name = "Cone";
    capture_entities[2U].object_name_byte_count = 4U;
    capture_entities[2U].transform = Transform(2.0F, 0.0F, 0.0F);
    capture_entities[2U].geometry = geometry[2U];
    capture_entities[2U].is_visible = true;
    capture_entities[2U].is_active = true;

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

    RuntimeAssetGraphLoadResult load_result{};
    const RuntimeAssetDataStatus load_status = LoadRuntimeAssetDataGraph(load_request, &load_result);
    if (load_status != RuntimeAssetDataStatus::Success) {
        std::fwrite(StatusName(load_status), sizeof(char), std::string_view(StatusName(load_status)).size(), stderr);
        std::fputc('\n', stderr);
        return FailStep("runtime asset graph load failed");
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
            graph.assets,
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
    {TEST_INVALID_DEPENDENCY, RuntimeAssetDataDependencyGraphRejectsMissingAndDuplicateRefs},
    {TEST_SHADER_PROGRAM_PIPELINE_BRIDGE, RuntimeAssetDataShaderProgramBridgeCreatesRhiPipelineFromLoadedBytecode},
    {TEST_SHADER_PROGRAM_PIPELINE_REJECTS, RuntimeAssetDataShaderProgramBridgeRejectsInvalidProgramDataWithoutRhiMutation},
    {TEST_LOADER_FILE_RESOURCE, RuntimeAssetDataLoaderUsesFileResourcePathNotInMemoryStructs},
    {TEST_SCENE_REFERENCES, RuntimeAssetDataSceneReferencesMeshMaterialTextureShader},
    {TEST_SHADER_PROGRAM_DEPENDENCIES, RuntimeAssetDataShaderProgramDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs},
    {TEST_SCENE_CAMERA_ANIMATION_DEPENDENCIES, RuntimeAssetDataSceneCameraAnimationDependencyValidatorRejectsTypeMismatchWithoutMutation},
    {TEST_ANIMATION_DEPENDENCIES, RuntimeAssetDataAnimationDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs},
    {TEST_LOADED_RENDER_RECORDS, RuntimeAssetDataLoadCreatesRenderSceneRuntimeRecords},
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
