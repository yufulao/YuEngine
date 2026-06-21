// 模块: Tests RenderScene
// 文件: Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <vector>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h"
#include "YuEngine/RenderScene/RenderSceneOneCubeCaptureRoute.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingRequest.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingResult.h"
#include "YuEngine/RenderScene/RenderSceneCameraFrameBinder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryKind.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRecord.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRequest.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameDrawRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameEntityRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameResult.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialTextureSlot.h"
#include "YuEngine/RenderScene/RenderSceneStatus.h"
#include "YuEngine/RenderScene/RenderSceneThreePrimitiveCaptureRoute.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiDrawDesc.h"
#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiFormat.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementDrainRequest.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementDrainResult.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementRecord.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementRequest.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainResizeRequest.h"
#include "YuEngine/Rhi/RhiSwapchainResizeResult.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

using yuengine::asset::AssetHandle;
using yuengine::rendercore::RenderCameraProjectionKind;
using yuengine::rendercore::RenderDrawableFramePipelineStatus;
using yuengine::renderscene::RenderSceneCameraBindingRequest;
using yuengine::renderscene::RenderSceneCameraBindingResult;
using yuengine::renderscene::RenderSceneCameraFrameBinder;
using yuengine::renderscene::RenderSceneOneCubeCaptureMissingLayer;
using yuengine::renderscene::RenderSceneOneCubeCaptureOutputStatus;
using yuengine::renderscene::RenderSceneOneCubeCaptureRequest;
using yuengine::renderscene::RenderSceneOneCubeCaptureResult;
using yuengine::renderscene::RenderSceneOneCubeCaptureRoute;
using yuengine::renderscene::RenderSceneOneCubeCaptureStatus;
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
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureMissingLayer;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureOutputStatus;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureRequest;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureResult;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureRoute;
using yuengine::renderscene::RenderSceneThreePrimitiveCaptureStatus;
using yuengine::renderscene::RenderSceneThreePrimitiveEntityRequest;
using yuengine::renderscene::RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
using yuengine::rhi::IRhiDevice;
using yuengine::rhi::RhiBackendKind;
using yuengine::rhi::RhiBufferDesc;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiCapabilities;
using yuengine::rhi::RhiCaptureResult;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiColorTargetDesc;
using yuengine::rhi::RhiCommandList;
using yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiDeviceSnapshot;
using yuengine::rhi::RhiDrawDesc;
using yuengine::rhi::RhiDrawIndexedDesc;
using yuengine::rhi::RhiFenceHandle;
using yuengine::rhi::RhiFormat;
using yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiPipelineDesc;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveRetirementDrainRequest;
using yuengine::rhi::RhiPrimitiveRetirementDrainResult;
using yuengine::rhi::RhiPrimitiveRetirementRecord;
using yuengine::rhi::RhiPrimitiveRetirementRequest;
using yuengine::rhi::RhiPrimitiveTopology;
using yuengine::rhi::RhiSamplerBinding;
using yuengine::rhi::RhiSamplerDesc;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiSampledTextureBinding;
using yuengine::rhi::RhiShaderModuleDesc;
using yuengine::rhi::RhiShaderModuleHandle;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::RhiSwapchainResizeRequest;
using yuengine::rhi::RhiSwapchainResizeResult;
using yuengine::rhi::RhiTextureDesc;
using yuengine::rhi::RhiTextureHandle;
using yuengine::rhi::RhiVertexBufferView;
using yuengine::rhi::MAX_CAPTURE_FIXTURE_EXTENT;
using yuengine::rhi::MAX_COLOR_TARGET_EXTENT;
using yuengine::rhi::MAX_COMMANDS;
using yuengine::rhi::RGBA8_BYTES_PER_PIXEL;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldTransformState;

