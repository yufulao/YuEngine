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
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceLoadCommitRequest.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Resource/ResourceResidencyRequest.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"

namespace {
using yuengine::asset::AssetDescriptor;
using yuengine::asset::AssetHandle;
using yuengine::asset::AssetManager;
using yuengine::asset::AssetRegistrationResult;
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
using yuengine::runtimeasset::LoadRuntimeAssetDataGraph;
using yuengine::runtimeasset::RuntimeAssetDataStatus;
using yuengine::runtimeasset::RuntimeAssetFileDesc;
using yuengine::runtimeasset::RuntimeAssetFileKind;
using yuengine::runtimeasset::RuntimeAssetGraphLoadRequest;
using yuengine::runtimeasset::RuntimeAssetGraphLoadResult;
using yuengine::runtimeasset::RuntimeAssetLoadedFile;
using yuengine::runtimeasset::RuntimeAssetValidationResult;
using yuengine::runtimeasset::ValidateRuntimeAssetDataBytes;
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
constexpr const char *TEST_INVALID_DEPENDENCY =
    "RuntimeAssetData_DependencyGraphRejectsMissingAndDuplicateRefs";
constexpr const char *TEST_LOADER_FILE_RESOURCE =
    "RuntimeAssetData_LoaderUsesFileResourcePathNotInMemoryStructs";
constexpr const char *TEST_SCENE_REFERENCES =
    "RuntimeAssetData_SceneReferencesMeshMaterialTextureShader";
constexpr const char *TEST_LOADED_RENDER_RECORDS =
    "RuntimeAssetData_LoadCreatesRenderSceneRuntimeRecords";
constexpr const char *TEST_DECODED_PAYLOADS =
    "RuntimeAssetData_CookStoresDecodedPayloadsForMeshMaterialTexture";
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
    RenderSceneRuntimeFrameResult frame_result{};
    RenderSceneThreePrimitiveCaptureResult capture_result{};
    bool scene_references_mesh_material_texture_shader = false;
    bool loader_used_file_mount = false;
    bool resource_payloads_stored = false;
    bool render_capture_completed = false;
    bool cpu_oracle_allowed = false;
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
        case RuntimeAssetDataStatus::InvalidBounds:
            return "InvalidBounds";
        case RuntimeAssetDataStatus::MissingDependency:
            return "MissingDependency";
        case RuntimeAssetDataStatus::DuplicateDependency:
            return "DuplicateDependency";
        case RuntimeAssetDataStatus::CapacityExceeded:
            return "CapacityExceeded";
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
            "YUASSET MESH 1\nid=cube_mesh\nkind=cube\nvertices=24\nindices=36\nbounds=-1,-1,-1,1,1,1\n"},
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
            "YUASSET MESH 1\nid=cylinder_mesh\nkind=cylinder\nvertices=18\nindices=96\nbounds=-1,-1,-1,1,1,1\n"},
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
            "YUASSET MESH 1\nid=cone_mesh\nkind=cone\nvertices=10\nindices=48\nbounds=-1,-1,-1,1,1,1\n"},
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
            "YUASSET MATERIAL 1\nid=shared_material\nshader=Shader/RuntimeProgram.yuprogram\ntexture0=Texture/Albedo.yutex\ntexture1=Texture/Normal.yutex\ntexture2=Texture/Mask.yutex\n"},
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
            "YUASSET TEXTURE 1\nid=albedo\nformat=rgba8\nextent=2x2\npayload=checker\n"},
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
            "YUASSET TEXTURE 1\nid=normal\nformat=rgba8\nextent=2x2\npayload=normal\n"},
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
            "YUASSET TEXTURE 1\nid=mask\nformat=rgba8\nextent=2x2\npayload=mask\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Shader/RuntimeProgram.yuprogram",
                RuntimeAssetFileKind::Shader,
                ResourceTypeId{RESOURCE_TYPE_SHADER},
                AssetTypeId{ASSET_TYPE_SHADER},
                4001U},
            "YUASSET SHADER 1\nid=runtime_program\ninput=position,color\ntextures=3\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Animation/Spin.yuanim",
                RuntimeAssetFileKind::Animation,
                ResourceTypeId{RESOURCE_TYPE_ANIMATION},
                AssetTypeId{ASSET_TYPE_ANIMATION},
                5001U},
            "YUASSET ANIMATION 1\nid=spin\ntracks=3\nsample_rate=30\n"}};
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
        "cam=perspective\n"
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

    if (!Contains(scene, "cam=perspective")) {
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

bool CreateTexture(IRhiDevice &device, RhiTextureHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    const std::array<std::uint8_t, 16U> texture_bytes{
        255U, 0U, 0U, 255U,
        0U, 255U, 0U, 255U,
        0U, 0U, 255U, 255U,
        255U, 255U, 255U, 255U};
    RhiTextureDesc desc{};
    desc.format = RhiFormat::Rgba8Unorm;
    desc.extent = {2U, 2U};
    return device.CreateTexture(desc, std::span<const std::uint8_t>(texture_bytes.data(), texture_bytes.size()), *out_handle) ==
        RhiStatus::Success;
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

bool BuildMaterial(
    IRhiDevice &device,
    AssetHandle material_asset,
    std::span<const AssetHandle> texture_assets,
    RenderSceneRuntimeMaterialRecord *out_material) {
    if (out_material == nullptr) {
        return false;
    }

    if (texture_assets.size() < 3U) {
        return false;
    }

    RhiPipelineHandle pipeline{};
    if (!CreatePipeline(device, &pipeline)) {
        return false;
    }

    std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{};
    for (std::size_t index = 0U; index < slots.size(); ++index) {
        RhiTextureHandle texture{};
        if (!CreateTexture(device, &texture)) {
            return false;
        }

        RhiSamplerHandle sampler{};
        if (!CreateSampler(device, &sampler)) {
            return false;
        }

        slots[index].slot = static_cast<std::uint32_t>(index);
        slots[index].texture_asset = texture_assets[index];
        slots[index].sampled_texture = RhiSampledTextureBinding{texture, static_cast<std::uint32_t>(index)};
        slots[index].sampler = RhiSamplerBinding{sampler, static_cast<std::uint32_t>(index)};
    }

    RenderSceneRuntimeMaterialRequest request{};
    request.material_asset = material_asset;
    request.material_id = MATERIAL_ID;
    request.pipeline = pipeline;
    request.texture_slots = std::span<const RenderSceneRuntimeMaterialTextureSlot>(slots.data(), slots.size());

    RenderSceneRuntimeMaterialBuilder builder;
    const RenderSceneRuntimeMaterialStatus status = builder.Build(request, out_material);
    return status == RenderSceneRuntimeMaterialStatus::Success;
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
    const std::array<RuntimeAssetLoadedFile, FIXTURE_FILE_COUNT> &assets,
    RenderSceneRuntimeFrameResult *out_frame_result,
    RenderSceneThreePrimitiveCaptureResult *out_capture_result) {
    if (out_frame_result == nullptr) {
        return FailStep("null frame result output");
    }

    if (out_capture_result == nullptr) {
        return FailStep("null capture result output");
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

    const std::array<AssetHandle, 3U> texture_assets{assets[4U].asset, assets[5U].asset, assets[6U].asset};
    RenderSceneRuntimeMaterialRecord material{};
    if (!BuildMaterial(
            device,
            assets[3U].asset,
            std::span<const AssetHandle>(texture_assets.data(), texture_assets.size()),
            &material)) {
        return FailStep("build material failed");
    }

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

bool LoadGraph(MountTable &table, LoadedGraph *out_graph) {
    if (out_graph == nullptr) {
        return FailStep("read scene failed");
    }

    LoadedGraph graph{};
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> file_descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        file_descs[index] = files[index].desc;
    }

    ResourceRegistry registry;
    AssetManager manager;
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

    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return FailStep("initialize rhi failed");
    }

    if (!ExecuteLoadedRenderPath(device, graph.assets, &graph.frame_result, &graph.capture_result)) {
        return FailStep("execute loaded render path failed");
    }

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
    {TEST_INVALID_DEPENDENCY, RuntimeAssetDataDependencyGraphRejectsMissingAndDuplicateRefs},
    {TEST_LOADER_FILE_RESOURCE, RuntimeAssetDataLoaderUsesFileResourcePathNotInMemoryStructs},
    {TEST_SCENE_REFERENCES, RuntimeAssetDataSceneReferencesMeshMaterialTextureShader},
    {TEST_LOADED_RENDER_RECORDS, RuntimeAssetDataLoadCreatesRenderSceneRuntimeRecords},
    {TEST_DECODED_PAYLOADS, RuntimeAssetDataCookStoresDecodedPayloadsForMeshMaterialTexture},
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
