#pragma once

#include "yuengine/runtime/SceneRuntime.h"

#include <filesystem>
#include <string>
#include <vector>

namespace yu::runtime {

struct RendererFrameContract {
    bool ready = false;
    std::string rendererProfile;
    int meshDrawCandidates = 0;
    int materialBindings = 0;
    int textureBindings = 0;
    int collisionDebugBatches = 0;
    int collisionTriangles = 0;
    int stageDependencyCount = 0;
};

struct ActorFrameContract {
    bool ready = false;
    std::string playerChara;
    std::string spawnPositionExpression;
    std::string spawnRotY;
    int actorInstances = 0;
};

struct CameraFrameContract {
    bool ready = false;
    std::string cameraSource;
    std::string defaultTarget;
    int railNodes = 0;
};

struct InputFrameContract {
    bool ready = false;
    std::string ownerService;
    std::string controlScope;
};

struct EventFrameContract {
    bool ready = false;
    std::string marker;
    int eventMarkers = 0;
};

struct FirstFrameRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool sceneRuntimeOk = false;
    RendererFrameContract renderer;
    ActorFrameContract actor;
    CameraFrameContract camera;
    InputFrameContract input;
    EventFrameContract event;
};

struct TitleUiRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool titleSetupFound = false;
    bool titleSetupExecuted = false;
    bool titleRenderExecuted = false;
    std::string module;
    std::string entryFunction;
    std::string scriptStatus;
    int scriptFunctions = 0;
    int scriptMethods = 0;
    int nativeObligations = 0;
    int nativeImplementedCalls = 0;
    int uniqueNativeApis = 0;
    int serviceStateEvents = 0;
    int uiObjectCalls = 0;
    int uiServiceCommands = 0;
    int unresolvedCalls = 0;
    bool truncated = false;
    int createdObjects = 0;
    int commandCount = 0;
    int drawCommands = 0;
    int graphStringCommands = 0;
    int stringSizeQueries = 0;
    int textDrawCommands = 0;
    int graphDrawCommands = 0;
    int colorCommands = 0;
    int localizedMenuTextCommands = 0;
    int drawListItemCommands = 0;
    int fontQueryCommands = 0;
    int fontScaleLimitCommands = 0;
    bool backgroundResourceBound = false;
    bool logoResourceBound = false;
    std::string lastCommand;
};

struct TitleBranchScenarioReport {
    std::string scenario;
    bool entryFound = false;
    bool executed = false;
    int menuSelectedIndex = 0;
    int scriptFunctions = 0;
    int scriptMethods = 0;
    int nativeObligations = 0;
    int uniqueNativeApis = 0;
    int serviceStateEvents = 0;
    int saveServiceQueries = 0;
    int platformStateQueries = 0;
    int audioServiceCommands = 0;
    int sceneServiceCommands = 0;
    int uiServiceCommands = 0;
    int uiObjectMutations = 0;
    int unresolvedCalls = 0;
    bool truncated = false;
    int saveListEntries = 0;
    int saveCapacityQueries = 0;
    int scenarioKeyGetQueries = 0;
    int makeNewGameCommands = 0;
    int loadAutoSaveCommands = 0;
    int startGameCommands = 0;
    int queuedStageLoads = 0;
    int shutdownPermissionQueries = 0;
    int shutdownGameCommands = 0;
    std::string startedMission;
    std::string startNewGame;
    std::string lastAutoSaveLoaded;
    std::string shutdownPermission;
    std::string shutdownRequested;
};

struct TitleBranchesRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    std::string module;
    std::string entryFunction;
    int scenarioCount = 0;
    int executedScenarios = 0;
    int unresolvedCalls = 0;
    bool truncated = false;
    int startGameScenarios = 0;
    int loadAutoSaveScenarios = 0;
    int makeNewGameScenarios = 0;
    int shutdownPermissionScenarios = 0;
    int shutdownGameScenarios = 0;
    int optionUiMutations = 0;
    std::vector<TitleBranchScenarioReport> scenarios;
};

struct GameplayFrameRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool sceneRuntimeOk = false;
    bool titleUiOk = false;
    bool titleBranchesOk = false;
    bool missionEventThreadOk = false;
    bool missionTutorialOk = false;
    int frameUpdates = 0;
    bool rendererFrameReady = false;
    bool uiFrameReady = false;
    bool saveFrameReady = false;
    bool actorFrameReady = false;
    bool cameraFrameReady = false;
    bool inputFrameReady = false;
    bool eventFrameReady = false;
    bool audioFrameReady = false;
    int meshDrawCandidates = 0;
    int materialBindings = 0;
    int textureBindings = 0;
    int titleUiCommands = 0;
    int titleUiDrawCommands = 0;
    int saveStartGameScenarios = 0;
    int saveLoadAutoSaveScenarios = 0;
    int saveMakeNewGameScenarios = 0;
    int actorInstances = 0;
    int playerControlCommands = 0;
    int cameraCommands = 0;
    int railNodes = 0;
    int eventCommands = 0;
    int tutorialUpdateCommands = 0;
    int audioCommands = 0;
    int gameplayCommandCount = 0;
};

