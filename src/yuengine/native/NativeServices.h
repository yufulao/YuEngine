#pragma once

#include "yuengine/native/NativeRegistry.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace yu::native {

struct NativeCallContext {
    std::string module;
    std::string function;
    int sourceLine = -1;
    int pc = -1;
};

struct NativeDispatchResult {
    std::string api;
    std::string service;
    std::string ownerLevel;
    std::string implementationStatus;
    std::string evidence;
    std::string obligation;
    bool apiKnown = false;
    bool serviceBound = false;
    bool implemented = false;
};

struct NativeServiceStateMutation {
    std::string service;
    std::string api;
    std::string action;
    std::string target;
    std::string value;
    std::string arguments;
};

struct SaveProfileScenarioRuntimeState {
    int emptySaveListQueries = 0;
    int saveListCountQueries = 0;
    int saveListGetQueries = 0;
    int saveEntryActiveQueries = 0;
    int saveListEntries = 0;
    int saveCapacityQueries = 0;
    int scenarioKeyQueries = 0;
    int scenarioKeyCountQueries = 0;
    int scenarioKeyGetQueries = 0;
    int makeNewGameCommands = 0;
    int loadAutoSaveCommands = 0;
    int startGameCommands = 0;
    std::string currentMissionKey;
    std::string currentMission;
    std::string startedMission;
    std::string difficultyMode;
    std::string startNewGame;
    std::string lastAutoSaveLoaded;
};

struct PlatformRuntimeState {
    int resetMenuButtonHoldingTimesCommands = 0;
    int shutdownPermissionQueries = 0;
    int shutdownGameCommands = 0;
    std::string shutdownPermission;
    std::string shutdownRequested;
    std::map<std::string, std::string> flags;
};

struct AudioRuntimeState {
    int playBgmCommands = 0;
    int fadeOutBgmCommands = 0;
    std::string currentBgmId;
    std::string bgmFadeOutDuration;
};

struct SceneStageRuntimeState {
    int fadeInCommands = 0;
    int fadeOutCommands = 0;
    int enterTransitionCommands = 0;
    int leaveTransitionCommands = 0;
    int queuedStageLoads = 0;
    int loaderCommands = 0;
    int loadedStageCommands = 0;
    int loadedEventScriptCommands = 0;
    int callSetupEventsCommands = 0;
    int charaPlaceLoads = 0;
    int placeParamQueries = 0;
    std::string fadeInDuration;
    std::string fadeInBlend;
    std::string currentMissionScript;
    std::string currentStage;
    std::string currentRailCamera;
    std::string activeLoader;
    std::string currentEventScript;
    std::string currentCharaPlace;
};

struct ActorTaskRuntimeState {
    int pushPlayerCharaCommands = 0;
    int actorMethodCommands = 0;
    int playerControlCommands = 0;
    int setPlayerPosCommands = 0;
    int setPlayerAngleYCommands = 0;
    int landPlayerCommands = 0;
    int getPlayerPosQueries = 0;
    int currentPlayerNameQueries = 0;
    int getPlayerQueries = 0;
    int getPlayerControlQueries = 0;
    int resetPlayerActionCommands = 0;
    int tutorialActorCreates = 0;
    int tutorialPageCreates = 0;
    int pushActorCommands = 0;
    int waitActorCommands = 0;
    std::string currentPlayerChara;
    std::string currentPlayerName;
    std::string currentPlayerPosition;
    std::string currentPlayerRotY;
    std::string playerControlEnabled;
    std::string lastTutorialActor;
    std::string lastTutorialPage;
    std::string playerLanded;
    std::string waitForLanding;
    std::string healProgressFilled;
    std::string armed;
    std::string lastEffect;
};

struct CameraRuntimeState {
    int pushGameCameraCommands = 0;
    int railCameraLoads = 0;
    int enableRailCameraCommands = 0;
    int enableAutoAdjustCommands = 0;
    int defaultCameraStateCommands = 0;
    int setGameCameraIfNotCommands = 0;
    std::string gameCameraPushed;
    std::string railCameraPath;
    std::string railCameraEnabled;
    std::string autoCameraAdjustEnabled;
    std::string defaultCameraStateTarget;
    std::string gameCameraIfNotTarget;
};

