// 模块: Tests RenderScene
// 文件: Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <span>
#include <string_view>
#include <system_error>
#include <vector>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Kernel/RuntimeFrameContext.h"
#include "YuEngine/Kernel/RuntimeFrameMode.h"
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
#include "YuEngine/RenderScene/RenderSceneMissingLayerDiagnosticRoute.h"
#include "YuEngine/RenderScene/RenderSceneOrbitCaptureRoute.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeVisualSceneProofRoute.h"
#include "YuEngine/RenderScene/RenderSceneThreePrimitiveCaptureRoute.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiBlendStateDesc.h"
#include "YuEngine/Rhi/RhiBlendUtility.h"
#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiConstantBufferBinding.h"
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
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldObjectDesc.h"
#include "YuEngine/World/WorldRegistrationResult.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformBridgeDesc.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformState.h"
#include "YuEngine/World/WorldTransformStatus.h"

using yuengine::animation::AnimationRuntimeChannel;
using yuengine::animation::AnimationRuntimeClipRecord;
using yuengine::animation::AnimationRuntimeInterpolation;
using yuengine::animation::AnimationRuntimeKeyframeRecord;
using yuengine::animation::AnimationRuntimeSampledValue;
using yuengine::animation::AnimationRuntimeSampler;
using yuengine::animation::AnimationRuntimeSampleRequest;
using yuengine::animation::AnimationRuntimeSampleResult;
using yuengine::animation::AnimationRuntimeStatus;
using yuengine::animation::AnimationRuntimeTrackRecord;
using yuengine::animation::AnimationRuntimeTransformApplyRequest;
using yuengine::animation::AnimationRuntimeTransformApplyResult;
using yuengine::asset::AssetHandle;
using yuengine::kernel::RuntimeFrameContext;
using yuengine::kernel::RuntimeFrameMode;
using yuengine::rendercore::RenderCameraProjectionKind;
using yuengine::rendercore::RenderCameraPose;
using yuengine::rendercore::RenderCameraVector3;
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
using yuengine::renderscene::RenderSceneMissingLayerDiagnosticFault;
using yuengine::renderscene::RenderSceneMissingLayerDiagnosticLayer;
using yuengine::renderscene::RenderSceneMissingLayerDiagnosticRequest;
using yuengine::renderscene::RenderSceneMissingLayerDiagnosticResult;
using yuengine::renderscene::RenderSceneMissingLayerDiagnosticRoute;
using yuengine::renderscene::RenderSceneMissingLayerDiagnosticStatus;
using yuengine::renderscene::RenderSceneOrbitCaptureFrameReport;
using yuengine::renderscene::RenderSceneOrbitCaptureMissingLayer;
using yuengine::renderscene::RenderSceneOrbitCaptureRequest;
using yuengine::renderscene::RenderSceneOrbitCaptureResult;
using yuengine::renderscene::RenderSceneOrbitCaptureRoute;
using yuengine::renderscene::RenderSceneOrbitCaptureStatus;
using yuengine::renderscene::RenderSceneRuntimeVisualSceneProofEntityReport;
using yuengine::renderscene::RenderSceneRuntimeVisualSceneImageArtifactReport;
using yuengine::renderscene::RenderSceneRuntimeVisualSceneImageArtifactStatus;
using yuengine::renderscene::RenderSceneRuntimeVisualSceneProofRequest;
using yuengine::renderscene::RenderSceneRuntimeVisualSceneProofResult;
using yuengine::renderscene::RenderSceneRuntimeVisualSceneProofRoute;
using yuengine::renderscene::RenderSceneRuntimeVisualSceneProofStatus;
using yuengine::renderscene::RenderSceneRuntimeVisualSceneCameraTweenEase;
using yuengine::renderscene::RenderSceneRuntimeVisualSceneCameraTweenFrameReport;
using yuengine::renderscene::RenderSceneRuntimeVisualSceneCameraTweenKeyframe;
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
using yuengine::rhi::MAX_RHI_CONSTANT_BUFFER_SLOTS;
using yuengine::rhi::RhiBlendMode;
using yuengine::rhi::RhiBlendStateDesc;
using yuengine::rhi::RhiBackendKind;
using yuengine::rhi::RhiBufferDesc;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using yuengine::rhi::RhiCapabilities;
using yuengine::rhi::RhiCaptureResult;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiColorTargetDesc;
using yuengine::rhi::RhiCommandList;
using yuengine::rhi::RhiConstantBufferBinding;
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
using yuengine::rhi::RhiShaderStage;
using yuengine::rhi::RhiShaderModuleDesc;
using yuengine::rhi::RhiShaderModuleHandle;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::RhiSwapchainResizeRequest;
using yuengine::rhi::RhiSwapchainResizeResult;
using yuengine::rhi::RhiTextureDesc;
using yuengine::rhi::RhiTextureHandle;
using yuengine::rhi::RhiVertexBufferView;
using yuengine::rhi::RHI_CONSTANT_BUFFER_ALIGNMENT;
using yuengine::rhi::MAX_CAPTURE_FIXTURE_EXTENT;
using yuengine::rhi::MAX_COLOR_TARGET_EXTENT;
using yuengine::rhi::MAX_COMMANDS;
using yuengine::rhi::RGBA8_BYTES_PER_PIXEL;
using yuengine::rhi::BlendRhiColor;
using yuengine::world::WorldInstance;
using yuengine::world::WorldObjectDesc;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldRegistrationResult;
using yuengine::world::WorldTransformBridge;
using yuengine::world::WorldTransformBridgeDesc;
using yuengine::world::WorldTransformResult;
using yuengine::world::WorldTransformState;
using yuengine::world::WorldTransformStatus;

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
constexpr const char *TEST_MATERIAL_CONSTANTS =
    "RenderScene_RuntimeMaterialCopiesMaterialConstants";
constexpr const char *TEST_MATERIAL_CONSTANT_OVERFLOW =
    "RenderScene_RuntimeMaterialRejectsOversizedMaterialConstants";
constexpr const char *TEST_MATERIAL_BLEND_STATE =
    "RenderScene_RuntimeMaterialCopiesBlendState";
constexpr const char *TEST_MATERIAL_INVALID_BLEND_STATE =
    "RenderScene_RuntimeMaterialRejectsInvalidBlendStateWithoutMutation";
constexpr const char *TEST_FRAME_THREE_ENTITIES =
    "RenderScene_RuntimeFrameSubmitsThreeEntitiesWithSharedMaterial";
constexpr const char *TEST_FRAME_PER_ENTITY_MATERIALS =
    "RenderScene_RuntimeFrameSubmitsEntitiesWithPerEntityMaterials";
constexpr const char *TEST_FRAME_MATERIAL_INDEX_RANGE =
    "RenderScene_RuntimeFrameRejectsMaterialTableIndexOutOfRange";
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
constexpr const char *TEST_L1_VIS_ANIMATED_TRANSFORM =
    "RenderScene_L1Vis004CapturesAnimatedTransformSceneThroughRuntimeRoute";
constexpr const char *TEST_L1_VIS_ORBIT_CAPTURE =
    "RenderScene_L1Vis005CapturesDeterministicOrbitSequence";
constexpr const char *TEST_L1_VIS_MISSING_LAYER_DIAGNOSTIC =
    "RenderScene_L1Vis006ReportsExactMissingLayers";
constexpr const char *TEST_L1_SAMPLE_011_RUNTIME_VISUAL_SCENE =
    "RenderScene_L1Sample011CapturesFinalRuntimeVisualScene";
constexpr const char *TEST_L1_SAMPLE_011_RUNTIME_VISUAL_IMAGE_ARTIFACTS =
    "RenderScene_L1Sample011EmitsFinalOrbitImageArtifacts";
constexpr const char *TEST_L1_SAMPLE_012_RUNTIME_VISUAL_BLOCKER =
    "RenderScene_L1Sample012ReportsExactRuntimeVisualBlocker";
constexpr const char *TEST_L1_SAMPLE_013_USER_VISIBLE_RESOLUTION_BLOCKER =
    "RenderScene_L1Sample013ReportsUserVisibleCaptureResolutionBlocker";
constexpr const char *TEST_L1_SAMPLE_014_USER_VISIBLE_TARGET =
    "RenderScene_L1Sample014EmitsUserVisibleCaptureTargetArtifacts";
constexpr const char *TEST_L1_SAMPLE_015_SCENE_PIXEL_SEMANTICS =
    "RenderScene_L1Sample015EmitsScenePixelSemanticsArtifacts";
constexpr const char *TEST_L1_SAMPLE_016_PERSPECTIVE_3D_CAMERA_TWEEN =
    "RenderScene_L1Sample016EmitsPerspective3DPrimitiveCameraTweenArtifacts";
constexpr const char *TEST_L1_SAMPLE_018_TRANSPARENT_PANEL_BLEND =
    "RenderScene_L1Sample018BlendsTransparentRuntimePanel";
constexpr const char *TEST_L1_SAMPLE_019_TEXTURED_GLASS_EMISSIVE_METAL =
    "RenderScene_L1Sample019RendersTexturedGlassEmissiveMetalMaterials";
constexpr const char *TEST_BOUNDARY =
    "RenderScene_RuntimeVisualFoundationNoEditorUiInputDependency";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr char L1_VIS_001_OUTPUT_PATH[] = "Artifacts/L1Vis001/StaticOneCube.rvf";
constexpr char L1_VIS_002_OUTPUT_PATH[] = "Artifacts/L1Vis002/ThreePrimitivePlaced.rvf";
constexpr char L1_VIS_003_OUTPUT_PATH[] = "Artifacts/L1Vis003/SharedThreeTextureMaterial.rvf";
constexpr char L1_VIS_004_OUTPUT_PATH[] = "Artifacts/L1Vis004/AnimatedTransform.rvf";
constexpr char L1_VIS_005_OUTPUT_PATH_PREFIX[] = "Artifacts/L1Vis005/OrbitCapture";
constexpr char L1_SAMPLE_011_OUTPUT_PATH_PREFIX[] = "Artifacts/L1Sample011/RuntimeVisualScene";
constexpr char L1_SAMPLE_011_IMAGE_OUTPUT_PATH_PREFIX[] =
    "Artifacts/L1Sample011/RVF012/RuntimeVisualScene";
constexpr char L1_SAMPLE_011_IMAGE_ARTIFACT_DIRECTORY[] = "Artifacts/L1Sample011/RVF012";
constexpr char L1_SAMPLE_013_IMAGE_OUTPUT_PATH_PREFIX[] =
    "Artifacts/L1Sample011/RVF013/RuntimeVisualScene";
constexpr char L1_SAMPLE_013_IMAGE_ARTIFACT_DIRECTORY[] = "Artifacts/L1Sample011/RVF013";
constexpr char L1_SAMPLE_014_IMAGE_OUTPUT_PATH_PREFIX[] =
    "Artifacts/L1Sample011/RVF014/RuntimeVisualScene";
constexpr char L1_SAMPLE_014_IMAGE_ARTIFACT_DIRECTORY[] = "Artifacts/L1Sample011/RVF014";
constexpr char L1_SAMPLE_015_IMAGE_OUTPUT_PATH_PREFIX[] =
    "Artifacts/L1Sample011/RVF015/RuntimeVisualScene";
constexpr char L1_SAMPLE_015_IMAGE_ARTIFACT_DIRECTORY[] = "Artifacts/L1Sample011/RVF015";
constexpr char L1_SAMPLE_016_IMAGE_OUTPUT_PATH_PREFIX[] =
    "Artifacts/L1Sample011/RVF016/RuntimeVisualScene";
constexpr char L1_SAMPLE_016_IMAGE_ARTIFACT_DIRECTORY[] = "Artifacts/L1Sample011/RVF016";
constexpr char L1_SAMPLE_011_IMAGE_HEADER[] = "P6\n12 4\n255\n";
constexpr char L1_SAMPLE_014_IMAGE_HEADER[] = "P6\n640 360\n255\n";
constexpr char L1_VIS_002_CUBE_NAME[] = "Cube";
constexpr char L1_VIS_002_CYLINDER_NAME[] = "Cylinder";
constexpr char L1_VIS_002_CONE_NAME[] = "Cone";
constexpr char L1_VIS_004_CLIP_NAME[] = "L1VisAnimatedTransform";
constexpr float PI_VALUE = 3.14159265359F;
constexpr float HALF_PI = 1.57079632679F;
constexpr float THREE_HALF_PI = 4.71238898038F;
constexpr float FULL_ORBIT_RADIANS = 6.28318530718F;
constexpr float L1_SAMPLE_016_CAMERA_ASPECT = 1.77777777778F;
constexpr float TOLERANCE = 0.0001F;
constexpr std::uint32_t FRAME_ID = 9101U;
constexpr std::uint32_t CAMERA_ID = 9201U;
constexpr std::uint32_t DRAW_ID = 9301U;
constexpr std::uint32_t PASS_ID = 9401U;
constexpr std::uint32_t MATERIAL_ID = 9501U;
constexpr std::uint32_t MATERIAL_ASSET_SLOT = 9601U;
constexpr std::uint32_t TEXTURE_ASSET_SLOT = 9701U;
constexpr std::size_t MATERIAL_CONSTANT_BYTE_COUNT = 16U;
constexpr std::uint32_t L1_VIS_004_CLIP_ID = 9801U;
constexpr std::uint32_t L1_VIS_004_TRACK_ID = 9901U;
constexpr std::size_t VERTEX_STRIDE_BYTES = 32U;
constexpr std::size_t VERTEX_BUFFER_BYTES = VERTEX_STRIDE_BYTES * 128U;
constexpr std::size_t INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * 256U;
constexpr std::size_t CAPTURE_BUDGET = 4096U;
constexpr std::uint16_t L1_VIS_CAPTURE_EXTENT = 4U;
constexpr std::uint8_t CAPTURE_SENTINEL = 0xCCU;
constexpr std::uint64_t L1_VIS_004_CLIP_START_NANOSECONDS = 1000000000ULL;
constexpr std::uint64_t L1_VIS_004_SAMPLE_NANOSECONDS = 1500000000ULL;
constexpr std::uint32_t L1_VIS_005_FRAME_COUNT = 5U;
constexpr std::uint32_t L1_SAMPLE_016_FRAME_COUNT = 8U;
constexpr std::uint16_t L1_SAMPLE_013_MINIMUM_IMAGE_WIDTH = 320U;
constexpr std::uint16_t L1_SAMPLE_013_MINIMUM_IMAGE_HEIGHT = 180U;
constexpr std::uint16_t L1_SAMPLE_014_TARGET_IMAGE_WIDTH = 640U;
constexpr std::uint16_t L1_SAMPLE_014_TARGET_IMAGE_HEIGHT = 360U;
constexpr std::uint16_t L1_SAMPLE_014_TARGET_CAPTURE_WIDTH = 640U;
constexpr std::uint16_t L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT = 360U;
constexpr float L1_VIS_005_ORBIT_RADIUS = 5.0F;
constexpr float L1_VIS_005_ORBIT_HEIGHT = 2.0F;

struct L1VisAnimatedEntityReport final {
    WorldObjectId world_object_id{};
    std::uint32_t clip_id = 0U;
    const char *clip_name = nullptr;
    std::size_t clip_name_byte_count = 0U;
    std::uint32_t track_id = 0U;
    AnimationRuntimeChannel channel = AnimationRuntimeChannel::RotationX;
    float sample_time_seconds = 0.0F;
    AnimationRuntimeInterpolation interpolation = AnimationRuntimeInterpolation::Linear;
    AnimationRuntimeSampledValue sampled_value{};
    WorldTransformState applied_transform{};
    WorldTransformState render_scene_consumed_transform{};
    RenderSceneThreePrimitiveCaptureStatus capture_status =
        RenderSceneThreePrimitiveCaptureStatus::InvalidArgument;
    RenderSceneThreePrimitiveCaptureMissingLayer first_missing_layer =
        RenderSceneThreePrimitiveCaptureMissingLayer::None;
};

struct L1Vis006DiagnosticExpectation final {
    RenderSceneMissingLayerDiagnosticFault fault = RenderSceneMissingLayerDiagnosticFault::None;
    RenderSceneMissingLayerDiagnosticLayer layer = RenderSceneMissingLayerDiagnosticLayer::None;
    RenderSceneMissingLayerDiagnosticStatus status = RenderSceneMissingLayerDiagnosticStatus::Success;
    const char *diagnostic_name = nullptr;
    bool blocked_by_environment = false;
};

struct RgbSample final {
    std::uint8_t r = 0U;
    std::uint8_t g = 0U;
    std::uint8_t b = 0U;
};

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool Approx(float left, float right) {
    const float delta = std::fabs(left - right);
    return delta <= TOLERANCE;
}

bool RhiColorsMatch(RhiColor left, RhiColor right) {
    if (left.r != right.r) {
        return false;
    }

    if (left.g != right.g) {
        return false;
    }

    if (left.b != right.b) {
        return false;
    }

    return left.a == right.a;
}