struct RendererBackendSubmissionReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool sceneRuntimeOk = false;
    bool titleUiOk = false;
    bool gameplayFrameOk = false;
    std::string rendererProfile;
    bool backendFrameReady = false;
    bool titlePassReady = false;
    bool worldPassReady = false;
    bool resourceUploadReady = false;
    bool cameraSubmissionReady = false;
    bool actorSubmissionReady = false;
    bool eventSubmissionReady = false;
    int submissionPasses = 0;
    int backendCommandCount = 0;
    int drawSubmissions = 0;
    int resourceUploadSubmissions = 0;
    int title2dSubmissions = 0;
    int titleGraphSubmissions = 0;
    int titleTextSubmissions = 0;
    int sceneMeshSubmissions = 0;
    int materialBindings = 0;
    int textureBindings = 0;
    int collisionDebugSubmissions = 0;
    int cameraSubmissions = 0;
    int actorSubmissions = 0;
    int eventMarkerSubmissions = 0;
    std::string lastUiCommand;
    std::string stagePath;
    std::string cameraSource;
    std::string playerChara;
    std::vector<std::string> backendObligations;
};

struct FrameSchedulerNodeReport {
    std::string node;
    std::string service;
    bool ready = false;
    int commands = 0;
    std::string source;
};

struct FrameSchedulerRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool gameplayFrameOk = false;
    bool rendererSubmissionOk = false;
    bool updateGraphReady = false;
    int frameIndex = 0;
    int nodeCount = 0;
    int executedNodes = 0;
    int serviceNodeCount = 0;
    int schedulerEdges = 0;
    int gameplayCommands = 0;
    int rendererBackendCommands = 0;
    int scheduledWorkItems = 0;
    int backendObligations = 0;
    int unresolvedNodes = 0;
    std::vector<FrameSchedulerNodeReport> nodes;
};

struct BackendObligationItem {
    std::string obligation;
    std::string status;
    std::string evidence;
};

struct BackendObligationsRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool frameSchedulerOk = false;
    bool rendererSubmissionOk = false;
    bool textureUploadContractReady = false;
    bool materialBindingContractReady = false;
    bool shaderEffectContractTracked = false;
    bool fontContractTracked = false;
    bool deviceContractTracked = false;
    bool oracleParityContractTracked = false;
    int textureDependencies = 0;
    int textureBytesFound = 0;
    int ddsTextures = 0;
    int64_t textureByteTotal = 0;
    int materialBindings = 0;
    int meshSubmissions = 0;
    int titleTextSubmissions = 0;
    int resolvedBackendContracts = 0;
    int trackedBackendObligations = 0;
    int openBackendObligations = 0;
    std::vector<BackendObligationItem> obligations;
};

struct MaterialSemanticsMaterialReport {
    int index = -1;
    std::string name;
    int textureSlotCount = 0;
    int resolvedTextureSlots = 0;
    int meshBindingCount = 0;
    std::vector<std::string> textureSlots;
};

struct MaterialSemanticsRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool sceneRuntimeOk = false;
    bool rendererSubmissionOk = false;
    bool backendObligationsOk = false;
    bool materialSemanticsContractReady = false;
    bool textureSlotContractReady = false;
    bool meshMaterialContractReady = false;
    bool shaderEffectContractTracked = false;
    bool postEffectSourceTracked = false;
    std::string modelPath;
    int materials = 0;
    int materialParameterBlocks = 0;
    int textureSlots = 0;
    int resolvedTextureSlots = 0;
    int unresolvedTextureSlots = 0;
    int meshSubmissions = 0;
    int namedMeshSubmissions = 0;
    int meshMaterialBindings = 0;
    int unresolvedMeshMaterialBindings = 0;
    int postEffectTechniques = 0;
    int postEffectPasses = 0;
    int postEffectSamplers = 0;
    std::vector<MaterialSemanticsMaterialReport> materialReports;
    std::vector<BackendObligationItem> obligations;
};

struct DevicePresentationRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool frameSchedulerOk = false;
    bool rendererSubmissionOk = false;
    bool backendObligationsOk = false;
    bool materialSemanticsOk = false;
    bool deviceProfileReady = false;
    bool swapchainContractTracked = false;
    bool resourceUploadPlanReady = false;
    bool renderStateContractTracked = false;
    bool drawQueueContractReady = false;
    bool presentContractTracked = false;
    std::string rendererProfile;
    int backbufferWidthCandidate = 0;
    int backbufferHeightCandidate = 0;
    int rendererBackendCommands = 0;
    int resourceUploadSubmissions = 0;
    int drawSubmissions = 0;
    int title2dSubmissions = 0;
    int worldMeshSubmissions = 0;
    int materialTextureSlots = 0;
    int textureBytesFound = 0;
    int materialBindings = 0;
    int meshSubmissions = 0;
    int postEffectTechniques = 0;
    int postEffectPasses = 0;
    int postEffectSamplers = 0;
    int resolvedDeviceContracts = 0;
    int trackedDeviceObligations = 0;
    int openDeviceObligations = 0;
    std::vector<BackendObligationItem> contracts;
};

