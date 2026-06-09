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
    int scenarioKeyQueries = 0;
    int scenarioKeyCountQueries = 0;
    int scenarioKeyGetQueries = 0;
    int makeNewGameCommands = 0;
    int startGameCommands = 0;
    std::string currentMissionKey;
    std::string currentMission;
    std::string startedMission;
    std::string difficultyMode;
    std::string startNewGame;
};

struct PlatformRuntimeState {
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
    std::string currentPlayerChara;
    std::string currentPlayerPosition;
    std::string currentPlayerRotY;
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
    std::string gameCameraPushed;
    std::string railCameraPath;
    std::string railCameraEnabled;
    std::string autoCameraAdjustEnabled;
    std::string defaultCameraStateTarget;
};

struct EventQuestFlagRuntimeState {
    int clearCurrentQuestCommands = 0;
    int missionRequestQueries = 0;
    int markerQueries = 0;
    int checkpointCommands = 0;
    int checkFallCommands = 0;
    std::string currentRequest;
    std::string currentMarker;
    std::string currentCheckpoint;
};

struct UiRuntimeObjectState {
    std::string className;
    int commandCount = 0;
    std::vector<std::string> commands;
};

struct UiRender2dRuntimeState {
    int createdObjects = 0;
    int commandCount = 0;
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
