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
            runtimeState_.saveProfileScenario.saveListEntries = 0;
        } else if (mutation.action == "save_list_count") {
            ++runtimeState_.saveProfileScenario.saveListCountQueries;
        } else if (mutation.action == "save_list_get") {
            ++runtimeState_.saveProfileScenario.saveListGetQueries;
        } else if (mutation.action == "save_entry_active") {
            ++runtimeState_.saveProfileScenario.saveEntryActiveQueries;
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

    if (mutation.service == "Platform Service" && mutation.action == "platform_flag") {
        runtimeState_.platform.flags[mutation.api] = mutation.value;
        return;
    }

    if (mutation.service == "Audio Service" && mutation.action == "play_bgm") {
        ++runtimeState_.audio.playBgmCommands;
        runtimeState_.audio.currentBgmId = valueAfterPrefix(mutation.value, "bgm_id=");
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

    if (mutation.service == "Scene And Stage Service" && mutation.action == "queue_scene_stage_load") {
        ++runtimeState_.sceneStage.queuedStageLoads;
        runtimeState_.sceneStage.currentMissionScript = valueAfterPrefix(mutation.value, "mission_script=");
        runtimeState_.sceneStage.currentStage = valueAfterPrefix(mutation.value, "stage=");
        runtimeState_.sceneStage.currentRailCamera = valueAfterPrefix(mutation.value, "rail_camera=");
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
    out << "\"scenario_key_queries\": " << state.saveProfileScenario.scenarioKeyQueries << ", ";
    out << "\"scenario_key_count_queries\": " << state.saveProfileScenario.scenarioKeyCountQueries << ", ";
    out << "\"scenario_key_get_queries\": " << state.saveProfileScenario.scenarioKeyGetQueries << ", ";
    out << "\"make_new_game_commands\": " << state.saveProfileScenario.makeNewGameCommands << ", ";
    out << "\"start_game_commands\": " << state.saveProfileScenario.startGameCommands << ", ";
    out << "\"current_mission_key\": \"" << core::jsonEscape(state.saveProfileScenario.currentMissionKey) << "\", ";
    out << "\"current_mission\": \"" << core::jsonEscape(state.saveProfileScenario.currentMission) << "\", ";
    out << "\"started_mission\": \"" << core::jsonEscape(state.saveProfileScenario.startedMission) << "\", ";
    out << "\"difficulty_mode\": \"" << core::jsonEscape(state.saveProfileScenario.difficultyMode) << "\", ";
    out << "\"start_new_game\": \"" << core::jsonEscape(state.saveProfileScenario.startNewGame) << "\"}, ";
    out << "\"platform\": {\"flags\": ";
    writeStringMap(out, state.platform.flags);
    out << "}, ";
    out << "\"audio\": {\"play_bgm_commands\": " << state.audio.playBgmCommands
        << ", \"current_bgm_id\": \"" << core::jsonEscape(state.audio.currentBgmId) << "\"}, ";
    out << "\"scene_stage\": {\"fade_in_commands\": " << state.sceneStage.fadeInCommands
        << ", \"fade_out_commands\": " << state.sceneStage.fadeOutCommands
        << ", \"queued_stage_loads\": " << state.sceneStage.queuedStageLoads
        << ", \"fade_in_duration\": \"" << core::jsonEscape(state.sceneStage.fadeInDuration)
        << "\", \"fade_in_blend\": \"" << core::jsonEscape(state.sceneStage.fadeInBlend)
        << "\", \"current_mission_script\": \"" << core::jsonEscape(state.sceneStage.currentMissionScript)
        << "\", \"current_stage\": \"" << core::jsonEscape(state.sceneStage.currentStage)
        << "\", \"current_rail_camera\": \"" << core::jsonEscape(state.sceneStage.currentRailCamera) << "\"}, ";
    out << "\"ui_render_2d\": {\"created_objects\": " << state.uiRender2d.createdObjects
        << ", \"command_count\": " << state.uiRender2d.commandCount << ", \"objects\": [";
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