struct TextureUploadRecordReport {
    std::string path;
    std::string role;
    std::string format;
    bool found = false;
    bool validDds = false;
    bool cubeMap = false;
    bool compressedPayloadMatches = false;
    int width = 0;
    int height = 0;
    int mipCount = 0;
    int blockBytes = 0;
    int cubeFaces = 1;
    int materialConsumerCount = 0;
    int64_t byteSize = -1;
    int64_t payloadBytes = 0;
    int64_t expectedPayloadBytes = 0;
};

struct TextureUploadRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool sceneRuntimeOk = false;
    bool backendObligationsOk = false;
    bool materialSemanticsOk = false;
    bool devicePresentationOk = false;
    bool textureUploadRuntimeReady = false;
    bool ddsHeaderContractReady = false;
    bool payloadLayoutContractReady = false;
    bool materialConsumerContractReady = false;
    bool samplerStateGateTracked = false;
    bool blendDepthStateGateTracked = false;
    bool fontAtlasGateTracked = false;
    bool oracleParityGateTracked = false;
    int stageTextureDependencies = 0;
    int textureUploadRecords = 0;
    int ddsMagicRecords = 0;
    int validDdsHeaders = 0;
    int dxt1Textures = 0;
    int dxt5Textures = 0;
    int unsupportedTextureFormats = 0;
    int cubeMapTextures = 0;
    int cubeMapFaces = 0;
    int compressedPayloadMatches = 0;
    int materialSlotConsumers = 0;
    int uniqueMaterialTextureUploads = 0;
    int duplicateMaterialConsumers = 0;
    int stageOnlyTextureUploads = 0;
    int textureWidthMin = 0;
    int textureWidthMax = 0;
    int textureHeightMin = 0;
    int textureHeightMax = 0;
    int mip9Textures = 0;
    int mip10Textures = 0;
    int mip11Textures = 0;
    int postEffectSamplers = 0;
    int resourceUploadSubmissions = 0;
    int titleTextSubmissions = 0;
    int titleStringSizeQueries = 0;
    int localizedMenuTextCommands = 0;
    int64_t textureByteTotal = 0;
    int64_t payloadByteTotal = 0;
    int64_t expectedPayloadByteTotal = 0;
    int resolvedUploadContracts = 0;
    int trackedUploadObligations = 0;
    int openUploadObligations = 0;
    std::vector<TextureUploadRecordReport> records;
    std::vector<BackendObligationItem> contracts;
};

struct BackendSamplerStateRecord {
    std::string name;
    std::string texture;
    std::string addressU;
    std::string addressV;
    std::string addressW;
    std::string mipFilter;
    std::string minFilter;
    std::string magFilter;
    std::string srgbTexture;
    bool ready = false;
};

struct BackendPassStateRecord {
    std::string technique;
    std::string pass;
    std::string vertexShaderProfile;
    std::string pixelShaderProfile;
    std::string zEnable;
    std::string srgbWriteEnable;
    std::string alphaBlendEnable;
    std::string alphaTestEnable;
    std::string stencilEnable;
    std::string stencilPass;
    std::string stencilFunc;
    std::string stencilRef;
    bool ready = false;
};

struct BackendFontAtlasRecord {
    std::string source;
    int fontQueries = 0;
    int fontScaleLimits = 0;
    int textDrawCommands = 0;
    int graphStringCommands = 0;
    int stringSizeQueries = 0;
    int localizedMenuTextCommands = 0;
    int drawListItemCommands = 0;
    bool glyphMetricInputsReady = false;
    bool atlasImplementationTracked = false;
};

struct BackendStateRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool textureUploadOk = false;
    bool devicePresentationOk = false;
    bool materialSemanticsOk = false;
    bool titleUiOk = false;
    bool samplerStateRecordsReady = false;
    bool passRenderStateRecordsReady = false;
    bool fontAtlasRecordsReady = false;
    bool materialShaderProgramGateTracked = false;
    bool gpuStateBindingGateTracked = false;
    bool oracleParityGateTracked = false;
    bool backendStateRuntimeReady = false;
    int samplerStateRecords = 0;
    int samplerTextureBindings = 0;
    int samplerClampAddressRecords = 0;
    int samplerLinearMinFilters = 0;
    int samplerPointMinFilters = 0;
    int samplerSrgbTrueRecords = 0;
    int samplerSrgbFalseRecords = 0;
    int passStateRecords = 0;
    int passVs30Shaders = 0;
    int passPs30Shaders = 0;
    int zDisabledPasses = 0;
    int alphaBlendDisabledPasses = 0;
    int alphaTestDisabledPasses = 0;
    int srgbWriteEnabledPasses = 0;
    int srgbWriteDisabledPasses = 0;
    int stencilEnabledPasses = 0;
    int stencilDisabledPasses = 0;
    int stencilReplacePasses = 0;
    int stencilKeepPasses = 0;
    int stencilEqualPasses = 0;
    int fontQueryRecords = 0;
    int fontScaleLimitRecords = 0;
    int textDrawCommands = 0;
    int graphStringCommands = 0;
    int stringSizeQueries = 0;
    int localizedMenuTextCommands = 0;
    int drawListItemCommands = 0;
    int textureUploadRecords = 0;
    int materialTextureConsumers = 0;
    int resolvedBackendStateContracts = 0;
    int trackedBackendStateObligations = 0;
    int openBackendStateObligations = 0;
    std::vector<BackendSamplerStateRecord> samplerRecords;
    std::vector<BackendPassStateRecord> passRecords;
    BackendFontAtlasRecord fontRecord;
    std::vector<BackendObligationItem> contracts;
};

