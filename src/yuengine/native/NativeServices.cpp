#include "yuengine/native/NativeServices.h"

#include "yuengine/core/Json.h"

#include <sstream>
#include <utility>

namespace yu::native {
namespace {

template <typename ServiceInterface>
class TrackingNativeService final : public ServiceInterface {
public:
    explicit TrackingNativeService(std::string serviceName)
        : serviceName_(std::move(serviceName))
    {
    }

    std::string name() const override
    {
        return serviceName_;
    }

    NativeDispatchResult dispatch(const ApiSurface& api, const NativeCallContext& context) const override
    {
        NativeDispatchResult result;
        result.api = api.name;
        result.service = api.service;
        result.ownerLevel = api.ownerLevel;
        result.implementationStatus = api.implementationStatus.empty() ? "not_started" : api.implementationStatus;
        result.evidence = api.evidence;
        result.apiKnown = true;
        result.serviceBound = true;
        result.implemented = result.implementationStatus == "implemented";
        if (!result.implemented) {
            result.obligation = "tracked obligation from " + context.module + ":" + context.function + ":pc"
                + std::to_string(context.pc);
        }
        return result;
    }

private:
    std::string serviceName_;
};

void writeStringArray(std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        out << (i ? ", " : "") << "\"" << core::jsonEscape(values[i]) << "\"";
    }
    out << "]";
}

void writeStringMap(std::ostringstream& out, const std::map<std::string, std::string>& values)
{
    out << "{";
    size_t index = 0;
    for (const auto& [key, value] : values) {
        out << (index++ == 0 ? "" : ", ") << "\"" << core::jsonEscape(key) << "\": \""
            << core::jsonEscape(value) << "\"";
    }
    out << "}";
}

std::string valueAfterPrefix(const std::string& value, const std::string& prefix)
{
    const size_t begin = value.find(prefix);
    if (begin == std::string::npos) {
        return {};
    }
    const size_t valueBegin = begin + prefix.size();
    size_t valueEnd = value.find(';', valueBegin);
    if (valueEnd == std::string::npos) {
        valueEnd = value.size();
    }
    while (valueEnd > valueBegin && value[valueEnd - 1] == ' ') {
        --valueEnd;
    }
    return value.substr(valueBegin, valueEnd - valueBegin);
}

UiRuntimeObjectState& ensureUiObject(NativeRuntimeServiceState& state, const std::string& objectName)
{
    return state.uiRender2d.objects[objectName];
}

void appendUiCommand(UiRuntimeObjectState& object, const std::string& command)
{
    ++object.commandCount;
    if (object.commands.size() < 64) {
        object.commands.push_back(command);
    }
}

} // namespace

NativeServiceCatalog::NativeServiceCatalog()
{
    registerService(std::make_unique<TrackingNativeService<ActorTaskService>>("Actor And Task Service"));
    registerService(std::make_unique<TrackingNativeService<AudioService>>("Audio Service"));
    registerService(std::make_unique<TrackingNativeService<CameraService>>("Camera Service"));
    registerService(
        std::make_unique<TrackingNativeService<CollisionPhysicsLiteService>>("Collision And Physics-Lite Service"));
    registerService(std::make_unique<TrackingNativeService<EventQuestFlagService>>("Event/Quest/Flag Service"));
    registerService(std::make_unique<TrackingNativeService<PlatformService>>("Platform Service"));
    registerService(std::make_unique<TrackingNativeService<ResourceService>>("Resource Service"));
    registerService(std::make_unique<TrackingNativeService<SaveProfileScenarioService>>("Save/Profile/Scenario Service"));
    registerService(std::make_unique<TrackingNativeService<SceneStageService>>("Scene And Stage Service"));
    registerService(std::make_unique<TrackingNativeService<ScriptService>>("Script Service"));
    registerService(std::make_unique<TrackingNativeService<UiRender2dService>>("UI And 2D Render Service"));
}

void NativeServiceCatalog::registerService(std::unique_ptr<NativeService> service)
{
    services_[service->name()] = std::move(service);
}

const NativeService* NativeServiceCatalog::find(const std::string& serviceName) const
{
    auto it = services_.find(serviceName);
    return it == services_.end() ? nullptr : it->second.get();
}

NativeDispatchResult NativeServiceCatalog::dispatch(const ApiSurface& api, const NativeCallContext& context) const
{
    const NativeService* service = find(api.service);
    if (service) {
        return service->dispatch(api, context);
    }

    NativeDispatchResult result;
    result.api = api.name;
    result.service = api.service;
    result.ownerLevel = api.ownerLevel;
    result.implementationStatus = "service_unbound";
    result.evidence = api.evidence;
    result.apiKnown = true;
    result.serviceBound = false;
    result.implemented = false;
    result.obligation = "native service interface is not registered";
    return result;
}

void NativeServiceCatalog::resetRuntimeState() const
{
    runtimeState_ = {};
}

