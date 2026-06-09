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

MissionEventThreadRuntimeReport runMissionEventThreadRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string missionEventThreadRuntimeReportToJson(const MissionEventThreadRuntimeReport& report);

MissionTutorialRuntimeReport runMissionTutorialRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string missionTutorialRuntimeReportToJson(const MissionTutorialRuntimeReport& report);

} // namespace yu::runtime