struct BackendResourceAllocationRecord {
    std::string name;
    std::string path;
    std::string resourceKind;
    std::string usage;
    std::string format;
    std::string status;
    int width = 0;
    int height = 0;
    int mipLevels = 0;
    int cubeFaces = 1;
    int materialConsumerCount = 0;
    int64_t byteSize = 0;
    int64_t payloadBytes = 0;
    int64_t expectedPayloadBytes = 0;
    bool ready = false;
};

struct BackendResourceAllocationRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool textureUploadOk = false;
    bool backendStateOk = false;
    bool stageTextureAllocationRecordsReady = false;
    bool smaaLookupAllocationRecordsReady = false;
    bool transientSurfaceAllocationGateTracked = false;
    bool fontAtlasAllocationGateTracked = false;
    bool d3dResourceCreationGateTracked = false;
    bool oracleParityGateTracked = false;
    bool resourceAllocationRuntimeReady = false;
    int allocationRecords = 0;
    int readyAllocationRecords = 0;
    int trackedOpenAllocationRecords = 0;
    int stageTextureAllocations = 0;
    int d3dDxt1Allocations = 0;
    int d3dDxt5Allocations = 0;
    int cubeTextureAllocations = 0;
    int smaaLookupAllocations = 0;
    int lookupL8Allocations = 0;
    int lookupA8L8Allocations = 0;
    int transientSurfaceCandidates = 0;
    int fontAtlasPlaceholders = 0;
    int samplerTextureDeclarations = 0;
    int materialTextureConsumers = 0;
    int64_t readyAllocationByteTotal = 0;
    int64_t readyAllocationPayloadBytes = 0;
    int64_t readyExpectedPayloadBytes = 0;
    int resolvedAllocationContracts = 0;
    int trackedAllocationObligations = 0;
    int openAllocationObligations = 0;
    std::vector<BackendResourceAllocationRecord> records;
    std::vector<BackendObligationItem> contracts;
};

struct BackendDeviceResourceExecutionRecord {
    std::string name;
    std::string path;
    std::string operation;
    std::string resourceKind;
    std::string format;
    std::string status;
    int width = 0;
    int height = 0;
    int mipLevels = 0;
    int cubeFaces = 1;
    int subresourceCount = 0;
    int materialBindingSlots = 0;
    int64_t byteSize = 0;
    int64_t payloadBytes = 0;
    bool ready = false;
};

struct BackendDeviceStateBindingRecord {
    std::string name;
    std::string operation;
    std::string target;
    std::string source;
    std::string status;
    int bindingSlots = 0;
    bool ready = false;
};

struct BackendDeviceExecutionRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool resourceAllocationOk = false;
    bool backendStateOk = false;
    bool devicePresentationOk = false;
    bool resourceCreationRecordsReady = false;
    bool uploadExecutionRecordsReady = false;
    bool stateBindingRecordsReady = false;
    bool lookupTextureBindingRecordsReady = false;
    bool materialTextureBindingGateTracked = false;
    bool transientSurfaceBindingGateTracked = false;
    bool fontAtlasExecutionGateTracked = false;
    bool d3dApiCallSubmissionGateTracked = false;
    bool presentOracleGateTracked = false;
    bool deviceExecutionRuntimeReady = false;
    int resourceCreationRecords = 0;
    int readyResourceCreationRecords = 0;
    int trackedOpenResourceCreationRecords = 0;
    int createTextureRecords = 0;
    int createCubeTextureRecords = 0;
    int renderTargetCreationCandidates = 0;
    int depthStencilCreationCandidates = 0;
    int fontAtlasCreationPlaceholders = 0;
    int textureUploadExecutionRecords = 0;
    int uploadSubresourceRecords = 0;
    int64_t readyUploadPayloadBytes = 0;
    int bindingRecords = 0;
    int readyBindingRecords = 0;
    int trackedOpenBindingRecords = 0;
    int materialTextureBindingRecords = 0;
    int materialTextureBindingSlots = 0;
    int samplerTextureBindingRecords = 0;
    int lookupTextureBindingRecords = 0;
    int transientSamplerBindingCandidates = 0;
    int samplerStateBindingRecords = 0;
    int renderStateBindingRecords = 0;
    int resolvedDeviceExecutionContracts = 0;
    int trackedDeviceExecutionObligations = 0;
    int openDeviceExecutionObligations = 0;
    std::vector<BackendDeviceResourceExecutionRecord> resourceRecords;
    std::vector<BackendDeviceStateBindingRecord> bindingRecordsDetail;
    std::vector<BackendObligationItem> contracts;
};