void NativeServiceCatalog::recordStateMutation(const NativeServiceStateMutation& mutation) const
{
    ++runtimeState_.mutations;

    if (mutation.service == "Save/Profile/Scenario Service") {
        if (mutation.action == "query_empty_save_list") {
            ++runtimeState_.saveProfileScenario.emptySaveListQueries;
            runtimeState_.saveProfileScenario.saveListEntries =
                mutation.value.find("entries=0") == std::string::npos ? 1 : 0;
        } else if (mutation.action == "save_list_count") {
            ++runtimeState_.saveProfileScenario.saveListCountQueries;
        } else if (mutation.action == "save_list_get") {
            ++runtimeState_.saveProfileScenario.saveListGetQueries;
        } else if (mutation.action == "save_entry_active") {
            ++runtimeState_.saveProfileScenario.saveEntryActiveQueries;
        } else if (mutation.action == "save_capacity_query") {
            ++runtimeState_.saveProfileScenario.saveCapacityQueries;
        } else if (mutation.action == "query_scenario_keys") {
            ++runtimeState_.saveProfileScenario.scenarioKeyQueries;
        } else if (mutation.action == "scenario_key_count") {
            ++runtimeState_.saveProfileScenario.scenarioKeyCountQueries;
        } else if (mutation.action == "scenario_key_get") {
            ++runtimeState_.saveProfileScenario.scenarioKeyGetQueries;
            runtimeState_.saveProfileScenario.currentMissionKey = valueAfterPrefix(mutation.value, "scenario_key=");
        } else if (mutation.action == "set_current_mission") {
            runtimeState_.saveProfileScenario.currentMission = valueAfterPrefix(mutation.value, "mission=");
        } else if (mutation.action == "set_current_mission_key") {
            runtimeState_.saveProfileScenario.currentMissionKey = valueAfterPrefix(mutation.value, "mission_key=");
        } else if (mutation.action == "set_difficulty_mode") {
            runtimeState_.saveProfileScenario.difficultyMode = valueAfterPrefix(mutation.value, "difficulty=");
        } else if (mutation.action == "make_new_game") {
            ++runtimeState_.saveProfileScenario.makeNewGameCommands;
            runtimeState_.saveProfileScenario.currentMissionKey = valueAfterPrefix(mutation.value, "scenario_key=");
            runtimeState_.saveProfileScenario.currentMission = valueAfterPrefix(mutation.value, "mission=");
        } else if (mutation.action == "load_auto_save") {
            ++runtimeState_.saveProfileScenario.loadAutoSaveCommands;
            runtimeState_.saveProfileScenario.lastAutoSaveLoaded = mutation.value;
        } else if (mutation.action == "start_game") {
            ++runtimeState_.saveProfileScenario.startGameCommands;
            runtimeState_.saveProfileScenario.startedMission = valueAfterPrefix(mutation.value, "mission=");
            runtimeState_.saveProfileScenario.startNewGame = valueAfterPrefix(mutation.value, "new_game=");
        } else if (mutation.action == "clear_menu_mission") {
            runtimeState_.saveProfileScenario.currentMission.clear();
            runtimeState_.saveProfileScenario.currentMissionKey.clear();
        }
        return;
    }

    if (mutation.service == "Platform Service") {
        if (mutation.action == "reset_menu_button_holding_times") {
            ++runtimeState_.platform.resetMenuButtonHoldingTimesCommands;
        } else if (mutation.action == "shutdown_permission") {
            ++runtimeState_.platform.shutdownPermissionQueries;
            runtimeState_.platform.shutdownPermission = mutation.value;
            runtimeState_.platform.flags[mutation.api] = mutation.value;
        } else if (mutation.action == "shutdown_game") {
            ++runtimeState_.platform.shutdownGameCommands;
            runtimeState_.platform.shutdownRequested = mutation.value;
            runtimeState_.platform.flags[mutation.api] = mutation.value;
        } else {
            runtimeState_.platform.flags[mutation.api] = mutation.value;
        }
        return;
    }

    if (mutation.service == "Audio Service" && mutation.action == "play_bgm") {
        ++runtimeState_.audio.playBgmCommands;
        runtimeState_.audio.currentBgmId = valueAfterPrefix(mutation.value, "bgm_id=");
        return;
    }

    if (mutation.service == "Audio Service" && mutation.action == "fade_out_bgm") {
        ++runtimeState_.audio.fadeOutBgmCommands;
        runtimeState_.audio.bgmFadeOutDuration = valueAfterPrefix(mutation.value, "duration=");
        return;
    }

    if (mutation.service == "Scene And Stage Service" && mutation.action == "fade_in") {
        ++runtimeState_.sceneStage.fadeInCommands;
        runtimeState_.sceneStage.fadeInDuration = valueAfterPrefix(mutation.value, "duration=");
        runtimeState_.sceneStage.fadeInBlend = valueAfterPrefix(mutation.value, "blend=");
        return;
    }

    if (mutation.service == "Scene And Stage Service" && mutation.action == "fade_out") {
        ++runtimeState_.sceneStage.fadeOutCommands;
        return;
    }

    if (mutation.service == "Scene And Stage Service" && mutation.action == "enter_transition") {
        ++runtimeState_.sceneStage.enterTransitionCommands;
        return;
    }

    if (mutation.service == "Scene And Stage Service" && mutation.action == "leave_transition") {
        ++runtimeState_.sceneStage.leaveTransitionCommands;
        return;
    }

    if (mutation.service == "Scene And Stage Service" && mutation.action == "queue_scene_stage_load") {
        ++runtimeState_.sceneStage.queuedStageLoads;
        runtimeState_.sceneStage.currentMissionScript = valueAfterPrefix(mutation.value, "mission_script=");
        runtimeState_.sceneStage.currentStage = valueAfterPrefix(mutation.value, "stage=");
        runtimeState_.sceneStage.currentRailCamera = valueAfterPrefix(mutation.value, "rail_camera=");
        return;
    }

    if (mutation.service == "Scene And Stage Service" && mutation.action == "create_loader") {
        ++runtimeState_.sceneStage.loaderCommands;
        runtimeState_.sceneStage.activeLoader = valueAfterPrefix(mutation.value, "loader=");
        return;
    }

    if (mutation.service == "Scene And Stage Service" && mutation.action == "load_stage") {
        ++runtimeState_.sceneStage.loadedStageCommands;
        runtimeState_.sceneStage.currentStage = valueAfterPrefix(mutation.value, "stage=");
        return;
    }

    if (mutation.service == "Scene And Stage Service" && mutation.action == "load_events_script_via_mission") {
        ++runtimeState_.sceneStage.loadedEventScriptCommands;
        runtimeState_.sceneStage.currentEventScript = valueAfterPrefix(mutation.value, "event_script=");
        runtimeState_.sceneStage.currentMissionScript = valueAfterPrefix(mutation.value, "mission_script=");
        return;
    }

    if (mutation.service == "Scene And Stage Service" && mutation.action == "call_setup_events") {
        ++runtimeState_.sceneStage.callSetupEventsCommands;
        return;
    }

    if (mutation.service == "Scene And Stage Service" && mutation.action == "load_chara_place") {
        ++runtimeState_.sceneStage.charaPlaceLoads;
        runtimeState_.sceneStage.currentCharaPlace = valueAfterPrefix(mutation.value, "place=");
        return;
    }

    if (mutation.service == "Scene And Stage Service" && mutation.action == "get_place_params") {
        ++runtimeState_.sceneStage.placeParamQueries;
        return;
    }

    if (mutation.service == "Actor And Task Service") {
        if (mutation.action == "push_player_chara") {
            ++runtimeState_.actorTask.pushPlayerCharaCommands;
            runtimeState_.actorTask.currentPlayerChara = valueAfterPrefix(mutation.value, "chara=");
            runtimeState_.actorTask.currentPlayerPosition = valueAfterPrefix(mutation.value, "pos=");
            runtimeState_.actorTask.currentPlayerRotY = valueAfterPrefix(mutation.value, "rot_y=");
            return;
        }

        ++runtimeState_.actorTask.actorMethodCommands;
        if (mutation.action == "set_player_control") {
            ++runtimeState_.actorTask.playerControlCommands;
            runtimeState_.actorTask.playerControlEnabled = valueAfterPrefix(mutation.value, "enabled=");
        } else if (mutation.action == "set_player_pos") {
            ++runtimeState_.actorTask.setPlayerPosCommands;
            runtimeState_.actorTask.currentPlayerPosition = valueAfterPrefix(mutation.value, "pos=");
        } else if (mutation.action == "set_player_angle_y") {
            ++runtimeState_.actorTask.setPlayerAngleYCommands;
            runtimeState_.actorTask.currentPlayerRotY = valueAfterPrefix(mutation.value, "rot_y=");
        } else if (mutation.action == "land_player") {
            ++runtimeState_.actorTask.landPlayerCommands;
            runtimeState_.actorTask.playerLanded = valueAfterPrefix(mutation.value, "landed=");
            const std::string pos = valueAfterPrefix(mutation.value, "pos=");
            if (!pos.empty()) {
                runtimeState_.actorTask.currentPlayerPosition = pos;
            }
        } else if (mutation.action == "get_player_pos") {
            ++runtimeState_.actorTask.getPlayerPosQueries;
        } else if (mutation.action == "current_player_name_query") {
            ++runtimeState_.actorTask.currentPlayerNameQueries;
            runtimeState_.actorTask.currentPlayerName = mutation.value;
        } else if (mutation.action == "get_player_query" || mutation.action == "get_player_chara_query") {
            ++runtimeState_.actorTask.getPlayerQueries;
            runtimeState_.actorTask.currentPlayerChara = mutation.value;
        } else if (mutation.action == "get_player_control_query") {
            ++runtimeState_.actorTask.getPlayerControlQueries;
        } else if (mutation.action == "tutorial_actor_create") {
            ++runtimeState_.actorTask.tutorialActorCreates;
            runtimeState_.actorTask.lastTutorialActor = mutation.target;
        } else if (mutation.action == "tutorial_page_create") {
            ++runtimeState_.actorTask.tutorialPageCreates;
            runtimeState_.actorTask.lastTutorialPage = mutation.target;
        } else if (mutation.action == "push_actor") {
            ++runtimeState_.actorTask.pushActorCommands;
            runtimeState_.actorTask.lastTutorialActor = mutation.target;
        } else if (mutation.action == "wait_actor") {
            ++runtimeState_.actorTask.waitActorCommands;
            runtimeState_.actorTask.lastTutorialActor = mutation.target;
        } else if (mutation.action == "reset_player_action") {
            ++runtimeState_.actorTask.resetPlayerActionCommands;
        } else if (mutation.action == "set_wait_for_landing") {
            runtimeState_.actorTask.waitForLanding = valueAfterPrefix(mutation.value, "enabled=");
        } else if (mutation.action == "fill_heal_progress") {
            runtimeState_.actorTask.healProgressFilled = mutation.value;
        } else if (mutation.action == "set_armed") {
            runtimeState_.actorTask.armed = valueAfterPrefix(mutation.value, "armed=");
        } else if (mutation.action == "play_effect") {
            runtimeState_.actorTask.lastEffect = valueAfterPrefix(mutation.value, "effect=");
        }
        return;
    }

    if (mutation.service == "Camera Service") {
        if (mutation.action == "push_task_game_camera") {
            ++runtimeState_.camera.pushGameCameraCommands;
            runtimeState_.camera.gameCameraPushed = mutation.value;
        } else if (mutation.action == "load_rail_camera") {
            ++runtimeState_.camera.railCameraLoads;
            runtimeState_.camera.railCameraPath = valueAfterPrefix(mutation.value, "rail_camera=");
        } else if (mutation.action == "set_enable_rail_camera") {
            ++runtimeState_.camera.enableRailCameraCommands;
            runtimeState_.camera.railCameraEnabled = valueAfterPrefix(mutation.value, "enabled=");
        } else if (mutation.action == "set_enable_auto_camera_adjust") {
            ++runtimeState_.camera.enableAutoAdjustCommands;
            runtimeState_.camera.autoCameraAdjustEnabled = valueAfterPrefix(mutation.value, "enabled=");
        } else if (mutation.action == "set_default_camera_state") {
            ++runtimeState_.camera.defaultCameraStateCommands;
            runtimeState_.camera.defaultCameraStateTarget = mutation.value;
        } else if (mutation.action == "set_game_camera_if_not") {
            ++runtimeState_.camera.setGameCameraIfNotCommands;
            runtimeState_.camera.gameCameraIfNotTarget = mutation.value;
        }
        return;
    }

    if (mutation.service == "Collision And Physics-Lite Service") {
        if (mutation.action == "event_volume_create") {
            ++runtimeState_.collisionPhysicsLite.eventVolumeCreates;
            runtimeState_.collisionPhysicsLite.lastEventVolume = mutation.target;
        } else if (mutation.action == "event_volume_activate") {
            ++runtimeState_.collisionPhysicsLite.eventVolumeActivationCommands;
            runtimeState_.collisionPhysicsLite.lastEventVolume = mutation.target;
            runtimeState_.collisionPhysicsLite.lastEventVolumeEnabled = valueAfterPrefix(mutation.value, "enabled=");
        }
        return;
    }

    if (mutation.service == "Event/Quest/Flag Service") {
        if (mutation.action == "clear_current_quest") {
            ++runtimeState_.eventQuestFlag.clearCurrentQuestCommands;
        } else if (mutation.action == "mission_request") {
            ++runtimeState_.eventQuestFlag.missionRequestQueries;
            runtimeState_.eventQuestFlag.currentRequest = mutation.value;
        } else if (mutation.action == "marker_from_request") {
            ++runtimeState_.eventQuestFlag.markerQueries;
            runtimeState_.eventQuestFlag.currentMarker = valueAfterPrefix(mutation.value, "marker=");
        } else if (mutation.action == "set_checkpoint") {
            ++runtimeState_.eventQuestFlag.checkpointCommands;
            runtimeState_.eventQuestFlag.currentCheckpoint = valueAfterPrefix(mutation.value, "checkpoint=");
        } else if (mutation.action == "check_fall") {
            ++runtimeState_.eventQuestFlag.checkFallCommands;
        } else if (mutation.action == "event_class_create") {
            ++runtimeState_.eventQuestFlag.eventClassCreates;
            runtimeState_.eventQuestFlag.currentEventUnit = mutation.target;
        } else if (mutation.action == "event_unit_query") {
            ++runtimeState_.eventQuestFlag.eventUnitQueries;
            runtimeState_.eventQuestFlag.currentEventUnit = mutation.target;
        } else if (mutation.action == "event_page_create") {
            ++runtimeState_.eventQuestFlag.eventPageCreates;
            runtimeState_.eventQuestFlag.currentEventPage = mutation.target;
        } else if (mutation.action == "event_marker_create") {
            ++runtimeState_.eventQuestFlag.eventMarkerCreates;
            runtimeState_.eventQuestFlag.currentMarker = mutation.target;
        } else if (mutation.action == "event_actor_create") {
            ++runtimeState_.eventQuestFlag.eventActorCreates;
            runtimeState_.eventQuestFlag.currentEventActor = mutation.target;
        } else if (mutation.action == "event_page_setup") {
            ++runtimeState_.eventQuestFlag.eventPageSetupCommands;
            runtimeState_.eventQuestFlag.currentEventPage = mutation.target;
        } else if (mutation.action == "event_page_done") {
            ++runtimeState_.eventQuestFlag.eventPageDoneCommands;
            runtimeState_.eventQuestFlag.currentEventPage = mutation.target;
        } else if (mutation.action == "event_flag_query") {
            ++runtimeState_.eventQuestFlag.eventFlagQueries;
        } else if (mutation.action == "event_flag_init") {
            ++runtimeState_.eventQuestFlag.eventFlagInitCommands;
        } else if (mutation.action == "event_flag_add") {
            ++runtimeState_.eventQuestFlag.eventFlagAddCommands;
        } else if (mutation.action == "clear_events_all") {
            ++runtimeState_.eventQuestFlag.clearEventsAllCommands;
        } else if (mutation.action == "event_unit_deploy") {
            ++runtimeState_.eventQuestFlag.eventUnitDeployCommands;
        } else if (mutation.action == "update_units") {
            ++runtimeState_.eventQuestFlag.updateUnitsCommands;
        } else if (mutation.action == "talk_branch") {
            ++runtimeState_.eventQuestFlag.talkBranchCommands;
        } else if (mutation.action == "event_wait_for") {
            ++runtimeState_.eventQuestFlag.waitForCommands;
        } else if (mutation.action == "dialog_reset") {
            ++runtimeState_.eventQuestFlag.dialogResetCommands;
        } else if (mutation.action == "dialog_hide") {
            ++runtimeState_.eventQuestFlag.dialogHideCommands;
        } else if (mutation.action == "dialog_show") {
            ++runtimeState_.eventQuestFlag.dialogShowCommands;
            ++runtimeState_.eventQuestFlag.dialogCommandCount;
        } else if (mutation.action == "dialog_speak_l") {
            ++runtimeState_.eventQuestFlag.dialogSpeakCommands;
            ++runtimeState_.eventQuestFlag.dialogCommandCount;
            runtimeState_.eventQuestFlag.lastDialogText = mutation.value;
        } else if (mutation.action == "dialog_wait") {
            ++runtimeState_.eventQuestFlag.dialogWaitCommands;
            ++runtimeState_.eventQuestFlag.dialogCommandCount;
        } else if (mutation.action == "dialog_bg_query") {
            ++runtimeState_.eventQuestFlag.dialogBgQueries;
        } else if (mutation.action.rfind("dialog_", 0) == 0 || mutation.action == "message_lookup") {
            ++runtimeState_.eventQuestFlag.dialogCommandCount;
        }
        return;
    }

    if (mutation.service == "UI And 2D Render Service") {
        if (mutation.action == "create_ui_object") {
            ++runtimeState_.uiRender2d.createdObjects;
            auto& object = ensureUiObject(runtimeState_, mutation.target);
            object.className = mutation.value;
            return;
        }

        ++runtimeState_.uiRender2d.commandCount;
        if (mutation.action == "graph_string") {
            ++runtimeState_.uiRender2d.graphStringCommands;
        } else if (mutation.action == "string_size_query") {
            ++runtimeState_.uiRender2d.stringSizeQueries;
        } else if (mutation.action == "draw_string" || mutation.action == "draw_string_align_right"
            || mutation.action == "draw_string_menu") {
            ++runtimeState_.uiRender2d.drawCommands;
            ++runtimeState_.uiRender2d.textDrawCommands;
        } else if (mutation.action == "draw_frame") {
            ++runtimeState_.uiRender2d.drawCommands;
            ++runtimeState_.uiRender2d.frameDrawCommands;
        } else if (mutation.action == "draw_rect" || mutation.action == "draw_rect_menu") {
            ++runtimeState_.uiRender2d.drawCommands;
            ++runtimeState_.uiRender2d.rectDrawCommands;
        } else if (mutation.action == "draw_graph") {
            ++runtimeState_.uiRender2d.drawCommands;
            ++runtimeState_.uiRender2d.graphDrawCommands;
        } else if (mutation.action == "color") {
            ++runtimeState_.uiRender2d.colorCommands;
        }
        runtimeState_.uiRender2d.lastCommand =
            mutation.api + ":" + mutation.action + "=" + mutation.value;
        auto& object = ensureUiObject(runtimeState_, mutation.target.empty() ? "<unknown_ui_target>" : mutation.target);
        appendUiCommand(object, mutation.api + ":" + mutation.action + "=" + mutation.value);
        return;
    }
}