struct CollisionPhysicsLiteRuntimeState {
    int eventVolumeCreates = 0;
    int eventVolumeActivationCommands = 0;
    std::string lastEventVolume;
    std::string lastEventVolumeEnabled;
};

struct EventQuestFlagRuntimeState {
    int clearCurrentQuestCommands = 0;
    int missionRequestQueries = 0;
    int markerQueries = 0;
    int checkpointCommands = 0;
    int checkFallCommands = 0;
    int eventClassCreates = 0;
    int eventUnitQueries = 0;
    int eventPageCreates = 0;
    int eventMarkerCreates = 0;
    int eventActorCreates = 0;
    int eventPageSetupCommands = 0;
    int eventPageDoneCommands = 0;
    int eventFlagQueries = 0;
    int eventFlagInitCommands = 0;
    int eventFlagAddCommands = 0;
    int clearEventsAllCommands = 0;
    int eventUnitDeployCommands = 0;
    int updateUnitsCommands = 0;
    int talkBranchCommands = 0;
    int waitForCommands = 0;
    int dialogResetCommands = 0;
    int dialogHideCommands = 0;
    int dialogShowCommands = 0;
    int dialogSpeakCommands = 0;
    int dialogWaitCommands = 0;
    int dialogBgQueries = 0;
    int dialogCommandCount = 0;
    std::string currentRequest;
    std::string currentMarker;
    std::string currentCheckpoint;
    std::string currentEventUnit;
    std::string currentEventPage;
    std::string currentEventActor;
    std::string lastDialogText;
};

struct UiRuntimeObjectState {
    std::string className;
    int commandCount = 0;
    std::vector<std::string> commands;
};

struct UiRender2dRuntimeState {
    int createdObjects = 0;
    int commandCount = 0;
    int drawCommands = 0;
    int graphStringCommands = 0;
    int stringSizeQueries = 0;
    int textDrawCommands = 0;
    int frameDrawCommands = 0;
    int rectDrawCommands = 0;
    int graphDrawCommands = 0;
    int colorCommands = 0;
    std::string lastCommand;
    std::map<std::string, UiRuntimeObjectState> objects;
};

struct NativeRuntimeServiceState {
    int mutations = 0;
    SaveProfileScenarioRuntimeState saveProfileScenario;
    PlatformRuntimeState platform;
    AudioRuntimeState audio;
    SceneStageRuntimeState sceneStage;
    ActorTaskRuntimeState actorTask;
    CameraRuntimeState camera;
    CollisionPhysicsLiteRuntimeState collisionPhysicsLite;
    EventQuestFlagRuntimeState eventQuestFlag;
    UiRender2dRuntimeState uiRender2d;
};

class NativeService {
public:
    virtual ~NativeService() = default;
    virtual std::string name() const = 0;
    virtual NativeDispatchResult dispatch(const ApiSurface& api, const NativeCallContext& context) const = 0;
};

class ActorTaskService : public NativeService {};
class AudioService : public NativeService {};
class CameraService : public NativeService {};
class CollisionPhysicsLiteService : public NativeService {};
class EventQuestFlagService : public NativeService {};
class PlatformService : public NativeService {};
class ResourceService : public NativeService {};
class SaveProfileScenarioService : public NativeService {};
class SceneStageService : public NativeService {};
class ScriptService : public NativeService {};
class UiRender2dService : public NativeService {};

class NativeServiceCatalog {
public:
    NativeServiceCatalog();

    const NativeService* find(const std::string& serviceName) const;
    NativeDispatchResult dispatch(const ApiSurface& api, const NativeCallContext& context) const;
    void resetRuntimeState() const;
    void recordStateMutation(const NativeServiceStateMutation& mutation) const;
    const NativeRuntimeServiceState& runtimeState() const;
    std::string runtimeStateToJson() const;
    std::vector<std::string> serviceNames() const;
    std::vector<std::string> unboundApis(const NativeRegistry& registry) const;
    size_t size() const;

private:
    void registerService(std::unique_ptr<NativeService> service);

    std::map<std::string, std::unique_ptr<NativeService>> services_;
    mutable NativeRuntimeServiceState runtimeState_;
};

std::string nativeServiceReportToJson(const NativeRegistry& registry, const NativeServiceCatalog& catalog);
std::string nativeRuntimeServiceStateToJson(const NativeRuntimeServiceState& state);

} // namespace yu::native