struct BackendPresentationOracleRecord {
    std::string name;
    std::string operation;
    std::string source;
    std::string status;
    int width = 0;
    int height = 0;
    int linkedRecordCount = 0;
    bool ready = false;
};

struct BackendPresentationOracleRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool deviceExecutionOk = false;
    bool devicePresentationOk = false;
    bool presentationOracleRuntimeReady = false;
    bool backbufferExtentRecordsReady = false;
    bool deviceExecutionInputRecordsReady = false;
    bool windowSurfaceGateTracked = false;
    bool swapchainCreationGateTracked = false;
    bool presentCallGateTracked = false;
    bool frameCaptureGateTracked = false;
    bool originalFrameOracleGateTracked = false;
    int presentationRecords = 0;
    int readyPresentationRecords = 0;
    int trackedOpenPresentationRecords = 0;
    int backbufferExtentRecords = 0;
    int backbufferWidth = 0;
    int backbufferHeight = 0;
    int deviceExecutionFrameInputs = 0;
    int linkedDeviceExecutionRecords = 0;
    int linkedDeviceReadyRecords = 0;
    int linkedDeviceOpenRecords = 0;
    int windowSurfaceCandidates = 0;
    int swapchainCreationCandidates = 0;
    int presentCallCandidates = 0;
    int frameCaptureCandidates = 0;
    int oracleTraceCandidates = 0;
    int rendererBackendCommands = 0;
    int drawSubmissions = 0;
    int resolvedPresentationContracts = 0;
    int trackedPresentationObligations = 0;
    int openPresentationObligations = 0;
    std::vector<BackendPresentationOracleRecord> records;
    std::vector<BackendObligationItem> contracts;
};

struct BackendPlatformBridgeCallRecord {
    std::string name;
    std::string api;
    std::string source;
    std::string status;
    int inputRecords = 0;
    int readyInputs = 0;
    int trackedOpenInputs = 0;
    int callCount = 0;
    int width = 0;
    int height = 0;
    bool ready = false;
};

struct BackendPlatformBridgeRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool deviceExecutionOk = false;
    bool presentationOracleOk = false;
    bool platformBridgeRuntimeReady = false;
    bool diagnosticBridgeReady = false;
    bool creationSubmissionQueueReady = false;
    bool uploadSubmissionQueueReady = false;
    bool stateSubmissionQueueReady = false;
    bool presentationSubmissionQueueTracked = false;
    bool d3dConcreteBackendGateTracked = false;
    bool frameCaptureGateTracked = false;
    bool originalOracleGateTracked = false;
    int bridgeCallRecords = 0;
    int readyBridgeCallRecords = 0;
    int trackedOpenBridgeCallRecords = 0;
    int diagnosticCallBatches = 0;
    int platformSurfaceRecords = 0;
    int d3dInterfaceRecords = 0;
    int createDeviceRecords = 0;
    int resourceCreationCallRecords = 0;
    int textureCreateCalls = 0;
    int cubeTextureCreateCalls = 0;
    int renderTargetCreateCandidates = 0;
    int depthStencilCreateCandidates = 0;
    int fontAtlasCreateCandidates = 0;
    int uploadCallRecords = 0;
    int uploadSubresourceCalls = 0;
    int stateBindingCallRecords = 0;
    int readyStateBindingCalls = 0;
    int trackedOpenStateBindingCalls = 0;
    int setTextureCalls = 0;
    int setSamplerStateCalls = 0;
    int setRenderStateBundleCalls = 0;
    int drawSubmissionCalls = 0;
    int presentCallRecords = 0;
    int captureOracleCallRecords = 0;
    int linkedDeviceExecutionRecords = 0;
    int linkedPresentationRecords = 0;
    int readyPlatformInputRecords = 0;
    int trackedOpenPlatformInputRecords = 0;
    int rendererBackendCommands = 0;
    int drawSubmissions = 0;
    int backbufferWidth = 0;
    int backbufferHeight = 0;
    int resolvedPlatformBridgeContracts = 0;
    int trackedPlatformBridgeObligations = 0;
    int openPlatformBridgeObligations = 0;
    std::vector<BackendPlatformBridgeCallRecord> records;
    std::vector<BackendObligationItem> contracts;
};

struct BackendExecutorResultRecord {
    std::string name;
    std::string sourceBridgeRecord;
    std::string api;
    std::string adapter;
    std::string resultStatus;
    std::string obligation;
    int inputRecords = 0;
    int callCount = 0;
    int executedCalls = 0;
    int preservedOpenCalls = 0;
    bool ready = false;
};