std::uint32_t RhiColorLight(RhiColor color) {
    return static_cast<std::uint32_t>(color.r) +
        static_cast<std::uint32_t>(color.g) +
        static_cast<std::uint32_t>(color.b);
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

std::size_t CaptureByteCount(std::uint16_t width, std::uint16_t height) {
    const std::size_t wide_width = width;
    const std::size_t wide_height = height;
    return wide_width * wide_height * RGBA8_BYTES_PER_PIXEL;
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
        const RhiSwapchainResizeRequest &request,
        RhiSwapchainResizeResult &out_result) override {
        out_result = RhiSwapchainResizeResult{};
        out_result.previous_extent = snapshot_.swapchain.extent;
        out_result.previous_color_target = target_;
        out_result.snapshot = snapshot_.swapchain;
        if (!snapshot_.swapchain.valid) {
            ++snapshot_.swapchain.rejected_resize_count;
            ++snapshot_.failed_operation_count;
            out_result.status = RhiStatus::InvalidLifecycle;
            out_result.snapshot = snapshot_.swapchain;
            return RhiStatus::InvalidLifecycle;
        }

        if (request.extent.width == 0U || request.extent.height == 0U) {
            ++snapshot_.swapchain.rejected_resize_count;
            ++snapshot_.failed_operation_count;
            out_result.status = RhiStatus::InvalidDescriptor;
            out_result.snapshot = snapshot_.swapchain;
            return RhiStatus::InvalidDescriptor;
        }

        if (request.extent.width > MAX_CAPTURE_FIXTURE_EXTENT ||
            request.extent.height > MAX_CAPTURE_FIXTURE_EXTENT) {
            ++snapshot_.swapchain.rejected_resize_count;
            ++snapshot_.failed_operation_count;
            out_result.status = RhiStatus::CapacityExceeded;
            out_result.snapshot = snapshot_.swapchain;
            return RhiStatus::CapacityExceeded;
        }

        if (request.extent.width == snapshot_.swapchain.extent.width &&
            request.extent.height == snapshot_.swapchain.extent.height) {
            out_result.status = RhiStatus::Success;
            out_result.snapshot = snapshot_.swapchain;
            return RhiStatus::Success;
        }

        ++target_.generation;
        snapshot_.swapchain.extent = request.extent;
        snapshot_.swapchain.color_target = target_;
        snapshot_.swapchain.presented = false;
        ++snapshot_.swapchain.resize_count;
        snapshot_.last_capture_bytes_written = 0U;
        snapshot_.last_capture_extent = {};
        submitted_ = false;
        presented_ = false;
        out_result.status = RhiStatus::Success;
        out_result.snapshot = snapshot_.swapchain;
        out_result.resized = true;
        return RhiStatus::Success;
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

    RhiStatus RecordBindConstantBuffer(
        RhiCommandList &command_list,
        const RhiConstantBufferBinding &binding) override {
        if (binding.slot >= MAX_RHI_CONSTANT_BUFFER_SLOTS) {
            ++snapshot_.failed_operation_count;
            ++snapshot_.rejected_constant_buffer_bind_count;
            return RhiStatus::InvalidDescriptor;
        }

        if (binding.stage != RhiShaderStage::Vertex && binding.stage != RhiShaderStage::Pixel) {
            ++snapshot_.failed_operation_count;
            ++snapshot_.rejected_constant_buffer_bind_count;
            return RhiStatus::InvalidDescriptor;
        }

        if (!constant_buffer_active_ ||
            binding.buffer.slot != constant_buffer_.slot ||
            binding.buffer.generation != constant_buffer_.generation) {
            ++snapshot_.failed_operation_count;
            ++snapshot_.rejected_constant_buffer_bind_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordBindConstantBuffer(binding);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            ++snapshot_.rejected_constant_buffer_bind_count;
            return status;
        }

        last_constant_buffer_binding_ = binding;
        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindBlendState(RhiCommandList &command_list, const RhiBlendStateDesc &desc) override {
        if (!IsBlendStateDescValid(desc)) {
            ++snapshot_.failed_operation_count;
            ++snapshot_.rejected_blend_state_bind_count;
            return RhiStatus::InvalidDescriptor;
        }

        const RhiStatus status = command_list.RecordBindBlendState(desc);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            ++snapshot_.rejected_blend_state_bind_count;
            return status;
        }

        last_blend_state_ = desc;
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
        snapshot_.submitted_constant_buffer_bind_count += command_snapshot.constant_buffer_bind_command_count;
        snapshot_.submitted_blend_state_bind_count += command_snapshot.blend_state_bind_command_count;
        snapshot_.last_indexed_draw_index_count = last_draw_index_count_;
        if (command_snapshot.constant_buffer_bind_command_count > 0U) {
            snapshot_.last_bound_constant_buffer_slot = last_constant_buffer_binding_.slot;
            snapshot_.last_bound_constant_buffer_stage = last_constant_buffer_binding_.stage;
        }

        if (command_snapshot.blend_state_bind_command_count > 0U) {
            snapshot_.last_alpha_blend_enabled = last_blend_state_.mode == RhiBlendMode::AlphaOver;
            snapshot_.last_blend_constant_alpha = last_blend_state_.constant_alpha;
        }

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

        const std::size_t byte_count = CaptureByteCount(
            snapshot_.swapchain.extent.width,
            snapshot_.swapchain.extent.height);
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
        snapshot_.last_capture_extent = snapshot_.swapchain.extent;
        return RhiCaptureResult{RhiStatus::Success, byte_count, snapshot_.swapchain.extent};
    }

    RhiStatus CreateBuffer(
        const RhiBufferDesc &desc,
        std::span<const std::uint8_t> initial_bytes,
        RhiBufferHandle &out_handle) override {
        out_handle = RhiBufferHandle{};
        if (desc.usage != RhiBufferUsage::Constant) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidDescriptor;
        }

        if (desc.size_bytes == 0U || (desc.size_bytes % RHI_CONSTANT_BUFFER_ALIGNMENT) != 0U) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidDescriptor;
        }

        if (initial_bytes.size() > desc.size_bytes) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidDescriptor;
        }

        if (constant_buffer_active_) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::CapacityExceeded;
        }

        constant_buffer_active_ = true;
        out_handle = constant_buffer_;
        ++snapshot_.resources.buffer_count;
        ++snapshot_.resources.created_primitive_count;
        return RhiStatus::Success;
    }

    RhiStatus UpdateBuffer(
        RhiBufferHandle,
        std::span<const std::uint8_t>,
        RhiFenceHandle &out_fence,
        std::uint64_t=0ULL) override {
        out_fence = RhiFenceHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroyBuffer(RhiBufferHandle handle) override {
        if (!constant_buffer_active_ ||
            handle.slot != constant_buffer_.slot ||
            handle.generation != constant_buffer_.generation) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        constant_buffer_active_ = false;
        --snapshot_.resources.buffer_count;
        ++snapshot_.resources.destroyed_primitive_count;
        return RhiStatus::Success;
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
        RhiFenceHandle &out_fence,
        std::uint64_t=0ULL) override {
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
        constant_buffer_ = RhiBufferHandle{3U, 1U};
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
        last_blend_state_ = RhiBlendStateDesc{};
        last_constant_buffer_binding_ = RhiConstantBufferBinding{};
        constant_buffer_active_ = false;
        submitted_ = false;
        presented_ = false;
    }

    bool IsBlendStateDescValid(const RhiBlendStateDesc &desc) const {
        if (desc.mode == RhiBlendMode::Opaque) {
            return true;
        }

        return desc.mode == RhiBlendMode::AlphaOver;
    }

    RhiDeviceSnapshot snapshot_{};
    RhiTextureHandle target_{};
    RhiPipelineHandle pipeline_{};
    RhiBufferHandle vertex_buffer_{};
    RhiBufferHandle index_buffer_{};
    RhiBufferHandle constant_buffer_{};
    RhiColor last_clear_color_{};
    RhiBlendStateDesc last_blend_state_{};
    RhiConstantBufferBinding last_constant_buffer_binding_{};
    std::uint32_t last_draw_index_count_ = 0U;
    bool constant_buffer_active_ = false;
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

std::array<std::uint8_t, MATERIAL_CONSTANT_BYTE_COUNT> MakeMaterialConstants() {
    std::array<std::uint8_t, MATERIAL_CONSTANT_BYTE_COUNT> constants{};
    constants[0U] = 0x20U;
    constants[1U] = 0x30U;
    constants[2U] = 0x40U;
    constants[3U] = 0xC0U;
    constants[4U] = 0x40U;
    constants[5U] = 0x80U;
    constants[6U] = 0x60U;
    constants[7U] = 0xC0U;
    constants[8U] = 0x02U;
    return constants;
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

RenderSceneRuntimeMaterialRecord MakeRuntimeMaterialRecordWithId(std::uint32_t material_id) {
    RenderSceneRuntimeMaterialRecord record = MakeRuntimeMaterialRecord();
    record.material_id = material_id;
    return record;
}

WorldTransformState MakeTransform(float x, float y, float z) {
    WorldTransformState transform{};
    transform.translation_x = x;
    transform.translation_y = y;
    transform.translation_z = z;
    return transform;
}

RuntimeFrameContext MakeL1Vis004FrameContext() {
    RuntimeFrameContext context{};
    context.frame_index = FRAME_ID;
    context.delta_time_nanoseconds = 16666667ULL;
    context.fixed_time_nanoseconds = L1_VIS_004_SAMPLE_NANOSECONDS;
    context.frame_mode = RuntimeFrameMode::Fixed;
    return context;
}

AnimationRuntimeClipRecord MakeL1Vis004Clip() {
    AnimationRuntimeClipRecord clip{};
    clip.clip_id = L1_VIS_004_CLIP_ID;
    clip.duration_seconds = 1.0F;
    clip.first_track_index = 0U;
    clip.track_count = RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    clip.layer_count = 1U;
    clip.is_valid = true;
    return clip;
}

AnimationRuntimeTrackRecord MakeL1Vis004Track(
    std::uint32_t index,
    WorldObjectId target,
    AnimationRuntimeChannel channel,
    AnimationRuntimeInterpolation interpolation) {
    AnimationRuntimeTrackRecord track{};
    track.track_id = L1_VIS_004_TRACK_ID + index;
    track.target = target;
    track.channel = channel;
    track.interpolation = interpolation;
    track.first_keyframe_index = static_cast<std::size_t>(index) * 2U;
    track.keyframe_count = 2U;
    track.is_valid = true;
    return track;
}

AnimationRuntimeKeyframeRecord MakeL1Vis004Keyframe(float time_seconds, float value) {
    AnimationRuntimeKeyframeRecord keyframe{};
    keyframe.time_seconds = time_seconds;
    keyframe.value = value;
    keyframe.is_valid = true;
    return keyframe;
}

AnimationRuntimeSampleRequest MakeL1Vis004SampleRequest(
    std::span<const AnimationRuntimeClipRecord> clips,
    std::span<const AnimationRuntimeTrackRecord> tracks,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes) {
    AnimationRuntimeSampleRequest request{};
    request.clip_id = L1_VIS_004_CLIP_ID;
    request.clips = clips;
    request.tracks = tracks;
    request.keyframes = keyframes;
    request.frame_context = MakeL1Vis004FrameContext();
    request.clip_start_time_nanoseconds = L1_VIS_004_CLIP_START_NANOSECONDS;
    return request;
}

bool RegisterAnimationTargets(
    WorldInstance &world,
    WorldTransformBridge &bridge,
    const std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        &entities) {
    for (const RenderSceneThreePrimitiveEntityRequest &entity : entities) {
        const WorldRegistrationResult registration =
            world.RegisterObject(WorldObjectDesc{entity.world_object_id, true});
        if (!registration.Succeeded()) {
            return false;
        }

        const WorldTransformResult transform_result =
            bridge.Register(entity.world_object_id, entity.transform);
        if (transform_result.status != WorldTransformStatus::Success) {
            return false;
        }
    }

    return true;
}

bool ApplyAnimatedTransformsToEntities(
    WorldTransformBridge &bridge,
    std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        *out_entities) {
    if (out_entities == nullptr) {
        return false;
    }

    for (RenderSceneThreePrimitiveEntityRequest &entity : *out_entities) {
        const WorldTransformResult transform_result = bridge.Query(entity.world_object_id);
        if (transform_result.status != WorldTransformStatus::Success) {
            return false;
        }

        entity.transform = transform_result.transform_state;
    }

    return true;
}

void FillAnimatedEntityReport(
    std::size_t index,
    const AnimationRuntimeSampleResult &sample_result,
    const AnimationRuntimeTrackRecord &track,
    const AnimationRuntimeSampledValue &sampled_value,
    const RenderSceneThreePrimitiveCaptureResult &capture_result,
    L1VisAnimatedEntityReport *out_report) {
    if (out_report == nullptr) {
        return;
    }

    const auto &entity_report = capture_result.entity_reports[index];
    out_report->world_object_id = track.target;
    out_report->clip_id = sample_result.clip_id;
    out_report->clip_name = L1_VIS_004_CLIP_NAME;
    out_report->clip_name_byte_count = sizeof(L1_VIS_004_CLIP_NAME) - 1U;
    out_report->track_id = track.track_id;
    out_report->channel = track.channel;
    out_report->sample_time_seconds = sample_result.sample_time_seconds;
    out_report->interpolation = track.interpolation;
    out_report->sampled_value = sampled_value;
    out_report->applied_transform = entity_report.transform;
    out_report->render_scene_consumed_transform = entity_report.draw_record.transform;
    out_report->capture_status = capture_result.status;
    out_report->first_missing_layer = capture_result.first_missing_layer;
}

bool TransformMatches(const WorldTransformState &left, const WorldTransformState &right) {
    if (!Approx(left.translation_x, right.translation_x)) {
        return false;
    }

    if (!Approx(left.translation_y, right.translation_y)) {
        return false;
    }

    if (!Approx(left.translation_z, right.translation_z)) {
        return false;
    }

    if (!Approx(left.rotation_x, right.rotation_x)) {
        return false;
    }

    if (!Approx(left.rotation_y, right.rotation_y)) {
        return false;
    }

    if (!Approx(left.rotation_z, right.rotation_z)) {
        return false;
    }

    if (!Approx(left.rotation_w, right.rotation_w)) {
        return false;
    }

    if (!Approx(left.scale_x, right.scale_x)) {
        return false;
    }

    if (!Approx(left.scale_y, right.scale_y)) {
        return false;
    }

    return Approx(left.scale_z, right.scale_z);
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

RenderSceneThreePrimitiveCaptureRequest MakeAnimatedThreeTextureMaterialCaptureRequest(
    L1Vis001RhiDevice &device,
    std::vector<std::uint8_t> &capture,
    std::span<const RenderSceneThreePrimitiveEntityRequest> entities) {
    RenderSceneThreePrimitiveCaptureRequest request =
        MakeThreeTextureMaterialCaptureRequest(device, capture, entities);
    request.output_path = L1_VIS_004_OUTPUT_PATH;
    request.output_path_byte_count = sizeof(L1_VIS_004_OUTPUT_PATH) - 1U;
    return request;
}

bool ApplyL1Vis004RuntimeAnimationToEntities(
    std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        *out_entities) {
    if (out_entities == nullptr) {
        return false;
    }

    WorldInstance world;
    WorldTransformBridgeDesc bridge_desc{};
    bridge_desc.bridge_capacity = static_cast<std::uint32_t>(RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT);
    WorldTransformBridge bridge(world, bridge_desc);
    if (!RegisterAnimationTargets(world, bridge, *out_entities)) {
        return false;
    }

    const std::array<AnimationRuntimeClipRecord, 1U> clips{MakeL1Vis004Clip()};
    const WorldObjectId cube_id = (*out_entities)[0U].world_object_id;
    const WorldObjectId cylinder_id = (*out_entities)[1U].world_object_id;
    const WorldObjectId cone_id = (*out_entities)[2U].world_object_id;
    const std::array<AnimationRuntimeTrackRecord, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> tracks{
        MakeL1Vis004Track(0U, cube_id,
            AnimationRuntimeChannel::RotationY, AnimationRuntimeInterpolation::Linear),
        MakeL1Vis004Track(1U, cylinder_id,
            AnimationRuntimeChannel::RotationZ, AnimationRuntimeInterpolation::Step),
        MakeL1Vis004Track(2U, cone_id,
            AnimationRuntimeChannel::RotationX, AnimationRuntimeInterpolation::Linear)};
    const std::array<AnimationRuntimeKeyframeRecord, 6U> keyframes{
        MakeL1Vis004Keyframe(0.0F, 0.0F),
        MakeL1Vis004Keyframe(1.0F, 0.8F),
        MakeL1Vis004Keyframe(0.0F, 0.25F),
        MakeL1Vis004Keyframe(1.0F, 0.75F),
        MakeL1Vis004Keyframe(0.0F, 0.0F),
        MakeL1Vis004Keyframe(1.0F, 1.0F)};

    std::array<AnimationRuntimeSampledValue, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> sampled_values{};
    AnimationRuntimeSampleResult sample_result{};
    AnimationRuntimeSampler sampler;
    const AnimationRuntimeSampleRequest sample_request =
        MakeL1Vis004SampleRequest(clips, tracks, keyframes);
    const AnimationRuntimeStatus sample_status =
        sampler.Sample(sample_request, sampled_values, &sample_result);
    if (sample_status != AnimationRuntimeStatus::Success) {
        return false;
    }

    AnimationRuntimeTransformApplyResult apply_result{};
    const AnimationRuntimeTransformApplyRequest apply_request{&bridge, sampled_values};
    const AnimationRuntimeStatus apply_status =
        sampler.ApplySampledTransform(apply_request, &apply_result);
    if (apply_status != AnimationRuntimeStatus::Success) {
        return false;
    }

    if (apply_result.applied_value_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return false;
    }

    if (apply_result.updated_object_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return false;
    }

    return ApplyAnimatedTransformsToEntities(bridge, out_entities);
}

RenderSceneOrbitCaptureRequest MakeOrbitCaptureRequest(
    L1Vis001RhiDevice &device,
    std::vector<std::uint8_t> &capture,
    std::span<const RenderSceneThreePrimitiveEntityRequest> entities) {
    RenderSceneOrbitCaptureRequest request{};
    request.first_frame_id = FRAME_ID;
    request.frame_count = L1_VIS_005_FRAME_COUNT;
    request.camera_template = CameraRecord();
    request.material = MakeRuntimeMaterialRecord();
    request.entities = entities;
    request.target = RenderCameraVector3{0.0F, 0.0F, 0.0F};
    request.orbit_radius = L1_VIS_005_ORBIT_RADIUS;
    request.orbit_height = L1_VIS_005_ORBIT_HEIGHT;
    request.rhi_device = &device;
    request.output_path_prefix = L1_VIS_005_OUTPUT_PATH_PREFIX;
    request.output_path_prefix_byte_count = sizeof(L1_VIS_005_OUTPUT_PATH_PREFIX) - 1U;
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget_per_entity = L1VisCaptureByteCount();
    return request;
}

RenderSceneRuntimeVisualSceneProofRequest MakeRuntimeVisualSceneProofRequest(
    L1Vis001RhiDevice &device,
    std::vector<std::uint8_t> &capture) {
    RenderSceneRuntimeVisualSceneProofRequest request{};
    request.first_frame_id = FRAME_ID;
    request.frame_count = L1_VIS_005_FRAME_COUNT;
    request.rhi_device = &device;
    request.output_path_prefix = L1_SAMPLE_011_OUTPUT_PATH_PREFIX;
    request.output_path_prefix_byte_count = sizeof(L1_SAMPLE_011_OUTPUT_PATH_PREFIX) - 1U;
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget_per_entity = L1VisCaptureByteCount();
    request.target_capture_environment_available = true;
    return request;
}

RenderSceneRuntimeVisualSceneProofRequest MakeRuntimeVisualSceneImageProofRequest(
    L1Vis001RhiDevice &device,
    std::vector<std::uint8_t> &capture) {
    RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneProofRequest(device, capture);
    request.image_artifact_requested = true;
    request.image_output_path_prefix = L1_SAMPLE_011_IMAGE_OUTPUT_PATH_PREFIX;
    request.image_output_path_prefix_byte_count =
        sizeof(L1_SAMPLE_011_IMAGE_OUTPUT_PATH_PREFIX) - 1U;
    return request;
}

RenderSceneRuntimeVisualSceneProofRequest MakeRuntimeVisualSceneUserVisibleImageProofRequest(
    L1Vis001RhiDevice &device,
    std::vector<std::uint8_t> &capture) {
    RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneProofRequest(device, capture);
    request.image_artifact_requested = true;
    request.image_output_path_prefix = L1_SAMPLE_013_IMAGE_OUTPUT_PATH_PREFIX;
    request.image_output_path_prefix_byte_count =
        sizeof(L1_SAMPLE_013_IMAGE_OUTPUT_PATH_PREFIX) - 1U;
    request.minimum_image_artifact_width = L1_SAMPLE_013_MINIMUM_IMAGE_WIDTH;
    request.minimum_image_artifact_height = L1_SAMPLE_013_MINIMUM_IMAGE_HEIGHT;
    return request;
}

RenderSceneRuntimeVisualSceneProofRequest MakeRuntimeVisualSceneUserVisibleTargetImageProofRequest(
    L1Vis001RhiDevice &device,
    std::vector<std::uint8_t> &capture) {
    RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneProofRequest(device, capture);
    request.image_artifact_requested = true;
    request.image_output_path_prefix = L1_SAMPLE_014_IMAGE_OUTPUT_PATH_PREFIX;
    request.image_output_path_prefix_byte_count =
        sizeof(L1_SAMPLE_014_IMAGE_OUTPUT_PATH_PREFIX) - 1U;
    request.minimum_image_artifact_width = L1_SAMPLE_013_MINIMUM_IMAGE_WIDTH;
    request.minimum_image_artifact_height = L1_SAMPLE_013_MINIMUM_IMAGE_HEIGHT;
    request.target_image_artifact_width = L1_SAMPLE_014_TARGET_IMAGE_WIDTH;
    request.target_image_artifact_height = L1_SAMPLE_014_TARGET_IMAGE_HEIGHT;
    request.target_capture_width = L1_SAMPLE_014_TARGET_CAPTURE_WIDTH;
    request.target_capture_height = L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT;
    request.capture_byte_budget_per_entity = CaptureByteCount(
        L1_SAMPLE_014_TARGET_CAPTURE_WIDTH,
        L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT);
    return request;
}

RenderSceneRuntimeVisualSceneProofRequest MakeRuntimeVisualSceneSemanticTargetImageProofRequest(
    L1Vis001RhiDevice &device,
    std::vector<std::uint8_t> &capture) {
    RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneUserVisibleTargetImageProofRequest(device, capture);
    request.image_output_path_prefix = L1_SAMPLE_015_IMAGE_OUTPUT_PATH_PREFIX;
    request.image_output_path_prefix_byte_count =
        sizeof(L1_SAMPLE_015_IMAGE_OUTPUT_PATH_PREFIX) - 1U;
    return request;
}

RenderSceneRuntimeVisualSceneProofRequest MakeRuntimeVisualScenePerspectiveTargetImageProofRequest(
    L1Vis001RhiDevice &device,
    std::vector<std::uint8_t> &capture) {
    RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneSemanticTargetImageProofRequest(device, capture);
    request.frame_count = L1_SAMPLE_016_FRAME_COUNT;
    request.image_output_path_prefix = L1_SAMPLE_016_IMAGE_OUTPUT_PATH_PREFIX;
    request.image_output_path_prefix_byte_count =
        sizeof(L1_SAMPLE_016_IMAGE_OUTPUT_PATH_PREFIX) - 1U;
    request.camera_tween_requested = true;
    return request;
}

RenderSceneRuntimeVisualSceneCameraTweenKeyframe MakeRuntimeVisualSceneCameraTweenKeyframe(
    float time_seconds,
    RenderCameraVector3 position,
    RenderCameraVector3 target,
    float vertical_fov_radians,
    RenderSceneRuntimeVisualSceneCameraTweenEase ease) {
    RenderSceneRuntimeVisualSceneCameraTweenKeyframe keyframe{};
    keyframe.time_seconds = time_seconds;
    keyframe.pose.position = position;
    keyframe.pose.target = target;
    keyframe.pose.up = RenderCameraVector3{0.0F, 1.0F, 0.0F};
    keyframe.vertical_fov_radians = vertical_fov_radians;
    keyframe.ease = ease;
    return keyframe;
}

std::array<RenderSceneRuntimeVisualSceneCameraTweenKeyframe, 3U>
MakeRuntimeVisualSceneCameraTweenKeyframes() {
    const float middle_time =
        4.0F / static_cast<float>(L1_SAMPLE_016_FRAME_COUNT - 1U);
    std::array<RenderSceneRuntimeVisualSceneCameraTweenKeyframe, 3U> keyframes{
        MakeRuntimeVisualSceneCameraTweenKeyframe(
            0.0F,
            RenderCameraVector3{-4.7F, 2.4F, -5.2F},
            RenderCameraVector3{-0.2F, 0.6F, 0.1F},
            1.08F,
            RenderSceneRuntimeVisualSceneCameraTweenEase::SmoothStep),
        MakeRuntimeVisualSceneCameraTweenKeyframe(
            middle_time,
            RenderCameraVector3{2.4F, 3.1F, -3.4F},
            RenderCameraVector3{0.1F, 0.7F, 0.0F},
            0.82F,
            RenderSceneRuntimeVisualSceneCameraTweenEase::Linear),
        MakeRuntimeVisualSceneCameraTweenKeyframe(
            1.0F,
            RenderCameraVector3{4.6F, 2.0F, 3.8F},
            RenderCameraVector3{0.3F, 0.5F, -0.2F},
            1.16F,
            RenderSceneRuntimeVisualSceneCameraTweenEase::SmoothStep)};
    return keyframes;
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

bool OrbitFrameCaptureSegmentWasWritten(
    const std::vector<std::uint8_t> &capture,
    std::size_t frame_index) {
    const std::size_t frame_byte_count =
        L1VisCaptureByteCount() * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    const std::size_t begin_index = frame_index * frame_byte_count;
    const std::size_t end_index = begin_index + frame_byte_count;
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

bool OrbitFrameCaptureSegmentWasWritten(
    const std::vector<std::uint8_t> &capture,
    std::size_t frame_index,
    std::size_t frame_byte_count) {
    const std::size_t begin_index = frame_index * frame_byte_count;
    const std::size_t end_index = begin_index + frame_byte_count;
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

std::string_view OrbitOutputPathForFrame(std::uint32_t frame_index) {
    if (frame_index == 0U) {
        return "Artifacts/L1Vis005/OrbitCapture.Frame000.rvf";
    }

    if (frame_index == 1U) {
        return "Artifacts/L1Vis005/OrbitCapture.Frame001.rvf";
    }

    if (frame_index == 2U) {
        return "Artifacts/L1Vis005/OrbitCapture.Frame002.rvf";
    }

    if (frame_index == 3U) {
        return "Artifacts/L1Vis005/OrbitCapture.Frame003.rvf";
    }

    if (frame_index == 4U) {
        return "Artifacts/L1Vis005/OrbitCapture.Frame004.rvf";
    }

    return {};
}

std::string_view RuntimeVisualSceneOutputPathForFrame(std::uint32_t frame_index) {
    if (frame_index == 0U) {
        return "Artifacts/L1Sample011/RuntimeVisualScene.Frame000.rvf";
    }

    if (frame_index == 1U) {
        return "Artifacts/L1Sample011/RuntimeVisualScene.Frame001.rvf";
    }

    if (frame_index == 2U) {
        return "Artifacts/L1Sample011/RuntimeVisualScene.Frame002.rvf";
    }

    if (frame_index == 3U) {
        return "Artifacts/L1Sample011/RuntimeVisualScene.Frame003.rvf";
    }

    if (frame_index == 4U) {
        return "Artifacts/L1Sample011/RuntimeVisualScene.Frame004.rvf";
    }

    return {};
}

std::string_view RuntimeVisualSceneImagePathForFrame(std::uint32_t frame_index) {
    if (frame_index == 0U) {
        return "Artifacts/L1Sample011/RVF012/RuntimeVisualScene.Frame000.ppm";
    }

    if (frame_index == 1U) {
        return "Artifacts/L1Sample011/RVF012/RuntimeVisualScene.Frame001.ppm";
    }

    if (frame_index == 2U) {
        return "Artifacts/L1Sample011/RVF012/RuntimeVisualScene.Frame002.ppm";
    }

    if (frame_index == 3U) {
        return "Artifacts/L1Sample011/RVF012/RuntimeVisualScene.Frame003.ppm";
    }

    if (frame_index == 4U) {
        return "Artifacts/L1Sample011/RVF012/RuntimeVisualScene.Frame004.ppm";
    }

    return {};
}

std::string_view RuntimeVisualSceneUserVisibleTargetImagePathForFrame(std::uint32_t frame_index) {
    if (frame_index == 0U) {
        return "Artifacts/L1Sample011/RVF014/RuntimeVisualScene.Frame000.ppm";
    }

    if (frame_index == 1U) {
        return "Artifacts/L1Sample011/RVF014/RuntimeVisualScene.Frame001.ppm";
    }

    if (frame_index == 2U) {
        return "Artifacts/L1Sample011/RVF014/RuntimeVisualScene.Frame002.ppm";
    }

    if (frame_index == 3U) {
        return "Artifacts/L1Sample011/RVF014/RuntimeVisualScene.Frame003.ppm";
    }

    if (frame_index == 4U) {
        return "Artifacts/L1Sample011/RVF014/RuntimeVisualScene.Frame004.ppm";
    }

    return {};
}

std::string_view RuntimeVisualSceneSemanticTargetImagePathForFrame(std::uint32_t frame_index) {
    if (frame_index == 0U) {
        return "Artifacts/L1Sample011/RVF015/RuntimeVisualScene.Frame000.ppm";
    }

    if (frame_index == 1U) {
        return "Artifacts/L1Sample011/RVF015/RuntimeVisualScene.Frame001.ppm";
    }

    if (frame_index == 2U) {
        return "Artifacts/L1Sample011/RVF015/RuntimeVisualScene.Frame002.ppm";
    }

    if (frame_index == 3U) {
        return "Artifacts/L1Sample011/RVF015/RuntimeVisualScene.Frame003.ppm";
    }

    if (frame_index == 4U) {
        return "Artifacts/L1Sample011/RVF015/RuntimeVisualScene.Frame004.ppm";
    }

    return {};
}

std::string_view RuntimeVisualScenePerspectiveTargetImagePathForFrame(std::uint32_t frame_index) {
    if (frame_index == 0U) {
        return "Artifacts/L1Sample011/RVF016/RuntimeVisualScene.Frame000.ppm";
    }

    if (frame_index == 1U) {
        return "Artifacts/L1Sample011/RVF016/RuntimeVisualScene.Frame001.ppm";
    }

    if (frame_index == 2U) {
        return "Artifacts/L1Sample011/RVF016/RuntimeVisualScene.Frame002.ppm";
    }

    if (frame_index == 3U) {
        return "Artifacts/L1Sample011/RVF016/RuntimeVisualScene.Frame003.ppm";
    }

    if (frame_index == 4U) {
        return "Artifacts/L1Sample011/RVF016/RuntimeVisualScene.Frame004.ppm";
    }

    if (frame_index == 5U) {
        return "Artifacts/L1Sample011/RVF016/RuntimeVisualScene.Frame005.ppm";
    }

    if (frame_index == 6U) {
        return "Artifacts/L1Sample011/RVF016/RuntimeVisualScene.Frame006.ppm";
    }

    if (frame_index == 7U) {
        return "Artifacts/L1Sample011/RVF016/RuntimeVisualScene.Frame007.ppm";
    }

    return {};
}

bool OrbitOutputPathMatches(const RenderSceneOrbitCaptureFrameReport &frame_report) {
    const std::string_view expected = OrbitOutputPathForFrame(frame_report.frame_index);
    if (expected.empty()) {
        return false;
    }

    const std::string_view actual(frame_report.output_path, frame_report.output_path_byte_count);
    return actual == expected;
}

bool RuntimeVisualSceneOutputPathMatches(const RenderSceneOrbitCaptureFrameReport &frame_report) {
    const std::string_view expected = RuntimeVisualSceneOutputPathForFrame(frame_report.frame_index);
    if (expected.empty()) {
        return false;
    }

    const std::string_view actual(frame_report.output_path, frame_report.output_path_byte_count);
    return actual == expected;
}

bool RuntimeVisualSceneImagePathMatches(
    const RenderSceneRuntimeVisualSceneImageArtifactReport &report) {
    const std::string_view expected = RuntimeVisualSceneImagePathForFrame(report.frame_index);
    if (expected.empty()) {
        return false;
    }

    const std::string_view actual(report.output_path, report.output_path_byte_count);
    return actual == expected;
}

bool RuntimeVisualSceneUserVisibleTargetImagePathMatches(
    const RenderSceneRuntimeVisualSceneImageArtifactReport &report) {
    const std::string_view expected =
        RuntimeVisualSceneUserVisibleTargetImagePathForFrame(report.frame_index);
    if (expected.empty()) {
        return false;
    }

    const std::string_view actual(report.output_path, report.output_path_byte_count);
    return actual == expected;
}

bool RuntimeVisualSceneSemanticTargetImagePathMatches(
    const RenderSceneRuntimeVisualSceneImageArtifactReport &report) {
    const std::string_view expected =
        RuntimeVisualSceneSemanticTargetImagePathForFrame(report.frame_index);
    if (expected.empty()) {
        return false;
    }

    const std::string_view actual(report.output_path, report.output_path_byte_count);
    return actual == expected;
}

bool RuntimeVisualScenePerspectiveTargetImagePathMatches(
    const RenderSceneRuntimeVisualSceneImageArtifactReport &report) {
    const std::string_view expected =
        RuntimeVisualScenePerspectiveTargetImagePathForFrame(report.frame_index);
    if (expected.empty()) {
        return false;
    }

    const std::string_view actual(report.output_path, report.output_path_byte_count);
    return actual == expected;
}

bool CleanRuntimeVisualSceneImageArtifactDirectory() {
    std::error_code error;
    std::filesystem::remove_all(L1_SAMPLE_011_IMAGE_ARTIFACT_DIRECTORY, error);
    return !error;
}

bool CleanRuntimeVisualSceneUserVisibleImageArtifactDirectory() {
    std::error_code error;
    std::filesystem::remove_all(L1_SAMPLE_013_IMAGE_ARTIFACT_DIRECTORY, error);
    return !error;
}

bool CleanRuntimeVisualSceneUserVisibleTargetImageArtifactDirectory() {
    std::error_code error;
    std::filesystem::remove_all(L1_SAMPLE_014_IMAGE_ARTIFACT_DIRECTORY, error);
    return !error;
}

bool CleanRuntimeVisualSceneSemanticTargetImageArtifactDirectory() {
    std::error_code error;
    std::filesystem::remove_all(L1_SAMPLE_015_IMAGE_ARTIFACT_DIRECTORY, error);
    return !error;
}

bool CleanRuntimeVisualScenePerspectiveTargetImageArtifactDirectory() {
    std::error_code error;
    std::filesystem::remove_all(L1_SAMPLE_016_IMAGE_ARTIFACT_DIRECTORY, error);
    return !error;
}

std::size_t RuntimeVisualSceneImageRgbByteCount() {
    const std::size_t width =
        static_cast<std::size_t>(L1_VIS_CAPTURE_EXTENT) *
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    const std::size_t height = L1_VIS_CAPTURE_EXTENT;
    return width * height * 3U;
}

std::size_t RuntimeVisualSceneTargetImageRgbByteCount() {
    const std::size_t width = L1_SAMPLE_014_TARGET_IMAGE_WIDTH;
    const std::size_t height = L1_SAMPLE_014_TARGET_IMAGE_HEIGHT;
    return width * height * 3U;
}

std::FILE *OpenBinaryReadFile(const char *path) {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    std::FILE *file = std::fopen(path, "rb");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    return file;
}

bool FileStartsWithBytes(
    const char *path,
    const char *expected_bytes,
    std::size_t expected_byte_count) {
    if (path == nullptr) {
        return false;
    }

    if (expected_bytes == nullptr) {
        return false;
    }

    std::FILE *file = OpenBinaryReadFile(path);
    if (file == nullptr) {
        return false;
    }

    std::array<char, 32U> actual_bytes{};
    if (expected_byte_count > actual_bytes.size()) {
        std::fclose(file);
        return false;
    }

    const std::size_t read_count =
        std::fread(actual_bytes.data(), sizeof(char), expected_byte_count, file);
    const int close_result = std::fclose(file);
    if (close_result != 0) {
        return false;
    }

    if (read_count != expected_byte_count) {
        return false;
    }

    for (std::size_t index = 0U; index < expected_byte_count; ++index) {
        if (actual_bytes[index] != expected_bytes[index]) {
            return false;
        }
    }

    return true;
}

bool ReadBinaryFile(const char *path, std::vector<std::uint8_t> *out_bytes) {
    if (path == nullptr) {
        return false;
    }

    if (out_bytes == nullptr) {
        return false;
    }

    std::error_code error;
    const std::uintmax_t file_size = std::filesystem::file_size(path, error);
    if (error) {
        return false;
    }

    if (file_size == 0U || file_size > 5000000U) {
        return false;
    }

    std::FILE *file = OpenBinaryReadFile(path);
    if (file == nullptr) {
        return false;
    }

    out_bytes->assign(static_cast<std::size_t>(file_size), 0U);
    const std::size_t read_count =
        std::fread(out_bytes->data(), sizeof(std::uint8_t), out_bytes->size(), file);
    const int close_result = std::fclose(file);
    if (close_result != 0) {
        return false;
    }

    return read_count == out_bytes->size();
}

bool FilesHaveDifferentBytes(const char *left_path, const char *right_path) {
    std::vector<std::uint8_t> left_bytes{};
    std::vector<std::uint8_t> right_bytes{};
    if (!ReadBinaryFile(left_path, &left_bytes)) {
        return false;
    }

    if (!ReadBinaryFile(right_path, &right_bytes)) {
        return false;
    }

    if (left_bytes.size() != right_bytes.size()) {
        return true;
    }

    for (std::size_t index = 0U; index < left_bytes.size(); ++index) {
        if (left_bytes[index] != right_bytes[index]) {
            return true;
        }
    }

    return false;
}

bool RgbSamplesMatch(RgbSample left, RgbSample right) {
    if (left.r != right.r) {
        return false;
    }

    if (left.g != right.g) {
        return false;
    }

    return left.b == right.b;
}

bool ReadPpmPixel(
    const std::vector<std::uint8_t> &bytes,
    std::size_t header_byte_count,
    std::size_t row,
    std::size_t column,
    RgbSample *out_sample) {
    if (out_sample == nullptr) {
        return false;
    }

    const std::size_t image_width = L1_SAMPLE_014_TARGET_IMAGE_WIDTH;
    const std::size_t image_height = L1_SAMPLE_014_TARGET_IMAGE_HEIGHT;
    if (row >= image_height) {
        return false;
    }

    if (column >= image_width) {
        return false;
    }

    const std::size_t pixel_offset =
        header_byte_count + (row * image_width + column) * 3U;
    if (pixel_offset + 2U >= bytes.size()) {
        return false;
    }

    out_sample->r = bytes[pixel_offset];
    out_sample->g = bytes[pixel_offset + 1U];
    out_sample->b = bytes[pixel_offset + 2U];
    return true;
}

std::uint32_t MaxRgbComponent(RgbSample sample) {
    std::uint32_t max_value = sample.r;
    if (sample.g > max_value) {
        max_value = sample.g;
    }

    if (sample.b > max_value) {
        max_value = sample.b;
    }

    return max_value;
}

bool IsBrightSurfaceSample(RgbSample sample) {
    return MaxRgbComponent(sample) > 78U;
}

bool IsDarkEdgeSample(RgbSample sample) {
    return MaxRgbComponent(sample) < 68U;
}

bool SampleExists(
    const std::array<RgbSample, 64U> &samples,
    std::size_t sample_count,
    RgbSample target) {
    for (std::size_t index = 0U; index < sample_count; ++index) {
        if (RgbSamplesMatch(samples[index], target)) {
            return true;
        }
    }

    return false;
}

bool RuntimeVisualFrameHasRichSurfaceColors(
    const std::vector<std::uint8_t> &bytes,
    std::size_t header_byte_count,
    std::size_t minimum_color_count) {
    std::array<RgbSample, 64U> samples{};
    std::size_t sample_count = 0U;
    for (std::size_t row = 0U; row < L1_SAMPLE_014_TARGET_IMAGE_HEIGHT; row += 3U) {
        for (std::size_t column = 0U; column < L1_SAMPLE_014_TARGET_IMAGE_WIDTH; column += 3U) {
            RgbSample sample{};
            if (!ReadPpmPixel(bytes, header_byte_count, row, column, &sample)) {
                return false;
            }

            if (!IsBrightSurfaceSample(sample)) {
                continue;
            }

            if (SampleExists(samples, sample_count, sample)) {
                continue;
            }

            samples[sample_count] = sample;
            ++sample_count;
            if (sample_count >= minimum_color_count) {
                return true;
            }

            if (sample_count >= samples.size()) {
                return true;
            }
        }
    }

    return false;
}

bool RuntimeVisualFrameHasEdgeCue(
    const std::vector<std::uint8_t> &bytes,
    std::size_t header_byte_count) {
    for (std::size_t row = 1U; row + 1U < L1_SAMPLE_014_TARGET_IMAGE_HEIGHT; row += 2U) {
        for (std::size_t column = 1U; column + 1U < L1_SAMPLE_014_TARGET_IMAGE_WIDTH; column += 2U) {
            RgbSample sample{};
            RgbSample right_sample{};
            if (!ReadPpmPixel(bytes, header_byte_count, row, column, &sample)) {
                return false;
            }

            if (!ReadPpmPixel(bytes, header_byte_count, row, column + 1U, &right_sample)) {
                return false;
            }

            if (IsDarkEdgeSample(sample) && IsBrightSurfaceSample(right_sample)) {
                return true;
            }

            if (IsBrightSurfaceSample(sample) && IsDarkEdgeSample(right_sample)) {
                return true;
            }
        }
    }

    return false;
}

bool RuntimeVisualFrameHasBrightCoverage(
    const std::vector<std::uint8_t> &bytes,
    std::size_t header_byte_count) {
    std::size_t min_column = L1_SAMPLE_014_TARGET_IMAGE_WIDTH;
    std::size_t max_column = 0U;
    std::size_t min_row = L1_SAMPLE_014_TARGET_IMAGE_HEIGHT;
    std::size_t max_row = 0U;
    bool found = false;
    for (std::size_t row = 0U; row < L1_SAMPLE_014_TARGET_IMAGE_HEIGHT; row += 2U) {
        for (std::size_t column = 0U; column < L1_SAMPLE_014_TARGET_IMAGE_WIDTH; column += 2U) {
            RgbSample sample{};
            if (!ReadPpmPixel(bytes, header_byte_count, row, column, &sample)) {
                return false;
            }

            if (!IsBrightSurfaceSample(sample)) {
                continue;
            }

            if (column < min_column) {
                min_column = column;
            }

            if (column > max_column) {
                max_column = column;
            }

            if (row < min_row) {
                min_row = row;
            }

            if (row > max_row) {
                max_row = row;
            }

            found = true;
        }
    }

    if (!found) {
        return false;
    }

    const std::size_t width = max_column - min_column;
    const std::size_t height = max_row - min_row;
    if (width < 130U) {
        return false;
    }

    return height >= 70U;
}

bool RuntimeVisualFrameHasPerspectivePrimitiveCues(
    const std::vector<std::uint8_t> &bytes,
    std::size_t header_byte_count) {
    if (!RuntimeVisualFrameHasRichSurfaceColors(bytes, header_byte_count, 14U)) {
        return false;
    }

    if (!RuntimeVisualFrameHasEdgeCue(bytes, header_byte_count)) {
        return false;
    }

    return RuntimeVisualFrameHasBrightCoverage(bytes, header_byte_count);
}

bool CameraVectorMatches(RenderCameraVector3 left, RenderCameraVector3 right) {
    if (!Approx(left.x, right.x)) {
        return false;
    }

    if (!Approx(left.y, right.y)) {
        return false;
    }

    return Approx(left.z, right.z);
}

bool CameraPoseMatches(const RenderCameraPose &left, const RenderCameraPose &right) {
    if (!CameraVectorMatches(left.position, right.position)) {
        return false;
    }

    if (!CameraVectorMatches(left.target, right.target)) {
        return false;
    }

    return CameraVectorMatches(left.up, right.up);
}

bool CameraTweenFrameMatchesKeyframe(
    const RenderSceneRuntimeVisualSceneCameraTweenFrameReport &frame_report,
    const RenderSceneRuntimeVisualSceneCameraTweenKeyframe &keyframe) {
    if (!Approx(frame_report.sample_time_seconds, keyframe.time_seconds)) {
        return false;
    }

    if (!Approx(frame_report.vertical_fov_radians, keyframe.vertical_fov_radians)) {
        return false;
    }

    return CameraPoseMatches(frame_report.camera_pose, keyframe.pose);
}

RenderScenePrimitiveGeometryKind RuntimeVisualExpectedPrimitiveKind(std::size_t entity_index) {
    if (entity_index == 0U) {
        return RenderScenePrimitiveGeometryKind::Cube;
    }

    if (entity_index == 1U) {
        return RenderScenePrimitiveGeometryKind::Cylinder;
    }

    return RenderScenePrimitiveGeometryKind::Cone;
}

bool OrbitFramePoseMatches(
    const RenderSceneOrbitCaptureFrameReport &frame_report,
    float expected_x,
    float expected_z) {
    if (!Approx(frame_report.camera_pose.position.x, expected_x)) {
        return false;
    }

    if (!Approx(frame_report.camera_pose.position.y, L1_VIS_005_ORBIT_HEIGHT)) {
        return false;
    }

    if (!Approx(frame_report.camera_pose.position.z, expected_z)) {
        return false;
    }

    if (!Approx(frame_report.camera_pose.target.x, 0.0F)) {
        return false;
    }

    if (!Approx(frame_report.camera_pose.target.y, 0.0F)) {
        return false;
    }

    return Approx(frame_report.camera_pose.target.z, 0.0F);
}

int CheckL1Vis006DiagnosticExpectation(
    const RenderSceneMissingLayerDiagnosticRoute &route,
    const L1Vis006DiagnosticExpectation &expectation) {
    RenderSceneMissingLayerDiagnosticRequest request{};
    request.fault = expectation.fault;
    request.target_capture_environment_available = true;

    RenderSceneMissingLayerDiagnosticResult result{};
    const RenderSceneMissingLayerDiagnosticStatus status = route.Execute(request, &result);
    if (status != expectation.status) {
        return Fail("l1 vis diagnostic status mismatch");
    }

    if (result.status != expectation.status) {
        return Fail("l1 vis diagnostic result status mismatch");
    }

    if (result.first_missing_layer != expectation.layer) {
        return Fail("l1 vis diagnostic layer mismatch");
    }

    if (result.fault != expectation.fault) {
        return Fail("l1 vis diagnostic fault mismatch");
    }

    if (result.blocked_by_environment != expectation.blocked_by_environment) {
        return Fail("l1 vis diagnostic env flag mismatch");
    }

    const std::string_view actual_name(
        result.diagnostic_name,
        result.diagnostic_name_byte_count);
    if (actual_name != expectation.diagnostic_name) {
        return Fail("l1 vis diagnostic name mismatch");
    }

    if (!expectation.blocked_by_environment &&
        status == RenderSceneMissingLayerDiagnosticStatus::BlockedByEnv) {
        return Fail("l1 vis diagnostic hid semantic layer as env block");
    }

    return 0;
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

int RenderSceneRuntimeMaterialCopiesMaterialConstants() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    const std::array<std::uint8_t, MATERIAL_CONSTANT_BYTE_COUNT> constants =
        MakeMaterialConstants();
    RenderSceneRuntimeMaterialRequest request = MakeMaterialRequest(slots);
    request.material_constant_bytes = std::span<const std::uint8_t>(constants.data(), constants.size());

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(request, &record);
    if (status != RenderSceneRuntimeMaterialStatus::Success) {
        return Fail("runtime material constants build failed");
    }

    if (record.material_constant_byte_count != constants.size()) {
        return Fail("runtime material constant count mismatch");
    }

    for (std::size_t index = 0U; index < constants.size(); ++index) {
        if (record.material_constant_bytes[index] != constants[index]) {
            return Fail("runtime material constants were not copied");
        }
    }

    if (builder.Validate(record) != RenderSceneRuntimeMaterialStatus::Success) {
        return Fail("runtime material constants validation failed");
    }

    return 0;
}

int RenderSceneRuntimeMaterialRejectsOversizedMaterialConstants() {
    constexpr std::size_t OVERSIZED_CONSTANT_BYTES =
        yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_CONSTANT_BYTES + 1U;
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    std::array<std::uint8_t, OVERSIZED_CONSTANT_BYTES> constants{};
    RenderSceneRuntimeMaterialRequest request = MakeMaterialRequest(slots);
    request.material_constant_bytes = std::span<const std::uint8_t>(constants.data(), constants.size());

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    record.material_id = 77U;
    record.is_resolved = true;
    const RenderSceneRuntimeMaterialStatus status = builder.Build(request, &record);
    if (status != RenderSceneRuntimeMaterialStatus::MaterialConstantCapacityExceeded) {
        return Fail("runtime material accepted oversized constants");
    }

    if (record.material_id != 77U || record.texture_slot_count != 0U) {
        return Fail("runtime material oversized constants mutated output");
    }

    return 0;
}

int RenderSceneRuntimeMaterialCopiesBlendState() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    RenderSceneRuntimeMaterialRequest request = MakeMaterialRequest(slots);
    request.blend_state.mode = RhiBlendMode::AlphaOver;
    request.blend_state.constant_alpha = static_cast<std::uint8_t>(128U);

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(request, &record);
    if (status != RenderSceneRuntimeMaterialStatus::Success) {
        return Fail("runtime material blend state build failed");
    }

    if (record.blend_state.mode != RhiBlendMode::AlphaOver ||
        record.blend_state.constant_alpha != 128U) {
        return Fail("runtime material did not copy blend state");
    }

    if (builder.Validate(record) != RenderSceneRuntimeMaterialStatus::Success) {
        return Fail("runtime material blend state validation failed");
    }

    return 0;
}

int RenderSceneRuntimeMaterialRejectsInvalidBlendStateWithoutMutation() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    RenderSceneRuntimeMaterialRequest request = MakeMaterialRequest(slots);
    request.blend_state.mode = static_cast<RhiBlendMode>(255);

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    record.material_id = 77U;
    record.blend_state.mode = RhiBlendMode::AlphaOver;
    record.blend_state.constant_alpha = static_cast<std::uint8_t>(64U);
    const RenderSceneRuntimeMaterialStatus status = builder.Build(request, &record);
    if (status != RenderSceneRuntimeMaterialStatus::InvalidBlendState) {
        return Fail("runtime material accepted invalid blend state");
    }

    if (record.material_id != 77U ||
        record.texture_slot_count != 0U ||
        record.blend_state.mode != RhiBlendMode::AlphaOver ||
        record.blend_state.constant_alpha != 64U) {
        return Fail("runtime material invalid blend state mutated output");
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

int RenderSceneRuntimeFrameSubmitsEntitiesWithPerEntityMaterials() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const std::array<RenderSceneRuntimeMaterialRecord, 2U> materials{
        MakeRuntimeMaterialRecordWithId(MATERIAL_ID),
        MakeRuntimeMaterialRecordWithId(MATERIAL_ID + 1U)};
    std::array<RenderSceneRuntimeFrameEntityRequest, 3U> entities{
        MakeRuntimeFrameEntity(101U, MakeTransform(-2.0F, 0.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cube),
        MakeRuntimeFrameEntity(102U, MakeTransform(0.0F, 1.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cylinder),
        MakeRuntimeFrameEntity(103U, MakeTransform(2.0F, 0.0F, 1.0F), RenderScenePrimitiveGeometryKind::Cone)};
    entities[0U].material_table_index = 1U;
    entities[1U].material_table_index = 0U;
    entities[2U].material_table_index = 1U;

    RenderSceneRuntimeFrameRequest request =
        MakeRuntimeFrameRequest(camera, materials[0U], entities);
    request.materials = std::span<const RenderSceneRuntimeMaterialRecord>(
        materials.data(),
        materials.size());

    std::array<RenderSceneRuntimeFrameDrawRecord, 3U> draws{};
    RenderSceneRuntimeFrameResult result{};
    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status = builder.Build(request, draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::Success) {
        return Fail("runtime frame per entity material submission failed");
    }

    if (result.output_draw_count != 3U ||
        result.material_count != 2U ||
        result.material_variant_count != 2U) {
        return Fail("runtime frame per entity material counts changed");
    }

    if (draws[0U].draw.material_id != MATERIAL_ID + 1U ||
        draws[1U].draw.material_id != MATERIAL_ID ||
        draws[2U].draw.material_id != MATERIAL_ID + 1U) {
        return Fail("runtime frame per entity material routing changed");
    }

    return 0;
}

int RenderSceneRuntimeFrameRejectsMaterialTableIndexOutOfRange() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const std::array<RenderSceneRuntimeMaterialRecord, 1U> materials{
        MakeRuntimeMaterialRecordWithId(MATERIAL_ID)};
    std::array<RenderSceneRuntimeFrameEntityRequest, 1U> entities{
        MakeRuntimeFrameEntity(101U, MakeTransform(0.0F, 0.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cube)};
    entities[0U].material_table_index = 1U;

    RenderSceneRuntimeFrameRequest request =
        MakeRuntimeFrameRequest(camera, materials[0U], entities);
    request.materials = std::span<const RenderSceneRuntimeMaterialRecord>(
        materials.data(),
        materials.size());

    std::array<RenderSceneRuntimeFrameDrawRecord, 1U> draws{};
    RenderSceneRuntimeFrameResult result{};
    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status = builder.Build(request, draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::MaterialIndexOutOfRange) {
        return Fail("runtime frame accepted invalid material table index");
    }

    if (result.first_failed_entity_index != 0U ||
        result.first_failed_material_index != 1U ||
        result.output_draw_count != 0U ||
        draws[0U].draw.material_id != 0U) {
        return Fail("runtime frame material table index diagnostics changed");
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
    std::array<RenderSceneRuntimeFrameEntityRequest, 4U> entities{
        MakeRuntimeFrameEntity(101U, MakeTransform(-2.0F, 0.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cube),
        MakeRuntimeFrameEntity(102U, MakeTransform(0.0F, 1.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cylinder),
        MakeRuntimeFrameEntity(103U, MakeTransform(2.0F, 0.0F, 1.0F), RenderScenePrimitiveGeometryKind::Cone),
        MakeRuntimeFrameEntity(104U, MakeTransform(4.0F, 0.0F, 1.0F), RenderScenePrimitiveGeometryKind::Cube)};
    entities[1U].is_visible = false;
    std::array<RenderSceneRuntimeFrameDrawRecord, 2U> draws{};
    draws[0U].draw.material_id = MATERIAL_ID + 41U;
    draws[1U].draw.material_id = MATERIAL_ID + 42U;
    RenderSceneRuntimeFrameResult result{};

    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status =
        builder.Build(MakeRuntimeFrameRequest(camera, material, entities), draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::OutputCapacityExceeded) {
        return Fail("runtime frame did not report output capacity");
    }

    if (result.status != RenderSceneRuntimeFrameStatus::OutputCapacityExceeded ||
        result.submitted_entity_count != 3U ||
        result.output_draw_count != 0U ||
        result.first_failed_entity_index != 3U ||
        result.first_failed_material_index != 0xFFFFFFFFU) {
        return Fail("runtime frame output capacity diagnostics changed");
    }

    if (draws[0U].draw.material_id != MATERIAL_ID + 41U ||
        draws[1U].draw.material_id != MATERIAL_ID + 42U) {
        return Fail("runtime frame output capacity mutated draw records");
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

int RenderSceneL1Vis004CapturesAnimatedTransformSceneThroughRuntimeRoute() {
    L1Vis001RhiDevice device;
    std::vector<std::uint8_t> capture(
        L1VisCaptureByteCount() * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT,
        CAPTURE_SENTINEL);
    std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        entities = MakeThreePrimitiveEntities();
    WorldInstance world;
    WorldTransformBridgeDesc bridge_desc{};
    bridge_desc.bridge_capacity = static_cast<std::uint32_t>(RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT);
    WorldTransformBridge bridge(world, bridge_desc);
    if (!RegisterAnimationTargets(world, bridge, entities)) {
        return Fail("l1 vis animated transform target setup failed");
    }

    const std::array<AnimationRuntimeClipRecord, 1U> clips{MakeL1Vis004Clip()};
    const std::array<AnimationRuntimeTrackRecord, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> tracks{
        MakeL1Vis004Track(0U, entities[0U].world_object_id,
            AnimationRuntimeChannel::RotationY, AnimationRuntimeInterpolation::Linear),
        MakeL1Vis004Track(1U, entities[1U].world_object_id,
            AnimationRuntimeChannel::RotationZ, AnimationRuntimeInterpolation::Step),
        MakeL1Vis004Track(2U, entities[2U].world_object_id,
            AnimationRuntimeChannel::RotationX, AnimationRuntimeInterpolation::Linear)};
    const std::array<AnimationRuntimeKeyframeRecord, 6U> keyframes{
        MakeL1Vis004Keyframe(0.0F, 0.0F),
        MakeL1Vis004Keyframe(1.0F, 0.8F),
        MakeL1Vis004Keyframe(0.0F, 0.25F),
        MakeL1Vis004Keyframe(1.0F, 0.75F),
        MakeL1Vis004Keyframe(0.0F, 0.0F),
        MakeL1Vis004Keyframe(1.0F, 1.0F)};
    std::array<AnimationRuntimeSampledValue, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> sampled_values{};
    AnimationRuntimeSampleResult sample_result{};
    AnimationRuntimeSampler sampler;
    const AnimationRuntimeStatus sample_status = sampler.Sample(
        MakeL1Vis004SampleRequest(clips, tracks, keyframes),
        sampled_values,
        &sample_result);
    if (sample_status != AnimationRuntimeStatus::Success) {
        return Fail("l1 vis animated transform failed at l1 anim sampling layer");
    }

    if (sample_result.clip_id != L1_VIS_004_CLIP_ID ||
        sample_result.sampled_value_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return Fail("l1 vis animated transform sample identity mismatch");
    }

    if (!Approx(sample_result.sample_time_seconds, 0.5F)) {
        return Fail("l1 vis animated transform did not use frame context time");
    }

    if (sampled_values[0U].channel != AnimationRuntimeChannel::RotationY ||
        sampled_values[1U].channel != AnimationRuntimeChannel::RotationZ ||
        sampled_values[2U].channel != AnimationRuntimeChannel::RotationX) {
        return Fail("l1 vis animated transform sampled wrong channels");
    }

    if (!Approx(sampled_values[0U].value, 0.4F) ||
        !Approx(sampled_values[1U].value, 0.25F) ||
        !Approx(sampled_values[2U].value, 0.5F)) {
        return Fail("l1 vis animated transform sampled wrong keyframe values");
    }

    AnimationRuntimeTransformApplyResult apply_result{};
    const AnimationRuntimeTransformApplyRequest apply_request{&bridge, sampled_values};
    const AnimationRuntimeStatus apply_status =
        sampler.ApplySampledTransform(apply_request, &apply_result);
    if (apply_status != AnimationRuntimeStatus::Success) {
        return Fail("l1 vis animated transform failed at l1 anim apply layer");
    }

    if (apply_result.applied_value_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT ||
        apply_result.updated_object_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return Fail("l1 vis animated transform apply counters mismatch");
    }

    if (!ApplyAnimatedTransformsToEntities(bridge, &entities)) {
        return Fail("l1 vis animated transform world apply query failed");
    }

    RenderSceneThreePrimitiveCaptureRoute route;
    RenderSceneThreePrimitiveCaptureResult result{};
    const RenderSceneThreePrimitiveCaptureRequest request =
        MakeAnimatedThreeTextureMaterialCaptureRequest(device, capture, entities);
    const RenderSceneThreePrimitiveCaptureStatus status = route.Execute(request, &result);
    if (status != RenderSceneThreePrimitiveCaptureStatus::Success) {
        return Fail("l1 vis animated transform route did not complete");
    }

    if (result.first_missing_layer != RenderSceneThreePrimitiveCaptureMissingLayer::None) {
        return Fail("l1 vis animated transform route reported missing layer on success");
    }

    if (result.output_status != RenderSceneThreePrimitiveCaptureOutputStatus::CaptureAvailable) {
        return Fail("l1 vis animated transform route did not report capture availability");
    }

    std::array<L1VisAnimatedEntityReport, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> reports{};
    for (std::size_t index = 0U; index < reports.size(); ++index) {
        FillAnimatedEntityReport(
            index,
            sample_result,
            tracks[index],
            sampled_values[index],
            result,
            &reports[index]);
    }

    for (std::size_t index = 0U; index < reports.size(); ++index) {
        const L1VisAnimatedEntityReport &report = reports[index];
        if (report.clip_id != L1_VIS_004_CLIP_ID) {
            return Fail("l1 vis animated transform report clip id mismatch");
        }

        if (std::string_view(report.clip_name, report.clip_name_byte_count) != L1_VIS_004_CLIP_NAME) {
            return Fail("l1 vis animated transform report clip name mismatch");
        }

        if (report.track_id != tracks[index].track_id || report.channel != tracks[index].channel) {
            return Fail("l1 vis animated transform report track mismatch");
        }

        if (report.interpolation != tracks[index].interpolation ||
            !Approx(report.sample_time_seconds, 0.5F)) {
            return Fail("l1 vis animated transform report sample metadata mismatch");
        }

        if (!Approx(report.sampled_value.value, sampled_values[index].value)) {
            return Fail("l1 vis animated transform report sampled value mismatch");
        }

        if (!TransformMatches(report.applied_transform, report.render_scene_consumed_transform)) {
            return Fail("l1 vis animated transform render scene consumed stale transform");
        }

        if (report.capture_status != RenderSceneThreePrimitiveCaptureStatus::Success ||
            report.first_missing_layer != RenderSceneThreePrimitiveCaptureMissingLayer::None) {
            return Fail("l1 vis animated transform report capture status mismatch");
        }
    }

    if (!Approx(reports[0U].render_scene_consumed_transform.rotation_y, 0.4F) ||
        !Approx(reports[1U].render_scene_consumed_transform.rotation_z, 0.25F) ||
        !Approx(reports[2U].render_scene_consumed_transform.rotation_x, 0.5F)) {
        return Fail("l1 vis animated transform render scene rotation mismatch");
    }

    if (!Approx(reports[0U].render_scene_consumed_transform.translation_x, -2.0F) ||
        !Approx(reports[1U].render_scene_consumed_transform.translation_y, 1.0F) ||
        !Approx(reports[2U].render_scene_consumed_transform.translation_z, 1.0F)) {
        return Fail("l1 vis animated transform lost base scene placement");
    }

    if (result.capture_bytes_written != capture.size()) {
        return Fail("l1 vis animated transform capture byte count mismatch");
    }

    for (std::size_t index = 0U; index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT; ++index) {
        if (result.render_results[index].status != RenderDrawableFramePipelineStatus::Success) {
            return Fail("l1 vis animated transform rendercore result mismatch");
        }

        if (!CaptureSegmentWasWritten(capture, index)) {
            return Fail("l1 vis animated transform capture segment was not written");
        }
    }

    if (result.output_path_byte_count != sizeof(L1_VIS_004_OUTPUT_PATH) - 1U) {
        return Fail("l1 vis animated transform output path metadata mismatch");
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    const std::size_t expected_binding_count =
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT * result.material_texture_slot_report_count;
    if (snapshot.submitted_indexed_draw_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT ||
        snapshot.submitted_sampled_texture_bind_count != expected_binding_count ||
        snapshot.submitted_sampler_bind_count != expected_binding_count ||
        snapshot.capture_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return Fail("l1 vis animated transform did not drive rhi counters");
    }

    return 0;
}

int RenderSceneL1Vis005CapturesDeterministicOrbitSequence() {
    L1Vis001RhiDevice device;
    const std::size_t frame_capture_byte_count =
        L1VisCaptureByteCount() * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    std::vector<std::uint8_t> capture(
        frame_capture_byte_count * L1_VIS_005_FRAME_COUNT,
        CAPTURE_SENTINEL);
    std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        entities = MakeThreePrimitiveEntities();
    if (!ApplyL1Vis004RuntimeAnimationToEntities(&entities)) {
        return Fail("l1 vis orbit capture animation setup failed");
    }

    RenderSceneOrbitCaptureRoute route;
    RenderSceneOrbitCaptureResult result{};
    const RenderSceneOrbitCaptureRequest request =
        MakeOrbitCaptureRequest(device, capture, entities);
    const RenderSceneOrbitCaptureStatus status = route.Execute(request, &result);
    if (status != RenderSceneOrbitCaptureStatus::Success) {
        return Fail("l1 vis orbit capture route did not complete");
    }

    if (result.first_missing_layer != RenderSceneOrbitCaptureMissingLayer::None) {
        return Fail("l1 vis orbit capture reported missing layer on success");
    }

    if (result.requested_frame_count != L1_VIS_005_FRAME_COUNT ||
        result.completed_frame_count != L1_VIS_005_FRAME_COUNT) {
        return Fail("l1 vis orbit capture did not emit bounded frame set");
    }

    if (!Approx(result.orbit_radius, L1_VIS_005_ORBIT_RADIUS) ||
        !Approx(result.orbit_height, L1_VIS_005_ORBIT_HEIGHT)) {
        return Fail("l1 vis orbit capture metadata lost orbit bounds");
    }

    if (result.frame_capture_byte_budget != frame_capture_byte_count ||
        result.capture_bytes_written != capture.size()) {
        return Fail("l1 vis orbit capture byte budget mismatch");
    }

    const std::array<float, L1_VIS_005_FRAME_COUNT> expected_angles{
        0.0F,
        HALF_PI,
        PI_VALUE,
        THREE_HALF_PI,
        FULL_ORBIT_RADIANS};
    const std::array<float, L1_VIS_005_FRAME_COUNT> expected_x{
        0.0F,
        L1_VIS_005_ORBIT_RADIUS,
        0.0F,
        -L1_VIS_005_ORBIT_RADIUS,
        0.0F};
    const std::array<float, L1_VIS_005_FRAME_COUNT> expected_z{
        -L1_VIS_005_ORBIT_RADIUS,
        0.0F,
        L1_VIS_005_ORBIT_RADIUS,
        0.0F,
        -L1_VIS_005_ORBIT_RADIUS};
    for (std::size_t index = 0U; index < L1_VIS_005_FRAME_COUNT; ++index) {
        const std::uint32_t frame_index = static_cast<std::uint32_t>(index);
        const RenderSceneOrbitCaptureFrameReport &frame_report = result.frames[index];
        if (frame_report.status != RenderSceneOrbitCaptureStatus::Success) {
            return Fail("l1 vis orbit capture frame status mismatch");
        }

        if (frame_report.first_missing_layer != RenderSceneOrbitCaptureMissingLayer::None) {
            return Fail("l1 vis orbit capture frame reported missing layer");
        }

        if (frame_report.frame_index != frame_index ||
            frame_report.frame_id != FRAME_ID + frame_index) {
            return Fail("l1 vis orbit capture frame identity mismatch");
        }

        if (!Approx(frame_report.orbit_angle_radians, expected_angles[index])) {
            return Fail("l1 vis orbit capture angle mismatch");
        }

        if (!OrbitFramePoseMatches(frame_report, expected_x[index], expected_z[index])) {
            return Fail("l1 vis orbit capture camera pose mismatch");
        }

        if (frame_report.capture.frame_id != frame_report.frame_id ||
            frame_report.capture.camera_id != CAMERA_ID ||
            frame_report.capture.output_byte_budget != frame_capture_byte_count) {
            return Fail("l1 vis orbit capture frame metadata mismatch");
        }

        if (frame_report.output_status != RenderSceneThreePrimitiveCaptureOutputStatus::CaptureAvailable ||
            !OrbitOutputPathMatches(frame_report)) {
            return Fail("l1 vis orbit capture output metadata mismatch");
        }

        if (frame_report.capture_bytes_written != frame_capture_byte_count ||
            frame_report.capture_result.capture_bytes_written != frame_capture_byte_count) {
            return Fail("l1 vis orbit capture frame byte count mismatch");
        }

        if (frame_report.capture_result.entity_report_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT ||
            frame_report.capture_result.render_result_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
            return Fail("l1 vis orbit capture frame entity count mismatch");
        }

        if (!Approx(frame_report.capture_result.entity_reports[0U].draw_record.transform.rotation_y, 0.4F) ||
            !Approx(frame_report.capture_result.entity_reports[1U].draw_record.transform.rotation_z, 0.25F) ||
            !Approx(frame_report.capture_result.entity_reports[2U].draw_record.transform.rotation_x, 0.5F)) {
            return Fail("l1 vis orbit capture did not consume animated transforms");
        }

        if (!OrbitFrameCaptureSegmentWasWritten(capture, index)) {
            return Fail("l1 vis orbit capture frame segment was not written");
        }
    }

    if (std::string_view(result.frames[0U].output_path, result.frames[0U].output_path_byte_count) ==
        std::string_view(result.frames[1U].output_path, result.frames[1U].output_path_byte_count)) {
        return Fail("l1 vis orbit capture collapsed output paths to one still");
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    const std::uint64_t expected_draw_count =
        static_cast<std::uint64_t>(L1_VIS_005_FRAME_COUNT) *
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    const std::uint64_t expected_binding_count = expected_draw_count * 3U;
    if (snapshot.submitted_indexed_draw_count != expected_draw_count ||
        snapshot.submitted_sampled_texture_bind_count != expected_binding_count ||
        snapshot.submitted_sampler_bind_count != expected_binding_count ||
        snapshot.capture_count != expected_draw_count) {
        return Fail("l1 vis orbit capture did not drive multi-frame rhi counters");
    }

    return 0;
}

int RenderSceneL1Vis006ReportsExactMissingLayers() {
    RenderSceneMissingLayerDiagnosticRoute route;

    RenderSceneMissingLayerDiagnosticRequest success_request{};
    RenderSceneMissingLayerDiagnosticResult success_result{};
    const RenderSceneMissingLayerDiagnosticStatus success_status =
        route.Execute(success_request, &success_result);
    if (success_status != RenderSceneMissingLayerDiagnosticStatus::Success) {
        return Fail("l1 vis diagnostic success route failed");
    }

    if (success_result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::None) {
        return Fail("l1 vis diagnostic success reported missing layer");
    }

    const std::array<L1Vis006DiagnosticExpectation, 16U> expectations{
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingCamera,
            RenderSceneMissingLayerDiagnosticLayer::Camera,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "Camera",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingGeometryModel,
            RenderSceneMissingLayerDiagnosticLayer::GeometryModel,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "GeometryModel",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingMaterialTextureSlots,
            RenderSceneMissingLayerDiagnosticLayer::MaterialTextureSlots,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "MaterialTextureSlots",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingTextureResourceResolution,
            RenderSceneMissingLayerDiagnosticLayer::TextureResourceResolution,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "TextureResourceResolution",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingSamplerBinding,
            RenderSceneMissingLayerDiagnosticLayer::SamplerBinding,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "SamplerBinding",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingShaderPipeline,
            RenderSceneMissingLayerDiagnosticLayer::ShaderPipeline,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "ShaderPipeline",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingScenePlacement,
            RenderSceneMissingLayerDiagnosticLayer::ScenePlacement,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "ScenePlacement",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingAnimationInterpolation,
            RenderSceneMissingLayerDiagnosticLayer::AnimationInterpolation,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "AnimationInterpolation",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingTransformApply,
            RenderSceneMissingLayerDiagnosticLayer::TransformApply,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "TransformApply",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingRenderSceneMultiEntitySubmission,
            RenderSceneMissingLayerDiagnosticLayer::RenderSceneMultiEntitySubmission,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "RenderSceneMultiEntitySubmission",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingRenderCoreRhiDrawCapture,
            RenderSceneMissingLayerDiagnosticLayer::RenderCoreRhiDrawCapture,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "RenderCoreRhiDrawCapture",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingCameraOrbitSequencing,
            RenderSceneMissingLayerDiagnosticLayer::CameraOrbitSequencing,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "CameraOrbitSequencing",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingOutputBounding,
            RenderSceneMissingLayerDiagnosticLayer::OutputBounding,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "OutputBounding",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingCaptureTargetResolution,
            RenderSceneMissingLayerDiagnosticLayer::CaptureTargetResolution,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "CaptureTargetResolution",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingCaptureOutputImage,
            RenderSceneMissingLayerDiagnosticLayer::CaptureOutputImage,
            RenderSceneMissingLayerDiagnosticStatus::Fail,
            "CaptureOutputImage",
            false},
        L1Vis006DiagnosticExpectation{
            RenderSceneMissingLayerDiagnosticFault::MissingRhiCaptureTarget,
            RenderSceneMissingLayerDiagnosticLayer::RhiCaptureTarget,
            RenderSceneMissingLayerDiagnosticStatus::BlockedByEnv,
            "RhiCaptureTarget",
            true}};

    for (const L1Vis006DiagnosticExpectation &expectation : expectations) {
        const int ret_code = CheckL1Vis006DiagnosticExpectation(route, expectation);
        if (ret_code != 0) {
            return ret_code;
        }
    }

    RenderSceneMissingLayerDiagnosticRequest env_request{};
    env_request.target_capture_environment_available = false;
    RenderSceneMissingLayerDiagnosticResult env_result{};
    const RenderSceneMissingLayerDiagnosticStatus env_status =
        route.Execute(env_request, &env_result);
    if (env_status != RenderSceneMissingLayerDiagnosticStatus::BlockedByEnv) {
        return Fail("l1 vis diagnostic did not reserve env block for capture target");
    }

    if (env_result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::RhiCaptureTarget) {
        return Fail("l1 vis diagnostic env block reported wrong layer");
    }

    return 0;
}

int RenderSceneL1Sample011CapturesFinalRuntimeVisualScene() {
    L1Vis001RhiDevice device;
    const std::size_t frame_capture_byte_count =
        L1VisCaptureByteCount() * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    std::vector<std::uint8_t> capture(
        frame_capture_byte_count * L1_VIS_005_FRAME_COUNT,
        CAPTURE_SENTINEL);

    RenderSceneRuntimeVisualSceneProofRoute route;
    RenderSceneRuntimeVisualSceneProofResult result{};
    const RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneProofRequest(device, capture);
    const RenderSceneRuntimeVisualSceneProofStatus status = route.Execute(request, &result);
    if (status != RenderSceneRuntimeVisualSceneProofStatus::Success) {
        return Fail("l1 sample runtime visual scene proof did not complete");
    }

    if (result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::None ||
        result.diagnostic.status != RenderSceneMissingLayerDiagnosticStatus::Success) {
        return Fail("l1 sample runtime visual scene proof reported missing layer");
    }

    if (result.requested_frame_count != L1_VIS_005_FRAME_COUNT ||
        result.completed_frame_count != L1_VIS_005_FRAME_COUNT) {
        return Fail("l1 sample runtime visual scene proof frame count mismatch");
    }

    if (result.frame_capture_byte_budget != frame_capture_byte_count ||
        result.capture_bytes_written != capture.size()) {
        return Fail("l1 sample runtime visual scene proof capture byte count mismatch");
    }

    if (result.entity_report_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT ||
        result.material_texture_slot_report_count != 3U) {
        return Fail("l1 sample runtime visual scene proof summary count mismatch");
    }

    const std::array<float, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> expected_values{
        0.4F,
        0.25F,
        0.5F};
    for (std::size_t index = 0U; index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT; ++index) {
        const RenderSceneRuntimeVisualSceneProofEntityReport &report = result.entity_reports[index];
        if (!report.animation_sampled || !report.transform_applied || !report.render_scene_submitted) {
            return Fail("l1 sample runtime visual scene proof did not consume runtime modules");
        }

        if (report.animation_clip_id == 0U || report.animation_track_id == 0U) {
            return Fail("l1 sample runtime visual scene proof dropped animation identity");
        }

        if (!Approx(report.sampled_value, expected_values[index])) {
            return Fail("l1 sample runtime visual scene proof sampled value mismatch");
        }

        if (!TransformMatches(report.animated_transform, report.render_scene_consumed_transform)) {
            return Fail("l1 sample runtime visual scene proof render scene lost animated transform");
        }
    }

    if (!Approx(result.entity_reports[0U].render_scene_consumed_transform.rotation_y, 0.4F) ||
        !Approx(result.entity_reports[1U].render_scene_consumed_transform.rotation_z, 0.25F) ||
        !Approx(result.entity_reports[2U].render_scene_consumed_transform.rotation_x, 0.5F)) {
        return Fail("l1 sample runtime visual scene proof transform channels mismatch");
    }

    const std::array<float, L1_VIS_005_FRAME_COUNT> expected_x{
        0.0F,
        L1_VIS_005_ORBIT_RADIUS,
        0.0F,
        -L1_VIS_005_ORBIT_RADIUS,
        0.0F};
    const std::array<float, L1_VIS_005_FRAME_COUNT> expected_z{
        -L1_VIS_005_ORBIT_RADIUS,
        0.0F,
        L1_VIS_005_ORBIT_RADIUS,
        0.0F,
        -L1_VIS_005_ORBIT_RADIUS};
    for (std::size_t index = 0U; index < L1_VIS_005_FRAME_COUNT; ++index) {
        const RenderSceneOrbitCaptureFrameReport &frame_report = result.orbit_result.frames[index];
        if (frame_report.status != RenderSceneOrbitCaptureStatus::Success) {
            return Fail("l1 sample runtime visual scene proof frame status mismatch");
        }

        if (!RuntimeVisualSceneOutputPathMatches(frame_report)) {
            return Fail("l1 sample runtime visual scene proof output path mismatch");
        }

        if (!OrbitFramePoseMatches(frame_report, expected_x[index], expected_z[index])) {
            return Fail("l1 sample runtime visual scene proof camera orbit mismatch");
        }

        if (frame_report.capture_bytes_written != frame_capture_byte_count ||
            frame_report.capture.output_byte_budget != frame_capture_byte_count) {
            return Fail("l1 sample runtime visual scene proof frame capture metadata mismatch");
        }

        if (!OrbitFrameCaptureSegmentWasWritten(capture, index)) {
            return Fail("l1 sample runtime visual scene proof frame capture segment was not written");
        }
    }

    if (std::string_view(result.orbit_result.frames[0U].output_path,
            result.orbit_result.frames[0U].output_path_byte_count) ==
        std::string_view(result.orbit_result.frames[1U].output_path,
            result.orbit_result.frames[1U].output_path_byte_count)) {
        return Fail("l1 sample runtime visual scene proof collapsed full orbit frames");
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    const std::uint64_t expected_draw_count =
        static_cast<std::uint64_t>(L1_VIS_005_FRAME_COUNT) *
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    const std::uint64_t expected_binding_count = expected_draw_count * 3U;
    if (snapshot.submitted_indexed_draw_count != expected_draw_count ||
        snapshot.submitted_sampled_texture_bind_count != expected_binding_count ||
        snapshot.submitted_sampler_bind_count != expected_binding_count ||
        snapshot.capture_count != expected_draw_count) {
        return Fail("l1 sample runtime visual scene proof did not drive rhi counters");
    }

    return 0;
}

int RenderSceneL1Sample011EmitsFinalOrbitImageArtifacts() {
    if (!CleanRuntimeVisualSceneImageArtifactDirectory()) {
        return Fail("l1 sample runtime visual image artifact cleanup failed");
    }

    L1Vis001RhiDevice device;
    const std::size_t frame_capture_byte_count =
        L1VisCaptureByteCount() * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    std::vector<std::uint8_t> capture(
        frame_capture_byte_count * L1_VIS_005_FRAME_COUNT,
        CAPTURE_SENTINEL);

    RenderSceneRuntimeVisualSceneProofRoute route;
    RenderSceneRuntimeVisualSceneProofResult result{};
    const RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneImageProofRequest(device, capture);
    const RenderSceneRuntimeVisualSceneProofStatus status = route.Execute(request, &result);
    if (status != RenderSceneRuntimeVisualSceneProofStatus::Success) {
        return Fail("l1 sample runtime visual image artifact route did not complete");
    }

    if (result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::None) {
        return Fail("l1 sample runtime visual image artifact reported missing layer");
    }

    if (result.image_artifact_report_count != L1_VIS_005_FRAME_COUNT) {
        return Fail("l1 sample runtime visual image artifact frame count mismatch");
    }

    const std::uint16_t expected_width =
        static_cast<std::uint16_t>(
            L1_VIS_CAPTURE_EXTENT * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT);
    const std::size_t expected_header_byte_count =
        sizeof(L1_SAMPLE_011_IMAGE_HEADER) - 1U;
    const std::size_t expected_file_byte_count =
        expected_header_byte_count + RuntimeVisualSceneImageRgbByteCount();
    if (result.image_artifact_bytes_written != expected_file_byte_count * L1_VIS_005_FRAME_COUNT) {
        return Fail("l1 sample runtime visual image artifact total byte count mismatch");
    }

    for (std::size_t index = 0U; index < L1_VIS_005_FRAME_COUNT; ++index) {
        const RenderSceneRuntimeVisualSceneImageArtifactReport &report =
            result.image_artifact_reports[index];
        if (report.status != RenderSceneRuntimeVisualSceneImageArtifactStatus::Written) {
            return Fail("l1 sample runtime visual image artifact status mismatch");
        }

        const std::uint32_t frame_index = static_cast<std::uint32_t>(index);
        if (report.frame_index != frame_index || report.frame_id != FRAME_ID + frame_index) {
            return Fail("l1 sample runtime visual image artifact identity mismatch");
        }

        if (!RuntimeVisualSceneImagePathMatches(report)) {
            return Fail("l1 sample runtime visual image artifact path mismatch");
        }

        if (report.width != expected_width || report.height != L1_VIS_CAPTURE_EXTENT) {
            return Fail("l1 sample runtime visual image artifact dimensions mismatch");
        }

        if (report.source_byte_count != frame_capture_byte_count) {
            return Fail("l1 sample runtime visual image artifact source byte mismatch");
        }

        if (report.file_byte_count != expected_file_byte_count) {
            return Fail("l1 sample runtime visual image artifact file byte mismatch");
        }

        std::error_code error;
        const bool image_exists = std::filesystem::exists(report.output_path, error);
        if (error || !image_exists) {
            return Fail("l1 sample runtime visual image artifact file missing");
        }

        const std::uintmax_t file_size = std::filesystem::file_size(report.output_path, error);
        if (error || file_size != static_cast<std::uintmax_t>(report.file_byte_count)) {
            return Fail("l1 sample runtime visual image artifact disk size mismatch");
        }

        if (!FileStartsWithBytes(
                report.output_path,
                L1_SAMPLE_011_IMAGE_HEADER,
                expected_header_byte_count)) {
            return Fail("l1 sample runtime visual image artifact header mismatch");
        }

        if (!OrbitFrameCaptureSegmentWasWritten(capture, index)) {
            return Fail("l1 sample runtime visual image artifact did not use route capture");
        }
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    const std::uint64_t expected_draw_count =
        static_cast<std::uint64_t>(L1_VIS_005_FRAME_COUNT) *
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    if (snapshot.capture_count != expected_draw_count) {
        return Fail("l1 sample runtime visual image artifact bypassed rhi capture");
    }

    return 0;
}

int RenderSceneL1Sample012ReportsExactRuntimeVisualBlocker() {
    L1Vis001RhiDevice device;
    const std::size_t frame_capture_byte_count =
        L1VisCaptureByteCount() * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    std::vector<std::uint8_t> capture(
        frame_capture_byte_count * L1_VIS_005_FRAME_COUNT,
        CAPTURE_SENTINEL);

    RenderSceneRuntimeVisualSceneProofRoute route;
    RenderSceneRuntimeVisualSceneProofResult result{};
    RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneProofRequest(device, capture);
    request.diagnostic_fault = RenderSceneMissingLayerDiagnosticFault::MissingScenePlacement;
    const RenderSceneRuntimeVisualSceneProofStatus status = route.Execute(request, &result);
    if (status != RenderSceneRuntimeVisualSceneProofStatus::Fail) {
        return Fail("l1 sample runtime visual blocker did not fail semantic fault");
    }

    if (result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::ScenePlacement ||
        result.diagnostic.status != RenderSceneMissingLayerDiagnosticStatus::Fail) {
        return Fail("l1 sample runtime visual blocker reported wrong semantic layer");
    }

    const std::string_view semantic_name(
        result.diagnostic.diagnostic_name,
        result.diagnostic.diagnostic_name_byte_count);
    if (semantic_name != "ScenePlacement") {
        return Fail("l1 sample runtime visual blocker diagnostic name mismatch");
    }

    if (result.diagnostic.blocked_by_environment) {
        return Fail("l1 sample runtime visual blocker hid semantic fault as env block");
    }

    if (CaptureWasWritten(capture) || device.Snapshot().submit_count != 0U) {
        return Fail("l1 sample runtime visual blocker mutated runtime output");
    }

    std::vector<std::uint8_t> env_capture(
        frame_capture_byte_count * L1_VIS_005_FRAME_COUNT,
        CAPTURE_SENTINEL);
    RenderSceneRuntimeVisualSceneProofResult env_result{};
    RenderSceneRuntimeVisualSceneProofRequest env_request =
        MakeRuntimeVisualSceneProofRequest(device, env_capture);
    env_request.target_capture_environment_available = false;
    const RenderSceneRuntimeVisualSceneProofStatus env_status =
        route.Execute(env_request, &env_result);
    if (env_status != RenderSceneRuntimeVisualSceneProofStatus::BlockedByEnv) {
        return Fail("l1 sample runtime visual blocker did not reserve env block for capture target");
    }

    if (env_result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::RhiCaptureTarget ||
        !env_result.diagnostic.blocked_by_environment) {
        return Fail("l1 sample runtime visual blocker env layer mismatch");
    }

    L1Vis001RhiDevice image_device;
    std::vector<std::uint8_t> image_capture(
        frame_capture_byte_count * L1_VIS_005_FRAME_COUNT,
        CAPTURE_SENTINEL);
    RenderSceneRuntimeVisualSceneProofResult image_result{};
    RenderSceneRuntimeVisualSceneProofRequest image_request =
        MakeRuntimeVisualSceneProofRequest(image_device, image_capture);
    image_request.image_artifact_requested = true;
    const RenderSceneRuntimeVisualSceneProofStatus image_status =
        route.Execute(image_request, &image_result);
    if (image_status != RenderSceneRuntimeVisualSceneProofStatus::Fail) {
        return Fail("l1 sample runtime visual image blocker did not fail exact layer");
    }

    if (image_result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::CaptureOutputImage) {
        return Fail("l1 sample runtime visual image blocker reported wrong layer");
    }

    const std::string_view image_name(
        image_result.diagnostic.diagnostic_name,
        image_result.diagnostic.diagnostic_name_byte_count);
    if (image_name != "CaptureOutputImage") {
        return Fail("l1 sample runtime visual image blocker diagnostic name mismatch");
    }

    if (image_result.diagnostic.blocked_by_environment) {
        return Fail("l1 sample runtime visual image blocker hid output layer as env block");
    }

    if (CaptureWasWritten(image_capture) || image_device.Snapshot().submit_count != 0U) {
        return Fail("l1 sample runtime visual image blocker mutated runtime output");
    }

    return 0;
}

int RenderSceneL1Sample013ReportsUserVisibleCaptureResolutionBlocker() {
    if (!CleanRuntimeVisualSceneUserVisibleImageArtifactDirectory()) {
        return Fail("l1 sample runtime visual user resolution cleanup failed");
    }

    L1Vis001RhiDevice device;
    const std::size_t frame_capture_byte_count =
        L1VisCaptureByteCount() * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    std::vector<std::uint8_t> capture(
        frame_capture_byte_count * L1_VIS_005_FRAME_COUNT,
        CAPTURE_SENTINEL);

    RenderSceneRuntimeVisualSceneProofRoute route;
    RenderSceneRuntimeVisualSceneProofResult result{};
    const RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneUserVisibleImageProofRequest(device, capture);
    const RenderSceneRuntimeVisualSceneProofStatus status = route.Execute(request, &result);
    if (status != RenderSceneRuntimeVisualSceneProofStatus::Fail) {
        return Fail("l1 sample runtime visual user resolution did not fail exact layer");
    }

    if (result.first_missing_layer !=
        RenderSceneMissingLayerDiagnosticLayer::CaptureTargetResolution) {
        return Fail("l1 sample runtime visual user resolution reported wrong layer");
    }

    if (result.diagnostic.status != RenderSceneMissingLayerDiagnosticStatus::Fail) {
        return Fail("l1 sample runtime visual user resolution status mismatch");
    }

    const std::string_view resolution_name(
        result.diagnostic.diagnostic_name,
        result.diagnostic.diagnostic_name_byte_count);
    if (resolution_name != "CaptureTargetResolution") {
        return Fail("l1 sample runtime visual user resolution diagnostic name mismatch");
    }

    if (result.diagnostic.blocked_by_environment) {
        return Fail("l1 sample runtime visual user resolution hid layer as env block");
    }

    if (result.requested_minimum_image_artifact_width != L1_SAMPLE_013_MINIMUM_IMAGE_WIDTH ||
        result.requested_minimum_image_artifact_height != L1_SAMPLE_013_MINIMUM_IMAGE_HEIGHT) {
        return Fail("l1 sample runtime visual user resolution request metadata mismatch");
    }

    const std::uint16_t expected_available_width =
        static_cast<std::uint16_t>(
            L1_VIS_CAPTURE_EXTENT * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT);
    if (result.available_image_artifact_width != expected_available_width ||
        result.available_image_artifact_height != L1_VIS_CAPTURE_EXTENT) {
        return Fail("l1 sample runtime visual user resolution available extent mismatch");
    }

    if (result.image_artifact_report_count != 0U || result.image_artifact_bytes_written != 0U) {
        return Fail("l1 sample runtime visual user resolution emitted undersized artifacts");
    }

    if (CaptureWasWritten(capture) || device.Snapshot().submit_count != 0U) {
        return Fail("l1 sample runtime visual user resolution mutated runtime output");
    }

    std::error_code error;
    const bool artifact_directory_exists =
        std::filesystem::exists(L1_SAMPLE_013_IMAGE_ARTIFACT_DIRECTORY, error);
    if (error || artifact_directory_exists) {
        return Fail("l1 sample runtime visual user resolution created artifact directory");
    }

    return 0;
}

int RenderSceneL1Sample014EmitsUserVisibleCaptureTargetArtifacts() {
    if (!CleanRuntimeVisualSceneUserVisibleTargetImageArtifactDirectory()) {
        return Fail("l1 sample runtime visual target image cleanup failed");
    }

    L1Vis001RhiDevice device;
    const std::size_t entity_capture_byte_count = CaptureByteCount(
        L1_SAMPLE_014_TARGET_CAPTURE_WIDTH,
        L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT);
    const std::size_t frame_capture_byte_count =
        entity_capture_byte_count * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    std::vector<std::uint8_t> capture(
        frame_capture_byte_count * L1_VIS_005_FRAME_COUNT,
        CAPTURE_SENTINEL);

    RenderSceneRuntimeVisualSceneProofRoute route;
    RenderSceneRuntimeVisualSceneProofResult result{};
    const RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneUserVisibleTargetImageProofRequest(device, capture);
    const RenderSceneRuntimeVisualSceneProofStatus status = route.Execute(request, &result);
    if (status != RenderSceneRuntimeVisualSceneProofStatus::Success) {
        return Fail("l1 sample runtime visual target image route did not complete");
    }

    if (result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::None ||
        result.diagnostic.status != RenderSceneMissingLayerDiagnosticStatus::Success) {
        return Fail("l1 sample runtime visual target image reported missing layer");
    }

    if (result.requested_target_image_artifact_width != L1_SAMPLE_014_TARGET_IMAGE_WIDTH ||
        result.requested_target_image_artifact_height != L1_SAMPLE_014_TARGET_IMAGE_HEIGHT ||
        result.requested_target_capture_width != L1_SAMPLE_014_TARGET_CAPTURE_WIDTH ||
        result.requested_target_capture_height != L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT) {
        return Fail("l1 sample runtime visual target image request metadata mismatch");
    }

    if (result.requested_minimum_image_artifact_width != L1_SAMPLE_013_MINIMUM_IMAGE_WIDTH ||
        result.requested_minimum_image_artifact_height != L1_SAMPLE_013_MINIMUM_IMAGE_HEIGHT) {
        return Fail("l1 sample runtime visual target image minimum metadata mismatch");
    }

    if (result.available_image_artifact_width != L1_SAMPLE_014_TARGET_IMAGE_WIDTH ||
        result.available_image_artifact_height != L1_SAMPLE_014_TARGET_IMAGE_HEIGHT) {
        return Fail("l1 sample runtime visual target image available extent mismatch");
    }

    if (result.frame_capture_byte_budget != frame_capture_byte_count ||
        result.capture_bytes_written != capture.size()) {
        return Fail("l1 sample runtime visual target image capture byte count mismatch");
    }

    if (result.image_artifact_report_count != L1_VIS_005_FRAME_COUNT) {
        return Fail("l1 sample runtime visual target image frame count mismatch");
    }

    const std::size_t expected_header_byte_count =
        sizeof(L1_SAMPLE_014_IMAGE_HEADER) - 1U;
    const std::size_t expected_file_byte_count =
        expected_header_byte_count + RuntimeVisualSceneTargetImageRgbByteCount();
    if (result.image_artifact_bytes_written != expected_file_byte_count * L1_VIS_005_FRAME_COUNT) {
        return Fail("l1 sample runtime visual target image total byte count mismatch");
    }

    for (std::size_t index = 0U; index < L1_VIS_005_FRAME_COUNT; ++index) {
        const RenderSceneRuntimeVisualSceneImageArtifactReport &report =
            result.image_artifact_reports[index];
        if (report.status != RenderSceneRuntimeVisualSceneImageArtifactStatus::Written) {
            return Fail("l1 sample runtime visual target image status mismatch");
        }

        const std::uint32_t frame_index = static_cast<std::uint32_t>(index);
        if (report.frame_index != frame_index || report.frame_id != FRAME_ID + frame_index) {
            return Fail("l1 sample runtime visual target image identity mismatch");
        }

        if (!RuntimeVisualSceneUserVisibleTargetImagePathMatches(report)) {
            return Fail("l1 sample runtime visual target image path mismatch");
        }

        if (report.width != L1_SAMPLE_014_TARGET_IMAGE_WIDTH ||
            report.height != L1_SAMPLE_014_TARGET_IMAGE_HEIGHT) {
            return Fail("l1 sample runtime visual target image dimensions mismatch");
        }

        if (report.source_byte_count != frame_capture_byte_count ||
            report.file_byte_count != expected_file_byte_count) {
            return Fail("l1 sample runtime visual target image byte metadata mismatch");
        }

        std::error_code error;
        const bool image_exists = std::filesystem::exists(report.output_path, error);
        if (error || !image_exists) {
            return Fail("l1 sample runtime visual target image file missing");
        }

        const std::uintmax_t file_size = std::filesystem::file_size(report.output_path, error);
        if (error || file_size != static_cast<std::uintmax_t>(report.file_byte_count)) {
            return Fail("l1 sample runtime visual target image disk size mismatch");
        }

        if (!FileStartsWithBytes(
                report.output_path,
                L1_SAMPLE_014_IMAGE_HEADER,
                expected_header_byte_count)) {
            return Fail("l1 sample runtime visual target image header mismatch");
        }

        const RenderSceneOrbitCaptureFrameReport &frame_report =
            result.orbit_result.frames[index];
        if (frame_report.status != RenderSceneOrbitCaptureStatus::Success ||
            frame_report.capture_bytes_written != frame_capture_byte_count ||
            frame_report.capture.output_byte_budget != frame_capture_byte_count) {
            return Fail("l1 sample runtime visual target image frame metadata mismatch");
        }

        if (!OrbitFrameCaptureSegmentWasWritten(capture, index, frame_capture_byte_count)) {
            return Fail("l1 sample runtime visual target image did not use route capture");
        }

        if (frame_report.capture_result.render_result_count !=
            RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
            return Fail("l1 sample runtime visual target image render result count mismatch");
        }

        for (std::size_t entity_index = 0U;
            entity_index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
            ++entity_index) {
            const auto &render_result = frame_report.capture_result.render_results[entity_index];
            if (render_result.capture_bytes_written != entity_capture_byte_count) {
                return Fail("l1 sample runtime visual target image render byte mismatch");
            }

            if (render_result.capture_extent.width != L1_SAMPLE_014_TARGET_CAPTURE_WIDTH ||
                render_result.capture_extent.height != L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT) {
                return Fail("l1 sample runtime visual target image render extent mismatch");
            }

            if (render_result.pass_result.capture_extent.width != L1_SAMPLE_014_TARGET_CAPTURE_WIDTH ||
                render_result.pass_result.capture_extent.height != L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT) {
                return Fail("l1 sample runtime visual target image pass extent mismatch");
            }
        }
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    const std::uint64_t expected_draw_count =
        static_cast<std::uint64_t>(L1_VIS_005_FRAME_COUNT) *
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    if (snapshot.swapchain.resize_count != 1U ||
        snapshot.capture_count != expected_draw_count ||
        snapshot.last_capture_bytes_written != entity_capture_byte_count) {
        return Fail("l1 sample runtime visual target image rhi counter mismatch");
    }

    if (snapshot.last_capture_extent.width != L1_SAMPLE_014_TARGET_CAPTURE_WIDTH ||
        snapshot.last_capture_extent.height != L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT) {
        return Fail("l1 sample runtime visual target image rhi extent mismatch");
    }

    return 0;
}

int RenderSceneL1Sample015EmitsScenePixelSemanticsArtifacts() {
    if (!CleanRuntimeVisualSceneSemanticTargetImageArtifactDirectory()) {
        return Fail("l1 sample runtime visual semantic image cleanup failed");
    }

    L1Vis001RhiDevice device;
    const std::size_t entity_capture_byte_count = CaptureByteCount(
        L1_SAMPLE_014_TARGET_CAPTURE_WIDTH,
        L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT);
    const std::size_t frame_capture_byte_count =
        entity_capture_byte_count * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    std::vector<std::uint8_t> capture(
        frame_capture_byte_count * L1_VIS_005_FRAME_COUNT,
        CAPTURE_SENTINEL);

    RenderSceneRuntimeVisualSceneProofRoute route;
    RenderSceneRuntimeVisualSceneProofResult result{};
    const RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneSemanticTargetImageProofRequest(device, capture);
    const RenderSceneRuntimeVisualSceneProofStatus status = route.Execute(request, &result);
    if (status != RenderSceneRuntimeVisualSceneProofStatus::Success) {
        return Fail("l1 sample runtime visual semantic image route did not complete");
    }

    if (result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::None ||
        result.diagnostic.status != RenderSceneMissingLayerDiagnosticStatus::Success) {
        return Fail("l1 sample runtime visual semantic image reported missing layer");
    }

    if (result.available_image_artifact_width != L1_SAMPLE_014_TARGET_IMAGE_WIDTH ||
        result.available_image_artifact_height != L1_SAMPLE_014_TARGET_IMAGE_HEIGHT) {
        return Fail("l1 sample runtime visual semantic image available extent mismatch");
    }

    if (result.entity_report_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT ||
        result.material_texture_slot_report_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return Fail("l1 sample runtime visual semantic image route report count mismatch");
    }

    for (std::size_t entity_index = 0U;
        entity_index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
        ++entity_index) {
        const RenderSceneRuntimeVisualSceneProofEntityReport &entity_report =
            result.entity_reports[entity_index];
        if (!entity_report.render_scene_submitted) {
            return Fail("l1 sample runtime visual semantic image entity was not submitted");
        }

        if (entity_report.primitive_kind != RuntimeVisualExpectedPrimitiveKind(entity_index)) {
            return Fail("l1 sample runtime visual semantic image primitive mismatch");
        }
    }

    if (result.frame_capture_byte_budget != frame_capture_byte_count ||
        result.capture_bytes_written != capture.size()) {
        return Fail("l1 sample runtime visual semantic image capture byte count mismatch");
    }

    if (result.image_artifact_report_count != L1_VIS_005_FRAME_COUNT) {
        return Fail("l1 sample runtime visual semantic image frame count mismatch");
    }

    const std::size_t expected_header_byte_count =
        sizeof(L1_SAMPLE_014_IMAGE_HEADER) - 1U;
    const std::size_t expected_file_byte_count =
        expected_header_byte_count + RuntimeVisualSceneTargetImageRgbByteCount();
    if (result.image_artifact_bytes_written != expected_file_byte_count * L1_VIS_005_FRAME_COUNT) {
        return Fail("l1 sample runtime visual semantic image total byte count mismatch");
    }

    for (std::size_t index = 0U; index < L1_VIS_005_FRAME_COUNT; ++index) {
        const RenderSceneRuntimeVisualSceneImageArtifactReport &report =
            result.image_artifact_reports[index];
        if (report.status != RenderSceneRuntimeVisualSceneImageArtifactStatus::Written) {
            return Fail("l1 sample runtime visual semantic image status mismatch");
        }

        if (!RuntimeVisualSceneSemanticTargetImagePathMatches(report)) {
            return Fail("l1 sample runtime visual semantic image path mismatch");
        }

        if (report.width != L1_SAMPLE_014_TARGET_IMAGE_WIDTH ||
            report.height != L1_SAMPLE_014_TARGET_IMAGE_HEIGHT) {
            return Fail("l1 sample runtime visual semantic image dimensions mismatch");
        }

        if (report.source_byte_count != frame_capture_byte_count ||
            report.file_byte_count != expected_file_byte_count) {
            return Fail("l1 sample runtime visual semantic image byte metadata mismatch");
        }

        if (!FileStartsWithBytes(
                report.output_path,
                L1_SAMPLE_014_IMAGE_HEADER,
                expected_header_byte_count)) {
            return Fail("l1 sample runtime visual semantic image header mismatch");
        }

        const RenderSceneOrbitCaptureFrameReport &frame_report =
            result.orbit_result.frames[index];
        if (frame_report.status != RenderSceneOrbitCaptureStatus::Success ||
            frame_report.capture_bytes_written != frame_capture_byte_count ||
            frame_report.capture.output_byte_budget != frame_capture_byte_count) {
            return Fail("l1 sample runtime visual semantic image frame metadata mismatch");
        }

        if (!OrbitFrameCaptureSegmentWasWritten(capture, index, frame_capture_byte_count)) {
            return Fail("l1 sample runtime visual semantic image did not use route capture");
        }

        if (frame_report.capture_result.entity_report_count !=
            RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
            return Fail("l1 sample runtime visual semantic image entity result count mismatch");
        }

        if (frame_report.capture_result.render_result_count !=
            RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
            return Fail("l1 sample runtime visual semantic image render result count mismatch");
        }

        if (frame_report.capture_result.material_texture_slot_report_count !=
            RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
            return Fail("l1 sample runtime visual semantic image material slot count mismatch");
        }

        for (std::size_t entity_index = 0U;
            entity_index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
            ++entity_index) {
            const auto &entity_report =
                frame_report.capture_result.entity_reports[entity_index];
            if (!entity_report.submitted ||
                entity_report.primitive_kind != RuntimeVisualExpectedPrimitiveKind(entity_index)) {
                return Fail("l1 sample runtime visual semantic image draw report mismatch");
            }

            const auto &slot_report =
                frame_report.capture_result.material_texture_slot_reports[entity_index];
            if (slot_report.slot != entity_index ||
                !slot_report.texture_resource_resolved ||
                !slot_report.sampled_texture_bound ||
                !slot_report.sampler_bound) {
                return Fail("l1 sample runtime visual semantic image material report mismatch");
            }
        }

        if (index != 0U) {
            continue;
        }

        std::vector<std::uint8_t> frame_bytes{};
        if (!ReadBinaryFile(report.output_path, &frame_bytes)) {
            return Fail("l1 sample runtime visual semantic image read failed");
        }

        if (!RuntimeVisualFrameHasPerspectivePrimitiveCues(
                frame_bytes,
                expected_header_byte_count)) {
            return Fail("l1 sample runtime visual semantic image still lacks scene cues");
        }
    }

    if (!FilesHaveDifferentBytes(
            result.image_artifact_reports[0U].output_path,
            result.image_artifact_reports[1U].output_path)) {
        return Fail("l1 sample runtime visual semantic image orbit frames are identical");
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    const std::uint64_t expected_draw_count =
        static_cast<std::uint64_t>(L1_VIS_005_FRAME_COUNT) *
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    if (snapshot.swapchain.resize_count != 1U ||
        snapshot.capture_count != expected_draw_count ||
        snapshot.last_capture_bytes_written != entity_capture_byte_count) {
        return Fail("l1 sample runtime visual semantic image rhi counter mismatch");
    }

    if (snapshot.last_capture_extent.width != L1_SAMPLE_014_TARGET_CAPTURE_WIDTH ||
        snapshot.last_capture_extent.height != L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT) {
        return Fail("l1 sample runtime visual semantic image rhi extent mismatch");
    }

    return 0;
}

int RenderSceneL1Sample016EmitsPerspective3DPrimitiveCameraTweenArtifacts() {
    if (!CleanRuntimeVisualScenePerspectiveTargetImageArtifactDirectory()) {
        return Fail("l1 sample runtime visual perspective image cleanup failed");
    }

    L1Vis001RhiDevice device;
    const std::size_t entity_capture_byte_count = CaptureByteCount(
        L1_SAMPLE_014_TARGET_CAPTURE_WIDTH,
        L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT);
    const std::size_t frame_capture_byte_count =
        entity_capture_byte_count * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    std::vector<std::uint8_t> capture(
        frame_capture_byte_count * L1_SAMPLE_016_FRAME_COUNT,
        CAPTURE_SENTINEL);

    RenderSceneRuntimeVisualSceneProofRoute route;
    RenderSceneRuntimeVisualSceneProofResult result{};
    const std::array<RenderSceneRuntimeVisualSceneCameraTweenKeyframe, 3U> keyframes =
        MakeRuntimeVisualSceneCameraTweenKeyframes();
    RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualScenePerspectiveTargetImageProofRequest(device, capture);
    request.camera_tween_keyframes = keyframes;
    const RenderSceneRuntimeVisualSceneProofStatus status = route.Execute(request, &result);
    if (status != RenderSceneRuntimeVisualSceneProofStatus::Success) {
        return Fail("l1 sample runtime visual perspective image route did not complete");
    }

    if (result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::None ||
        result.diagnostic.status != RenderSceneMissingLayerDiagnosticStatus::Success) {
        return Fail("l1 sample runtime visual perspective image reported missing layer");
    }

    if (result.camera_projection_kind != RenderCameraProjectionKind::Perspective ||
        !result.camera_perspective_projection_used ||
        !result.camera_tween_used) {
        return Fail("l1 sample runtime visual perspective image did not use perspective camera");
    }

    if (!Approx(result.camera_vertical_fov_radians, keyframes[0U].vertical_fov_radians) ||
        !Approx(result.camera_aspect_ratio, L1_SAMPLE_016_CAMERA_ASPECT)) {
        return Fail("l1 sample runtime visual perspective image projection metadata mismatch");
    }

    if (result.camera_tween_keyframe_count != keyframes.size()) {
        return Fail("l1 sample runtime visual perspective image keyframe count mismatch");
    }

    if (result.requested_frame_count != L1_SAMPLE_016_FRAME_COUNT ||
        result.completed_frame_count != L1_SAMPLE_016_FRAME_COUNT ||
        result.orbit_result.completed_frame_count != L1_SAMPLE_016_FRAME_COUNT) {
        return Fail("l1 sample runtime visual perspective image frame count mismatch");
    }

    if (result.available_image_artifact_width != L1_SAMPLE_014_TARGET_IMAGE_WIDTH ||
        result.available_image_artifact_height != L1_SAMPLE_014_TARGET_IMAGE_HEIGHT) {
        return Fail("l1 sample runtime visual perspective image available extent mismatch");
    }

    if (result.entity_report_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT ||
        result.material_texture_slot_report_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return Fail("l1 sample runtime visual perspective image route report count mismatch");
    }

    if (result.frame_capture_byte_budget != frame_capture_byte_count ||
        result.capture_bytes_written != capture.size()) {
        return Fail("l1 sample runtime visual perspective image capture byte count mismatch");
    }

    if (result.image_artifact_report_count != L1_SAMPLE_016_FRAME_COUNT) {
        return Fail("l1 sample runtime visual perspective image artifact count mismatch");
    }

    const std::size_t expected_header_byte_count =
        sizeof(L1_SAMPLE_014_IMAGE_HEADER) - 1U;
    const std::size_t expected_file_byte_count =
        expected_header_byte_count + RuntimeVisualSceneTargetImageRgbByteCount();
    if (result.image_artifact_bytes_written !=
        expected_file_byte_count * L1_SAMPLE_016_FRAME_COUNT) {
        return Fail("l1 sample runtime visual perspective image total byte count mismatch");
    }

    for (std::size_t entity_index = 0U;
        entity_index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
        ++entity_index) {
        const RenderSceneRuntimeVisualSceneProofEntityReport &entity_report =
            result.entity_reports[entity_index];
        if (!entity_report.render_scene_submitted) {
            return Fail("l1 sample runtime visual perspective image entity was not submitted");
        }

        if (entity_report.primitive_kind != RuntimeVisualExpectedPrimitiveKind(entity_index)) {
            return Fail("l1 sample runtime visual perspective image primitive mismatch");
        }
    }

    for (std::size_t index = 0U; index < L1_SAMPLE_016_FRAME_COUNT; ++index) {
        const RenderSceneRuntimeVisualSceneImageArtifactReport &report =
            result.image_artifact_reports[index];
        if (report.status != RenderSceneRuntimeVisualSceneImageArtifactStatus::Written) {
            return Fail("l1 sample runtime visual perspective image status mismatch");
        }

        if (!RuntimeVisualScenePerspectiveTargetImagePathMatches(report)) {
            return Fail("l1 sample runtime visual perspective image path mismatch");
        }

        if (report.width != L1_SAMPLE_014_TARGET_IMAGE_WIDTH ||
            report.height != L1_SAMPLE_014_TARGET_IMAGE_HEIGHT ||
            report.source_byte_count != frame_capture_byte_count ||
            report.file_byte_count != expected_file_byte_count) {
            return Fail("l1 sample runtime visual perspective image metadata mismatch");
        }

        if (!FileStartsWithBytes(
                report.output_path,
                L1_SAMPLE_014_IMAGE_HEADER,
                expected_header_byte_count)) {
            return Fail("l1 sample runtime visual perspective image header mismatch");
        }

        const RenderSceneOrbitCaptureFrameReport &frame_report =
            result.orbit_result.frames[index];
        const RenderSceneRuntimeVisualSceneCameraTweenFrameReport &tween_report =
            result.camera_tween_frame_reports[index];

        if (frame_report.status != RenderSceneOrbitCaptureStatus::Success ||
            frame_report.capture_bytes_written != frame_capture_byte_count ||
            frame_report.capture.output_byte_budget != frame_capture_byte_count) {
            return Fail("l1 sample runtime visual perspective image frame metadata mismatch");
        }

        if (tween_report.frame_index != index ||
            tween_report.frame_id != request.first_frame_id + static_cast<std::uint32_t>(index) ||
            !CameraPoseMatches(tween_report.camera_pose, frame_report.camera_pose)) {
            return Fail("l1 sample runtime visual perspective image tween frame mismatch");
        }

        if (index == 0U &&
            !CameraTweenFrameMatchesKeyframe(tween_report, keyframes[0U])) {
            return Fail("l1 sample runtime visual perspective image start keyframe mismatch");
        }

        if (index == 4U &&
            !CameraTweenFrameMatchesKeyframe(tween_report, keyframes[1U])) {
            return Fail("l1 sample runtime visual perspective image middle keyframe mismatch");
        }

        if (index == L1_SAMPLE_016_FRAME_COUNT - 1U &&
            !CameraTweenFrameMatchesKeyframe(tween_report, keyframes[2U])) {
            return Fail("l1 sample runtime visual perspective image end keyframe mismatch");
        }

        if (!OrbitFrameCaptureSegmentWasWritten(capture, index, frame_capture_byte_count)) {
            return Fail("l1 sample runtime visual perspective image did not use route capture");
        }

        if (frame_report.capture_result.material_texture_slot_report_count !=
            RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
            return Fail("l1 sample runtime visual perspective image material slot count mismatch");
        }

        for (std::size_t slot_index = 0U;
            slot_index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
            ++slot_index) {
            const auto &slot_report =
                frame_report.capture_result.material_texture_slot_reports[slot_index];
            if (slot_report.slot != slot_index ||
                !slot_report.texture_resource_resolved ||
                !slot_report.sampled_texture_bound ||
                !slot_report.sampler_bound) {
                return Fail("l1 sample runtime visual perspective image material report mismatch");
            }
        }

        if (index > 0U &&
            !FilesHaveDifferentBytes(
                result.image_artifact_reports[index - 1U].output_path,
                report.output_path)) {
            return Fail("l1 sample runtime visual perspective image adjacent frames are identical");
        }
    }

    const RenderSceneRuntimeVisualSceneCameraTweenFrameReport &interpolated_frame =
        result.camera_tween_frame_reports[1U];
    if (interpolated_frame.source_keyframe_index != 0U ||
        interpolated_frame.target_keyframe_index != 1U ||
        interpolated_frame.linear_t <= 0.0F ||
        interpolated_frame.linear_t >= 1.0F ||
        Approx(interpolated_frame.linear_t, interpolated_frame.eased_t)) {
        return Fail("l1 sample runtime visual perspective image interpolation mismatch");
    }

    if (Approx(
            result.camera_tween_frame_reports[0U].vertical_fov_radians,
            result.camera_tween_frame_reports[4U].vertical_fov_radians) ||
        Approx(
            result.camera_tween_frame_reports[4U].vertical_fov_radians,
            result.camera_tween_frame_reports[L1_SAMPLE_016_FRAME_COUNT - 1U].vertical_fov_radians)) {
        return Fail("l1 sample runtime visual perspective image fov did not change");
    }

    if (!FilesHaveDifferentBytes(
            result.image_artifact_reports[0U].output_path,
            result.image_artifact_reports[L1_SAMPLE_016_FRAME_COUNT - 1U].output_path)) {
        return Fail("l1 sample runtime visual perspective image first and last frames are identical");
    }

    std::vector<std::uint8_t> first_frame_bytes{};
    if (!ReadBinaryFile(result.image_artifact_reports[0U].output_path, &first_frame_bytes)) {
        return Fail("l1 sample runtime visual perspective image read failed");
    }

    if (!RuntimeVisualFrameHasPerspectivePrimitiveCues(
            first_frame_bytes,
            expected_header_byte_count)) {
        return Fail("l1 sample runtime visual perspective image lacks 3d primitive cues");
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    const std::uint64_t expected_draw_count =
        static_cast<std::uint64_t>(L1_SAMPLE_016_FRAME_COUNT) *
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    if (snapshot.swapchain.resize_count != 1U ||
        snapshot.capture_count != expected_draw_count ||
        snapshot.last_capture_bytes_written != entity_capture_byte_count) {
        return Fail("l1 sample runtime visual perspective image rhi counter mismatch");
    }

    if (snapshot.last_capture_extent.width != L1_SAMPLE_014_TARGET_CAPTURE_WIDTH ||
        snapshot.last_capture_extent.height != L1_SAMPLE_014_TARGET_CAPTURE_HEIGHT) {
        return Fail("l1 sample runtime visual perspective image rhi extent mismatch");
    }

    return 0;
}

int RenderSceneL1Sample018BlendsTransparentRuntimePanel() {
    L1Vis001RhiDevice device;
    const std::size_t entity_capture_byte_count = L1VisCaptureByteCount();
    const std::size_t frame_capture_byte_count =
        entity_capture_byte_count * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    std::vector<std::uint8_t> capture(
        frame_capture_byte_count * L1_SAMPLE_016_FRAME_COUNT,
        CAPTURE_SENTINEL);

    RenderSceneRuntimeVisualSceneProofRoute route;
    RenderSceneRuntimeVisualSceneProofResult result{};
    const std::array<RenderSceneRuntimeVisualSceneCameraTweenKeyframe, 3U> keyframes =
        MakeRuntimeVisualSceneCameraTweenKeyframes();
    RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneProofRequest(device, capture);
    request.frame_count = L1_SAMPLE_016_FRAME_COUNT;
    request.camera_tween_requested = true;
    request.camera_tween_keyframes = keyframes;
    request.transparent_panel_blend_requested = true;
    const RenderSceneRuntimeVisualSceneProofStatus status = route.Execute(request, &result);
    if (status != RenderSceneRuntimeVisualSceneProofStatus::Success) {
        return Fail("l1 sample runtime visual transparent panel blend route did not complete");
    }

    if (result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::None ||
        result.diagnostic.status != RenderSceneMissingLayerDiagnosticStatus::Success) {
        return Fail("l1 sample runtime visual transparent panel blend reported missing layer");
    }

    if (!result.camera_tween_used || !result.camera_perspective_projection_used) {
        return Fail("l1 sample runtime visual transparent panel blend lost camera tween");
    }

    if (!result.transparent_panel_blend_used ||
        !result.transparent_panel_overlaps_background ||
        !result.transparent_panel_overlaps_primitive) {
        return Fail("l1 sample runtime visual transparent panel blend metadata mismatch");
    }

    if (result.transparent_panel_alpha == 0U || result.transparent_panel_alpha == 255U) {
        return Fail("l1 sample runtime visual transparent panel alpha was not translucent");
    }

    RhiBlendStateDesc alpha_state{};
    alpha_state.mode = RhiBlendMode::AlphaOver;
    alpha_state.constant_alpha = 255U;
    const RhiColor expected_background =
        BlendRhiColor(
            result.transparent_panel_source_color,
            result.transparent_panel_background_color,
            alpha_state);
    const RhiColor expected_primitive =
        BlendRhiColor(
            result.transparent_panel_source_color,
            result.transparent_panel_primitive_color,
            alpha_state);

    RhiBlendStateDesc opaque_state{};
    opaque_state.mode = RhiBlendMode::Opaque;
    const RhiColor expected_opaque =
        BlendRhiColor(
            result.transparent_panel_source_color,
            result.transparent_panel_background_color,
            opaque_state);

    if (!RhiColorsMatch(result.transparent_panel_blended_background_pixel, expected_background) ||
        !RhiColorsMatch(result.transparent_panel_blended_primitive_pixel, expected_primitive) ||
        !RhiColorsMatch(result.transparent_panel_opaque_pixel, expected_opaque)) {
        return Fail("l1 sample runtime visual transparent panel blend color math mismatch");
    }

    if (RhiColorsMatch(
            result.transparent_panel_blended_background_pixel,
            result.transparent_panel_opaque_pixel)) {
        return Fail("l1 sample runtime visual transparent panel background was opaque overwrite");
    }

    if (RhiColorsMatch(
            result.transparent_panel_blended_primitive_pixel,
            result.transparent_panel_opaque_pixel)) {
        return Fail("l1 sample runtime visual transparent panel primitive was opaque overwrite");
    }

    if (result.image_artifact_report_count != 0U ||
        result.image_artifact_bytes_written != 0U) {
        return Fail("l1 sample runtime visual transparent panel blend wrote unexpected image artifact");
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    const std::uint64_t expected_draw_count =
        static_cast<std::uint64_t>(L1_SAMPLE_016_FRAME_COUNT) *
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    const std::uint64_t expected_submit_count = expected_draw_count + 1U;
    if (snapshot.submit_count != expected_submit_count ||
        snapshot.submitted_indexed_draw_count != expected_draw_count ||
        snapshot.submitted_blend_state_bind_count != 1U) {
        return Fail("l1 sample runtime visual transparent panel blend rhi submit mismatch");
    }

    if (!snapshot.last_alpha_blend_enabled ||
        snapshot.last_blend_constant_alpha != 255U ||
        snapshot.rejected_blend_state_bind_count != 0U) {
        return Fail("l1 sample runtime visual transparent panel blend rhi state mismatch");
    }

    return 0;
}

int RenderSceneL1Sample019RendersTexturedGlassEmissiveMetalMaterials() {
    L1Vis001RhiDevice device;
    const std::size_t entity_capture_byte_count = L1VisCaptureByteCount();
    const std::size_t frame_capture_byte_count =
        entity_capture_byte_count * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    std::vector<std::uint8_t> capture(
        frame_capture_byte_count * L1_SAMPLE_016_FRAME_COUNT,
        CAPTURE_SENTINEL);

    RenderSceneRuntimeVisualSceneProofRoute route;
    RenderSceneRuntimeVisualSceneProofResult result{};
    const std::array<RenderSceneRuntimeVisualSceneCameraTweenKeyframe, 3U> keyframes =
        MakeRuntimeVisualSceneCameraTweenKeyframes();
    RenderSceneRuntimeVisualSceneProofRequest request =
        MakeRuntimeVisualSceneProofRequest(device, capture);
    request.frame_count = L1_SAMPLE_016_FRAME_COUNT;
    request.camera_tween_requested = true;
    request.camera_tween_keyframes = keyframes;
    request.transparent_panel_blend_requested = true;
    request.material_proof_requested = true;
    const RenderSceneRuntimeVisualSceneProofStatus status = route.Execute(request, &result);
    if (status != RenderSceneRuntimeVisualSceneProofStatus::Success) {
        return Fail("l1 sample runtime visual material proof route did not complete");
    }

    if (result.first_missing_layer != RenderSceneMissingLayerDiagnosticLayer::None ||
        result.diagnostic.status != RenderSceneMissingLayerDiagnosticStatus::Success) {
        return Fail("l1 sample runtime visual material proof reported missing layer");
    }

    if (!result.camera_tween_used ||
        !result.camera_perspective_projection_used ||
        !result.transparent_panel_blend_used) {
        return Fail("l1 sample runtime visual material proof lost prior visual layers");
    }

    if (!result.textured_material_used ||
        !result.textured_material_varies_from_pure_color ||
        !result.glass_material_used ||
        !result.emissive_material_used ||
        !result.emissive_material_brighter_than_diffuse ||
        !result.metal_material_used ||
        !result.metal_material_differs_from_diffuse) {
        return Fail("l1 sample runtime visual material proof metadata mismatch");
    }

    if (RhiColorsMatch(result.textured_material_sample_a, result.textured_material_sample_b)) {
        return Fail("l1 sample runtime visual textured material samples did not vary");
    }

    if (RhiColorsMatch(
            result.textured_material_sample_a,
            result.textured_material_flat_reference) ||
        RhiColorsMatch(
            result.textured_material_sample_b,
            result.textured_material_flat_reference)) {
        return Fail("l1 sample runtime visual textured material matched flat color");
    }

    RhiBlendStateDesc alpha_state{};
    alpha_state.mode = RhiBlendMode::AlphaOver;
    alpha_state.constant_alpha = 255U;
    const RhiColor expected_glass =
        BlendRhiColor(
            result.transparent_panel_source_color,
            result.transparent_panel_primitive_color,
            alpha_state);
    if (!RhiColorsMatch(result.glass_material_blended_pixel, expected_glass)) {
        return Fail("l1 sample runtime visual glass material blend math mismatch");
    }

    if (RhiColorsMatch(
            result.glass_material_blended_pixel,
            result.glass_material_opaque_pixel)) {
        return Fail("l1 sample runtime visual glass material was opaque overwrite");
    }

    if (RhiColorLight(result.emissive_material_pixel) <=
        RhiColorLight(result.emissive_material_diffuse_reference)) {
        return Fail("l1 sample runtime visual emissive material was not brighter than diffuse");
    }

    if (RhiColorsMatch(
            result.metal_material_pixel,
            result.metal_material_diffuse_reference)) {
        return Fail("l1 sample runtime visual metal material matched diffuse");
    }

    if (RhiColorLight(result.metal_material_pixel) <=
        RhiColorLight(result.metal_material_diffuse_reference)) {
        return Fail("l1 sample runtime visual metal material lacked highlight evidence");
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    const std::uint64_t expected_draw_count =
        static_cast<std::uint64_t>(L1_SAMPLE_016_FRAME_COUNT) *
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    const std::uint64_t expected_submit_count = expected_draw_count + 1U;
    if (snapshot.submit_count != expected_submit_count ||
        snapshot.submitted_indexed_draw_count != expected_draw_count ||
        snapshot.submitted_blend_state_bind_count != 1U) {
        return Fail("l1 sample runtime visual material proof rhi submit mismatch");
    }

    if (!snapshot.last_alpha_blend_enabled ||
        snapshot.rejected_blend_state_bind_count != 0U) {
        return Fail("l1 sample runtime visual material proof blend state mismatch");
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

int RenderSceneRuntimeVisualFoundationNoEditorUiInputDependency() {
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

    if (name == TEST_MATERIAL_CONSTANTS) {
        return RenderSceneRuntimeMaterialCopiesMaterialConstants();
    }

    if (name == TEST_MATERIAL_CONSTANT_OVERFLOW) {
        return RenderSceneRuntimeMaterialRejectsOversizedMaterialConstants();
    }

    if (name == TEST_MATERIAL_BLEND_STATE) {
        return RenderSceneRuntimeMaterialCopiesBlendState();
    }

    if (name == TEST_MATERIAL_INVALID_BLEND_STATE) {
        return RenderSceneRuntimeMaterialRejectsInvalidBlendStateWithoutMutation();
    }

    if (name == TEST_FRAME_THREE_ENTITIES) {
        return RenderSceneRuntimeFrameSubmitsThreeEntitiesWithSharedMaterial();
    }

    if (name == TEST_FRAME_PER_ENTITY_MATERIALS) {
        return RenderSceneRuntimeFrameSubmitsEntitiesWithPerEntityMaterials();
    }

    if (name == TEST_FRAME_MATERIAL_INDEX_RANGE) {
        return RenderSceneRuntimeFrameRejectsMaterialTableIndexOutOfRange();
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

    if (name == TEST_L1_VIS_ANIMATED_TRANSFORM) {
        return RenderSceneL1Vis004CapturesAnimatedTransformSceneThroughRuntimeRoute();
    }

    if (name == TEST_L1_VIS_ORBIT_CAPTURE) {
        return RenderSceneL1Vis005CapturesDeterministicOrbitSequence();
    }

    if (name == TEST_L1_VIS_MISSING_LAYER_DIAGNOSTIC) {
        return RenderSceneL1Vis006ReportsExactMissingLayers();
    }

    if (name == TEST_L1_SAMPLE_011_RUNTIME_VISUAL_SCENE) {
        return RenderSceneL1Sample011CapturesFinalRuntimeVisualScene();
    }

    if (name == TEST_L1_SAMPLE_011_RUNTIME_VISUAL_IMAGE_ARTIFACTS) {
        return RenderSceneL1Sample011EmitsFinalOrbitImageArtifacts();
    }

    if (name == TEST_L1_SAMPLE_012_RUNTIME_VISUAL_BLOCKER) {
        return RenderSceneL1Sample012ReportsExactRuntimeVisualBlocker();
    }

    if (name == TEST_L1_SAMPLE_013_USER_VISIBLE_RESOLUTION_BLOCKER) {
        return RenderSceneL1Sample013ReportsUserVisibleCaptureResolutionBlocker();
    }

    if (name == TEST_L1_SAMPLE_014_USER_VISIBLE_TARGET) {
        return RenderSceneL1Sample014EmitsUserVisibleCaptureTargetArtifacts();
    }

    if (name == TEST_L1_SAMPLE_015_SCENE_PIXEL_SEMANTICS) {
        return RenderSceneL1Sample015EmitsScenePixelSemanticsArtifacts();
    }

    if (name == TEST_L1_SAMPLE_016_PERSPECTIVE_3D_CAMERA_TWEEN) {
        return RenderSceneL1Sample016EmitsPerspective3DPrimitiveCameraTweenArtifacts();
    }

    if (name == TEST_L1_SAMPLE_018_TRANSPARENT_PANEL_BLEND) {
        return RenderSceneL1Sample018BlendsTransparentRuntimePanel();
    }

    if (name == TEST_L1_SAMPLE_019_TEXTURED_GLASS_EMISSIVE_METAL) {
        return RenderSceneL1Sample019RendersTexturedGlassEmissiveMetalMaterials();
    }

    if (name == TEST_BOUNDARY) {
        return RenderSceneRuntimeVisualFoundationNoEditorUiInputDependency();
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
