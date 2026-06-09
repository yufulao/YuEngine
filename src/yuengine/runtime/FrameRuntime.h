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

MissionEventThreadRuntimeReport runMissionEventThreadRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string missionEventThreadRuntimeReportToJson(const MissionEventThreadRuntimeReport& report);

MissionTutorialRuntimeReport runMissionTutorialRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string missionTutorialRuntimeReportToJson(const MissionTutorialRuntimeReport& report);

} // namespace yu::runtime