struct BackendExecutorRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool platformBridgeOk = false;
    bool executorRuntimeReady = false;
    bool diagnosticExecutorReady = false;
    bool oneToOneBridgeMappingReady = false;
    bool readyCallExecutionResultsReady = false;
    bool trackedOpenCallResultsReady = false;
    bool concreteD3DExecutionGateTracked = false;
    bool hwndDeviceGateTracked = false;
    bool drawExecutionGateTracked = false;
    bool presentExecutionGateTracked = false;
    bool frameCaptureGateTracked = false;
    bool originalOracleGateTracked = false;
    int executionResultRecords = 0;
    int diagnosticSuccessRecords = 0;
    int trackedOpenExecutionRecords = 0;
    int blockedExecutionRecords = 0;
    int consumedBridgeCallRecords = 0;
    int readyBridgeCallRecords = 0;
    int trackedOpenBridgeCallRecords = 0;
    int resultCallCountTotal = 0;
    int diagnosticExecutedCalls = 0;
    int preservedOpenCalls = 0;
    int submittedDiagnosticBatches = 0;
    int executedResourceCreationCalls = 0;
    int executedUploadSubresourceCalls = 0;
    int executedStateBindingCalls = 0;
    int preservedPlatformSurfaceGates = 0;
    int preservedDeviceCreationGates = 0;
    int preservedDrawSubmissionGates = 0;
    int preservedPresentGates = 0;
    int preservedCaptureOracleGates = 0;
    int linkedPlatformInputRecords = 0;
    int readyPlatformInputRecords = 0;
    int trackedOpenPlatformInputRecords = 0;
    int backbufferWidth = 0;
    int backbufferHeight = 0;
    int resolvedExecutorContracts = 0;
    int trackedExecutorObligations = 0;
    int openExecutorObligations = 0;
    std::vector<BackendExecutorResultRecord> results;
    std::vector<BackendObligationItem> contracts;
};

struct BackendDeviceAdapterRecord {
    std::string name;
    std::string sourceExecutorResult;
    std::string sourceBridgeRecord;
    std::string api;
    std::string adapter;
    std::string stage;
    std::string status;
    std::string obligation;
    int inputRecords = 0;
    int callCount = 0;
    int width = 0;
    int height = 0;
    int inheritedExecutedCalls = 0;
    int inheritedPreservedOpenCalls = 0;
    int realExecutedCalls = 0;
    int blockedRealCalls = 0;
    bool sourceReady = false;
    bool deviceHandleRequired = false;
    bool deviceHandleReady = false;
};

struct BackendDeviceAdapterRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool backendExecutorOk = false;
    bool deviceAdapterRuntimeReady = false;
    bool executorResultsConsumedReady = false;
    bool platformDevicePreconditionsTracked = false;
    bool downstreamExecutionBlockedUntilDevice = false;
    bool backbufferExtentCarried = false;
    bool realWindowSurfaceGateTracked = false;
    bool realD3DInterfaceGateTracked = false;
    bool realD3DDeviceGateTracked = false;
    bool realDeviceHandleReady = false;
    bool resourceExecutionRequiresDevice = false;
    bool drawPresentCaptureRequiresDevice = false;
    int adapterRecordCount = 0;
    int consumedExecutorResultRecords = 0;
    int sourceDiagnosticSuccessRecords = 0;
    int sourceTrackedOpenRecords = 0;
    int diagnosticContextRecords = 0;
    int platformDeviceAdapterRecords = 0;
    int windowSurfaceAdapterRecords = 0;
    int d3dInterfaceAdapterRecords = 0;
    int createDeviceAdapterRecords = 0;
    int downstreamBlockedRecords = 0;
    int resourceQueueBlockedRecords = 0;
    int renderQueueBlockedRecords = 0;
    int platformDevicePreconditionCalls = 0;
    int downstreamRealCallsBlockedUntilDevice = 0;
    int blockedRealCallsTotal = 0;
    int realExecutedCalls = 0;
    int realResourceCreationCallsBlocked = 0;
    int realUploadSubresourceCallsBlocked = 0;
    int realStateBindingCallsBlocked = 0;
    int realDrawCallsBlocked = 0;
    int realPresentCallsBlocked = 0;
    int realCaptureOracleCallsBlocked = 0;
    int inheritedDiagnosticExecutedCalls = 0;
    int inheritedPreservedOpenCalls = 0;
    int linkedPlatformInputRecords = 0;
    int readyPlatformInputRecords = 0;
    int trackedOpenPlatformInputRecords = 0;
    int backbufferWidth = 0;
    int backbufferHeight = 0;
    int resolvedDeviceAdapterContracts = 0;
    int trackedDeviceAdapterObligations = 0;
    int openDeviceAdapterObligations = 0;
    std::vector<BackendDeviceAdapterRecord> records;
    std::vector<BackendObligationItem> contracts;
};

struct BackendDeviceCreationExecutionRecord {
    std::string name;
    std::string sourceAdapterRecord;
    std::string sourceExecutorResult;
    std::string sourceBridgeRecord;
    std::string api;
    std::string status;
    std::string evidence;
    std::string detail;
    int width = 0;
    int height = 0;
    int attemptCount = 0;
    int successCount = 0;
    int failureCount = 0;
    bool resultRecorded = false;
    bool realHandleReady = false;
};

