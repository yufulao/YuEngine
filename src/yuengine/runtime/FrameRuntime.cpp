#include "yuengine/runtime/FrameRuntime.h"

#include "yuengine/core/Json.h"
#include "yuengine/native/NativeRegistry.h"
#include "yuengine/project/ProjectManifest.h"
#include "yuengine/script/ScriptRuntime.h"
#include "yuengine/script/SqasmModule.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <stdexcept>

namespace yu::runtime {
namespace {

void addError(FirstFrameRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(MissionEventThreadRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(MissionTutorialRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void writeStringArray(std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        out << (i == 0 ? "" : ", ") << "\"" << core::jsonEscape(values[i]) << "\"";
    }
    out << "]";
}

std::vector<std::filesystem::path> scriptRoots(const project::ProjectManifest& manifest)
{
    std::vector<std::filesystem::path> roots;
    for (const auto& root : manifest.scriptRoots) {
        roots.push_back(root.path);
    }
    return roots;
}

void appendUnique(std::vector<std::string>& values, const std::string& value)
{
    if (std::find(values.begin(), values.end(), value) == values.end()) {
        values.push_back(value);
    }
}

std::vector<script::SqasmModule> loadStartupBaselineModules(
    const project::ProjectManifest& manifest,
    const std::string& entryModule)
{
    std::vector<std::string> moduleNames;
    for (const auto& preload : manifest.startup.preloadScripts) {
        appendUnique(moduleNames, preload);
    }
    for (const auto& dependency : manifest.startup.dependencyScripts) {
        appendUnique(moduleNames, dependency);
    }

    std::vector<script::SqasmModule> modules;
    const auto roots = scriptRoots(manifest);
    for (const auto& moduleName : moduleNames) {
        if (moduleName == entryModule) {
            continue;
        }
        const auto modulePath = script::resolveScriptModule(roots, moduleName);
        if (modulePath.empty()) {
            throw std::runtime_error("startup baseline script module not found: " + moduleName);
        }
        modules.push_back(script::loadSqasmModule(modulePath));
    }
    return modules;
}

int uniqueNativeApiCount(const script::ScriptExecutionReport& report)
{
    std::set<std::string> apis;
    for (const auto& obligation : report.obligations) {
        apis.insert(obligation.api);
    }
    return static_cast<int>(apis.size());
}

} // namespace

FirstFrameRuntimeReport buildFirstFrameRuntimeReport(
    const SceneRuntimeMaterializationReport& sceneRuntime,
    const std::string& rendererProfile)
{
    FirstFrameRuntimeReport report;
    report.projectId = sceneRuntime.projectId;
    report.sceneRuntimeOk = sceneRuntime.ok;
    if (!sceneRuntime.ok) {
        addError(report, "scene runtime materialization is not ready");
    }

    report.renderer.rendererProfile = rendererProfile;
    report.renderer.meshDrawCandidates = sceneRuntime.stage.modelMeshCount;
    report.renderer.materialBindings = sceneRuntime.stage.materialCount;
    report.renderer.textureBindings = sceneRuntime.stage.textureDependencyCount;
    report.renderer.collisionDebugBatches = sceneRuntime.stage.collisionTriangleCount > 0 ? 1 : 0;
    report.renderer.collisionTriangles = sceneRuntime.stage.collisionTriangleCount;
    report.renderer.stageDependencyCount = sceneRuntime.stage.dependencyCount;
    report.renderer.ready = sceneRuntime.stage.ready && report.renderer.meshDrawCandidates > 0
        && report.renderer.textureBindings > 0 && report.renderer.collisionTriangles > 0
        && !report.renderer.rendererProfile.empty();
    if (!report.renderer.ready) {
        addError(report, "renderer first-frame contract is not ready");
    }

    report.actor.playerChara = sceneRuntime.actor.playerChara;
    report.actor.spawnPositionExpression = sceneRuntime.actor.spawnPositionExpression;
    report.actor.spawnRotY = sceneRuntime.actor.spawnRotY;
    report.actor.actorInstances = sceneRuntime.actor.ready ? 1 : 0;
    report.actor.ready = sceneRuntime.actor.ready && report.actor.actorInstances == 1;
    if (!report.actor.ready) {
        addError(report, "actor first-frame contract is not ready");
    }

    report.camera.cameraSource = sceneRuntime.camera.railCameraPath;
    report.camera.defaultTarget = sceneRuntime.camera.defaultCameraStateTarget;
    report.camera.railNodes = sceneRuntime.camera.railNodeCountCandidate;
    report.camera.ready = sceneRuntime.camera.ready && report.camera.railNodes > 0
        && !report.camera.defaultTarget.empty();
    if (!report.camera.ready) {
        addError(report, "camera first-frame contract is not ready");
    }

    report.event.marker = sceneRuntime.eventMarker.marker;
    report.event.eventMarkers = sceneRuntime.eventMarker.ready ? 1 : 0;
    report.event.ready = sceneRuntime.eventMarker.ready && report.event.eventMarkers == 1;
    if (!report.event.ready) {
        addError(report, "event first-frame contract is not ready");
    }

    report.input.ownerService = "Actor And Task Service";
    report.input.controlScope = "player_actor_camera_scene";
    report.input.ready = report.actor.ready && report.camera.ready && report.event.ready;
    if (!report.input.ready) {
        addError(report, "input first-frame ownership contract is not ready");
    }

    return report;
}

FirstFrameRuntimeReport runFirstFrameRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    const auto manifest = project::loadProjectManifest(manifestPath);
    const auto sceneRuntime = runSceneRuntimeMaterialization(manifestPath, repoRoot);
    return buildFirstFrameRuntimeReport(sceneRuntime, manifest.renderer);
}

std::string firstFrameRuntimeReportToJson(const FirstFrameRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.first_frame_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " scene_runtime_ok=" << (report.sceneRuntimeOk ? "true" : "false")
        << " renderer_frame_ready=" << (report.renderer.ready ? "true" : "false")
        << " actor_frame_ready=" << (report.actor.ready ? "true" : "false")
        << " camera_frame_ready=" << (report.camera.ready ? "true" : "false")
        << " input_frame_ready=" << (report.input.ready ? "true" : "false")
        << " event_frame_ready=" << (report.event.ready ? "true" : "false")
        << " mesh_draw_candidates=" << report.renderer.meshDrawCandidates
        << " texture_bindings=" << report.renderer.textureBindings
        << " collision_triangles=" << report.renderer.collisionTriangles
        << " actor_instances=" << report.actor.actorInstances
        << " rail_nodes=" << report.camera.railNodes
        << " event_markers=" << report.event.eventMarkers << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"renderer_frame\": {";
    out << "\"ready\": " << (report.renderer.ready ? "true" : "false")
        << ", \"renderer_profile\": \"" << core::jsonEscape(report.renderer.rendererProfile)
        << "\", \"mesh_draw_candidates\": " << report.renderer.meshDrawCandidates
        << ", \"material_bindings\": " << report.renderer.materialBindings
        << ", \"texture_bindings\": " << report.renderer.textureBindings
        << ", \"collision_debug_batches\": " << report.renderer.collisionDebugBatches
        << ", \"collision_triangles\": " << report.renderer.collisionTriangles
        << ", \"stage_dependency_count\": " << report.renderer.stageDependencyCount << "},\n";
    out << "  \"actor_frame\": {";
    out << "\"ready\": " << (report.actor.ready ? "true" : "false")
        << ", \"player_chara\": \"" << core::jsonEscape(report.actor.playerChara)
        << "\", \"actor_instances\": " << report.actor.actorInstances
        << ", \"spawn_position_expression\": \"" << core::jsonEscape(report.actor.spawnPositionExpression)
        << "\", \"spawn_rot_y\": \"" << core::jsonEscape(report.actor.spawnRotY) << "\"},\n";
    out << "  \"camera_frame\": {";
    out << "\"ready\": " << (report.camera.ready ? "true" : "false")
        << ", \"camera_source\": \"" << core::jsonEscape(report.camera.cameraSource)
        << "\", \"default_target\": \"" << core::jsonEscape(report.camera.defaultTarget)
        << "\", \"rail_nodes\": " << report.camera.railNodes << "},\n";
    out << "  \"input_frame\": {";
    out << "\"ready\": " << (report.input.ready ? "true" : "false")
        << ", \"owner_service\": \"" << core::jsonEscape(report.input.ownerService)
        << "\", \"control_scope\": \"" << core::jsonEscape(report.input.controlScope) << "\"},\n";
    out << "  \"event_frame\": {";
    out << "\"ready\": " << (report.event.ready ? "true" : "false")
        << ", \"marker\": \"" << core::jsonEscape(report.event.marker)
        << "\", \"event_markers\": " << report.event.eventMarkers << "}\n";
    out << "}\n";
    return out.str();
}

MissionEventThreadRuntimeReport runMissionEventThreadRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    MissionEventThreadRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        report.projectId = manifest.projectId;

