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

MissionEventThreadRuntimeReport runMissionEventThreadRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string missionEventThreadRuntimeReportToJson(const MissionEventThreadRuntimeReport& report);

MissionTutorialRuntimeReport runMissionTutorialRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string missionTutorialRuntimeReportToJson(const MissionTutorialRuntimeReport& report);

} // namespace yu::runtime