struct BackendDeviceCreationRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool deviceAdapterOk = false;
    bool deviceCreationRuntimeReady = false;
    bool adapterPreconditionsConsumedReady = false;
    bool platformExecutionAttempted = false;
    bool windowSurfaceResultRecorded = false;
    bool d3dInterfaceResultRecorded = false;
    bool d3dDeviceResultRecorded = false;
    bool downstreamQueuesPreserved = false;
    bool backbufferExtentCarried = false;
    bool platformSupported = false;
    bool realWindowSurfaceReady = false;
    bool realD3DInterfaceReady = false;
    bool realDeviceHandleReady = false;
    int executionResultRecords = 0;
    int adapterRecordCount = 0;
    int adapterPreconditionRecordsConsumed = 0;
    int sourceDownstreamBlockedRecords = 0;
    int downstreamRealCallsDeferred = 0;
    int windowSurfaceAttempts = 0;
    int d3dInterfaceAttempts = 0;
    int d3dDeviceAttempts = 0;
    int realSuccessRecords = 0;
    int realFailedRecords = 0;
    int blockedByDependencyRecords = 0;
    int linkedPlatformInputRecords = 0;
    int readyPlatformInputRecords = 0;
    int trackedOpenPlatformInputRecords = 0;
    int backbufferWidth = 0;
    int backbufferHeight = 0;
    int resolvedDeviceCreationContracts = 0;
    int trackedDeviceCreationObligations = 0;
    int openDeviceCreationObligations = 0;
    std::vector<BackendDeviceCreationExecutionRecord> records;
    std::vector<BackendObligationItem> contracts;
};

struct BackendResourceCreationExecutionRecord {
    std::string name;
    std::string sourceResourceRecord;
    std::string operation;
    std::string resourceKind;
    std::string format;
    std::string status;
    std::string detail;
    int width = 0;
    int height = 0;
    int mipLevels = 0;
    int cubeFaces = 1;
    int subresourceCount = 0;
    int64_t payloadBytes = 0;
    bool sourceReady = false;
    bool resultRecorded = false;
    bool realHandleReady = false;
};

struct BackendResourceCreationRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool deviceCreationOk = false;
    bool deviceExecutionOk = false;
    bool resourceCreationRuntimeReady = false;
    bool persistentDeviceServiceReady = false;
    bool readyResourceCreationExecuted = false;
    bool trackedOpenResourcesPreserved = false;
    bool downstreamExecutionDeferred = false;
    bool backbufferExtentCarried = false;
    int sourceResourceRecords = 0;
    int readySourceResourceRecords = 0;
    int trackedOpenSourceResourceRecords = 0;
    int resultRecords = 0;
    int realResourceHandlesCreated = 0;
    int failedResourceHandles = 0;
    int preservedTrackedOpenResources = 0;
    int createTextureResults = 0;
    int createCubeTextureResults = 0;
    int smaaLookupTextureResults = 0;
    int stageTextureResults = 0;
    int deferredRenderTargetCandidates = 0;
    int deferredDepthStencilCandidates = 0;
    int deferredFontAtlasCandidates = 0;
    int uploadSubresourceRecordsDeferred = 0;
    int stateBindingRecordsDeferred = 0;
    int drawPresentCaptureRecordsDeferred = 0;
    int64_t readyPayloadBytesCreated = 0;
    int backbufferWidth = 0;
    int backbufferHeight = 0;
    int resolvedResourceCreationContracts = 0;
    int trackedResourceCreationObligations = 0;
    int openResourceCreationObligations = 0;
    std::vector<BackendResourceCreationExecutionRecord> records;
    std::vector<BackendObligationItem> contracts;
};

struct MissionEventThreadRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool sceneRuntimeOk = false;
    bool eventThreadFound = false;
    bool eventThreadExecuted = false;
    std::string module;
    std::string entryFunction;
    std::string scriptStatus;
    int scriptFunctions = 0;
    int scriptMethods = 0;
    int nativeObligations = 0;
    int nativeImplementedCalls = 0;
    int uniqueNativeApis = 0;
    int serviceStateEvents = 0;
    int engineObjectCalls = 0;
    int valueMethodCalls = 0;
    int unresolvedCalls = 0;
    bool truncated = false;
    int playerControlCommands = 0;
    std::string playerControlEnabled;
    int resetMenuButtonHoldingTimesCommands = 0;
    int dialogResetCommands = 0;
    int dialogHideCommands = 0;
    int resetPlayerActionCommands = 0;
    int eventUnitQueries = 0;
    int eventPageSetupCommands = 0;
    int eventPageDoneCommands = 0;
    int eventVolumeCreates = 0;
    int eventVolumeActivationCommands = 0;
    std::string lastEventVolumeEnabled;
    int setGameCameraIfNotCommands = 0;
    std::string gameCameraIfNotTarget;
};