namespace {
constexpr const char *TEST_CAMERA_FRAME =
    "RenderScene_RuntimeCameraRecordBuildsDeterministicFrame";
constexpr const char *TEST_CAMERA_MISSING =
    "RenderScene_RuntimeCameraActiveBindingRejectsMissingCamera";
constexpr const char *TEST_CAMERA_CAPTURE =
    "RenderScene_RuntimeCameraCaptureMetadataRecordsFrameAndTarget";
constexpr const char *TEST_GEOMETRY_RANGES =
    "RenderScene_PrimitiveGeometryBuildsCubeCylinderConeRanges";
constexpr const char *TEST_GEOMETRY_MISSING =
    "RenderScene_PrimitiveGeometryMissingRecordReportsStatus";
constexpr const char *TEST_GEOMETRY_SMALL_BUFFER =
    "RenderScene_PrimitiveGeometryRejectsSmallBufferRanges";
constexpr const char *TEST_MATERIAL_THREE_SLOTS =
    "RenderScene_RuntimeMaterialBindsThreeTextureSlots";
constexpr const char *TEST_MATERIAL_MISSING_SLOT =
    "RenderScene_RuntimeMaterialRejectsMissingThirdSlot";
constexpr const char *TEST_MATERIAL_INVALID_TEXTURE =
    "RenderScene_RuntimeMaterialReportsInvalidTextureAsset";
constexpr const char *TEST_MATERIAL_INVALID_TEXTURE_BINDING =
    "RenderScene_RuntimeMaterialReportsInvalidTextureBinding";
constexpr const char *TEST_MATERIAL_INVALID_SAMPLER =
    "RenderScene_RuntimeMaterialReportsInvalidSamplerBinding";
constexpr const char *TEST_MATERIAL_INVALID_PIPELINE =
    "RenderScene_RuntimeMaterialReportsInvalidPipeline";
constexpr const char *TEST_FRAME_THREE_ENTITIES =
    "RenderScene_RuntimeFrameSubmitsThreeEntitiesWithSharedMaterial";
constexpr const char *TEST_FRAME_DUPLICATE_TRANSFORM =
    "RenderScene_RuntimeFrameRejectsDuplicateTransforms";
constexpr const char *TEST_FRAME_OUTPUT_CAPACITY =
    "RenderScene_RuntimeFrameRejectsSmallOutputCapacity";
constexpr const char *TEST_FRAME_MISSING_MATERIAL =
    "RenderScene_RuntimeFrameReportsMissingMaterial";
constexpr const char *TEST_FRAME_MISSING_GEOMETRY =
    "RenderScene_RuntimeFrameReportsMissingGeometry";
constexpr const char *TEST_L1_VIS_ONE_CUBE_CAPTURE =
    "RenderScene_L1Vis001CapturesStaticCubeThroughRuntimeRoute";
constexpr const char *TEST_L1_VIS_ENV_BLOCKED =
    "RenderScene_L1Vis001ReportsBlockedEnvForMissingSwapchain";
constexpr const char *TEST_L1_VIS_SHADER_MISSING =
    "RenderScene_L1Vis001ReportsShaderPipelineMissingLayer";
constexpr const char *TEST_L1_VIS_THREE_PRIMITIVE_CAPTURE =
    "RenderScene_L1Vis002CapturesThreePrimitivePlacedSceneThroughRuntimeRoute";
constexpr const char *TEST_L1_VIS_THREE_PRIMITIVE_GEOMETRY_MISSING =
    "RenderScene_L1Vis002ReportsGeometryMissingLayerForCylinder";
constexpr const char *TEST_L1_VIS_SHARED_THREE_TEXTURE_MATERIAL =
    "RenderScene_L1Vis003CapturesSharedThreeTextureMaterialSceneThroughRuntimeRoute";
constexpr const char *TEST_BOUNDARY =
    "RenderScene_RuntimeVisualFoundationNoEditorWebUiInputDependency";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr char L1_VIS_001_OUTPUT_PATH[] = "Artifacts/L1Vis001/StaticOneCube.rvf";
constexpr char L1_VIS_002_OUTPUT_PATH[] = "Artifacts/L1Vis002/ThreePrimitivePlaced.rvf";
constexpr char L1_VIS_003_OUTPUT_PATH[] = "Artifacts/L1Vis003/SharedThreeTextureMaterial.rvf";
constexpr char L1_VIS_002_CUBE_NAME[] = "Cube";
constexpr char L1_VIS_002_CYLINDER_NAME[] = "Cylinder";
constexpr char L1_VIS_002_CONE_NAME[] = "Cone";
constexpr float HALF_PI = 1.57079632679F;
constexpr float TOLERANCE = 0.0001F;
constexpr std::uint32_t FRAME_ID = 9101U;
constexpr std::uint32_t CAMERA_ID = 9201U;
constexpr std::uint32_t DRAW_ID = 9301U;
constexpr std::uint32_t PASS_ID = 9401U;
constexpr std::uint32_t MATERIAL_ID = 9501U;
constexpr std::uint32_t MATERIAL_ASSET_SLOT = 9601U;
constexpr std::uint32_t TEXTURE_ASSET_SLOT = 9701U;
constexpr std::size_t VERTEX_STRIDE_BYTES = 32U;
constexpr std::size_t VERTEX_BUFFER_BYTES = VERTEX_STRIDE_BYTES * 128U;
constexpr std::size_t INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * 256U;
constexpr std::size_t CAPTURE_BUDGET = 4096U;
constexpr std::uint16_t L1_VIS_CAPTURE_EXTENT = 4U;
constexpr std::uint8_t CAPTURE_SENTINEL = 0xCCU;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool Approx(float left, float right) {
    const float delta = std::fabs(left - right);
    return delta <= TOLERANCE;
}

bool TextureHandlesMatch(RhiTextureHandle left, RhiTextureHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool SamplerHandlesMatch(RhiSamplerHandle left, RhiSamplerHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

RhiTextureHandle TextureHandleForSlot(std::uint32_t slot) {
    return RhiTextureHandle{10U + slot, 1U};
}

RhiSamplerHandle SamplerHandleForSlot(std::uint32_t slot) {
    return RhiSamplerHandle{20U + slot, 1U};
}

std::size_t L1VisCaptureByteCount() {
    const std::size_t width = L1_VIS_CAPTURE_EXTENT;
    const std::size_t height = L1_VIS_CAPTURE_EXTENT;
    return width * height * RGBA8_BYTES_PER_PIXEL;
}

class L1Vis001RhiDevice final : public IRhiDevice {
public:
    L1Vis001RhiDevice() {
        ResetSwapchain();
    }

    RhiStatus Initialize(const RhiDeviceDesc &) override {
        ResetSwapchain();
        return RhiStatus::Success;
    }

    RhiStatus CreateColorTarget(const RhiColorTargetDesc &, RhiTextureHandle &out_handle) override {
        out_handle = RhiTextureHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus GetSwapchainColorTarget(RhiTextureHandle &out_handle) const override {
        out_handle = RhiTextureHandle{};
        if (!snapshot_.swapchain.valid) {
            return RhiStatus::InvalidLifecycle;
        }

        out_handle = target_;
        return RhiStatus::Success;
    }

    RhiStatus ResizeSwapchain(
        const RhiSwapchainResizeRequest &,
        RhiSwapchainResizeResult &out_result) override {
        out_result = RhiSwapchainResizeResult{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroyTarget(RhiTextureHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordClear(RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) override {
        if (!TextureHandlesMatch(handle, target_)) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordClear(handle, color);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        last_clear_color_ = color;
        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindPipeline(RhiCommandList &command_list, RhiPipelineHandle handle) override {
        if (handle.slot != pipeline_.slot || handle.generation != pipeline_.generation) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordBindPipeline(handle);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindVertexBuffer(RhiCommandList &command_list, const RhiVertexBufferView &view) override {
        if (view.buffer.slot != vertex_buffer_.slot || view.buffer.generation != vertex_buffer_.generation) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordBindVertexBuffer(view);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindIndexBuffer(RhiCommandList &command_list, const RhiIndexBufferView &view) override {
        if (view.buffer.slot != index_buffer_.slot || view.buffer.generation != index_buffer_.generation) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordBindIndexBuffer(view);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindSampledTexture(
        RhiCommandList &command_list,
        const RhiSampledTextureBinding &binding) override {
        const RhiTextureHandle expected = TextureHandleForSlot(binding.slot);
        if (!TextureHandlesMatch(binding.texture, expected)) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordBindSampledTexture(binding);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindSampler(RhiCommandList &command_list, const RhiSamplerBinding &binding) override {
        const RhiSamplerHandle expected = SamplerHandleForSlot(binding.slot);
        if (!SamplerHandlesMatch(binding.sampler, expected)) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordBindSampler(binding);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordDraw(RhiCommandList &, const RhiDrawDesc &) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordDrawIndexed(RhiCommandList &command_list, const RhiDrawIndexedDesc &desc) override {
        if (desc.topology != RhiPrimitiveTopology::TriangleList) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidDescriptor;
        }

        const RhiStatus status = command_list.RecordDrawIndexed(desc);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        last_draw_index_count_ = desc.index_count;
        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus Submit(const RhiCommandList &command_list) override {
        if (!command_list.IsComplete()) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidLifecycle;
        }

        if (!TextureHandlesMatch(command_list.TargetHandle(), target_)) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const auto command_snapshot = command_list.Snapshot();
        snapshot_.command_storage_capacity_before_frame = command_snapshot.capacity;
        snapshot_.command_storage_capacity_after_last_frame = command_snapshot.capacity;
        snapshot_.submitted_indexed_draw_count += command_snapshot.indexed_draw_command_count;
        snapshot_.submitted_sampled_texture_bind_count += command_snapshot.sampled_texture_bind_command_count;
        snapshot_.submitted_sampler_bind_count += command_snapshot.sampler_bind_command_count;
        snapshot_.last_indexed_draw_index_count = last_draw_index_count_;
        ++snapshot_.submit_count;
        submitted_ = true;
        return RhiStatus::Success;
    }

    RhiStatus Present() override {
        if (!submitted_) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidLifecycle;
        }

        ++snapshot_.present_count;
        snapshot_.swapchain.presented = true;
        presented_ = true;
        return RhiStatus::Success;
    }

    RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination) override {
        if (!presented_) {
            ++snapshot_.failed_operation_count;
            return RhiCaptureResult{RhiStatus::InvalidLifecycle, 0U};
        }

        const std::size_t byte_count = L1VisCaptureByteCount();
        if (destination.size() < byte_count) {
            ++snapshot_.failed_operation_count;
            return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
        }

        for (std::size_t index = 0U; index < byte_count; index += RGBA8_BYTES_PER_PIXEL) {
            destination[index] = last_clear_color_.r;
            destination[index + 1U] = static_cast<std::uint8_t>(last_draw_index_count_);
            destination[index + 2U] = last_clear_color_.b;
            destination[index + 3U] = last_clear_color_.a;
        }

        ++snapshot_.capture_count;
        snapshot_.last_capture_bytes_written = byte_count;
        return RhiCaptureResult{RhiStatus::Success, byte_count};
    }

    RhiStatus CreateBuffer(
        const RhiBufferDesc &,
        std::span<const std::uint8_t>,
        RhiBufferHandle &out_handle) override {
        out_handle = RhiBufferHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus UpdateBuffer(
        RhiBufferHandle,
        std::span<const std::uint8_t>,
        RhiFenceHandle &out_fence) override {
        out_fence = RhiFenceHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroyBuffer(RhiBufferHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus CreateTexture(
        const RhiTextureDesc &,
        std::span<const std::uint8_t>,
        RhiTextureHandle &out_handle) override {
        out_handle = RhiTextureHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus UpdateTexture(
        RhiTextureHandle,
        std::span<const std::uint8_t>,
        RhiFenceHandle &out_fence) override {
        out_fence = RhiFenceHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroyTexture(RhiTextureHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus CreateSampler(const RhiSamplerDesc &, RhiSamplerHandle &out_handle) override {
        out_handle = RhiSamplerHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroySampler(RhiSamplerHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus CreateShaderModule(const RhiShaderModuleDesc &, RhiShaderModuleHandle &out_handle) override {
        out_handle = RhiShaderModuleHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroyShaderModule(RhiShaderModuleHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus CreatePipeline(const RhiPipelineDesc &, RhiPipelineHandle &out_handle) override {
        out_handle = RhiPipelineHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroyPipeline(RhiPipelineHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RequestPrimitiveRetirement(
        const RhiPrimitiveRetirementRequest &,
        RhiPrimitiveRetirementRecord &out_record) override {
        out_record = RhiPrimitiveRetirementRecord{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus QueryPrimitiveRetirement(
        std::uint64_t,
        RhiPrimitiveRetirementRecord &out_record) const override {
        out_record = RhiPrimitiveRetirementRecord{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DrainPrimitiveRetirements(
        const RhiPrimitiveRetirementDrainRequest &,
        RhiPrimitiveRetirementDrainResult &out_result) override {
        out_result = RhiPrimitiveRetirementDrainResult{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiCapabilities Capabilities() const override {
        RhiCapabilities capabilities{};
        capabilities.backend_kind = RhiBackendKind::Null;
        capabilities.color_format = RhiFormat::Rgba8Unorm;
        capabilities.color_target_capacity = 1U;
        capabilities.command_list_capacity = MAX_COMMANDS;
        capabilities.max_color_target_extent = MAX_COLOR_TARGET_EXTENT;
        capabilities.max_capture_fixture_extent = MAX_CAPTURE_FIXTURE_EXTENT;
        capabilities.supports_capture = true;
        capabilities.supports_swapchain = true;
        return capabilities;
    }

    RhiDeviceSnapshot Snapshot() const override {
        return snapshot_;
    }

    void SetSwapchainValid(bool value) {
        snapshot_.swapchain.valid = value;
    }

private:
    void ResetSwapchain() {
        target_ = RhiTextureHandle{7U, 1U};
        pipeline_ = RhiPipelineHandle{4U, 1U};
        vertex_buffer_ = RhiBufferHandle{1U, 1U};
        index_buffer_ = RhiBufferHandle{2U, 1U};
        snapshot_ = RhiDeviceSnapshot{};
        snapshot_.color_target_capacity = 1U;
        snapshot_.color_target_count = 1U;
        snapshot_.created_target_count = 1U;
        snapshot_.swapchain.valid = true;
        snapshot_.swapchain.extent.width = L1_VIS_CAPTURE_EXTENT;
        snapshot_.swapchain.extent.height = L1_VIS_CAPTURE_EXTENT;
        snapshot_.swapchain.color_format = RhiFormat::Rgba8Unorm;
        snapshot_.swapchain.color_target = target_;
        last_clear_color_ = RhiColor{};
        last_draw_index_count_ = 0U;
        submitted_ = false;
        presented_ = false;
    }

    RhiDeviceSnapshot snapshot_{};
    RhiTextureHandle target_{};
    RhiPipelineHandle pipeline_{};
    RhiBufferHandle vertex_buffer_{};
    RhiBufferHandle index_buffer_{};
    RhiColor last_clear_color_{};
    std::uint32_t last_draw_index_count_ = 0U;
    bool submitted_ = false;
    bool presented_ = false;
};

RenderSceneRuntimeCameraRecord CameraRecord(std::uint32_t camera_id=CAMERA_ID) {
    RenderSceneRuntimeCameraRecord camera{};
    camera.camera_id = camera_id;
    camera.pose.position = {0.0F, 0.0F, -5.0F};
    camera.pose.target = {0.0F, 0.0F, 0.0F};
    camera.pose.up = {0.0F, 1.0F, 0.0F};
    camera.projection.kind = RenderCameraProjectionKind::Perspective;
    camera.projection.vertical_fov_radians = HALF_PI;
    camera.projection.aspect_ratio = 1.0F;
    camera.projection.near_z = 0.1F;
    camera.projection.far_z = 100.0F;
    camera.target = RhiTextureHandle{7U, 1U};
    camera.clear_color = RhiColor{10U, 20U, 30U, 255U};
    camera.is_active = true;
    return camera;
}

RenderSceneCameraBindingRequest CameraRequest(
    std::span<const RenderSceneRuntimeCameraRecord> cameras,
    std::uint32_t active_camera_id=CAMERA_ID) {
    RenderSceneCameraBindingRequest request{};
    request.frame_id = FRAME_ID;
    request.active_camera_id = active_camera_id;
    request.cameras = cameras;
    request.capture_byte_budget = CAPTURE_BUDGET;
    request.capture_requested = true;
    return request;
}

RhiVertexBufferView VertexBufferView() {
    RhiVertexBufferView view{};
    view.buffer = RhiBufferHandle{1U, 1U};
    view.stride_bytes = VERTEX_STRIDE_BYTES;
    view.size_bytes = VERTEX_BUFFER_BYTES;
    return view;
}

RhiIndexBufferView IndexBufferView() {
    RhiIndexBufferView view{};
    view.buffer = RhiBufferHandle{2U, 1U};
    view.size_bytes = INDEX_BUFFER_BYTES;
    view.format = RhiIndexFormat::Uint16;
    return view;
}

RenderScenePrimitiveGeometryRequest GeometryRequest(RenderScenePrimitiveGeometryKind kind) {
    RenderScenePrimitiveGeometryRequest request{};
    request.geometry_asset = AssetHandle{3U, 1U};
    request.kind = kind;
    request.segment_count = 16U;
    request.draw_id = DRAW_ID;
    request.pass_id = PASS_ID;
    request.material_id = MATERIAL_ID;
    request.vertex_buffer = VertexBufferView();
    request.index_buffer = IndexBufferView();
    return request;
}

AssetHandle MakeAsset(std::uint32_t slot) {
    return AssetHandle{slot, 1U};
}

RhiPipelineHandle MakePipelineHandle() {
    return RhiPipelineHandle{4U, 1U};
}

RenderSceneRuntimeMaterialTextureSlot MakeMaterialTextureSlot(std::uint32_t slot) {
    RenderSceneRuntimeMaterialTextureSlot texture_slot{};
    texture_slot.slot = slot;
    texture_slot.texture_asset = MakeAsset(TEXTURE_ASSET_SLOT + slot);
    texture_slot.sampled_texture.texture = TextureHandleForSlot(slot);
    texture_slot.sampled_texture.slot = slot;
    texture_slot.sampler.sampler = SamplerHandleForSlot(slot);
    texture_slot.sampler.slot = slot;
    return texture_slot;
}

RenderSceneRuntimeMaterialRequest MakeMaterialRequest(
    std::span<const RenderSceneRuntimeMaterialTextureSlot> slots) {
    RenderSceneRuntimeMaterialRequest request{};
    request.material_asset = MakeAsset(MATERIAL_ASSET_SLOT);
    request.material_id = MATERIAL_ID;
    request.pipeline = MakePipelineHandle();
    request.texture_slots = slots;
    return request;
}

RenderSceneCameraBindingResult MakeCameraBinding() {
    const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{CameraRecord()};
    RenderSceneCameraBindingResult result{};
    RenderSceneCameraFrameBinder binder;
    binder.BuildActiveCameraFrame(CameraRequest(cameras), &result);
    return result;
}

RenderScenePrimitiveGeometryRecord MakePrimitiveGeometryRecord(RenderScenePrimitiveGeometryKind kind) {
    RenderScenePrimitiveGeometryBuilder builder;
    RenderScenePrimitiveGeometryRecord record{};
    builder.Build(GeometryRequest(kind), &record);
    return record;
}

RenderSceneRuntimeMaterialRecord MakeRuntimeMaterialRecord() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    builder.Build(MakeMaterialRequest(slots), &record);
    return record;
}

WorldTransformState MakeTransform(float x, float y, float z) {
    WorldTransformState transform{};
    transform.translation_x = x;
    transform.translation_y = y;
    transform.translation_z = z;
    return transform;
}

RenderSceneRuntimeFrameEntityRequest MakeRuntimeFrameEntity(
    std::uint32_t world_object_id,
    const WorldTransformState &transform,
    RenderScenePrimitiveGeometryKind kind) {
    RenderSceneRuntimeFrameEntityRequest entity{};
    entity.world_object_id = WorldObjectId{world_object_id};
    entity.transform = transform;
    entity.geometry = MakePrimitiveGeometryRecord(kind);
    entity.is_visible = true;
    entity.is_active = true;
    return entity;
}

RenderSceneRuntimeFrameRequest MakeRuntimeFrameRequest(
    const RenderSceneCameraBindingResult &camera,
    const RenderSceneRuntimeMaterialRecord &material,
    std::span<const RenderSceneRuntimeFrameEntityRequest> entities) {
    RenderSceneRuntimeFrameRequest request{};
    request.frame_id = FRAME_ID;
    request.camera = camera;
    request.material = material;
    request.entities = entities;
    return request;
}

RenderSceneOneCubeCaptureRequest MakeOneCubeCaptureRequest(
    L1Vis001RhiDevice &device,
    std::vector<std::uint8_t> &capture) {
    RenderSceneOneCubeCaptureRequest request{};
    request.frame_id = FRAME_ID;
    request.camera = MakeCameraBinding();
    request.cube_geometry = MakePrimitiveGeometryRecord(RenderScenePrimitiveGeometryKind::Cube);
    request.material = MakeRuntimeMaterialRecord();
    request.world_object_id = WorldObjectId{501U};
    request.transform = MakeTransform(0.0F, 0.0F, 0.0F);
    request.rhi_device = &device;
    request.output_path = L1_VIS_001_OUTPUT_PATH;
    request.output_path_byte_count = sizeof(L1_VIS_001_OUTPUT_PATH) - 1U;
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget = capture.size();
    return request;
}

std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
MakeThreePrimitiveEntities() {
    std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> entities{};

    entities[0U].world_object_id = WorldObjectId{601U};
    entities[0U].object_name = L1_VIS_002_CUBE_NAME;
    entities[0U].object_name_byte_count = sizeof(L1_VIS_002_CUBE_NAME) - 1U;
    entities[0U].transform = MakeTransform(-2.0F, 0.0F, 0.0F);
    entities[0U].geometry = MakePrimitiveGeometryRecord(RenderScenePrimitiveGeometryKind::Cube);

    entities[1U].world_object_id = WorldObjectId{602U};
    entities[1U].object_name = L1_VIS_002_CYLINDER_NAME;
    entities[1U].object_name_byte_count = sizeof(L1_VIS_002_CYLINDER_NAME) - 1U;
    entities[1U].transform = MakeTransform(0.0F, 1.0F, 0.0F);
    entities[1U].geometry = MakePrimitiveGeometryRecord(RenderScenePrimitiveGeometryKind::Cylinder);

    entities[2U].world_object_id = WorldObjectId{603U};
    entities[2U].object_name = L1_VIS_002_CONE_NAME;
    entities[2U].object_name_byte_count = sizeof(L1_VIS_002_CONE_NAME) - 1U;
    entities[2U].transform = MakeTransform(2.0F, 0.0F, 1.0F);
    entities[2U].geometry = MakePrimitiveGeometryRecord(RenderScenePrimitiveGeometryKind::Cone);

    return entities;
}

RenderSceneThreePrimitiveCaptureRequest MakeThreePrimitiveCaptureRequest(
    L1Vis001RhiDevice &device,
    std::vector<std::uint8_t> &capture,
    std::span<const RenderSceneThreePrimitiveEntityRequest> entities) {
    RenderSceneThreePrimitiveCaptureRequest request{};
    request.frame_id = FRAME_ID;
    request.camera = MakeCameraBinding();
    request.material = MakeRuntimeMaterialRecord();
    request.entities = entities;
    request.rhi_device = &device;
    request.output_path = L1_VIS_002_OUTPUT_PATH;
    request.output_path_byte_count = sizeof(L1_VIS_002_OUTPUT_PATH) - 1U;
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget_per_entity = L1VisCaptureByteCount();
    return request;
}

RenderSceneThreePrimitiveCaptureRequest MakeThreeTextureMaterialCaptureRequest(
    L1Vis001RhiDevice &device,
    std::vector<std::uint8_t> &capture,
    std::span<const RenderSceneThreePrimitiveEntityRequest> entities) {
    RenderSceneThreePrimitiveCaptureRequest request =
        MakeThreePrimitiveCaptureRequest(device, capture, entities);
    request.output_path = L1_VIS_003_OUTPUT_PATH;
    request.output_path_byte_count = sizeof(L1_VIS_003_OUTPUT_PATH) - 1U;
    return request;
}

bool CaptureWasWritten(const std::vector<std::uint8_t> &capture) {
    for (std::uint8_t value : capture) {
        if (value != CAPTURE_SENTINEL) {
            return true;
        }
    }

    return false;
}

bool CaptureSegmentWasWritten(const std::vector<std::uint8_t> &capture, std::size_t segment_index) {
    const std::size_t segment_byte_count = L1VisCaptureByteCount();
    const std::size_t begin_index = segment_index * segment_byte_count;
    const std::size_t end_index = begin_index + segment_byte_count;
    if (end_index > capture.size()) {
        return false;
    }

    for (std::size_t index = begin_index; index < end_index; ++index) {
        if (capture[index] != CAPTURE_SENTINEL) {
            return true;
        }
    }

    return false;
}

int RenderSceneRuntimeCameraRecordBuildsDeterministicFrame() {
    const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{CameraRecord()};
    RenderSceneCameraBindingResult result{};
    RenderSceneCameraFrameBinder binder;
    const RenderSceneStatus status = binder.BuildActiveCameraFrame(CameraRequest(cameras), &result);
    if (status != RenderSceneStatus::Success) {
        return Fail("runtime camera binding failed");
    }

    if (result.camera.camera_id != CAMERA_ID || !result.camera.is_active) {
        return Fail("runtime camera binding did not select active camera");
    }

    if (!Approx(result.camera.frame.view.values[14U], 5.0F)) {
        return Fail("runtime camera view matrix was not deterministic");
    }

    if (!Approx(result.camera.frame.projection.values[0U], 1.0F)) {
        return Fail("runtime camera projection was not deterministic");
    }

    return 0;
}

int RenderSceneRuntimeCameraActiveBindingRejectsMissingCamera() {
    const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{CameraRecord()};
    RenderSceneCameraBindingResult result{};
    result.camera.camera_id = 77U;

    RenderSceneCameraFrameBinder binder;
    const RenderSceneStatus status = binder.BuildActiveCameraFrame(CameraRequest(cameras, CAMERA_ID + 1U), &result);
    if (status != RenderSceneStatus::MissingCamera) {
        return Fail("runtime camera binding did not report missing camera");
    }

    if (result.camera.camera_id != 0U) {
        return Fail("runtime camera binding leaked stale output on missing camera");
    }

    return 0;
}

int RenderSceneRuntimeCameraCaptureMetadataRecordsFrameAndTarget() {
    const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{CameraRecord()};
    RenderSceneCameraBindingResult result{};

    RenderSceneCameraFrameBinder binder;
    const RenderSceneStatus status = binder.BuildActiveCameraFrame(CameraRequest(cameras), &result);
    if (status != RenderSceneStatus::Success) {
        return Fail("runtime camera capture setup failed");
    }

    if (!result.capture.capture_requested) {
        return Fail("runtime camera capture flag was not recorded");
    }

    if (result.capture.frame_id != FRAME_ID || result.capture.camera_id != CAMERA_ID) {
        return Fail("runtime camera capture identity metadata mismatch");
    }

    if (result.capture.target.generation != 1U) {
        return Fail("runtime camera capture target metadata mismatch");
    }

    if (result.capture.output_byte_budget != CAPTURE_BUDGET) {
        return Fail("runtime camera capture budget metadata mismatch");
    }

    return 0;
}

int RenderScenePrimitiveGeometryBuildsCubeCylinderConeRanges() {
    RenderScenePrimitiveGeometryBuilder builder;
    RenderScenePrimitiveGeometryRecord cube{};
    RenderScenePrimitiveGeometryRecord cylinder{};
    RenderScenePrimitiveGeometryRecord cone{};

    if (builder.Build(GeometryRequest(RenderScenePrimitiveGeometryKind::Cube), &cube) !=
        RenderScenePrimitiveGeometryStatus::Success) {
        return Fail("cube geometry record failed");
    }

    if (builder.Build(GeometryRequest(RenderScenePrimitiveGeometryKind::Cylinder), &cylinder) !=
        RenderScenePrimitiveGeometryStatus::Success) {
        return Fail("cylinder geometry record failed");
    }

    if (builder.Build(GeometryRequest(RenderScenePrimitiveGeometryKind::Cone), &cone) !=
        RenderScenePrimitiveGeometryStatus::Success) {
        return Fail("cone geometry record failed");
    }

    if (cube.vertex_count != 24U || cube.index_count != 36U) {
        return Fail("cube geometry bounds mismatch");
    }

    if (cylinder.vertex_count != 34U || cylinder.index_count != 192U) {
        return Fail("cylinder geometry bounds mismatch");
    }

    if (cone.vertex_count != 18U || cone.index_count != 96U) {
        return Fail("cone geometry bounds mismatch");
    }

    if (cube.draw.draw.topology != RhiPrimitiveTopology::TriangleList) {
        return Fail("primitive geometry topology mismatch");
    }

    if (builder.Validate(cube) != RenderScenePrimitiveGeometryStatus::Success) {
        return Fail("cube geometry validation failed");
    }

    return 0;
}

int RenderScenePrimitiveGeometryMissingRecordReportsStatus() {
    RenderScenePrimitiveGeometryBuilder builder;
    RenderScenePrimitiveGeometryRecord record{};
    const RenderScenePrimitiveGeometryStatus status = builder.Validate(record);
    if (status != RenderScenePrimitiveGeometryStatus::MissingGeometryRecord) {
        return Fail("primitive geometry did not report missing record");
    }

    return 0;
}

int RenderScenePrimitiveGeometryRejectsSmallBufferRanges() {
    RenderScenePrimitiveGeometryBuilder builder;
    RenderScenePrimitiveGeometryRequest request = GeometryRequest(RenderScenePrimitiveGeometryKind::Cylinder);
    request.index_buffer.size_bytes = sizeof(std::uint16_t) * 16U;
    RenderScenePrimitiveGeometryRecord record{};
    const RenderScenePrimitiveGeometryStatus status = builder.Build(request, &record);
    if (status != RenderScenePrimitiveGeometryStatus::InvalidDrawRecord) {
        return Fail("primitive geometry accepted undersized index buffer");
    }

    return 0;
}

int RenderSceneRuntimeMaterialBindsThreeTextureSlots() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(2U),
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U)};
    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(MakeMaterialRequest(slots), &record);
    if (status != RenderSceneRuntimeMaterialStatus::Success) {
        return Fail("runtime material three slot build failed");
    }

    if (record.material_id != MATERIAL_ID || record.texture_slot_count != 3U) {
        return Fail("runtime material identity or slot count mismatch");
    }

    if (record.texture_slots[0U].slot != 0U || record.texture_slots[1U].slot != 1U) {
        return Fail("runtime material slots were not sorted");
    }

    if (record.texture_slots[2U].sampled_texture.slot != 2U) {
        return Fail("runtime material sampled texture slot mismatch");
    }

    if (record.texture_slots[2U].sampler.slot != 2U) {
        return Fail("runtime material sampler slot mismatch");
    }

    if (builder.Validate(record) != RenderSceneRuntimeMaterialStatus::Success) {
        return Fail("runtime material validation failed");
    }

    return 0;
}

int RenderSceneRuntimeMaterialRejectsMissingThirdSlot() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 2U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U)};
    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(MakeMaterialRequest(slots), &record);
    if (status != RenderSceneRuntimeMaterialStatus::MissingTextureSlot) {
        return Fail("runtime material did not report missing third slot");
    }

    return 0;
}

int RenderSceneRuntimeMaterialReportsInvalidTextureAsset() {
    std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    slots[1U].texture_asset = AssetHandle{};

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(MakeMaterialRequest(slots), &record);
    if (status != RenderSceneRuntimeMaterialStatus::InvalidTextureAsset) {
        return Fail("runtime material did not report invalid texture asset");
    }

    return 0;
}

int RenderSceneRuntimeMaterialReportsInvalidTextureBinding() {
    std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    slots[0U].sampled_texture.texture.generation = 0U;

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(MakeMaterialRequest(slots), &record);
    if (status != RenderSceneRuntimeMaterialStatus::InvalidTextureBinding) {
        return Fail("runtime material did not report invalid texture binding");
    }

    return 0;
}

int RenderSceneRuntimeMaterialReportsInvalidSamplerBinding() {
    std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    slots[2U].sampler.sampler.generation = 0U;

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(MakeMaterialRequest(slots), &record);
    if (status != RenderSceneRuntimeMaterialStatus::InvalidSamplerBinding) {
        return Fail("runtime material did not report invalid sampler binding");
    }

    return 0;
}

int RenderSceneRuntimeMaterialReportsInvalidPipeline() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    RenderSceneRuntimeMaterialRequest request = MakeMaterialRequest(slots);
    request.pipeline.generation = 0U;

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(request, &record);
    if (status != RenderSceneRuntimeMaterialStatus::InvalidPipeline) {
        return Fail("runtime material did not report invalid pipeline");
    }

    return 0;
}

int RenderSceneRuntimeFrameSubmitsThreeEntitiesWithSharedMaterial() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const RenderSceneRuntimeMaterialRecord material = MakeRuntimeMaterialRecord();
    const std::array<RenderSceneRuntimeFrameEntityRequest, 3U> entities{
        MakeRuntimeFrameEntity(101U, MakeTransform(-2.0F, 0.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cube),
        MakeRuntimeFrameEntity(102U, MakeTransform(0.0F, 1.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cylinder),
        MakeRuntimeFrameEntity(103U, MakeTransform(2.0F, 0.0F, 1.0F), RenderScenePrimitiveGeometryKind::Cone)};
    std::array<RenderSceneRuntimeFrameDrawRecord, 3U> draws{};
    RenderSceneRuntimeFrameResult result{};

    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status =
        builder.Build(MakeRuntimeFrameRequest(camera, material, entities), draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::Success) {
        return Fail("runtime frame three entity submission failed");
    }

    if (result.output_draw_count != 3U || result.material_id != MATERIAL_ID) {
        return Fail("runtime frame output count or material mismatch");
    }

    if (draws[0U].draw.material_id != MATERIAL_ID || draws[1U].draw.material_id != MATERIAL_ID) {
        return Fail("runtime frame did not share material across draws");
    }

    if (draws[2U].geometry_kind != RenderScenePrimitiveGeometryKind::Cone) {
        return Fail("runtime frame geometry kind mismatch");
    }

    if (draws[0U].transform.translation_x == draws[1U].transform.translation_x) {
        return Fail("runtime frame transforms were not distinct");
    }

    if (draws[1U].draw.draw.index_count != 192U) {
        return Fail("runtime frame cylinder draw range mismatch");
    }

    return 0;
}

int RenderSceneRuntimeFrameRejectsDuplicateTransforms() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const RenderSceneRuntimeMaterialRecord material = MakeRuntimeMaterialRecord();
    const WorldTransformState transform = MakeTransform(1.0F, 0.0F, 0.0F);
    const std::array<RenderSceneRuntimeFrameEntityRequest, 2U> entities{
        MakeRuntimeFrameEntity(101U, transform, RenderScenePrimitiveGeometryKind::Cube),
        MakeRuntimeFrameEntity(102U, transform, RenderScenePrimitiveGeometryKind::Cylinder)};
    std::array<RenderSceneRuntimeFrameDrawRecord, 2U> draws{};
    RenderSceneRuntimeFrameResult result{};

    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status =
        builder.Build(MakeRuntimeFrameRequest(camera, material, entities), draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::DuplicateTransform) {
        return Fail("runtime frame did not report duplicate transform");
    }

    return 0;
}

int RenderSceneRuntimeFrameRejectsSmallOutputCapacity() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const RenderSceneRuntimeMaterialRecord material = MakeRuntimeMaterialRecord();
    const std::array<RenderSceneRuntimeFrameEntityRequest, 3U> entities{
        MakeRuntimeFrameEntity(101U, MakeTransform(-2.0F, 0.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cube),
        MakeRuntimeFrameEntity(102U, MakeTransform(0.0F, 1.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cylinder),
        MakeRuntimeFrameEntity(103U, MakeTransform(2.0F, 0.0F, 1.0F), RenderScenePrimitiveGeometryKind::Cone)};
    std::array<RenderSceneRuntimeFrameDrawRecord, 2U> draws{};
    RenderSceneRuntimeFrameResult result{};

    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status =
        builder.Build(MakeRuntimeFrameRequest(camera, material, entities), draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::OutputCapacityExceeded) {
        return Fail("runtime frame did not report output capacity");
    }

    return 0;
}

int RenderSceneRuntimeFrameReportsMissingMaterial() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const RenderSceneRuntimeMaterialRecord material{};
    const std::array<RenderSceneRuntimeFrameEntityRequest, 1U> entities{
        MakeRuntimeFrameEntity(101U, MakeTransform(0.0F, 0.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cube)};
    std::array<RenderSceneRuntimeFrameDrawRecord, 1U> draws{};
    RenderSceneRuntimeFrameResult result{};

    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status =
        builder.Build(MakeRuntimeFrameRequest(camera, material, entities), draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::MissingMaterialRecord) {
        return Fail("runtime frame did not report missing material");
    }

    return 0;
}

int RenderSceneRuntimeFrameReportsMissingGeometry() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const RenderSceneRuntimeMaterialRecord material = MakeRuntimeMaterialRecord();
    std::array<RenderSceneRuntimeFrameEntityRequest, 1U> entities{
        MakeRuntimeFrameEntity(101U, MakeTransform(0.0F, 0.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cube)};
    entities[0U].geometry = RenderScenePrimitiveGeometryRecord{};
    std::array<RenderSceneRuntimeFrameDrawRecord, 1U> draws{};
    RenderSceneRuntimeFrameResult result{};

    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status =
        builder.Build(MakeRuntimeFrameRequest(camera, material, entities), draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::MissingGeometryRecord) {
        return Fail("runtime frame did not report missing geometry");
    }

    return 0;
}

int RenderSceneL1Vis001CapturesStaticCubeThroughRuntimeRoute() {
    L1Vis001RhiDevice device;
    std::vector<std::uint8_t> capture(L1VisCaptureByteCount(), CAPTURE_SENTINEL);
    RenderSceneOneCubeCaptureRoute route;
    RenderSceneOneCubeCaptureResult result{};

    const RenderSceneOneCubeCaptureStatus status =
        route.Execute(MakeOneCubeCaptureRequest(device, capture), &result);
    if (status != RenderSceneOneCubeCaptureStatus::Success) {
        return Fail("l1 vis one cube route did not complete");
    }

    if (result.first_missing_layer != RenderSceneOneCubeCaptureMissingLayer::None) {
        return Fail("l1 vis one cube route reported a missing layer on success");
    }

    if (result.output_status != RenderSceneOneCubeCaptureOutputStatus::CaptureAvailable) {
        return Fail("l1 vis one cube route did not report capture availability");
    }

    if (result.capture.frame_id != FRAME_ID || result.capture.camera_id != CAMERA_ID) {
        return Fail("l1 vis one cube route capture metadata mismatch");
    }

    if (result.frame_result.output_draw_count != 1U || result.frame_result.submitted_entity_count != 1U) {
        return Fail("l1 vis one cube route did not submit exactly one entity");
    }

    if (result.draw_record.geometry_kind != RenderScenePrimitiveGeometryKind::Cube) {
        return Fail("l1 vis one cube route did not submit cube geometry");
    }

    if (result.draw_record.draw.draw.index_count != 36U) {
        return Fail("l1 vis one cube route cube draw range mismatch");
    }

    if (result.render_result.status != RenderDrawableFramePipelineStatus::Success) {
        return Fail("l1 vis one cube route did not execute rendercore pipeline");
    }

    if (result.capture_bytes_written != capture.size()) {
        return Fail("l1 vis one cube route capture byte count mismatch");
    }

    if (!CaptureWasWritten(capture)) {
        return Fail("l1 vis one cube route did not write capture bytes");
    }

    if (result.output_path_byte_count != sizeof(L1_VIS_001_OUTPUT_PATH) - 1U) {
        return Fail("l1 vis one cube route output path metadata mismatch");
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    if (snapshot.submitted_indexed_draw_count != 1U ||
        snapshot.submitted_sampled_texture_bind_count != 1U ||
        snapshot.submitted_sampler_bind_count != 1U ||
        snapshot.capture_count != 1U) {
        return Fail("l1 vis one cube route did not drive rhi draw capture counters");
    }

    return 0;
}

int RenderSceneL1Vis001ReportsBlockedEnvForMissingSwapchain() {
    L1Vis001RhiDevice device;
    device.SetSwapchainValid(false);
    std::vector<std::uint8_t> capture(L1VisCaptureByteCount(), CAPTURE_SENTINEL);
    RenderSceneOneCubeCaptureRoute route;
    RenderSceneOneCubeCaptureResult result{};

    const RenderSceneOneCubeCaptureStatus status =
        route.Execute(MakeOneCubeCaptureRequest(device, capture), &result);
    if (status != RenderSceneOneCubeCaptureStatus::BlockedByEnv) {
        return Fail("l1 vis one cube route did not report env block for missing swapchain");
    }

    if (result.first_missing_layer != RenderSceneOneCubeCaptureMissingLayer::RhiCaptureTarget) {
        return Fail("l1 vis one cube route reported wrong env missing layer");
    }

    if (result.output_status != RenderSceneOneCubeCaptureOutputStatus::BlockedByEnv) {
        return Fail("l1 vis one cube route reported wrong output status for env block");
    }

    if (result.render_result.status != RenderDrawableFramePipelineStatus::InvalidSwapchain) {
        return Fail("l1 vis one cube route did not expose rendercore swapchain status");
    }

    if (CaptureWasWritten(capture)) {
        return Fail("l1 vis one cube route wrote capture on env block");
    }

    return 0;
}

int RenderSceneL1Vis001ReportsShaderPipelineMissingLayer() {
    L1Vis001RhiDevice device;
    std::vector<std::uint8_t> capture(L1VisCaptureByteCount(), CAPTURE_SENTINEL);
    RenderSceneOneCubeCaptureRequest request = MakeOneCubeCaptureRequest(device, capture);
    request.material.pipeline = RhiPipelineHandle{};
    RenderSceneOneCubeCaptureRoute route;
    RenderSceneOneCubeCaptureResult result{};

    const RenderSceneOneCubeCaptureStatus status = route.Execute(request, &result);
    if (status != RenderSceneOneCubeCaptureStatus::Fail) {
        return Fail("l1 vis one cube route did not fail on missing shader pipeline");
    }

    if (result.first_missing_layer != RenderSceneOneCubeCaptureMissingLayer::ShaderPipeline) {
        return Fail("l1 vis one cube route reported wrong semantic missing layer");
    }

    if (result.output_status == RenderSceneOneCubeCaptureOutputStatus::BlockedByEnv) {
        return Fail("l1 vis one cube route hid semantic failure as env block");
    }

    if (device.Snapshot().submit_count != 0U || CaptureWasWritten(capture)) {
        return Fail("l1 vis one cube route mutated rhi on semantic failure");
    }

    return 0;
}

int RenderSceneL1Vis002CapturesThreePrimitivePlacedSceneThroughRuntimeRoute() {
    L1Vis001RhiDevice device;
    std::vector<std::uint8_t> capture(
        L1VisCaptureByteCount() * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT,
        CAPTURE_SENTINEL);
    const std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        entities = MakeThreePrimitiveEntities();
    RenderSceneThreePrimitiveCaptureRoute route;
    RenderSceneThreePrimitiveCaptureResult result{};

    const RenderSceneThreePrimitiveCaptureRequest request =
        MakeThreePrimitiveCaptureRequest(device, capture, entities);
    const RenderSceneThreePrimitiveCaptureStatus status = route.Execute(request, &result);
    if (status != RenderSceneThreePrimitiveCaptureStatus::Success) {
        return Fail("l1 vis three primitive route did not complete");
    }

    if (result.first_missing_layer != RenderSceneThreePrimitiveCaptureMissingLayer::None) {
        return Fail("l1 vis three primitive route reported a missing layer on success");
    }

    if (result.output_status != RenderSceneThreePrimitiveCaptureOutputStatus::CaptureAvailable) {
        return Fail("l1 vis three primitive route did not report capture availability");
    }

    if (result.frame_result.output_draw_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT ||
        result.frame_result.submitted_entity_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return Fail("l1 vis three primitive route did not submit exactly three entities");
    }

    if (result.entity_report_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT ||
        result.render_result_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return Fail("l1 vis three primitive route report counts mismatch");
    }

    if (result.entity_reports[0U].world_object_id.value != 601U ||
        result.entity_reports[1U].world_object_id.value != 602U ||
        result.entity_reports[2U].world_object_id.value != 603U) {
        return Fail("l1 vis three primitive route object ids mismatch");
    }

    if (std::string_view(result.entity_reports[0U].object_name, result.entity_reports[0U].object_name_byte_count) !=
        L1_VIS_002_CUBE_NAME) {
        return Fail("l1 vis three primitive cube name mismatch");
    }

    if (result.entity_reports[0U].primitive_kind != RenderScenePrimitiveGeometryKind::Cube ||
        result.entity_reports[1U].primitive_kind != RenderScenePrimitiveGeometryKind::Cylinder ||
        result.entity_reports[2U].primitive_kind != RenderScenePrimitiveGeometryKind::Cone) {
        return Fail("l1 vis three primitive geometry kind mismatch");
    }

    if (!Approx(result.entity_reports[0U].transform.translation_x, -2.0F) ||
        !Approx(result.entity_reports[1U].transform.translation_y, 1.0F) ||
        !Approx(result.entity_reports[2U].transform.translation_z, 1.0F)) {
        return Fail("l1 vis three primitive fixed transforms mismatch");
    }

    if (result.entity_reports[0U].draw_record.draw.draw.index_count != 36U ||
        result.entity_reports[1U].draw_record.draw.draw.index_count != 192U ||
        result.entity_reports[2U].draw_record.draw.draw.index_count != 96U) {
        return Fail("l1 vis three primitive draw ranges mismatch");
    }

    if (result.capture_bytes_written != capture.size()) {
        return Fail("l1 vis three primitive capture byte count mismatch");
    }

    for (std::size_t index = 0U; index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT; ++index) {
        if (result.render_results[index].status != RenderDrawableFramePipelineStatus::Success) {
            return Fail("l1 vis three primitive rendercore result mismatch");
        }

        if (!CaptureSegmentWasWritten(capture, index)) {
            return Fail("l1 vis three primitive capture segment was not written");
        }
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    const std::size_t expected_binding_count =
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT * result.material_texture_slot_report_count;
    if (snapshot.submitted_indexed_draw_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT ||
        snapshot.submitted_sampled_texture_bind_count != expected_binding_count ||
        snapshot.submitted_sampler_bind_count != expected_binding_count ||
        snapshot.capture_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return Fail("l1 vis three primitive route did not drive rhi counters");
    }

    return 0;
}

int RenderSceneL1Vis003CapturesSharedThreeTextureMaterialSceneThroughRuntimeRoute() {
    L1Vis001RhiDevice device;
    std::vector<std::uint8_t> capture(
        L1VisCaptureByteCount() * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT,
        CAPTURE_SENTINEL);
    const std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        entities = MakeThreePrimitiveEntities();
    RenderSceneThreePrimitiveCaptureRoute route;
    RenderSceneThreePrimitiveCaptureResult result{};

    const RenderSceneThreePrimitiveCaptureRequest request =
        MakeThreeTextureMaterialCaptureRequest(device, capture, entities);
    const RenderSceneThreePrimitiveCaptureStatus status = route.Execute(request, &result);
    if (status != RenderSceneThreePrimitiveCaptureStatus::Success) {
        return Fail("l1 vis shared three texture material route did not complete");
    }

    if (result.shared_material_id != MATERIAL_ID ||
        result.frame_result.material_id != MATERIAL_ID) {
        return Fail("l1 vis shared material id mismatch");
    }

    if (result.material_texture_slot_report_count != 3U ||
        result.frame_result.material_texture_slot_count != 3U) {
        return Fail("l1 vis shared material did not report three texture slots");
    }

    for (std::size_t index = 0U; index < result.material_texture_slot_report_count; ++index) {
        const auto &slot_report = result.material_texture_slot_reports[index];
        const std::uint32_t slot = static_cast<std::uint32_t>(index);
        if (slot_report.material_id != MATERIAL_ID || slot_report.slot != slot) {
            return Fail("l1 vis shared material slot identity mismatch");
        }

        if (slot_report.texture_asset.slot != TEXTURE_ASSET_SLOT + slot ||
            slot_report.texture_asset.generation != 1U) {
            return Fail("l1 vis shared material texture resource mismatch");
        }

        if (!TextureHandlesMatch(slot_report.sampled_texture.texture, TextureHandleForSlot(slot)) ||
            slot_report.sampled_texture.slot != slot) {
            return Fail("l1 vis shared material sampled texture binding mismatch");
        }

        if (!SamplerHandlesMatch(slot_report.sampler.sampler, SamplerHandleForSlot(slot)) ||
            slot_report.sampler.slot != slot) {
            return Fail("l1 vis shared material sampler binding mismatch");
        }

        if (!slot_report.texture_resource_resolved ||
            !slot_report.sampled_texture_bound ||
            !slot_report.sampler_bound) {
            return Fail("l1 vis shared material binding status mismatch");
        }
    }

    if (result.material_texture_slot_reports[0U].texture_asset.slot ==
        result.material_texture_slot_reports[1U].texture_asset.slot) {
        return Fail("l1 vis shared material texture resources were not distinct");
    }

    if (result.material_texture_slot_reports[1U].texture_asset.slot ==
        result.material_texture_slot_reports[2U].texture_asset.slot) {
        return Fail("l1 vis shared material texture resources were not distinct");
    }

    for (std::size_t index = 0U; index < result.entity_report_count; ++index) {
        if (result.entity_reports[index].material_id != MATERIAL_ID) {
            return Fail("l1 vis entity did not use shared material id");
        }

        if (result.entity_reports[index].draw_record.draw.material_id != MATERIAL_ID) {
            return Fail("l1 vis entity draw record did not use shared material id");
        }

        if (result.render_results[index].material_id != MATERIAL_ID) {
            return Fail("l1 vis render result did not use shared material id");
        }

        if (result.render_results[index].pass_result.recorded_command_count != 13U) {
            return Fail("l1 vis shared material draw did not bind three texture and sampler slots");
        }
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    const std::size_t expected_binding_count =
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT * result.material_texture_slot_report_count;
    if (snapshot.submitted_sampled_texture_bind_count != expected_binding_count ||
        snapshot.submitted_sampler_bind_count != expected_binding_count ||
        snapshot.submitted_indexed_draw_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return Fail("l1 vis shared material rhi binding counters mismatch");
    }

    if (result.output_path_byte_count != sizeof(L1_VIS_003_OUTPUT_PATH) - 1U) {
        return Fail("l1 vis shared material output path metadata mismatch");
    }

    if (result.capture_bytes_written != capture.size()) {
        return Fail("l1 vis shared material capture byte count mismatch");
    }

    return 0;
}

int RenderSceneL1Vis002ReportsGeometryMissingLayerForCylinder() {
    L1Vis001RhiDevice device;
    std::vector<std::uint8_t> capture(
        L1VisCaptureByteCount() * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT,
        CAPTURE_SENTINEL);
    std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        entities = MakeThreePrimitiveEntities();
    entities[1U].geometry = RenderScenePrimitiveGeometryRecord{};

    RenderSceneThreePrimitiveCaptureRoute route;
    RenderSceneThreePrimitiveCaptureResult result{};
    const RenderSceneThreePrimitiveCaptureRequest request =
        MakeThreePrimitiveCaptureRequest(device, capture, entities);
    const RenderSceneThreePrimitiveCaptureStatus status = route.Execute(request, &result);
    if (status != RenderSceneThreePrimitiveCaptureStatus::Fail) {
        return Fail("l1 vis three primitive route did not fail on missing cylinder geometry");
    }

    if (result.first_missing_layer != RenderSceneThreePrimitiveCaptureMissingLayer::GeometryModel) {
        return Fail("l1 vis three primitive route reported wrong missing layer");
    }

    if (result.output_status == RenderSceneThreePrimitiveCaptureOutputStatus::BlockedByEnv) {
        return Fail("l1 vis three primitive route hid semantic failure as env block");
    }

    if (device.Snapshot().submit_count != 0U || CaptureWasWritten(capture)) {
        return Fail("l1 vis three primitive route mutated rhi on semantic failure");
    }

    return 0;
}

int RenderSceneRuntimeVisualFoundationNoEditorWebUiInputDependency() {
    RenderSceneCameraFrameBinder binder;
    RenderScenePrimitiveGeometryBuilder builder;
    RenderSceneRuntimeFrameBuilder frame_builder;
    RenderSceneRuntimeMaterialBuilder material_builder;
    RenderScenePrimitiveGeometryRecord record{};
    const RenderScenePrimitiveGeometryStatus status = builder.Validate(record);
    if (status != RenderScenePrimitiveGeometryStatus::MissingGeometryRecord) {
        return Fail("runtime visual boundary setup failed");
    }

    const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{CameraRecord()};
    RenderSceneCameraBindingResult result{};
    if (binder.BuildActiveCameraFrame(CameraRequest(cameras), &result) != RenderSceneStatus::Success) {
        return Fail("runtime visual boundary camera setup failed");
    }

    RenderSceneRuntimeMaterialRecord material_record{};
    if (material_builder.Validate(material_record) != RenderSceneRuntimeMaterialStatus::MissingMaterialRecord) {
        return Fail("runtime visual boundary material setup failed");
    }

    RenderSceneRuntimeFrameResult frame_result{};
    std::array<RenderSceneRuntimeFrameDrawRecord, 1U> draws{};
    RenderSceneRuntimeFrameRequest frame_request{};
    frame_request.frame_id = FRAME_ID;
    if (frame_builder.Build(frame_request, draws, &frame_result) != RenderSceneRuntimeFrameStatus::MissingCamera) {
        return Fail("runtime visual boundary frame setup failed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_CAMERA_FRAME) {
        return RenderSceneRuntimeCameraRecordBuildsDeterministicFrame();
    }

    if (name == TEST_CAMERA_MISSING) {
        return RenderSceneRuntimeCameraActiveBindingRejectsMissingCamera();
    }

    if (name == TEST_CAMERA_CAPTURE) {
        return RenderSceneRuntimeCameraCaptureMetadataRecordsFrameAndTarget();
    }

    if (name == TEST_GEOMETRY_RANGES) {
        return RenderScenePrimitiveGeometryBuildsCubeCylinderConeRanges();
    }

    if (name == TEST_GEOMETRY_MISSING) {
        return RenderScenePrimitiveGeometryMissingRecordReportsStatus();
    }

    if (name == TEST_GEOMETRY_SMALL_BUFFER) {
        return RenderScenePrimitiveGeometryRejectsSmallBufferRanges();
    }

    if (name == TEST_MATERIAL_THREE_SLOTS) {
        return RenderSceneRuntimeMaterialBindsThreeTextureSlots();
    }

    if (name == TEST_MATERIAL_MISSING_SLOT) {
        return RenderSceneRuntimeMaterialRejectsMissingThirdSlot();
    }

    if (name == TEST_MATERIAL_INVALID_TEXTURE) {
        return RenderSceneRuntimeMaterialReportsInvalidTextureAsset();
    }

    if (name == TEST_MATERIAL_INVALID_TEXTURE_BINDING) {
        return RenderSceneRuntimeMaterialReportsInvalidTextureBinding();
    }

    if (name == TEST_MATERIAL_INVALID_SAMPLER) {
        return RenderSceneRuntimeMaterialReportsInvalidSamplerBinding();
    }

    if (name == TEST_MATERIAL_INVALID_PIPELINE) {
        return RenderSceneRuntimeMaterialReportsInvalidPipeline();
    }

    if (name == TEST_FRAME_THREE_ENTITIES) {
        return RenderSceneRuntimeFrameSubmitsThreeEntitiesWithSharedMaterial();
    }

    if (name == TEST_FRAME_DUPLICATE_TRANSFORM) {
        return RenderSceneRuntimeFrameRejectsDuplicateTransforms();
    }

    if (name == TEST_FRAME_OUTPUT_CAPACITY) {
        return RenderSceneRuntimeFrameRejectsSmallOutputCapacity();
    }

    if (name == TEST_FRAME_MISSING_MATERIAL) {
        return RenderSceneRuntimeFrameReportsMissingMaterial();
    }

    if (name == TEST_FRAME_MISSING_GEOMETRY) {
        return RenderSceneRuntimeFrameReportsMissingGeometry();
    }

    if (name == TEST_L1_VIS_ONE_CUBE_CAPTURE) {
        return RenderSceneL1Vis001CapturesStaticCubeThroughRuntimeRoute();
    }

    if (name == TEST_L1_VIS_ENV_BLOCKED) {
        return RenderSceneL1Vis001ReportsBlockedEnvForMissingSwapchain();
    }

    if (name == TEST_L1_VIS_SHADER_MISSING) {
        return RenderSceneL1Vis001ReportsShaderPipelineMissingLayer();
    }

    if (name == TEST_L1_VIS_THREE_PRIMITIVE_CAPTURE) {
        return RenderSceneL1Vis002CapturesThreePrimitivePlacedSceneThroughRuntimeRoute();
    }

    if (name == TEST_L1_VIS_THREE_PRIMITIVE_GEOMETRY_MISSING) {
        return RenderSceneL1Vis002ReportsGeometryMissingLayerForCylinder();
    }

    if (name == TEST_L1_VIS_SHARED_THREE_TEXTURE_MATERIAL) {
        return RenderSceneL1Vis003CapturesSharedThreeTextureMaterialSceneThroughRuntimeRoute();
    }

    if (name == TEST_BOUNDARY) {
        return RenderSceneRuntimeVisualFoundationNoEditorWebUiInputDependency();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
