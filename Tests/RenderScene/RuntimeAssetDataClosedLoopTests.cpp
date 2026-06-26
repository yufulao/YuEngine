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
#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Package/PackageArtifact.h"
#include "YuEngine/Kernel/RuntimeFramePhase.h"
#include "YuEngine/Package/PackageEntryDescriptor.h"
#include "YuEngine/Package/PackageLoadPlanResult.h"
#include "YuEngine/Package/PackageRegistry.h"
#include "YuEngine/Package/PackageSourceKey.h"
#include "YuEngine/Package/PackageStatus.h"
#include "YuEngine/Platform/PlatformNativeSurface.h"
#include "YuEngine/Platform/PlatformWindowDesc.h"
#include "YuEngine/Platform/PlatformWindowStatus.h"
#include "YuEngine/Platform/WindowsPlatformWindow.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipeline.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineRequest.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h"
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
#include "YuEngine/ResourceBrowser/ResourceBrowserDiagnostics.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceCreateResult.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceFactory.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiNativeSurfaceDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"
#include "YuEngine/PreviewHost/PreviewHost.h"
#include "YuEngine/ResourceBrowser/ResourceBrowserSurface.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridge.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeRequest.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeResult.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"
#include "YuEngine/World/WorldSceneAuthoringDocument.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"

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
using yuengine::kernel::RuntimeFramePhase;
using yuengine::package::PackageArtifactDependency;
using yuengine::package::PackageArtifactReadRequest;
using yuengine::package::PackageArtifactResult;
using yuengine::package::PackageArtifactWriteRequest;
using yuengine::package::PackageEntryDescriptor;
using yuengine::package::PackageEntryId;
using yuengine::package::PackageId;
using yuengine::package::PackageLoadPlan;
using yuengine::package::PackageLoadPlanResult;
using yuengine::package::PackageRegistry;
using yuengine::package::PackageSourceKey;
using yuengine::package::PackageStatus;
using yuengine::package::ReadPackageArtifact;
using yuengine::package::WritePackageArtifact;
using yuengine::platform::PlatformNativeSurface;
using yuengine::platform::PlatformWindowDesc;
using yuengine::platform::PlatformWindowStatus;
using yuengine::platform::WindowsPlatformWindow;
using yuengine::previewhost::PreviewHost;
using yuengine::previewhost::PreviewHostCommandOutputRef;
using yuengine::previewhost::PreviewHostDiagnostic;
using yuengine::previewhost::PreviewHostDiagnosticCode;
using yuengine::previewhost::PreviewHostDocumentKind;
using yuengine::previewhost::PreviewHostFrameDescriptor;
using yuengine::previewhost::PreviewHostFrameFormat;
using yuengine::previewhost::PreviewHostFrameRequest;
using yuengine::previewhost::PreviewHostFrameResult;
using yuengine::previewhost::PreviewHostHitRecord;
using yuengine::previewhost::PreviewHostResourceBrowserPreviewRequest;
using yuengine::previewhost::PreviewHostResourceBrowserPreviewResult;
using yuengine::previewhost::PreviewHostSelectionRecord;
using yuengine::previewhost::PreviewHostSessionDesc;
using yuengine::previewhost::PreviewHostSessionId;
using yuengine::previewhost::PreviewHostSessionResult;
using yuengine::previewhost::PreviewHostStatus;
using yuengine::previewhost::PreviewHostTransformFeedback;
using yuengine::previewhost::PreviewHostSceneDocumentViewportBlockedLayer;
using yuengine::previewhost::PreviewHostSceneDocumentViewportRequest;
using yuengine::previewhost::PreviewHostSceneDocumentViewportResult;
using yuengine::previewhost::PreviewHostViewportSessionRequest;
using yuengine::previewhost::PreviewHostViewportSessionResult;
using yuengine::rendercore::RenderDrawableFramePipeline;
using yuengine::rendercore::RenderDrawableFramePipelineRequest;
using yuengine::rendercore::RenderDrawableFramePipelineStatus;
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
using yuengine::resourcebrowser::BuildResourceBrowserRuntimeAssetDiagnostics;
using yuengine::resourcebrowser::ResourceBrowserDependencyState;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticCode;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticPhase;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticRecord;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticsRequest;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticsResult;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticsStatus;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticSeverity;
using yuengine::resourcebrowser::ResourceBrowserResourceEntry;
using yuengine::resourcebrowser::ResourceBrowserAssetManagerGap;
using yuengine::resourcebrowser::ResourceBrowserAssetManagerGapRow;
using yuengine::resourcebrowser::ResourceBrowserDepthCatalogRow;
using yuengine::resourcebrowser::ResourceBrowserExternalAuthoringSourceRow;
using yuengine::resourcebrowser::ResourceBrowserImporterBoundaryRow;
using yuengine::resourcebrowser::ResourceBrowserImporterCommitRejectedLayer;
using yuengine::resourcebrowser::ResourceBrowserImporterCommitSelectionLedgerRecord;
using yuengine::resourcebrowser::ResourceBrowserImporterCommitWorkflowRequest;
using yuengine::resourcebrowser::ResourceBrowserImporterCommitWorkflowResult;
using yuengine::resourcebrowser::ResourceBrowserImporterCommitWorkflowStatus;
using yuengine::resourcebrowser::ResourceBrowserImporterReadiness;
using yuengine::resourcebrowser::ResourceBrowserImportSettings;
using yuengine::resourcebrowser::ResourceBrowserSourceBoundary;
using yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState;
using yuengine::resourcebrowser::ResourceBrowserSurfaceRequest;
using yuengine::resourcebrowser::ResourceBrowserSurfaceResult;
using yuengine::resourcebrowser::ResourceBrowserSurfaceRow;
using yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionRequest;
using yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionResult;
using yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionStatus;
using yuengine::resourcebrowser::ResourceBrowserSurfaceStatus;
using yuengine::resourcebrowser::BuildResourceBrowserImporterCommitWorkflow;
using yuengine::resourcebrowser::BuildResourceBrowserNativeSurface;
using yuengine::resourcebrowser::ResolveResourceBrowserSurfaceSelection;
using yuengine::runtimeasset::HashRuntimeAssetDataBytes;
using yuengine::runtimeasset::BuildRuntimeAssetRenderSceneSubmission;
using yuengine::runtimeasset::BuildRuntimeAssetCookedVisualProofRoute;
using yuengine::runtimeasset::BuildRuntimeAssetCookedShaderProgramPipeline;
using yuengine::runtimeasset::BuildRuntimeAssetCookedTextureMaterialBridge;
using yuengine::runtimeasset::BuildRuntimeAssetShaderProgramPipeline;
using yuengine::runtimeasset::CompileRuntimeAssetShaderProgram;
using yuengine::runtimeasset::DecodeRuntimeAssetShaderProgramData;
using yuengine::runtimeasset::ExecuteRuntimeAssetImportCookCommand;
using yuengine::runtimeasset::LoadRuntimeAssetDataGraph;
using yuengine::runtimeasset::PackRuntimeAssetMaterialConstants;
using yuengine::runtimeasset::RUNTIME_ASSET_PACKED_MATERIAL_CONSTANT_BYTES;
using yuengine::runtimeasset::RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT;
using yuengine::runtimeasset::RuntimeAssetArtifactClass;
using yuengine::runtimeasset::RuntimeAssetCookedMaterialSlotDesc;
using yuengine::runtimeasset::RuntimeAssetCookedProgramDesc;
using yuengine::runtimeasset::RuntimeAssetCookedProgramPipelineClass;
using yuengine::runtimeasset::RuntimeAssetCookedShaderBytecodeFormat;
using yuengine::runtimeasset::RuntimeAssetCookedShaderProgramPipelineRequest;
using yuengine::runtimeasset::RuntimeAssetCookedShaderProgramPipelineResult;
using yuengine::runtimeasset::RuntimeAssetCookedShaderStagePayloadDesc;
using yuengine::runtimeasset::RuntimeAssetCookedTextureColorSpace;
using yuengine::runtimeasset::RuntimeAssetCookedTextureMaterialBridgeRequest;
using yuengine::runtimeasset::RuntimeAssetCookedTextureMaterialBridgeResult;
using yuengine::runtimeasset::RuntimeAssetCookedTexturePayloadDesc;
using yuengine::runtimeasset::RuntimeAssetDataStatus;
using yuengine::runtimeasset::RuntimeAssetDeterministicDiskFixtureResult;
using yuengine::runtimeasset::RuntimeAssetFileDesc;
using yuengine::runtimeasset::RuntimeAssetFileKind;
using yuengine::runtimeasset::RuntimeAssetGraphLoadRequest;
using yuengine::runtimeasset::RuntimeAssetGraphLoadResult;
using yuengine::runtimeasset::RuntimeAssetImportCookCommandKind;
using yuengine::runtimeasset::RuntimeAssetImportCookCommandRequest;
using yuengine::runtimeasset::RuntimeAssetImportCookCommandResult;
using yuengine::runtimeasset::RuntimeAssetImportCookMissingLayer;
using yuengine::runtimeasset::RuntimeAssetLoadedFile;
using yuengine::runtimeasset::RuntimeAssetLoadedShaderProgramData;
using yuengine::runtimeasset::RuntimeAssetLoadTransactionPhase;
using yuengine::runtimeasset::RuntimeAssetPackedMaterialConstants;
using yuengine::runtimeasset::RuntimeAssetPackageArtifactProductRunMissingLayer;
using yuengine::runtimeasset::RuntimeAssetPackageArtifactProductRunRequest;
using yuengine::runtimeasset::RuntimeAssetPackageArtifactProductRunResult;
using yuengine::runtimeasset::RuntimeAssetPackagedRunBlockedLayer;
using yuengine::runtimeasset::RuntimeAssetPackagedRunRequest;
using yuengine::runtimeasset::RuntimeAssetPackagedRunResult;
using yuengine::runtimeasset::RuntimeAssetRenderSceneGeometryBinding;
using yuengine::runtimeasset::RuntimeAssetRenderSceneMaterialBinding;
using yuengine::runtimeasset::RuntimeAssetRenderSceneSubmissionRequest;
using yuengine::runtimeasset::RuntimeAssetRenderSceneSubmissionResult;
using yuengine::runtimeasset::RuntimeAssetSceneCameraRecord;
using yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using yuengine::runtimeasset::RuntimeAssetSceneLoaderOutput;
using yuengine::runtimeasset::RuntimeAssetSceneResourceRef;
using yuengine::runtimeasset::RuntimeAssetSceneTransformOutputRecord;
using yuengine::runtimeasset::RuntimeAssetShaderCompilerBackendKind;
using yuengine::runtimeasset::RuntimeAssetShaderCompilerBackendRequest;
using yuengine::runtimeasset::RuntimeAssetShaderCompilerBackendResult;
using yuengine::runtimeasset::RuntimeAssetShaderProgramPipelineRequest;
using yuengine::runtimeasset::RuntimeAssetShaderProgramPipelineResult;
using yuengine::runtimeasset::RuntimeAssetValidationResult;
using yuengine::runtimeasset::RuntimeAssetVisualProofMissingLayer;
using yuengine::runtimeasset::RuntimeAssetVisualProofRequest;
using yuengine::runtimeasset::RuntimeAssetVisualProofResult;
using yuengine::runtimeasset::RunRuntimeAssetPackagedEntryPoint;
using yuengine::runtimeasset::RunRuntimeAssetPackageArtifactProductCommand;
using yuengine::runtimeasset::ValidateRuntimeAssetDataBytes;
using yuengine::streaming::ResourceDecodedTextureBridge;
using yuengine::streaming::ResourceDecodedTextureBridgeRequest;
using yuengine::streaming::ResourceDecodedTextureBridgeResult;
using yuengine::streaming::ResourceDecodedTextureBridgeStatus;
using yuengine::rhi::IRhiDevice;
using yuengine::rhi::MAX_COMMANDS;
using yuengine::rhi::NullRhiDevice;
using yuengine::rhi::RhiBackendKind;
using yuengine::rhi::RhiBufferDesc;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiColorTargetDesc;
using yuengine::rhi::RhiConstantBufferBinding;
using yuengine::rhi::RhiDeviceCreateResult;
using yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiDeviceFactory;
using yuengine::rhi::RhiFormat;
using yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiInputElementFormat;
using yuengine::rhi::RhiInputElementSemantic;
using yuengine::rhi::RhiInputLayoutDesc;
using yuengine::rhi::RhiNativeSurfaceDesc;
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
using yuengine::world::WorldSceneAuthoringDependencyRecord;
using yuengine::world::WorldSceneAuthoringDocument;
using yuengine::world::WorldSceneAuthoringDocumentStatus;
using yuengine::world::WorldSceneAuthoringRuntimeExport;
using yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;
using yuengine::world::WorldTransformState;

constexpr const char *TEST_GENERATOR =
    "RuntimeAssetData_GeneratorWritesDeterministicFilesAndHashes";
constexpr const char *TEST_IMPORT_COOK_COMMAND_WRITES =
    "RuntimeAssetData_ImportCookCommandWritesSourceAndCookedDiskFixtures";
constexpr const char *TEST_IMPORT_COOK_COMMAND_LOADS =
    "RuntimeAssetData_ImportCookCommandLoadsGeneratedSourceAndCookedViaFileResourceRoute";
constexpr const char *TEST_IMPORT_COOK_COMMAND_MISSING_LAYER =
    "RuntimeAssetData_ImportCookCommandReportsMissingLayerStatus";
constexpr const char *TEST_UNSUPPORTED_VERSION =
    "RuntimeAssetData_FormatHeaderRejectsUnsupportedVersion";
constexpr const char *TEST_INVALID_BOUNDS =
    "RuntimeAssetData_ValidatorRejectsInvalidBoundsWithoutOutputs";
constexpr const char *TEST_TYPED_MESH_MATERIAL_TEXTURE =
    "RuntimeAssetData_MeshMaterialTextureTypedValidatorsAcceptStructuredMetadata";
constexpr const char *TEST_MESH_PAYLOAD_POLICY =
    "RuntimeAssetData_MeshPayloadPolicyRejectsSizeHashAndSplitMismatch";
constexpr const char *TEST_MESH_LAYOUT_TOPOLOGY =
    "RuntimeAssetData_MeshLayoutTopologyDecodesIntoLoadedRecords";
constexpr const char *TEST_MESH_PAYLOAD_DECODED_BUFFERS =
    "RuntimeAssetData_ImportedMeshPayloadBytesFeedRenderGeometryBuffers";
constexpr const char *TEST_MATERIAL_TYPED_REFS =
    "RuntimeAssetData_MaterialValidatorRejectsMissingDuplicateAndTypeMismatchRefs";
constexpr const char *TEST_MATERIAL_PARAMETER_SEMANTICS =
    "RuntimeAssetData_MaterialParameterSemanticsLoadIntoRuntimeRecords";
constexpr const char *TEST_MATERIAL_CONSTANT_PACK =
    "RuntimeAssetData_MaterialConstantsPackLoadedParameters";
constexpr const char *TEST_TEXTURE_TYPED_METADATA =
    "RuntimeAssetData_TextureValidatorRejectsInvalidFormatExtentPayload";
constexpr const char *TEST_SHADER_SCENE_ANIMATION_SCHEMA =
    "RuntimeAssetData_ShaderSceneAnimationRequireSourceSchema";
constexpr const char *TEST_INVALID_DEPENDENCY =
    "RuntimeAssetData_DependencyGraphRejectsMissingAndDuplicateRefs";
constexpr const char *TEST_VALIDATOR_MISSING_DEPENDENCY_TOKEN =
    "RuntimeAssetData_ValidatorReportsMissingDependencyToken";
constexpr const char *TEST_VALIDATOR_DUPLICATE_DEPENDENCY_TOKEN =
    "RuntimeAssetData_ValidatorReportsDuplicateDependencyToken";
constexpr const char *TEST_VALIDATOR_TYPE_MISMATCH_EXPECTED_ACTUAL =
    "RuntimeAssetData_ValidatorReportsTypeMismatchExpectedActual";
constexpr const char *TEST_COOKED_DEPENDENCY_FAILED_DEP_INDEX =
    "RuntimeAssetData_CookedDependencyRowsReportFailedDepIndex";
constexpr const char *TEST_SHADER_IMPORT_POLICY =
    "RuntimeAssetData_ShaderImportPolicyValidatesSourceCookedAndLoadedRecords";
constexpr const char *TEST_SHADER_COMPILER_BACKEND =
    "RuntimeAssetData_ShaderCompilerBackendProducesProgramReflection";
constexpr const char *TEST_SHADER_PROGRAM_PIPELINE_BRIDGE =
    "RuntimeAssetData_ShaderProgramBridgeCreatesRhiPipelineFromLoadedBytecode";
constexpr const char *TEST_SHADER_PROGRAM_PIPELINE_REJECTS =
    "RuntimeAssetData_ShaderProgramBridgeRejectsInvalidProgramDataWithoutRhiMutation";
constexpr const char *TEST_COOKED_SHADER_STAGE_MODULES =
    "RuntimeAssetData_CookedShaderStagePayloadsCreateRhiModules";
constexpr const char *TEST_COOKED_PROGRAM_PIPELINE_REFLECTION =
    "RuntimeAssetData_CookedProgramPipelineUsesLoadedReflectionAndInputLayout";
constexpr const char *TEST_COOKED_SHADER_PAYLOAD_REJECTS =
    "RuntimeAssetData_CookedShaderPayloadRejectsStageBytecodeHashAndReflectionMismatchWithoutMutation";
constexpr const char *TEST_COOKED_SHADER_PROGRAM_RHI_CLEANUP =
    "RuntimeAssetData_CookedShaderProgramRhiPartialCreationFailureDestroysTransientHandles";
constexpr const char *TEST_LOADER_FILE_RESOURCE =
    "RuntimeAssetData_LoaderUsesFileResourcePathNotInMemoryStructs";
constexpr const char *TEST_SCENE_REFERENCES =
    "RuntimeAssetData_SceneReferencesMeshMaterialTextureShader";
constexpr const char *TEST_CAMERA_TWEEN_DESCRIPTOR =
    "RuntimeAssetData_CameraTweenDescriptorLoadsFromDiskSceneReference";
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
constexpr const char *TEST_LOADER_TRANSACTION_FILE_COUNT_PREFLIGHT =
    "RuntimeAssetData_LoaderRejectsOversizedFileCountBeforeReadAndMutation";
constexpr const char *TEST_SHADER_PROGRAM_DEPENDENCIES =
    "RuntimeAssetData_ShaderProgramDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs";
constexpr const char *TEST_SCENE_CAMERA_ANIMATION_DEPENDENCIES =
    "RuntimeAssetData_SceneCameraAnimationDependencyValidatorRejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_ANIMATION_DEPENDENCIES =
    "RuntimeAssetData_AnimationDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs";
constexpr const char *TEST_LOADED_RENDER_RECORDS =
    "RuntimeAssetData_LoadCreatesRenderSceneRuntimeRecords";
constexpr const char *TEST_GENERIC_RENDER_SCENE_SUBMISSION =
    "RuntimeAssetData_GenericRenderSceneSubmissionBuildsFrameFromLoadedSceneRecords";
constexpr const char *TEST_GENERIC_RENDER_SCENE_SUBMISSION_MESH_REFS =
    "RuntimeAssetData_GenericRenderSceneSubmissionUsesMeshRefsNotEntityOrder";
constexpr const char *TEST_GENERIC_RENDER_SCENE_SUBMISSION_MISSING_TRANSFORM =
    "RuntimeAssetData_GenericRenderSceneSubmissionRejectsMissingTransformWithoutMutation";
constexpr const char *TEST_GENERIC_RENDER_SCENE_SUBMISSION_MISSING_MESH =
    "RuntimeAssetData_GenericRenderSceneSubmissionRejectsMissingMeshRefWithoutMutation";
constexpr const char *TEST_GENERIC_RENDER_SCENE_SUBMISSION_MISSING_MATERIAL =
    "RuntimeAssetData_GenericRenderSceneSubmissionRejectsMissingMaterialRefWithoutMutation";
constexpr const char *TEST_GENERIC_RENDER_SCENE_SUBMISSION_MATERIAL_VARIANTS =
    "RuntimeAssetData_GenericRenderSceneSubmissionReportsMaterialVariantsUntilFrameApiSupportsThem";
constexpr const char *TEST_PRODUCTION_SCENE_LOADER_OUTPUT =
    "RuntimeAssetData_ProductionSceneLoaderOutputsDeterministicRecords";
constexpr const char *TEST_DISK_ANIMATION_SAMPLING =
    "RuntimeAssetData_DiskAnimationSamplingFeedsSceneTransforms";
constexpr const char *TEST_SCENE_LOADER_INVALID_ENTITY_NO_MUTATION =
    "RuntimeAssetData_SceneLoaderRejectsInvalidEntityWithoutOutputMutation";
constexpr const char *TEST_SCENE_LOADER_INVALID_KEYFRAME_NO_MUTATION =
    "RuntimeAssetData_SceneLoaderRejectsInvalidKeyframesWithoutOutputMutation";
constexpr const char *TEST_SCENE_ANIMATION_BOUNDED_N_ENTITY =
    "RuntimeAssetData_SceneAnimationLoaderLoadsBoundedNEntityScene";
constexpr const char *TEST_SCENE_ANIMATION_CAPACITY_NO_MUTATION =
    "RuntimeAssetData_SceneAnimationLoaderRejectsEntityCapacityOverflowWithoutMutation";
constexpr const char *TEST_SCENE_ANIMATION_REFS_NO_MUTATION =
    "RuntimeAssetData_SceneAnimationLoaderRejectsMissingRefsWithoutMutation";
constexpr const char *TEST_SCENE_ANIMATION_INVALID_RECORDS_NO_MUTATION =
    "RuntimeAssetData_SceneAnimationLoaderRejectsInvalidRecordsWithoutMutation";
constexpr const char *TEST_SCENE_CAMERA_FAMILY_NO_MUTATION =
    "RuntimeAssetData_SceneAnimationLoaderRejectsCameraFamilyFailuresWithoutMutation";
constexpr const char *TEST_SCENE_ANIMATION_PATH_INDEPENDENT =
    "RuntimeAssetData_SceneAnimationLoaderPathIndependentSceneAnimationDetection";
constexpr const char *TEST_DECODED_PAYLOADS =
    "RuntimeAssetData_CookStoresDecodedPayloadsForMeshMaterialTexture";
constexpr const char *TEST_TEXTURE_MATERIAL_SLOT_BRIDGE =
    "RuntimeAssetData_DecodedTexturePayloadsDriveRhiMaterialSlots";
constexpr const char *TEST_TEXTURE_MATERIAL_SLOT_BRIDGE_FAILURES =
    "RuntimeAssetData_TextureMaterialSlotBridgeFailuresDoNotMutateRenderSceneOutputs";
constexpr const char *TEST_COOKED_TEXTURE_PAYLOAD_LAYOUT =
    "RuntimeAssetData_CookedTexturePayloadTableValidatesLayoutHashAndRowPitch";
constexpr const char *TEST_COOKED_MATERIAL_SLOT_TABLE =
    "RuntimeAssetData_CookedMaterialTextureSlotTableResolvesLoadedPayloads";
constexpr const char *TEST_COOKED_MATERIAL_CONSTANTS =
    "RuntimeAssetData_CookedMaterialConstantsBridgeToRenderSceneRecord";
constexpr const char *TEST_COOKED_MATERIAL_CONSTANT_REJECTS =
    "RuntimeAssetData_CookedMaterialConstantsRejectInvalidLoadedMaterialWithoutMutation";
constexpr const char *TEST_COOKED_PAYLOAD_DESCRIPTOR_REJECTS =
    "RuntimeAssetData_CookedPayloadBridgeRejectsTextureFormatExtentSizeAlignmentHashWithoutMutation";
constexpr const char *TEST_COOKED_SLOT_DEPENDENCY_REJECTS =
    "RuntimeAssetData_CookedPayloadBridgeRejectsMissingDuplicateTypeMismatchDepsWithoutMutation";
constexpr const char *TEST_COOKED_SLOT_OVERFLOW =
    "RuntimeAssetData_CookedMaterialSlotOverflowDoesNotMutateRenderSceneOutputs";
constexpr const char *TEST_COOKED_RHI_CLEANUP =
    "RuntimeAssetData_CookedRhiPartialCreationFailureDestroysTransientHandles";
constexpr const char *TEST_COOKED_VISUAL_PROOF =
    "RuntimeAssetData_CookedRecordsDriveRuntimeVisualProofThroughRenderCoreRhi";
constexpr const char *TEST_COOKED_VISUAL_PROOF_D3D11 =
    "RuntimeAssetData_D3D11Hardware_CookedRecordsDriveDeviceBackedVisualProof";
constexpr const char *TEST_COOKED_VISUAL_PROOF_MISSING_LAYERS =
    "RuntimeAssetData_CookedRuntimeVisualProofReportsExactMissingLayers";
constexpr const char *TEST_PACKAGE_COOK_RUN_SMOKE =
    "RuntimeAssetData_PackageCookRunSmokeRunsPackagedRuntimeEntryPoint";
constexpr const char *TEST_PACKAGE_ARTIFACT_PRODUCT_RUN =
    "RuntimeAssetData_PackageArtifactCookRunSmokeRunsProductRuntimeEntryPoint";
constexpr const char *TEST_PRODUCT_RUN_COMMAND =
    "RuntimeAssetData_ProductRunCommandConsumesPackageArtifactPath";
constexpr const char *TEST_PRODUCT_RUN_COMMAND_MISSING_ARTIFACT =
    "RuntimeAssetData_ProductRunCommandReportsMissingPackageArtifactPath";
constexpr const char *TEST_RUNTIME_DEPENDENCIES =
    "RuntimeAssetData_LoadRegistersResourceAndAssetDependencyEdges";
constexpr const char *TEST_LOAD_RENDER =
    "RuntimeAssetData_RenderClosedLoop_CapturesCubeCylinderConeThroughRhi";
constexpr const char *TEST_CPU_ORACLE =
    "RuntimeAssetData_CpuPpmOracleDoesNotBypassRhiRenderCore";
constexpr const char *TEST_NO_UPPER =
    "RuntimeAssetData_DoesNotDependOnEditorWebUiInputOrGdiViewer";
constexpr const char *TEST_PREVIEW_HOST_CAPTURE =
    "PreviewHost_ConsumesRuntimeAssetGraphAndCapturesThroughRhi";
constexpr const char *TEST_PREVIEW_HOST_COMMAND_OUTPUT =
    "PreviewHost_ConsumesImportCookCommandOutputs";
constexpr const char *TEST_PREVIEW_HOST_DIAGNOSTICS =
    "PreviewHost_ReportsBoundedResourceDiagnostics";
constexpr const char *TEST_PREVIEW_HOST_RESOURCE_BROWSER_DECISION =
    "PreviewHost_UsesResourceBrowserDiagnosticsForPreviewDecision";
constexpr const char *TEST_PREVIEW_HOST_STALE_SESSION =
    "PreviewHost_RejectsStaleSessionWithoutMutation";
constexpr const char *TEST_PREVIEW_HOST_NOT_COOKED =
    "PreviewHost_ReportsNotCookedRuntimeAssetRef";
constexpr const char *TEST_PREVIEW_HOST_VIEWPORT_SURFACE =
    "PreviewHost_BuildsViewportSessionSurfaceFromResourceBrowserSelection";
constexpr const char *TEST_PREVIEW_HOST_IMPORTER_COMMIT_VIEWPORT =
    "PreviewHost_ConsumesResourceBrowserImporterCommitOutputs";
constexpr const char *TEST_PREVIEW_HOST_VIEWPORT_SURFACE_BLOCKED =
    "PreviewHost_RejectsBlockedViewportSelectionWithoutFrameMutation";
constexpr const char *TEST_PREVIEW_HOST_SCENE_DOCUMENT_BRIDGE =
    "PreviewHost_BuildsViewportSessionFromSceneAuthoringDocument";
constexpr const char *TEST_PREVIEW_HOST_SCENE_DOCUMENT_BRIDGE_REJECTS =
    "PreviewHost_RejectsInvalidSceneAuthoringDocumentWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char *MOUNT_ID = "runtime";
constexpr const char *SCENE_PATH = "Scene/CanonicalScene.yuscene";
constexpr std::uint64_t FNV_OFFSET = 14695981039346656037ULL;
constexpr std::uint64_t FNV_PRIME = 1099511628211ULL;
constexpr PackageId RUNTIME_ASSET_SMOKE_PACKAGE{7001U};
constexpr PackageEntryId RUNTIME_ASSET_SMOKE_SCENE_ENTRY{100U};
constexpr const char *RUNTIME_ASSET_SMOKE_PACKAGE_ARTIFACT_PATH =
    "Package/RuntimeAssetSmoke.yupackage";
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
constexpr std::uint32_t RESOURCE_TYPE_CAMERA = 107U;
constexpr std::uint32_t ASSET_TYPE_MESH = 201U;
constexpr std::uint32_t ASSET_TYPE_MATERIAL = 202U;
constexpr std::uint32_t ASSET_TYPE_TEXTURE = 203U;
constexpr std::uint32_t ASSET_TYPE_SHADER = 204U;
constexpr std::uint32_t ASSET_TYPE_SCENE = 205U;
constexpr std::uint32_t ASSET_TYPE_ANIMATION = 206U;
constexpr std::uint32_t ASSET_TYPE_CAMERA = 207U;
constexpr std::size_t FIXTURE_FILE_COUNT = 10U;
constexpr std::size_t CAPTURE_BYTES_PER_ENTITY = 64U;
constexpr std::size_t TOTAL_CAPTURE_BYTES = CAPTURE_BYTES_PER_ENTITY * 3U;
constexpr std::uint32_t AUTHORING_SENTINEL_COUNT = 0xCAFEU;
constexpr int SKIP_RETURN_CODE = 77;
constexpr std::uint32_t MATERIAL_PARAMETER_COUNT = 5U;
constexpr std::uint32_t MATERIAL_BASE_COLOR_RGBA = 0x203040C0U;
constexpr std::uint32_t MATERIAL_EMISSIVE_STRENGTH = 64U;
constexpr std::uint32_t MATERIAL_METALLIC = 128U;
constexpr std::uint32_t MATERIAL_ROUGHNESS = 96U;
constexpr std::uint32_t MATERIAL_OPACITY = 192U;
constexpr std::uint32_t SHADER_IMPORT_POLICY_FIELD_COUNT = 7U;

struct FixtureFile final {
    RuntimeAssetFileDesc desc{};
    std::string bytes{};
};

struct LoadedGraph final {
    std::array<RuntimeAssetLoadedFile, FIXTURE_FILE_COUNT> assets{};
    std::array<RuntimeAssetSceneResourceRef, FIXTURE_FILE_COUNT> scene_resource_refs{};
    std::array<RuntimeAssetSceneCameraRecord, 1U> scene_cameras{};
    std::array<RuntimeAssetSceneEntityRecord, 3U> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, 3U> scene_transforms{};
    RuntimeAssetLoadedFile scene_asset{};
    RuntimeAssetSceneLoaderOutput scene_output{};
    RuntimeAssetGraphLoadResult load_result{};
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

struct BoundedLoadedGraph final {
    std::array<RuntimeAssetLoadedFile, FIXTURE_FILE_COUNT> assets{};
    std::array<RuntimeAssetSceneResourceRef, FIXTURE_FILE_COUNT> scene_resource_refs{};
    std::array<RuntimeAssetSceneCameraRecord, 2U> scene_cameras{};
    std::array<RuntimeAssetSceneEntityRecord, 4U> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, 4U> scene_transforms{};
    RuntimeAssetLoadedFile scene_asset{};
    RuntimeAssetSceneLoaderOutput scene_output{};
    RenderSceneRuntimeFrameResult frame_result{};
    std::size_t render_result_count = 0U;
    std::size_t capture_bytes_written = 0U;
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

    RhiStatus RecordBindConstantBuffer(
        yuengine::rhi::RhiCommandList &command_list,
        const RhiConstantBufferBinding &binding) override {
        return device_.RecordBindConstantBuffer(command_list, binding);
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

struct RuntimeAssetD3D11DeviceContext final {
    ~RuntimeAssetD3D11DeviceContext() {
        if (device != nullptr) {
            RhiDeviceFactory::DestroyDevice(device);
            device = nullptr;
        }

        if (window.IsCreated()) {
            window.Destroy();
        }
    }

    WindowsPlatformWindow window{};
    std::vector<std::byte> storage{};
    IRhiDevice *device = nullptr;
};

RhiNativeSurfaceDesc RhiSurfaceFromPlatformSurface(const PlatformNativeSurface &surface) {
    RhiNativeSurfaceDesc desc{};
    desc.window_value = surface.window_value;
    desc.instance_value = surface.instance_value;
    desc.valid = surface.valid;
    return desc;
}

int CreateD3D11RuntimeAssetDevice(RuntimeAssetD3D11DeviceContext *context) {
    if (context == nullptr) {
        std::fputs("null D3D11 RuntimeAsset device context\n", stderr);
        return 1;
    }

    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine RuntimeAsset D3D11 Visual Proof";
    window_desc.client_width = 2U;
    window_desc.client_height = 2U;
    window_desc.visible = false;
    const PlatformWindowStatus window_status = context->window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        std::fprintf(stderr, "D3D11 RuntimeAsset window unavailable status=%u\n", static_cast<unsigned>(window_status));
        return SKIP_RETURN_CODE;
    }

    const PlatformNativeSurface surface = context->window.GetNativeSurface();
    if (!surface.valid) {
        std::fputs("D3D11 RuntimeAsset native surface unavailable\n", stderr);
        return SKIP_RETURN_CODE;
    }

    const std::size_t storage_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::D3D11);
    if (storage_size == 0U) {
        std::fputs("D3D11 RuntimeAsset backend storage unavailable\n", stderr);
        return SKIP_RETURN_CODE;
    }

    context->storage.assign(storage_size, std::byte{0});
    RhiDeviceDesc desc{};
    desc.backend_kind = RhiBackendKind::D3D11;
    desc.native_surface = RhiSurfaceFromPlatformSurface(surface);
    desc.command_list_capacity = MAX_COMMANDS;
    desc.requires_native_surface = true;
    desc.requires_swapchain = true;
    desc.swapchain.color_format = RhiFormat::Rgba8Unorm;
    desc.swapchain.extent = {2U, 2U};
    desc.swapchain.vsync_enabled = false;

    const RhiDeviceCreateResult create_result = RhiDeviceFactory::CreateDevice(
        desc,
        std::span<std::byte>(context->storage.data(), context->storage.size()));
    if (create_result.status != RhiStatus::Success || create_result.device == nullptr) {
        std::fprintf(
            stderr,
            "D3D11 RuntimeAsset device unavailable status=%u\n",
            static_cast<unsigned>(create_result.status));
        return SKIP_RETURN_CODE;
    }

    context->device = create_result.device;
    return 0;
}

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

std::array<std::uint8_t, RUNTIME_ASSET_PACKED_MATERIAL_CONSTANT_BYTES> ExpectedPackedMaterialConstants() {
    std::array<std::uint8_t, RUNTIME_ASSET_PACKED_MATERIAL_CONSTANT_BYTES> constants{};
    constants[0U] = 0x20U;
    constants[1U] = 0x30U;
    constants[2U] = 0x40U;
    constants[3U] = 0xC0U;
    constants[4U] = static_cast<std::uint8_t>(MATERIAL_EMISSIVE_STRENGTH);
    constants[5U] = static_cast<std::uint8_t>(MATERIAL_METALLIC);
    constants[6U] = static_cast<std::uint8_t>(MATERIAL_ROUGHNESS);
    constants[7U] = static_cast<std::uint8_t>(MATERIAL_OPACITY);
    constants[8U] = 0x02U;
    return constants;
}

bool ExpectPackedMaterialConstants(const RuntimeAssetPackedMaterialConstants &constants) {
    const std::array<std::uint8_t, RUNTIME_ASSET_PACKED_MATERIAL_CONSTANT_BYTES> expected =
        ExpectedPackedMaterialConstants();
    if (constants.byte_count != expected.size()) {
        return FailStep("packed material constant byte count mismatch");
    }

    for (std::size_t index = 0U; index < expected.size(); ++index) {
        if (constants.bytes[index] != expected[index]) {
            return FailStep("packed material constant bytes mismatch");
        }
    }

    const std::uint64_t expected_hash = HashRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(constants.bytes.data(), constants.byte_count));
    if (constants.hash == 0U || constants.hash != expected_hash) {
        return FailStep("packed material constant hash mismatch");
    }

    return true;
}

bool ExpectRenderSceneMaterialConstants(
    const RenderSceneRuntimeMaterialRecord &material,
    std::uint64_t expected_hash) {
    const std::array<std::uint8_t, RUNTIME_ASSET_PACKED_MATERIAL_CONSTANT_BYTES> expected =
        ExpectedPackedMaterialConstants();
    if (material.material_constant_byte_count != expected.size()) {
        return FailStep("render scene material constant byte count mismatch");
    }

    for (std::size_t index = 0U; index < expected.size(); ++index) {
        if (material.material_constant_bytes[index] != expected[index]) {
            return FailStep("render scene material constant bytes mismatch");
        }
    }

    const std::uint64_t actual_hash = HashRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(
            material.material_constant_bytes.data(),
            material.material_constant_byte_count));
    if (actual_hash != expected_hash) {
        return FailStep("render scene material constant hash mismatch");
    }

    return true;
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
        case RuntimeAssetDataStatus::FileWriteFailed:
            return "FileWriteFailed";
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
        case RuntimeAssetDataStatus::RhiTextureFailed:
            return "RhiTextureFailed";
        case RuntimeAssetDataStatus::RhiSamplerFailed:
            return "RhiSamplerFailed";
        case RuntimeAssetDataStatus::RenderSceneMaterialFailed:
            return "RenderSceneMaterialFailed";
        case RuntimeAssetDataStatus::RhiCaptureFailed:
            return "RhiCaptureFailed";
        default:
            break;
    }

    return "Unknown";
}

const char *VisualProofLayerName(RuntimeAssetVisualProofMissingLayer layer) {
    switch (layer) {
        case RuntimeAssetVisualProofMissingLayer::None:
            return "None";
        case RuntimeAssetVisualProofMissingLayer::Model:
            return "Model";
        case RuntimeAssetVisualProofMissingLayer::MaterialSlot:
            return "MaterialSlot";
        case RuntimeAssetVisualProofMissingLayer::ShaderPipeline:
            return "ShaderPipeline";
        case RuntimeAssetVisualProofMissingLayer::SceneTransform:
            return "SceneTransform";
        case RuntimeAssetVisualProofMissingLayer::Camera:
            return "Camera";
        case RuntimeAssetVisualProofMissingLayer::RhiCapture:
            return "RhiCapture";
        default:
            break;
    }

    return "Unknown";
}

const char *PackagedRunBlockedLayerName(RuntimeAssetPackagedRunBlockedLayer layer) {
    switch (layer) {
        case RuntimeAssetPackagedRunBlockedLayer::None:
            return "None";
        case RuntimeAssetPackagedRunBlockedLayer::PackageLoadPlan:
            return "PackageLoadPlan";
        case RuntimeAssetPackagedRunBlockedLayer::RuntimeAssetData:
            return "RuntimeAssetData";
        case RuntimeAssetPackagedRunBlockedLayer::ResourceAsset:
            return "ResourceAsset";
        case RuntimeAssetPackagedRunBlockedLayer::ShaderProgram:
            return "ShaderProgram";
        case RuntimeAssetPackagedRunBlockedLayer::RenderSceneRenderCoreRhi:
            return "RenderSceneRenderCoreRhi";
        case RuntimeAssetPackagedRunBlockedLayer::RuntimeAppFrameLoop:
            return "RuntimeAppFrameLoop";
    }

    return "Unknown";
}

std::filesystem::path TestRoot(std::string_view test_name) {
    std::filesystem::path root = std::filesystem::temp_directory_path();
    root /= "YuEngineRuntimeAssetDataTests";
    root /= std::string(test_name);
    return root;
}

constexpr std::uint32_t MESH_VERTEX_STRIDE_BYTES = 16U;
constexpr std::uint32_t MESH_INDEX_STRIDE_BYTES = 2U;
constexpr std::uint32_t MESH_TEXCOORD_OFFSET_BYTES = 8U;

constexpr std::uint32_t MeshVertexPayloadByteCount(std::uint32_t vertex_count) {
    return vertex_count * MESH_VERTEX_STRIDE_BYTES;
}

constexpr std::uint32_t MeshIndexPayloadByteCount(std::uint32_t index_count) {
    return index_count * MESH_INDEX_STRIDE_BYTES;
}

constexpr std::uint32_t MeshPayloadByteCount(
    std::uint32_t vertex_count,
    std::uint32_t index_count) {
    return MeshVertexPayloadByteCount(vertex_count) +
           MeshIndexPayloadByteCount(index_count);
}

std::string MeshPayload(char seed, std::uint32_t byte_count);
std::string SourceMeshText(
    std::string_view header_line,
    std::string_view id,
    std::string_view shape,
    std::uint32_t vertex_count,
    std::uint32_t index_count,
    std::uint32_t vertex_payload_byte_count,
    std::uint32_t index_payload_byte_count,
    std::string_view payload);
std::string SourceMaterialText();
std::string SourceShaderText();

std::array<FixtureFile, FIXTURE_FILE_COUNT> CanonicalFiles() {
    constexpr std::uint32_t cube_vertex_count = 24U;
    constexpr std::uint32_t cube_index_count = 36U;
    constexpr std::uint32_t cylinder_vertex_count = 18U;
    constexpr std::uint32_t cylinder_index_count = 96U;
    constexpr std::uint32_t cone_vertex_count = 10U;
    constexpr std::uint32_t cone_index_count = 48U;
    constexpr std::uint32_t cube_payload_byte_count =
        MeshPayloadByteCount(cube_vertex_count, cube_index_count);
    constexpr std::uint32_t cylinder_payload_byte_count =
        MeshPayloadByteCount(cylinder_vertex_count, cylinder_index_count);
    constexpr std::uint32_t cone_payload_byte_count =
        MeshPayloadByteCount(cone_vertex_count, cone_index_count);
    constexpr std::uint32_t cube_vertex_payload_byte_count =
        MeshVertexPayloadByteCount(cube_vertex_count);
    constexpr std::uint32_t cube_index_payload_byte_count =
        MeshIndexPayloadByteCount(cube_index_count);
    constexpr std::uint32_t cylinder_vertex_payload_byte_count =
        MeshVertexPayloadByteCount(cylinder_vertex_count);
    constexpr std::uint32_t cylinder_index_payload_byte_count =
        MeshIndexPayloadByteCount(cylinder_index_count);
    constexpr std::uint32_t cone_vertex_payload_byte_count =
        MeshVertexPayloadByteCount(cone_vertex_count);
    constexpr std::uint32_t cone_index_payload_byte_count =
        MeshIndexPayloadByteCount(cone_index_count);
    const std::string cube_payload = MeshPayload('A', cube_payload_byte_count);
    const std::string cylinder_payload = MeshPayload('K', cylinder_payload_byte_count);
    const std::string cone_payload = MeshPayload('U', cone_payload_byte_count);
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
                cube_payload_byte_count},
            SourceMeshText(
                "YUASSET MESH 1",
                "cube_mesh",
                "cube",
                cube_vertex_count,
                cube_index_count,
                cube_vertex_payload_byte_count,
                cube_index_payload_byte_count,
                cube_payload)},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Mesh/Cylinder.yumesh",
                RuntimeAssetFileKind::Mesh,
                ResourceTypeId{RESOURCE_TYPE_MESH},
                AssetTypeId{ASSET_TYPE_MESH},
                1002U,
                yuengine::resource::ResourceDecodePlanAssetClass::Mesh,
                yuengine::resource::ResourceDecodeResultClass::Mesh,
                cylinder_payload_byte_count},
            SourceMeshText(
                "YUASSET MESH 1",
                "cylinder_mesh",
                "cylinder",
                cylinder_vertex_count,
                cylinder_index_count,
                cylinder_vertex_payload_byte_count,
                cylinder_index_payload_byte_count,
                cylinder_payload)},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Mesh/Cone.yumesh",
                RuntimeAssetFileKind::Mesh,
                ResourceTypeId{RESOURCE_TYPE_MESH},
                AssetTypeId{ASSET_TYPE_MESH},
                1003U,
                yuengine::resource::ResourceDecodePlanAssetClass::Mesh,
                yuengine::resource::ResourceDecodeResultClass::Mesh,
                cone_payload_byte_count},
            SourceMeshText(
                "YUASSET MESH 1",
                "cone_mesh",
                "cone",
                cone_vertex_count,
                cone_index_count,
                cone_vertex_payload_byte_count,
                cone_index_payload_byte_count,
                cone_payload)},
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
            SourceMaterialText()},
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
            SourceShaderText()},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Animation/Spin.yuanim",
                RuntimeAssetFileKind::Animation,
                ResourceTypeId{RESOURCE_TYPE_ANIMATION},
                AssetTypeId{ASSET_TYPE_ANIMATION},
                5001U},
            "YUASSET ANIMATION 1\nschema=rav0-source\nid=spin\nclip=1\nduration=1\ntarget=scene_entity:101\ntrack=transform:rotation_y\nkey0=0:0\nkey1=1:1\ntracks=1\nsample_rate=30\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Camera/Main.yucamera",
                RuntimeAssetFileKind::Camera,
                ResourceTypeId{RESOURCE_TYPE_CAMERA},
                AssetTypeId{ASSET_TYPE_CAMERA},
                7001U},
            "YUASSET CAMERA 1\nschema=rav0-source\nid=main_camera\nprojection=perspective\nfov_degrees=55\nnear=0.1\nfar=100\nkeyframes=3\nkey0=0:-4,2,-6:0,0,0\nkey1=0.5:0,3,-5:0,0,0\nkey2=1:4,2,-6:0,0,0\n"}};
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
        "cam=Camera/Main.yucamera\n"
        "e0=101:-2,0,0\n"
        "e1=102:0,0,0\n"
        "e2=103:2,0,0\n");
}

std::string BoundedSceneBytes() {
    return std::string(
        "YUASSET SCENE 1\n"
        "schema=rav0-source\n"
        "id=bounded_scene\n"
        "m0=Mesh/C.yumesh\n"
        "m1=Mesh/Y.yumesh\n"
        "m2=Mesh/N.yumesh\n"
        "mat=Material/M.yumat\n"
        "t0=Texture/A.yutex\n"
        "prog=Shader/P.yuprogram\n"
        "anim=Animation/S.yuanim\n"
        "cam=Camera/Main.yucamera\n"
        "entities=4\n"
        "cameras=2\n"
        "camera0=11:inactive\n"
        "camera1=12:active\n"
        "e0=101:-3,0,0|mesh_ref=0|material_ref=0|texture_ref=0|shader_ref=0|camera=1|animation_ref=0|sort=30\n"
        "e1=102:-1,0,0|mesh_ref=1|material_ref=0|texture_ref=1|shader_ref=0|camera=1|animation_ref=0|sort=20\n"
        "e2=103:1,0,0|mesh_ref=2|material_ref=0|texture_ref=2|shader_ref=0|camera=1|animation_ref=0|sort=40\n"
        "e3=104:3,1,0|mesh_ref=0|material_ref=0|texture_ref=0|shader_ref=0|camera=1|animation_ref=0|sort=10\n");
}

std::string BoundedAnimationBytes() {
    return std::string(
        "YUASSET ANIMATION 1\n"
        "schema=rav0-source\n"
        "id=bounded_spin\n"
        "target=scene_entity:101\n"
        "track=transform:rotation_y\n"
        "clips=1\n"
        "tracks=2\n"
        "keyframes=4\n"
        "clip0=id=1|duration=1|first_track_index=0|track_count=2|sample_rate=30\n"
        "track0=id=1|target_ref=scene_entity:101|channel=transform:rotation_y|first_key=0|key_count=2|interp=linear\n"
        "track1=id=2|target_ref=scene_entity:104|channel=transform:translation_y|first_key=2|key_count=2|interp=linear\n"
        "key0=0:0\n"
        "key1=1:1\n"
        "key2=0:1\n"
        "key3=1:3\n");
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
        "cam=Camera/Main.alt\n"
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
    descs[9U].path = "Camera/Main.alt";
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

std::string MeshPayload(char seed, std::uint32_t byte_count) {
    std::string payload{};
    payload.reserve(byte_count);
    for (std::uint32_t index = 0U; index < byte_count; ++index) {
        const int payload_value = static_cast<int>(seed) + static_cast<int>(index % 10U);
        const char value = static_cast<char>(payload_value);
        payload.push_back(value);
    }

    return payload;
}

std::string SourceMeshText(
    std::string_view header_line,
    std::string_view id,
    std::string_view shape,
    std::uint32_t vertex_count,
    std::uint32_t index_count,
    std::uint32_t vertex_payload_byte_count,
    std::uint32_t index_payload_byte_count,
    std::string_view payload) {
    std::string text(header_line);
    text += "\nschema=rav0-source\nid=";
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
    text += "\ninput=layout:position,texcoord\nvertexStrideBytes=";
    text += std::to_string(MESH_VERTEX_STRIDE_BYTES);
    text += "\nindexFormat=uint16\nindexStrideBytes=";
    text += std::to_string(MESH_INDEX_STRIDE_BYTES);
    text += "\ntopology=triangle_list";
    text += "\npayloadBytes=";
    text += std::to_string(payload.size());
    text += "\npayloadAlign=4\npayloadHash=";
    text += std::to_string(HashText(payload));
    text += "\npayload=";
    text += payload;
    text += "\n";
    return text;
}

std::string SourceMeshText(std::string_view header_line) {
    constexpr std::uint32_t vertex_count = 24U;
    constexpr std::uint32_t index_count = 36U;
    constexpr std::uint32_t vertex_payload_byte_count =
        MeshVertexPayloadByteCount(vertex_count);
    constexpr std::uint32_t index_payload_byte_count =
        MeshIndexPayloadByteCount(index_count);
    constexpr std::uint32_t payload_byte_count =
        MeshPayloadByteCount(vertex_count, index_count);
    const std::string payload = MeshPayload('A', payload_byte_count);
    return SourceMeshText(
        header_line,
        "cube_mesh",
        "cube",
        vertex_count,
        index_count,
        vertex_payload_byte_count,
        index_payload_byte_count,
        payload);
}

std::string MaterialParameterFields() {
    return std::string(
        "parameterCount=5\n"
        "baseColorRgba=32,48,64,192\n"
        "emissiveStrength=64\n"
        "metallic=128\n"
        "roughness=96\n"
        "opacity=192\n"
        "alphaMode=blend\n");
}

std::string SourceMaterialText() {
    std::string text(
        "YUASSET MATERIAL 1\n"
        "schema=rav0-source\n"
        "id=shared_material\n"
        "shader=Shader/RuntimeProgram.yuprogram\n"
        "texture0=Texture/Albedo.yutex\n"
        "texture1=Texture/Normal.yutex\n"
        "texture2=Texture/Mask.yutex\n");
    text += MaterialParameterFields();
    return text;
}

std::string ShaderImportPolicyFields() {
    return std::string(
        "importLanguage=hlsl\n"
        "importTarget=d3d11\n"
        "entry_vs=VSMain\n"
        "entry_ps=PSMain\n"
        "profile_vs=vs_5_0\n"
        "profile_ps=ps_5_0\n"
        "compileFlags=deterministic\n");
}

std::string SourceShaderTextWithBody(std::string_view body) {
    std::string text(
        "YUASSET SHADER 1\n"
        "schema=rav0-source\n"
        "id=runtime_program\n");
    text += ShaderImportPolicyFields();
    text += body;
    return text;
}

std::string SourceShaderText() {
    return SourceShaderTextWithBody(
        "stage_vs=bytecode:runtime_program_vs\n"
        "stage_ps=bytecode:runtime_program_ps\n"
        "input=layout:position,color\n"
        "textures=3\n");
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

std::string ReplaceFirst(std::string text, std::string_view from, std::string_view to) {
    const std::size_t found = text.find(from);
    if (found == std::string::npos) {
        return text;
    }

    text.replace(found, from.size(), to);
    return text;
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

bool WriteBoundedFixture(MountTable &table) {
    if (!WriteCanonicalFixture(table)) {
        return false;
    }

    if (!WriteBytes(table, SCENE_PATH, BytesFromString(BoundedSceneBytes()))) {
        return false;
    }

    return WriteBytes(table, "Animation/Spin.yuanim", BytesFromString(BoundedAnimationBytes()));
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

    if (!Contains(scene, "cam=Camera/Main.yucamera")) {
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

bool ReadDecodedTexturePayloadBytes(
    ResourceRegistry &registry,
    const RuntimeAssetLoadedFile &texture_file,
    std::array<std::uint8_t, RUNTIME_TEXTURE_BYTE_COUNT> *out_bytes) {
    if (out_bytes == nullptr) {
        return FailStep("null decoded texture payload output");
    }

    std::uint32_t read_byte_count = 0U;
    const ResourceDecodedPayloadStatus status = registry.ReadDecodedPayload(
        DecodedPayloadRequestFor(texture_file),
        out_bytes->data(),
        static_cast<std::uint32_t>(out_bytes->size()),
        &read_byte_count);
    if (status != ResourceDecodedPayloadStatus::Success || read_byte_count != out_bytes->size()) {
        return FailStep("read decoded texture payload failed");
    }

    return true;
}

bool BuildCookedTexturePayloadDesc(
    ResourceRegistry &registry,
    const RuntimeAssetLoadedFile &texture_file,
    std::uint32_t texture_index,
    RuntimeAssetCookedTexturePayloadDesc *out_desc) {
    if (out_desc == nullptr) {
        return FailStep("null cooked texture payload desc output");
    }

    std::array<std::uint8_t, RUNTIME_TEXTURE_BYTE_COUNT> decoded_bytes{};
    if (!ReadDecodedTexturePayloadBytes(registry, texture_file, &decoded_bytes)) {
        return false;
    }

    RuntimeAssetCookedTexturePayloadDesc desc{};
    desc.loaded_texture = &texture_file;
    desc.texture_desc = RuntimeTextureDesc();
    desc.color_space = RuntimeAssetCookedTextureColorSpace::Linear;
    desc.row_pitch_bytes = RUNTIME_TEXTURE_WIDTH * static_cast<std::uint32_t>(yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    desc.slice_pitch_bytes = RUNTIME_TEXTURE_BYTE_COUNT;
    desc.payload_offset_bytes = 0U;
    desc.payload_byte_count = RUNTIME_TEXTURE_BYTE_COUNT;
    desc.payload_alignment_bytes = static_cast<std::uint32_t>(yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    desc.payload_hash = HashRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(decoded_bytes.data(), decoded_bytes.size()));
    desc.decoded_payload_id = texture_file.decoded_payload_id;
    desc.staging_request_id = texture_file.stable_id + 500000U + texture_index;
    desc.upload_id = texture_file.stable_id + 600000U + texture_index;
    *out_desc = desc;
    return true;
}

bool BuildCookedTexturePayloadDescs(
    ResourceRegistry &registry,
    std::span<const RuntimeAssetLoadedFile> texture_files,
    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> *out_descs) {
    if (out_descs == nullptr) {
        return FailStep("null cooked texture payload descs output");
    }

    if (texture_files.size() != out_descs->size()) {
        return FailStep("unexpected texture payload desc count");
    }

    for (std::size_t index = 0U; index < texture_files.size(); ++index) {
        if (!BuildCookedTexturePayloadDesc(
                registry,
                texture_files[index],
                static_cast<std::uint32_t>(index),
                &(*out_descs)[index])) {
            return false;
        }
    }

    return true;
}

std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> CookedMaterialSlots() {
    std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> slots{};
    for (std::size_t index = 0U; index < slots.size(); ++index) {
        slots[index].material_slot = static_cast<std::uint32_t>(index);
        slots[index].texture_payload_index = static_cast<std::uint32_t>(index);
        slots[index].expected_format = RhiFormat::Rgba8Unorm;
        slots[index].expected_color_space = RuntimeAssetCookedTextureColorSpace::Linear;
        slots[index].texture_binding_slot = static_cast<std::uint32_t>(index);
        slots[index].sampler_binding_slot = static_cast<std::uint32_t>(index);
        slots[index].sampler_desc.linear_filter = false;
        slots[index].sampler_desc.clamp_to_edge = true;
    }

    return slots;
}

RuntimeAssetCookedTextureMaterialBridgeResult InvokeCookedMaterialBridgeWithMaterial(
    IRhiDevice &device,
    ResourceRegistry &registry,
    AssetManager &manager,
    AssetHandle material_asset,
    const RuntimeAssetLoadedFile *loaded_material,
    RhiPipelineHandle pipeline,
    std::span<const RuntimeAssetCookedTexturePayloadDesc> textures,
    std::span<const RuntimeAssetCookedMaterialSlotDesc> slots,
    RenderSceneRuntimeMaterialRecord *out_material) {
    std::array<std::uint8_t, RUNTIME_TEXTURE_BYTE_COUNT> scratch_bytes{};
    RuntimeAssetCookedTextureMaterialBridgeRequest request{};
    request.resource_registry = &registry;
    request.asset_manager = &manager;
    request.rhi_device = &device;
    request.loaded_material = loaded_material;
    request.material_asset = material_asset;
    request.material_id = MATERIAL_ID;
    request.pipeline = pipeline;
    request.textures = textures;
    request.material_slots = slots;
    request.scratch_bytes = std::span<std::uint8_t>(scratch_bytes.data(), scratch_bytes.size());
    request.out_material = out_material;

    RuntimeAssetCookedTextureMaterialBridgeResult result{};
    BuildRuntimeAssetCookedTextureMaterialBridge(request, &result);
    return result;
}

RuntimeAssetCookedTextureMaterialBridgeResult InvokeCookedMaterialBridge(
    IRhiDevice &device,
    ResourceRegistry &registry,
    AssetManager &manager,
    AssetHandle material_asset,
    RhiPipelineHandle pipeline,
    std::span<const RuntimeAssetCookedTexturePayloadDesc> textures,
    std::span<const RuntimeAssetCookedMaterialSlotDesc> slots,
    RenderSceneRuntimeMaterialRecord *out_material) {
    return InvokeCookedMaterialBridgeWithMaterial(
        device,
        registry,
        manager,
        material_asset,
        nullptr,
        pipeline,
        textures,
        slots,
        out_material);
}

RuntimeAssetCookedTextureMaterialBridgeResult BuildCookedMaterial(
    IRhiDevice &device,
    ResourceRegistry &registry,
    AssetManager &manager,
    AssetHandle material_asset,
    std::span<const RuntimeAssetLoadedFile> texture_assets,
    RenderSceneRuntimeMaterialRecord *out_material) {
    RuntimeAssetCookedTextureMaterialBridgeResult result{};
    if (out_material == nullptr) {
        return result;
    }

    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> textures{};
    if (!BuildCookedTexturePayloadDescs(registry, texture_assets, &textures)) {
        return result;
    }

    const std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> slots =
        CookedMaterialSlots();
    RhiPipelineHandle pipeline{};
    if (!CreatePipeline(device, &pipeline)) {
        result.status = RuntimeAssetDataStatus::InvalidArgument;
        return result;
    }

    return InvokeCookedMaterialBridge(
        device,
        registry,
        manager,
        material_asset,
        pipeline,
        std::span<const RuntimeAssetCookedTexturePayloadDesc>(textures.data(), textures.size()),
        std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
        out_material);
}

bool ExpectCookedBridgeFailureWithoutRhiMutation(
    ResourceRegistry &registry,
    AssetManager &manager,
    AssetHandle material_asset,
    std::span<const RuntimeAssetCookedTexturePayloadDesc> textures,
    std::span<const RuntimeAssetCookedMaterialSlotDesc> slots,
    RuntimeAssetDataStatus expected_status) {
    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return FailStep("initialize cooked bridge failure rhi failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreatePipeline(device, &pipeline)) {
        return FailStep("create cooked bridge failure pipeline failed");
    }

    const auto before_snapshot = device.Snapshot();
    RenderSceneRuntimeMaterialRecord material{};
    const RuntimeAssetCookedTextureMaterialBridgeResult result = InvokeCookedMaterialBridge(
        device,
        registry,
        manager,
        material_asset,
        pipeline,
        textures,
        slots,
        &material);
    if (result.status != expected_status) {
        std::fwrite(StatusName(result.status), sizeof(char), std::string_view(StatusName(result.status)).size(), stderr);
        std::fputc('\n', stderr);
        return FailStep("cooked bridge failure returned unexpected status");
    }

    if (result.mutated_state) {
        return FailStep("cooked bridge preflight failure reported mutation");
    }

    if (material.is_resolved || material.texture_slot_count != 0U) {
        return FailStep("cooked bridge failure mutated material output");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.resources.texture_count != before_snapshot.resources.texture_count ||
        after_snapshot.resources.sampler_count != before_snapshot.resources.sampler_count) {
        return FailStep("cooked bridge preflight failure mutated rhi primitives");
    }

    return true;
}

RhiVertexBufferView VertexView(RhiBufferHandle handle, std::size_t vertex_count);
RhiIndexBufferView IndexView(RhiBufferHandle handle, std::size_t index_count);
bool BuildGeometry(
    RenderScenePrimitiveGeometryKind kind,
    AssetHandle asset,
    std::uint32_t draw_id,
    RhiVertexBufferView vertex_view,
    RhiIndexBufferView index_view,
    RenderScenePrimitiveGeometryRecord *out_record);
bool BuildCamera(
    IRhiDevice &device,
    RenderSceneCameraBindingResult *out_camera,
    std::uint32_t camera_id);
bool LoadRuntimeAssetRecords(
    MountTable &table,
    ResourceRegistry &registry,
    AssetManager &manager,
    LoadedGraph *out_graph);

bool BuildPreviewHostSceneInputs(
    IRhiDevice &device,
    ResourceRegistry &registry,
    AssetManager &manager,
    const LoadedGraph &graph,
    std::array<RenderScenePrimitiveGeometryRecord, 3U> *out_geometry,
    RenderSceneRuntimeMaterialRecord *out_material,
    RenderSceneCameraBindingResult *out_camera) {
    if (out_geometry == nullptr || out_material == nullptr || out_camera == nullptr) {
        return FailStep("null preview host scene input output");
    }

    RhiBufferHandle buffer_slot_guard{};
    if (!CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U, &buffer_slot_guard)) {
        return FailStep("create preview buffer slot guard failed");
    }

    RhiBufferHandle cube_vertex{};
    RhiBufferHandle cube_index{};
    RhiBufferHandle cylinder_vertex{};
    RhiBufferHandle cylinder_index{};
    RhiBufferHandle cone_vertex{};
    RhiBufferHandle cone_index{};
    if (!CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U * 24U, &cube_vertex) ||
        !CreateBuffer(device, RhiBufferUsage::Index, sizeof(std::uint16_t) * 36U, &cube_index) ||
        !CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U * 18U, &cylinder_vertex) ||
        !CreateBuffer(device, RhiBufferUsage::Index, sizeof(std::uint16_t) * 96U, &cylinder_index) ||
        !CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U * 10U, &cone_vertex) ||
        !CreateBuffer(device, RhiBufferUsage::Index, sizeof(std::uint16_t) * 48U, &cone_index)) {
        return FailStep("create preview geometry buffers failed");
    }

    if (!BuildGeometry(
            RenderScenePrimitiveGeometryKind::Cube,
            graph.assets[0U].asset,
            21U,
            VertexView(cube_vertex, 24U),
            IndexView(cube_index, 36U),
            &(*out_geometry)[0U]) ||
        !BuildGeometry(
            RenderScenePrimitiveGeometryKind::Cylinder,
            graph.assets[1U].asset,
            22U,
            VertexView(cylinder_vertex, 18U),
            IndexView(cylinder_index, 96U),
            &(*out_geometry)[1U]) ||
        !BuildGeometry(
            RenderScenePrimitiveGeometryKind::Cone,
            graph.assets[2U].asset,
            23U,
            VertexView(cone_vertex, 10U),
            IndexView(cone_index, 48U),
            &(*out_geometry)[2U])) {
        return FailStep("build preview geometry failed");
    }

    const std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{
        graph.assets[4U],
        graph.assets[5U],
        graph.assets[6U]};
    const RuntimeAssetCookedTextureMaterialBridgeResult material_result = BuildCookedMaterial(
        device,
        registry,
        manager,
        graph.assets[3U].asset,
        std::span<const RuntimeAssetLoadedFile>(texture_assets.data(), texture_assets.size()),
        out_material);
    if (material_result.status != RuntimeAssetDataStatus::Success ||
        material_result.runtime_texture_upload_count != RUNTIME_TEXTURE_SLOT_COUNT ||
        material_result.material_texture_slot_count != RUNTIME_TEXTURE_SLOT_COUNT) {
        return FailStep("build preview material failed");
    }

    if (!BuildCamera(device, out_camera, 1U)) {
        return FailStep("build preview camera failed");
    }

    return true;
}

bool LoadGenericRenderSceneSubmissionInputs(
    std::string_view root_name,
    MountTable *table,
    ResourceRegistry *registry,
    AssetManager *manager,
    LoadedGraph *graph,
    RuntimeAssetRhiDevice *device,
    std::array<RenderScenePrimitiveGeometryRecord, 3U> *geometry,
    RenderSceneRuntimeMaterialRecord *material,
    RenderSceneCameraBindingResult *camera,
    std::array<RuntimeAssetRenderSceneGeometryBinding, 3U> *geometry_bindings,
    RuntimeAssetRenderSceneMaterialBinding *material_binding) {
    if (table == nullptr ||
        registry == nullptr ||
        manager == nullptr ||
        graph == nullptr ||
        device == nullptr ||
        geometry == nullptr ||
        material == nullptr ||
        camera == nullptr ||
        geometry_bindings == nullptr ||
        material_binding == nullptr) {
        return FailStep("null generic render scene submission input output");
    }

    if (!CreateMountedTable(TestRoot(root_name), table)) {
        return FailStep("generic render scene submission mount setup failed");
    }

    if (!WriteCanonicalFixture(*table)) {
        return FailStep("generic render scene submission fixture write failed");
    }

    if (!LoadRuntimeAssetRecords(*table, *registry, *manager, graph)) {
        return FailStep("generic render scene submission graph load failed");
    }

    if (device->Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return FailStep("generic render scene submission rhi init failed");
    }

    if (!BuildPreviewHostSceneInputs(*device, *registry, *manager, *graph, geometry, material, camera)) {
        return FailStep("generic render scene submission scene inputs failed");
    }

    for (std::size_t index = 0U; index < geometry_bindings->size(); ++index) {
        (*geometry_bindings)[index].resource_ref_index = graph->scene_entities[index].mesh_ref_index;
        (*geometry_bindings)[index].geometry = (*geometry)[index];
    }

    material_binding->resource_ref_index = graph->scene_entities[0U].material_ref_index;
    material_binding->material = *material;
    return true;
}

RuntimeAssetRenderSceneSubmissionRequest BuildGenericRenderSceneSubmissionRequest(
    const LoadedGraph &graph,
    std::span<const RuntimeAssetSceneEntityRecord> scene_entities,
    std::span<const RuntimeAssetSceneTransformOutputRecord> scene_transforms,
    std::span<const RuntimeAssetRenderSceneGeometryBinding> geometry_bindings,
    std::span<const RuntimeAssetRenderSceneMaterialBinding> material_bindings,
    const RenderSceneCameraBindingResult &camera,
    std::span<RenderSceneRuntimeFrameEntityRequest> frame_entities,
    std::span<RenderSceneRuntimeFrameDrawRecord> draws) {
    RuntimeAssetRenderSceneSubmissionRequest request{};
    request.scene_output = &graph.scene_output;
    request.scene_entities = scene_entities;
    request.scene_transforms = scene_transforms;
    request.geometry_bindings = geometry_bindings;
    request.material_bindings = material_bindings;
    request.camera = camera;
    request.out_frame_entities = frame_entities;
    request.out_draws = draws;
    request.frame_id = FRAME_ID + 40U;
    request.require_shared_material = true;
    return request;
}

void SeedGenericRenderSceneSubmissionSentinels(
    std::span<RenderSceneRuntimeFrameEntityRequest> frame_entities,
    std::span<RenderSceneRuntimeFrameDrawRecord> draws) {
    for (std::size_t index = 0U; index < frame_entities.size(); ++index) {
        RenderSceneRuntimeFrameEntityRequest &entity = frame_entities[index];
        entity.world_object_id = WorldObjectId{9000U + static_cast<std::uint32_t>(index)};
        entity.transform.translation_x = 9100.0F + static_cast<float>(index);
        entity.geometry.kind = RenderScenePrimitiveGeometryKind::Cone;
        entity.is_visible = false;
        entity.is_active = false;
    }

    for (std::size_t index = 0U; index < draws.size(); ++index) {
        RenderSceneRuntimeFrameDrawRecord &draw = draws[index];
        draw.world_object_id = WorldObjectId{9200U + static_cast<std::uint32_t>(index)};
        draw.transform.translation_x = 9300.0F + static_cast<float>(index);
        draw.geometry_kind = RenderScenePrimitiveGeometryKind::Cylinder;
        draw.draw.draw_id = 9400U + static_cast<std::uint32_t>(index);
    }
}

bool GenericRenderSceneSubmissionSentinelsUnchanged(
    std::span<const RenderSceneRuntimeFrameEntityRequest> frame_entities,
    std::span<const RenderSceneRuntimeFrameDrawRecord> draws) {
    for (std::size_t index = 0U; index < frame_entities.size(); ++index) {
        const RenderSceneRuntimeFrameEntityRequest &entity = frame_entities[index];
        if (entity.world_object_id.value != 9000U + index ||
            !Approx(entity.transform.translation_x, 9100.0F + static_cast<float>(index)) ||
            entity.geometry.kind != RenderScenePrimitiveGeometryKind::Cone ||
            entity.is_visible ||
            entity.is_active) {
            return FailStep("generic render scene submission failure mutated entity output");
        }
    }

    for (std::size_t index = 0U; index < draws.size(); ++index) {
        const RenderSceneRuntimeFrameDrawRecord &draw = draws[index];
        if (draw.world_object_id.value != 9200U + index ||
            !Approx(draw.transform.translation_x, 9300.0F + static_cast<float>(index)) ||
            draw.geometry_kind != RenderScenePrimitiveGeometryKind::Cylinder ||
            draw.draw.draw_id != 9400U + index) {
            return FailStep("generic render scene submission failure mutated draw output");
        }
    }

    return true;
}

bool BuildPreviewHostResourceBrowserSelection(
    MountTable &table,
    ResourceRegistry &registry,
    AssetManager &manager,
    const LoadedGraph &graph,
    std::uint32_t selected_index,
    std::span<const ResourceBrowserDiagnosticRecord> surface_diagnostics,
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> *out_entries,
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> *out_rows,
    ResourceBrowserSurfaceSelectionResult *out_selection) {
    if (out_entries == nullptr || out_rows == nullptr || out_selection == nullptr) {
        return FailStep("null preview host resource browser selection output");
    }

    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        descs[index] = files[index].desc;
    }

    std::array<ResourceBrowserDiagnosticRecord, 8U> diagnostics{};
    ResourceBrowserDiagnosticsRequest diagnostics_request{};
    diagnostics_request.mount_table = &table;
    diagnostics_request.mount = MountId(MOUNT_ID);
    diagnostics_request.files = descs.data();
    diagnostics_request.file_count = static_cast<std::uint32_t>(descs.size());
    diagnostics_request.loaded_files = graph.assets.data();
    diagnostics_request.loaded_file_count = graph.load_result.loaded_file_count;
    diagnostics_request.resource_registry = &registry;
    diagnostics_request.asset_manager = &manager;
    diagnostics_request.entries = out_entries->data();
    diagnostics_request.entry_capacity = static_cast<std::uint32_t>(out_entries->size());
    diagnostics_request.diagnostics = diagnostics.data();
    diagnostics_request.diagnostic_capacity =
        static_cast<std::uint32_t>(diagnostics.size());

    ResourceBrowserDiagnosticsResult diagnostics_result{};
    if (BuildResourceBrowserRuntimeAssetDiagnostics(diagnostics_request, &diagnostics_result) !=
        ResourceBrowserDiagnosticsStatus::Success) {
        return FailStep("preview host resource browser diagnostics failed");
    }

    ResourceBrowserSurfaceRequest surface_request{};
    surface_request.entries =
        std::span<const ResourceBrowserResourceEntry>(out_entries->data(), out_entries->size());
    surface_request.diagnostics = surface_diagnostics;
    surface_request.rows = std::span<ResourceBrowserSurfaceRow>(out_rows->data(), out_rows->size());

    ResourceBrowserSurfaceResult surface_result{};
    if (BuildResourceBrowserNativeSurface(surface_request, &surface_result) !=
        ResourceBrowserSurfaceStatus::Success) {
        return FailStep("preview host resource browser surface failed");
    }

    if (selected_index >= surface_result.row_count) {
        return FailStep("preview host resource browser selection index invalid");
    }

    ResourceBrowserSurfaceSelectionRequest selection_request{};
    selection_request.entries =
        std::span<const ResourceBrowserResourceEntry>(out_entries->data(), out_entries->size());
    selection_request.rows =
        std::span<const ResourceBrowserSurfaceRow>(out_rows->data(), out_rows->size());
    selection_request.diagnostics = surface_diagnostics;
    selection_request.import_settings = (*out_entries)[selected_index].import_settings;
    selection_request.selected_index = selected_index;
    selection_request.validate_import_settings = true;

    ResolveResourceBrowserSurfaceSelection(selection_request, out_selection);
    return true;
}

yuengine::object::ObjectHandle ObjectHandleForWorldObject(
    WorldObjectId world_object_id) {
    return yuengine::object::ObjectHandle{
        world_object_id.value,
        9U};
}

WorldSceneAuthoringDocument MakePreviewHostSceneDocument(
    const std::array<WorldSceneObjectTransformRestoreIdentityRecord, 3U> &identities,
    const std::array<WorldSceneObjectTransformRestoreTransformRecord, 3U> &transforms,
    const std::array<WorldSceneAuthoringDependencyRecord, 1U> &dependencies) {
    WorldSceneAuthoringDocument document{};
    document.header.scene_document_id = 0x830001U;
    document.header.deterministic_document_hash = 0x8300AAU;
    document.header.identity_record_count =
        static_cast<std::uint32_t>(identities.size());
    document.header.transform_record_count =
        static_cast<std::uint32_t>(transforms.size());
    document.header.dependency_record_count =
        static_cast<std::uint32_t>(dependencies.size());
    document.identity_records = identities.data();
    document.transform_records = transforms.data();
    document.dependency_records = dependencies.data();
    return document;
}

WorldSceneAuthoringRuntimeExport MakePreviewHostSceneRuntimeExport(
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 3U> &identities,
    std::uint32_t *identity_count,
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 3U> &transforms,
    std::uint32_t *transform_count,
    std::uint32_t *attachment_count,
    std::uint32_t *binding_count,
    std::array<WorldSceneAuthoringDependencyRecord, 1U> &dependencies,
    std::uint32_t *dependency_count) {
    WorldSceneAuthoringRuntimeExport runtime_export{};
    runtime_export.identity_records = identities.data();
    runtime_export.identity_capacity =
        static_cast<std::uint32_t>(identities.size());
    runtime_export.identity_count = identity_count;
    runtime_export.transform_records = transforms.data();
    runtime_export.transform_capacity =
        static_cast<std::uint32_t>(transforms.size());
    runtime_export.transform_count = transform_count;
    runtime_export.attachment_capacity = 0U;
    runtime_export.attachment_count = attachment_count;
    runtime_export.binding_capacity = 0U;
    runtime_export.binding_count = binding_count;
    runtime_export.dependency_records = dependencies.data();
    runtime_export.dependency_capacity =
        static_cast<std::uint32_t>(dependencies.size());
    runtime_export.dependency_count = dependency_count;
    return runtime_export;
}

bool LoadRuntimeAssetRecords(
    MountTable &table,
    ResourceRegistry &registry,
    AssetManager &manager,
    LoadedGraph *out_graph);

bool LoadCookedTextureMaterialFixture(
    std::string_view test_root,
    ResourceRegistry &registry,
    AssetManager &manager,
    LoadedGraph *out_graph,
    std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> *out_texture_assets,
    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> *out_textures) {
    if (out_graph == nullptr || out_texture_assets == nullptr || out_textures == nullptr) {
        return FailStep("null cooked texture material fixture output");
    }

    MountTable table;
    if (!CreateMountedTable(TestRoot(test_root), &table)) {
        return FailStep("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return FailStep("generator write failed");
    }

    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return FailStep("runtime asset records failed");
    }

    std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{
        graph.assets[4U],
        graph.assets[5U],
        graph.assets[6U]};
    *out_texture_assets = texture_assets;
    if (!BuildCookedTexturePayloadDescs(
            registry,
            std::span<const RuntimeAssetLoadedFile>(out_texture_assets->data(), out_texture_assets->size()),
            out_textures)) {
        return false;
    }

    *out_graph = graph;
    return true;
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

bool BuildCamera(
    IRhiDevice &device,
    RenderSceneCameraBindingResult *out_camera,
    std::uint32_t camera_id = 1U) {
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
    camera.camera_id = camera_id;
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
    const RuntimeAssetCookedTextureMaterialBridgeResult material_result = BuildCookedMaterial(
        device,
        registry,
        manager,
        graph.assets[3U].asset,
        std::span<const RuntimeAssetLoadedFile>(texture_assets.data(), texture_assets.size()),
        &material);
    if (material_result.status != RuntimeAssetDataStatus::Success) {
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
    graph.load_result = load_result;

    *out_graph = graph;
    return true;
}

bool LoadBoundedRuntimeAssetRecords(
    MountTable &table,
    ResourceRegistry &registry,
    AssetManager &manager,
    BoundedLoadedGraph *out_graph) {
    if (out_graph == nullptr) {
        return FailStep("null bounded graph output");
    }

    BoundedLoadedGraph graph{};
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
        std::fwrite(StatusName(load_status), sizeof(char), std::string_view(StatusName(load_status)).size(), stderr);
        std::fputc('\n', stderr);
        return FailStep("bounded runtime asset graph load failed");
    }

    graph.scene_asset = load_result.scene;
    *out_graph = graph;
    return true;
}

bool ExecuteBoundedRenderPath(
    IRhiDevice &device,
    ResourceRegistry &registry,
    AssetManager &manager,
    BoundedLoadedGraph *graph) {
    if (graph == nullptr) {
        return FailStep("null bounded graph render input");
    }

    RhiBufferHandle buffer_slot_guard{};
    if (!CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U, &buffer_slot_guard)) {
        return FailStep("create bounded buffer slot guard failed");
    }

    RhiBufferHandle cube_vertex{};
    RhiBufferHandle cube_index{};
    RhiBufferHandle cylinder_vertex{};
    RhiBufferHandle cylinder_index{};
    RhiBufferHandle cone_vertex{};
    RhiBufferHandle cone_index{};
    if (!CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U * 24U, &cube_vertex) ||
        !CreateBuffer(device, RhiBufferUsage::Index, sizeof(std::uint16_t) * 36U, &cube_index) ||
        !CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U * 18U, &cylinder_vertex) ||
        !CreateBuffer(device, RhiBufferUsage::Index, sizeof(std::uint16_t) * 96U, &cylinder_index) ||
        !CreateBuffer(device, RhiBufferUsage::Vertex, sizeof(float) * 2U * 10U, &cone_vertex) ||
        !CreateBuffer(device, RhiBufferUsage::Index, sizeof(std::uint16_t) * 48U, &cone_index)) {
        return FailStep("create bounded render buffers failed");
    }

    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    if (!BuildGeometry(RenderScenePrimitiveGeometryKind::Cube, graph->assets[0U].asset, 21U,
            VertexView(cube_vertex, 24U), IndexView(cube_index, 36U), &geometry[0U]) ||
        !BuildGeometry(RenderScenePrimitiveGeometryKind::Cylinder, graph->assets[1U].asset, 22U,
            VertexView(cylinder_vertex, 18U), IndexView(cylinder_index, 96U), &geometry[1U]) ||
        !BuildGeometry(RenderScenePrimitiveGeometryKind::Cone, graph->assets[2U].asset, 23U,
            VertexView(cone_vertex, 10U), IndexView(cone_index, 48U), &geometry[2U])) {
        return FailStep("build bounded geometry failed");
    }

    const std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{
        graph->assets[4U],
        graph->assets[5U],
        graph->assets[6U]};
    RenderSceneRuntimeMaterialRecord material{};
    const RuntimeTextureMaterialSlotBridgeResult material_result = BuildMaterial(
        device,
        registry,
        manager,
        graph->assets[3U].asset,
        std::span<const RuntimeAssetLoadedFile>(texture_assets.data(), texture_assets.size()),
        RuntimeTextureDesc(),
        &material);
    if (material_result.status != RuntimeTextureMaterialSlotBridgeStatus::Success) {
        return FailStep("build bounded material failed");
    }

    std::uint32_t active_camera_id = 0U;
    for (const RuntimeAssetSceneCameraRecord &camera : graph->scene_cameras) {
        if (camera.is_active) {
            active_camera_id = camera.camera_id;
        }
    }

    RenderSceneCameraBindingResult camera{};
    if (!BuildCamera(device, &camera, active_camera_id)) {
        return FailStep("build bounded camera failed");
    }

    std::array<RenderSceneRuntimeFrameEntityRequest, 4U> frame_entities{};
    for (std::size_t index = 0U; index < graph->scene_output.entity_count; ++index) {
        const RuntimeAssetSceneEntityRecord &scene_entity = graph->scene_entities[index];
        if (scene_entity.mesh_ref_index > 2U) {
            return FailStep("bounded scene entity mesh ref escaped test geometry table");
        }

        frame_entities[index].world_object_id = scene_entity.world_object_id;
        frame_entities[index].transform = scene_entity.transform;
        frame_entities[index].geometry = geometry[scene_entity.mesh_ref_index];
        frame_entities[index].is_visible = scene_entity.is_visible;
        frame_entities[index].is_active = scene_entity.is_active;
    }

    std::array<RenderSceneRuntimeFrameDrawRecord, 4U> draws{};
    RenderSceneRuntimeFrameRequest frame_request{};
    frame_request.frame_id = FRAME_ID + 10U;
    frame_request.camera = camera;
    frame_request.material = material;
    frame_request.entities = std::span<const RenderSceneRuntimeFrameEntityRequest>(
        frame_entities.data(),
        graph->scene_output.entity_count);
    RenderSceneRuntimeFrameBuilder frame_builder;
    const RenderSceneRuntimeFrameStatus frame_status = frame_builder.Build(
        frame_request,
        std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()),
        &graph->frame_result);
    if (frame_status != RenderSceneRuntimeFrameStatus::Success) {
        return FailStep("build bounded frame failed");
    }

    std::array<
        RhiSampledTextureBinding,
        yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS> sampled_textures{};
    std::array<
        RhiSamplerBinding,
        yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS> samplers{};
    for (std::size_t index = 0U; index < material.texture_slot_count; ++index) {
        sampled_textures[index] = material.texture_slots[index].sampled_texture;
        samplers[index] = material.texture_slots[index].sampler;
    }

    std::array<std::uint8_t, CAPTURE_BYTES_PER_ENTITY * 4U> capture_bytes{};
    for (std::size_t index = 0U; index < graph->frame_result.output_draw_count; ++index) {
        const std::size_t capture_offset = index * CAPTURE_BYTES_PER_ENTITY;
        RenderDrawableFramePipelineRequest render_request{};
        render_request.rhi_device = &device;
        render_request.pipeline = material.pipeline;
        render_request.vertex_buffer = draws[index].draw.vertex_buffer;
        render_request.index_buffer = draws[index].draw.index_buffer;
        render_request.sampled_texture = sampled_textures[0U];
        render_request.sampler = samplers[0U];
        render_request.sampled_textures = std::span<const RhiSampledTextureBinding>(
            sampled_textures.data(),
            material.texture_slot_count);
        render_request.samplers = std::span<const RhiSamplerBinding>(
            samplers.data(),
            material.texture_slot_count);
        render_request.draw = draws[index].draw.draw;
        render_request.clear_color = camera.camera.clear_color;
        render_request.capture_output =
            std::span<std::uint8_t>(capture_bytes.data() + capture_offset, CAPTURE_BYTES_PER_ENTITY);
        render_request.capture_byte_budget = CAPTURE_BYTES_PER_ENTITY;
        render_request.frame_id = FRAME_ID + 100U + static_cast<std::uint32_t>(index);
        render_request.pass_id = draws[index].draw.pass_id + static_cast<std::uint32_t>(index);
        render_request.material_id = material.material_id;

        RenderDrawableFramePipeline render_pipeline;
        const auto render_result = render_pipeline.Execute(render_request);
        if (render_result.status != RenderDrawableFramePipelineStatus::Success) {
            return FailStep("bounded render pipeline failed");
        }

        ++graph->render_result_count;
        graph->capture_bytes_written += render_result.capture_bytes_written;
    }

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

void SeedBoundedSceneLoaderFailureSentinels(
    std::span<RuntimeAssetSceneResourceRef> refs,
    std::span<RuntimeAssetSceneCameraRecord> cameras,
    std::span<RuntimeAssetSceneEntityRecord> entities,
    std::span<RuntimeAssetSceneTransformOutputRecord> transforms,
    RuntimeAssetSceneLoaderOutput *output) {
    for (std::size_t index = 0U; index < refs.size(); ++index) {
        refs[index].kind = RuntimeAssetFileKind::Shader;
        refs[index].stable_id = 17000U + index;
        refs[index].loaded_file_index = 190U + static_cast<std::uint32_t>(index);
        refs[index].resource = ResourceHandle{110U + static_cast<std::uint32_t>(index), 120U};
        refs[index].asset = AssetHandle{130U + static_cast<std::uint32_t>(index), 140U};
    }

    for (std::size_t index = 0U; index < cameras.size(); ++index) {
        cameras[index].camera_id = 210U + static_cast<std::uint32_t>(index);
        cameras[index].is_active = index == 0U;
    }

    for (std::size_t index = 0U; index < entities.size(); ++index) {
        entities[index].entity_id = 220U + static_cast<std::uint32_t>(index);
        entities[index].world_object_id = WorldObjectId{230U + static_cast<std::uint32_t>(index)};
        entities[index].transform.translation_x = 240.0F + static_cast<float>(index);
        entities[index].transform.rotation_y = 250.0F + static_cast<float>(index);
        entities[index].mesh_ref_index = 260U + static_cast<std::uint32_t>(index);
        entities[index].is_visible = false;
        entities[index].is_active = false;
    }

    for (std::size_t index = 0U; index < transforms.size(); ++index) {
        transforms[index].world_object_id = WorldObjectId{270U + static_cast<std::uint32_t>(index)};
        transforms[index].transform.translation_x = 280.0F + static_cast<float>(index);
        transforms[index].transform.rotation_y = 290.0F + static_cast<float>(index);
    }

    if (output != nullptr) {
        output->status = RuntimeAssetDataStatus::BudgetExceeded;
        output->scene_id = 1777U;
        output->entity_count = 1778U;
        output->transform_count = 1779U;
        output->camera_count = 1780U;
        output->animation_sampled_value_count = 1781U;
    }
}

bool BoundedSceneLoaderFailureSentinelsUnchanged(
    std::span<const RuntimeAssetSceneResourceRef> refs,
    std::span<const RuntimeAssetSceneCameraRecord> cameras,
    std::span<const RuntimeAssetSceneEntityRecord> entities,
    std::span<const RuntimeAssetSceneTransformOutputRecord> transforms,
    const RuntimeAssetSceneLoaderOutput &output) {
    for (std::size_t index = 0U; index < refs.size(); ++index) {
        if (refs[index].kind != RuntimeAssetFileKind::Shader ||
            refs[index].stable_id != 17000U + index ||
            refs[index].loaded_file_index != 190U + index ||
            refs[index].resource.slot != 110U + index ||
            refs[index].resource.generation != 120U ||
            refs[index].asset.slot != 130U + index ||
            refs[index].asset.generation != 140U) {
            return FailStep("bounded failure mutated resource refs");
        }
    }

    for (std::size_t index = 0U; index < cameras.size(); ++index) {
        if (cameras[index].camera_id != 210U + index ||
            cameras[index].is_active != (index == 0U)) {
            return FailStep("bounded failure mutated cameras");
        }
    }

    for (std::size_t index = 0U; index < entities.size(); ++index) {
        if (entities[index].entity_id != 220U + index ||
            entities[index].world_object_id.value != 230U + index ||
            !Approx(entities[index].transform.translation_x, 240.0F + static_cast<float>(index)) ||
            !Approx(entities[index].transform.rotation_y, 250.0F + static_cast<float>(index)) ||
            entities[index].mesh_ref_index != 260U + index ||
            entities[index].is_visible ||
            entities[index].is_active) {
            return FailStep("bounded failure mutated entities");
        }
    }

    for (std::size_t index = 0U; index < transforms.size(); ++index) {
        if (transforms[index].world_object_id.value != 270U + index ||
            !Approx(transforms[index].transform.translation_x, 280.0F + static_cast<float>(index)) ||
            !Approx(transforms[index].transform.rotation_y, 290.0F + static_cast<float>(index))) {
            return FailStep("bounded failure mutated transforms");
        }
    }

    if (output.status != RuntimeAssetDataStatus::BudgetExceeded ||
        output.scene_id != 1777U ||
        output.entity_count != 1778U ||
        output.transform_count != 1779U ||
        output.camera_count != 1780U ||
        output.animation_sampled_value_count != 1781U) {
        return FailStep("bounded failure mutated scene output");
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

bool ProbeBoundedSceneLoaderFailureWithoutOutputMutation(
    MountTable &table,
    RuntimeAssetDataStatus expected_status,
    std::uint32_t entity_capacity = 4U,
    std::uint32_t camera_capacity = 2U,
    std::uint32_t transform_capacity = 4U) {
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> file_descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        file_descs[index] = files[index].desc;
    }

    ResourceRegistry registry;
    AssetManager manager;
    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const AssetSnapshot before_asset_snapshot = manager.Snapshot();
    std::array<RuntimeAssetLoadedFile, FIXTURE_FILE_COUNT> loaded_files{};
    std::array<RuntimeAssetSceneResourceRef, FIXTURE_FILE_COUNT> scene_resource_refs{};
    std::array<RuntimeAssetSceneCameraRecord, 2U> scene_cameras{};
    std::array<RuntimeAssetSceneEntityRecord, 4U> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, 4U> scene_transforms{};
    RuntimeAssetSceneLoaderOutput scene_output{};
    SeedBoundedSceneLoaderFailureSentinels(
        std::span<RuntimeAssetSceneResourceRef>(scene_resource_refs.data(), scene_resource_refs.size()),
        std::span<RuntimeAssetSceneCameraRecord>(scene_cameras.data(), scene_cameras.size()),
        std::span<RuntimeAssetSceneEntityRecord>(scene_entities.data(), scene_entities.size()),
        std::span<RuntimeAssetSceneTransformOutputRecord>(scene_transforms.data(), scene_transforms.size()),
        &scene_output);

    RuntimeAssetGraphLoadRequest load_request{};
    load_request.mount_table = &table;
    load_request.mount = MountId(MOUNT_ID);
    load_request.scene_path = VirtualPath(SCENE_PATH);
    load_request.scene_resource_type = ResourceTypeId{RESOURCE_TYPE_SCENE};
    load_request.scene_asset_type = AssetTypeId{ASSET_TYPE_SCENE};
    load_request.scene_stable_id = 6002U;
    load_request.files = file_descs.data();
    load_request.file_count = static_cast<std::uint32_t>(file_descs.size());
    load_request.resource_registry = &registry;
    load_request.asset_manager = &manager;
    load_request.loaded_files = loaded_files.data();
    load_request.loaded_file_capacity = static_cast<std::uint32_t>(loaded_files.size());
    load_request.scene_resource_refs = scene_resource_refs.data();
    load_request.scene_resource_ref_capacity = static_cast<std::uint32_t>(scene_resource_refs.size());
    load_request.scene_cameras = scene_cameras.data();
    load_request.scene_camera_capacity = camera_capacity;
    load_request.scene_entities = scene_entities.data();
    load_request.scene_entity_capacity = entity_capacity;
    load_request.scene_transforms = scene_transforms.data();
    load_request.scene_transform_capacity = transform_capacity;
    load_request.scene_output = &scene_output;
    load_request.animation_frame_context.frame_index = 1U;
    load_request.animation_frame_context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    load_request.animation_frame_context.fixed_time_nanoseconds = HALF_SECOND_NANOSECONDS;

    RuntimeAssetGraphLoadResult load_result{};
    const RuntimeAssetDataStatus load_status = LoadRuntimeAssetDataGraph(load_request, &load_result);
    if (load_status != expected_status || load_result.status != expected_status) {
        return FailStep("bounded scene loader failure did not return expected status");
    }

    if (!BoundedSceneLoaderFailureSentinelsUnchanged(
            std::span<const RuntimeAssetSceneResourceRef>(scene_resource_refs.data(), scene_resource_refs.size()),
            std::span<const RuntimeAssetSceneCameraRecord>(scene_cameras.data(), scene_cameras.size()),
            std::span<const RuntimeAssetSceneEntityRecord>(scene_entities.data(), scene_entities.size()),
            std::span<const RuntimeAssetSceneTransformOutputRecord>(scene_transforms.data(), scene_transforms.size()),
            scene_output)) {
        return false;
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count ||
        after_resource_snapshot.dependency_edge_count != before_resource_snapshot.dependency_edge_count ||
        after_resource_snapshot.load_commit_record_count != before_resource_snapshot.load_commit_record_count ||
        after_resource_snapshot.loaded_resource_count != before_resource_snapshot.loaded_resource_count) {
        return FailStep("bounded scene loader failure mutated Resource registry state");
    }

    const AssetSnapshot after_asset_snapshot = manager.Snapshot();
    if (after_asset_snapshot.active_asset_count != before_asset_snapshot.active_asset_count ||
        after_asset_snapshot.active_dependency_edge_count != before_asset_snapshot.active_dependency_edge_count ||
        after_asset_snapshot.registered_asset_count != before_asset_snapshot.registered_asset_count ||
        after_asset_snapshot.referenced_asset_count != before_asset_snapshot.referenced_asset_count) {
        return FailStep("bounded scene loader failure mutated Asset manager state");
    }

    return true;
}

bool ProbeBoundedFailureCase(
    const char *root_name,
    const std::string &scene_text,
    const std::string &animation_text,
    RuntimeAssetDataStatus expected_status) {
    MountTable table;
    if (!CreateMountedTable(TestRoot(root_name), &table)) {
        return FailStep("bounded failure case mount setup failed");
    }

    if (!WriteBoundedFixture(table)) {
        return FailStep("bounded failure case fixture write failed");
    }

    if (!WriteBytes(table, SCENE_PATH, BytesFromString(scene_text))) {
        return FailStep("bounded failure case scene override failed");
    }

    if (!WriteBytes(table, "Animation/Spin.yuanim", BytesFromString(animation_text))) {
        return FailStep("bounded failure case animation override failed");
    }

    return ProbeBoundedSceneLoaderFailureWithoutOutputMutation(table, expected_status);
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

struct GeneratedFixtureCommandContext final {
    MountTable table;
    std::array<RuntimeAssetFileDesc, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> source_files{};
    std::array<RuntimeAssetFileDesc, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> cooked_files{};
    RuntimeAssetImportCookCommandResult command{};
};

bool ExecuteGeneratedFixtureCommand(std::string_view root_name, GeneratedFixtureCommandContext *out_context) {
    if (out_context == nullptr) {
        return FailStep("null generated fixture context");
    }

    GeneratedFixtureCommandContext context{};
    if (!CreateMountedTable(TestRoot(root_name), &context.table)) {
        return FailStep("generated fixture mount setup failed");
    }

    RuntimeAssetImportCookCommandRequest request{};
    request.command = RuntimeAssetImportCookCommandKind::GenerateDeterministicDiskFixture;
    request.fixture.mount_table = &context.table;
    request.fixture.mount = MountId(MOUNT_ID);
    request.fixture.source_files = context.source_files.data();
    request.fixture.source_file_capacity = static_cast<std::uint32_t>(context.source_files.size());
    request.fixture.cooked_files = context.cooked_files.data();
    request.fixture.cooked_file_capacity = static_cast<std::uint32_t>(context.cooked_files.size());

    const RuntimeAssetDataStatus status = ExecuteRuntimeAssetImportCookCommand(request, &context.command);
    if (status != RuntimeAssetDataStatus::Success || context.command.status != RuntimeAssetDataStatus::Success) {
        std::fprintf(
            stderr,
            "generated fixture command failed status=%s result=%s validation=%s layer=%u kind=%u index=%u\n",
            StatusName(status),
            StatusName(context.command.status),
            StatusName(context.command.fixture.validation_status),
            static_cast<unsigned>(context.command.fixture.missing_layer),
            static_cast<unsigned>(context.command.fixture.first_failed_kind),
            context.command.fixture.first_failed_artifact_index);
        return FailStep("generated fixture command failed");
    }

    *out_context = context;
    return true;
}

struct PreviewHostImporterCommitBuffers final {
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    std::array<RuntimeAssetLoadedFile, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> loaded_files{};
    std::array<RuntimeAssetSceneResourceRef, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> scene_refs{};
    std::array<RuntimeAssetSceneCameraRecord, 1U> scene_cameras{};
    std::array<RuntimeAssetSceneEntityRecord, 3U> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, 3U> scene_transforms{};
    RuntimeAssetSceneLoaderOutput scene_output{};
    std::array<ResourceBrowserDepthCatalogRow, FIXTURE_FILE_COUNT> catalog_rows{};
    std::array<ResourceBrowserImporterBoundaryRow, FIXTURE_FILE_COUNT> importer_rows{};
    std::array<ResourceBrowserAssetManagerGapRow, FIXTURE_FILE_COUNT> asset_gap_rows{};
    std::array<ResourceBrowserImporterCommitSelectionLedgerRecord, 1U> ledger{};
};

ResourceBrowserExternalAuthoringSourceRow PreviewHostExternalSourceRowForCookedFile(
    const RuntimeAssetFileDesc &desc,
    std::uint32_t runtime_asset_index) {
    ResourceBrowserExternalAuthoringSourceRow row{};
    row.manifest_path = "External/PreviewHostImport.yuexport";
    row.source_path = "external-import:PreviewHost/RuntimeAssetFixture";
    row.payload_path = desc.path;
    row.target_kind = desc.kind;
    row.stable_id = desc.stable_id;
    row.content_hash = desc.stable_id;
    row.runtime_asset_index = runtime_asset_index;
    row.dependency_count = 1U;
    row.manifest_readable = true;
    row.payload_available = true;
    row.dependencies_valid = true;
    row.runtime_asset_descriptor_ready = true;
    row.manifest_ready = true;
    row.preview_supported = true;
    row.selected = true;
    return row;
}

ResourceBrowserImportSettings PreviewHostImportSettingsForExternalRow(
    const RuntimeAssetFileDesc &desc,
    const ResourceBrowserExternalAuthoringSourceRow &row) {
    ResourceBrowserImportSettings settings{};
    settings.source_path = row.source_path;
    settings.target_kind = row.target_kind;
    settings.resource_type = desc.resource_type;
    settings.asset_type = desc.asset_type;
    settings.stable_id = row.stable_id;
    settings.importer_version = 1U;
    settings.expected_schema_version = 1U;
    return settings;
}

ResourceBrowserImporterCommitWorkflowResult BuildPreviewHostImporterCommitWorkflow(
    GeneratedFixtureCommandContext &fixture,
    ResourceRegistry &registry,
    AssetManager &manager,
    std::uint32_t selected_index,
    const ResourceBrowserImportSettings &import_settings,
    std::span<const ResourceBrowserExternalAuthoringSourceRow> external_rows,
    PreviewHostImporterCommitBuffers *buffers) {
    ResourceBrowserImporterCommitWorkflowResult result{};
    if (buffers == nullptr) {
        return result;
    }

    ResourceBrowserImporterCommitWorkflowRequest request{};
    request.mount_table = &fixture.table;
    request.mount = MountId(MOUNT_ID);
    request.import_cook_result = &fixture.command;
    request.scene = fixture.command.fixture.cooked_scene;
    request.files = std::span<const RuntimeAssetFileDesc>(
        fixture.cooked_files.data(),
        fixture.command.fixture.cooked_file_count);
    request.external_source_rows = external_rows;
    request.resource_registry = &registry;
    request.asset_manager = &manager;
    request.import_settings = import_settings;
    request.selected_index = selected_index;
    request.validate_import_settings = true;
    request.entries = std::span<ResourceBrowserResourceEntry>(
        buffers->entries.data(),
        buffers->entries.size());
    request.diagnostics = std::span<ResourceBrowserDiagnosticRecord>(
        buffers->diagnostics.data(),
        buffers->diagnostics.size());
    request.loaded_files = std::span<RuntimeAssetLoadedFile>(
        buffers->loaded_files.data(),
        buffers->loaded_files.size());
    request.scene_resource_refs = std::span<RuntimeAssetSceneResourceRef>(
        buffers->scene_refs.data(),
        buffers->scene_refs.size());
    request.scene_cameras = std::span<RuntimeAssetSceneCameraRecord>(
        buffers->scene_cameras.data(),
        buffers->scene_cameras.size());
    request.scene_entities = std::span<RuntimeAssetSceneEntityRecord>(
        buffers->scene_entities.data(),
        buffers->scene_entities.size());
    request.scene_transforms = std::span<RuntimeAssetSceneTransformOutputRecord>(
        buffers->scene_transforms.data(),
        buffers->scene_transforms.size());
    request.scene_output = &buffers->scene_output;
    request.catalog_rows = std::span<ResourceBrowserDepthCatalogRow>(
        buffers->catalog_rows.data(),
        buffers->catalog_rows.size());
    request.importer_rows = std::span<ResourceBrowserImporterBoundaryRow>(
        buffers->importer_rows.data(),
        buffers->importer_rows.size());
    request.asset_gap_rows = std::span<ResourceBrowserAssetManagerGapRow>(
        buffers->asset_gap_rows.data(),
        buffers->asset_gap_rows.size());
    request.selection_ledger = std::span<ResourceBrowserImporterCommitSelectionLedgerRecord>(
        buffers->ledger.data(),
        buffers->ledger.size());

    BuildResourceBrowserImporterCommitWorkflow(request, &result);
    return result;
}

bool BuildPreviewHostSelectionFromImporterCommit(
    const PreviewHostImporterCommitBuffers &buffers,
    const ResourceBrowserImporterCommitWorkflowResult &commit_result,
    const ResourceBrowserImportSettings &import_settings,
    std::uint32_t selected_index,
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> *out_rows,
    ResourceBrowserSurfaceSelectionResult *out_selection) {
    if (out_rows == nullptr || out_selection == nullptr) {
        return FailStep("null importer commit selection output");
    }

    ResourceBrowserSurfaceRequest surface_request{};
    surface_request.entries = std::span<const ResourceBrowserResourceEntry>(
        buffers.entries.data(),
        commit_result.entry_count);
    surface_request.diagnostics = std::span<const ResourceBrowserDiagnosticRecord>(
        buffers.diagnostics.data(),
        commit_result.diagnostic_count);
    surface_request.rows = std::span<ResourceBrowserSurfaceRow>(
        out_rows->data(),
        out_rows->size());

    ResourceBrowserSurfaceResult surface_result{};
    if (BuildResourceBrowserNativeSurface(surface_request, &surface_result) !=
        ResourceBrowserSurfaceStatus::Success) {
        return FailStep("importer commit surface build failed");
    }

    if (surface_result.row_count != commit_result.entry_count) {
        return FailStep("importer commit surface row count mismatch");
    }

    ResourceBrowserSurfaceSelectionRequest selection_request{};
    selection_request.entries = std::span<const ResourceBrowserResourceEntry>(
        buffers.entries.data(),
        commit_result.entry_count);
    selection_request.rows = std::span<const ResourceBrowserSurfaceRow>(
        out_rows->data(),
        surface_result.row_count);
    selection_request.diagnostics = std::span<const ResourceBrowserDiagnosticRecord>(
        buffers.diagnostics.data(),
        commit_result.diagnostic_count);
    selection_request.import_settings = import_settings;
    selection_request.selected_index = selected_index;
    selection_request.validate_import_settings = true;

    ResolveResourceBrowserSurfaceSelection(selection_request, out_selection);
    return true;
}

bool ReadAndValidateGeneratedArtifact(
    MountTable &table,
    const RuntimeAssetFileDesc &desc,
    RuntimeAssetArtifactClass expected_class,
    RuntimeAssetValidationResult *out_validation) {
    std::vector<std::uint8_t> bytes{};
    if (!ReadFile(table, desc.path, &bytes)) {
        return FailStep("generated artifact was not written to disk");
    }

    RuntimeAssetValidationResult validation{};
    const RuntimeAssetDataStatus status = ValidateRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(bytes.data(), bytes.size()),
        desc.kind,
        &validation);
    if (status != RuntimeAssetDataStatus::Success || validation.artifact_class != expected_class) {
        return FailStep("generated artifact validation failed");
    }

    if (out_validation != nullptr) {
        *out_validation = validation;
    }

    return true;
}

bool LoadGeneratedRuntimeAssetGraph(
    MountTable &table,
    const RuntimeAssetFileDesc &scene_desc,
    std::span<const RuntimeAssetFileDesc> files,
    RuntimeAssetGraphLoadResult *out_result) {
    if (out_result == nullptr) {
        return FailStep("null generated load result");
    }

    ResourceRegistry registry;
    AssetManager manager;
    std::array<RuntimeAssetLoadedFile, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> loaded_files{};
    std::array<RuntimeAssetSceneResourceRef, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> scene_refs{};
    std::array<RuntimeAssetSceneCameraRecord, 1U> scene_cameras{};
    std::array<RuntimeAssetSceneEntityRecord, 3U> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, 3U> scene_transforms{};
    RuntimeAssetSceneLoaderOutput scene_output{};

    RuntimeAssetGraphLoadRequest load_request{};
    load_request.mount_table = &table;
    load_request.mount = MountId(MOUNT_ID);
    load_request.scene_path = VirtualPath(scene_desc.path);
    load_request.scene_resource_type = scene_desc.resource_type;
    load_request.scene_asset_type = scene_desc.asset_type;
    load_request.scene_stable_id = scene_desc.stable_id;
    load_request.files = files.data();
    load_request.file_count = static_cast<std::uint32_t>(files.size());
    load_request.resource_registry = &registry;
    load_request.asset_manager = &manager;
    load_request.loaded_files = loaded_files.data();
    load_request.loaded_file_capacity = static_cast<std::uint32_t>(loaded_files.size());
    load_request.scene_resource_refs = scene_refs.data();
    load_request.scene_resource_ref_capacity = static_cast<std::uint32_t>(scene_refs.size());
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
    const RuntimeAssetDataStatus status = LoadRuntimeAssetDataGraph(load_request, &load_result);
    if (status != RuntimeAssetDataStatus::Success) {
        std::fwrite(StatusName(status), sizeof(char), std::string_view(StatusName(status)).size(), stderr);
        std::fputc('\n', stderr);
        return FailStep("generated runtime asset graph load failed");
    }

    if (load_result.file_read_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U ||
        load_result.loaded_file_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        !load_result.scene_registered ||
        !load_result.scene_references_runtime_asset_families ||
        load_result.transaction_result.status != RuntimeAssetDataStatus::Success ||
        !load_result.transaction_result.mutated_state) {
        return FailStep("generated runtime asset graph diagnostics changed");
    }

    *out_result = load_result;
    return true;
}

int RuntimeAssetDataImportCookCommandWritesSourceAndCookedDiskFixtures() {
    GeneratedFixtureCommandContext first{};
    if (!ExecuteGeneratedFixtureCommand("ImportCookCommandA", &first)) {
        return Fail("first import/cook command failed");
    }

    GeneratedFixtureCommandContext second{};
    if (!ExecuteGeneratedFixtureCommand("ImportCookCommandB", &second)) {
        return Fail("second import/cook command failed");
    }

    const RuntimeAssetDeterministicDiskFixtureResult &first_result = first.command.fixture;
    const RuntimeAssetDeterministicDiskFixtureResult &second_result = second.command.fixture;
    if (!first_result.wrote_to_disk ||
        !first_result.validated_source_files ||
        !first_result.validated_cooked_files ||
        first_result.source_file_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        first_result.cooked_file_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        first_result.source_artifact_write_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U ||
        first_result.cooked_artifact_write_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U ||
        first_result.validation_count != (RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U) * 2U) {
        return Fail("import/cook command did not report source+cooked disk generation");
    }

    if (first_result.source_graph_hash == 0U ||
        first_result.cooked_graph_hash == 0U ||
        first_result.source_graph_hash != second_result.source_graph_hash ||
        first_result.cooked_graph_hash != second_result.cooked_graph_hash ||
        first_result.source_scene_hash != second_result.source_scene_hash ||
        first_result.cooked_scene_hash != second_result.cooked_scene_hash) {
        return Fail("generated fixture hashes were not deterministic");
    }

    for (const RuntimeAssetFileDesc &desc : first.source_files) {
        if (!ReadAndValidateGeneratedArtifact(first.table, desc, RuntimeAssetArtifactClass::Source, nullptr)) {
            return Fail("generated source artifact did not validate");
        }
    }

    for (const RuntimeAssetFileDesc &desc : first.cooked_files) {
        if (!ReadAndValidateGeneratedArtifact(first.table, desc, RuntimeAssetArtifactClass::Cooked, nullptr)) {
            return Fail("generated cooked artifact did not validate");
        }
    }

    RuntimeAssetValidationResult source_scene_validation{};
    RuntimeAssetValidationResult cooked_scene_validation{};
    if (!ReadAndValidateGeneratedArtifact(
            first.table,
            first_result.source_scene,
            RuntimeAssetArtifactClass::Source,
            &source_scene_validation) ||
        !ReadAndValidateGeneratedArtifact(
            first.table,
            first_result.cooked_scene,
            RuntimeAssetArtifactClass::Cooked,
            &cooked_scene_validation)) {
        return Fail("generated scenes did not validate");
    }

    if (source_scene_validation.dependency_count == 0U ||
        cooked_scene_validation.dependency_table_count == 0U ||
        cooked_scene_validation.payload_hash == 0U) {
        return Fail("generated scenes did not carry dependency/table/hash metadata");
    }

    return 0;
}

int RuntimeAssetDataImportCookCommandLoadsGeneratedSourceAndCookedViaFileResourceRoute() {
    GeneratedFixtureCommandContext context{};
    if (!ExecuteGeneratedFixtureCommand("ImportCookCommandLoad", &context)) {
        return Fail("import/cook command fixture setup failed");
    }

    RuntimeAssetGraphLoadResult source_load{};
    if (!LoadGeneratedRuntimeAssetGraph(
            context.table,
            context.command.fixture.source_scene,
            std::span<const RuntimeAssetFileDesc>(context.source_files.data(), context.source_files.size()),
            &source_load)) {
        return Fail("generated source graph did not load");
    }

    RuntimeAssetGraphLoadResult cooked_load{};
    if (!LoadGeneratedRuntimeAssetGraph(
            context.table,
            context.command.fixture.cooked_scene,
            std::span<const RuntimeAssetFileDesc>(context.cooked_files.data(), context.cooked_files.size()),
            &cooked_load)) {
        return Fail("generated cooked graph did not load");
    }

    if (source_load.resource_dependency_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        source_load.asset_dependency_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        cooked_load.resource_dependency_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        cooked_load.asset_dependency_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT) {
        return Fail("generated graph did not commit Resource/Asset dependency edges");
    }

    if (source_load.transaction_plan.resource_commit_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U ||
        cooked_load.transaction_plan.resource_commit_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U) {
        return Fail("generated graph did not route through Resource commit plan");
    }

    return 0;
}

int RuntimeAssetDataImportCookCommandReportsMissingLayerStatus() {
    std::array<RuntimeAssetFileDesc, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> source_files{};
    std::array<RuntimeAssetFileDesc, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> cooked_files{};

    RuntimeAssetImportCookCommandRequest missing_file_vfs_request{};
    missing_file_vfs_request.command = RuntimeAssetImportCookCommandKind::GenerateDeterministicDiskFixture;
    missing_file_vfs_request.fixture.mount = MountId(MOUNT_ID);
    missing_file_vfs_request.fixture.source_files = source_files.data();
    missing_file_vfs_request.fixture.source_file_capacity = static_cast<std::uint32_t>(source_files.size());
    missing_file_vfs_request.fixture.cooked_files = cooked_files.data();
    missing_file_vfs_request.fixture.cooked_file_capacity = static_cast<std::uint32_t>(cooked_files.size());

    RuntimeAssetImportCookCommandResult missing_file_vfs{};
    const RuntimeAssetDataStatus missing_file_status =
        ExecuteRuntimeAssetImportCookCommand(missing_file_vfs_request, &missing_file_vfs);
    if (missing_file_status != RuntimeAssetDataStatus::InvalidArgument ||
        missing_file_vfs.missing_layer != RuntimeAssetImportCookMissingLayer::FileVfs ||
        missing_file_vfs.fixture.status != RuntimeAssetDataStatus::InvalidArgument) {
        return Fail("missing File/VFS layer did not report explicit status");
    }

    RuntimeAssetImportCookCommandRequest unknown_request{};
    RuntimeAssetImportCookCommandResult unknown_result{};
    const RuntimeAssetDataStatus unknown_status =
        ExecuteRuntimeAssetImportCookCommand(unknown_request, &unknown_result);
    if (unknown_status != RuntimeAssetDataStatus::InvalidArgument ||
        unknown_result.missing_layer != RuntimeAssetImportCookMissingLayer::Command) {
        return Fail("unknown command did not report command layer");
    }

    RuntimeAssetImportCookCommandRequest small_capacity_request = missing_file_vfs_request;
    MountTable table;
    if (!CreateMountedTable(TestRoot("ImportCookCommandCapacity"), &table)) {
        return Fail("capacity mount setup failed");
    }

    small_capacity_request.fixture.mount_table = &table;
    small_capacity_request.fixture.source_file_capacity = 1U;
    RuntimeAssetImportCookCommandResult small_capacity{};
    const RuntimeAssetDataStatus capacity_status =
        ExecuteRuntimeAssetImportCookCommand(small_capacity_request, &small_capacity);
    if (capacity_status != RuntimeAssetDataStatus::CapacityExceeded ||
        small_capacity.missing_layer != RuntimeAssetImportCookMissingLayer::RuntimeAssetData) {
        return Fail("descriptor capacity failure did not report RuntimeAssetData layer");
    }

    return 0;
}

struct CookedVisualProofContext final {
    GeneratedFixtureCommandContext fixture;
    ResourceRegistry registry;
    AssetManager manager;
    std::array<RuntimeAssetLoadedFile, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> loaded_files{};
    std::array<RuntimeAssetSceneResourceRef, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> scene_refs{};
    std::array<RuntimeAssetSceneCameraRecord, 1U> scene_cameras{};
    std::array<RuntimeAssetSceneEntityRecord, 3U> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, 3U> scene_transforms{};
    RuntimeAssetSceneLoaderOutput scene_output{};
    RuntimeAssetGraphLoadResult load_result{};
    RuntimeAssetLoadedShaderProgramData shader_program{};
    RuntimeAssetRhiDevice device;
    std::array<std::uint8_t, RUNTIME_TEXTURE_BYTE_COUNT> scratch_bytes{};
    std::array<std::uint8_t, TOTAL_CAPTURE_BYTES * 2U> capture_bytes{};
};

bool DecodeCookedVisualProofShader(CookedVisualProofContext *context) {
    if (context == nullptr) {
        return FailStep("null cooked visual proof shader context");
    }

    const RuntimeAssetFileDesc *shader_desc = nullptr;
    for (const RuntimeAssetFileDesc &desc : context->fixture.cooked_files) {
        if (desc.kind == RuntimeAssetFileKind::Shader) {
            shader_desc = &desc;
            break;
        }
    }

    if (shader_desc == nullptr) {
        return FailStep("generated cooked fixture did not include shader desc");
    }

    std::vector<std::uint8_t> bytes{};
    if (!ReadFile(context->fixture.table, shader_desc->path, &bytes)) {
        return FailStep("generated cooked shader read failed");
    }

    const RuntimeAssetDataStatus status = DecodeRuntimeAssetShaderProgramData(
        std::span<const std::uint8_t>(bytes.data(), bytes.size()),
        static_cast<std::uint32_t>(shader_desc->stable_id),
        &context->shader_program);
    if (status != RuntimeAssetDataStatus::Success ||
        context->shader_program.validation.artifact_class != RuntimeAssetArtifactClass::Cooked) {
        return FailStep("generated cooked shader decode failed");
    }

    return true;
}

bool LoadCookedVisualProofGraph(CookedVisualProofContext *context) {
    if (context == nullptr) {
        return FailStep("null cooked visual proof graph context");
    }

    RuntimeAssetGraphLoadRequest load_request{};
    load_request.mount_table = &context->fixture.table;
    load_request.mount = MountId(MOUNT_ID);
    load_request.scene_path = VirtualPath(context->fixture.command.fixture.cooked_scene.path);
    load_request.scene_resource_type = context->fixture.command.fixture.cooked_scene.resource_type;
    load_request.scene_asset_type = context->fixture.command.fixture.cooked_scene.asset_type;
    load_request.scene_stable_id = context->fixture.command.fixture.cooked_scene.stable_id;
    load_request.files = context->fixture.cooked_files.data();
    load_request.file_count = static_cast<std::uint32_t>(context->fixture.cooked_files.size());
    load_request.resource_registry = &context->registry;
    load_request.asset_manager = &context->manager;
    load_request.loaded_files = context->loaded_files.data();
    load_request.loaded_file_capacity = static_cast<std::uint32_t>(context->loaded_files.size());
    load_request.scene_resource_refs = context->scene_refs.data();
    load_request.scene_resource_ref_capacity = static_cast<std::uint32_t>(context->scene_refs.size());
    load_request.scene_cameras = context->scene_cameras.data();
    load_request.scene_camera_capacity = static_cast<std::uint32_t>(context->scene_cameras.size());
    load_request.scene_entities = context->scene_entities.data();
    load_request.scene_entity_capacity = static_cast<std::uint32_t>(context->scene_entities.size());
    load_request.scene_transforms = context->scene_transforms.data();
    load_request.scene_transform_capacity = static_cast<std::uint32_t>(context->scene_transforms.size());
    load_request.scene_output = &context->scene_output;
    load_request.animation_frame_context.frame_index = 1U;
    load_request.animation_frame_context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    load_request.animation_frame_context.fixed_time_nanoseconds = HALF_SECOND_NANOSECONDS;

    const RuntimeAssetDataStatus status = LoadRuntimeAssetDataGraph(load_request, &context->load_result);
    if (status != RuntimeAssetDataStatus::Success ||
        context->load_result.status != RuntimeAssetDataStatus::Success) {
        std::fprintf(
            stderr,
            "generated cooked graph load failed status=%s result=%s phase=%u record=%u dependency=%u loaded=%u\n",
            StatusName(status),
            StatusName(context->load_result.status),
            static_cast<unsigned>(context->load_result.transaction_result.phase),
            context->load_result.transaction_result.first_failed_record_index,
            context->load_result.transaction_result.first_failed_dependency_index,
            context->load_result.loaded_file_count);
        return FailStep("generated cooked graph load failed");
    }

    if (context->load_result.loaded_file_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        context->load_result.scene.artifact_class != RuntimeAssetArtifactClass::Cooked ||
        context->load_result.scene_registered == false ||
        context->scene_output.status != RuntimeAssetDataStatus::Success) {
        return FailStep("generated cooked graph diagnostics are incomplete");
    }

    for (const RuntimeAssetLoadedFile &file : context->loaded_files) {
        if (file.artifact_class != RuntimeAssetArtifactClass::Cooked ||
            file.schema_version != 1U ||
            file.identity_hash == 0U ||
            file.payload_hash == 0U ||
            file.record_table_count == 0U) {
            return FailStep("loaded cooked record validation metadata was not carried forward");
        }
    }

    return true;
}

bool SetupCookedVisualProofContext(std::string_view root_name, CookedVisualProofContext *context) {
    if (context == nullptr) {
        return FailStep("null cooked visual proof context");
    }

    if (!ExecuteGeneratedFixtureCommand(root_name, &context->fixture)) {
        return FailStep("generated cooked visual proof fixture setup failed");
    }

    if (!LoadCookedVisualProofGraph(context)) {
        return false;
    }

    if (!DecodeCookedVisualProofShader(context)) {
        return false;
    }

    if (context->device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return FailStep("cooked visual proof rhi init failed");
    }

    return true;
}

RuntimeAssetVisualProofRequest CookedVisualProofRequest(
    CookedVisualProofContext &context,
    std::uint32_t frame_count = 2U) {
    RuntimeAssetVisualProofRequest request{};
    request.resource_registry = &context.registry;
    request.asset_manager = &context.manager;
    request.rhi_device = &context.device;
    request.scene = &context.load_result.scene;
    request.loaded_files = std::span<const RuntimeAssetLoadedFile>(
        context.loaded_files.data(),
        context.load_result.loaded_file_count);
    request.scene_cameras = std::span<const RuntimeAssetSceneCameraRecord>(
        context.scene_cameras.data(),
        context.scene_output.camera_count);
    request.scene_entities = std::span<const RuntimeAssetSceneEntityRecord>(
        context.scene_entities.data(),
        context.scene_output.entity_count);
    request.scene_transforms = std::span<const RuntimeAssetSceneTransformOutputRecord>(
        context.scene_transforms.data(),
        context.scene_output.transform_count);
    request.scene_output = &context.scene_output;
    request.shader_program = &context.shader_program;
    request.scratch_bytes = std::span<std::uint8_t>(
        context.scratch_bytes.data(),
        context.scratch_bytes.size());
    request.capture_output = std::span<std::uint8_t>(
        context.capture_bytes.data(),
        context.capture_bytes.size());
    request.capture_byte_budget_per_entity = CAPTURE_BYTES_PER_ENTITY;
    request.first_frame_id = FRAME_ID + 500U;
    request.frame_count = frame_count;
    request.output_path = "Artifacts/RuntimeAssetData/CookedVisualProof.rvf";
    request.output_path_byte_count = 48U;
    request.require_cooked_records = true;
    return request;
}

struct PackageCookRunSmokeLedger final {
    RuntimeAssetDataStatus import_cook_status = RuntimeAssetDataStatus::InvalidArgument;
    PackageStatus package_status = PackageStatus::NotFound;
    PackageArtifactResult package_artifact_write{};
    PackageArtifactResult package_artifact_read{};
    RuntimeAssetPackagedRunResult packaged_run{};
    PackageLoadPlan package_load_plan{};
    std::uint32_t source_file_count = 0U;
    std::uint32_t cooked_file_count = 0U;
    std::uint32_t package_load_plan_record_count = 0U;
    bool import_cook_command_success = false;
    bool package_manifest_success = false;
    bool package_artifact_write_success = false;
    bool package_artifact_read_success = false;
    bool package_load_plan_success = false;
};

bool BuildRuntimeAssetPackageEntryDescriptor(
    MountTable &table,
    const RuntimeAssetFileDesc &desc,
    PackageEntryId entry,
    PackageEntryDescriptor *out_descriptor) {
    if (out_descriptor == nullptr) {
        return FailStep("package smoke entry descriptor output was null");
    }

    std::vector<std::uint8_t> bytes{};
    if (!ReadFile(table, desc.path, &bytes)) {
        return FailStep("package smoke entry source was not readable");
    }

    const std::string logical_key = std::string("runtime_asset_") + std::to_string(desc.stable_id);
    out_descriptor->package = RUNTIME_ASSET_SMOKE_PACKAGE;
    out_descriptor->entry = entry;
    out_descriptor->type = desc.resource_type;
    out_descriptor->logical_key = ResourceLogicalKey(logical_key);
    out_descriptor->source_key = PackageSourceKey(std::string("cooked_record_") + std::to_string(desc.stable_id));
    out_descriptor->byte_offset = 0U;
    out_descriptor->byte_size = static_cast<std::uint32_t>(bytes.size());
    return true;
}

bool BuildRuntimeAssetCookedPackagePlan(
    CookedVisualProofContext &context,
    PackageCookRunSmokeLedger *ledger) {
    if (ledger == nullptr) {
        return FailStep("null package cook run ledger");
    }

    std::array<PackageEntryDescriptor, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U> entries{};
    std::array<PackageArtifactDependency, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> dependencies{};
    const RuntimeAssetFileDesc &scene_desc = context.fixture.command.fixture.cooked_scene;
    if (!BuildRuntimeAssetPackageEntryDescriptor(
            context.fixture.table,
            scene_desc,
            RUNTIME_ASSET_SMOKE_SCENE_ENTRY,
            &entries[0U])) {
        ledger->package_status = PackageStatus::InvalidArtifact;
        return false;
    }

    for (std::uint32_t index = 0U; index < context.fixture.command.fixture.cooked_file_count; ++index) {
        const RuntimeAssetFileDesc &desc = context.fixture.cooked_files[index];
        const PackageEntryId entry{index + 1U};
        if (!BuildRuntimeAssetPackageEntryDescriptor(
                context.fixture.table,
                desc,
                entry,
                &entries[index + 1U])) {
            ledger->package_status = PackageStatus::InvalidArtifact;
            return false;
        }

        dependencies[index] = PackageArtifactDependency{RUNTIME_ASSET_SMOKE_SCENE_ENTRY, entry};
    }

    PackageArtifactWriteRequest write_request{};
    write_request.mount_table = &context.fixture.table;
    write_request.mount = MountId(MOUNT_ID);
    write_request.artifact_path = VirtualPath(RUNTIME_ASSET_SMOKE_PACKAGE_ARTIFACT_PATH);
    write_request.package = RUNTIME_ASSET_SMOKE_PACKAGE;
    write_request.entries = entries.data();
    write_request.entry_count = static_cast<std::uint32_t>(entries.size());
    write_request.dependencies = dependencies.data();
    write_request.dependency_count = context.fixture.command.fixture.cooked_file_count;
    ledger->package_artifact_write = WritePackageArtifact(write_request);
    ledger->package_status = ledger->package_artifact_write.status;
    if (ledger->package_artifact_write.status != PackageStatus::Success ||
        !ledger->package_artifact_write.wrote_artifact) {
        return false;
    }

    ledger->package_artifact_write_success = true;
    PackageRegistry artifact_registry;
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = &context.fixture.table;
    read_request.mount = MountId(MOUNT_ID);
    read_request.artifact_path = VirtualPath(RUNTIME_ASSET_SMOKE_PACKAGE_ARTIFACT_PATH);
    read_request.registry = &artifact_registry;
    ledger->package_artifact_read = ReadPackageArtifact(read_request);
    ledger->package_status = ledger->package_artifact_read.status;
    if (ledger->package_artifact_read.status != PackageStatus::Success ||
        !ledger->package_artifact_read.read_artifact ||
        !ledger->package_artifact_read.rebuilt_registry) {
        return false;
    }

    ledger->package_manifest_success = true;
    ledger->package_artifact_read_success = true;
    const PackageLoadPlanResult plan = artifact_registry.ResolveEntryByResourceKey(
        RUNTIME_ASSET_SMOKE_PACKAGE,
        scene_desc.resource_type,
        ResourceLogicalKey(std::string("runtime_asset_") + std::to_string(scene_desc.stable_id)));
    ledger->package_status = plan.status;
    if (!plan.Succeeded()) {
        return false;
    }

    ledger->package_load_plan_success = true;
    ledger->package_load_plan_record_count = plan.plan.record_count;
    ledger->package_load_plan = plan.plan;
    return plan.plan.record_count == context.fixture.command.fixture.cooked_file_count + 1U &&
        plan.plan.records[plan.plan.record_count - 1U].entry.value == RUNTIME_ASSET_SMOKE_SCENE_ENTRY.value;
}

RuntimeAssetPackagedRunRequest BuildPackagedRunRequest(CookedVisualProofContext &context, PackageCookRunSmokeLedger &ledger) {
    RuntimeAssetPackagedRunRequest request{};
    request.mount_table = &context.fixture.table;
    request.mount = MountId(MOUNT_ID);
    request.package_load_plan = &ledger.package_load_plan;
    request.scene = context.fixture.command.fixture.cooked_scene;
    request.files = context.fixture.cooked_files.data();
    request.file_count = static_cast<std::uint32_t>(context.fixture.cooked_files.size());
    request.resource_registry = &context.registry;
    request.asset_manager = &context.manager;
    request.rhi_device = &context.device;
    request.loaded_files = context.loaded_files.data();
    request.loaded_file_capacity = static_cast<std::uint32_t>(context.loaded_files.size());
    request.scene_resource_refs = context.scene_refs.data();
    request.scene_resource_ref_capacity = static_cast<std::uint32_t>(context.scene_refs.size());
    request.scene_cameras = context.scene_cameras.data();
    request.scene_camera_capacity = static_cast<std::uint32_t>(context.scene_cameras.size());
    request.scene_entities = context.scene_entities.data();
    request.scene_entity_capacity = static_cast<std::uint32_t>(context.scene_entities.size());
    request.scene_transforms = context.scene_transforms.data();
    request.scene_transform_capacity = static_cast<std::uint32_t>(context.scene_transforms.size());
    request.scene_output = &context.scene_output;
    request.shader_program = &context.shader_program;
    request.animation_frame_context.frame_index = 1U;
    request.animation_frame_context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    request.animation_frame_context.fixed_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    request.animation_frame_context.phase = RuntimeFramePhase::LoadOrCommitResources;
    request.scratch_bytes = std::span<std::uint8_t>(context.scratch_bytes.data(), context.scratch_bytes.size());
    request.capture_output = std::span<std::uint8_t>(context.capture_bytes.data(), context.capture_bytes.size());
    request.capture_byte_budget_per_entity = CAPTURE_BYTES_PER_ENTITY;
    request.first_frame_id = FRAME_ID + 700U;
    request.visual_frame_count = 2U;
    request.output_path = "Artifacts/RuntimeAssetData/PackageCookRun.rvf";
    request.output_path_byte_count = 44U;
    request.runtime_app.frame_count = 1U;
    request.runtime_app.fixed_delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    return request;
}

ResourceLogicalKey RuntimeAssetSmokeSceneLogicalKey(const RuntimeAssetFileDesc &scene_desc) {
    return ResourceLogicalKey(std::string("runtime_asset_") + std::to_string(scene_desc.stable_id));
}

RuntimeAssetPackageArtifactProductRunRequest BuildProductRunCommandRequest(
    CookedVisualProofContext &context,
    PackageCookRunSmokeLedger &ledger,
    const char *artifact_path = RUNTIME_ASSET_SMOKE_PACKAGE_ARTIFACT_PATH) {
    RuntimeAssetPackageArtifactProductRunRequest request{};
    request.mount_table = &context.fixture.table;
    request.mount = MountId(MOUNT_ID);
    request.package_artifact_path = VirtualPath(artifact_path);
    request.package = RUNTIME_ASSET_SMOKE_PACKAGE;
    request.scene_resource_type = context.fixture.command.fixture.cooked_scene.resource_type;
    request.scene_logical_key = RuntimeAssetSmokeSceneLogicalKey(context.fixture.command.fixture.cooked_scene);
    request.packaged_run = BuildPackagedRunRequest(context, ledger);
    request.packaged_run.package_load_plan = nullptr;
    return request;
}

PackageCookRunSmokeLedger ExecutePackagedPackageCookRunSmoke() {
    PackageCookRunSmokeLedger ledger{};
    CookedVisualProofContext context{};

    if (!ExecuteGeneratedFixtureCommand("PackageCookRunSmoke", &context.fixture)) {
        return ledger;
    }

    ledger.import_cook_status = context.fixture.command.status;
    ledger.source_file_count = context.fixture.command.fixture.source_file_count;
    ledger.cooked_file_count = context.fixture.command.fixture.cooked_file_count;
    ledger.import_cook_command_success =
        context.fixture.command.status == RuntimeAssetDataStatus::Success &&
        context.fixture.command.fixture.wrote_to_disk &&
        context.fixture.command.fixture.validated_source_files &&
        context.fixture.command.fixture.validated_cooked_files;
    if (!ledger.import_cook_command_success) {
        return ledger;
    }

    if (!BuildRuntimeAssetCookedPackagePlan(context, &ledger)) {
        return ledger;
    }

    if (context.device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return ledger;
    }

    RuntimeAssetPackagedRunRequest request = BuildPackagedRunRequest(context, ledger);
    RunRuntimeAssetPackagedEntryPoint(request, &ledger.packaged_run);
    return ledger;
}

int RuntimeAssetDataCookedRecordsDriveRuntimeVisualProofThroughRenderCoreRhi() {
    CookedVisualProofContext context{};
    if (!SetupCookedVisualProofContext("CookedVisualProofSuccess", &context)) {
        return Fail("cooked visual proof setup failed");
    }

    RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context);
    RuntimeAssetVisualProofResult result{};
    const RuntimeAssetDataStatus status = BuildRuntimeAssetCookedVisualProofRoute(request, &result);
    if (status != RuntimeAssetDataStatus::Success ||
        result.status != RuntimeAssetDataStatus::Success ||
        result.first_missing_layer != RuntimeAssetVisualProofMissingLayer::None) {
        std::fprintf(
            stderr,
            "status=%s result=%s layer=%s capture_bytes=%zu draws=%u\n",
            StatusName(status),
            StatusName(result.status),
            VisualProofLayerName(result.first_missing_layer),
            result.capture_bytes_written,
            result.submitted_draw_count);
        for (const RuntimeAssetLoadedFile &file : context.loaded_files) {
            if (file.kind == RuntimeAssetFileKind::Mesh) {
                std::fprintf(
                    stderr,
                    "mesh stable=%llu geom=%u vertices=%u indices=%u asset=%u/%u resource=%u/%u\n",
                    static_cast<unsigned long long>(file.stable_id),
                    static_cast<unsigned int>(file.mesh_geometry_kind),
                    file.vertex_count,
                    file.index_count,
                    file.asset.slot,
                    file.asset.generation,
                    file.resource.slot,
                    file.resource.generation);
            }
        }
        return Fail("cooked visual proof route rejected generated cooked records");
    }

    if (!result.loaded_records_verified ||
        result.cooked_record_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U ||
        result.source_record_count != 0U ||
        result.mesh_record_count != 3U ||
        result.texture_record_count != 3U) {
        return Fail("cooked visual proof did not verify the loaded RuntimeAsset ledger");
    }

    if (!result.shader_pipeline_from_runtime_asset ||
        !result.mesh_buffers_from_decoded_payloads ||
        !result.material_slots_from_cooked_payloads ||
        !result.scene_transforms_from_animation_sampling ||
        !result.render_scene_routed ||
        !result.render_core_rhi_capture_routed) {
        return Fail("cooked visual proof did not route every required runtime layer");
    }

    const std::size_t expected_capture_bytes =
        static_cast<std::size_t>(result.submitted_draw_count) * RUNTIME_TEXTURE_BYTE_COUNT;
    if (result.scene_entity_count != 3U ||
        result.scene_transform_count != 3U ||
        result.scene_camera_count != 1U ||
        result.animation_sampled_value_count == 0U ||
        result.material_texture_slot_count != RUNTIME_TEXTURE_SLOT_COUNT ||
        result.runtime_texture_upload_count != RUNTIME_TEXTURE_SLOT_COUNT ||
        result.completed_frame_count != 2U ||
        result.submitted_draw_count != 6U ||
        result.capture_bytes_written != expected_capture_bytes) {
        std::fprintf(
            stderr,
            "entities=%u transforms=%u cameras=%u anim=%u slots=%u uploads=%u frames=%u draws=%u capture=%zu expected_capture=%zu\n",
            result.scene_entity_count,
            result.scene_transform_count,
            result.scene_camera_count,
            result.animation_sampled_value_count,
            result.material_texture_slot_count,
            result.runtime_texture_upload_count,
            result.completed_frame_count,
            result.submitted_draw_count,
            result.capture_bytes_written,
            expected_capture_bytes);
        return Fail("cooked visual proof route produced incomplete capture ledger");
    }

    return 0;
}

bool ExpectCookedVisualProofMissingLayer(
    RuntimeAssetVisualProofRequest request,
    RuntimeAssetDataStatus expected_status,
    RuntimeAssetVisualProofMissingLayer expected_layer);

int RuntimeAssetDataImportedMeshPayloadBytesFeedRenderGeometryBuffers() {
    std::size_t expected_vertex_payload_bytes = 0U;
    expected_vertex_payload_bytes += MeshVertexPayloadByteCount(24U);
    expected_vertex_payload_bytes += MeshVertexPayloadByteCount(18U);
    expected_vertex_payload_bytes += MeshVertexPayloadByteCount(10U);

    std::size_t expected_index_payload_bytes = 0U;
    expected_index_payload_bytes += MeshIndexPayloadByteCount(36U);
    expected_index_payload_bytes += MeshIndexPayloadByteCount(96U);
    expected_index_payload_bytes += MeshIndexPayloadByteCount(48U);

    {
        CookedVisualProofContext context{};
        if (!SetupCookedVisualProofContext("DecodedMeshPayloadGeometryBuffers", &context)) {
            return Fail("decoded mesh payload visual proof setup failed");
        }

        RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context);
        RuntimeAssetVisualProofResult result{};
        const RuntimeAssetDataStatus status = BuildRuntimeAssetCookedVisualProofRoute(request, &result);
        if (status != RuntimeAssetDataStatus::Success ||
            result.status != RuntimeAssetDataStatus::Success ||
            result.first_missing_layer != RuntimeAssetVisualProofMissingLayer::None) {
            return Fail("decoded mesh payload visual route did not close");
        }

        if (!result.mesh_buffers_from_decoded_payloads ||
            result.mesh_decoded_payload_count != 3U ||
            result.mesh_vertex_payload_byte_count != expected_vertex_payload_bytes ||
            result.mesh_index_payload_byte_count != expected_index_payload_bytes) {
            std::fprintf(
                stderr,
                "mesh_payloads=%u vertex=%zu expected_vertex=%zu index=%zu expected_index=%zu decoded=%u\n",
                result.mesh_buffers_from_decoded_payloads ? 1U : 0U,
                result.mesh_vertex_payload_byte_count,
                expected_vertex_payload_bytes,
                result.mesh_index_payload_byte_count,
                expected_index_payload_bytes,
                result.mesh_decoded_payload_count);
            return Fail("decoded mesh payload bytes did not feed geometry buffers");
        }
    }

    {
        CookedVisualProofContext context{};
        if (!SetupCookedVisualProofContext("DecodedMeshPayloadMissingPayload", &context)) {
            return Fail("decoded mesh missing payload setup failed");
        }

        std::array<RuntimeAssetLoadedFile, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> loaded_files =
            context.loaded_files;
        loaded_files[0U].decoded_payload_id += 700000U;
        RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context, 1U);
        request.loaded_files = std::span<const RuntimeAssetLoadedFile>(loaded_files.data(), loaded_files.size());
        if (!ExpectCookedVisualProofMissingLayer(
                request,
                RuntimeAssetDataStatus::MissingDependency,
                RuntimeAssetVisualProofMissingLayer::Model)) {
            return Fail("missing mesh decoded payload was not localized");
        }
    }

    {
        CookedVisualProofContext context{};
        if (!SetupCookedVisualProofContext("DecodedMeshPayloadHashMismatch", &context)) {
            return Fail("decoded mesh hash mismatch setup failed");
        }

        std::array<RuntimeAssetLoadedFile, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> loaded_files =
            context.loaded_files;
        loaded_files[0U].payload_hash ^= 1ULL;
        RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context, 1U);
        request.loaded_files = std::span<const RuntimeAssetLoadedFile>(loaded_files.data(), loaded_files.size());
        if (!ExpectCookedVisualProofMissingLayer(
                request,
                RuntimeAssetDataStatus::HashMismatch,
                RuntimeAssetVisualProofMissingLayer::Model)) {
            return Fail("mesh decoded payload hash mismatch was not localized");
        }
    }

    return 0;
}

int RuntimeAssetDataD3D11HardwareCookedRecordsDriveDeviceBackedVisualProof() {
    CookedVisualProofContext context{};
    if (!SetupCookedVisualProofContext("CookedVisualProofD3D11", &context)) {
        return Fail("D3D11 cooked visual proof setup failed");
    }

    RuntimeAssetD3D11DeviceContext d3d11_context{};
    const int device_status = CreateD3D11RuntimeAssetDevice(&d3d11_context);
    if (device_status != 0) {
        return device_status;
    }

    RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context);
    request.rhi_device = d3d11_context.device;

    RuntimeAssetVisualProofResult result{};
    const RuntimeAssetDataStatus status = BuildRuntimeAssetCookedVisualProofRoute(request, &result);
    if (status != RuntimeAssetDataStatus::Success ||
        result.status != RuntimeAssetDataStatus::Success ||
        result.first_missing_layer != RuntimeAssetVisualProofMissingLayer::None) {
        const auto snapshot = d3d11_context.device->Snapshot();
        std::fprintf(
            stderr,
            "BlockedByLayer=%s status=%s result=%s shader=%s material=%s material_rhi=%u capture_bytes=%zu draws=%u targets=%zu buffers=%zu textures=%zu failed_rhi=%llu\n",
            VisualProofLayerName(result.first_missing_layer),
            StatusName(status),
            StatusName(result.status),
            StatusName(result.shader_pipeline_result.status),
            StatusName(result.material_result.status),
            static_cast<unsigned>(result.material_result.rhi_status),
            result.capture_bytes_written,
            result.submitted_draw_count,
            snapshot.color_target_count,
            snapshot.resources.buffer_count,
            snapshot.resources.texture_count,
            static_cast<unsigned long long>(snapshot.failed_operation_count));
        return Fail("D3D11 RuntimeAsset visual route did not close");
    }

    const std::size_t expected_capture_bytes =
        static_cast<std::size_t>(result.submitted_draw_count) * RUNTIME_TEXTURE_BYTE_COUNT;
    if (!result.loaded_records_verified ||
        !result.shader_pipeline_from_runtime_asset ||
        !result.mesh_buffers_from_decoded_payloads ||
        !result.material_slots_from_cooked_payloads ||
        !result.scene_transforms_from_animation_sampling ||
        !result.render_scene_routed ||
        !result.render_core_rhi_capture_routed ||
        result.completed_frame_count != 2U ||
        result.submitted_draw_count != 6U ||
        result.capture_bytes_written != expected_capture_bytes) {
        std::fprintf(
            stderr,
            "D3D11 ledger records=%u shader=%u material=%u anim=%u scene=%u core=%u frames=%u draws=%u capture=%zu expected=%zu\n",
            result.cooked_record_count,
            result.shader_pipeline_from_runtime_asset ? 1U : 0U,
            result.material_slots_from_cooked_payloads ? 1U : 0U,
            result.scene_transforms_from_animation_sampling ? 1U : 0U,
            result.render_scene_routed ? 1U : 0U,
            result.render_core_rhi_capture_routed ? 1U : 0U,
            result.completed_frame_count,
            result.submitted_draw_count,
            result.capture_bytes_written,
            expected_capture_bytes);
        return Fail("D3D11 RuntimeAsset visual route ledger was incomplete");
    }

    const auto snapshot = d3d11_context.device->Snapshot();
    if (snapshot.submitted_indexed_draw_count < static_cast<std::uint64_t>(result.submitted_draw_count) ||
        snapshot.present_count < static_cast<std::uint64_t>(result.submitted_draw_count) ||
        snapshot.capture_count < static_cast<std::uint64_t>(result.submitted_draw_count) ||
        snapshot.last_capture_bytes_written != RUNTIME_TEXTURE_BYTE_COUNT ||
        snapshot.last_capture_extent.width != 2U ||
        snapshot.last_capture_extent.height != 2U) {
        std::fprintf(
            stderr,
            "D3D11 snapshot draws=%llu presents=%llu captures=%llu last_capture=%zu extent=%ux%u\n",
            static_cast<unsigned long long>(snapshot.submitted_indexed_draw_count),
            static_cast<unsigned long long>(snapshot.present_count),
            static_cast<unsigned long long>(snapshot.capture_count),
            snapshot.last_capture_bytes_written,
            snapshot.last_capture_extent.width,
            snapshot.last_capture_extent.height);
        return Fail("D3D11 RHI did not submit/present/capture the RuntimeAsset route");
    }

    return 0;
}

bool ExpectCookedVisualProofMissingLayer(
    RuntimeAssetVisualProofRequest request,
    RuntimeAssetDataStatus expected_status,
    RuntimeAssetVisualProofMissingLayer expected_layer) {
    RuntimeAssetVisualProofResult result{};
    const RuntimeAssetDataStatus status = BuildRuntimeAssetCookedVisualProofRoute(request, &result);
    if (status != expected_status ||
        result.status != expected_status ||
        result.first_missing_layer != expected_layer ||
        result.completed_frame_count != 0U) {
        std::fprintf(
            stderr,
            "expected=%s/%s actual=%s/%s completed=%u\n",
            StatusName(expected_status),
            VisualProofLayerName(expected_layer),
            StatusName(status),
            VisualProofLayerName(result.first_missing_layer),
            result.completed_frame_count);
        return FailStep("cooked visual proof did not report expected missing layer");
    }

    return true;
}

int RuntimeAssetDataCookedRuntimeVisualProofReportsExactMissingLayers() {
    {
        CookedVisualProofContext context{};
        if (!SetupCookedVisualProofContext("CookedVisualProofMissingModel", &context)) {
            return Fail("missing model setup failed");
        }

        std::array<RuntimeAssetLoadedFile, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> loaded_files =
            context.loaded_files;
        loaded_files[0U].mesh_geometry_kind = yuengine::runtimeasset::RuntimeAssetMeshGeometryKind::Unknown;
        RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context, 1U);
        request.loaded_files = std::span<const RuntimeAssetLoadedFile>(loaded_files.data(), loaded_files.size());
        if (!ExpectCookedVisualProofMissingLayer(
                request,
                RuntimeAssetDataStatus::MissingDependency,
                RuntimeAssetVisualProofMissingLayer::Model)) {
            return Fail("missing model was not localized");
        }
    }

    {
        CookedVisualProofContext context{};
        if (!SetupCookedVisualProofContext("CookedVisualProofMissingMaterialSlot", &context)) {
            return Fail("missing material slot setup failed");
        }

        std::array<RuntimeAssetLoadedFile, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> loaded_files =
            context.loaded_files;
        loaded_files[3U].texture_slot_count = 0U;
        RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context, 1U);
        request.loaded_files = std::span<const RuntimeAssetLoadedFile>(loaded_files.data(), loaded_files.size());
        if (!ExpectCookedVisualProofMissingLayer(
                request,
                RuntimeAssetDataStatus::MissingDependency,
                RuntimeAssetVisualProofMissingLayer::MaterialSlot)) {
            return Fail("missing material slot was not localized");
        }
    }

    {
        CookedVisualProofContext context{};
        if (!SetupCookedVisualProofContext("CookedVisualProofMissingShaderPipeline", &context)) {
            return Fail("missing shader pipeline setup failed");
        }

        RuntimeAssetLoadedShaderProgramData shader_program = context.shader_program;
        shader_program.vertex_bytecode_size = 0U;
        RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context, 1U);
        request.shader_program = &shader_program;
        if (!ExpectCookedVisualProofMissingLayer(
                request,
                RuntimeAssetDataStatus::RhiPipelineFailed,
                RuntimeAssetVisualProofMissingLayer::ShaderPipeline)) {
            return Fail("missing shader pipeline was not localized");
        }
    }

    {
        CookedVisualProofContext context{};
        if (!SetupCookedVisualProofContext("CookedVisualProofShaderCompilerMismatch", &context)) {
            return Fail("shader compiler mismatch setup failed");
        }

        RuntimeAssetLoadedShaderProgramData shader_program = context.shader_program;
        shader_program.status = RuntimeAssetDataStatus::HashMismatch;
        RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context, 1U);
        request.shader_program = &shader_program;
        if (!ExpectCookedVisualProofMissingLayer(
                request,
                RuntimeAssetDataStatus::HashMismatch,
                RuntimeAssetVisualProofMissingLayer::ShaderPipeline)) {
            return Fail("shader compiler mismatch was not localized");
        }
    }

    {
        CookedVisualProofContext context{};
        if (!SetupCookedVisualProofContext("CookedVisualProofMissingSceneTransform", &context)) {
            return Fail("missing scene transform setup failed");
        }

        RuntimeAssetSceneLoaderOutput scene_output = context.scene_output;
        scene_output.transform_count = 0U;
        RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context, 1U);
        request.scene_output = &scene_output;
        if (!ExpectCookedVisualProofMissingLayer(
                request,
                RuntimeAssetDataStatus::MissingDependency,
                RuntimeAssetVisualProofMissingLayer::SceneTransform)) {
            return Fail("missing scene transform was not localized");
        }
    }

    {
        CookedVisualProofContext context{};
        if (!SetupCookedVisualProofContext("CookedVisualProofMissingCamera", &context)) {
            return Fail("missing camera setup failed");
        }

        std::array<RuntimeAssetSceneCameraRecord, 1U> cameras = context.scene_cameras;
        cameras[0U].is_active = false;
        RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context, 1U);
        request.scene_cameras = std::span<const RuntimeAssetSceneCameraRecord>(cameras.data(), cameras.size());
        if (!ExpectCookedVisualProofMissingLayer(
                request,
                RuntimeAssetDataStatus::MissingDependency,
                RuntimeAssetVisualProofMissingLayer::Camera)) {
            return Fail("missing camera was not localized");
        }
    }

    {
        CookedVisualProofContext context{};
        if (!SetupCookedVisualProofContext("CookedVisualProofMissingRhiCapture", &context)) {
            return Fail("missing rhi capture setup failed");
        }

        RuntimeAssetVisualProofRequest request = CookedVisualProofRequest(context, 1U);
        request.rhi_device = nullptr;
        if (!ExpectCookedVisualProofMissingLayer(
                request,
                RuntimeAssetDataStatus::RhiCaptureFailed,
                RuntimeAssetVisualProofMissingLayer::RhiCapture)) {
            return Fail("missing RHI capture was not localized");
        }
    }

    return 0;
}

int RuntimeAssetDataPackageCookRunSmokeRunsPackagedRuntimeEntryPoint() {
    const PackageCookRunSmokeLedger ledger = ExecutePackagedPackageCookRunSmoke();
    if (!ledger.import_cook_command_success ||
        ledger.source_file_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        ledger.cooked_file_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT) {
        std::fprintf(
            stderr,
            "import=%s source=%u cooked=%u\n",
            StatusName(ledger.import_cook_status),
            ledger.source_file_count,
            ledger.cooked_file_count);
        return Fail("package smoke import/cook command floor failed");
    }

    if (!ledger.package_manifest_success ||
        !ledger.package_artifact_write_success ||
        !ledger.package_artifact_read_success ||
        !ledger.package_load_plan_success ||
        ledger.package_load_plan_record_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U) {
        std::fprintf(
            stderr,
            "package=%u artifactWrite=%u artifactRead=%u planRecords=%u\n",
            static_cast<unsigned>(ledger.package_status),
            ledger.package_artifact_write_success ? 1U : 0U,
            ledger.package_artifact_read_success ? 1U : 0U,
            ledger.package_load_plan_record_count);
        return Fail("package smoke Package artifact/manifest/load-plan floor failed");
    }

    if (!ledger.package_artifact_write.wrote_artifact ||
        !ledger.package_artifact_read.read_artifact ||
        !ledger.package_artifact_read.rebuilt_registry ||
        ledger.package_artifact_write.artifact_byte_count == 0U ||
        ledger.package_artifact_read.artifact_byte_count == 0U ||
        ledger.package_artifact_read.entry_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U ||
        ledger.package_artifact_read.dependency_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT) {
        std::fprintf(
            stderr,
            "artifact write=%zu read=%zu entries=%u deps=%u rebuilt=%u\n",
            ledger.package_artifact_write.artifact_byte_count,
            ledger.package_artifact_read.artifact_byte_count,
            ledger.package_artifact_read.entry_count,
            ledger.package_artifact_read.dependency_count,
            ledger.package_artifact_read.rebuilt_registry ? 1U : 0U);
        return Fail("package smoke did not consume file-backed artifact metadata");
    }

    const RuntimeAssetPackagedRunResult &run = ledger.packaged_run;
    if (run.status != RuntimeAssetDataStatus::Success ||
        run.blocked_layer != RuntimeAssetPackagedRunBlockedLayer::None ||
        !run.package_load_plan_consumed ||
        !run.packaged_runtime_entrypoint_available) {
        std::fprintf(
            stderr,
            "BlockedByLayer=%s status=%s consumed=%u available=%u package=%u records=%u\n",
            PackagedRunBlockedLayerName(run.blocked_layer),
            StatusName(run.status),
            run.package_load_plan_consumed ? 1U : 0U,
            run.packaged_runtime_entrypoint_available ? 1U : 0U,
            static_cast<unsigned>(run.package_status),
            run.package_load_plan_record_count);
        return Fail("package smoke packaged RuntimeAsset entrypoint did not close");
    }

    if (!run.runtime_asset_validation_load_success ||
        !run.resource_asset_registration_success ||
        run.loaded_file_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        run.resource_dependency_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        run.asset_dependency_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT) {
        std::fprintf(
            stderr,
            "BlockedByLayer=%s load=%s loaded=%u resourceDeps=%u assetDeps=%u\n",
            PackagedRunBlockedLayerName(run.blocked_layer),
            StatusName(run.graph_load_result.status),
            run.loaded_file_count,
            run.resource_dependency_count,
            run.asset_dependency_count);
        return Fail("package smoke RuntimeAsset load floor failed");
    }

    if (!run.shader_program_decoded ||
        !run.render_scene_render_core_rhi_success ||
        run.visual_proof_result.status != RuntimeAssetDataStatus::Success ||
        run.visual_proof_result.completed_frame_count != 2U ||
        run.visual_proof_result.submitted_draw_count != 6U ||
        !run.visual_proof_result.loaded_records_verified) {
        std::fprintf(
            stderr,
            "BlockedByLayer=%s shader=%u visual=%s frames=%u draws=%u records=%u\n",
            PackagedRunBlockedLayerName(run.blocked_layer),
            run.shader_program_decoded ? 1U : 0U,
            StatusName(run.visual_proof_result.status),
            run.visual_proof_result.completed_frame_count,
            run.visual_proof_result.submitted_draw_count,
            run.visual_proof_result.loaded_records_verified ? 1U : 0U);
        return Fail("package smoke RenderScene/RenderCore/RHI floor failed");
    }

    if (!run.runtime_app_frame_loop_success ||
        !run.runtime_app_result.succeeded ||
        run.runtime_app_completed_frame_count != 1U ||
        run.runtime_app_result.last_frame_context.phase != RuntimeFramePhase::EndFrame) {
        std::fprintf(
            stderr,
            "BlockedByLayer=%s runtimeApp=%u frames=%u phase=%u\n",
            PackagedRunBlockedLayerName(run.blocked_layer),
            static_cast<unsigned>(run.runtime_app_result.status),
            run.runtime_app_completed_frame_count,
            static_cast<unsigned>(run.runtime_app_result.last_frame_context.phase));
        return Fail("package smoke RuntimeApp frame-loop floor failed");
    }

    return 0;
}

int RuntimeAssetDataPackageArtifactCookRunSmokeRunsProductRuntimeEntryPoint() {
    return RuntimeAssetDataPackageCookRunSmokeRunsPackagedRuntimeEntryPoint();
}

int RuntimeAssetDataProductRunCommandConsumesPackageArtifactPath() {
    PackageCookRunSmokeLedger ledger{};
    CookedVisualProofContext context{};
    if (!ExecuteGeneratedFixtureCommand("ProductRunCommandSuccess", &context.fixture)) {
        return Fail("product run command fixture setup failed");
    }

    if (!BuildRuntimeAssetCookedPackagePlan(context, &ledger)) {
        return Fail("product run command package artifact setup failed");
    }

    if (context.device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("product run command rhi init failed");
    }

    RuntimeAssetPackageArtifactProductRunRequest request =
        BuildProductRunCommandRequest(context, ledger);
    if (request.packaged_run.package_load_plan != nullptr) {
        return Fail("product run command fixture still carried caller load plan");
    }

    RuntimeAssetPackageArtifactProductRunResult result{};
    const RuntimeAssetDataStatus status =
        RunRuntimeAssetPackageArtifactProductCommand(request, &result);
    if (status != RuntimeAssetDataStatus::Success ||
        result.status != RuntimeAssetDataStatus::Success ||
        result.missing_layer != RuntimeAssetPackageArtifactProductRunMissingLayer::None) {
        std::fprintf(
            stderr,
            "status=%s result=%s layer=%u package=%u file=%u records=%u\n",
            StatusName(status),
            StatusName(result.status),
            static_cast<unsigned>(result.missing_layer),
            static_cast<unsigned>(result.package_status),
            static_cast<unsigned>(result.file_status),
            result.package_load_plan_record_count);
        return Fail("product run command did not consume artifact path");
    }

    if (!result.package_artifact_read ||
        !result.package_registry_rebuilt ||
        !result.package_load_plan_resolved ||
        !result.packaged_run_executed ||
        result.package_load_plan_record_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U) {
        return Fail("product run command did not expose artifact/load-plan ledger");
    }

    const RuntimeAssetPackagedRunResult &run = result.packaged_run;
    if (run.blocked_layer != RuntimeAssetPackagedRunBlockedLayer::None ||
        !run.package_load_plan_consumed ||
        !run.packaged_runtime_entrypoint_available ||
        !run.runtime_app_frame_loop_success) {
        std::fprintf(
            stderr,
            "BlockedByLayer=%s consumed=%u available=%u runtimeApp=%u\n",
            PackagedRunBlockedLayerName(run.blocked_layer),
            run.package_load_plan_consumed ? 1U : 0U,
            run.packaged_runtime_entrypoint_available ? 1U : 0U,
            run.runtime_app_frame_loop_success ? 1U : 0U);
        return Fail("product run command did not run packaged RuntimeAsset entrypoint");
    }

    return 0;
}

int RuntimeAssetDataProductRunCommandReportsMissingPackageArtifactPath() {
    PackageCookRunSmokeLedger ledger{};
    CookedVisualProofContext context{};
    if (!ExecuteGeneratedFixtureCommand("ProductRunCommandMissingArtifact", &context.fixture)) {
        return Fail("missing artifact product command fixture setup failed");
    }

    RuntimeAssetPackageArtifactProductRunRequest request =
        BuildProductRunCommandRequest(context, ledger, "Package/MissingProductRun.yupackage");
    RuntimeAssetPackageArtifactProductRunResult result{};
    const RuntimeAssetDataStatus status =
        RunRuntimeAssetPackageArtifactProductCommand(request, &result);
    if (status != RuntimeAssetDataStatus::FileReadFailed ||
        result.status != RuntimeAssetDataStatus::FileReadFailed ||
        result.missing_layer != RuntimeAssetPackageArtifactProductRunMissingLayer::FileVfs ||
        result.package_artifact_read ||
        result.package_registry_rebuilt ||
        result.package_load_plan_resolved ||
        result.packaged_run_executed) {
        std::fprintf(
            stderr,
            "status=%s result=%s layer=%u artifact=%u rebuilt=%u plan=%u executed=%u\n",
            StatusName(status),
            StatusName(result.status),
            static_cast<unsigned>(result.missing_layer),
            result.package_artifact_read ? 1U : 0U,
            result.package_registry_rebuilt ? 1U : 0U,
            result.package_load_plan_resolved ? 1U : 0U,
            result.packaged_run_executed ? 1U : 0U);
        return Fail("missing package artifact did not stop product command at File/VFS");
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
            "cam=Camera/Main.yucamera\n",
            RuntimeAssetFileKind::Scene,
            RuntimeAssetDataStatus::InvalidSchema)) {
        return Fail("scene without source schema was not rejected");
    }

    RuntimeAssetValidationResult camera_result{};
    if (!ValidateText(files[9U].bytes, RuntimeAssetFileKind::Camera, &camera_result)) {
        return Fail("camera schema validator rejected canonical camera");
    }

    if (camera_result.schema_version != 1U || camera_result.identity_hash == 0U) {
        return Fail("camera schema metadata was not recorded");
    }

    if (!ExpectValidationStatus(
            "YUASSET CAMERA 1\n"
            "id=main_camera\n"
            "projection=perspective\n"
            "fov_degrees=55\n"
            "near=0.1\n"
            "far=100\n"
            "keyframes=3\n"
            "key0=0:-4,2,-6:0,0,0\n"
            "key1=0.5:0,3,-5:0,0,0\n"
            "key2=1:4,2,-6:0,0,0\n",
            RuntimeAssetFileKind::Camera,
            RuntimeAssetDataStatus::InvalidSchema)) {
        return Fail("camera without source schema was not rejected");
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

int RuntimeAssetDataCameraTweenDescriptorLoadsFromDiskSceneReference() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("CameraTweenDescriptor"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("generator write failed");
    }

    std::vector<std::uint8_t> camera_bytes{};
    if (!ReadFile(table, "Camera/Main.yucamera", &camera_bytes)) {
        return Fail("camera file read failed");
    }

    RuntimeAssetValidationResult camera_result{};
    const RuntimeAssetDataStatus camera_status = ValidateRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(camera_bytes.data(), camera_bytes.size()),
        RuntimeAssetFileKind::Camera,
        &camera_result);
    if (camera_status != RuntimeAssetDataStatus::Success) {
        return Fail("camera descriptor validator failed");
    }

    if (camera_result.dependency_count != 3U ||
        camera_result.schema_version != 1U ||
        camera_result.identity_hash == 0U) {
        return Fail("camera tween descriptor metadata changed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("loaded graph failed");
    }

    if (graph.load_result.loaded_file_count != FIXTURE_FILE_COUNT ||
        graph.scene_output.resource_ref_count != FIXTURE_FILE_COUNT) {
        return Fail("camera descriptor was not loaded into runtime graph");
    }

    if (graph.assets[9U].kind != RuntimeAssetFileKind::Camera ||
        graph.assets[9U].stable_id != 7001U ||
        !graph.assets[9U].cache_payload_stored) {
        return Fail("camera descriptor loaded file identity changed");
    }

    if (graph.scene_resource_refs[9U].kind != RuntimeAssetFileKind::Camera ||
        graph.scene_resource_refs[9U].stable_id != 7001U ||
        !graph.scene_resource_refs[9U].resource.IsValid() ||
        !graph.scene_resource_refs[9U].asset.IsValid()) {
        return Fail("camera descriptor resource ref was not staged");
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

int RuntimeAssetDataLoaderRejectsOversizedFileCountBeforeReadAndMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("LoaderFileCountPreflightNoMutation"), &table)) {
        return Fail("mount setup failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const AssetSnapshot before_asset_snapshot = manager.Snapshot();

    const std::uint32_t oversized_file_count = yuengine::resource::MAX_RESOURCE_COUNT;
    std::vector<RuntimeAssetFileDesc> file_descs(oversized_file_count);
    std::vector<RuntimeAssetLoadedFile> loaded_files(oversized_file_count);
    std::vector<RuntimeAssetSceneResourceRef> scene_resource_refs(oversized_file_count);
    std::array<RuntimeAssetSceneCameraRecord, 1U> scene_cameras{};
    std::array<RuntimeAssetSceneEntityRecord, 1U> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, 1U> scene_transforms{};
    RuntimeAssetSceneLoaderOutput scene_output{};
    scene_output.status = RuntimeAssetDataStatus::BudgetExceeded;
    scene_resource_refs[0U].stable_id = 0xBAD0U;
    loaded_files[0U].stable_id = 0xBAD1U;

    RuntimeAssetGraphLoadRequest load_request{};
    load_request.mount_table = &table;
    load_request.mount = MountId(MOUNT_ID);
    load_request.scene_path = VirtualPath("Scene/OversizedShouldNotRead.yuscene");
    load_request.scene_resource_type = ResourceTypeId{RESOURCE_TYPE_SCENE};
    load_request.scene_asset_type = AssetTypeId{ASSET_TYPE_SCENE};
    load_request.scene_stable_id = 7001U;
    load_request.files = file_descs.data();
    load_request.file_count = oversized_file_count;
    load_request.resource_registry = &registry;
    load_request.asset_manager = &manager;
    load_request.loaded_files = loaded_files.data();
    load_request.loaded_file_capacity = oversized_file_count;
    load_request.scene_resource_refs = scene_resource_refs.data();
    load_request.scene_resource_ref_capacity = oversized_file_count;
    load_request.scene_cameras = scene_cameras.data();
    load_request.scene_camera_capacity = static_cast<std::uint32_t>(scene_cameras.size());
    load_request.scene_entities = scene_entities.data();
    load_request.scene_entity_capacity = static_cast<std::uint32_t>(scene_entities.size());
    load_request.scene_transforms = scene_transforms.data();
    load_request.scene_transform_capacity = static_cast<std::uint32_t>(scene_transforms.size());
    load_request.scene_output = &scene_output;

    RuntimeAssetGraphLoadResult load_result{};
    const RuntimeAssetDataStatus status = LoadRuntimeAssetDataGraph(load_request, &load_result);
    if (status != RuntimeAssetDataStatus::CapacityExceeded ||
        load_result.status != RuntimeAssetDataStatus::CapacityExceeded) {
        return Fail("oversized file count was not rejected in request preflight");
    }

    if (load_result.transaction_plan.status != RuntimeAssetDataStatus::CapacityExceeded ||
        load_result.transaction_plan.phase != RuntimeAssetLoadTransactionPhase::Preflight ||
        load_result.transaction_result.status != RuntimeAssetDataStatus::CapacityExceeded ||
        load_result.transaction_result.phase != RuntimeAssetLoadTransactionPhase::Preflight ||
        load_result.transaction_result.mutated_state) {
        return Fail("oversized file count did not report pre-read transaction diagnostics");
    }

    if (load_result.file_read_count != 0U ||
        load_result.loaded_file_count != 0U ||
        load_result.scene_registered ||
        load_result.resource_dependency_count != 0U ||
        load_result.asset_dependency_count != 0U) {
        return Fail("oversized file count read or committed graph state before rejection");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    const AssetSnapshot after_asset_snapshot = manager.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count ||
        after_resource_snapshot.dependency_edge_count != before_resource_snapshot.dependency_edge_count ||
        after_asset_snapshot.active_asset_count != before_asset_snapshot.active_asset_count ||
        after_asset_snapshot.active_dependency_edge_count != before_asset_snapshot.active_dependency_edge_count) {
        return Fail("oversized file count mutated registry or asset manager before rejection");
    }

    if (scene_output.status != RuntimeAssetDataStatus::BudgetExceeded ||
        scene_resource_refs[0U].stable_id != 0xBAD0U ||
        loaded_files[0U].stable_id != 0xBAD1U) {
        return Fail("oversized file count mutated caller outputs before rejection");
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

bool ExpectValidationFailureDiagnostic(
    std::string_view text,
    RuntimeAssetFileKind kind,
    RuntimeAssetDataStatus expected_status,
    std::uint32_t expected_dependency_index,
    std::uint32_t expected_token_index,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetFileKind actual_kind) {
    const std::vector<std::uint8_t> bytes = BytesFromString(std::string(text));
    RuntimeAssetValidationResult result{};
    const RuntimeAssetDataStatus status = ValidateRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(bytes.data(), bytes.size()),
        kind,
        &result);
    if (status != expected_status || result.status != expected_status) {
        return false;
    }

    if (result.first_failed_dependency_status != expected_status) {
        return false;
    }

    if (result.first_failed_dependency_index != expected_dependency_index ||
        result.first_failed_dependency_token_index != expected_token_index) {
        return false;
    }

    if (result.first_failed_expected_kind != expected_kind ||
        result.first_failed_actual_kind != actual_kind) {
        return false;
    }

    return true;
}

int RuntimeAssetDataValidatorReportsMissingDependencyToken() {
    constexpr std::string_view text =
        "YUASSET SCENE 1\n"
        "schema=rav0-source\n"
        "id=missing_scene_ref\n"
        "m1=Mesh/Cylinder.yumesh\n"
        "m2=Mesh/Cone.yumesh\n"
        "mat=Material/Shared.yumat\n"
        "t0=Texture/Albedo.yutex\n"
        "prog=Shader/RuntimeProgram.yuprogram\n"
        "cam=Camera/Main.yucamera\n"
        "anim=Animation/Spin.yuanim\n";
    if (!ExpectValidationFailureDiagnostic(
            text,
            RuntimeAssetFileKind::Scene,
            RuntimeAssetDataStatus::MissingDependency,
            0U,
            0U,
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetFileKind::Unknown)) {
        return Fail("missing dependency diagnostic did not report first token");
    }

    return 0;
}

int RuntimeAssetDataValidatorReportsDuplicateDependencyToken() {
    constexpr std::string_view text =
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
        "cam=Camera/Main.yucamera\n"
        "anim=Animation/Spin.yuanim\n";
    if (!ExpectValidationFailureDiagnostic(
            text,
            RuntimeAssetFileKind::Scene,
            RuntimeAssetDataStatus::DuplicateDependency,
            0U,
            0U,
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetFileKind::Mesh)) {
        return Fail("duplicate dependency diagnostic did not report first token");
    }

    return 0;
}

int RuntimeAssetDataValidatorReportsTypeMismatchExpectedActual() {
    constexpr std::string_view text =
        "YUASSET MATERIAL 1\n"
        "schema=rav0-source\n"
        "id=shared_material\n"
        "shader=Texture/Albedo.yutex\n"
        "texture0=Texture/Albedo.yutex\n"
        "texture1=Texture/Normal.yutex\n"
        "texture2=Texture/Mask.yutex\n";
    if (!ExpectValidationFailureDiagnostic(
            text,
            RuntimeAssetFileKind::Material,
            RuntimeAssetDataStatus::TypeMismatch,
            0U,
            0U,
            RuntimeAssetFileKind::Shader,
            RuntimeAssetFileKind::Texture)) {
        return Fail("type mismatch diagnostic did not report expected and actual kinds");
    }

    return 0;
}

int RuntimeAssetDataCookedDependencyRowsReportFailedDepIndex() {
    const std::string text =
        CookedTextureText("checker", 2U, 1U, 64U, 4U, HashText("checker"));
    if (!ExpectValidationFailureDiagnostic(
            text,
            RuntimeAssetFileKind::Texture,
            RuntimeAssetDataStatus::MissingDependency,
            1U,
            1U,
            RuntimeAssetFileKind::Unknown,
            RuntimeAssetFileKind::Unknown)) {
        return Fail("cooked dependency diagnostic did not report failed dep index");
    }

    return 0;
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
    constexpr std::uint32_t source_payload_byte_count =
        MeshPayloadByteCount(24U, 36U);
    const std::string source_payload = MeshPayload('A', source_payload_byte_count);
    const std::string source_text = SourceMeshText("YUASSET MESH 1");
    if (!ValidateText(source_text, RuntimeAssetFileKind::Mesh, &source_result)) {
        return Fail("source parser rejected canonical mesh");
    }

    if (source_result.artifact_class != RuntimeAssetArtifactClass::Source ||
        source_result.source_hash != source_result.hash ||
        source_result.record_table_count != 1U ||
        source_result.record_table_byte_count != source_text.size() ||
        source_result.payload_byte_count != source_payload.size() ||
        source_result.payload_alignment != 4U ||
        source_result.payload_hash != HashText(source_payload) ||
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

    auto probe = [misleading_scene_path](
                     std::string_view scene_text,
                     RuntimeAssetDataStatus expected_status) -> bool {
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
            "cam=Camera/Main.yucamera\n"
            "anim=Animation/Spin.yuanim\n",
            RuntimeAssetDataStatus::InvalidSchema)) {
        return Fail("loader did not reject missing scene schema before mutation");
    }

    constexpr std::uint32_t misleading_mesh_vertex_count = 24U;
    constexpr std::uint32_t misleading_mesh_index_count = 36U;
    constexpr std::uint32_t misleading_mesh_vertex_payload_byte_count =
        MeshVertexPayloadByteCount(misleading_mesh_vertex_count);
    constexpr std::uint32_t misleading_mesh_index_payload_byte_count =
        MeshIndexPayloadByteCount(misleading_mesh_index_count);
    constexpr std::uint32_t misleading_mesh_payload_byte_count =
        MeshPayloadByteCount(misleading_mesh_vertex_count, misleading_mesh_index_count);
    const std::string misleading_mesh_payload =
        MeshPayload('A', misleading_mesh_payload_byte_count);
    if (!probe(
            SourceMeshText(
                "YUASSET MESH 1",
                "mesh_inside_yuscene_path",
                "cube",
                misleading_mesh_vertex_count,
                misleading_mesh_index_count,
                misleading_mesh_vertex_payload_byte_count,
                misleading_mesh_index_payload_byte_count,
                misleading_mesh_payload),
            RuntimeAssetDataStatus::InvalidKind)) {
        return Fail("loader trusted misleading .yuscene suffix over internal kind");
    }

    return 0;
}

int RuntimeAssetDataMeshMaterialTextureTypedValidatorsAcceptStructuredMetadata() {
    RuntimeAssetValidationResult mesh_result{};
    constexpr std::uint32_t mesh_payload_byte_count =
        MeshPayloadByteCount(24U, 36U);
    const std::string mesh_payload = MeshPayload('A', mesh_payload_byte_count);
    const std::string mesh_text = SourceMeshText("YUASSET MESH 1");
    if (!ValidateText(mesh_text, RuntimeAssetFileKind::Mesh, &mesh_result)) {
        return Fail("mesh metadata validator rejected valid mesh");
    }

    if (mesh_result.version != 1U || mesh_result.schema_version != 1U ||
        mesh_result.identity_hash == 0U) {
        return Fail("mesh metadata did not report version schema identity");
    }

    if (mesh_result.vertex_count != 24U || mesh_result.index_count != 36U) {
        return Fail("mesh metadata counts were not parsed");
    }

    if (mesh_result.payload_byte_count != mesh_payload.size() ||
        mesh_result.payload_alignment != 4U ||
        mesh_result.payload_hash != HashText(mesh_payload)) {
        return Fail("mesh payload metadata was not parsed");
    }

    if (mesh_result.mesh_input_layout.element_count != 2U ||
        mesh_result.mesh_input_layout.stride_bytes != MESH_VERTEX_STRIDE_BYTES ||
        mesh_result.mesh_input_layout.elements[0U].semantic != RhiInputElementSemantic::Position ||
        mesh_result.mesh_input_layout.elements[1U].semantic != RhiInputElementSemantic::TexCoord ||
        mesh_result.mesh_index_format != RhiIndexFormat::Uint16 ||
        mesh_result.mesh_topology != RhiPrimitiveTopology::TriangleList ||
        mesh_result.mesh_vertex_stride_bytes != MESH_VERTEX_STRIDE_BYTES ||
        mesh_result.mesh_index_stride_bytes != MESH_INDEX_STRIDE_BYTES) {
        return Fail("mesh layout topology metadata was not parsed");
    }

    RuntimeAssetValidationResult material_result{};
    const std::string material_text = SourceMaterialText();
    if (!ValidateText(material_text, RuntimeAssetFileKind::Material, &material_result)) {
        return Fail("material metadata validator rejected valid material");
    }

    if (material_result.dependency_count != 4U || material_result.texture_slot_count != 3U) {
        return Fail("material dependency metadata was not parsed");
    }

    if (material_result.material_parameter_count != MATERIAL_PARAMETER_COUNT ||
        material_result.material_base_color_rgba != MATERIAL_BASE_COLOR_RGBA ||
        material_result.material_emissive_strength != MATERIAL_EMISSIVE_STRENGTH ||
        material_result.material_metallic != MATERIAL_METALLIC ||
        material_result.material_roughness != MATERIAL_ROUGHNESS ||
        material_result.material_opacity != MATERIAL_OPACITY ||
        material_result.material_alpha_mode != yuengine::runtimeasset::RuntimeAssetMaterialAlphaMode::Blend) {
        return Fail("material parameter metadata was not parsed");
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

bool ExpectLoadedMeshLayout(
    const RuntimeAssetLoadedFile &mesh,
    yuengine::runtimeasset::RuntimeAssetMeshGeometryKind expected_geometry_kind,
    std::uint32_t expected_vertex_count,
    std::uint32_t expected_index_count,
    std::uint32_t expected_payload_byte_count) {
    if (mesh.kind != RuntimeAssetFileKind::Mesh ||
        mesh.mesh_geometry_kind != expected_geometry_kind ||
        mesh.vertex_count != expected_vertex_count ||
        mesh.index_count != expected_index_count ||
        mesh.payload_byte_count != expected_payload_byte_count ||
        mesh.decoded_byte_count != expected_payload_byte_count) {
        return FailStep("loaded mesh record did not preserve identity counts or payload size");
    }

    if (mesh.mesh_input_layout.element_count != 2U ||
        mesh.mesh_input_layout.stride_bytes != MESH_VERTEX_STRIDE_BYTES ||
        mesh.mesh_input_layout.elements[0U].semantic != RhiInputElementSemantic::Position ||
        mesh.mesh_input_layout.elements[0U].format != RhiInputElementFormat::Float32x2 ||
        mesh.mesh_input_layout.elements[0U].offset_bytes != 0U ||
        mesh.mesh_input_layout.elements[1U].semantic != RhiInputElementSemantic::TexCoord ||
        mesh.mesh_input_layout.elements[1U].format != RhiInputElementFormat::Float32x2 ||
        mesh.mesh_input_layout.elements[1U].offset_bytes != MESH_TEXCOORD_OFFSET_BYTES) {
        return FailStep("loaded mesh record did not preserve input layout");
    }

    if (mesh.mesh_index_format != RhiIndexFormat::Uint16 ||
        mesh.mesh_topology != RhiPrimitiveTopology::TriangleList ||
        mesh.mesh_vertex_stride_bytes != MESH_VERTEX_STRIDE_BYTES ||
        mesh.mesh_index_stride_bytes != MESH_INDEX_STRIDE_BYTES) {
        return FailStep("loaded mesh record did not preserve index format or topology");
    }

    return true;
}

int RuntimeAssetDataMeshLayoutTopologyDecodesIntoLoadedRecords() {
    constexpr std::uint32_t cube_payload_byte_count =
        MeshPayloadByteCount(24U, 36U);
    constexpr std::uint32_t cylinder_payload_byte_count =
        MeshPayloadByteCount(18U, 96U);
    constexpr std::uint32_t cone_payload_byte_count =
        MeshPayloadByteCount(10U, 48U);
    MountTable table;
    if (!CreateMountedTable(TestRoot("MeshLayoutTopologyLoadedRecords"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("canonical fixture write failed");
    }

    LoadedGraph graph{};
    if (!LoadGraph(table, &graph)) {
        return Fail("canonical graph load failed");
    }

    if (!ExpectLoadedMeshLayout(
            graph.assets[0U],
            yuengine::runtimeasset::RuntimeAssetMeshGeometryKind::Cube,
            24U,
            36U,
            cube_payload_byte_count)) {
        return Fail("cube mesh layout topology record mismatch");
    }

    if (!ExpectLoadedMeshLayout(
            graph.assets[1U],
            yuengine::runtimeasset::RuntimeAssetMeshGeometryKind::Cylinder,
            18U,
            96U,
            cylinder_payload_byte_count)) {
        return Fail("cylinder mesh layout topology record mismatch");
    }

    if (!ExpectLoadedMeshLayout(
            graph.assets[2U],
            yuengine::runtimeasset::RuntimeAssetMeshGeometryKind::Cone,
            10U,
            48U,
            cone_payload_byte_count)) {
        return Fail("cone mesh layout topology record mismatch");
    }

    return 0;
}

int RuntimeAssetDataMeshPayloadPolicyRejectsSizeHashAndSplitMismatch() {
    constexpr std::uint32_t vertex_count = 24U;
    constexpr std::uint32_t index_count = 36U;
    constexpr std::uint32_t vertex_payload_byte_count =
        MeshVertexPayloadByteCount(vertex_count);
    constexpr std::uint32_t index_payload_byte_count =
        MeshIndexPayloadByteCount(index_count);
    constexpr std::uint32_t payload_byte_count =
        MeshPayloadByteCount(vertex_count, index_count);
    const std::string payload = MeshPayload('A', payload_byte_count);
    const std::string valid_text =
        SourceMeshText(
            "YUASSET MESH 1",
            "cube_mesh",
            "cube",
            vertex_count,
            index_count,
            vertex_payload_byte_count,
            index_payload_byte_count,
            payload);

    const std::string payload_size_token =
        "payloadBytes=" + std::to_string(payload_byte_count);
    const std::string bad_payload_size =
        ReplaceFirst(valid_text, payload_size_token, "payloadBytes=95");
    if (!ExpectValidationStatus(
            bad_payload_size,
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::InvalidSize)) {
        return Fail("mesh payload size mismatch was not rejected");
    }

    const std::string hash_token = "payloadHash=" + std::to_string(HashText(payload));
    const std::string bad_payload_hash = ReplaceFirst(valid_text, hash_token, "payloadHash=1");
    if (!ExpectValidationStatus(
            bad_payload_hash,
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::HashMismatch)) {
        return Fail("mesh payload hash mismatch was not rejected");
    }

    const std::string index_payload_token =
        "indexPayloadBytes=" + std::to_string(index_payload_byte_count);
    const std::string bad_split_sum =
        ReplaceFirst(valid_text, index_payload_token, "indexPayloadBytes=47");
    if (!ExpectValidationStatus(
            bad_split_sum,
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::InvalidSize)) {
        return Fail("mesh payload split sum mismatch was not rejected");
    }

    const std::string vertex_payload_token =
        "vertexPayloadBytes=" + std::to_string(vertex_payload_byte_count);
    const std::string bad_zero_vertex_split =
        ReplaceFirst(valid_text, vertex_payload_token, "vertexPayloadBytes=0");
    if (!ExpectValidationStatus(
            bad_zero_vertex_split,
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::InvalidSize)) {
        return Fail("mesh zero vertex payload split was not rejected");
    }

    const std::string bad_layout =
        ReplaceFirst(valid_text, "input=layout:position,texcoord", "input=layout:color,texcoord");
    if (!ExpectValidationStatus(
            bad_layout,
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::InvalidInputLayout)) {
        return Fail("mesh input layout without position was not rejected");
    }

    const std::string bad_vertex_stride =
        ReplaceFirst(valid_text, "vertexStrideBytes=16", "vertexStrideBytes=12");
    if (!ExpectValidationStatus(
            bad_vertex_stride,
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::InvalidInputLayout)) {
        return Fail("mesh vertex stride mismatch was not rejected");
    }

    const std::string bad_index_format =
        ReplaceFirst(valid_text, "indexFormat=uint16", "indexFormat=uint8");
    if (!ExpectValidationStatus(
            bad_index_format,
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::UnsupportedFieldValue)) {
        return Fail("mesh unsupported index format was not rejected");
    }

    const std::string bad_topology =
        ReplaceFirst(valid_text, "topology=triangle_list", "topology=line_list");
    if (!ExpectValidationStatus(
            bad_topology,
            RuntimeAssetFileKind::Mesh,
            RuntimeAssetDataStatus::UnsupportedFieldValue)) {
        return Fail("mesh unsupported topology was not rejected");
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

int RuntimeAssetDataMaterialParameterSemanticsLoadIntoRuntimeRecords() {
    const std::string valid_text = SourceMaterialText();
    const std::string bad_count = ReplaceFirst(valid_text, "parameterCount=5", "parameterCount=4");
    if (!ExpectValidationStatus(
            bad_count,
            RuntimeAssetFileKind::Material,
            RuntimeAssetDataStatus::InvalidCount)) {
        return Fail("material parameter count mismatch was not rejected");
    }

    const std::string bad_color = ReplaceFirst(valid_text, "baseColorRgba=32,48,64,192", "baseColorRgba=32,48,64");
    if (!ExpectValidationStatus(
            bad_color,
            RuntimeAssetFileKind::Material,
            RuntimeAssetDataStatus::InvalidBounds)) {
        return Fail("material color component mismatch was not rejected");
    }

    const std::string bad_opacity = ReplaceFirst(valid_text, "opacity=192", "opacity=300");
    if (!ExpectValidationStatus(
            bad_opacity,
            RuntimeAssetFileKind::Material,
            RuntimeAssetDataStatus::InvalidBounds)) {
        return Fail("material opacity range mismatch was not rejected");
    }

    const std::string bad_alpha_mode = ReplaceFirst(valid_text, "alphaMode=blend", "alphaMode=mask");
    if (!ExpectValidationStatus(
            bad_alpha_mode,
            RuntimeAssetFileKind::Material,
            RuntimeAssetDataStatus::UnsupportedFieldValue)) {
        return Fail("material alpha mode mismatch was not rejected");
    }

    MountTable table;
    if (!CreateMountedTable(TestRoot("MaterialParameters"), &table)) {
        return Fail("material parameter mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("material parameter fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("material parameter graph load failed");
    }

    const RuntimeAssetLoadedFile &material = graph.assets[3U];
    if (material.kind != RuntimeAssetFileKind::Material ||
        material.texture_slot_count != 3U ||
        material.material_parameter_count != MATERIAL_PARAMETER_COUNT ||
        material.material_base_color_rgba != MATERIAL_BASE_COLOR_RGBA ||
        material.material_emissive_strength != MATERIAL_EMISSIVE_STRENGTH ||
        material.material_metallic != MATERIAL_METALLIC ||
        material.material_roughness != MATERIAL_ROUGHNESS ||
        material.material_opacity != MATERIAL_OPACITY ||
        material.material_alpha_mode != yuengine::runtimeasset::RuntimeAssetMaterialAlphaMode::Blend) {
        return Fail("material parameters did not reach loaded runtime record");
    }

    return 0;
}

int RuntimeAssetDataMaterialConstantsPackLoadedParameters() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("MaterialConstantPack"), &table)) {
        return Fail("material constant pack mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("material constant pack fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("material constant pack graph load failed");
    }

    RuntimeAssetPackedMaterialConstants constants{};
    const RuntimeAssetDataStatus status =
        PackRuntimeAssetMaterialConstants(graph.assets[3U], &constants);
    if (status != RuntimeAssetDataStatus::Success) {
        return Fail("material constant pack failed");
    }

    if (!ExpectPackedMaterialConstants(constants)) {
        return Fail("material constant pack did not preserve expected bytes");
    }

    RuntimeAssetPackedMaterialConstants bad_output{};
    RuntimeAssetLoadedFile bad_material = graph.assets[3U];
    bad_material.material_parameter_count = MATERIAL_PARAMETER_COUNT + 1U;
    const RuntimeAssetDataStatus bad_status =
        PackRuntimeAssetMaterialConstants(bad_material, &bad_output);
    if (bad_status != RuntimeAssetDataStatus::InvalidCount) {
        return Fail("material constant pack accepted invalid parameter count");
    }

    if (bad_output.byte_count != 0U || bad_output.hash != 0U) {
        return Fail("material constant pack failure mutated output constants");
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

int RuntimeAssetDataShaderImportPolicyValidatesSourceCookedAndLoadedRecords() {
    const std::string valid_source = SourceShaderText();
    RuntimeAssetValidationResult source_result{};
    if (!ValidateText(valid_source, RuntimeAssetFileKind::Shader, &source_result)) {
        return Fail("shader import policy rejected canonical source shader");
    }

    const std::uint64_t policy_hash = HashText(ShaderImportPolicyFields());
    if (source_result.shader_import_policy_count != SHADER_IMPORT_POLICY_FIELD_COUNT ||
        source_result.shader_import_policy_hash != policy_hash ||
        source_result.shader_stage_count != 2U ||
        source_result.texture_slot_count != 3U) {
        return Fail("shader import policy metadata was not parsed");
    }

    const std::string missing_policy = ReplaceFirst(valid_source, ShaderImportPolicyFields(), "");
    if (!ExpectValidationStatus(
            missing_policy,
            RuntimeAssetFileKind::Shader,
            RuntimeAssetDataStatus::UnsupportedFieldValue)) {
        return Fail("missing shader import policy was not rejected");
    }

    const std::string bad_language = ReplaceFirst(valid_source, "importLanguage=hlsl", "importLanguage=glsl");
    if (!ExpectValidationStatus(
            bad_language,
            RuntimeAssetFileKind::Shader,
            RuntimeAssetDataStatus::UnsupportedFieldValue)) {
        return Fail("unsupported shader language was not rejected");
    }

    const std::string bad_target = ReplaceFirst(valid_source, "importTarget=d3d11", "importTarget=vulkan");
    if (!ExpectValidationStatus(
            bad_target,
            RuntimeAssetFileKind::Shader,
            RuntimeAssetDataStatus::UnsupportedFieldValue)) {
        return Fail("unsupported shader target was not rejected");
    }

    const std::string bad_entry = ReplaceFirst(valid_source, "entry_vs=VSMain", "entry_vs=WrongMain");
    if (!ExpectValidationStatus(
            bad_entry,
            RuntimeAssetFileKind::Shader,
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("shader vertex entry mismatch was not rejected");
    }

    const std::string bad_profile = ReplaceFirst(valid_source, "profile_ps=ps_5_0", "profile_ps=ps_4_0");
    if (!ExpectValidationStatus(
            bad_profile,
            RuntimeAssetFileKind::Shader,
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("shader pixel profile mismatch was not rejected");
    }

    const std::string bad_flags = ReplaceFirst(valid_source, "compileFlags=deterministic", "compileFlags=debug");
    if (!ExpectValidationStatus(
            bad_flags,
            RuntimeAssetFileKind::Shader,
            RuntimeAssetDataStatus::UnsupportedFieldValue)) {
        return Fail("shader compile flags mismatch was not rejected");
    }

    MountTable table;
    if (!CreateMountedTable(TestRoot("ShaderImportPolicySourceLoad"), &table)) {
        return Fail("shader import policy source mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("shader import policy source fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("shader import policy graph load failed");
    }

    const RuntimeAssetLoadedFile &source_shader = graph.assets[7U];
    if (source_shader.kind != RuntimeAssetFileKind::Shader ||
        source_shader.shader_import_policy_count != SHADER_IMPORT_POLICY_FIELD_COUNT ||
        source_shader.shader_import_policy_hash != policy_hash) {
        return Fail("shader import policy did not reach loaded source record");
    }

    GeneratedFixtureCommandContext context{};
    if (!ExecuteGeneratedFixtureCommand("ShaderImportPolicyCookedLoad", &context)) {
        return Fail("shader import policy cooked fixture generation failed");
    }

    const RuntimeAssetFileDesc *shader_desc = nullptr;
    for (const RuntimeAssetFileDesc &desc : context.cooked_files) {
        if (desc.kind == RuntimeAssetFileKind::Shader) {
            shader_desc = &desc;
            break;
        }
    }

    if (shader_desc == nullptr) {
        return Fail("shader import policy cooked fixture did not include shader");
    }

    std::vector<std::uint8_t> cooked_bytes{};
    if (!ReadFile(context.table, shader_desc->path, &cooked_bytes)) {
        return Fail("shader import policy cooked shader read failed");
    }

    RuntimeAssetValidationResult cooked_result{};
    const RuntimeAssetDataStatus cooked_status = ValidateRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(cooked_bytes.data(), cooked_bytes.size()),
        RuntimeAssetFileKind::Shader,
        &cooked_result);
    if (cooked_status != RuntimeAssetDataStatus::Success ||
        cooked_result.artifact_class != RuntimeAssetArtifactClass::Cooked ||
        cooked_result.shader_import_policy_count != SHADER_IMPORT_POLICY_FIELD_COUNT ||
        cooked_result.shader_import_policy_hash != policy_hash) {
        return Fail("shader import policy did not validate generated cooked shader");
    }

    return 0;
}

RuntimeAssetShaderProgramPipelineRequest ProgramPipelineRequest(
    RuntimeAssetRhiDevice *device,
    const RuntimeAssetLoadedShaderProgramData *program) {
    RuntimeAssetShaderProgramPipelineRequest request{};
    request.device = device;
    request.program = program;
    return request;
}

int RuntimeAssetDataShaderCompilerBackendProducesProgramReflection() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("ShaderCompilerBackend"), &table)) {
        return Fail("shader compiler backend mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("shader compiler backend fixture write failed");
    }

    std::vector<std::uint8_t> source_bytes{};
    if (!ReadFile(table, "Shader/RuntimeProgram.yuprogram", &source_bytes)) {
        return Fail("shader compiler backend source read failed");
    }

    RuntimeAssetShaderCompilerBackendRequest compile_request{};
    compile_request.backend_kind = RuntimeAssetShaderCompilerBackendKind::DeterministicFixture;
    compile_request.source_bytes = std::span<const std::uint8_t>(
        source_bytes.data(),
        source_bytes.size());
    compile_request.program_id = 4001U;
    compile_request.expected_import_policy_hash = HashText(ShaderImportPolicyFields());

    RuntimeAssetShaderCompilerBackendResult compile_result{};
    const RuntimeAssetDataStatus compile_status =
        CompileRuntimeAssetShaderProgram(compile_request, &compile_result);
    if (compile_status != RuntimeAssetDataStatus::Success ||
        compile_result.status != RuntimeAssetDataStatus::Success ||
        !compile_result.compiled_program ||
        compile_result.program.status != RuntimeAssetDataStatus::Success) {
        return Fail("shader compiler backend rejected canonical source shader");
    }

    if (compile_result.import_policy_hash != compile_request.expected_import_policy_hash ||
        compile_result.compiled_shader_stage_count != 2U ||
        compile_result.reflection_input_element_count != 2U ||
        compile_result.reflection_texture_slot_count != 3U ||
        compile_result.vertex_bytecode_hash == 0U ||
        compile_result.pixel_bytecode_hash == 0U ||
        compile_result.vertex_bytecode_hash == compile_result.pixel_bytecode_hash) {
        return Fail("shader compiler backend did not report policy, stage, or reflection identity");
    }

    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("shader compiler backend rhi init failed");
    }

    const RuntimeAssetShaderProgramPipelineRequest pipeline_request =
        ProgramPipelineRequest(&device, &compile_result.program);
    RuntimeAssetShaderProgramPipelineResult pipeline_result{};
    const RuntimeAssetDataStatus pipeline_status =
        BuildRuntimeAssetShaderProgramPipeline(pipeline_request, &pipeline_result);
    if (pipeline_status != RuntimeAssetDataStatus::Success ||
        pipeline_result.pipeline.generation == 0U ||
        pipeline_result.pipeline_desc.input_layout.element_count != 2U ||
        pipeline_result.texture_slot_count != 3U) {
        return Fail("shader compiler backend output did not feed RHI pipeline bridge");
    }

    RuntimeAssetShaderCompilerBackendRequest unknown_backend_request = compile_request;
    unknown_backend_request.backend_kind = RuntimeAssetShaderCompilerBackendKind::Unknown;
    RuntimeAssetShaderCompilerBackendResult unknown_backend_result{};
    const RuntimeAssetDataStatus unknown_backend_status =
        CompileRuntimeAssetShaderProgram(unknown_backend_request, &unknown_backend_result);
    if (unknown_backend_status != RuntimeAssetDataStatus::UnsupportedFieldValue ||
        unknown_backend_result.compiled_program) {
        return Fail("shader compiler backend accepted unknown backend kind");
    }

    RuntimeAssetShaderCompilerBackendRequest policy_mismatch_request = compile_request;
    policy_mismatch_request.expected_import_policy_hash = 1U;
    RuntimeAssetShaderCompilerBackendResult policy_mismatch_result{};
    const RuntimeAssetDataStatus policy_mismatch_status =
        CompileRuntimeAssetShaderProgram(policy_mismatch_request, &policy_mismatch_result);
    if (policy_mismatch_status != RuntimeAssetDataStatus::HashMismatch ||
        policy_mismatch_result.compiled_program ||
        policy_mismatch_result.program.status != RuntimeAssetDataStatus::HashMismatch) {
        return Fail("shader compiler backend accepted import policy hash mismatch");
    }

    return 0;
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
            SourceShaderTextWithBody(
            "stage_vs=Texture/Albedo.yutex\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:position,color\n"
            "textures=3\n"),
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("runtime asset shader bridge accepted invalid stage refs");
    }

    if (!ExpectShaderBridgeRejectedWithoutRhiMutation(
            SourceShaderTextWithBody(
            "stage_vs=bytecode:\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:position,color\n"
            "textures=3\n"),
            RuntimeAssetDataStatus::InvalidSize)) {
        return Fail("runtime asset shader bridge accepted missing bytecode");
    }

    if (!ExpectShaderBridgeRejectedWithoutRhiMutation(
            SourceShaderTextWithBody(
            "stage_vs=bytecode:runtime_program_vs\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "stage_vs_hash=1\n"
            "input=layout:position,color\n"
            "textures=3\n"),
            RuntimeAssetDataStatus::HashMismatch)) {
        return Fail("runtime asset shader bridge accepted hash mismatch");
    }

    if (!ExpectShaderBridgeRejectedWithoutRhiMutation(
            SourceShaderTextWithBody(
            "stage_vs=bytecode:runtime_program_vs\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:color\n"
            "textures=3\n"),
            RuntimeAssetDataStatus::InvalidInputLayout)) {
        return Fail("runtime asset shader bridge accepted input-layout mismatch");
    }

    if (!ExpectShaderBridgeRejectedWithoutRhiMutation(
            SourceShaderTextWithBody(
            "stage_vs=bytecode:runtime_program_vs\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:position,normal\n"
            "textures=3\n"),
            RuntimeAssetDataStatus::UnsupportedFieldValue)) {
        return Fail("runtime asset shader bridge accepted unsupported semantic");
    }

    if (!ExpectShaderBridgeRejectedWithoutRhiMutation(
            SourceShaderTextWithBody(
            "stage_vs=bytecode:runtime_program_vs\n"
            "stage_ps=bytecode:runtime_program_ps\n"
            "input=layout:position,color,texcoord\n"
            "textures=3\n"),
            RuntimeAssetDataStatus::CapacityExceeded)) {
        return Fail("runtime asset shader bridge accepted layout capacity overflow");
    }

    return 0;
}

template <std::size_t Size>
RuntimeAssetCookedShaderStagePayloadDesc CookedShaderStagePayload(
    RhiShaderStage stage,
    const std::array<std::uint8_t, Size> &bytes) {
    RuntimeAssetCookedShaderStagePayloadDesc desc{};
    desc.stage = stage;
    desc.entry_point = stage == RhiShaderStage::Vertex ? "VSMain" : "PSMain";
    desc.bytecode_profile = stage == RhiShaderStage::Vertex ? "vs_5_0" : "ps_5_0";
    desc.bytecode_format = RuntimeAssetCookedShaderBytecodeFormat::OpaqueBytecode;
    desc.payload_id = stage == RhiShaderStage::Vertex ? 501U : 502U;
    desc.payload_bytes = bytes.data();
    desc.payload_byte_count = static_cast<std::uint32_t>(bytes.size());
    desc.bytecode_offset = 0U;
    desc.bytecode_byte_count = static_cast<std::uint32_t>(bytes.size());
    desc.bytecode_alignment = 4U;
    const std::uint64_t hash = HashRuntimeAssetDataBytes(
        std::span<const std::uint8_t>(bytes.data(), bytes.size()));
    desc.bytecode_hash = hash;
    desc.expected_stage_hash = hash;
    return desc;
}

std::array<RuntimeAssetCookedShaderStagePayloadDesc, 2U> ValidCookedShaderStages(
    const std::array<std::uint8_t, 8U> &vertex_bytes,
    const std::array<std::uint8_t, 8U> &pixel_bytes) {
    return {
        CookedShaderStagePayload(RhiShaderStage::Vertex, vertex_bytes),
        CookedShaderStagePayload(RhiShaderStage::Pixel, pixel_bytes)};
}

RuntimeAssetCookedProgramDesc CookedProgramDescriptor(
    std::span<const RuntimeAssetCookedShaderStagePayloadDesc> stages) {
    RuntimeAssetCookedProgramDesc program{};
    program.program_id = 7001U;
    program.pipeline_class = RuntimeAssetCookedProgramPipelineClass::Graphics;
    program.stages = stages.data();
    program.stage_count = static_cast<std::uint32_t>(stages.size());
    program.input_layout = RuntimeInputLayout();
    program.vertex_stride_bytes = static_cast<std::uint32_t>(program.input_layout.stride_bytes);
    program.texture_slot_count = 2U;
    program.sampler_slot_count = 2U;
    program.required_input_semantics[0U] = RhiInputElementSemantic::Position;
    program.required_input_semantic_count = 1U;
    program.constant_range_count = 1U;
    return program;
}

RuntimeAssetCookedShaderProgramPipelineRequest CookedProgramPipelineRequest(
    RuntimeAssetRhiDevice *device,
    const RuntimeAssetCookedProgramDesc *program) {
    RuntimeAssetCookedShaderProgramPipelineRequest request{};
    request.device = device;
    request.program = program;
    return request;
}

bool BuildCookedShaderProgram(
    RuntimeAssetRhiDevice &device,
    const RuntimeAssetCookedProgramDesc &program,
    RuntimeAssetCookedShaderProgramPipelineResult *out_result) {
    if (out_result == nullptr) {
        return false;
    }

    const RuntimeAssetCookedShaderProgramPipelineRequest request =
        CookedProgramPipelineRequest(&device, &program);
    const RuntimeAssetDataStatus status =
        BuildRuntimeAssetCookedShaderProgramPipeline(request, out_result);
    return status == out_result->status;
}

int RuntimeAssetDataCookedShaderStagePayloadsCreateRhiModules() {
    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("rhi init failed");
    }

    const std::array<std::uint8_t, 8U> vertex_bytes{0x56U, 0x53U, 0x10U, 0x01U, 0x22U, 0x33U, 0x44U, 0x55U};
    const std::array<std::uint8_t, 8U> pixel_bytes{0x50U, 0x53U, 0x20U, 0x02U, 0x66U, 0x77U, 0x88U, 0x99U};
    const std::array<RuntimeAssetCookedShaderStagePayloadDesc, 2U> stages =
        ValidCookedShaderStages(vertex_bytes, pixel_bytes);
    const RuntimeAssetCookedProgramDesc program =
        CookedProgramDescriptor(std::span<const RuntimeAssetCookedShaderStagePayloadDesc>(stages.data(), stages.size()));

    RuntimeAssetCookedShaderProgramPipelineResult result{};
    if (!BuildCookedShaderProgram(device, program, &result) ||
        result.status != RuntimeAssetDataStatus::Success) {
        return Fail("cooked shader bridge rejected valid stage payloads");
    }

    if (!result.published_handles ||
        result.vertex_shader.generation == 0U ||
        result.pixel_shader.generation == 0U ||
        result.pipeline.generation == 0U) {
        return Fail("cooked shader bridge did not publish complete RHI handles");
    }

    if (result.created_shader_module_count != 2U || result.destroyed_shader_module_count != 0U) {
        return Fail("cooked shader bridge reported wrong creation ledger");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.shader_module_count != 2U || snapshot.resources.pipeline_count != 1U) {
        return Fail("cooked shader bridge did not create RHI module/pipeline ownership");
    }

    return 0;
}

int RuntimeAssetDataCookedProgramPipelineUsesLoadedReflectionAndInputLayout() {
    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("rhi init failed");
    }

    const std::array<std::uint8_t, 8U> vertex_bytes{0x31U, 0x32U, 0x33U, 0x34U, 0x35U, 0x36U, 0x37U, 0x38U};
    const std::array<std::uint8_t, 8U> pixel_bytes{0x41U, 0x42U, 0x43U, 0x44U, 0x45U, 0x46U, 0x47U, 0x48U};
    const std::array<RuntimeAssetCookedShaderStagePayloadDesc, 2U> stages =
        ValidCookedShaderStages(vertex_bytes, pixel_bytes);
    RuntimeAssetCookedProgramDesc program =
        CookedProgramDescriptor(std::span<const RuntimeAssetCookedShaderStagePayloadDesc>(stages.data(), stages.size()));
    program.input_layout.elements[1U].semantic = RhiInputElementSemantic::Color;
    program.input_layout.elements[1U].format = RhiInputElementFormat::Float32x4;
    program.input_layout.elements[1U].offset_bytes = sizeof(float) * 2U;
    program.input_layout.element_count = 2U;
    program.input_layout.stride_bytes = sizeof(float) * 6U;
    program.vertex_stride_bytes = static_cast<std::uint32_t>(program.input_layout.stride_bytes);
    program.texture_slot_count = 3U;
    program.sampler_slot_count = 2U;
    program.required_input_semantics[0U] = RhiInputElementSemantic::Position;
    program.required_input_semantics[1U] = RhiInputElementSemantic::Color;
    program.required_input_semantic_count = 2U;

    RuntimeAssetCookedShaderProgramPipelineResult result{};
    if (!BuildCookedShaderProgram(device, program, &result) ||
        result.status != RuntimeAssetDataStatus::Success) {
        return Fail("cooked program bridge rejected loaded reflection");
    }

    if (result.pipeline_desc.input_layout.element_count != 2U ||
        result.pipeline_desc.input_layout.stride_bytes != sizeof(float) * 6U ||
        result.texture_slot_count != 3U ||
        result.sampler_slot_count != 2U) {
        return Fail("cooked program bridge did not preserve reflection/input layout/slot counts");
    }

    if (result.vertex_bytecode_hash != stages[0U].bytecode_hash ||
        result.pixel_bytecode_hash != stages[1U].bytecode_hash) {
        return Fail("cooked program bridge did not report loaded stage hashes");
    }

    return 0;
}

bool ExpectCookedShaderBridgeRejectedWithoutRhiMutation(
    const RuntimeAssetCookedProgramDesc &program,
    RuntimeAssetDataStatus expected_status) {
    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return FailStep("rhi init failed");
    }

    const auto before = device.Snapshot();
    RuntimeAssetCookedShaderProgramPipelineResult result{};
    if (!BuildCookedShaderProgram(device, program, &result) || result.status != expected_status) {
        return FailStep("cooked shader bridge returned unexpected rejection status");
    }

    const auto after = device.Snapshot();
    if (before.resources.shader_module_count != after.resources.shader_module_count ||
        before.resources.pipeline_count != after.resources.pipeline_count ||
        before.resources.created_primitive_count != after.resources.created_primitive_count ||
        before.failed_operation_count != after.failed_operation_count) {
        return FailStep("cooked shader bridge mutated RHI during preflight rejection");
    }

    if (result.published_handles ||
        result.vertex_shader.generation != 0U ||
        result.pixel_shader.generation != 0U ||
        result.pipeline.generation != 0U) {
        return FailStep("cooked shader bridge published handles on preflight rejection");
    }

    return true;
}

int RuntimeAssetDataCookedShaderPayloadRejectsStageBytecodeHashAndReflectionMismatchWithoutMutation() {
    const std::array<std::uint8_t, 8U> vertex_bytes{0x11U, 0x12U, 0x13U, 0x14U, 0x15U, 0x16U, 0x17U, 0x18U};
    const std::array<std::uint8_t, 8U> pixel_bytes{0x21U, 0x22U, 0x23U, 0x24U, 0x25U, 0x26U, 0x27U, 0x28U};

    {
        std::array<RuntimeAssetCookedShaderStagePayloadDesc, 2U> stages =
            ValidCookedShaderStages(vertex_bytes, pixel_bytes);
        stages[0U].bytecode_profile = "ps_5_0";
        const RuntimeAssetCookedProgramDesc program =
            CookedProgramDescriptor(std::span<const RuntimeAssetCookedShaderStagePayloadDesc>(stages.data(), stages.size()));
        if (!ExpectCookedShaderBridgeRejectedWithoutRhiMutation(program, RuntimeAssetDataStatus::TypeMismatch)) {
            return Fail("cooked shader bridge accepted stage/profile mismatch");
        }
    }

    {
        std::array<RuntimeAssetCookedShaderStagePayloadDesc, 2U> stages =
            ValidCookedShaderStages(vertex_bytes, pixel_bytes);
        stages[0U].bytecode_byte_count = 0U;
        const RuntimeAssetCookedProgramDesc program =
            CookedProgramDescriptor(std::span<const RuntimeAssetCookedShaderStagePayloadDesc>(stages.data(), stages.size()));
        if (!ExpectCookedShaderBridgeRejectedWithoutRhiMutation(program, RuntimeAssetDataStatus::InvalidSize)) {
            return Fail("cooked shader bridge accepted missing bytecode");
        }
    }

    {
        std::array<RuntimeAssetCookedShaderStagePayloadDesc, 2U> stages =
            ValidCookedShaderStages(vertex_bytes, pixel_bytes);
        stages[1U].bytecode_hash = 1U;
        const RuntimeAssetCookedProgramDesc program =
            CookedProgramDescriptor(std::span<const RuntimeAssetCookedShaderStagePayloadDesc>(stages.data(), stages.size()));
        if (!ExpectCookedShaderBridgeRejectedWithoutRhiMutation(program, RuntimeAssetDataStatus::HashMismatch)) {
            return Fail("cooked shader bridge accepted bytecode hash mismatch");
        }
    }

    {
        std::array<RuntimeAssetCookedShaderStagePayloadDesc, 2U> stages =
            ValidCookedShaderStages(vertex_bytes, pixel_bytes);
        RuntimeAssetCookedProgramDesc program =
            CookedProgramDescriptor(std::span<const RuntimeAssetCookedShaderStagePayloadDesc>(stages.data(), stages.size()));
        program.input_layout.elements[0U].semantic = RhiInputElementSemantic::Unsupported;
        if (!ExpectCookedShaderBridgeRejectedWithoutRhiMutation(program, RuntimeAssetDataStatus::UnsupportedFieldValue)) {
            return Fail("cooked shader bridge accepted unsupported input semantic");
        }
    }

    {
        std::array<RuntimeAssetCookedShaderStagePayloadDesc, 2U> stages =
            ValidCookedShaderStages(vertex_bytes, pixel_bytes);
        RuntimeAssetCookedProgramDesc program =
            CookedProgramDescriptor(std::span<const RuntimeAssetCookedShaderStagePayloadDesc>(stages.data(), stages.size()));
        program.required_input_semantics[1U] = RhiInputElementSemantic::Color;
        program.required_input_semantic_count = 2U;
        if (!ExpectCookedShaderBridgeRejectedWithoutRhiMutation(program, RuntimeAssetDataStatus::InvalidInputLayout)) {
            return Fail("cooked shader bridge accepted reflection/input-layout mismatch");
        }
    }

    {
        std::array<RuntimeAssetCookedShaderStagePayloadDesc, 2U> stages =
            ValidCookedShaderStages(vertex_bytes, pixel_bytes);
        RuntimeAssetCookedProgramDesc program =
            CookedProgramDescriptor(std::span<const RuntimeAssetCookedShaderStagePayloadDesc>(stages.data(), stages.size()));
        program.texture_slot_count = static_cast<std::uint32_t>(yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS + 1U);
        if (!ExpectCookedShaderBridgeRejectedWithoutRhiMutation(program, RuntimeAssetDataStatus::CapacityExceeded)) {
            return Fail("cooked shader bridge accepted slot overflow");
        }
    }

    return 0;
}

bool CreatePipelineWithShaderHandles(
    IRhiDevice &device,
    RhiShaderModuleHandle *out_vertex_shader,
    RhiShaderModuleHandle *out_pixel_shader,
    RhiPipelineHandle *out_pipeline) {
    if (out_vertex_shader == nullptr || out_pixel_shader == nullptr || out_pipeline == nullptr) {
        return false;
    }

    if (!CreateShaderModule(device, RhiShaderStage::Vertex, out_vertex_shader)) {
        return false;
    }

    if (!CreateShaderModule(device, RhiShaderStage::Pixel, out_pixel_shader)) {
        return false;
    }

    RhiPipelineDesc desc{};
    desc.vertex_shader = *out_vertex_shader;
    desc.pixel_shader = *out_pixel_shader;
    desc.input_layout = RuntimeInputLayout();
    return device.CreatePipeline(desc, *out_pipeline) == RhiStatus::Success;
}

int RuntimeAssetDataCookedShaderProgramRhiPartialCreationFailureDestroysTransientHandles() {
    const std::array<std::uint8_t, 8U> vertex_bytes{0x61U, 0x62U, 0x63U, 0x64U, 0x65U, 0x66U, 0x67U, 0x68U};
    const std::array<std::uint8_t, 8U> pixel_bytes{0x71U, 0x72U, 0x73U, 0x74U, 0x75U, 0x76U, 0x77U, 0x78U};

    {
        RuntimeAssetRhiDevice device;
        if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
            return Fail("rhi init failed");
        }

        std::array<RhiShaderModuleHandle, yuengine::rhi::MAX_RHI_SHADER_MODULES - 1U> existing_modules{};
        for (std::size_t index = 0U; index < existing_modules.size(); ++index) {
            const RhiShaderStage stage = (index % 2U) == 0U ? RhiShaderStage::Vertex : RhiShaderStage::Pixel;
            if (!CreateShaderModule(device, stage, &existing_modules[index])) {
                return Fail("failed to reserve shader module capacity");
            }
        }

        const auto before = device.Snapshot();
        std::array<RuntimeAssetCookedShaderStagePayloadDesc, 2U> stages =
            ValidCookedShaderStages(vertex_bytes, pixel_bytes);
        const RuntimeAssetCookedProgramDesc program =
            CookedProgramDescriptor(std::span<const RuntimeAssetCookedShaderStagePayloadDesc>(stages.data(), stages.size()));
        RuntimeAssetCookedShaderProgramPipelineResult result{};
        if (!BuildCookedShaderProgram(device, program, &result) ||
            result.status != RuntimeAssetDataStatus::RhiShaderModuleFailed) {
            return Fail("cooked shader bridge did not report pixel module failure");
        }

        const auto after = device.Snapshot();
        if (after.resources.shader_module_count != before.resources.shader_module_count ||
            after.resources.pipeline_count != before.resources.pipeline_count ||
            after.failed_operation_count != before.failed_operation_count + 1U) {
            return Fail("cooked shader bridge did not cleanup transient vertex module");
        }

        if (result.created_shader_module_count != 1U ||
            result.destroyed_shader_module_count != 1U ||
            result.published_handles ||
            result.vertex_shader.generation != 0U ||
            result.pixel_shader.generation != 0U ||
            result.pipeline.generation != 0U) {
            return Fail("cooked shader bridge published handles after module failure");
        }
    }

    {
        RuntimeAssetRhiDevice device;
        if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
            return Fail("rhi init failed");
        }

        std::array<RhiShaderModuleHandle, yuengine::rhi::MAX_RHI_PIPELINES> vertex_shaders{};
        std::array<RhiShaderModuleHandle, yuengine::rhi::MAX_RHI_PIPELINES> pixel_shaders{};
        std::array<RhiPipelineHandle, yuengine::rhi::MAX_RHI_PIPELINES> pipelines{};
        for (std::size_t index = 0U; index < pipelines.size(); ++index) {
            if (!CreatePipelineWithShaderHandles(
                    device,
                    &vertex_shaders[index],
                    &pixel_shaders[index],
                    &pipelines[index])) {
                return Fail("failed to reserve pipeline capacity");
            }

            if (device.DestroyShaderModule(pixel_shaders[index]) != RhiStatus::Success ||
                device.DestroyShaderModule(vertex_shaders[index]) != RhiStatus::Success) {
                return Fail("failed to free shader module capacity");
            }
        }

        const auto before = device.Snapshot();
        if (before.resources.pipeline_count != yuengine::rhi::MAX_RHI_PIPELINES ||
            before.resources.shader_module_count != 0U) {
            return Fail("pipeline failure setup did not isolate pipeline capacity");
        }

        std::array<RuntimeAssetCookedShaderStagePayloadDesc, 2U> stages =
            ValidCookedShaderStages(vertex_bytes, pixel_bytes);
        const RuntimeAssetCookedProgramDesc program =
            CookedProgramDescriptor(std::span<const RuntimeAssetCookedShaderStagePayloadDesc>(stages.data(), stages.size()));
        RuntimeAssetCookedShaderProgramPipelineResult result{};
        if (!BuildCookedShaderProgram(device, program, &result) ||
            result.status != RuntimeAssetDataStatus::RhiPipelineFailed) {
            return Fail("cooked shader bridge did not report pipeline capacity failure");
        }

        const auto after = device.Snapshot();
        if (after.resources.shader_module_count != before.resources.shader_module_count ||
            after.resources.pipeline_count != before.resources.pipeline_count ||
            after.failed_operation_count != before.failed_operation_count + 1U) {
            return Fail("cooked shader bridge did not cleanup transient modules after pipeline failure");
        }

        if (result.created_shader_module_count != 2U ||
            result.destroyed_shader_module_count != 2U ||
            result.published_handles ||
            result.vertex_shader.generation != 0U ||
            result.pixel_shader.generation != 0U ||
            result.pipeline.generation != 0U) {
            return Fail("cooked shader bridge published handles after pipeline failure");
        }
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
            "cam=Camera/Main.yucamera\n",
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

int RuntimeAssetDataGenericRenderSceneSubmissionBuildsFrameFromLoadedSceneRecords() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    RuntimeAssetRhiDevice device;
    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    std::array<RuntimeAssetRenderSceneGeometryBinding, 3U> geometry_bindings{};
    RuntimeAssetRenderSceneMaterialBinding material_binding{};
    if (!LoadGenericRenderSceneSubmissionInputs(
            "GenericRenderSceneSubmission",
            &table,
            &registry,
            &manager,
            &graph,
            &device,
            &geometry,
            &material,
            &camera,
            &geometry_bindings,
            &material_binding)) {
        return Fail("generic render scene submission inputs failed");
    }

    std::array<RuntimeAssetRenderSceneMaterialBinding, 1U> material_bindings{material_binding};
    std::array<RenderSceneRuntimeFrameEntityRequest, 3U> frame_entities{};
    std::array<RenderSceneRuntimeFrameDrawRecord, 3U> draws{};
    RuntimeAssetRenderSceneSubmissionRequest request = BuildGenericRenderSceneSubmissionRequest(
        graph,
        std::span<const RuntimeAssetSceneEntityRecord>(graph.scene_entities.data(), graph.scene_entities.size()),
        std::span<const RuntimeAssetSceneTransformOutputRecord>(graph.scene_transforms.data(), graph.scene_transforms.size()),
        std::span<const RuntimeAssetRenderSceneGeometryBinding>(geometry_bindings.data(), geometry_bindings.size()),
        std::span<const RuntimeAssetRenderSceneMaterialBinding>(material_bindings.data(), material_bindings.size()),
        camera,
        std::span<RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
        std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()));

    RuntimeAssetRenderSceneSubmissionResult result{};
    const RuntimeAssetDataStatus status = BuildRuntimeAssetRenderSceneSubmission(request, &result);
    if (status != RuntimeAssetDataStatus::Success ||
        result.status != RuntimeAssetDataStatus::Success ||
        result.frame_status != RenderSceneRuntimeFrameStatus::Success) {
        return Fail("generic render scene submission failed");
    }

    if (result.submitted_entity_count != graph.scene_output.entity_count ||
        result.output_draw_count != graph.scene_output.entity_count ||
        result.skipped_entity_count != 0U ||
        result.material_variant_count != 1U ||
        result.shared_material_ref_index != graph.scene_entities[0U].material_ref_index) {
        return Fail("generic render scene submission counts changed");
    }

    for (std::size_t index = 0U; index < frame_entities.size(); ++index) {
        const RuntimeAssetSceneEntityRecord &source_entity = graph.scene_entities[index];
        const RenderSceneRuntimeFrameEntityRequest &frame_entity = frame_entities[index];
        if (frame_entity.world_object_id.value != source_entity.world_object_id.value ||
            !Approx(frame_entity.transform.translation_x, graph.scene_transforms[index].transform.translation_x) ||
            frame_entity.geometry.kind != geometry[index].kind ||
            draws[index].draw.material_id != material.material_id) {
            return Fail("generic render scene submission output record mismatch");
        }
    }

    return 0;
}

int RuntimeAssetDataGenericRenderSceneSubmissionUsesMeshRefsNotEntityOrder() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    RuntimeAssetRhiDevice device;
    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    std::array<RuntimeAssetRenderSceneGeometryBinding, 3U> geometry_bindings{};
    RuntimeAssetRenderSceneMaterialBinding material_binding{};
    if (!LoadGenericRenderSceneSubmissionInputs(
            "GenericRenderSceneSubmissionMeshRefs",
            &table,
            &registry,
            &manager,
            &graph,
            &device,
            &geometry,
            &material,
            &camera,
            &geometry_bindings,
            &material_binding)) {
        return Fail("generic render scene submission mesh ref inputs failed");
    }

    std::array<RuntimeAssetRenderSceneGeometryBinding, 3U> reversed_geometry_bindings{
        geometry_bindings[2U],
        geometry_bindings[1U],
        geometry_bindings[0U]};
    std::array<RuntimeAssetRenderSceneMaterialBinding, 1U> material_bindings{material_binding};
    std::array<RenderSceneRuntimeFrameEntityRequest, 3U> frame_entities{};
    std::array<RenderSceneRuntimeFrameDrawRecord, 3U> draws{};
    RuntimeAssetRenderSceneSubmissionRequest request = BuildGenericRenderSceneSubmissionRequest(
        graph,
        std::span<const RuntimeAssetSceneEntityRecord>(graph.scene_entities.data(), graph.scene_entities.size()),
        std::span<const RuntimeAssetSceneTransformOutputRecord>(graph.scene_transforms.data(), graph.scene_transforms.size()),
        std::span<const RuntimeAssetRenderSceneGeometryBinding>(
            reversed_geometry_bindings.data(),
            reversed_geometry_bindings.size()),
        std::span<const RuntimeAssetRenderSceneMaterialBinding>(material_bindings.data(), material_bindings.size()),
        camera,
        std::span<RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
        std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()));

    RuntimeAssetRenderSceneSubmissionResult result{};
    const RuntimeAssetDataStatus status = BuildRuntimeAssetRenderSceneSubmission(request, &result);
    if (status != RuntimeAssetDataStatus::Success || result.output_draw_count != 3U) {
        return Fail("generic render scene submission mesh ref route failed");
    }

    for (std::size_t index = 0U; index < frame_entities.size(); ++index) {
        if (frame_entities[index].geometry.kind != geometry[index].kind ||
            frame_entities[index].geometry.draw.draw_id != geometry[index].draw.draw_id) {
            return Fail("generic render scene submission used binding array order");
        }
    }

    return 0;
}

int RuntimeAssetDataGenericRenderSceneSubmissionRejectsMissingTransformWithoutMutation() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    RuntimeAssetRhiDevice device;
    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    std::array<RuntimeAssetRenderSceneGeometryBinding, 3U> geometry_bindings{};
    RuntimeAssetRenderSceneMaterialBinding material_binding{};
    if (!LoadGenericRenderSceneSubmissionInputs(
            "GenericRenderSceneSubmissionMissingTransform",
            &table,
            &registry,
            &manager,
            &graph,
            &device,
            &geometry,
            &material,
            &camera,
            &geometry_bindings,
            &material_binding)) {
        return Fail("generic render scene submission missing transform inputs failed");
    }

    std::array<RuntimeAssetSceneTransformOutputRecord, 3U> transforms = graph.scene_transforms;
    transforms[1U].world_object_id = WorldObjectId{990001U};
    std::array<RuntimeAssetRenderSceneMaterialBinding, 1U> material_bindings{material_binding};
    std::array<RenderSceneRuntimeFrameEntityRequest, 3U> frame_entities{};
    std::array<RenderSceneRuntimeFrameDrawRecord, 3U> draws{};
    SeedGenericRenderSceneSubmissionSentinels(
        std::span<RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
        std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()));
    RuntimeAssetRenderSceneSubmissionRequest request = BuildGenericRenderSceneSubmissionRequest(
        graph,
        std::span<const RuntimeAssetSceneEntityRecord>(graph.scene_entities.data(), graph.scene_entities.size()),
        std::span<const RuntimeAssetSceneTransformOutputRecord>(transforms.data(), transforms.size()),
        std::span<const RuntimeAssetRenderSceneGeometryBinding>(geometry_bindings.data(), geometry_bindings.size()),
        std::span<const RuntimeAssetRenderSceneMaterialBinding>(material_bindings.data(), material_bindings.size()),
        camera,
        std::span<RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
        std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()));

    RuntimeAssetRenderSceneSubmissionResult result{};
    const RuntimeAssetDataStatus status = BuildRuntimeAssetRenderSceneSubmission(request, &result);
    if (status != RuntimeAssetDataStatus::MissingDependency ||
        result.first_failed_entity_index != 1U ||
        result.frame_status != RenderSceneRuntimeFrameStatus::MissingEntity) {
        return Fail("generic render scene submission missing transform diagnostics changed");
    }

    if (!GenericRenderSceneSubmissionSentinelsUnchanged(
            std::span<const RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
            std::span<const RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()))) {
        return Fail("generic render scene submission missing transform mutated outputs");
    }

    return 0;
}

int RuntimeAssetDataGenericRenderSceneSubmissionRejectsMissingMeshRefWithoutMutation() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    RuntimeAssetRhiDevice device;
    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    std::array<RuntimeAssetRenderSceneGeometryBinding, 3U> geometry_bindings{};
    RuntimeAssetRenderSceneMaterialBinding material_binding{};
    if (!LoadGenericRenderSceneSubmissionInputs(
            "GenericRenderSceneSubmissionMissingMesh",
            &table,
            &registry,
            &manager,
            &graph,
            &device,
            &geometry,
            &material,
            &camera,
            &geometry_bindings,
            &material_binding)) {
        return Fail("generic render scene submission missing mesh inputs failed");
    }

    const std::array<RuntimeAssetRenderSceneGeometryBinding, 2U> missing_geometry_bindings{
        geometry_bindings[0U],
        geometry_bindings[2U]};
    std::array<RuntimeAssetRenderSceneMaterialBinding, 1U> material_bindings{material_binding};
    std::array<RenderSceneRuntimeFrameEntityRequest, 3U> frame_entities{};
    std::array<RenderSceneRuntimeFrameDrawRecord, 3U> draws{};
    SeedGenericRenderSceneSubmissionSentinels(
        std::span<RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
        std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()));
    RuntimeAssetRenderSceneSubmissionRequest request = BuildGenericRenderSceneSubmissionRequest(
        graph,
        std::span<const RuntimeAssetSceneEntityRecord>(graph.scene_entities.data(), graph.scene_entities.size()),
        std::span<const RuntimeAssetSceneTransformOutputRecord>(graph.scene_transforms.data(), graph.scene_transforms.size()),
        std::span<const RuntimeAssetRenderSceneGeometryBinding>(
            missing_geometry_bindings.data(),
            missing_geometry_bindings.size()),
        std::span<const RuntimeAssetRenderSceneMaterialBinding>(material_bindings.data(), material_bindings.size()),
        camera,
        std::span<RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
        std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()));

    RuntimeAssetRenderSceneSubmissionResult result{};
    const RuntimeAssetDataStatus status = BuildRuntimeAssetRenderSceneSubmission(request, &result);
    if (status != RuntimeAssetDataStatus::MissingDependency ||
        result.first_failed_entity_index != 1U ||
        result.first_missing_resource_ref_index != graph.scene_entities[1U].mesh_ref_index ||
        result.frame_status != RenderSceneRuntimeFrameStatus::MissingGeometryRecord) {
        return Fail("generic render scene submission missing mesh diagnostics changed");
    }

    if (!GenericRenderSceneSubmissionSentinelsUnchanged(
            std::span<const RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
            std::span<const RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()))) {
        return Fail("generic render scene submission missing mesh mutated outputs");
    }

    return 0;
}

int RuntimeAssetDataGenericRenderSceneSubmissionRejectsMissingMaterialRefWithoutMutation() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    RuntimeAssetRhiDevice device;
    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    std::array<RuntimeAssetRenderSceneGeometryBinding, 3U> geometry_bindings{};
    RuntimeAssetRenderSceneMaterialBinding material_binding{};
    if (!LoadGenericRenderSceneSubmissionInputs(
            "GenericRenderSceneSubmissionMissingMaterial",
            &table,
            &registry,
            &manager,
            &graph,
            &device,
            &geometry,
            &material,
            &camera,
            &geometry_bindings,
            &material_binding)) {
        return Fail("generic render scene submission missing material inputs failed");
    }

    material_binding.resource_ref_index = 990002U;
    std::array<RuntimeAssetRenderSceneMaterialBinding, 1U> material_bindings{material_binding};
    std::array<RenderSceneRuntimeFrameEntityRequest, 3U> frame_entities{};
    std::array<RenderSceneRuntimeFrameDrawRecord, 3U> draws{};
    SeedGenericRenderSceneSubmissionSentinels(
        std::span<RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
        std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()));
    RuntimeAssetRenderSceneSubmissionRequest request = BuildGenericRenderSceneSubmissionRequest(
        graph,
        std::span<const RuntimeAssetSceneEntityRecord>(graph.scene_entities.data(), graph.scene_entities.size()),
        std::span<const RuntimeAssetSceneTransformOutputRecord>(graph.scene_transforms.data(), graph.scene_transforms.size()),
        std::span<const RuntimeAssetRenderSceneGeometryBinding>(geometry_bindings.data(), geometry_bindings.size()),
        std::span<const RuntimeAssetRenderSceneMaterialBinding>(material_bindings.data(), material_bindings.size()),
        camera,
        std::span<RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
        std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()));

    RuntimeAssetRenderSceneSubmissionResult result{};
    const RuntimeAssetDataStatus status = BuildRuntimeAssetRenderSceneSubmission(request, &result);
    if (status != RuntimeAssetDataStatus::MissingDependency ||
        result.first_failed_entity_index != 0U ||
        result.first_missing_resource_ref_index != graph.scene_entities[0U].material_ref_index ||
        result.frame_status != RenderSceneRuntimeFrameStatus::MissingMaterialRecord) {
        return Fail("generic render scene submission missing material diagnostics changed");
    }

    if (!GenericRenderSceneSubmissionSentinelsUnchanged(
            std::span<const RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
            std::span<const RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()))) {
        return Fail("generic render scene submission missing material mutated outputs");
    }

    return 0;
}

int RuntimeAssetDataGenericRenderSceneSubmissionReportsMaterialVariantsUntilFrameApiSupportsThem() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    RuntimeAssetRhiDevice device;
    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    std::array<RuntimeAssetRenderSceneGeometryBinding, 3U> geometry_bindings{};
    RuntimeAssetRenderSceneMaterialBinding material_binding{};
    if (!LoadGenericRenderSceneSubmissionInputs(
            "GenericRenderSceneSubmissionMaterialVariants",
            &table,
            &registry,
            &manager,
            &graph,
            &device,
            &geometry,
            &material,
            &camera,
            &geometry_bindings,
            &material_binding)) {
        return Fail("generic render scene submission material variant inputs failed");
    }

    std::array<RuntimeAssetSceneEntityRecord, 3U> scene_entities = graph.scene_entities;
    const std::uint32_t variant_material_ref_index = graph.scene_entities[0U].material_ref_index + 100U;
    scene_entities[1U].material_ref_index = variant_material_ref_index;
    RuntimeAssetRenderSceneMaterialBinding variant_material_binding{};
    variant_material_binding.resource_ref_index = variant_material_ref_index;
    variant_material_binding.material = material;
    std::array<RuntimeAssetRenderSceneMaterialBinding, 2U> material_bindings{
        material_binding,
        variant_material_binding};
    std::array<RenderSceneRuntimeFrameEntityRequest, 3U> frame_entities{};
    std::array<RenderSceneRuntimeFrameDrawRecord, 3U> draws{};
    SeedGenericRenderSceneSubmissionSentinels(
        std::span<RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
        std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()));
    RuntimeAssetRenderSceneSubmissionRequest request = BuildGenericRenderSceneSubmissionRequest(
        graph,
        std::span<const RuntimeAssetSceneEntityRecord>(scene_entities.data(), scene_entities.size()),
        std::span<const RuntimeAssetSceneTransformOutputRecord>(graph.scene_transforms.data(), graph.scene_transforms.size()),
        std::span<const RuntimeAssetRenderSceneGeometryBinding>(geometry_bindings.data(), geometry_bindings.size()),
        std::span<const RuntimeAssetRenderSceneMaterialBinding>(material_bindings.data(), material_bindings.size()),
        camera,
        std::span<RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
        std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()));

    RuntimeAssetRenderSceneSubmissionResult result{};
    const RuntimeAssetDataStatus status = BuildRuntimeAssetRenderSceneSubmission(request, &result);
    if (status != RuntimeAssetDataStatus::UnsupportedFieldValue ||
        result.first_failed_entity_index != 1U ||
        result.first_missing_resource_ref_index != variant_material_ref_index ||
        result.material_variant_count != 2U ||
        result.frame_status != RenderSceneRuntimeFrameStatus::InvalidMaterialRecord) {
        return Fail("generic render scene submission material variant diagnostics changed");
    }

    if (!GenericRenderSceneSubmissionSentinelsUnchanged(
            std::span<const RenderSceneRuntimeFrameEntityRequest>(frame_entities.data(), frame_entities.size()),
            std::span<const RenderSceneRuntimeFrameDrawRecord>(draws.data(), draws.size()))) {
        return Fail("generic render scene submission material variant mutated outputs");
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
        graph.scene_resource_refs[8U].kind != RuntimeAssetFileKind::Animation ||
        graph.scene_resource_refs[9U].kind != RuntimeAssetFileKind::Camera) {
        return Fail("production scene loader resource refs changed");
    }

    if (!graph.scene_resource_refs[0U].resource.IsValid() ||
        !graph.scene_resource_refs[0U].asset.IsValid() ||
        graph.scene_resource_refs[0U].stable_id != 1001U ||
        graph.scene_resource_refs[8U].stable_id != 5001U ||
        graph.scene_resource_refs[9U].stable_id != 7001U) {
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
        "cam=Camera/Main.yucamera\n"
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

int RuntimeAssetDataSceneAnimationLoaderLoadsBoundedNEntityScene() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("BoundedNEntityScene"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteBoundedFixture(table)) {
        return Fail("bounded fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    BoundedLoadedGraph graph{};
    if (!LoadBoundedRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("bounded runtime asset records failed");
    }

    if (graph.scene_output.status != RuntimeAssetDataStatus::Success ||
        graph.scene_output.entity_count != 4U ||
        graph.scene_output.transform_count != 4U ||
        graph.scene_output.camera_count != 2U ||
        graph.scene_output.animation_sampled_value_count != 2U) {
        return Fail("bounded scene loader output counts changed");
    }

    if (graph.scene_cameras[0U].camera_id != 11U ||
        graph.scene_cameras[0U].is_active ||
        graph.scene_cameras[1U].camera_id != 12U ||
        !graph.scene_cameras[1U].is_active) {
        return Fail("bounded camera table was not emitted");
    }

    if (graph.scene_entities[0U].world_object_id.value != 104U ||
        graph.scene_entities[1U].world_object_id.value != 102U ||
        graph.scene_entities[2U].world_object_id.value != 101U ||
        graph.scene_entities[3U].world_object_id.value != 103U) {
        return Fail("bounded entity deterministic sort order changed");
    }

    if (graph.scene_entities[0U].mesh_ref_index != 0U ||
        graph.scene_entities[1U].mesh_ref_index != 1U ||
        graph.scene_entities[3U].mesh_ref_index != 2U ||
        graph.scene_entities[0U].camera_index != 1U) {
        return Fail("bounded entity resource or camera refs changed");
    }

    if (!Approx(graph.scene_entities[2U].transform.rotation_y, 0.5F) ||
        !Approx(graph.scene_entities[0U].transform.translation_y, 2.0F)) {
        return Fail("bounded animation tracks did not sample/apply to scene transforms");
    }

    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("initialize bounded render rhi failed");
    }

    if (!ExecuteBoundedRenderPath(device, registry, manager, &graph)) {
        return Fail("bounded render path failed");
    }

    if (graph.frame_result.output_draw_count != 4U ||
        graph.render_result_count != 4U ||
        graph.capture_bytes_written == 0U) {
        return Fail("RenderScene/RenderCore/RHI did not consume bounded loader records");
    }

    return 0;
}

int RuntimeAssetDataSceneAnimationLoaderRejectsEntityCapacityOverflowWithoutMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("BoundedCapacityNoMutation"), &table)) {
        return Fail("mount setup failed");
    }

    if (!WriteBoundedFixture(table)) {
        return Fail("bounded fixture write failed");
    }

    if (!ProbeBoundedSceneLoaderFailureWithoutOutputMutation(
            table,
            RuntimeAssetDataStatus::CapacityExceeded,
            3U,
            2U,
            4U)) {
        return Fail("bounded entity capacity overflow mutated outputs");
    }

    return 0;
}

int RuntimeAssetDataSceneAnimationLoaderRejectsMissingRefsWithoutMutation() {
    const std::string scene = ReplaceFirst(BoundedSceneBytes(), "mesh_ref=1", "mesh_ref=99");
    if (!ProbeBoundedFailureCase(
            "BoundedMissingRefsNoMutation",
            scene,
            BoundedAnimationBytes(),
            RuntimeAssetDataStatus::MissingDependency)) {
        return Fail("bounded missing ref failure mutated outputs");
    }

    return 0;
}

int RuntimeAssetDataSceneAnimationLoaderRejectsInvalidRecordsWithoutMutation() {
    if (!ProbeBoundedFailureCase(
            "BoundedInvalidTransformNoMutation",
            ReplaceFirst(BoundedSceneBytes(), "104:3,1,0", "104:3,nan,0"),
            BoundedAnimationBytes(),
            RuntimeAssetDataStatus::InvalidBounds)) {
        return Fail("bounded invalid transform failure mutated outputs");
    }

    if (!ProbeBoundedFailureCase(
            "BoundedInvalidTrackRangeNoMutation",
            BoundedSceneBytes(),
            ReplaceFirst(BoundedAnimationBytes(), "track_count=2", "track_count=3"),
            RuntimeAssetDataStatus::InvalidBounds)) {
        return Fail("bounded invalid track range failure mutated outputs");
    }

    if (!ProbeBoundedFailureCase(
            "BoundedNonFiniteKeyframeNoMutation",
            BoundedSceneBytes(),
            ReplaceFirst(BoundedAnimationBytes(), "key3=1:3", "key3=1:nan"),
            RuntimeAssetDataStatus::InvalidBounds)) {
        return Fail("bounded non-finite keyframe failure mutated outputs");
    }

    if (!ProbeBoundedFailureCase(
            "BoundedNonMonotonicKeyframeNoMutation",
            BoundedSceneBytes(),
            ReplaceFirst(BoundedAnimationBytes(), "key1=1:1", "key1=0:1"),
            RuntimeAssetDataStatus::InvalidBounds)) {
        return Fail("bounded non-monotonic keyframe failure mutated outputs");
    }

    if (!ProbeBoundedFailureCase(
            "BoundedTargetMismatchNoMutation",
            BoundedSceneBytes(),
            ReplaceFirst(BoundedAnimationBytes(), "target_ref=scene_entity:104", "target_ref=scene_entity:999"),
            RuntimeAssetDataStatus::InvalidDependency)) {
        return Fail("bounded target mismatch failure mutated outputs");
    }

    if (!ProbeBoundedFailureCase(
            "BoundedHashMismatchNoMutation",
            ReplaceFirst(BoundedSceneBytes(), "id=bounded_scene\n", "id=bounded_scene\nexpected_hash=1\n"),
            BoundedAnimationBytes(),
            RuntimeAssetDataStatus::HashMismatch)) {
        return Fail("bounded hash mismatch failure mutated outputs");
    }

    return 0;
}

int RuntimeAssetDataSceneAnimationLoaderRejectsCameraFamilyFailuresWithoutMutation() {
    if (!ProbeBoundedFailureCase(
            "BoundedDuplicateActiveCameraNoMutation",
            ReplaceFirst(BoundedSceneBytes(), "camera0=11:inactive", "camera0=11:active"),
            BoundedAnimationBytes(),
            RuntimeAssetDataStatus::DuplicateDependency)) {
        return Fail("bounded duplicate active camera failure mutated outputs");
    }

    if (!ProbeBoundedFailureCase(
            "BoundedNoActiveCameraNoMutation",
            ReplaceFirst(BoundedSceneBytes(), "camera1=12:active", "camera1=12:inactive"),
            BoundedAnimationBytes(),
            RuntimeAssetDataStatus::MissingDependency)) {
        return Fail("bounded no active camera failure mutated outputs");
    }

    if (!ProbeBoundedFailureCase(
            "BoundedInvalidCameraRowNoMutation",
            ReplaceFirst(BoundedSceneBytes(), "camera1=12:active", "camera1=12:maybe"),
            BoundedAnimationBytes(),
            RuntimeAssetDataStatus::InvalidDependency)) {
        return Fail("bounded invalid camera row failure mutated outputs");
    }

    const char *valid_camera_ref =
        "camera=1|animation_ref=0|sort=10";
    const char *invalid_camera_ref =
        "camera=2|animation_ref=0|sort=10";
    if (!ProbeBoundedFailureCase(
            "BoundedInvalidCameraRefNoMutation",
            ReplaceFirst(BoundedSceneBytes(), valid_camera_ref, invalid_camera_ref),
            BoundedAnimationBytes(),
            RuntimeAssetDataStatus::InvalidDependency)) {
        return Fail("bounded invalid camera ref failure mutated outputs");
    }

    return 0;
}

int RuntimeAssetDataSceneAnimationLoaderPathIndependentSceneAnimationDetection() {
    constexpr const char *scene_path = "Scene/BoundedScene.payload";
    constexpr const char *animation_path = "Animation/BoundedAnimation.payload";
    MountTable table;
    if (!CreateMountedTable(TestRoot("BoundedPathIndependent"), &table)) {
        return Fail("mount setup failed");
    }

    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    for (std::size_t index = 0U; index < files.size(); ++index) {
        if (index == 8U) {
            continue;
        }

        if (!WriteBytes(table, files[index].desc.path, BytesFromString(std::string(files[index].bytes)))) {
            return Fail("path-independent bounded file write failed");
        }
    }

    std::string scene_text = ReplaceFirst(BoundedSceneBytes(), "Animation/S.yuanim", animation_path);
    if (!WriteBytes(table, scene_path, BytesFromString(scene_text)) ||
        !WriteBytes(table, animation_path, BytesFromString(BoundedAnimationBytes()))) {
        return Fail("path-independent bounded scene/animation write failed");
    }

    std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> file_descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        file_descs[index] = files[index].desc;
    }
    file_descs[8U].path = animation_path;

    ResourceRegistry registry;
    AssetManager manager;
    BoundedLoadedGraph graph{};
    RuntimeAssetGraphLoadRequest load_request{};
    load_request.mount_table = &table;
    load_request.mount = MountId(MOUNT_ID);
    load_request.scene_path = VirtualPath(scene_path);
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
    load_request.animation_frame_context.frame_index = 1U;
    load_request.animation_frame_context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    load_request.animation_frame_context.fixed_time_nanoseconds = HALF_SECOND_NANOSECONDS;

    RuntimeAssetGraphLoadResult load_result{};
    const RuntimeAssetDataStatus load_status = LoadRuntimeAssetDataGraph(load_request, &load_result);
    if (load_status != RuntimeAssetDataStatus::Success) {
        return Fail("path-independent bounded scene/animation load failed");
    }

    if (graph.scene_output.entity_count != 4U ||
        graph.scene_output.animation_sampled_value_count != 2U ||
        !load_result.scene_references_runtime_asset_families) {
        return Fail("path-independent bounded scene/animation detection changed");
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

int RuntimeAssetDataCookedTexturePayloadTableValidatesLayoutHashAndRowPitch() {
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{};
    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> textures{};
    if (!LoadCookedTextureMaterialFixture(
            "CookedTexturePayloadLayout",
            registry,
            manager,
            &graph,
            &texture_assets,
            &textures)) {
        return Fail("cooked texture material fixture failed");
    }

    const std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> slots =
        CookedMaterialSlots();
    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("initialize cooked texture layout rhi failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreatePipeline(device, &pipeline)) {
        return Fail("create cooked texture layout pipeline failed");
    }

    RenderSceneRuntimeMaterialRecord material{};
    const RuntimeAssetCookedTextureMaterialBridgeResult result = InvokeCookedMaterialBridge(
        device,
        registry,
        manager,
        graph.assets[3U].asset,
        pipeline,
        std::span<const RuntimeAssetCookedTexturePayloadDesc>(textures.data(), textures.size()),
        std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
        &material);
    if (result.status != RuntimeAssetDataStatus::Success ||
        !result.published_material ||
        !result.mutated_state) {
        return Fail("valid cooked texture payload table did not bridge");
    }

    if (result.runtime_texture_upload_count != RUNTIME_TEXTURE_SLOT_COUNT ||
        result.material_texture_slot_count != RUNTIME_TEXTURE_SLOT_COUNT ||
        material.texture_slot_count != RUNTIME_TEXTURE_SLOT_COUNT) {
        return Fail("valid cooked texture payload bridge counts changed");
    }

    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> bad_row_pitch = textures;
    bad_row_pitch[0U].row_pitch_bytes -= static_cast<std::uint32_t>(yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(bad_row_pitch.data(), bad_row_pitch.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
            RuntimeAssetDataStatus::InvalidSize)) {
        return Fail("invalid cooked row pitch mutated outputs");
    }

    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> bad_hash = textures;
    bad_hash[0U].payload_hash += 1U;
    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(bad_hash.data(), bad_hash.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
            RuntimeAssetDataStatus::HashMismatch)) {
        return Fail("invalid cooked payload hash mutated outputs");
    }

    return 0;
}

int RuntimeAssetDataCookedMaterialTextureSlotTableResolvesLoadedPayloads() {
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{};
    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> textures{};
    if (!LoadCookedTextureMaterialFixture(
            "CookedMaterialSlotTable",
            registry,
            manager,
            &graph,
            &texture_assets,
            &textures)) {
        return Fail("cooked material slot fixture failed");
    }

    const std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> slots =
        CookedMaterialSlots();
    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("initialize cooked material slot rhi failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreatePipeline(device, &pipeline)) {
        return Fail("create cooked material slot pipeline failed");
    }

    RenderSceneRuntimeMaterialRecord material{};
    const RuntimeAssetCookedTextureMaterialBridgeResult result = InvokeCookedMaterialBridge(
        device,
        registry,
        manager,
        graph.assets[3U].asset,
        pipeline,
        std::span<const RuntimeAssetCookedTexturePayloadDesc>(textures.data(), textures.size()),
        std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
        &material);
    if (result.status != RuntimeAssetDataStatus::Success) {
        return Fail("cooked material slot bridge failed");
    }

    for (std::size_t index = 0U; index < texture_assets.size(); ++index) {
        const RenderSceneRuntimeMaterialTextureSlot &slot = material.texture_slots[index];
        if (slot.slot != index ||
            slot.texture_asset.slot != texture_assets[index].asset.slot ||
            slot.sampled_texture.slot != index ||
            slot.sampler.slot != index ||
            slot.sampled_texture.texture.generation == 0U ||
            slot.sampler.sampler.generation == 0U) {
            return Fail("cooked material slot did not resolve expected binding");
        }

        AssetRecord texture_record{};
        if (manager.QueryAsset(texture_assets[index].asset, &texture_record) != AssetStatus::Success ||
            !texture_record.texture_ready.is_ready ||
            texture_record.texture_ready.sampled_texture.slot != index) {
            return Fail("cooked material slot did not mark texture asset ready");
        }
    }

    return 0;
}

int RuntimeAssetDataCookedMaterialConstantsBridgeToRenderSceneRecord() {
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{};
    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> textures{};
    if (!LoadCookedTextureMaterialFixture(
            "CookedMaterialConstants",
            registry,
            manager,
            &graph,
            &texture_assets,
            &textures)) {
        return Fail("cooked material constant fixture failed");
    }

    const std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> slots =
        CookedMaterialSlots();
    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("initialize cooked material constant rhi failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreatePipeline(device, &pipeline)) {
        return Fail("create cooked material constant pipeline failed");
    }

    RuntimeAssetPackedMaterialConstants constants{};
    if (PackRuntimeAssetMaterialConstants(graph.assets[3U], &constants) != RuntimeAssetDataStatus::Success ||
        !ExpectPackedMaterialConstants(constants)) {
        return Fail("cook material constant expected pack failed");
    }

    RenderSceneRuntimeMaterialRecord material{};
    const RuntimeAssetCookedTextureMaterialBridgeResult result = InvokeCookedMaterialBridgeWithMaterial(
        device,
        registry,
        manager,
        graph.assets[3U].asset,
        &graph.assets[3U],
        pipeline,
        std::span<const RuntimeAssetCookedTexturePayloadDesc>(textures.data(), textures.size()),
        std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
        &material);
    if (result.status != RuntimeAssetDataStatus::Success ||
        !result.published_material ||
        result.material_constant_byte_count != constants.byte_count ||
        result.material_constant_hash != constants.hash) {
        return Fail("cooked material constants did not bridge");
    }

    if (!ExpectRenderSceneMaterialConstants(material, constants.hash)) {
        return Fail("cooked material constants did not reach render scene record");
    }

    return 0;
}

int RuntimeAssetDataCookedMaterialConstantsRejectInvalidLoadedMaterialWithoutMutation() {
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{};
    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> textures{};
    if (!LoadCookedTextureMaterialFixture(
            "CookedMaterialConstantRejects",
            registry,
            manager,
            &graph,
            &texture_assets,
            &textures)) {
        return Fail("cooked material constant reject fixture failed");
    }

    const std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> slots =
        CookedMaterialSlots();
    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("initialize cooked material constant reject rhi failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreatePipeline(device, &pipeline)) {
        return Fail("create cooked material constant reject pipeline failed");
    }

    RuntimeAssetLoadedFile bad_material = graph.assets[3U];
    bad_material.material_parameter_count = MATERIAL_PARAMETER_COUNT + 1U;
    const auto before_snapshot = device.Snapshot();
    RenderSceneRuntimeMaterialRecord material{};
    material.material_id = 77U;
    const RuntimeAssetCookedTextureMaterialBridgeResult result = InvokeCookedMaterialBridgeWithMaterial(
        device,
        registry,
        manager,
        graph.assets[3U].asset,
        &bad_material,
        pipeline,
        std::span<const RuntimeAssetCookedTexturePayloadDesc>(textures.data(), textures.size()),
        std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
        &material);
    if (result.status != RuntimeAssetDataStatus::InvalidCount ||
        result.mutated_state ||
        result.published_material ||
        result.material_constant_byte_count != 0U ||
        result.material_constant_hash != 0U) {
        return Fail("invalid material constants bridge did not fail before mutation");
    }

    if (material.material_id != 77U || material.is_resolved || material.texture_slot_count != 0U) {
        return Fail("invalid material constants bridge mutated render scene output");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.resources.texture_count != before_snapshot.resources.texture_count ||
        after_snapshot.resources.sampler_count != before_snapshot.resources.sampler_count) {
        return Fail("invalid material constants bridge mutated rhi primitives");
    }

    return 0;
}

int RuntimeAssetDataCookedPayloadBridgeRejectsTextureFormatExtentSizeAlignmentHashWithoutMutation() {
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{};
    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> textures{};
    if (!LoadCookedTextureMaterialFixture(
            "CookedPayloadDescriptorRejects",
            registry,
            manager,
            &graph,
            &texture_assets,
            &textures)) {
        return Fail("cooked descriptor fixture failed");
    }

    const std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> slots =
        CookedMaterialSlots();

    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> bad_format = textures;
    bad_format[0U].texture_desc.format = RhiFormat::Unsupported;
    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(bad_format.data(), bad_format.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
            RuntimeAssetDataStatus::UnsupportedFieldValue)) {
        return Fail("unsupported cooked texture format mutated outputs");
    }

    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> bad_extent = textures;
    bad_extent[0U].texture_desc.extent.width = 0U;
    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(bad_extent.data(), bad_extent.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
            RuntimeAssetDataStatus::InvalidBounds)) {
        return Fail("invalid cooked texture extent mutated outputs");
    }

    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> bad_size = textures;
    bad_size[0U].payload_byte_count -= 1U;
    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(bad_size.data(), bad_size.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
            RuntimeAssetDataStatus::InvalidSize)) {
        return Fail("invalid cooked texture size mutated outputs");
    }

    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> bad_alignment = textures;
    bad_alignment[0U].payload_alignment_bytes = 3U;
    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(bad_alignment.data(), bad_alignment.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
            RuntimeAssetDataStatus::InvalidAlignment)) {
        return Fail("invalid cooked texture alignment mutated outputs");
    }

    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> bad_hash = textures;
    bad_hash[0U].payload_hash += 17U;
    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(bad_hash.data(), bad_hash.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
            RuntimeAssetDataStatus::HashMismatch)) {
        return Fail("invalid cooked texture hash mutated outputs");
    }

    return 0;
}

int RuntimeAssetDataCookedPayloadBridgeRejectsMissingDuplicateTypeMismatchDepsWithoutMutation() {
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{};
    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> textures{};
    if (!LoadCookedTextureMaterialFixture(
            "CookedSlotDependencyRejects",
            registry,
            manager,
            &graph,
            &texture_assets,
            &textures)) {
        return Fail("cooked dependency fixture failed");
    }

    const std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> slots =
        CookedMaterialSlots();

    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> missing_payload = textures;
    missing_payload[0U].decoded_payload_id += 700000U;
    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(missing_payload.data(), missing_payload.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
            RuntimeAssetDataStatus::MissingDependency)) {
        return Fail("missing cooked decoded payload mutated outputs");
    }

    std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> duplicate_slots = slots;
    duplicate_slots[1U].material_slot = duplicate_slots[0U].material_slot;
    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(textures.data(), textures.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(duplicate_slots.data(), duplicate_slots.size()),
            RuntimeAssetDataStatus::DuplicateDependency)) {
        return Fail("duplicate cooked material slot mutated outputs");
    }

    std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> color_mismatch = slots;
    color_mismatch[1U].expected_color_space = RuntimeAssetCookedTextureColorSpace::Srgb;
    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(textures.data(), textures.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(color_mismatch.data(), color_mismatch.size()),
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("cooked color-space type mismatch mutated outputs");
    }

    std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> wrong_type_assets = texture_assets;
    wrong_type_assets[0U].kind = RuntimeAssetFileKind::Material;
    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> wrong_type_textures{};
    if (!BuildCookedTexturePayloadDescs(
            registry,
            std::span<const RuntimeAssetLoadedFile>(wrong_type_assets.data(), wrong_type_assets.size()),
            &wrong_type_textures)) {
        return Fail("build wrong-type cooked texture descs failed");
    }

    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(wrong_type_textures.data(), wrong_type_textures.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
            RuntimeAssetDataStatus::TypeMismatch)) {
        return Fail("wrong loaded texture kind mutated outputs");
    }

    return 0;
}

int RuntimeAssetDataCookedMaterialSlotOverflowDoesNotMutateRenderSceneOutputs() {
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{};
    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> textures{};
    if (!LoadCookedTextureMaterialFixture(
            "CookedSlotOverflow",
            registry,
            manager,
            &graph,
            &texture_assets,
            &textures)) {
        return Fail("cooked overflow fixture failed");
    }

    constexpr std::size_t OVERFLOW_SLOT_COUNT = yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS + 1U;
    std::array<RuntimeAssetCookedMaterialSlotDesc, OVERFLOW_SLOT_COUNT> overflow_slots{};
    const std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> base_slots =
        CookedMaterialSlots();
    for (std::size_t index = 0U; index < overflow_slots.size(); ++index) {
        overflow_slots[index] = base_slots[index % base_slots.size()];
        overflow_slots[index].material_slot = static_cast<std::uint32_t>(index);
        overflow_slots[index].texture_binding_slot = static_cast<std::uint32_t>(index);
        overflow_slots[index].sampler_binding_slot = static_cast<std::uint32_t>(index);
    }

    if (!ExpectCookedBridgeFailureWithoutRhiMutation(
            registry,
            manager,
            graph.assets[3U].asset,
            std::span<const RuntimeAssetCookedTexturePayloadDesc>(textures.data(), textures.size()),
            std::span<const RuntimeAssetCookedMaterialSlotDesc>(overflow_slots.data(), overflow_slots.size()),
            RuntimeAssetDataStatus::CapacityExceeded)) {
        return Fail("cooked material slot overflow mutated outputs");
    }

    return 0;
}

int RuntimeAssetDataCookedRhiPartialCreationFailureDestroysTransientHandles() {
    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    std::array<RuntimeAssetLoadedFile, RUNTIME_TEXTURE_SLOT_COUNT> texture_assets{};
    std::array<RuntimeAssetCookedTexturePayloadDesc, RUNTIME_TEXTURE_SLOT_COUNT> textures{};
    if (!LoadCookedTextureMaterialFixture(
            "CookedRhiCleanup",
            registry,
            manager,
            &graph,
            &texture_assets,
            &textures)) {
        return Fail("cooked cleanup fixture failed");
    }

    const std::array<RuntimeAssetCookedMaterialSlotDesc, RUNTIME_TEXTURE_SLOT_COUNT> slots =
        CookedMaterialSlots();
    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("initialize cooked cleanup rhi failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreatePipeline(device, &pipeline)) {
        return Fail("create cooked cleanup pipeline failed");
    }

    std::array<std::uint8_t, RUNTIME_TEXTURE_BYTE_COUNT> texture_bytes{};
    for (std::size_t index = 0U; index < yuengine::rhi::MAX_RHI_TEXTURES - 2U; ++index) {
        RhiTextureHandle texture{};
        const RhiStatus status = device.CreateTexture(
            RuntimeTextureDesc(),
            std::span<const std::uint8_t>(texture_bytes.data(), texture_bytes.size()),
            texture);
        if (status != RhiStatus::Success) {
            return Fail("failed to prefill rhi texture capacity for cooked cleanup");
        }
    }

    const auto before_snapshot = device.Snapshot();
    RenderSceneRuntimeMaterialRecord material{};
    const RuntimeAssetCookedTextureMaterialBridgeResult result = InvokeCookedMaterialBridge(
        device,
        registry,
        manager,
        graph.assets[3U].asset,
        pipeline,
        std::span<const RuntimeAssetCookedTexturePayloadDesc>(textures.data(), textures.size()),
        std::span<const RuntimeAssetCookedMaterialSlotDesc>(slots.data(), slots.size()),
        &material);
    if (result.status != RuntimeAssetDataStatus::RhiTextureFailed ||
        result.texture_bridge_status != ResourceDecodedTextureBridgeStatus::UploadProcessFailed ||
        result.rhi_status != RhiStatus::CapacityExceeded) {
        return Fail("cooked rhi capacity failure did not report texture failure");
    }

    if (!result.mutated_state || result.published_material) {
        return Fail("cooked rhi failure mutation/publish flags are wrong");
    }

    if (result.cleanup_texture_count != 2U || result.cleanup_sampler_count != 2U) {
        return Fail("cooked rhi partial failure cleanup ledger changed");
    }

    if (material.is_resolved || material.texture_slot_count != 0U) {
        return Fail("cooked rhi partial failure published material output");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.resources.texture_count != before_snapshot.resources.texture_count ||
        after_snapshot.resources.sampler_count != before_snapshot.resources.sampler_count) {
        return Fail("cooked rhi partial failure left transient handles active");
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

int PreviewHostConsumesRuntimeAssetGraphAndCapturesThroughRhi() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("PreviewHostCapture"), &table)) {
        return Fail("preview host mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("preview host fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("preview host runtime asset graph load failed");
    }

    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("preview host rhi initialize failed");
    }

    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    if (!BuildPreviewHostSceneInputs(
            device,
            registry,
            manager,
            graph,
            &geometry,
            &material,
            &camera)) {
        return Fail("preview host render inputs failed");
    }

    PreviewHost host;
    PreviewHostSessionResult session_result{};
    if (host.StartSession(PreviewHostSessionDesc{PreviewHostDocumentKind::Scene}, &session_result) !=
        PreviewHostStatus::Success) {
        return Fail("preview host session start failed");
    }

    std::array<std::uint8_t, TOTAL_CAPTURE_BYTES> capture_bytes{};
    std::array<PreviewHostDiagnostic, 4U> diagnostics{};
    std::array<PreviewHostHitRecord, 3U> hits{};
    std::array<PreviewHostSelectionRecord, 3U> selections{};
    std::array<PreviewHostTransformFeedback, 3U> transforms{};
    constexpr std::string_view output_path = "Artifacts/PreviewHost/Canonical.rvf";

    PreviewHostFrameRequest request{};
    request.session = session_result.session;
    request.document_kind = PreviewHostDocumentKind::Scene;
    request.frame.frame_id = FRAME_ID + 101U;
    request.frame.width = 2U;
    request.frame.height = 2U;
    request.frame.format = PreviewHostFrameFormat::Rgba8;
    request.frame.capture_requested = true;
    request.runtime_graph = &graph.load_result;
    request.scene_output = &graph.scene_output;
    request.loaded_files = std::span<const RuntimeAssetLoadedFile>(graph.assets.data(), graph.assets.size());
    request.resource_refs = std::span<const RuntimeAssetSceneResourceRef>(
        graph.scene_resource_refs.data(),
        graph.scene_resource_refs.size());
    request.scene_entities = std::span<const RuntimeAssetSceneEntityRecord>(
        graph.scene_entities.data(),
        graph.scene_entities.size());
    request.geometry_records = std::span<const RenderScenePrimitiveGeometryRecord>(
        geometry.data(),
        geometry.size());
    request.camera = camera;
    request.material = material;
    request.rhi_device = &device;
    request.output_path = output_path.data();
    request.output_path_byte_count = output_path.size();
    request.capture_output = std::span<std::uint8_t>(capture_bytes.data(), capture_bytes.size());
    request.capture_byte_budget_per_entity = CAPTURE_BYTES_PER_ENTITY;
    request.diagnostics = std::span<PreviewHostDiagnostic>(diagnostics.data(), diagnostics.size());
    request.hit_records = std::span<PreviewHostHitRecord>(hits.data(), hits.size());
    request.selection_records = std::span<PreviewHostSelectionRecord>(selections.data(), selections.size());
    request.transform_feedback =
        std::span<PreviewHostTransformFeedback>(transforms.data(), transforms.size());

    PreviewHostFrameResult frame_result{};
    const PreviewHostStatus status = host.BuildFrame(request, &frame_result);
    if (status != PreviewHostStatus::Success) {
        std::fprintf(
            stderr,
            "preview host status=%u diagnostics=%zu code=%u missing=%u frame=%u capture=%u runtime=%u\n",
            static_cast<unsigned>(status),
            frame_result.diagnostic_count,
            static_cast<unsigned>(diagnostics[0U].code),
            static_cast<unsigned>(frame_result.capture.first_missing_layer),
            static_cast<unsigned>(frame_result.render_frame.status),
            static_cast<unsigned>(frame_result.capture.status),
            static_cast<unsigned>(frame_result.runtime_asset_status));
        return Fail("preview host capture did not succeed");
    }

    if (!frame_result.consumed_runtime_asset_graph || !frame_result.consumed_resource_refs) {
        return Fail("preview host did not consume RuntimeAsset graph/resource refs");
    }

    if (!frame_result.submitted_render_scene_frame ||
        !frame_result.captured_through_render_core_rhi ||
        frame_result.capture.status != RenderSceneThreePrimitiveCaptureStatus::Success) {
        return Fail("preview host did not capture through RenderScene/RenderCore/RHI");
    }

    if (frame_result.submitted_entity_count != 3U ||
        frame_result.hit_record_count != 3U ||
        frame_result.selection_record_count != 3U ||
        frame_result.transform_feedback_count != 3U) {
        return Fail("preview host output counts changed");
    }

    if (!hits[0U].hit_available ||
        !selections[1U].selectable ||
        !transforms[2U].transform_available) {
        return Fail("preview host feedback records were not written");
    }

    if (frame_result.capture_bytes_written == 0U) {
        return Fail("preview host capture bytes were not written");
    }

    return 0;
}

int PreviewHostConsumesImportCookCommandOutputs() {
    CookedVisualProofContext context{};
    if (!SetupCookedVisualProofContext("PreviewHostCommandOutput", &context)) {
        return Fail("preview host command output setup failed");
    }

    LoadedGraph graph{};
    graph.assets = context.loaded_files;
    graph.scene_resource_refs = context.scene_refs;
    graph.scene_entities = context.scene_entities;
    graph.scene_output = context.scene_output;
    graph.load_result = context.load_result;

    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    if (!BuildPreviewHostSceneInputs(
            context.device,
            context.registry,
            context.manager,
            graph,
            &geometry,
            &material,
            &camera)) {
        return Fail("preview host command output render inputs failed");
    }

    PreviewHost host;
    PreviewHostSessionResult session_result{};
    if (host.StartSession(PreviewHostSessionDesc{PreviewHostDocumentKind::Scene}, &session_result) !=
        PreviewHostStatus::Success) {
        return Fail("preview host command output session start failed");
    }

    PreviewHostCommandOutputRef command_output{};
    command_output.command = &context.fixture.command;
    command_output.cooked_scene = &context.fixture.command.fixture.cooked_scene;
    command_output.cooked_files = std::span<const RuntimeAssetFileDesc>(
        context.fixture.cooked_files.data(),
        context.fixture.command.fixture.cooked_file_count);
    command_output.require_cooked_records = true;

    std::array<std::uint8_t, TOTAL_CAPTURE_BYTES> capture_bytes{};
    std::array<PreviewHostDiagnostic, 4U> diagnostics{};
    std::array<PreviewHostHitRecord, 3U> hits{};
    std::array<PreviewHostSelectionRecord, 3U> selections{};
    std::array<PreviewHostTransformFeedback, 3U> transforms{};
    constexpr std::string_view output_path = "Artifacts/PreviewHost/ImportCookCommand.rvf";

    PreviewHostFrameRequest request{};
    request.session = session_result.session;
    request.document_kind = PreviewHostDocumentKind::Scene;
    request.frame.frame_id = FRAME_ID + 451U;
    request.frame.width = 2U;
    request.frame.height = 2U;
    request.frame.format = PreviewHostFrameFormat::Rgba8;
    request.frame.capture_requested = true;
    request.command_output = command_output;
    request.runtime_graph = &context.load_result;
    request.scene_output = &context.scene_output;
    request.loaded_files = std::span<const RuntimeAssetLoadedFile>(
        context.loaded_files.data(),
        context.load_result.loaded_file_count);
    request.resource_refs = std::span<const RuntimeAssetSceneResourceRef>(
        context.scene_refs.data(),
        context.scene_output.resource_ref_count);
    request.scene_entities = std::span<const RuntimeAssetSceneEntityRecord>(
        context.scene_entities.data(),
        context.scene_output.entity_count);
    request.geometry_records = std::span<const RenderScenePrimitiveGeometryRecord>(
        geometry.data(),
        geometry.size());
    request.camera = camera;
    request.material = material;
    request.rhi_device = &context.device;
    request.output_path = output_path.data();
    request.output_path_byte_count = output_path.size();
    request.capture_output = std::span<std::uint8_t>(capture_bytes.data(), capture_bytes.size());
    request.capture_byte_budget_per_entity = CAPTURE_BYTES_PER_ENTITY;
    request.diagnostics = std::span<PreviewHostDiagnostic>(diagnostics.data(), diagnostics.size());
    request.hit_records = std::span<PreviewHostHitRecord>(hits.data(), hits.size());
    request.selection_records = std::span<PreviewHostSelectionRecord>(selections.data(), selections.size());
    request.transform_feedback =
        std::span<PreviewHostTransformFeedback>(transforms.data(), transforms.size());

    PreviewHostFrameResult frame_result{};
    if (host.BuildFrame(request, &frame_result) != PreviewHostStatus::Success) {
        return Fail("preview host did not consume import/cook command output");
    }

    if (!frame_result.consumed_import_cook_command_output ||
        frame_result.import_cook_status != RuntimeAssetDataStatus::Success ||
        frame_result.import_cook_missing_layer != RuntimeAssetImportCookMissingLayer::None ||
        frame_result.command_cooked_file_count != RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT ||
        !frame_result.consumed_runtime_asset_graph ||
        !frame_result.submitted_render_scene_frame ||
        !frame_result.captured_through_render_core_rhi ||
        frame_result.capture_bytes_written == 0U) {
        return Fail("preview host command output ledger was incomplete");
    }

    if (frame_result.hit_record_count != 3U ||
        frame_result.selection_record_count != 3U ||
        frame_result.transform_feedback_count != 3U ||
        !hits[0U].hit_available ||
        !selections[1U].selectable ||
        !transforms[2U].transform_available) {
        return Fail("preview host command output feedback was incomplete");
    }

    std::array<PreviewHostDiagnostic, 2U> failure_diagnostics{};
    std::array<PreviewHostHitRecord, 3U> failure_hits{};
    failure_hits[0U].entity_index = 77U;

    PreviewHostFrameRequest missing_command_request = request;
    missing_command_request.command_output.command = nullptr;
    missing_command_request.diagnostics =
        std::span<PreviewHostDiagnostic>(failure_diagnostics.data(), failure_diagnostics.size());
    missing_command_request.hit_records =
        std::span<PreviewHostHitRecord>(failure_hits.data(), failure_hits.size());
    PreviewHostFrameResult missing_command_result{};
    if (host.BuildFrame(missing_command_request, &missing_command_result) !=
        PreviewHostStatus::MissingCommandOutput) {
        return Fail("preview host did not report missing command output");
    }

    if (failure_diagnostics[0U].code != PreviewHostDiagnosticCode::MissingCommandOutput ||
        failure_diagnostics[0U].import_cook_missing_layer != RuntimeAssetImportCookMissingLayer::Command ||
        failure_hits[0U].entity_index != 77U) {
        return Fail("preview host missing command diagnostic changed");
    }

    std::array<RuntimeAssetFileDesc, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> invalid_cooked_files =
        context.fixture.cooked_files;
    invalid_cooked_files[0U].stable_id += 99U;
    PreviewHostCommandOutputRef invalid_command_output = command_output;
    invalid_command_output.cooked_files = std::span<const RuntimeAssetFileDesc>(
        invalid_cooked_files.data(),
        invalid_cooked_files.size());
    PreviewHostFrameRequest invalid_cooked_request = request;
    invalid_cooked_request.command_output = invalid_command_output;
    invalid_cooked_request.diagnostics =
        std::span<PreviewHostDiagnostic>(failure_diagnostics.data(), failure_diagnostics.size());
    PreviewHostFrameResult invalid_cooked_result{};
    if (host.BuildFrame(invalid_cooked_request, &invalid_cooked_result) !=
        PreviewHostStatus::InvalidCookedRecord) {
        return Fail("preview host did not report invalid cooked command record");
    }

    if (failure_diagnostics[0U].code != PreviewHostDiagnosticCode::InvalidCookedRecord ||
        failure_diagnostics[0U].import_cook_missing_layer != RuntimeAssetImportCookMissingLayer::RuntimeAssetData) {
        return Fail("preview host invalid cooked diagnostic changed");
    }

    RuntimeAssetImportCookCommandResult unsupported_command = context.fixture.command;
    unsupported_command.status = RuntimeAssetDataStatus::InvalidArgument;
    unsupported_command.missing_layer = RuntimeAssetImportCookMissingLayer::Resource;
    unsupported_command.fixture.status = RuntimeAssetDataStatus::InvalidArgument;
    PreviewHostCommandOutputRef unsupported_command_output = command_output;
    unsupported_command_output.command = &unsupported_command;
    PreviewHostFrameRequest unsupported_request = request;
    unsupported_request.command_output = unsupported_command_output;
    unsupported_request.diagnostics =
        std::span<PreviewHostDiagnostic>(failure_diagnostics.data(), failure_diagnostics.size());
    PreviewHostFrameResult unsupported_result{};
    if (host.BuildFrame(unsupported_request, &unsupported_result) !=
        PreviewHostStatus::UnsupportedBridgeLayer) {
        return Fail("preview host did not report unsupported command bridge layer");
    }

    if (failure_diagnostics[0U].code != PreviewHostDiagnosticCode::UnsupportedBridgeLayer ||
        failure_diagnostics[0U].import_cook_missing_layer != RuntimeAssetImportCookMissingLayer::Resource) {
        return Fail("preview host unsupported bridge diagnostic changed");
    }

    return 0;
}

int PreviewHostReportsBoundedResourceDiagnostics() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("PreviewHostDiagnostics"), &table)) {
        return Fail("preview diagnostics mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("preview diagnostics fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("preview diagnostics runtime asset graph load failed");
    }

    PreviewHost host;
    PreviewHostSessionResult scene_session{};
    if (host.StartSession(PreviewHostSessionDesc{PreviewHostDocumentKind::Scene}, &scene_session) !=
        PreviewHostStatus::Success) {
        return Fail("preview diagnostics scene session start failed");
    }

    std::array<PreviewHostDiagnostic, 2U> diagnostics{};
    std::array<PreviewHostHitRecord, 3U> hits{};
    hits[0U].entity_index = 99U;

    std::array<RuntimeAssetSceneResourceRef, FIXTURE_FILE_COUNT> type_mismatch_refs =
        graph.scene_resource_refs;
    type_mismatch_refs[0U].kind = RuntimeAssetFileKind::Texture;

    PreviewHostFrameRequest request{};
    request.session = scene_session.session;
    request.document_kind = PreviewHostDocumentKind::Scene;
    request.frame.frame_id = FRAME_ID + 201U;
    request.runtime_graph = &graph.load_result;
    request.scene_output = &graph.scene_output;
    request.loaded_files = std::span<const RuntimeAssetLoadedFile>(graph.assets.data(), graph.assets.size());
    request.resource_refs = std::span<const RuntimeAssetSceneResourceRef>(
        type_mismatch_refs.data(),
        type_mismatch_refs.size());
    request.scene_entities = std::span<const RuntimeAssetSceneEntityRecord>(
        graph.scene_entities.data(),
        graph.scene_entities.size());
    request.diagnostics = std::span<PreviewHostDiagnostic>(diagnostics.data(), diagnostics.size());
    request.hit_records = std::span<PreviewHostHitRecord>(hits.data(), hits.size());

    PreviewHostFrameResult frame_result{};
    if (host.BuildFrame(request, &frame_result) != PreviewHostStatus::TypeMismatch) {
        return Fail("preview host did not report type mismatch");
    }

    if (frame_result.diagnostic_count != 1U ||
        diagnostics[0U].code != PreviewHostDiagnosticCode::TypeMismatch ||
        diagnostics[0U].expected_kind != RuntimeAssetFileKind::Mesh) {
        return Fail("preview host type mismatch diagnostic changed");
    }

    if (hits[0U].entity_index != 99U) {
        return Fail("preview host mutated feedback output on type mismatch");
    }

    std::array<RuntimeAssetSceneResourceRef, FIXTURE_FILE_COUNT> missing_refs =
        graph.scene_resource_refs;
    missing_refs[0U].loaded_file_index = static_cast<std::uint32_t>(graph.assets.size() + 1U);
    request.resource_refs = std::span<const RuntimeAssetSceneResourceRef>(
        missing_refs.data(),
        missing_refs.size());
    if (host.BuildFrame(request, &frame_result) != PreviewHostStatus::MissingResourceRef) {
        return Fail("preview host did not report missing resource ref");
    }

    if (diagnostics[0U].code != PreviewHostDiagnosticCode::MissingResourceRef ||
        diagnostics[0U].resource_ref_index != 0U) {
        return Fail("preview host missing ref diagnostic changed");
    }

    PreviewHostSessionResult animation_session{};
    if (host.StartSession(PreviewHostSessionDesc{PreviewHostDocumentKind::Animation}, &animation_session) !=
        PreviewHostStatus::Success) {
        return Fail("preview diagnostics animation session start failed");
    }

    request.session = animation_session.session;
    request.document_kind = PreviewHostDocumentKind::Animation;
    request.resource_refs = std::span<const RuntimeAssetSceneResourceRef>(
        graph.scene_resource_refs.data(),
        graph.scene_resource_refs.size());
    if (host.BuildFrame(request, &frame_result) != PreviewHostStatus::UnsupportedDocumentKind) {
        return Fail("preview host did not report unsupported document kind");
    }

    if (diagnostics[0U].code != PreviewHostDiagnosticCode::UnsupportedDocumentKind) {
        return Fail("preview host unsupported diagnostic changed");
    }

    return 0;
}

int PreviewHostUsesResourceBrowserDiagnosticsForPreviewDecision() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("PreviewHostResourceBrowserDecision"), &table)) {
        return Fail("preview resource browser mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("preview resource browser fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("preview resource browser graph load failed");
    }

    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        descs[index] = files[index].desc;
    }

    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 8U> resource_browser_diagnostics{};
    ResourceBrowserDiagnosticsRequest diagnostics_request{};
    diagnostics_request.mount_table = &table;
    diagnostics_request.mount = MountId(MOUNT_ID);
    diagnostics_request.files = descs.data();
    diagnostics_request.file_count = static_cast<std::uint32_t>(descs.size());
    diagnostics_request.loaded_files = graph.assets.data();
    diagnostics_request.loaded_file_count = graph.load_result.loaded_file_count;
    diagnostics_request.resource_registry = &registry;
    diagnostics_request.asset_manager = &manager;
    diagnostics_request.entries = entries.data();
    diagnostics_request.entry_capacity = static_cast<std::uint32_t>(entries.size());
    diagnostics_request.diagnostics = resource_browser_diagnostics.data();
    diagnostics_request.diagnostic_capacity =
        static_cast<std::uint32_t>(resource_browser_diagnostics.size());

    ResourceBrowserDiagnosticsResult diagnostics_result{};
    if (BuildResourceBrowserRuntimeAssetDiagnostics(diagnostics_request, &diagnostics_result) !=
        ResourceBrowserDiagnosticsStatus::Success) {
        return Fail("resource browser diagnostics did not build for preview decision");
    }

    PreviewHost host;
    PreviewHostResourceBrowserPreviewRequest preview_request{};
    preview_request.entry = &entries[0U];
    preview_request.entry_index = 0U;
    preview_request.diagnostics = std::span<const ResourceBrowserDiagnosticRecord>(
        resource_browser_diagnostics.data(),
        diagnostics_result.diagnostic_count);

    PreviewHostResourceBrowserPreviewResult preview_result{};
    if (host.ResolveResourceBrowserPreview(preview_request, &preview_result) != PreviewHostStatus::Success) {
        return Fail("preview host rejected valid resource browser entry");
    }

    if (!preview_result.accepted_resource_browser_entry ||
        !preview_result.preview_eligible ||
        preview_result.document_kind != PreviewHostDocumentKind::Resource ||
        preview_result.resource_browser_diagnostic_count != 0U ||
        preview_result.used_locator_path_as_type_truth ||
        preview_result.diagnostic.from_resource_browser_diagnostics) {
        return Fail("preview host valid resource browser decision changed");
    }

    ResourceBrowserResourceEntry bad_entry = entries[0U];
    bad_entry.import_settings.source_path = "Mesh/LocatorStillOnly.yumesh";
    bad_entry.import_settings.target_kind = RuntimeAssetFileKind::Material;
    bad_entry.validation.status = RuntimeAssetDataStatus::InvalidSchema;
    bad_entry.validation.kind = RuntimeAssetFileKind::Mesh;
    bad_entry.dependency_state = ResourceBrowserDependencyState::StaleSchema;

    std::array<ResourceBrowserDiagnosticRecord, 1U> bad_diagnostics{};
    bad_diagnostics[0U].code = ResourceBrowserDiagnosticCode::StaleSchema;
    bad_diagnostics[0U].severity = ResourceBrowserDiagnosticSeverity::Error;
    bad_diagnostics[0U].phase = ResourceBrowserDiagnosticPhase::Validate;
    bad_diagnostics[0U].runtime_status = RuntimeAssetDataStatus::InvalidSchema;
    bad_diagnostics[0U].source_path = bad_entry.import_settings.source_path;
    bad_diagnostics[0U].expected_kind = RuntimeAssetFileKind::Material;
    bad_diagnostics[0U].file_index = 0U;

    preview_request.entry = &bad_entry;
    preview_request.entry_index = 0U;
    preview_request.diagnostics = std::span<const ResourceBrowserDiagnosticRecord>(
        bad_diagnostics.data(),
        bad_diagnostics.size());
    if (host.ResolveResourceBrowserPreview(preview_request, &preview_result) !=
        PreviewHostStatus::RuntimeAssetStatusFailed) {
        return Fail("preview host did not preserve resource browser validation blocker");
    }

    if (preview_result.preview_eligible ||
        preview_result.used_locator_path_as_type_truth ||
        preview_result.diagnostic.code != PreviewHostDiagnosticCode::RuntimeAssetStatusFailed ||
        preview_result.diagnostic.resource_browser_code != ResourceBrowserDiagnosticCode::StaleSchema ||
        preview_result.diagnostic.resource_browser_phase != ResourceBrowserDiagnosticPhase::Validate ||
        preview_result.diagnostic.runtime_asset_status != RuntimeAssetDataStatus::InvalidSchema ||
        preview_result.diagnostic.expected_kind != RuntimeAssetFileKind::Material ||
        preview_result.diagnostic.actual_kind != RuntimeAssetFileKind::Mesh ||
        !preview_result.diagnostic.from_resource_browser_diagnostics) {
        return Fail("preview host resource browser validation diagnostic lost lower detail");
    }

    ResourceBrowserResourceEntry not_loaded_entry = entries[1U];
    not_loaded_entry.from_runtime_asset_load = false;
    not_loaded_entry.from_resource_registry = false;
    not_loaded_entry.from_asset_record = false;
    preview_request.entry = &not_loaded_entry;
    preview_request.entry_index = 1U;
    preview_request.diagnostics = {};
    if (host.ResolveResourceBrowserPreview(preview_request, &preview_result) !=
        PreviewHostStatus::RuntimeAssetGraphStale) {
        return Fail("preview host did not reject missing loaded record");
    }

    if (preview_result.preview_eligible ||
        preview_result.diagnostic.code != PreviewHostDiagnosticCode::RuntimeAssetGraphStale ||
        preview_result.diagnostic.resource_browser_dependency_state != ResourceBrowserDependencyState::Ready) {
        return Fail("preview host missing loaded-record diagnostic changed");
    }

    return 0;
}

int PreviewHostRejectsStaleSessionWithoutMutation() {
    PreviewHost host;
    PreviewHostSessionResult start_result{};
    if (host.StartSession(PreviewHostSessionDesc{PreviewHostDocumentKind::Scene}, &start_result) !=
        PreviewHostStatus::Success) {
        return Fail("preview stale session start failed");
    }

    PreviewHostSessionResult stop_result{};
    if (host.StopSession(start_result.session, &stop_result) != PreviewHostStatus::Success) {
        return Fail("preview stale session stop failed");
    }

    std::array<PreviewHostDiagnostic, 1U> diagnostics{};
    std::array<PreviewHostHitRecord, 1U> hits{};
    hits[0U].entity_index = 77U;

    PreviewHostFrameRequest request{};
    request.session = start_result.session;
    request.document_kind = PreviewHostDocumentKind::Scene;
    request.frame.frame_id = FRAME_ID + 301U;
    request.diagnostics = std::span<PreviewHostDiagnostic>(diagnostics.data(), diagnostics.size());
    request.hit_records = std::span<PreviewHostHitRecord>(hits.data(), hits.size());

    PreviewHostFrameResult frame_result{};
    if (host.BuildFrame(request, &frame_result) != PreviewHostStatus::StaleSession) {
        return Fail("preview host did not reject stale session");
    }

    if (frame_result.diagnostic_count != 1U ||
        diagnostics[0U].status != PreviewHostStatus::StaleSession) {
        return Fail("preview stale session diagnostic changed");
    }

    if (hits[0U].entity_index != 77U) {
        return Fail("preview stale session mutated hit output");
    }

    return 0;
}

int PreviewHostReportsNotCookedRuntimeAssetRef() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("PreviewHostNotCooked"), &table)) {
        return Fail("preview not cooked mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("preview not cooked fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("preview not cooked runtime asset graph load failed");
    }

    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("preview not cooked rhi initialize failed");
    }

    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    if (!BuildPreviewHostSceneInputs(
            device,
            registry,
            manager,
            graph,
            &geometry,
            &material,
            &camera)) {
        return Fail("preview not cooked render inputs failed");
    }

    graph.assets[4U].decoded_payload_stored = false;

    PreviewHost host;
    PreviewHostSessionResult session_result{};
    if (host.StartSession(PreviewHostSessionDesc{PreviewHostDocumentKind::Scene}, &session_result) !=
        PreviewHostStatus::Success) {
        return Fail("preview not cooked session start failed");
    }

    std::array<PreviewHostDiagnostic, 2U> diagnostics{};
    PreviewHostFrameRequest request{};
    request.session = session_result.session;
    request.document_kind = PreviewHostDocumentKind::Scene;
    request.frame.frame_id = FRAME_ID + 401U;
    request.frame.format = PreviewHostFrameFormat::Headless;
    request.runtime_graph = &graph.load_result;
    request.scene_output = &graph.scene_output;
    request.loaded_files = std::span<const RuntimeAssetLoadedFile>(graph.assets.data(), graph.assets.size());
    request.resource_refs = std::span<const RuntimeAssetSceneResourceRef>(
        graph.scene_resource_refs.data(),
        graph.scene_resource_refs.size());
    request.scene_entities = std::span<const RuntimeAssetSceneEntityRecord>(
        graph.scene_entities.data(),
        graph.scene_entities.size());
    request.geometry_records = std::span<const RenderScenePrimitiveGeometryRecord>(
        geometry.data(),
        geometry.size());
    request.camera = camera;
    request.material = material;
    request.diagnostics = std::span<PreviewHostDiagnostic>(diagnostics.data(), diagnostics.size());

    PreviewHostFrameResult frame_result{};
    if (host.BuildFrame(request, &frame_result) != PreviewHostStatus::NotCooked) {
        std::fprintf(
            stderr,
            "preview not-cooked status=%u diagnostics=%zu code=%u expected=%u actual=%u ref=%u file=%u\n",
            static_cast<unsigned>(frame_result.status),
            frame_result.diagnostic_count,
            static_cast<unsigned>(diagnostics[0U].code),
            static_cast<unsigned>(diagnostics[0U].expected_kind),
            static_cast<unsigned>(diagnostics[0U].actual_kind),
            diagnostics[0U].resource_ref_index,
            diagnostics[0U].loaded_file_index);
        return Fail("preview host did not report not-cooked ref");
    }

    if (frame_result.diagnostic_count != 1U ||
        diagnostics[0U].code != PreviewHostDiagnosticCode::NotCooked ||
        diagnostics[0U].expected_kind != RuntimeAssetFileKind::Texture) {
        return Fail("preview host not-cooked diagnostic changed");
    }

    if (frame_result.submitted_render_scene_frame || frame_result.captured_through_render_core_rhi) {
        return Fail("preview host rendered after not-cooked ref");
    }

    return 0;
}

int PreviewHostBuildsViewportSessionSurfaceFromResourceBrowserSelection() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("PreviewHostViewportSurface"), &table)) {
        return Fail("preview viewport surface mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("preview viewport surface fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("preview viewport surface runtime asset graph load failed");
    }

    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("preview viewport surface rhi initialize failed");
    }

    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    if (!BuildPreviewHostSceneInputs(
            device,
            registry,
            manager,
            graph,
            &geometry,
            &material,
            &camera)) {
        return Fail("preview viewport surface render inputs failed");
    }

    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> rows{};
    ResourceBrowserSurfaceSelectionResult selection{};
    if (!BuildPreviewHostResourceBrowserSelection(
            table,
            registry,
            manager,
            graph,
            0U,
            {},
            &entries,
            &rows,
            &selection)) {
        return Fail("preview viewport surface resource browser selection failed");
    }

    if (selection.status != ResourceBrowserSurfaceSelectionStatus::Success ||
        !selection.state.preview_eligible ||
        selection.state.preview_state != ResourceBrowserSurfacePreviewState::Eligible) {
        return Fail("preview viewport surface selection was not eligible");
    }

    PreviewHost host;
    PreviewHostSessionResult session_result{};
    if (host.StartSession(PreviewHostSessionDesc{PreviewHostDocumentKind::Scene}, &session_result) !=
        PreviewHostStatus::Success) {
        return Fail("preview viewport surface session start failed");
    }

    std::array<PreviewHostDiagnostic, 4U> diagnostics{};
    std::array<PreviewHostHitRecord, 3U> hits{};
    std::array<PreviewHostSelectionRecord, 3U> selections{};
    std::array<PreviewHostTransformFeedback, 3U> transforms{};

    PreviewHostFrameRequest frame_request{};
    frame_request.session = session_result.session;
    frame_request.document_kind = PreviewHostDocumentKind::Scene;
    frame_request.frame.frame_id = FRAME_ID + 501U;
    frame_request.frame.width = 640U;
    frame_request.frame.height = 360U;
    frame_request.frame.format = PreviewHostFrameFormat::Headless;
    frame_request.camera_state.camera_id = 1U;
    frame_request.camera_state.orbit_angle_radians = 0.75F;
    frame_request.camera_state.orbit_radius = 4.0F;
    frame_request.camera_state.orbit_height = 1.5F;
    frame_request.runtime_graph = &graph.load_result;
    frame_request.scene_output = &graph.scene_output;
    frame_request.loaded_files =
        std::span<const RuntimeAssetLoadedFile>(graph.assets.data(), graph.assets.size());
    frame_request.resource_refs = std::span<const RuntimeAssetSceneResourceRef>(
        graph.scene_resource_refs.data(),
        graph.scene_resource_refs.size());
    frame_request.scene_entities = std::span<const RuntimeAssetSceneEntityRecord>(
        graph.scene_entities.data(),
        graph.scene_entities.size());
    frame_request.geometry_records =
        std::span<const RenderScenePrimitiveGeometryRecord>(geometry.data(), geometry.size());
    frame_request.camera = camera;
    frame_request.material = material;
    frame_request.diagnostics = std::span<PreviewHostDiagnostic>(diagnostics.data(), diagnostics.size());
    frame_request.hit_records = std::span<PreviewHostHitRecord>(hits.data(), hits.size());
    frame_request.selection_records =
        std::span<PreviewHostSelectionRecord>(selections.data(), selections.size());
    frame_request.transform_feedback =
        std::span<PreviewHostTransformFeedback>(transforms.data(), transforms.size());

    PreviewHostViewportSessionRequest viewport_request{};
    viewport_request.frame_request = frame_request;
    viewport_request.resource_browser_selection = &selection.state;
    viewport_request.selected_entity_index = 1U;
    viewport_request.require_selected_entity = true;

    PreviewHostViewportSessionResult viewport_result{};
    if (host.BuildViewportSessionSurface(viewport_request, &viewport_result) !=
        PreviewHostStatus::Success) {
        return Fail("preview viewport surface did not build");
    }

    if (!viewport_result.consumed_viewport_controls ||
        !viewport_result.consumed_resource_browser_selection ||
        !viewport_result.resource_browser_preview_eligible ||
        !viewport_result.resource_asset_mapping_preserved ||
        viewport_result.used_locator_path_as_type_truth ||
        !viewport_result.built_frame ||
        !viewport_result.frame.consumed_runtime_asset_graph ||
        !viewport_result.frame.submitted_render_scene_frame ||
        !viewport_result.frame.headless_output) {
        return Fail("preview viewport surface ledger was incomplete");
    }

    if (!viewport_result.selected_entity_available ||
        viewport_result.selected_entity_index != 1U ||
        viewport_result.viewport_width != 640U ||
        viewport_result.viewport_height != 360U ||
        viewport_result.camera_state.orbit_angle_radians != 0.75F ||
        viewport_result.camera_state.orbit_radius != 4.0F ||
        viewport_result.camera_state.orbit_height != 1.5F) {
        return Fail("preview viewport surface controls changed");
    }

    if (!viewport_result.emitted_hit_feedback ||
        !viewport_result.emitted_selection_feedback ||
        !viewport_result.emitted_transform_feedback ||
        viewport_result.frame.hit_record_count != 3U ||
        viewport_result.frame.selection_record_count != 3U ||
        viewport_result.frame.transform_feedback_count != 3U ||
        !hits[1U].hit_available ||
        !selections[1U].selectable ||
        !transforms[1U].transform_available) {
        return Fail("preview viewport surface feedback was not emitted");
    }

    return 0;
}

int PreviewHostConsumesResourceBrowserImporterCommitOutputs() {
    GeneratedFixtureCommandContext fixture{};
    if (!ExecuteGeneratedFixtureCommand("PreviewHostImporterCommit", &fixture)) {
        return Fail("preview importer commit fixture setup failed");
    }

    if (fixture.command.fixture.cooked_file_count !=
        RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT) {
        return Fail("preview importer commit fixture count changed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    const ResourceSnapshot resource_before = registry.Snapshot();
    const AssetSnapshot asset_before = manager.Snapshot();
    constexpr std::uint32_t SELECTED_TEXTURE = 4U;
    const RuntimeAssetFileDesc &selected_desc = fixture.cooked_files[SELECTED_TEXTURE];
    const ResourceBrowserExternalAuthoringSourceRow external =
        PreviewHostExternalSourceRowForCookedFile(selected_desc, SELECTED_TEXTURE);
    const std::array<ResourceBrowserExternalAuthoringSourceRow, 1U> external_rows{external};
    const ResourceBrowserImportSettings import_settings =
        PreviewHostImportSettingsForExternalRow(selected_desc, external);

    PreviewHostImporterCommitBuffers buffers{};
    const ResourceBrowserImporterCommitWorkflowResult commit_result =
        BuildPreviewHostImporterCommitWorkflow(
            fixture,
            registry,
            manager,
            SELECTED_TEXTURE,
            import_settings,
            std::span<const ResourceBrowserExternalAuthoringSourceRow>(
                external_rows.data(),
                external_rows.size()),
            &buffers);
    const ResourceSnapshot resource_after = registry.Snapshot();
    const AssetSnapshot asset_after = manager.Snapshot();
    if (!commit_result.Succeeded() ||
        commit_result.rejected_layer != ResourceBrowserImporterCommitRejectedLayer::None ||
        commit_result.loaded_file_count != fixture.command.fixture.cooked_file_count ||
        commit_result.graph_load_result.status != RuntimeAssetDataStatus::Success ||
        !commit_result.preflighted_before_mutation ||
        !commit_result.mutation_allowed ||
        !commit_result.mutated_runtime_state ||
        !commit_result.committed_resource_registry ||
        !commit_result.committed_asset_manager ||
        !commit_result.selection_committed ||
        commit_result.selection_rejected ||
        !commit_result.external_manifest_ready ||
        resource_after.registered_resource_count <= resource_before.registered_resource_count ||
        asset_after.active_asset_count <= asset_before.active_asset_count) {
        return Fail("preview importer commit did not produce committed runtime outputs");
    }

    if (buffers.catalog_rows[SELECTED_TEXTURE].source_boundary !=
            ResourceBrowserSourceBoundary::ExternalImportBoundary ||
        buffers.catalog_rows[SELECTED_TEXTURE].importer_readiness !=
            ResourceBrowserImporterReadiness::Ready ||
        buffers.catalog_rows[SELECTED_TEXTURE].asset_manager_gap !=
            ResourceBrowserAssetManagerGap::None ||
        !buffers.catalog_rows[SELECTED_TEXTURE].preview_request_ready ||
        !buffers.importer_rows[SELECTED_TEXTURE].external_import_boundary ||
        !buffers.importer_rows[SELECTED_TEXTURE].importer_ready ||
        buffers.asset_gap_rows[SELECTED_TEXTURE].gap != ResourceBrowserAssetManagerGap::None ||
        !buffers.asset_gap_rows[SELECTED_TEXTURE].asset_manager_ready ||
        !buffers.ledger[0U].selection_committed ||
        buffers.ledger[0U].selection_rejected ||
        !buffers.ledger[0U].external_manifest_ready ||
        !buffers.ledger[0U].mutated_runtime_state) {
        return Fail("preview importer commit row evidence mismatch");
    }

    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> rows{};
    ResourceBrowserSurfaceSelectionResult selection{};
    if (!BuildPreviewHostSelectionFromImporterCommit(
            buffers,
            commit_result,
            import_settings,
            SELECTED_TEXTURE,
            &rows,
            &selection)) {
        return Fail("preview importer commit selection build failed");
    }

    if (selection.status != ResourceBrowserSurfaceSelectionStatus::Success ||
        !selection.state.preview_eligible ||
        selection.state.preview_state != ResourceBrowserSurfacePreviewState::Eligible ||
        !selection.state.resource_asset_mapping_preserved ||
        selection.state.used_locator_path_as_type_truth) {
        return Fail("preview importer commit selection was not eligible");
    }

    LoadedGraph graph{};
    for (std::uint32_t index = 0U; index < commit_result.loaded_file_count; ++index) {
        graph.assets[index] = buffers.loaded_files[index];
    }

    for (std::uint32_t index = 0U; index < buffers.scene_output.resource_ref_count; ++index) {
        graph.scene_resource_refs[index] = buffers.scene_refs[index];
    }

    for (std::uint32_t index = 0U; index < buffers.scene_output.entity_count; ++index) {
        graph.scene_entities[index] = buffers.scene_entities[index];
    }

    graph.scene_output = buffers.scene_output;
    graph.load_result = commit_result.graph_load_result;

    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("preview importer commit rhi initialize failed");
    }

    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    if (!BuildPreviewHostSceneInputs(
            device,
            registry,
            manager,
            graph,
            &geometry,
            &material,
            &camera)) {
        return Fail("preview importer commit render inputs failed");
    }

    PreviewHost host;
    PreviewHostSessionResult session_result{};
    if (host.StartSession(PreviewHostSessionDesc{PreviewHostDocumentKind::Scene}, &session_result) !=
        PreviewHostStatus::Success) {
        return Fail("preview importer commit session start failed");
    }

    PreviewHostCommandOutputRef command_output{};
    command_output.command = &fixture.command;
    command_output.cooked_scene = &fixture.command.fixture.cooked_scene;
    command_output.cooked_files = std::span<const RuntimeAssetFileDesc>(
        fixture.cooked_files.data(),
        fixture.command.fixture.cooked_file_count);
    command_output.require_cooked_records = true;

    std::array<std::uint8_t, TOTAL_CAPTURE_BYTES> capture_bytes{};
    std::array<PreviewHostDiagnostic, 4U> diagnostics{};
    std::array<PreviewHostHitRecord, 3U> hits{};
    std::array<PreviewHostSelectionRecord, 3U> selections{};
    std::array<PreviewHostTransformFeedback, 3U> transforms{};
    constexpr std::string_view output_path = "Artifacts/PreviewHost/ImporterCommitViewport.rvf";

    PreviewHostFrameRequest frame_request{};
    frame_request.session = session_result.session;
    frame_request.document_kind = PreviewHostDocumentKind::Scene;
    frame_request.frame.frame_id = FRAME_ID + 601U;
    frame_request.frame.width = 2U;
    frame_request.frame.height = 2U;
    frame_request.frame.format = PreviewHostFrameFormat::Rgba8;
    frame_request.frame.capture_requested = true;
    frame_request.camera_state.camera_id = 1U;
    frame_request.camera_state.orbit_angle_radians = 0.85F;
    frame_request.camera_state.orbit_radius = 4.25F;
    frame_request.camera_state.orbit_height = 1.75F;
    frame_request.command_output = command_output;
    frame_request.runtime_graph = &commit_result.graph_load_result;
    frame_request.scene_output = &buffers.scene_output;
    frame_request.loaded_files = std::span<const RuntimeAssetLoadedFile>(
        buffers.loaded_files.data(),
        commit_result.loaded_file_count);
    frame_request.resource_refs = std::span<const RuntimeAssetSceneResourceRef>(
        buffers.scene_refs.data(),
        buffers.scene_output.resource_ref_count);
    frame_request.scene_entities = std::span<const RuntimeAssetSceneEntityRecord>(
        buffers.scene_entities.data(),
        buffers.scene_output.entity_count);
    frame_request.geometry_records =
        std::span<const RenderScenePrimitiveGeometryRecord>(geometry.data(), geometry.size());
    frame_request.camera = camera;
    frame_request.material = material;
    frame_request.rhi_device = &device;
    frame_request.output_path = output_path.data();
    frame_request.output_path_byte_count = output_path.size();
    frame_request.capture_output = std::span<std::uint8_t>(capture_bytes.data(), capture_bytes.size());
    frame_request.capture_byte_budget_per_entity = CAPTURE_BYTES_PER_ENTITY;
    frame_request.diagnostics = std::span<PreviewHostDiagnostic>(diagnostics.data(), diagnostics.size());
    frame_request.hit_records = std::span<PreviewHostHitRecord>(hits.data(), hits.size());
    frame_request.selection_records =
        std::span<PreviewHostSelectionRecord>(selections.data(), selections.size());
    frame_request.transform_feedback =
        std::span<PreviewHostTransformFeedback>(transforms.data(), transforms.size());

    PreviewHostViewportSessionRequest viewport_request{};
    viewport_request.frame_request = frame_request;
    viewport_request.resource_browser_selection = &selection.state;
    viewport_request.selected_entity_index = 2U;
    viewport_request.require_selected_entity = true;

    PreviewHostViewportSessionResult viewport_result{};
    if (host.BuildViewportSessionSurface(viewport_request, &viewport_result) !=
        PreviewHostStatus::Success) {
        return Fail("preview importer commit viewport did not build");
    }

    if (!viewport_result.consumed_viewport_controls ||
        !viewport_result.consumed_resource_browser_selection ||
        !viewport_result.resource_browser_preview_eligible ||
        !viewport_result.resource_asset_mapping_preserved ||
        viewport_result.used_locator_path_as_type_truth ||
        !viewport_result.built_frame ||
        !viewport_result.frame.consumed_import_cook_command_output ||
        !viewport_result.frame.consumed_runtime_asset_graph ||
        !viewport_result.frame.consumed_resource_refs ||
        !viewport_result.frame.submitted_render_scene_frame ||
        !viewport_result.frame.captured_through_render_core_rhi ||
        viewport_result.frame.capture_bytes_written == 0U) {
        return Fail("preview importer commit viewport ledger was incomplete");
    }

    if (viewport_result.frame.command_cooked_file_count != commit_result.loaded_file_count ||
        viewport_result.frame.submitted_entity_count != buffers.scene_output.entity_count ||
        viewport_result.frame.hit_record_count != buffers.scene_output.entity_count ||
        viewport_result.frame.selection_record_count != buffers.scene_output.entity_count ||
        viewport_result.frame.transform_feedback_count != buffers.scene_output.entity_count ||
        !viewport_result.selected_entity_available ||
        viewport_result.selected_entity_index != 2U ||
        !hits[2U].hit_available ||
        !selections[2U].selectable ||
        !transforms[2U].transform_available) {
        return Fail("preview importer commit viewport outputs changed");
    }

    return 0;
}

int PreviewHostRejectsBlockedViewportSelectionWithoutFrameMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("PreviewHostViewportBlocked"), &table)) {
        return Fail("preview blocked viewport mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("preview blocked viewport fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("preview blocked viewport runtime asset graph load failed");
    }

    std::array<ResourceBrowserDiagnosticRecord, 1U> blocking_diagnostics{};
    blocking_diagnostics[0U].code = ResourceBrowserDiagnosticCode::StaleSchema;
    blocking_diagnostics[0U].severity = ResourceBrowserDiagnosticSeverity::Error;
    blocking_diagnostics[0U].phase = ResourceBrowserDiagnosticPhase::Validate;
    blocking_diagnostics[0U].runtime_status = RuntimeAssetDataStatus::InvalidSchema;
    blocking_diagnostics[0U].expected_kind = RuntimeAssetFileKind::Mesh;
    blocking_diagnostics[0U].file_index = 0U;

    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> rows{};
    ResourceBrowserSurfaceSelectionResult selection{};
    if (!BuildPreviewHostResourceBrowserSelection(
            table,
            registry,
            manager,
            graph,
            0U,
            std::span<const ResourceBrowserDiagnosticRecord>(
                blocking_diagnostics.data(),
                blocking_diagnostics.size()),
            &entries,
            &rows,
            &selection)) {
        return Fail("preview blocked viewport resource browser selection failed");
    }

    if (selection.status != ResourceBrowserSurfaceSelectionStatus::PreviewBlocked ||
        selection.state.preview_state != ResourceBrowserSurfacePreviewState::BlockedByDiagnostic ||
        selection.state.matched_diagnostic_count != 1U) {
        return Fail("preview blocked viewport selection did not preserve blocker");
    }

    PreviewHost host;
    PreviewHostSessionResult session_result{};
    if (host.StartSession(PreviewHostSessionDesc{PreviewHostDocumentKind::Scene}, &session_result) !=
        PreviewHostStatus::Success) {
        return Fail("preview blocked viewport session start failed");
    }

    std::array<PreviewHostDiagnostic, 1U> diagnostics{};
    std::array<PreviewHostHitRecord, 1U> hits{};
    hits[0U].entity_index = 88U;

    PreviewHostFrameRequest frame_request{};
    frame_request.session = session_result.session;
    frame_request.document_kind = PreviewHostDocumentKind::Scene;
    frame_request.frame.frame_id = FRAME_ID + 601U;
    frame_request.frame.width = 320U;
    frame_request.frame.height = 180U;
    frame_request.camera_state.camera_id = 1U;
    frame_request.diagnostics = std::span<PreviewHostDiagnostic>(diagnostics.data(), diagnostics.size());
    frame_request.hit_records = std::span<PreviewHostHitRecord>(hits.data(), hits.size());

    PreviewHostViewportSessionRequest viewport_request{};
    viewport_request.frame_request = frame_request;
    viewport_request.resource_browser_selection = &selection.state;
    viewport_request.selected_entity_index = 0U;
    viewport_request.require_selected_entity = true;

    PreviewHostViewportSessionResult viewport_result{};
    if (host.BuildViewportSessionSurface(viewport_request, &viewport_result) !=
        PreviewHostStatus::RuntimeAssetStatusFailed) {
        return Fail("preview blocked viewport selection was not rejected");
    }

    if (viewport_result.built_frame ||
        viewport_result.frame.submitted_render_scene_frame ||
        viewport_result.frame.captured_through_render_core_rhi ||
        hits[0U].entity_index != 88U) {
        return Fail("preview blocked viewport mutated frame outputs");
    }

    if (!viewport_result.consumed_resource_browser_selection ||
        viewport_result.resource_browser_preview_eligible ||
        viewport_result.matched_resource_browser_diagnostic_count != 1U ||
        viewport_result.resource_browser_preview_state !=
            ResourceBrowserSurfacePreviewState::BlockedByDiagnostic ||
        diagnostics[0U].code != PreviewHostDiagnosticCode::RuntimeAssetStatusFailed ||
        diagnostics[0U].resource_browser_code != ResourceBrowserDiagnosticCode::StaleSchema ||
        !diagnostics[0U].from_resource_browser_diagnostics) {
        return Fail("preview blocked viewport diagnostic ledger changed");
    }

    return 0;
}

int PreviewHostBuildsViewportSessionFromSceneAuthoringDocument() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("PreviewHostSceneDocumentBridge"), &table)) {
        return Fail("preview scene document bridge mount setup failed");
    }

    if (!WriteCanonicalFixture(table)) {
        return Fail("preview scene document bridge fixture write failed");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadRuntimeAssetRecords(table, registry, manager, &graph)) {
        return Fail("preview scene document bridge runtime graph load failed");
    }

    RuntimeAssetRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("preview scene document bridge rhi initialize failed");
    }

    std::array<RenderScenePrimitiveGeometryRecord, 3U> geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    RenderSceneCameraBindingResult camera{};
    if (!BuildPreviewHostSceneInputs(
            device,
            registry,
            manager,
            graph,
            &geometry,
            &material,
            &camera)) {
        return Fail("preview scene document bridge render inputs failed");
    }

    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> rows{};
    ResourceBrowserSurfaceSelectionResult selection{};
    if (!BuildPreviewHostResourceBrowserSelection(
            table,
            registry,
            manager,
            graph,
            0U,
            {},
            &entries,
            &rows,
            &selection)) {
        return Fail("preview scene document bridge resource browser selection failed");
    }

    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 3U> identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 3U> transforms{};
    for (std::uint32_t index = 0U; index < graph.scene_output.entity_count; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = graph.scene_entities[index];
        identities[index].world_object_id = entity.world_object_id;
        identities[index].object_handle = ObjectHandleForWorldObject(entity.world_object_id);
        transforms[index].world_object_id = entity.world_object_id;
        transforms[index].transform_state = entity.transform;
        transforms[index].transform_state.translation_x += 10.0F + static_cast<float>(index);
        transforms[index].transform_state.rotation_y += 0.5F + static_cast<float>(index);
    }

    std::array<WorldSceneAuthoringDependencyRecord, 1U> dependencies{};
    dependencies[0U].stable_resource_id = graph.scene_resource_refs[0U].stable_id;
    dependencies[0U].resource_handle = graph.scene_resource_refs[0U].resource;
    dependencies[0U].expected_resource_type = ResourceTypeId{RESOURCE_TYPE_MESH};
    const WorldSceneAuthoringDocument document =
        MakePreviewHostSceneDocument(identities, transforms, dependencies);

    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 3U> output_identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 3U> output_transforms{};
    std::array<WorldSceneAuthoringDependencyRecord, 1U> output_dependencies{};
    std::uint32_t identity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t attachment_count = 0U;
    std::uint32_t binding_count = 0U;
    std::uint32_t dependency_count = 0U;
    WorldSceneAuthoringRuntimeExport runtime_export =
        MakePreviewHostSceneRuntimeExport(
            output_identities,
            &identity_count,
            output_transforms,
            &transform_count,
            &attachment_count,
            &binding_count,
            output_dependencies,
            &dependency_count);

    PreviewHost host;
    PreviewHostSessionResult session_result{};
    if (host.StartSession(PreviewHostSessionDesc{PreviewHostDocumentKind::Scene}, &session_result) !=
        PreviewHostStatus::Success) {
        return Fail("preview scene document bridge session start failed");
    }

    std::array<PreviewHostDiagnostic, 4U> diagnostics{};
    std::array<PreviewHostHitRecord, 3U> hits{};
    std::array<PreviewHostSelectionRecord, 3U> selections{};
    std::array<PreviewHostTransformFeedback, 3U> feedback{};
    PreviewHostFrameRequest frame_request{};
    frame_request.session = session_result.session;
    frame_request.document_kind = PreviewHostDocumentKind::Scene;
    frame_request.frame.frame_id = FRAME_ID + 701U;
    frame_request.frame.width = 800U;
    frame_request.frame.height = 450U;
    frame_request.frame.format = PreviewHostFrameFormat::Headless;
    frame_request.camera_state.camera_id = 1U;
    frame_request.camera_state.orbit_angle_radians = 1.25F;
    frame_request.camera_state.orbit_radius = 5.0F;
    frame_request.camera_state.orbit_height = 2.0F;
    frame_request.runtime_graph = &graph.load_result;
    frame_request.scene_output = &graph.scene_output;
    frame_request.loaded_files =
        std::span<const RuntimeAssetLoadedFile>(graph.assets.data(), graph.assets.size());
    frame_request.resource_refs = std::span<const RuntimeAssetSceneResourceRef>(
        graph.scene_resource_refs.data(),
        graph.scene_resource_refs.size());
    frame_request.scene_entities = std::span<const RuntimeAssetSceneEntityRecord>(
        graph.scene_entities.data(),
        graph.scene_entities.size());
    frame_request.geometry_records =
        std::span<const RenderScenePrimitiveGeometryRecord>(geometry.data(), geometry.size());
    frame_request.camera = camera;
    frame_request.material = material;
    frame_request.diagnostics = std::span<PreviewHostDiagnostic>(diagnostics.data(), diagnostics.size());
    frame_request.hit_records = std::span<PreviewHostHitRecord>(hits.data(), hits.size());
    frame_request.selection_records =
        std::span<PreviewHostSelectionRecord>(selections.data(), selections.size());
    frame_request.transform_feedback =
        std::span<PreviewHostTransformFeedback>(feedback.data(), feedback.size());

    PreviewHostViewportSessionRequest viewport_request{};
    viewport_request.frame_request = frame_request;
    viewport_request.resource_browser_selection = &selection.state;
    viewport_request.selected_entity_index = 2U;
    viewport_request.require_selected_entity = true;

    std::array<RuntimeAssetSceneEntityRecord, 3U> scene_entity_output{};
    for (std::uint32_t index = 0U; index < graph.scene_output.entity_count; ++index) {
        scene_entity_output[index] = graph.scene_entities[index];
    }

    PreviewHostSceneDocumentViewportRequest bridge_request{};
    bridge_request.scene_document = &document;
    bridge_request.runtime_export = runtime_export;
    bridge_request.viewport_request = viewport_request;
    bridge_request.scene_entity_output =
        std::span<RuntimeAssetSceneEntityRecord>(
            scene_entity_output.data(),
            graph.scene_output.entity_count);

    PreviewHostSceneDocumentViewportResult bridge_result{};
    if (host.BuildSceneDocumentViewportSession(bridge_request, &bridge_result) !=
        PreviewHostStatus::Success) {
        return Fail("preview scene document bridge did not build viewport session");
    }

    if (!bridge_result.consumed_scene_authoring_document ||
        !bridge_result.exported_runtime_records ||
        !bridge_result.consumed_runtime_scene_entities ||
        !bridge_result.updated_scene_entities_from_document ||
        !bridge_result.built_viewport_session ||
        !bridge_result.preserved_resource_browser_selection ||
        bridge_result.blocked_layer != PreviewHostSceneDocumentViewportBlockedLayer::None) {
        return Fail("preview scene document bridge ledger incomplete");
    }

    if (bridge_result.exported_identity_count != identities.size() ||
        bridge_result.exported_transform_count != transforms.size() ||
        bridge_result.exported_dependency_count != dependencies.size() ||
        bridge_result.updated_scene_entity_count != transforms.size()) {
        return Fail("preview scene document bridge exported counts changed");
    }

    if (!bridge_result.viewport.built_frame ||
        !bridge_result.viewport.frame.consumed_runtime_asset_graph ||
        !bridge_result.viewport.frame.submitted_render_scene_frame ||
        !bridge_result.viewport.frame.headless_output ||
        !bridge_result.viewport.emitted_transform_feedback ||
        bridge_result.viewport.selected_entity_index != 2U ||
        bridge_result.viewport.viewport_width != 800U ||
        bridge_result.viewport.viewport_height != 450U) {
        return Fail("preview scene document bridge viewport result changed");
    }

    for (std::uint32_t index = 0U; index < graph.scene_output.entity_count; ++index) {
        if (!Approx(scene_entity_output[index].transform.translation_x,
                transforms[index].transform_state.translation_x) ||
            !Approx(scene_entity_output[index].transform.rotation_y,
                transforms[index].transform_state.rotation_y) ||
            !feedback[index].transform_available ||
            !Approx(feedback[index].transform.translation_x,
                transforms[index].transform_state.translation_x) ||
            !hits[index].hit_available ||
            !selections[index].selectable) {
            return Fail("preview scene document bridge did not publish transformed entities");
        }
    }

    return 0;
}

int PreviewHostRejectsInvalidSceneAuthoringDocumentWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 2U> identities{};
    identities[0U].world_object_id = WorldObjectId{1U};
    identities[0U].object_handle = yuengine::object::ObjectHandle{1U, 9U};
    identities[1U].world_object_id = WorldObjectId{1U};
    identities[1U].object_handle = yuengine::object::ObjectHandle{2U, 9U};

    WorldSceneAuthoringDocument document{};
    document.header.scene_document_id = 0x830002U;
    document.header.deterministic_document_hash = 0x8300BBU;
    document.header.identity_record_count =
        static_cast<std::uint32_t>(identities.size());
    document.identity_records = identities.data();

    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 3U> output_identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 3U> output_transforms{};
    std::array<WorldSceneAuthoringDependencyRecord, 1U> output_dependencies{};
    output_identities[0U].world_object_id = WorldObjectId{90U};
    output_transforms[0U].world_object_id = WorldObjectId{91U};
    output_transforms[0U].transform_state.translation_x = 92.0F;
    output_dependencies[0U].stable_resource_id = 93U;
    std::uint32_t identity_count = AUTHORING_SENTINEL_COUNT;
    std::uint32_t transform_count = AUTHORING_SENTINEL_COUNT;
    std::uint32_t attachment_count = AUTHORING_SENTINEL_COUNT;
    std::uint32_t binding_count = AUTHORING_SENTINEL_COUNT;
    std::uint32_t dependency_count = AUTHORING_SENTINEL_COUNT;
    WorldSceneAuthoringRuntimeExport runtime_export =
        MakePreviewHostSceneRuntimeExport(
            output_identities,
            &identity_count,
            output_transforms,
            &transform_count,
            &attachment_count,
            &binding_count,
            output_dependencies,
            &dependency_count);

    std::array<RuntimeAssetSceneEntityRecord, 3U> scene_entity_output{};
    scene_entity_output[0U].entity_id = 800U;
    scene_entity_output[0U].world_object_id = WorldObjectId{801U};
    scene_entity_output[0U].transform.translation_x = 802.0F;
    std::array<PreviewHostHitRecord, 1U> hits{};
    hits[0U].entity_index = 88U;
    hits[0U].hit_available = false;
    std::array<PreviewHostTransformFeedback, 1U> feedback{};
    feedback[0U].world_object_id = WorldObjectId{89U};

    PreviewHostFrameRequest frame_request{};
    frame_request.hit_records = std::span<PreviewHostHitRecord>(hits.data(), hits.size());
    frame_request.transform_feedback =
        std::span<PreviewHostTransformFeedback>(feedback.data(), feedback.size());

    PreviewHostViewportSessionRequest viewport_request{};
    viewport_request.frame_request = frame_request;

    PreviewHostSceneDocumentViewportRequest bridge_request{};
    bridge_request.scene_document = &document;
    bridge_request.runtime_export = runtime_export;
    bridge_request.viewport_request = viewport_request;
    bridge_request.scene_entity_output =
        std::span<RuntimeAssetSceneEntityRecord>(
            scene_entity_output.data(),
            scene_entity_output.size());

    PreviewHost host;
    PreviewHostSceneDocumentViewportResult bridge_result{};
    if (host.BuildSceneDocumentViewportSession(bridge_request, &bridge_result) !=
        PreviewHostStatus::RuntimeAssetStatusFailed) {
        return Fail("preview scene document bridge accepted invalid authoring document");
    }

    if (bridge_result.scene_document_status !=
            WorldSceneAuthoringDocumentStatus::DuplicateIdentityWorldObjectId ||
        bridge_result.blocked_layer !=
            PreviewHostSceneDocumentViewportBlockedLayer::SceneAuthoringDocument ||
        bridge_result.built_viewport_session ||
        bridge_result.viewport.built_frame) {
        return Fail("preview scene document bridge failure ledger changed");
    }

    if (identity_count != AUTHORING_SENTINEL_COUNT ||
        transform_count != AUTHORING_SENTINEL_COUNT ||
        attachment_count != AUTHORING_SENTINEL_COUNT ||
        binding_count != AUTHORING_SENTINEL_COUNT ||
        dependency_count != AUTHORING_SENTINEL_COUNT ||
        output_identities[0U].world_object_id.value != 90U ||
        output_transforms[0U].world_object_id.value != 91U ||
        !Approx(output_transforms[0U].transform_state.translation_x, 92.0F) ||
        output_dependencies[0U].stable_resource_id != 93U) {
        return Fail("preview scene document bridge mutated runtime export on failure");
    }

    if (scene_entity_output[0U].entity_id != 800U ||
        scene_entity_output[0U].world_object_id.value != 801U ||
        !Approx(scene_entity_output[0U].transform.translation_x, 802.0F) ||
        hits[0U].entity_index != 88U ||
        hits[0U].hit_available ||
        feedback[0U].world_object_id.value != 89U ||
        feedback[0U].transform_available) {
        return Fail("preview scene document bridge mutated frame outputs on failure");
    }

    return 0;
}

const std::unordered_map<std::string_view, TestFunction> TESTS = {
    {TEST_GENERATOR, RuntimeAssetDataGeneratorWritesDeterministicFilesAndHashes},
    {TEST_IMPORT_COOK_COMMAND_WRITES, RuntimeAssetDataImportCookCommandWritesSourceAndCookedDiskFixtures},
    {TEST_IMPORT_COOK_COMMAND_LOADS, RuntimeAssetDataImportCookCommandLoadsGeneratedSourceAndCookedViaFileResourceRoute},
    {TEST_IMPORT_COOK_COMMAND_MISSING_LAYER, RuntimeAssetDataImportCookCommandReportsMissingLayerStatus},
    {TEST_UNSUPPORTED_VERSION, RuntimeAssetDataFormatHeaderRejectsUnsupportedVersion},
    {TEST_INVALID_BOUNDS, RuntimeAssetDataValidatorRejectsInvalidBoundsWithoutOutputs},
    {TEST_TYPED_MESH_MATERIAL_TEXTURE, RuntimeAssetDataMeshMaterialTextureTypedValidatorsAcceptStructuredMetadata},
    {TEST_MESH_PAYLOAD_POLICY, RuntimeAssetDataMeshPayloadPolicyRejectsSizeHashAndSplitMismatch},
    {TEST_MESH_LAYOUT_TOPOLOGY, RuntimeAssetDataMeshLayoutTopologyDecodesIntoLoadedRecords},
    {TEST_MESH_PAYLOAD_DECODED_BUFFERS, RuntimeAssetDataImportedMeshPayloadBytesFeedRenderGeometryBuffers},
    {TEST_MATERIAL_TYPED_REFS, RuntimeAssetDataMaterialValidatorRejectsMissingDuplicateAndTypeMismatchRefs},
    {TEST_MATERIAL_PARAMETER_SEMANTICS, RuntimeAssetDataMaterialParameterSemanticsLoadIntoRuntimeRecords},
    {TEST_MATERIAL_CONSTANT_PACK, RuntimeAssetDataMaterialConstantsPackLoadedParameters},
    {TEST_TEXTURE_TYPED_METADATA, RuntimeAssetDataTextureValidatorRejectsInvalidFormatExtentPayload},
    {TEST_SHADER_SCENE_ANIMATION_SCHEMA, RuntimeAssetDataShaderSceneAnimationRequireSourceSchema},
    {TEST_INVALID_DEPENDENCY, RuntimeAssetDataDependencyGraphRejectsMissingAndDuplicateRefs},
    {TEST_VALIDATOR_MISSING_DEPENDENCY_TOKEN, RuntimeAssetDataValidatorReportsMissingDependencyToken},
    {TEST_VALIDATOR_DUPLICATE_DEPENDENCY_TOKEN, RuntimeAssetDataValidatorReportsDuplicateDependencyToken},
    {TEST_VALIDATOR_TYPE_MISMATCH_EXPECTED_ACTUAL, RuntimeAssetDataValidatorReportsTypeMismatchExpectedActual},
    {TEST_COOKED_DEPENDENCY_FAILED_DEP_INDEX, RuntimeAssetDataCookedDependencyRowsReportFailedDepIndex},
    {TEST_SHADER_IMPORT_POLICY, RuntimeAssetDataShaderImportPolicyValidatesSourceCookedAndLoadedRecords},
    {TEST_SHADER_COMPILER_BACKEND, RuntimeAssetDataShaderCompilerBackendProducesProgramReflection},
    {TEST_SHADER_PROGRAM_PIPELINE_BRIDGE, RuntimeAssetDataShaderProgramBridgeCreatesRhiPipelineFromLoadedBytecode},
    {TEST_SHADER_PROGRAM_PIPELINE_REJECTS, RuntimeAssetDataShaderProgramBridgeRejectsInvalidProgramDataWithoutRhiMutation},
    {TEST_COOKED_SHADER_STAGE_MODULES, RuntimeAssetDataCookedShaderStagePayloadsCreateRhiModules},
    {TEST_COOKED_PROGRAM_PIPELINE_REFLECTION,
     RuntimeAssetDataCookedProgramPipelineUsesLoadedReflectionAndInputLayout},
    {TEST_COOKED_SHADER_PAYLOAD_REJECTS,
     RuntimeAssetDataCookedShaderPayloadRejectsStageBytecodeHashAndReflectionMismatchWithoutMutation},
    {TEST_COOKED_SHADER_PROGRAM_RHI_CLEANUP,
     RuntimeAssetDataCookedShaderProgramRhiPartialCreationFailureDestroysTransientHandles},
    {TEST_LOADER_FILE_RESOURCE, RuntimeAssetDataLoaderUsesFileResourcePathNotInMemoryStructs},
    {TEST_SCENE_REFERENCES, RuntimeAssetDataSceneReferencesMeshMaterialTextureShader},
    {TEST_CAMERA_TWEEN_DESCRIPTOR, RuntimeAssetDataCameraTweenDescriptorLoadsFromDiskSceneReference},
    {TEST_SCENE_FAMILY_PATH_INDEPENDENT, RuntimeAssetDataSceneFamilyDetectionIsPathIndependent},
    {TEST_SOURCE_COOKED_METADATA, RuntimeAssetDataSourceCookedParserReportsBoundedMetadata},
    {TEST_SOURCE_COOKED_REJECTS, RuntimeAssetDataSourceCookedParserRejectsInvalidTablesHashesAndDependencies},
    {TEST_HEADER_REJECTS_PARTIAL_VERSION, RuntimeAssetDataHeaderParserRejectsPartialVersionsAndNoise},
    {TEST_LOADER_REJECTS_SCHEMA_KIND_SUFFIX,
     RuntimeAssetDataLoaderRejectsSchemaKindAndMisleadingSuffixBeforeMutation},
    {TEST_LOADER_TRANSACTION_INVALID_SCHEMA, RuntimeAssetDataLoaderRejectsMissingSchemaBeforeMutation},
    {TEST_LOADER_TRANSACTION_COMMIT_FAILURE, RuntimeAssetDataLoaderCommitFailureReportsMutatedState},
    {TEST_LOADER_TRANSACTION_FILE_COUNT_PREFLIGHT,
     RuntimeAssetDataLoaderRejectsOversizedFileCountBeforeReadAndMutation},
    {TEST_SHADER_PROGRAM_DEPENDENCIES, RuntimeAssetDataShaderProgramDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs},
    {TEST_SCENE_CAMERA_ANIMATION_DEPENDENCIES, RuntimeAssetDataSceneCameraAnimationDependencyValidatorRejectsTypeMismatchWithoutMutation},
    {TEST_ANIMATION_DEPENDENCIES, RuntimeAssetDataAnimationDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs},
    {TEST_LOADED_RENDER_RECORDS, RuntimeAssetDataLoadCreatesRenderSceneRuntimeRecords},
    {TEST_GENERIC_RENDER_SCENE_SUBMISSION,
     RuntimeAssetDataGenericRenderSceneSubmissionBuildsFrameFromLoadedSceneRecords},
    {TEST_GENERIC_RENDER_SCENE_SUBMISSION_MESH_REFS,
     RuntimeAssetDataGenericRenderSceneSubmissionUsesMeshRefsNotEntityOrder},
    {TEST_GENERIC_RENDER_SCENE_SUBMISSION_MISSING_TRANSFORM,
     RuntimeAssetDataGenericRenderSceneSubmissionRejectsMissingTransformWithoutMutation},
    {TEST_GENERIC_RENDER_SCENE_SUBMISSION_MISSING_MESH,
     RuntimeAssetDataGenericRenderSceneSubmissionRejectsMissingMeshRefWithoutMutation},
    {TEST_GENERIC_RENDER_SCENE_SUBMISSION_MISSING_MATERIAL,
     RuntimeAssetDataGenericRenderSceneSubmissionRejectsMissingMaterialRefWithoutMutation},
    {TEST_GENERIC_RENDER_SCENE_SUBMISSION_MATERIAL_VARIANTS,
     RuntimeAssetDataGenericRenderSceneSubmissionReportsMaterialVariantsUntilFrameApiSupportsThem},
    {TEST_PRODUCTION_SCENE_LOADER_OUTPUT, RuntimeAssetDataProductionSceneLoaderOutputsDeterministicRecords},
    {TEST_DISK_ANIMATION_SAMPLING, RuntimeAssetDataDiskAnimationSamplingFeedsSceneTransforms},
    {TEST_SCENE_LOADER_INVALID_ENTITY_NO_MUTATION,
     RuntimeAssetDataSceneLoaderRejectsInvalidEntityWithoutOutputMutation},
    {TEST_SCENE_LOADER_INVALID_KEYFRAME_NO_MUTATION,
     RuntimeAssetDataSceneLoaderRejectsInvalidKeyframesWithoutOutputMutation},
    {TEST_SCENE_ANIMATION_BOUNDED_N_ENTITY,
     RuntimeAssetDataSceneAnimationLoaderLoadsBoundedNEntityScene},
    {TEST_SCENE_ANIMATION_CAPACITY_NO_MUTATION,
     RuntimeAssetDataSceneAnimationLoaderRejectsEntityCapacityOverflowWithoutMutation},
    {TEST_SCENE_ANIMATION_REFS_NO_MUTATION,
     RuntimeAssetDataSceneAnimationLoaderRejectsMissingRefsWithoutMutation},
    {TEST_SCENE_ANIMATION_INVALID_RECORDS_NO_MUTATION,
     RuntimeAssetDataSceneAnimationLoaderRejectsInvalidRecordsWithoutMutation},
    {TEST_SCENE_CAMERA_FAMILY_NO_MUTATION,
     RuntimeAssetDataSceneAnimationLoaderRejectsCameraFamilyFailuresWithoutMutation},
    {TEST_SCENE_ANIMATION_PATH_INDEPENDENT,
     RuntimeAssetDataSceneAnimationLoaderPathIndependentSceneAnimationDetection},
    {TEST_DECODED_PAYLOADS, RuntimeAssetDataCookStoresDecodedPayloadsForMeshMaterialTexture},
    {TEST_TEXTURE_MATERIAL_SLOT_BRIDGE, RuntimeAssetDataDecodedTexturePayloadsDriveRhiMaterialSlots},
    {TEST_TEXTURE_MATERIAL_SLOT_BRIDGE_FAILURES,
     RuntimeAssetDataTextureMaterialSlotBridgeFailuresDoNotMutateRenderSceneOutputs},
    {TEST_COOKED_TEXTURE_PAYLOAD_LAYOUT,
     RuntimeAssetDataCookedTexturePayloadTableValidatesLayoutHashAndRowPitch},
    {TEST_COOKED_MATERIAL_SLOT_TABLE,
     RuntimeAssetDataCookedMaterialTextureSlotTableResolvesLoadedPayloads},
    {TEST_COOKED_MATERIAL_CONSTANTS,
     RuntimeAssetDataCookedMaterialConstantsBridgeToRenderSceneRecord},
    {TEST_COOKED_MATERIAL_CONSTANT_REJECTS,
     RuntimeAssetDataCookedMaterialConstantsRejectInvalidLoadedMaterialWithoutMutation},
    {TEST_COOKED_PAYLOAD_DESCRIPTOR_REJECTS,
     RuntimeAssetDataCookedPayloadBridgeRejectsTextureFormatExtentSizeAlignmentHashWithoutMutation},
    {TEST_COOKED_SLOT_DEPENDENCY_REJECTS,
     RuntimeAssetDataCookedPayloadBridgeRejectsMissingDuplicateTypeMismatchDepsWithoutMutation},
    {TEST_COOKED_SLOT_OVERFLOW,
     RuntimeAssetDataCookedMaterialSlotOverflowDoesNotMutateRenderSceneOutputs},
    {TEST_COOKED_RHI_CLEANUP,
     RuntimeAssetDataCookedRhiPartialCreationFailureDestroysTransientHandles},
    {TEST_COOKED_VISUAL_PROOF,
     RuntimeAssetDataCookedRecordsDriveRuntimeVisualProofThroughRenderCoreRhi},
    {TEST_COOKED_VISUAL_PROOF_D3D11,
     RuntimeAssetDataD3D11HardwareCookedRecordsDriveDeviceBackedVisualProof},
    {TEST_COOKED_VISUAL_PROOF_MISSING_LAYERS,
     RuntimeAssetDataCookedRuntimeVisualProofReportsExactMissingLayers},
    {TEST_PACKAGE_COOK_RUN_SMOKE,
     RuntimeAssetDataPackageCookRunSmokeRunsPackagedRuntimeEntryPoint},
    {TEST_PACKAGE_ARTIFACT_PRODUCT_RUN,
     RuntimeAssetDataPackageArtifactCookRunSmokeRunsProductRuntimeEntryPoint},
    {TEST_PRODUCT_RUN_COMMAND, RuntimeAssetDataProductRunCommandConsumesPackageArtifactPath},
    {TEST_PRODUCT_RUN_COMMAND_MISSING_ARTIFACT,
     RuntimeAssetDataProductRunCommandReportsMissingPackageArtifactPath},
    {TEST_RUNTIME_DEPENDENCIES, RuntimeAssetDataLoadRegistersResourceAndAssetDependencyEdges},
    {TEST_LOAD_RENDER, RuntimeAssetDataRenderClosedLoopCapturesCubeCylinderConeThroughRhi},
    {TEST_CPU_ORACLE, RuntimeAssetDataCpuPpmOracleDoesNotBypassRhiRenderCore},
    {TEST_NO_UPPER, RuntimeAssetDataDoesNotDependOnEditorWebUiInputOrGdiViewer},
    {TEST_PREVIEW_HOST_CAPTURE, PreviewHostConsumesRuntimeAssetGraphAndCapturesThroughRhi},
    {TEST_PREVIEW_HOST_COMMAND_OUTPUT, PreviewHostConsumesImportCookCommandOutputs},
    {TEST_PREVIEW_HOST_DIAGNOSTICS, PreviewHostReportsBoundedResourceDiagnostics},
    {TEST_PREVIEW_HOST_RESOURCE_BROWSER_DECISION, PreviewHostUsesResourceBrowserDiagnosticsForPreviewDecision},
    {TEST_PREVIEW_HOST_STALE_SESSION, PreviewHostRejectsStaleSessionWithoutMutation},
    {TEST_PREVIEW_HOST_NOT_COOKED, PreviewHostReportsNotCookedRuntimeAssetRef},
    {TEST_PREVIEW_HOST_VIEWPORT_SURFACE, PreviewHostBuildsViewportSessionSurfaceFromResourceBrowserSelection},
    {TEST_PREVIEW_HOST_IMPORTER_COMMIT_VIEWPORT, PreviewHostConsumesResourceBrowserImporterCommitOutputs},
    {TEST_PREVIEW_HOST_VIEWPORT_SURFACE_BLOCKED,
     PreviewHostRejectsBlockedViewportSelectionWithoutFrameMutation},
    {TEST_PREVIEW_HOST_SCENE_DOCUMENT_BRIDGE,
     PreviewHostBuildsViewportSessionFromSceneAuthoringDocument},
    {TEST_PREVIEW_HOST_SCENE_DOCUMENT_BRIDGE_REJECTS,
     PreviewHostRejectsInvalidSceneAuthoringDocumentWithoutMutation},
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