struct MissionTutorialRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool sceneRuntimeOk = false;
    bool tutorialThreadFound = false;
    bool tutorialThreadExecuted = false;
    bool updateUnitsExecuted = false;
    std::string module;
    std::string entryFunction;
    std::string scriptStatus;
    int scriptFunctions = 0;
    int scriptMethods = 0;
    int nativeObligations = 0;
    int nativeImplementedCalls = 0;
    int uniqueNativeApis = 0;
    int serviceStateEvents = 0;
    int engineObjectCalls = 0;
    int valueMethodCalls = 0;
    int unresolvedCalls = 0;
    bool truncated = false;
    int eventFlagAddCommands = 0;
    int currentPlayerNameQueries = 0;
    int getPlayerQueries = 0;
    int getPlayerControlQueries = 0;
    int dialogShowCommands = 0;
    int dialogSpeakCommands = 0;
    int dialogWaitCommands = 0;
    int dialogHideCommands = 0;
    int tutorialActorCreates = 0;
    int tutorialPageCreates = 0;
    int pushActorCommands = 0;
    int waitActorCommands = 0;
    int playerControlCommands = 0;
    std::string playerControlEnabled;
    int setPlayerAngleYCommands = 0;
    int landPlayerCommands = 0;
    int updateUnitsCommands = 0;
    int enterTransitionCommands = 0;
    int leaveTransitionCommands = 0;
    std::string lastTutorialPage;
    std::string lastDialogText;
};

FirstFrameRuntimeReport buildFirstFrameRuntimeReport(
    const SceneRuntimeMaterializationReport& sceneRuntime,
    const std::string& rendererProfile);

FirstFrameRuntimeReport runFirstFrameRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string firstFrameRuntimeReportToJson(const FirstFrameRuntimeReport& report);

TitleUiRuntimeReport runTitleUiRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string titleUiRuntimeReportToJson(const TitleUiRuntimeReport& report);

TitleBranchesRuntimeReport runTitleBranchesRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string titleBranchesRuntimeReportToJson(const TitleBranchesRuntimeReport& report);

GameplayFrameRuntimeReport runGameplayFrameRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string gameplayFrameRuntimeReportToJson(const GameplayFrameRuntimeReport& report);

RendererBackendSubmissionReport runRendererBackendSubmissionRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string rendererBackendSubmissionReportToJson(const RendererBackendSubmissionReport& report);

FrameSchedulerRuntimeReport runFrameSchedulerRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string frameSchedulerRuntimeReportToJson(const FrameSchedulerRuntimeReport& report);

BackendObligationsRuntimeReport runBackendObligationsRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string backendObligationsRuntimeReportToJson(const BackendObligationsRuntimeReport& report);

MaterialSemanticsRuntimeReport runMaterialSemanticsRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string materialSemanticsRuntimeReportToJson(const MaterialSemanticsRuntimeReport& report);

DevicePresentationRuntimeReport runDevicePresentationRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string devicePresentationRuntimeReportToJson(const DevicePresentationRuntimeReport& report);

TextureUploadRuntimeReport runTextureUploadRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string textureUploadRuntimeReportToJson(const TextureUploadRuntimeReport& report);

BackendStateRuntimeReport runBackendStateRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string backendStateRuntimeReportToJson(const BackendStateRuntimeReport& report);

BackendResourceAllocationRuntimeReport runBackendResourceAllocationRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string backendResourceAllocationRuntimeReportToJson(
    const BackendResourceAllocationRuntimeReport& report);

BackendDeviceExecutionRuntimeReport runBackendDeviceExecutionRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string backendDeviceExecutionRuntimeReportToJson(
    const BackendDeviceExecutionRuntimeReport& report);

BackendPresentationOracleRuntimeReport runBackendPresentationOracleRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string backendPresentationOracleRuntimeReportToJson(
    const BackendPresentationOracleRuntimeReport& report);

BackendPlatformBridgeRuntimeReport runBackendPlatformBridgeRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string backendPlatformBridgeRuntimeReportToJson(
    const BackendPlatformBridgeRuntimeReport& report);

BackendExecutorRuntimeReport runBackendExecutorRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string backendExecutorRuntimeReportToJson(
    const BackendExecutorRuntimeReport& report);

BackendDeviceAdapterRuntimeReport runBackendDeviceAdapterRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string backendDeviceAdapterRuntimeReportToJson(
    const BackendDeviceAdapterRuntimeReport& report);

BackendDeviceCreationRuntimeReport runBackendDeviceCreationRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string backendDeviceCreationRuntimeReportToJson(
    const BackendDeviceCreationRuntimeReport& report);

BackendResourceCreationRuntimeReport runBackendResourceCreationRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string backendResourceCreationRuntimeReportToJson(
    const BackendResourceCreationRuntimeReport& report);

MissionEventThreadRuntimeReport runMissionEventThreadRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string missionEventThreadRuntimeReportToJson(const MissionEventThreadRuntimeReport& report);

MissionTutorialRuntimeReport runMissionTutorialRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string missionTutorialRuntimeReportToJson(const MissionTutorialRuntimeReport& report);

} // namespace yu::runtime