const NativeRuntimeServiceState& NativeServiceCatalog::runtimeState() const
{
    return runtimeState_;
}

std::string NativeServiceCatalog::runtimeStateToJson() const
{
    return nativeRuntimeServiceStateToJson(runtimeState_);
}

std::vector<std::string> NativeServiceCatalog::serviceNames() const
{
    std::vector<std::string> names;
    for (const auto& [name, _] : services_) {
        names.push_back(name);
    }
    return names;
}

std::vector<std::string> NativeServiceCatalog::unboundApis(const NativeRegistry& registry) const
{
    std::vector<std::string> apis;
    for (const auto& [name, api] : registry.apis()) {
        if (!find(api.service)) {
            apis.push_back(name);
        }
    }
    return apis;
}

size_t NativeServiceCatalog::size() const
{
    return services_.size();
}

std::string nativeRuntimeServiceStateToJson(const NativeRuntimeServiceState& state)
{
    std::ostringstream out;
    out << "{";
    out << "\"mutations\": " << state.mutations << ", ";
    out << "\"save_profile_scenario\": {";
    out << "\"empty_save_list_queries\": " << state.saveProfileScenario.emptySaveListQueries << ", ";
    out << "\"save_list_count_queries\": " << state.saveProfileScenario.saveListCountQueries << ", ";
    out << "\"save_list_get_queries\": " << state.saveProfileScenario.saveListGetQueries << ", ";
    out << "\"save_entry_active_queries\": " << state.saveProfileScenario.saveEntryActiveQueries << ", ";
    out << "\"save_list_entries\": " << state.saveProfileScenario.saveListEntries << ", ";
    out << "\"save_capacity_queries\": " << state.saveProfileScenario.saveCapacityQueries << ", ";
    out << "\"scenario_key_queries\": " << state.saveProfileScenario.scenarioKeyQueries << ", ";
    out << "\"scenario_key_count_queries\": " << state.saveProfileScenario.scenarioKeyCountQueries << ", ";
    out << "\"scenario_key_get_queries\": " << state.saveProfileScenario.scenarioKeyGetQueries << ", ";
    out << "\"make_new_game_commands\": " << state.saveProfileScenario.makeNewGameCommands << ", ";
    out << "\"load_auto_save_commands\": " << state.saveProfileScenario.loadAutoSaveCommands << ", ";
    out << "\"start_game_commands\": " << state.saveProfileScenario.startGameCommands << ", ";
    out << "\"current_mission_key\": \"" << core::jsonEscape(state.saveProfileScenario.currentMissionKey) << "\", ";
    out << "\"current_mission\": \"" << core::jsonEscape(state.saveProfileScenario.currentMission) << "\", ";
    out << "\"started_mission\": \"" << core::jsonEscape(state.saveProfileScenario.startedMission) << "\", ";
    out << "\"difficulty_mode\": \"" << core::jsonEscape(state.saveProfileScenario.difficultyMode) << "\", ";
    out << "\"start_new_game\": \"" << core::jsonEscape(state.saveProfileScenario.startNewGame) << "\", ";
    out << "\"last_auto_save_loaded\": \""
        << core::jsonEscape(state.saveProfileScenario.lastAutoSaveLoaded) << "\"}, ";
    out << "\"platform\": {\"reset_menu_button_holding_times_commands\": "
        << state.platform.resetMenuButtonHoldingTimesCommands
        << ", \"shutdown_permission_queries\": " << state.platform.shutdownPermissionQueries
        << ", \"shutdown_game_commands\": " << state.platform.shutdownGameCommands
        << ", \"shutdown_permission\": \"" << core::jsonEscape(state.platform.shutdownPermission)
        << "\", \"shutdown_requested\": \"" << core::jsonEscape(state.platform.shutdownRequested)
        << "\", \"flags\": ";
    writeStringMap(out, state.platform.flags);
    out << "}, ";
    out << "\"audio\": {\"play_bgm_commands\": " << state.audio.playBgmCommands
        << ", \"fade_out_bgm_commands\": " << state.audio.fadeOutBgmCommands
        << ", \"current_bgm_id\": \"" << core::jsonEscape(state.audio.currentBgmId)
        << "\", \"bgm_fade_out_duration\": \"" << core::jsonEscape(state.audio.bgmFadeOutDuration) << "\"}, ";
    out << "\"scene_stage\": {\"fade_in_commands\": " << state.sceneStage.fadeInCommands
        << ", \"fade_out_commands\": " << state.sceneStage.fadeOutCommands
        << ", \"enter_transition_commands\": " << state.sceneStage.enterTransitionCommands
        << ", \"leave_transition_commands\": " << state.sceneStage.leaveTransitionCommands
        << ", \"queued_stage_loads\": " << state.sceneStage.queuedStageLoads
        << ", \"loader_commands\": " << state.sceneStage.loaderCommands
        << ", \"loaded_stage_commands\": " << state.sceneStage.loadedStageCommands
        << ", \"loaded_event_script_commands\": " << state.sceneStage.loadedEventScriptCommands
        << ", \"call_setup_events_commands\": " << state.sceneStage.callSetupEventsCommands
        << ", \"chara_place_loads\": " << state.sceneStage.charaPlaceLoads
        << ", \"place_param_queries\": " << state.sceneStage.placeParamQueries
        << ", \"fade_in_duration\": \"" << core::jsonEscape(state.sceneStage.fadeInDuration)
        << "\", \"fade_in_blend\": \"" << core::jsonEscape(state.sceneStage.fadeInBlend)
        << "\", \"current_mission_script\": \"" << core::jsonEscape(state.sceneStage.currentMissionScript)
        << "\", \"current_stage\": \"" << core::jsonEscape(state.sceneStage.currentStage)
        << "\", \"current_rail_camera\": \"" << core::jsonEscape(state.sceneStage.currentRailCamera)
        << "\", \"active_loader\": \"" << core::jsonEscape(state.sceneStage.activeLoader)
        << "\", \"current_event_script\": \"" << core::jsonEscape(state.sceneStage.currentEventScript)
        << "\", \"current_chara_place\": \"" << core::jsonEscape(state.sceneStage.currentCharaPlace) << "\"}, ";
    out << "\"actor_task\": {\"push_player_chara_commands\": "
        << state.actorTask.pushPlayerCharaCommands
        << ", \"actor_method_commands\": " << state.actorTask.actorMethodCommands
        << ", \"player_control_commands\": " << state.actorTask.playerControlCommands
        << ", \"set_player_pos_commands\": " << state.actorTask.setPlayerPosCommands
        << ", \"set_player_angle_y_commands\": " << state.actorTask.setPlayerAngleYCommands
        << ", \"land_player_commands\": " << state.actorTask.landPlayerCommands
        << ", \"get_player_pos_queries\": " << state.actorTask.getPlayerPosQueries
        << ", \"current_player_name_queries\": " << state.actorTask.currentPlayerNameQueries
        << ", \"get_player_queries\": " << state.actorTask.getPlayerQueries
        << ", \"get_player_control_queries\": " << state.actorTask.getPlayerControlQueries
        << ", \"reset_player_action_commands\": " << state.actorTask.resetPlayerActionCommands
        << ", \"tutorial_actor_creates\": " << state.actorTask.tutorialActorCreates
        << ", \"tutorial_page_creates\": " << state.actorTask.tutorialPageCreates
        << ", \"push_actor_commands\": " << state.actorTask.pushActorCommands
        << ", \"wait_actor_commands\": " << state.actorTask.waitActorCommands
        << ", \"current_player_chara\": \"" << core::jsonEscape(state.actorTask.currentPlayerChara)
        << "\", \"current_player_name\": \"" << core::jsonEscape(state.actorTask.currentPlayerName)
        << "\", \"current_player_position\": \"" << core::jsonEscape(state.actorTask.currentPlayerPosition)
        << "\", \"current_player_rot_y\": \"" << core::jsonEscape(state.actorTask.currentPlayerRotY)
        << "\", \"player_control_enabled\": \"" << core::jsonEscape(state.actorTask.playerControlEnabled)
        << "\", \"last_tutorial_actor\": \"" << core::jsonEscape(state.actorTask.lastTutorialActor)
        << "\", \"last_tutorial_page\": \"" << core::jsonEscape(state.actorTask.lastTutorialPage)
        << "\", \"player_landed\": \"" << core::jsonEscape(state.actorTask.playerLanded)
        << "\", \"wait_for_landing\": \"" << core::jsonEscape(state.actorTask.waitForLanding)
        << "\", \"heal_progress_filled\": \"" << core::jsonEscape(state.actorTask.healProgressFilled)
        << "\", \"armed\": \"" << core::jsonEscape(state.actorTask.armed)
        << "\", \"last_effect\": \"" << core::jsonEscape(state.actorTask.lastEffect) << "\"}, ";
    out << "\"camera\": {\"push_game_camera_commands\": " << state.camera.pushGameCameraCommands
        << ", \"rail_camera_loads\": " << state.camera.railCameraLoads
        << ", \"enable_rail_camera_commands\": " << state.camera.enableRailCameraCommands
        << ", \"enable_auto_adjust_commands\": " << state.camera.enableAutoAdjustCommands
        << ", \"default_camera_state_commands\": " << state.camera.defaultCameraStateCommands
        << ", \"set_game_camera_if_not_commands\": " << state.camera.setGameCameraIfNotCommands
        << ", \"game_camera_pushed\": \"" << core::jsonEscape(state.camera.gameCameraPushed)
        << "\", \"rail_camera_path\": \"" << core::jsonEscape(state.camera.railCameraPath)
        << "\", \"rail_camera_enabled\": \"" << core::jsonEscape(state.camera.railCameraEnabled)
        << "\", \"auto_camera_adjust_enabled\": \"" << core::jsonEscape(state.camera.autoCameraAdjustEnabled)
        << "\", \"default_camera_state_target\": \""
        << core::jsonEscape(state.camera.defaultCameraStateTarget)
        << "\", \"game_camera_if_not_target\": \"" << core::jsonEscape(state.camera.gameCameraIfNotTarget)
        << "\"}, ";
    out << "\"collision_physics_lite\": {\"event_volume_creates\": "
        << state.collisionPhysicsLite.eventVolumeCreates
        << ", \"event_volume_activation_commands\": "
        << state.collisionPhysicsLite.eventVolumeActivationCommands
        << ", \"last_event_volume\": \"" << core::jsonEscape(state.collisionPhysicsLite.lastEventVolume)
        << "\", \"last_event_volume_enabled\": \""
        << core::jsonEscape(state.collisionPhysicsLite.lastEventVolumeEnabled) << "\"}, ";
    out << "\"event_quest_flag\": {\"clear_current_quest_commands\": "
        << state.eventQuestFlag.clearCurrentQuestCommands
        << ", \"mission_request_queries\": " << state.eventQuestFlag.missionRequestQueries
        << ", \"marker_queries\": " << state.eventQuestFlag.markerQueries
        << ", \"checkpoint_commands\": " << state.eventQuestFlag.checkpointCommands
        << ", \"check_fall_commands\": " << state.eventQuestFlag.checkFallCommands
        << ", \"event_class_creates\": " << state.eventQuestFlag.eventClassCreates
        << ", \"event_unit_queries\": " << state.eventQuestFlag.eventUnitQueries
        << ", \"event_page_creates\": " << state.eventQuestFlag.eventPageCreates
        << ", \"event_marker_creates\": " << state.eventQuestFlag.eventMarkerCreates
        << ", \"event_actor_creates\": " << state.eventQuestFlag.eventActorCreates
        << ", \"event_page_setup_commands\": " << state.eventQuestFlag.eventPageSetupCommands
        << ", \"event_page_done_commands\": " << state.eventQuestFlag.eventPageDoneCommands
        << ", \"event_flag_queries\": " << state.eventQuestFlag.eventFlagQueries
        << ", \"event_flag_init_commands\": " << state.eventQuestFlag.eventFlagInitCommands
        << ", \"event_flag_add_commands\": " << state.eventQuestFlag.eventFlagAddCommands
        << ", \"clear_events_all_commands\": " << state.eventQuestFlag.clearEventsAllCommands
        << ", \"event_unit_deploy_commands\": " << state.eventQuestFlag.eventUnitDeployCommands
        << ", \"update_units_commands\": " << state.eventQuestFlag.updateUnitsCommands
        << ", \"talk_branch_commands\": " << state.eventQuestFlag.talkBranchCommands
        << ", \"wait_for_commands\": " << state.eventQuestFlag.waitForCommands
        << ", \"dialog_reset_commands\": " << state.eventQuestFlag.dialogResetCommands
        << ", \"dialog_hide_commands\": " << state.eventQuestFlag.dialogHideCommands
        << ", \"dialog_show_commands\": " << state.eventQuestFlag.dialogShowCommands
        << ", \"dialog_speak_commands\": " << state.eventQuestFlag.dialogSpeakCommands
        << ", \"dialog_wait_commands\": " << state.eventQuestFlag.dialogWaitCommands
        << ", \"dialog_bg_queries\": " << state.eventQuestFlag.dialogBgQueries
        << ", \"dialog_command_count\": " << state.eventQuestFlag.dialogCommandCount
        << ", \"current_request\": \"" << core::jsonEscape(state.eventQuestFlag.currentRequest)
        << "\", \"current_marker\": \"" << core::jsonEscape(state.eventQuestFlag.currentMarker)
        << "\", \"current_checkpoint\": \"" << core::jsonEscape(state.eventQuestFlag.currentCheckpoint)
        << "\", \"current_event_unit\": \"" << core::jsonEscape(state.eventQuestFlag.currentEventUnit)
        << "\", \"current_event_page\": \"" << core::jsonEscape(state.eventQuestFlag.currentEventPage)
        << "\", \"current_event_actor\": \"" << core::jsonEscape(state.eventQuestFlag.currentEventActor)
        << "\", \"last_dialog_text\": \"" << core::jsonEscape(state.eventQuestFlag.lastDialogText)
        << "\"}, ";
    out << "\"ui_render_2d\": {\"created_objects\": " << state.uiRender2d.createdObjects
        << ", \"command_count\": " << state.uiRender2d.commandCount
        << ", \"draw_commands\": " << state.uiRender2d.drawCommands
        << ", \"graph_string_commands\": " << state.uiRender2d.graphStringCommands
        << ", \"string_size_queries\": " << state.uiRender2d.stringSizeQueries
        << ", \"text_draw_commands\": " << state.uiRender2d.textDrawCommands
        << ", \"frame_draw_commands\": " << state.uiRender2d.frameDrawCommands
        << ", \"rect_draw_commands\": " << state.uiRender2d.rectDrawCommands
        << ", \"graph_draw_commands\": " << state.uiRender2d.graphDrawCommands
        << ", \"color_commands\": " << state.uiRender2d.colorCommands
        << ", \"last_command\": \"" << core::jsonEscape(state.uiRender2d.lastCommand)
        << "\", \"objects\": [";
    size_t index = 0;
    for (const auto& [name, object] : state.uiRender2d.objects) {
        out << (index++ == 0 ? "" : ", ") << "{\"name\": \"" << core::jsonEscape(name)
            << "\", \"class\": \"" << core::jsonEscape(object.className)
            << "\", \"command_count\": " << object.commandCount << ", \"commands\": ";
        writeStringArray(out, object.commands);
        out << "}";
    }
    out << "]}}";
    return out.str();
}

