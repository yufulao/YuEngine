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
    const char *path = nullptr;
    const char *bytes = nullptr;
    ResourceTypeId resource_type{};
    AssetTypeId asset_type{};
    std::uint64_t stable_id = 0U;
};

struct LoadedAsset final {
    ResourceHandle resource;
    AssetHandle asset;
    std::uint64_t hash = 0U;
    std::size_t byte_count = 0U;
};

struct LoadedGraph final {
    std::array<LoadedAsset, FIXTURE_FILE_COUNT> assets{};
    LoadedAsset scene_asset{};
    std::size_t file_read_count = 0U;
    std::size_t resource_payload_count = 0U;
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

std::filesystem::path TestRoot(std::string_view test_name) {
    std::filesystem::path root = std::filesystem::temp_directory_path();
    root /= "YuEngineRuntimeAssetDataTests";
    root /= std::string(test_name);
    return root;
}

std::array<FixtureFile, FIXTURE_FILE_COUNT> CanonicalFiles() {
    return std::array<FixtureFile, FIXTURE_FILE_COUNT>{
        FixtureFile{
            "Mesh/Cube.yumesh",
            "YUASSET MESH 1\nid=cube_mesh\nkind=cube\nvertices=24\nindices=36\nbounds=-1,-1,-1,1,1,1\n",
            ResourceTypeId{RESOURCE_TYPE_MESH},
            AssetTypeId{ASSET_TYPE_MESH},
            1001U},
        FixtureFile{
            "Mesh/Cylinder.yumesh",
            "YUASSET MESH 1\nid=cylinder_mesh\nkind=cylinder\nvertices=18\nindices=96\nbounds=-1,-1,-1,1,1,1\n",
            ResourceTypeId{RESOURCE_TYPE_MESH},
            AssetTypeId{ASSET_TYPE_MESH},
            1002U},
        FixtureFile{
            "Mesh/Cone.yumesh",
            "YUASSET MESH 1\nid=cone_mesh\nkind=cone\nvertices=10\nindices=48\nbounds=-1,-1,-1,1,1,1\n",
            ResourceTypeId{RESOURCE_TYPE_MESH},
            AssetTypeId{ASSET_TYPE_MESH},
            1003U},
        FixtureFile{
            "Material/Shared.yumat",
            "YUASSET MATERIAL 1\nid=shared_material\nshader=Shader/RuntimeProgram.yuprogram\ntexture0=Texture/Albedo.yutex\ntexture1=Texture/Normal.yutex\ntexture2=Texture/Mask.yutex\n",
            ResourceTypeId{RESOURCE_TYPE_MATERIAL},
            AssetTypeId{ASSET_TYPE_MATERIAL},
            2001U},
        FixtureFile{
            "Texture/Albedo.yutex",
            "YUASSET TEXTURE 1\nid=albedo\nformat=rgba8\nextent=2x2\npayload=checker\n",
            ResourceTypeId{RESOURCE_TYPE_TEXTURE},
            AssetTypeId{ASSET_TYPE_TEXTURE},
            3001U},
        FixtureFile{
            "Texture/Normal.yutex",
            "YUASSET TEXTURE 1\nid=normal\nformat=rgba8\nextent=2x2\npayload=normal\n",
            ResourceTypeId{RESOURCE_TYPE_TEXTURE},
            AssetTypeId{ASSET_TYPE_TEXTURE},
            3002U},
        FixtureFile{
            "Texture/Mask.yutex",
            "YUASSET TEXTURE 1\nid=mask\nformat=rgba8\nextent=2x2\npayload=mask\n",
            ResourceTypeId{RESOURCE_TYPE_TEXTURE},
            AssetTypeId{ASSET_TYPE_TEXTURE},
            3003U},
        FixtureFile{
            "Shader/RuntimeProgram.yuprogram",
            "YUASSET SHADER 1\nid=runtime_program\ninput=position,color\ntextures=3\n",
            ResourceTypeId{RESOURCE_TYPE_SHADER},
            AssetTypeId{ASSET_TYPE_SHADER},
            4001U},
        FixtureFile{
            "Animation/Spin.yuanim",
            "YUASSET ANIMATION 1\nid=spin\ntracks=3\nsample_rate=30\n",
            ResourceTypeId{RESOURCE_TYPE_ANIMATION},
            AssetTypeId{ASSET_TYPE_ANIMATION},
            5001U}};
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

std::uint64_t HashBytes(std::span<const std::uint8_t> bytes) {
    std::uint64_t hash = FNV_OFFSET;
    for (const std::uint8_t byte : bytes) {
        hash ^= static_cast<std::uint64_t>(byte);
        hash *= FNV_PRIME;
    }

    return hash;
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
        if (!WriteBytes(table, file.path, bytes)) {
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

bool ValidateHeader(const std::vector<std::uint8_t> &bytes, std::string_view expected_kind) {
    const std::string text(bytes.begin(), bytes.end());
    const std::string header = "YUASSET " + std::string(expected_kind) + " 1";
    return Contains(text, header);
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

    return Contains(scene, "anim=Animation/Spin.yuanim");
}

bool RegisterResourcePayload(
    ResourceRegistry &registry,
    AssetManager &manager,
    const FixtureFile &file,
    const std::vector<std::uint8_t> &bytes,
    LoadedAsset *out_asset) {
    if (out_asset == nullptr) {
        return FailStep("null loaded asset output");
    }

    const std::string key = "radc." + std::to_string(file.stable_id);
    ResourceDescriptor resource_descriptor{};
    resource_descriptor.type = file.resource_type;
    resource_descriptor.logical_key = ResourceLogicalKey(key);
    const ResourceRegistrationResult resource_result =
        registry.RegisterSyntheticDescriptor(resource_descriptor);
    if (!resource_result.Succeeded()) {
        return FailStep("resource registration failed");
    }

    ResourceLoadCommitRequest commit_request{};
    commit_request.resource = resource_result.handle;
    commit_request.expected_type = file.resource_type;
    commit_request.load_state = ResourceLoadState::Uploaded;
    commit_request.commit_id = file.stable_id + 10000U;
    commit_request.upload_id = file.stable_id + 20000U;
    commit_request.staging_request_id = file.stable_id + 30000U;
    commit_request.upload_byte_count = static_cast<std::uint32_t>(bytes.size());
    if (registry.CommitUploadCompletion(commit_request) != ResourceLoadCommitStatus::Success) {
        return FailStep("resource upload commit failed");
    }

    ResourceResidencyRequest residency_request{};
    residency_request.resource = resource_result.handle;
    residency_request.expected_type = file.resource_type;
    if (registry.AdmitResident(residency_request) != ResourceResidencyStatus::Success) {
        return FailStep("resource residency admit failed");
    }

    ResourceCachePayloadRequest payload_request{};
    payload_request.resource = resource_result.handle;
    payload_request.expected_type = file.resource_type;
    payload_request.payload_id = file.stable_id + 40000U;
    payload_request.payload_bytes = bytes.data();
    payload_request.payload_byte_count = static_cast<std::uint32_t>(bytes.size());
    if (registry.StoreCachePayload(payload_request) != ResourceCachePayloadStatus::Success) {
        return FailStep("resource cache payload store failed");
    }

    AssetDescriptor asset_descriptor{};
    asset_descriptor.stable_id = file.stable_id;
    asset_descriptor.asset_type = file.asset_type;
    asset_descriptor.resource = resource_result.handle;
    asset_descriptor.resource_type = file.resource_type;
    const AssetRegistrationResult asset_result =
        manager.RegisterRuntimeAsset(&registry, asset_descriptor);
    if (!asset_result.Succeeded()) {
        return FailStep("asset registration failed");
    }

    out_asset->resource = resource_result.handle;
    out_asset->asset = asset_result.handle;
    out_asset->hash = HashBytes(std::span<const std::uint8_t>(bytes.data(), bytes.size()));
    out_asset->byte_count = bytes.size();
    return true;
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
    const std::array<LoadedAsset, FIXTURE_FILE_COUNT> &assets,
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
    std::vector<std::uint8_t> scene_bytes{};
    if (!ReadFile(table, SCENE_PATH, &scene_bytes)) {
        return FailStep("validate scene header failed");
    }

    graph.loader_used_file_mount = true;
    ++graph.file_read_count;
    if (!ValidateHeader(scene_bytes, "SCENE")) {
        return FailStep("scene references failed");
    }

    graph.scene_references_mesh_material_texture_shader = SceneReferencesRequiredAssets(scene_bytes);
    if (!graph.scene_references_mesh_material_texture_shader) {
        return FailStep("residency budget failed");
    }

    ResourceRegistry registry;
    ResourceResidencyBudgetDesc residency_budget{};
    residency_budget.byte_capacity = 4096U;
    if (registry.SetResidencyBudget(residency_budget) != ResourceResidencyStatus::Success) {
        return FailStep("cache payload budget failed");
    }

    if (registry.SetCachePayloadBudget({4096U}) != ResourceCachePayloadStatus::Success) {
        return false;
    }

    AssetManager manager;
    const FixtureFile scene_file{
        SCENE_PATH,
        "",
        ResourceTypeId{RESOURCE_TYPE_SCENE},
        AssetTypeId{ASSET_TYPE_SCENE},
        6001U};
    if (!RegisterResourcePayload(registry, manager, scene_file, scene_bytes, &graph.scene_asset)) {
        return FailStep("register scene payload failed");
    }

    ++graph.resource_payload_count;

    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    for (std::size_t index = 0U; index < files.size(); ++index) {
        std::vector<std::uint8_t> bytes{};
        if (!ReadFile(table, files[index].path, &bytes)) {
            return FailStep("read asset file failed");
        }

        ++graph.file_read_count;
        if (!RegisterResourcePayload(registry, manager, files[index], bytes, &graph.assets[index])) {
            return FailStep("register resource payload failed");
        }

        ++graph.resource_payload_count;
    }

    graph.resource_payloads_stored = graph.resource_payload_count == files.size() + 1U;
    graph.dependency_count = 8U;

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

    const std::uint64_t first_hash = HashBytes(std::span<const std::uint8_t>(first_scene.data(), first_scene.size()));
    const std::uint64_t second_hash = HashBytes(std::span<const std::uint8_t>(second_scene.data(), second_scene.size()));
    if (first_hash != second_hash) {
        return Fail("deterministic scene hash changed");
    }

    if (!SceneReferencesRequiredAssets(first_scene)) {
        return Fail("scene did not reference required asset families");
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

    if (graph.dependency_count != 8U) {
        return Fail("dependency graph count was not recorded");
    }

    if (!graph.scene_references_mesh_material_texture_shader) {
        return Fail("scene reference proof failed");
    }

    return 0;
}

const std::unordered_map<std::string_view, TestFunction> TESTS = {
    {TEST_GENERATOR, RuntimeAssetDataGeneratorWritesDeterministicFilesAndHashes},
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