        const auto sceneRuntime = runSceneRuntimeMaterialization(manifestPath, repoRoot);
        report.sceneRuntimeOk = sceneRuntime.ok;
        if (!sceneRuntime.ok) {
            addError(report, "scene runtime materialization is not ready");
        }

        std::string missionModuleName = manifest.oracle.firstMissionCandidate;
        if (missionModuleName.empty()) {
            addError(report, "first mission candidate is not available in project oracle hints");
            return report;
        }

        const auto modulePath = script::resolveScriptModule(scriptRoots(manifest), missionModuleName);
        if (modulePath.empty()) {
            addError(report, "first mission event script module not found: " + missionModuleName);
            return report;
        }

        native::NativeRegistry registry;
        registry.loadMarkdownSurface(repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md");
        native::NativeServiceCatalog catalog;

        const auto module = script::loadSqasmModule(modulePath);
        const auto baselineModules = loadStartupBaselineModules(manifest, missionModuleName);

        script::ScriptRunOptions options;
        options.frames = 0;
        options.inputScenario = "passive";
        options.executeEventSetupScripts = false;

        report.module = module.path.generic_string();
        report.entryFunction = "threadEvent0000_00";
        const auto execution =
            script::runEntryScript(module, baselineModules, report.entryFunction, registry, catalog, options);
        const auto& state = catalog.runtimeState();

        report.eventThreadFound = execution.entryFound;
        report.eventThreadExecuted = execution.executed;
        report.scriptStatus = execution.status;
        report.scriptFunctions = execution.scriptFunctions;
        report.scriptMethods = execution.scriptMethods;
        report.nativeObligations = execution.nativeObligations;
        report.nativeImplementedCalls = execution.nativeImplementedCalls;
        report.uniqueNativeApis = uniqueNativeApiCount(execution);
        report.serviceStateEvents = execution.serviceStateEventCount;
        report.engineObjectCalls = execution.engineObjectCalls;
        report.valueMethodCalls = execution.valueMethodCalls;
        report.unresolvedCalls = execution.unresolvedCalls;
        report.truncated = execution.truncated;
        report.playerControlCommands = state.actorTask.playerControlCommands;
        report.playerControlEnabled = state.actorTask.playerControlEnabled;
        report.resetMenuButtonHoldingTimesCommands = state.platform.resetMenuButtonHoldingTimesCommands;
        report.dialogResetCommands = state.eventQuestFlag.dialogResetCommands;
        report.dialogHideCommands = state.eventQuestFlag.dialogHideCommands;
        report.resetPlayerActionCommands = state.actorTask.resetPlayerActionCommands;
        report.eventUnitQueries = state.eventQuestFlag.eventUnitQueries;
        report.eventPageSetupCommands = state.eventQuestFlag.eventPageSetupCommands;
        report.eventPageDoneCommands = state.eventQuestFlag.eventPageDoneCommands;
        report.eventVolumeCreates = state.collisionPhysicsLite.eventVolumeCreates;
        report.eventVolumeActivationCommands = state.collisionPhysicsLite.eventVolumeActivationCommands;
        report.lastEventVolumeEnabled = state.collisionPhysicsLite.lastEventVolumeEnabled;
        report.setGameCameraIfNotCommands = state.camera.setGameCameraIfNotCommands;
        report.gameCameraIfNotTarget = state.camera.gameCameraIfNotTarget;

        if (!report.eventThreadFound || !report.eventThreadExecuted) {
            addError(report, "first mission event thread did not execute");
        }
        if (report.unresolvedCalls != 0) {
            addError(report, "first mission event thread has unresolved calls");
        }
        if (report.truncated) {
            addError(report, "first mission event thread execution truncated");
        }
        if (report.playerControlCommands < 2 || report.playerControlEnabled != "true") {
            addError(report, "player-control gate was not recovered from the event thread");
        }
        if (report.resetMenuButtonHoldingTimesCommands < 1) {
            addError(report, "menu button hold reset was not recovered from the event thread");
        }
        if (report.dialogResetCommands < 1 || report.dialogHideCommands < 1) {
            addError(report, "dialog reset/hide lifecycle was not recovered from the event thread");
        }
        if (report.resetPlayerActionCommands < 1) {
            addError(report, "resetPlayerAction was not recovered from the event thread");
        }
        if (report.eventUnitQueries < 1 || report.eventPageSetupCommands < 1 || report.eventPageDoneCommands < 1) {
            addError(report, "event unit/page setup and done state were not recovered from the event thread");
        }
        if (report.eventVolumeActivationCommands < 1 || report.lastEventVolumeEnabled != "false") {
            addError(report, "event volume activation contract was not recovered from the event thread");
        }
        if (report.setGameCameraIfNotCommands < 1) {
            addError(report, "game camera restoration contract was not recovered from the event thread");
        }
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string missionEventThreadRuntimeReportToJson(const MissionEventThreadRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.mission_event_thread_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " scene_runtime_ok=" << (report.sceneRuntimeOk ? "true" : "false")
        << " event_thread_found=" << (report.eventThreadFound ? "true" : "false")
        << " event_thread_executed=" << (report.eventThreadExecuted ? "true" : "false")
        << " entry=" << report.entryFunction
        << " player_control_commands=" << report.playerControlCommands
        << " player_control_enabled=" << report.playerControlEnabled
        << " reset_menu_button_holding_times_commands=" << report.resetMenuButtonHoldingTimesCommands
        << " dialog_reset_commands=" << report.dialogResetCommands
        << " dialog_hide_commands=" << report.dialogHideCommands
        << " reset_player_action_commands=" << report.resetPlayerActionCommands
        << " event_unit_queries=" << report.eventUnitQueries
        << " event_page_setup_commands=" << report.eventPageSetupCommands
        << " event_page_done_commands=" << report.eventPageDoneCommands
        << " event_volume_creates=" << report.eventVolumeCreates
        << " event_volume_activation_commands=" << report.eventVolumeActivationCommands
        << " last_event_volume_enabled=" << report.lastEventVolumeEnabled
        << " set_game_camera_if_not_commands=" << report.setGameCameraIfNotCommands
        << " unresolved_calls=" << report.unresolvedCalls
        << " truncated=" << (report.truncated ? "true" : "false") << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"script\": {";
    out << "\"module\": \"" << core::jsonEscape(report.module)
        << "\", \"entry_function\": \"" << core::jsonEscape(report.entryFunction)
        << "\", \"status\": \"" << core::jsonEscape(report.scriptStatus)
        << "\", \"script_functions\": " << report.scriptFunctions
        << ", \"script_methods\": " << report.scriptMethods
        << ", \"native_obligations\": " << report.nativeObligations
        << ", \"native_implemented_calls\": " << report.nativeImplementedCalls
        << ", \"unique_native_apis\": " << report.uniqueNativeApis
        << ", \"service_state_events\": " << report.serviceStateEvents
        << ", \"engine_object_calls\": " << report.engineObjectCalls
        << ", \"value_method_calls\": " << report.valueMethodCalls
        << ", \"unresolved_calls\": " << report.unresolvedCalls
        << ", \"truncated\": " << (report.truncated ? "true" : "false") << "},\n";
    out << "  \"player_control\": {";
    out << "\"commands\": " << report.playerControlCommands
        << ", \"enabled\": \"" << core::jsonEscape(report.playerControlEnabled)
        << "\", \"reset_player_action_commands\": " << report.resetPlayerActionCommands << "},\n";
    out << "  \"platform_input\": {";
    out << "\"reset_menu_button_holding_times_commands\": "
        << report.resetMenuButtonHoldingTimesCommands << "},\n";
    out << "  \"dialog\": {";
    out << "\"reset_commands\": " << report.dialogResetCommands
        << ", \"hide_commands\": " << report.dialogHideCommands << "},\n";
    out << "  \"event_page\": {";
    out << "\"event_unit_queries\": " << report.eventUnitQueries
        << ", \"setup_commands\": " << report.eventPageSetupCommands
        << ", \"done_commands\": " << report.eventPageDoneCommands << "},\n";
    out << "  \"event_volume\": {";
    out << "\"creates\": " << report.eventVolumeCreates
        << ", \"activation_commands\": " << report.eventVolumeActivationCommands
        << ", \"last_enabled\": \"" << core::jsonEscape(report.lastEventVolumeEnabled) << "\"},\n";
    out << "  \"camera\": {";
    out << "\"set_game_camera_if_not_commands\": " << report.setGameCameraIfNotCommands
        << ", \"target\": \"" << core::jsonEscape(report.gameCameraIfNotTarget) << "\"}\n";
    out << "}\n";
    return out.str();
}

MissionTutorialRuntimeReport runMissionTutorialRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    MissionTutorialRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        report.projectId = manifest.projectId;

        const auto sceneRuntime = runSceneRuntimeMaterialization(manifestPath, repoRoot);
        report.sceneRuntimeOk = sceneRuntime.ok;
        if (!sceneRuntime.ok) {
            addError(report, "scene runtime materialization is not ready");
        }

        std::string missionModuleName = manifest.oracle.firstMissionCandidate;
        if (missionModuleName.empty()) {
            addError(report, "first mission candidate is not available in project oracle hints");
            return report;
        }

        const auto modulePath = script::resolveScriptModule(scriptRoots(manifest), missionModuleName);
        if (modulePath.empty()) {
            addError(report, "first mission event script module not found: " + missionModuleName);
            return report;
        }

        native::NativeRegistry registry;
        registry.loadMarkdownSurface(repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md");

        const auto module = script::loadSqasmModule(modulePath);
        const auto baselineModules = loadStartupBaselineModules(manifest, missionModuleName);

        script::ScriptRunOptions options;
        options.frames = 0;
        options.inputScenario = "passive";
        options.executeEventSetupScripts = false;

        report.module = module.path.generic_string();
        report.entryFunction = "threadEvent0020_00";

        native::NativeServiceCatalog tutorialCatalog;
        const auto tutorialExecution =
            script::runEntryScript(module, baselineModules, report.entryFunction, registry, tutorialCatalog, options);
        const auto& tutorialState = tutorialCatalog.runtimeState();

        native::NativeServiceCatalog updateCatalog;
        const auto updateExecution =
            script::runEntryScript(module, baselineModules, "updateUnits", registry, updateCatalog, options);
        const auto& updateState = updateCatalog.runtimeState();

        report.tutorialThreadFound = tutorialExecution.entryFound;
        report.tutorialThreadExecuted = tutorialExecution.executed;
        report.updateUnitsExecuted = updateExecution.entryFound && updateExecution.executed;
        report.scriptStatus = tutorialExecution.status;
        report.scriptFunctions = tutorialExecution.scriptFunctions + updateExecution.scriptFunctions;
        report.scriptMethods = tutorialExecution.scriptMethods + updateExecution.scriptMethods;
        report.nativeObligations = tutorialExecution.nativeObligations + updateExecution.nativeObligations;
        report.nativeImplementedCalls =
            tutorialExecution.nativeImplementedCalls + updateExecution.nativeImplementedCalls;
        report.uniqueNativeApis = uniqueNativeApiCount(tutorialExecution) + uniqueNativeApiCount(updateExecution);
        report.serviceStateEvents =
            tutorialExecution.serviceStateEventCount + updateExecution.serviceStateEventCount;
        report.engineObjectCalls = tutorialExecution.engineObjectCalls + updateExecution.engineObjectCalls;
        report.valueMethodCalls = tutorialExecution.valueMethodCalls + updateExecution.valueMethodCalls;
        report.unresolvedCalls = tutorialExecution.unresolvedCalls + updateExecution.unresolvedCalls;
        report.truncated = tutorialExecution.truncated || updateExecution.truncated;

        report.eventFlagAddCommands = tutorialState.eventQuestFlag.eventFlagAddCommands;
        report.currentPlayerNameQueries = tutorialState.actorTask.currentPlayerNameQueries;
        report.getPlayerQueries = tutorialState.actorTask.getPlayerQueries;
        report.getPlayerControlQueries = tutorialState.actorTask.getPlayerControlQueries;
        report.dialogShowCommands = tutorialState.eventQuestFlag.dialogShowCommands;
        report.dialogSpeakCommands = tutorialState.eventQuestFlag.dialogSpeakCommands;
        report.dialogWaitCommands = tutorialState.eventQuestFlag.dialogWaitCommands;
        report.dialogHideCommands = tutorialState.eventQuestFlag.dialogHideCommands;
        report.tutorialActorCreates = tutorialState.actorTask.tutorialActorCreates;
        report.tutorialPageCreates = tutorialState.actorTask.tutorialPageCreates;
        report.pushActorCommands = tutorialState.actorTask.pushActorCommands;
        report.waitActorCommands = tutorialState.actorTask.waitActorCommands;
        report.playerControlCommands = tutorialState.actorTask.playerControlCommands;
        report.playerControlEnabled = tutorialState.actorTask.playerControlEnabled;
        report.setPlayerAngleYCommands = tutorialState.actorTask.setPlayerAngleYCommands;
        report.landPlayerCommands = tutorialState.actorTask.landPlayerCommands;
        report.updateUnitsCommands = updateState.eventQuestFlag.updateUnitsCommands;
        report.enterTransitionCommands = tutorialState.sceneStage.enterTransitionCommands;
        report.leaveTransitionCommands = tutorialState.sceneStage.leaveTransitionCommands;
        report.lastTutorialPage = tutorialState.actorTask.lastTutorialPage;
        report.lastDialogText = tutorialState.eventQuestFlag.lastDialogText;

        if (!report.tutorialThreadFound || !report.tutorialThreadExecuted) {
            addError(report, "first mission tutorial thread did not execute");
        }
        if (!report.updateUnitsExecuted || report.updateUnitsCommands < 1) {
            addError(report, "first mission updateUnits did not execute through Event/Quest/Flag Service");
        }
        if (report.unresolvedCalls != 0) {
            addError(report, "first mission tutorial flow has unresolved calls");
        }
        if (report.truncated) {
            addError(report, "first mission tutorial flow execution truncated");
        }
        if (report.eventFlagAddCommands < 1) {
            addError(report, "event flag add command was not recovered from the tutorial event thread");
        }
        if (report.currentPlayerNameQueries < 1) {
            addError(report, "current player name query was not recovered from tutorial dialog");
        }
        if (report.dialogShowCommands < 1 || report.dialogSpeakCommands < 1
            || report.dialogWaitCommands < 1 || report.dialogHideCommands < 1) {
            addError(report, "tutorial dialog lifecycle was not recovered");
        }
        if (report.tutorialActorCreates < 1 || report.tutorialPageCreates < 1) {
            addError(report, "tutorial actor/page creation was not recovered");
        }
        if (report.pushActorCommands < 1 || report.waitActorCommands < 1) {
            addError(report, "tutorial actor scheduler push/wait was not recovered");
        }
        if (report.playerControlCommands < 2 || report.playerControlEnabled != "true") {
            addError(report, "player-control recovery around tutorial flow is incomplete");
        }
        if (report.setPlayerAngleYCommands < 1 || report.landPlayerCommands < 1) {
            addError(report, "tutorial marker/player placement was not recovered");
        }
        if (report.enterTransitionCommands < 1 || report.leaveTransitionCommands < 1) {
            addError(report, "tutorial transition lifecycle was not recovered");
        }
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string missionTutorialRuntimeReportToJson(const MissionTutorialRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.mission_tutorial_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " scene_runtime_ok=" << (report.sceneRuntimeOk ? "true" : "false")
        << " tutorial_thread_found=" << (report.tutorialThreadFound ? "true" : "false")
        << " tutorial_thread_executed=" << (report.tutorialThreadExecuted ? "true" : "false")
        << " update_units_executed=" << (report.updateUnitsExecuted ? "true" : "false")
        << " entry=" << report.entryFunction
        << " event_flag_add_commands=" << report.eventFlagAddCommands
        << " current_player_name_queries=" << report.currentPlayerNameQueries
        << " dialog_show_commands=" << report.dialogShowCommands
        << " dialog_speak_commands=" << report.dialogSpeakCommands
        << " dialog_wait_commands=" << report.dialogWaitCommands
        << " dialog_hide_commands=" << report.dialogHideCommands
        << " tutorial_actor_creates=" << report.tutorialActorCreates
        << " tutorial_page_creates=" << report.tutorialPageCreates
        << " push_actor_commands=" << report.pushActorCommands
        << " wait_actor_commands=" << report.waitActorCommands
        << " player_control_commands=" << report.playerControlCommands
        << " player_control_enabled=" << report.playerControlEnabled
        << " set_player_angle_y_commands=" << report.setPlayerAngleYCommands
        << " land_player_commands=" << report.landPlayerCommands
        << " update_units_commands=" << report.updateUnitsCommands
        << " enter_transition_commands=" << report.enterTransitionCommands
        << " leave_transition_commands=" << report.leaveTransitionCommands
        << " unresolved_calls=" << report.unresolvedCalls
        << " truncated=" << (report.truncated ? "true" : "false") << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"script\": {";
    out << "\"module\": \"" << core::jsonEscape(report.module)
        << "\", \"entry_function\": \"" << core::jsonEscape(report.entryFunction)
        << "\", \"status\": \"" << core::jsonEscape(report.scriptStatus)
        << "\", \"script_functions\": " << report.scriptFunctions
        << ", \"script_methods\": " << report.scriptMethods
        << ", \"native_obligations\": " << report.nativeObligations
        << ", \"native_implemented_calls\": " << report.nativeImplementedCalls
        << ", \"unique_native_apis\": " << report.uniqueNativeApis
        << ", \"service_state_events\": " << report.serviceStateEvents
        << ", \"engine_object_calls\": " << report.engineObjectCalls
        << ", \"value_method_calls\": " << report.valueMethodCalls
        << ", \"unresolved_calls\": " << report.unresolvedCalls
        << ", \"truncated\": " << (report.truncated ? "true" : "false") << "},\n";
    out << "  \"dialog\": {";
    out << "\"show_commands\": " << report.dialogShowCommands
        << ", \"speak_commands\": " << report.dialogSpeakCommands
        << ", \"wait_commands\": " << report.dialogWaitCommands
        << ", \"hide_commands\": " << report.dialogHideCommands
        << ", \"last_dialog_text\": \"" << core::jsonEscape(report.lastDialogText) << "\"},\n";
    out << "  \"tutorial_actor\": {";
    out << "\"creates\": " << report.tutorialActorCreates
        << ", \"pages\": " << report.tutorialPageCreates
        << ", \"push_commands\": " << report.pushActorCommands
        << ", \"wait_commands\": " << report.waitActorCommands
        << ", \"last_page\": \"" << core::jsonEscape(report.lastTutorialPage) << "\"},\n";
    out << "  \"player\": {";
    out << "\"current_player_name_queries\": " << report.currentPlayerNameQueries
        << ", \"get_player_queries\": " << report.getPlayerQueries
        << ", \"get_player_control_queries\": " << report.getPlayerControlQueries
        << ", \"control_commands\": " << report.playerControlCommands
        << ", \"control_enabled\": \"" << core::jsonEscape(report.playerControlEnabled)
        << "\", \"set_angle_y_commands\": " << report.setPlayerAngleYCommands
        << ", \"land_player_commands\": " << report.landPlayerCommands << "},\n";
    out << "  \"event_state\": {";
    out << "\"event_flag_add_commands\": " << report.eventFlagAddCommands
        << ", \"update_units_commands\": " << report.updateUnitsCommands << "},\n";
    out << "  \"transition\": {";
    out << "\"enter_commands\": " << report.enterTransitionCommands
        << ", \"leave_commands\": " << report.leaveTransitionCommands << "}\n";
    out << "}\n";
    return out.str();
}

} // namespace yu::runtime