std::string nativeServiceReportToJson(const NativeRegistry& registry, const NativeServiceCatalog& catalog)
{
    const auto serviceApiCounts = registry.serviceApiCounts();
    const auto serviceCallCounts = registry.serviceCallCounts();
    const auto unbound = catalog.unboundApis(registry);
    const size_t notStarted = registry.implementationStatusCount("not_started");
    const size_t unowned = registry.unownedCount();

    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.native_service_report.v1\",\n";
    out << "  \"metrics\": \"services=" << catalog.size() << " native_apis=" << registry.size()
        << " unowned_apis=" << unowned << " unbound_apis=" << unbound.size() << " not_started=" << notStarted
        << "\",\n";
    out << "  \"services\": " << catalog.size() << ",\n";
    out << "  \"native_apis\": " << registry.size() << ",\n";
    out << "  \"unowned_apis\": " << unowned << ",\n";
    out << "  \"unbound_apis\": " << unbound.size() << ",\n";
    out << "  \"not_started\": " << notStarted << ",\n";
    out << "  \"unbound_api_names\": ";
    writeStringArray(out, unbound);
    out << ",\n";
    out << "  \"service_interfaces\": [\n";
    auto names = catalog.serviceNames();
    for (size_t i = 0; i < names.size(); ++i) {
        const auto& name = names[i];
        const auto apiIt = serviceApiCounts.find(name);
        const auto callIt = serviceCallCounts.find(name);
        out << "    {\"name\": \"" << core::jsonEscape(name) << "\", \"apis\": "
            << (apiIt == serviceApiCounts.end() ? 0 : apiIt->second) << ", \"documented_calls\": "
            << (callIt == serviceCallCounts.end() ? 0 : callIt->second) << "}";
        out << (i + 1 == names.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

} // namespace yu::native
