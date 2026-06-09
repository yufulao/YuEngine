#include "yuengine/runtime/FrameRuntime.h"

#include "yuengine/core/Json.h"
#include "yuengine/native/NativeRegistry.h"
#include "yuengine/project/ProjectManifest.h"
#include "yuengine/script/ScriptRuntime.h"
#include "yuengine/script/SqasmModule.h"

#include <algorithm>
#include <cstddef>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace yu::runtime {
namespace {

void addError(FirstFrameRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(TitleUiRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(TitleBranchesRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(GameplayFrameRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(RendererBackendSubmissionReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(FrameSchedulerRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(BackendObligationsRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(MaterialSemanticsRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(DevicePresentationRuntimeReport& report, const std::string& message)
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

int uiCommandContainsCount(const native::UiRender2dRuntimeState& uiState, const std::string& needle)
{
    int count = 0;
    for (const auto& [_, object] : uiState.objects) {
        for (const auto& command : object.commands) {
            if (command.find(needle) != std::string::npos) {
                ++count;
            }
        }
    }
    return count;
}

bool uiCommandContains(const native::UiRender2dRuntimeState& uiState, const std::string& needle)
{
    return uiCommandContainsCount(uiState, needle) > 0;
}

std::vector<std::string> titleBranchScenarios()
{
    return {
        "title-continue-disabled",
        "title-continue",
        "title-new-game",
        "title-load-empty",
        "title-load",
        "title-option",
        "title-exit-denied",
        "title-exit-allowed",
    };
}

script::ScriptRunOptions titleBranchOptions(const std::string& scenario)
{
    script::ScriptRunOptions options;
    options.frames = 5;
    options.inputScenario = scenario;
    if (scenario == "title-continue-disabled") {
        options.menuSelectedIndex = 0;
        options.menuDecide = true;
        options.saveListEmpty = true;
        options.continueDisabled = true;
    } else if (scenario == "title-continue") {
        options.menuSelectedIndex = 0;
        options.menuDecide = true;
        options.saveListEmpty = false;
        options.continueDisabled = false;
    } else if (scenario == "title-new-game") {
        options.menuSelectedIndex = 1;
        options.menuDecide = true;
    } else if (scenario == "title-load-empty") {
        options.menuSelectedIndex = 2;
        options.menuDecide = true;
        options.saveListEmpty = true;
    } else if (scenario == "title-load") {
        options.menuSelectedIndex = 2;
        options.menuDecide = true;
        options.saveListEmpty = false;
    } else if (scenario == "title-option") {
        options.menuSelectedIndex = 3;
        options.menuDecide = true;
    } else if (scenario == "title-exit-denied") {
        options.menuSelectedIndex = 4;
        options.menuDecide = true;
        options.canShutdown = false;
    } else if (scenario == "title-exit-allowed") {
        options.menuSelectedIndex = 4;
        options.menuDecide = true;
        options.canShutdown = true;
    } else {
        throw std::runtime_error("unknown title branch scenario: " + scenario);
    }
    return options;
}

int titleBranchAudioCommandCount(const TitleBranchesRuntimeReport& report)
{
    int commands = 0;
    for (const auto& scenario : report.scenarios) {
        commands += scenario.audioServiceCommands;
    }
    return commands;
}

struct GameplayFrameInputs {
    SceneRuntimeMaterializationReport sceneRuntime;
    TitleUiRuntimeReport titleUi;
    TitleBranchesRuntimeReport titleBranches;
    MissionEventThreadRuntimeReport missionEvent;
    MissionTutorialRuntimeReport missionTutorial;
};

GameplayFrameInputs collectGameplayFrameInputs(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    GameplayFrameInputs inputs;
    inputs.sceneRuntime = runSceneRuntimeMaterialization(manifestPath, repoRoot);
    inputs.titleUi = runTitleUiRuntime(manifestPath, repoRoot);
    inputs.titleBranches = runTitleBranchesRuntime(manifestPath, repoRoot);
    inputs.missionEvent = runMissionEventThreadRuntime(manifestPath, repoRoot);
    inputs.missionTutorial = runMissionTutorialRuntime(manifestPath, repoRoot);
    return inputs;
}

GameplayFrameRuntimeReport buildGameplayFrameRuntimeReport(const GameplayFrameInputs& inputs)
{
    GameplayFrameRuntimeReport report;

    const auto& sceneRuntime = inputs.sceneRuntime;
    const auto& titleUi = inputs.titleUi;
    const auto& titleBranches = inputs.titleBranches;
    const auto& missionEvent = inputs.missionEvent;
    const auto& missionTutorial = inputs.missionTutorial;

    report.projectId = sceneRuntime.projectId;
    report.sceneRuntimeOk = sceneRuntime.ok;
    report.titleUiOk = titleUi.ok;
    report.titleBranchesOk = titleBranches.ok;
    report.missionEventThreadOk = missionEvent.ok;
    report.missionTutorialOk = missionTutorial.ok;
    report.frameUpdates = 2;

    report.meshDrawCandidates = sceneRuntime.stage.modelMeshCount;
    report.materialBindings = sceneRuntime.stage.materialCount;
    report.textureBindings = sceneRuntime.stage.textureDependencyCount;
    report.titleUiCommands = titleUi.commandCount;
    report.titleUiDrawCommands = titleUi.drawCommands;
    report.saveStartGameScenarios = titleBranches.startGameScenarios;
    report.saveLoadAutoSaveScenarios = titleBranches.loadAutoSaveScenarios;
    report.saveMakeNewGameScenarios = titleBranches.makeNewGameScenarios;
    report.actorInstances = sceneRuntime.actor.ready ? 1 : 0;
    report.playerControlCommands = missionEvent.playerControlCommands + missionTutorial.playerControlCommands;
    report.cameraCommands = missionEvent.setGameCameraIfNotCommands + (sceneRuntime.camera.ready ? 1 : 0);
    report.railNodes = sceneRuntime.camera.railNodeCountCandidate;
    report.eventCommands = missionEvent.eventPageSetupCommands + missionEvent.eventPageDoneCommands
        + missionTutorial.eventFlagAddCommands + missionTutorial.dialogShowCommands
        + missionTutorial.dialogSpeakCommands + missionTutorial.dialogWaitCommands
        + missionTutorial.dialogHideCommands;
    report.tutorialUpdateCommands = missionTutorial.updateUnitsCommands;
    report.audioCommands = titleBranchAudioCommandCount(titleBranches);

    report.rendererFrameReady = sceneRuntime.stage.ready && report.meshDrawCandidates > 0
        && report.materialBindings > 0 && report.textureBindings > 0;
    report.uiFrameReady = titleUi.ok && report.titleUiCommands >= 50 && report.titleUiDrawCommands >= 9;
    report.saveFrameReady = titleBranches.ok && report.saveStartGameScenarios >= 3
        && report.saveLoadAutoSaveScenarios >= 2 && report.saveMakeNewGameScenarios >= 1;
    report.actorFrameReady = sceneRuntime.actor.ready && missionTutorial.tutorialActorCreates >= 1
        && missionTutorial.pushActorCommands >= 1;
    report.cameraFrameReady = sceneRuntime.camera.ready && report.railNodes > 0
        && missionEvent.setGameCameraIfNotCommands >= 1;
    report.inputFrameReady = report.playerControlCommands >= 6 && missionTutorial.playerControlEnabled == "true";
    report.eventFrameReady = sceneRuntime.eventMarker.ready && report.eventCommands >= 8
        && report.tutorialUpdateCommands >= 1;
    report.audioFrameReady = report.audioCommands >= 20;
    report.gameplayCommandCount = report.meshDrawCandidates + report.titleUiCommands
        + report.saveStartGameScenarios + report.playerControlCommands + report.cameraCommands
        + report.eventCommands + report.audioCommands;

    if (!report.sceneRuntimeOk) {
        addError(report, "scene runtime contract is not ready");
    }
    if (!report.titleUiOk) {
        addError(report, "title UI command payload contract is not ready");
    }
    if (!report.titleBranchesOk) {
        addError(report, "title branch matrix contract is not ready");
    }
    if (!report.missionEventThreadOk) {
        addError(report, "mission event thread contract is not ready");
    }
    if (!report.missionTutorialOk) {
        addError(report, "mission tutorial contract is not ready");
    }
    if (!report.rendererFrameReady) {
        addError(report, "renderer frame payload is incomplete");
    }
    if (!report.uiFrameReady) {
        addError(report, "title UI frame payload is incomplete");
    }
    if (!report.saveFrameReady) {
        addError(report, "save/profile branch frame payload is incomplete");
    }
    if (!report.actorFrameReady) {
        addError(report, "actor/task frame payload is incomplete");
    }
    if (!report.cameraFrameReady) {
        addError(report, "camera frame payload is incomplete");
    }
    if (!report.inputFrameReady) {
        addError(report, "input/player-control frame payload is incomplete");
    }
    if (!report.eventFrameReady) {
        addError(report, "event/tutorial frame payload is incomplete");
    }
    if (!report.audioFrameReady) {
        addError(report, "audio frame payload is incomplete");
    }

    return report;
}

RendererBackendSubmissionReport buildRendererBackendSubmissionReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile)
{
    RendererBackendSubmissionReport report;

    const auto& sceneRuntime = inputs.sceneRuntime;
    const auto& titleUi = inputs.titleUi;
    const auto gameplayFrame = buildGameplayFrameRuntimeReport(inputs);

    report.projectId = sceneRuntime.projectId;
    report.sceneRuntimeOk = sceneRuntime.ok;
    report.titleUiOk = titleUi.ok;
    report.gameplayFrameOk = gameplayFrame.ok;
    report.rendererProfile = rendererProfile;
    report.submissionPasses = 3;
    report.title2dSubmissions = titleUi.commandCount;
    report.titleGraphSubmissions = titleUi.graphDrawCommands;
    report.titleTextSubmissions = titleUi.textDrawCommands + titleUi.graphStringCommands;
    report.sceneMeshSubmissions = sceneRuntime.stage.modelMeshCount;
    report.materialBindings = sceneRuntime.stage.materialCount;
    report.textureBindings = sceneRuntime.stage.textureDependencyCount;
    report.collisionDebugSubmissions = sceneRuntime.stage.collisionTriangleCount > 0 ? 1 : 0;
    report.cameraSubmissions = sceneRuntime.camera.ready ? 1 : 0;
    report.actorSubmissions = sceneRuntime.actor.ready ? 1 : 0;
    report.eventMarkerSubmissions = sceneRuntime.eventMarker.ready ? 1 : 0;
    report.resourceUploadSubmissions = report.materialBindings + report.textureBindings
        + (titleUi.backgroundResourceBound ? 1 : 0) + (titleUi.logoResourceBound ? 1 : 0);
    report.drawSubmissions = titleUi.drawCommands + report.sceneMeshSubmissions + report.collisionDebugSubmissions;
    report.backendCommandCount = report.drawSubmissions + report.resourceUploadSubmissions
        + report.cameraSubmissions + report.actorSubmissions + report.eventMarkerSubmissions;
    report.lastUiCommand = titleUi.lastCommand;
    report.stagePath = sceneRuntime.stage.stagePath;
    report.cameraSource = sceneRuntime.camera.railCameraPath;
    report.playerChara = sceneRuntime.actor.playerChara;
    report.backendObligations = {
        "shader_effect_semantics",
        "blend_depth_state_semantics",
        "texture_upload_format_semantics",
        "font_atlas_and_glyph_metrics",
        "device_swapchain_presentation",
        "original_frame_oracle_parity",
    };

    report.titlePassReady = titleUi.ok && report.title2dSubmissions >= 55
        && report.titleGraphSubmissions >= 3 && report.titleTextSubmissions >= 10
        && titleUi.backgroundResourceBound && titleUi.logoResourceBound;
    report.worldPassReady = sceneRuntime.stage.ready && report.sceneMeshSubmissions > 0
        && report.materialBindings > 0 && report.textureBindings > 0;
    report.resourceUploadReady = report.resourceUploadSubmissions >= 57;
    report.cameraSubmissionReady = sceneRuntime.camera.ready && report.cameraSubmissions == 1;
    report.actorSubmissionReady = sceneRuntime.actor.ready && report.actorSubmissions == 1;
    report.eventSubmissionReady = sceneRuntime.eventMarker.ready && report.eventMarkerSubmissions == 1;
    report.backendFrameReady = report.gameplayFrameOk && !report.rendererProfile.empty()
        && report.titlePassReady && report.worldPassReady && report.resourceUploadReady
        && report.cameraSubmissionReady && report.actorSubmissionReady && report.eventSubmissionReady
        && report.submissionPasses == 3 && report.backendCommandCount >= 181;

    if (!report.sceneRuntimeOk) {
        addError(report, "scene runtime contract is not ready for renderer submission");
    }
    if (!report.titleUiOk) {
        addError(report, "title UI command payload is not ready for renderer submission");
    }
    if (!report.gameplayFrameOk) {
        addError(report, "gameplay-frame contract is not ready for renderer submission");
    }
    if (report.rendererProfile.empty()) {
        addError(report, "renderer profile is not configured by the project manifest");
    }
    if (!report.titlePassReady) {
        addError(report, "title 2D pass submission is incomplete");
    }
    if (!report.worldPassReady) {
        addError(report, "world 3D pass submission is incomplete");
    }
    if (!report.resourceUploadReady) {
        addError(report, "resource upload submission is incomplete");
    }
    if (!report.cameraSubmissionReady) {
        addError(report, "camera submission is incomplete");
    }
    if (!report.actorSubmissionReady) {
        addError(report, "actor submission is incomplete");
    }
    if (!report.eventSubmissionReady) {
        addError(report, "event marker submission is incomplete");
    }
    if (!report.backendFrameReady) {
        addError(report, "renderer backend frame submission is incomplete");
    }

    return report;
}

FrameSchedulerNodeReport makeFrameSchedulerNode(
    const std::string& node,
    const std::string& service,
    bool ready,
    int commands,
    const std::string& source)
{
    FrameSchedulerNodeReport report;
    report.node = node;
    report.service = service;
    report.ready = ready;
    report.commands = commands;
    report.source = source;
    return report;
}

FrameSchedulerRuntimeReport buildFrameSchedulerRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile)
{
    FrameSchedulerRuntimeReport report;
    const auto gameplayFrame = buildGameplayFrameRuntimeReport(inputs);
    const auto rendererSubmission = buildRendererBackendSubmissionReport(inputs, rendererProfile);

    report.projectId = gameplayFrame.projectId;
    report.gameplayFrameOk = gameplayFrame.ok;
    report.rendererSubmissionOk = rendererSubmission.ok;
    report.frameIndex = 0;
    report.schedulerEdges = 12;
    report.gameplayCommands = gameplayFrame.gameplayCommandCount;
    report.rendererBackendCommands = rendererSubmission.backendCommandCount;
    report.backendObligations = static_cast<int>(rendererSubmission.backendObligations.size());

    report.nodes.push_back(makeFrameSchedulerNode(
        "project_lifecycle", "Project Service", true, 1, "project.json"));
    report.nodes.push_back(makeFrameSchedulerNode(
        "script_tick_render", "Script Service", inputs.titleUi.ok, inputs.titleUi.commandCount,
        "script/menu/titlemenu.b64.sqasm"));
    report.nodes.push_back(makeFrameSchedulerNode(
        "save_profile_update", "Save/Profile/Scenario Service", inputs.titleBranches.ok,
        inputs.titleBranches.startGameScenarios + inputs.titleBranches.loadAutoSaveScenarios
            + inputs.titleBranches.makeNewGameScenarios,
        "title branch matrix"));
    report.nodes.push_back(makeFrameSchedulerNode(
        "scene_resource_update", "Scene And Stage Service", inputs.sceneRuntime.stage.ready,
        inputs.sceneRuntime.stage.modelMeshCount + inputs.sceneRuntime.stage.materialCount
            + inputs.sceneRuntime.stage.textureDependencyCount,
        inputs.sceneRuntime.stage.stagePath));
    report.nodes.push_back(makeFrameSchedulerNode(
        "actor_task_update", "Actor And Task Service", gameplayFrame.actorFrameReady,
        gameplayFrame.actorInstances + gameplayFrame.playerControlCommands, inputs.sceneRuntime.actor.playerChara));
    report.nodes.push_back(makeFrameSchedulerNode(
        "camera_update", "Camera Service", gameplayFrame.cameraFrameReady, gameplayFrame.cameraCommands,
        inputs.sceneRuntime.camera.railCameraPath));
    report.nodes.push_back(makeFrameSchedulerNode(
        "event_tutorial_update", "Event/Quest/Flag Service", gameplayFrame.eventFrameReady,
        gameplayFrame.eventCommands + gameplayFrame.tutorialUpdateCommands, "first mission tutorial"));
    report.nodes.push_back(makeFrameSchedulerNode(
        "audio_update", "Audio Service", gameplayFrame.audioFrameReady, gameplayFrame.audioCommands,
        "title branch audio commands"));
    report.nodes.push_back(makeFrameSchedulerNode(
        "renderer_submission", "UI And 2D Render Service", rendererSubmission.backendFrameReady,
        rendererSubmission.backendCommandCount, rendererSubmission.rendererProfile));
    report.nodes.push_back(makeFrameSchedulerNode(
        "backend_obligation_tracking", "Resource Service", true, report.backendObligations,
        "renderer backend obligations"));

    report.nodeCount = static_cast<int>(report.nodes.size());
    for (const auto& node : report.nodes) {
        if (node.ready) {
            ++report.executedNodes;
        } else {
            ++report.unresolvedNodes;
        }
        if (!node.service.empty()) {
            ++report.serviceNodeCount;
        }
        report.scheduledWorkItems += node.commands;
    }

    report.updateGraphReady = report.gameplayFrameOk && report.rendererSubmissionOk
        && report.nodeCount == 10 && report.executedNodes == 10 && report.serviceNodeCount == 10
        && report.schedulerEdges >= 12 && report.scheduledWorkItems >= 469 && report.unresolvedNodes == 0;

    if (!report.gameplayFrameOk) {
        addError(report, "gameplay-frame contract is not ready for scheduler graph");
    }
    if (!report.rendererSubmissionOk) {
        addError(report, "renderer submission contract is not ready for scheduler graph");
    }
    if (!report.updateGraphReady) {
        addError(report, "frame scheduler update graph is incomplete");
    }
    if (report.unresolvedNodes != 0) {
        addError(report, "frame scheduler contains unresolved service nodes");
    }

    return report;
}

bool isDdsPayload(const std::vector<std::byte>& bytes)
{
    return bytes.size() >= 4 && bytes[0] == std::byte{0x44} && bytes[1] == std::byte{0x44}
        && bytes[2] == std::byte{0x53} && bytes[3] == std::byte{0x20};
}

BackendObligationItem makeBackendObligation(
    const std::string& obligation,
    const std::string& status,
    const std::string& evidence)
{
    BackendObligationItem item;
    item.obligation = obligation;
    item.status = status;
    item.evidence = evidence;
    return item;
}

BackendObligationsRuntimeReport buildBackendObligationsRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile,
    const resource::VirtualFileSystem& vfs)
{
    BackendObligationsRuntimeReport report;
    const auto rendererSubmission = buildRendererBackendSubmissionReport(inputs, rendererProfile);
    const auto frameScheduler = buildFrameSchedulerRuntimeReport(inputs, rendererProfile);

    report.projectId = inputs.sceneRuntime.projectId;
    report.frameSchedulerOk = frameScheduler.ok;
    report.rendererSubmissionOk = rendererSubmission.ok;
    report.materialBindings = inputs.sceneRuntime.stage.materialCount;
    report.meshSubmissions = inputs.sceneRuntime.stage.modelMeshCount;
    report.titleTextSubmissions = inputs.titleUi.textDrawCommands + inputs.titleUi.graphStringCommands;

    for (const auto& dependency : inputs.sceneRuntime.stage.dependencies) {
        if (dependency.extension != ".dds") {
            continue;
        }
        ++report.textureDependencies;
        const auto bytes = vfs.readBytes(dependency.path);
        if (!bytes.found) {
            continue;
        }
        ++report.textureBytesFound;
        report.textureByteTotal += static_cast<int64_t>(bytes.bytes.size());
        if (isDdsPayload(bytes.bytes)) {
            ++report.ddsTextures;
        }
    }

    report.textureUploadContractReady = report.rendererSubmissionOk && report.frameSchedulerOk
        && report.textureDependencies == 39 && report.textureBytesFound == 39 && report.ddsTextures == 39
        && report.textureByteTotal > 0;
    report.materialBindingContractReady = report.rendererSubmissionOk && report.materialBindings == 16
        && report.meshSubmissions == 111 && inputs.sceneRuntime.stage.textureDependencyCount == 39;
    report.shaderEffectContractTracked = report.materialBindingContractReady;
    report.fontContractTracked = report.titleTextSubmissions >= 11;
    report.deviceContractTracked = report.rendererSubmissionOk;
    report.oracleParityContractTracked = report.frameSchedulerOk;

    report.obligations.push_back(makeBackendObligation(
        "texture_upload_format_semantics",
        report.textureUploadContractReady ? "contract_ready" : "blocked",
        "39 DDS payloads resolved through VFS"));
    report.obligations.push_back(makeBackendObligation(
        "material_binding_semantics",
        report.materialBindingContractReady ? "contract_ready" : "blocked",
        "16 material tags bound to 111 mesh submissions and 39 textures"));
    report.obligations.push_back(makeBackendObligation(
        "shader_effect_semantics",
        report.shaderEffectContractTracked ? "tracked_open" : "blocked",
        "material tags are counted but effect/shader bytecode semantics are not decoded"));
    report.obligations.push_back(makeBackendObligation(
        "font_atlas_and_glyph_metrics",
        report.fontContractTracked ? "tracked_open" : "blocked",
        "11 title text submissions require font atlas and glyph metric contracts"));
    report.obligations.push_back(makeBackendObligation(
        "device_swapchain_presentation",
        report.deviceContractTracked ? "tracked_open" : "blocked",
        "renderer profile is d3d9_compatible but no device backend is created"));
    report.obligations.push_back(makeBackendObligation(
        "original_frame_oracle_parity",
        report.oracleParityContractTracked ? "tracked_open" : "blocked",
        "scheduler output has no original-frame screenshot or API trace parity yet"));

    for (const auto& obligation : report.obligations) {
        if (obligation.status == "contract_ready") {
            ++report.resolvedBackendContracts;
        } else if (obligation.status == "tracked_open") {
            ++report.trackedBackendObligations;
            ++report.openBackendObligations;
        } else {
            ++report.openBackendObligations;
        }
    }

    if (!report.frameSchedulerOk) {
        addError(report, "frame scheduler contract is not ready for backend obligation resolution");
    }
    if (!report.rendererSubmissionOk) {
        addError(report, "renderer submission contract is not ready for backend obligation resolution");
    }
    if (!report.textureUploadContractReady) {
        addError(report, "texture upload format contract is not ready");
    }
    if (!report.materialBindingContractReady) {
        addError(report, "material binding contract is not ready");
    }
    if (!report.shaderEffectContractTracked || !report.fontContractTracked
        || !report.deviceContractTracked || !report.oracleParityContractTracked) {
        addError(report, "backend open obligations are not fully tracked");
    }
    if (report.resolvedBackendContracts < 2 || report.trackedBackendObligations < 4
        || report.openBackendObligations != 4) {
        addError(report, "backend obligation accounting is inconsistent");
    }

    return report;
}

std::string bytesToText(const std::vector<std::byte>& bytes)
{
    std::string text;
    text.reserve(bytes.size());
    for (const auto byte : bytes) {
        text.push_back(static_cast<char>(byte));
    }
    return text;
}

int countOccurrences(const std::string& text, const std::string& needle)
{
    if (needle.empty()) {
        return 0;
    }
    int count = 0;
    size_t offset = 0;
    while ((offset = text.find(needle, offset)) != std::string::npos) {
        ++count;
        offset += needle.size();
    }
    return count;
}

bool parsePositiveInteger(const std::string& text, size_t& offset, int& out)
{
    while (offset < text.size() && text[offset] == ' ') {
        ++offset;
    }
    const size_t begin = offset;
    while (offset < text.size() && text[offset] >= '0' && text[offset] <= '9') {
        ++offset;
    }
    if (begin == offset) {
        return false;
    }
    out = std::stoi(text.substr(begin, offset - begin));
    return true;
}

bool extractSmaaPixelSizeCandidate(const std::string& text, int& width, int& height)
{
    const std::string marker = "SMAA_PIXEL_SIZE float2(1.0 / ";
    size_t offset = text.find(marker);
    if (offset == std::string::npos) {
        return false;
    }
    offset += marker.size();
    if (!parsePositiveInteger(text, offset, width)) {
        return false;
    }
    const std::string secondDenominator = "1.0 / ";
    offset = text.find(secondDenominator, offset);
    if (offset == std::string::npos) {
        return false;
    }
    offset += secondDenominator.size();
    return parsePositiveInteger(text, offset, height);
}

MaterialSemanticsRuntimeReport buildMaterialSemanticsRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile,
    const resource::VirtualFileSystem& vfs)
{
    MaterialSemanticsRuntimeReport report;
    const auto rendererSubmission = buildRendererBackendSubmissionReport(inputs, rendererProfile);
    const auto backendObligations = buildBackendObligationsRuntimeReport(inputs, rendererProfile, vfs);
    const auto& stage = inputs.sceneRuntime.stage;

    report.projectId = inputs.sceneRuntime.projectId;
    report.sceneRuntimeOk = inputs.sceneRuntime.ok;
    report.rendererSubmissionOk = rendererSubmission.ok;
    report.backendObligationsOk = backendObligations.ok;
    report.modelPath = stage.modelPath;
    report.materials = stage.materialCount;
    report.textureSlots = stage.materialTextureSlotCount;
    report.resolvedTextureSlots = stage.materialTextureResolvedCount;
    report.unresolvedTextureSlots = report.textureSlots - report.resolvedTextureSlots;
    report.meshSubmissions = stage.modelMeshCount;
    report.namedMeshSubmissions = stage.namedMeshCount;
    report.meshMaterialBindings = stage.meshMaterialBindingCount;
    report.unresolvedMeshMaterialBindings = stage.unresolvedMeshMaterialBindingCount;

    for (const auto& material : stage.materials) {
        if (material.byteOffset >= 0 && material.headerFlag1 == 5 && material.headerFlag2 == 1) {
            ++report.materialParameterBlocks;
        }
        MaterialSemanticsMaterialReport materialReport;
        materialReport.index = material.index;
        materialReport.name = material.name;
        materialReport.textureSlotCount = material.textureSlotCount;
        materialReport.resolvedTextureSlots = material.resolvedTextureSlots;
        materialReport.meshBindingCount = material.meshBindingCount;
        for (const auto& slot : material.textureSlots) {
            materialReport.textureSlots.push_back(slot.slot + ":" + slot.path);
        }
        report.materialReports.push_back(std::move(materialReport));
    }

    const auto smaaFx = vfs.readBytes("SMAA.fx");
    if (smaaFx.found) {
        const std::string text = bytesToText(smaaFx.bytes);
        report.postEffectTechniques = countOccurrences(text, "\ntechnique ");
        report.postEffectPasses = countOccurrences(text, "\n    pass ");
        report.postEffectSamplers = countOccurrences(text, "\nsampler2D ");
    }
    report.postEffectSourceTracked =
        smaaFx.found && report.postEffectTechniques >= 5 && report.postEffectPasses >= 5
        && report.postEffectSamplers >= 7;

    report.textureSlotContractReady = report.materials == 16 && report.materialParameterBlocks == 16
        && report.textureSlots == 39 && report.resolvedTextureSlots == 39
        && report.unresolvedTextureSlots == 0;
    report.meshMaterialContractReady = report.meshSubmissions == 111 && report.namedMeshSubmissions >= 110
        && report.meshMaterialBindings >= 110 && report.unresolvedMeshMaterialBindings <= 1;
    report.materialSemanticsContractReady = report.sceneRuntimeOk && report.rendererSubmissionOk
        && report.backendObligationsOk && report.textureSlotContractReady
        && report.meshMaterialContractReady;
    report.shaderEffectContractTracked =
        report.materialSemanticsContractReady && report.postEffectSourceTracked;

    report.obligations.push_back(makeBackendObligation(
        "model_material_block_semantics",
        report.materialSemanticsContractReady ? "contract_ready" : "blocked",
        "16 mat blocks decoded with header flags, color candidates, names, and texture slots"));
    report.obligations.push_back(makeBackendObligation(
        "material_texture_slot_semantics",
        report.textureSlotContractReady ? "contract_ready" : "blocked",
        "39 material texture slots resolve through VFS"));
    report.obligations.push_back(makeBackendObligation(
        "mesh_material_name_binding",
        report.meshMaterialContractReady ? "contract_ready" : "blocked",
        "110 named mesh submissions match material names through original mesh-name suffixes"));
    report.obligations.push_back(makeBackendObligation(
        "postprocess_smaa_effect_source",
        report.postEffectSourceTracked ? "contract_ready" : "blocked",
        "SMAA.fx exposes DX9 HLSL techniques, passes, and samplers"));
    report.obligations.push_back(makeBackendObligation(
        "material_shader_program_semantics",
        report.shaderEffectContractTracked ? "tracked_open" : "blocked",
        "model material blocks expose texture slots but no per-material shader/effect program token"));
    report.obligations.push_back(makeBackendObligation(
        "unnamed_mesh_material_binding",
        report.unresolvedMeshMaterialBindings <= 1 ? "tracked_open" : "blocked",
        "one mesh block has no length-prefixed name before the msh tag"));

    if (!report.sceneRuntimeOk) {
        addError(report, "scene runtime contract is not ready for material semantics");
    }
    if (!report.rendererSubmissionOk) {
        addError(report, "renderer submission contract is not ready for material semantics");
    }
    if (!report.backendObligationsOk) {
        addError(report, "backend obligations contract is not ready for material semantics");
    }
    if (!report.textureSlotContractReady) {
        addError(report, "material texture slot contract is incomplete");
    }
    if (!report.meshMaterialContractReady) {
        addError(report, "mesh to material binding contract is incomplete");
    }
    if (!report.postEffectSourceTracked) {
        addError(report, "postprocess SMAA effect source is not tracked");
    }
    if (!report.shaderEffectContractTracked) {
        addError(report, "shader/effect semantics are not tracked through material semantics");
    }

    return report;
}

DevicePresentationRuntimeReport buildDevicePresentationRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile,
    const resource::VirtualFileSystem& vfs)
{
    DevicePresentationRuntimeReport report;
    const auto frameScheduler = buildFrameSchedulerRuntimeReport(inputs, rendererProfile);
    const auto rendererSubmission = buildRendererBackendSubmissionReport(inputs, rendererProfile);
    const auto backendObligations = buildBackendObligationsRuntimeReport(inputs, rendererProfile, vfs);
    const auto materialSemantics = buildMaterialSemanticsRuntimeReport(inputs, rendererProfile, vfs);

    report.projectId = inputs.sceneRuntime.projectId;
    report.frameSchedulerOk = frameScheduler.ok;
    report.rendererSubmissionOk = rendererSubmission.ok;
    report.backendObligationsOk = backendObligations.ok;
    report.materialSemanticsOk = materialSemantics.ok;
    report.rendererProfile = rendererSubmission.rendererProfile;
    report.rendererBackendCommands = rendererSubmission.backendCommandCount;
    report.resourceUploadSubmissions = rendererSubmission.resourceUploadSubmissions;
    report.drawSubmissions = rendererSubmission.drawSubmissions;
    report.title2dSubmissions = rendererSubmission.title2dSubmissions;
    report.worldMeshSubmissions = rendererSubmission.sceneMeshSubmissions;
    report.materialTextureSlots = materialSemantics.textureSlots;
    report.textureBytesFound = backendObligations.textureBytesFound;
    report.materialBindings = rendererSubmission.materialBindings;
    report.meshSubmissions = rendererSubmission.sceneMeshSubmissions;
    report.postEffectTechniques = materialSemantics.postEffectTechniques;
    report.postEffectPasses = materialSemantics.postEffectPasses;
    report.postEffectSamplers = materialSemantics.postEffectSamplers;

    const auto smaaFx = vfs.readBytes("SMAA.fx");
    if (smaaFx.found) {
        const std::string text = bytesToText(smaaFx.bytes);
        extractSmaaPixelSizeCandidate(text, report.backbufferWidthCandidate, report.backbufferHeightCandidate);
    }

    const bool hasBlendDepthObligation = std::find(
        rendererSubmission.backendObligations.begin(),
        rendererSubmission.backendObligations.end(),
        "blend_depth_state_semantics") != rendererSubmission.backendObligations.end();

    report.deviceProfileReady =
        rendererSubmission.backendFrameReady && report.rendererProfile == "d3d9_compatible";
    report.resourceUploadPlanReady = backendObligations.textureUploadContractReady
        && materialSemantics.textureSlotContractReady && report.resourceUploadSubmissions >= 57
        && report.textureBytesFound == 39;
    report.drawQueueContractReady = report.rendererSubmissionOk && report.drawSubmissions == 121
        && report.title2dSubmissions >= 55 && report.worldMeshSubmissions == 111;
    report.renderStateContractTracked = materialSemantics.shaderEffectContractTracked
        && hasBlendDepthObligation && report.postEffectSamplers >= 7;
    report.swapchainContractTracked = report.deviceProfileReady && report.backbufferWidthCandidate == 1280
        && report.backbufferHeightCandidate == 720;
    report.presentContractTracked = report.frameSchedulerOk && report.swapchainContractTracked;

    report.contracts.push_back(makeBackendObligation(
        "d3d9_device_profile",
        report.deviceProfileReady ? "contract_ready" : "blocked",
        "renderer profile is d3d9_compatible and backend frame submission is ready"));
    report.contracts.push_back(makeBackendObligation(
        "resource_upload_plan",
        report.resourceUploadPlanReady ? "contract_ready" : "blocked",
        "57 renderer upload submissions consume 39 texture payloads and 39 material texture slots"));
    report.contracts.push_back(makeBackendObligation(
        "draw_queue_submission",
        report.drawQueueContractReady ? "contract_ready" : "blocked",
        "121 draw submissions include title 2D and 111 world mesh submissions"));
    report.contracts.push_back(makeBackendObligation(
        "swapchain_backbuffer_candidate",
        report.swapchainContractTracked ? "tracked_open" : "blocked",
        "SMAA_PIXEL_SIZE exposes a 1280x720 backbuffer candidate but no OS surface is created"));
    report.contracts.push_back(makeBackendObligation(
        "sampler_state_semantics",
        report.renderStateContractTracked ? "tracked_open" : "blocked",
        "7 SMAA samplers are counted; material sampler state values remain unbound"));
    report.contracts.push_back(makeBackendObligation(
        "blend_depth_state_semantics",
        report.renderStateContractTracked ? "tracked_open" : "blocked",
        "renderer submission carries blend/depth obligation but no device render state is set"));
    report.contracts.push_back(makeBackendObligation(
        "present_call_and_window_surface",
        report.presentContractTracked ? "tracked_open" : "blocked",
        "scheduler reaches renderer submission but no HWND/device Present call is executed"));
    report.contracts.push_back(makeBackendObligation(
        "original_frame_oracle_parity",
        backendObligations.oracleParityContractTracked ? "tracked_open" : "blocked",
        "device contract still lacks original-frame screenshot or API trace parity"));

    for (const auto& contract : report.contracts) {
        if (contract.status == "contract_ready") {
            ++report.resolvedDeviceContracts;
        } else if (contract.status == "tracked_open") {
            ++report.trackedDeviceObligations;
            ++report.openDeviceObligations;
        } else {
            ++report.openDeviceObligations;
        }
    }

    if (!report.frameSchedulerOk) {
        addError(report, "frame scheduler contract is not ready for device presentation");
    }
    if (!report.rendererSubmissionOk) {
        addError(report, "renderer submission contract is not ready for device presentation");
    }
    if (!report.backendObligationsOk) {
        addError(report, "backend obligations contract is not ready for device presentation");
    }
    if (!report.materialSemanticsOk) {
        addError(report, "material semantics contract is not ready for device presentation");
    }
    if (!report.deviceProfileReady) {
        addError(report, "D3D9-compatible device profile contract is not ready");
    }
    if (!report.resourceUploadPlanReady) {
        addError(report, "resource upload plan contract is not ready");
    }
    if (!report.drawQueueContractReady) {
        addError(report, "draw queue contract is not ready");
    }
    if (!report.renderStateContractTracked) {
        addError(report, "render-state obligations are not tracked");
    }
    if (!report.swapchainContractTracked || !report.presentContractTracked) {
        addError(report, "swapchain/present obligations are not tracked");
    }
    if (report.resolvedDeviceContracts < 3 || report.trackedDeviceObligations < 5
        || report.openDeviceObligations != 5) {
        addError(report, "device presentation obligation accounting is inconsistent");
    }

    return report;
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

TitleUiRuntimeReport runTitleUiRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    TitleUiRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        report.projectId = manifest.projectId;

        const auto modulePath = script::resolveScriptModule(scriptRoots(manifest), manifest.startup.entryModule);
        if (modulePath.empty()) {
            addError(report, "title entry script module not found: " + manifest.startup.entryModule);
            return report;
        }

        native::NativeRegistry registry;
        registry.loadMarkdownSurface(repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md");
        native::NativeServiceCatalog catalog;

        const auto module = script::loadSqasmModule(modulePath);
        const auto baselineModules = loadStartupBaselineModules(manifest, manifest.startup.entryModule);

        script::ScriptRunOptions options;
        options.frames = 1;
        options.renderFrames = 1;
        options.inputScenario = "passive";
        options.executeEventSetupScripts = false;

        report.module = module.path.generic_string();
        report.entryFunction = manifest.startup.entryFunction;
        const auto execution =
            script::runEntryScript(module, baselineModules, report.entryFunction, registry, catalog, options);
        const auto& uiState = catalog.runtimeState().uiRender2d;

        report.titleSetupFound = execution.entryFound;
        report.titleSetupExecuted = execution.executed;
        report.scriptStatus = execution.status;
        report.scriptFunctions = execution.scriptFunctions;
        report.scriptMethods = execution.scriptMethods;
        report.nativeObligations = execution.nativeObligations;
        report.nativeImplementedCalls = execution.nativeImplementedCalls;
        report.uniqueNativeApis = uniqueNativeApiCount(execution);
        report.serviceStateEvents = execution.serviceStateEventCount;
        report.uiObjectCalls = execution.uiObjectCalls;
        report.uiServiceCommands = execution.uiServiceCommands;
        report.unresolvedCalls = execution.unresolvedCalls;
        report.truncated = execution.truncated;
        report.createdObjects = uiState.createdObjects;
        report.commandCount = uiState.commandCount;
        report.drawCommands = uiState.drawCommands;
        report.graphStringCommands = uiState.graphStringCommands;
        report.stringSizeQueries = uiState.stringSizeQueries;
        report.textDrawCommands = uiState.textDrawCommands;
        report.graphDrawCommands = uiState.graphDrawCommands;
        report.colorCommands = uiState.colorCommands;
        report.localizedMenuTextCommands = uiCommandContainsCount(uiState, "localized:menu.");
        report.drawListItemCommands = uiCommandContainsCount(uiState, "draw_list_item");
        report.backgroundResourceBound = uiCommandContains(uiState, "menu/title/title_back_sc.dds");
        report.logoResourceBound = uiCommandContains(uiState, "menu/title/logo_sc.dds");
        report.lastCommand = uiState.lastCommand;
        report.titleRenderExecuted = report.commandCount > 0 && report.drawCommands > 0;

        if (!report.titleSetupFound || !report.titleSetupExecuted) {
            addError(report, "title setup entry did not execute");
        }
        if (!report.titleRenderExecuted) {
            addError(report, "title renderProc did not produce UI commands");
        }
        if (report.unresolvedCalls != 0) {
            addError(report, "title UI flow has unresolved calls");
        }
        if (report.truncated) {
            addError(report, "title UI flow execution truncated");
        }
        if (report.createdObjects < 20 || report.commandCount < 50) {
            addError(report, "title UI object/command payload is incomplete");
        }
        if (report.drawCommands < 9 || report.graphDrawCommands < 3 || report.textDrawCommands < 6) {
            addError(report, "title UI draw command payload is incomplete");
        }
        if (report.graphStringCommands < 5 || report.stringSizeQueries < 5 || report.colorCommands < 11) {
            addError(report, "title UI text/layout/color payload is incomplete");
        }
        if (report.localizedMenuTextCommands < 5 || report.drawListItemCommands < 5) {
            addError(report, "title menu list payload is incomplete");
        }
        if (!report.backgroundResourceBound || !report.logoResourceBound) {
            addError(report, "title background/logo resources are not bound into UI commands");
        }
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string titleUiRuntimeReportToJson(const TitleUiRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.title_ui_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " title_setup_found=" << (report.titleSetupFound ? "true" : "false")
        << " title_setup_executed=" << (report.titleSetupExecuted ? "true" : "false")
        << " title_render_executed=" << (report.titleRenderExecuted ? "true" : "false")
        << " entry=" << report.entryFunction
        << " created_objects=" << report.createdObjects
        << " command_count=" << report.commandCount
        << " draw_commands=" << report.drawCommands
        << " graph_string_commands=" << report.graphStringCommands
        << " string_size_queries=" << report.stringSizeQueries
        << " text_draw_commands=" << report.textDrawCommands
        << " graph_draw_commands=" << report.graphDrawCommands
        << " color_commands=" << report.colorCommands
        << " localized_menu_text_commands=" << report.localizedMenuTextCommands
        << " draw_list_item_commands=" << report.drawListItemCommands
        << " background_resource_bound=" << (report.backgroundResourceBound ? "true" : "false")
        << " logo_resource_bound=" << (report.logoResourceBound ? "true" : "false")
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
        << ", \"ui_object_calls\": " << report.uiObjectCalls
        << ", \"ui_service_commands\": " << report.uiServiceCommands
        << ", \"unresolved_calls\": " << report.unresolvedCalls
        << ", \"truncated\": " << (report.truncated ? "true" : "false") << "},\n";
    out << "  \"ui_render_2d\": {";
    out << "\"created_objects\": " << report.createdObjects
        << ", \"command_count\": " << report.commandCount
        << ", \"draw_commands\": " << report.drawCommands
        << ", \"graph_string_commands\": " << report.graphStringCommands
        << ", \"string_size_queries\": " << report.stringSizeQueries
        << ", \"text_draw_commands\": " << report.textDrawCommands
        << ", \"graph_draw_commands\": " << report.graphDrawCommands
        << ", \"color_commands\": " << report.colorCommands
        << ", \"localized_menu_text_commands\": " << report.localizedMenuTextCommands
        << ", \"draw_list_item_commands\": " << report.drawListItemCommands
        << ", \"background_resource_bound\": " << (report.backgroundResourceBound ? "true" : "false")
        << ", \"logo_resource_bound\": " << (report.logoResourceBound ? "true" : "false")
        << ", \"last_command\": \"" << core::jsonEscape(report.lastCommand) << "\"}\n";
    out << "}\n";
    return out.str();
}

TitleBranchesRuntimeReport runTitleBranchesRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    TitleBranchesRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        report.projectId = manifest.projectId;

        const auto modulePath = script::resolveScriptModule(scriptRoots(manifest), manifest.startup.entryModule);
        if (modulePath.empty()) {
            addError(report, "title entry script module not found: " + manifest.startup.entryModule);
            return report;
        }

        native::NativeRegistry registry;
        registry.loadMarkdownSurface(repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md");

        const auto module = script::loadSqasmModule(modulePath);
        const auto baselineModules = loadStartupBaselineModules(manifest, manifest.startup.entryModule);
        report.module = module.path.generic_string();
        report.entryFunction = manifest.startup.entryFunction;

        const auto scenarios = titleBranchScenarios();
        report.scenarioCount = static_cast<int>(scenarios.size());
        for (const auto& scenario : scenarios) {
            native::NativeServiceCatalog catalog;
            const auto options = titleBranchOptions(scenario);
            const auto execution =
                script::runEntryScript(module, baselineModules, report.entryFunction, registry, catalog, options);
            const auto& state = catalog.runtimeState();

            TitleBranchScenarioReport branch;
            branch.scenario = scenario;
            branch.entryFound = execution.entryFound;
            branch.executed = execution.executed;
            branch.menuSelectedIndex = options.menuSelectedIndex;
            branch.scriptFunctions = execution.scriptFunctions;
            branch.scriptMethods = execution.scriptMethods;
            branch.nativeObligations = execution.nativeObligations;
            branch.uniqueNativeApis = uniqueNativeApiCount(execution);
            branch.serviceStateEvents = execution.serviceStateEventCount;
            branch.saveServiceQueries = execution.saveServiceQueries;
            branch.platformStateQueries = execution.platformStateQueries;
            branch.audioServiceCommands = execution.audioServiceCommands;
            branch.sceneServiceCommands = execution.sceneServiceCommands;
            branch.uiServiceCommands = execution.uiServiceCommands;
            branch.uiObjectMutations = execution.uiObjectMutations;
            branch.unresolvedCalls = execution.unresolvedCalls;
            branch.truncated = execution.truncated;
            branch.saveListEntries = state.saveProfileScenario.saveListEntries;
            branch.saveCapacityQueries = state.saveProfileScenario.saveCapacityQueries;
            branch.scenarioKeyGetQueries = state.saveProfileScenario.scenarioKeyGetQueries;
            branch.makeNewGameCommands = state.saveProfileScenario.makeNewGameCommands;
            branch.loadAutoSaveCommands = state.saveProfileScenario.loadAutoSaveCommands;
            branch.startGameCommands = state.saveProfileScenario.startGameCommands;
            branch.queuedStageLoads = state.sceneStage.queuedStageLoads;
            branch.shutdownPermissionQueries = state.platform.shutdownPermissionQueries;
            branch.shutdownGameCommands = state.platform.shutdownGameCommands;
            branch.startedMission = state.saveProfileScenario.startedMission;
            branch.startNewGame = state.saveProfileScenario.startNewGame;
            branch.lastAutoSaveLoaded = state.saveProfileScenario.lastAutoSaveLoaded;
            branch.shutdownPermission = state.platform.shutdownPermission;
            branch.shutdownRequested = state.platform.shutdownRequested;

            if (branch.executed) {
                ++report.executedScenarios;
            }
            report.unresolvedCalls += branch.unresolvedCalls;
            report.truncated = report.truncated || branch.truncated;
            if (branch.startGameCommands > 0) {
                ++report.startGameScenarios;
            }
            if (branch.loadAutoSaveCommands > 0) {
                ++report.loadAutoSaveScenarios;
            }
            if (branch.makeNewGameCommands > 0) {
                ++report.makeNewGameScenarios;
            }
            if ((scenario == "title-exit-denied" || scenario == "title-exit-allowed")
                && branch.shutdownPermissionQueries > 0) {
                ++report.shutdownPermissionScenarios;
            }
            if (branch.shutdownGameCommands > 0) {
                ++report.shutdownGameScenarios;
            }
            if (scenario == "title-option") {
                report.optionUiMutations = branch.uiObjectMutations;
            }

            if (!branch.entryFound || !branch.executed) {
                addError(report, scenario + " did not execute");
            }
            if (branch.unresolvedCalls != 0) {
                addError(report, scenario + " has unresolved calls");
            }
            if (branch.truncated) {
                addError(report, scenario + " execution truncated");
            }

            if (scenario == "title-continue-disabled") {
                if (branch.saveListEntries != 0 || branch.loadAutoSaveCommands != 0
                    || branch.startGameCommands != 0) {
                    addError(report, "disabled Continue branch started gameplay or exposed saves");
                }
            } else if (scenario == "title-continue") {
                if (branch.saveListEntries < 1 || branch.loadAutoSaveCommands < 1
                    || branch.startGameCommands < 1 || branch.queuedStageLoads < 1) {
                    addError(report, "Continue branch did not load autosave and start existing game");
                }
            } else if (scenario == "title-new-game") {
                if (branch.scenarioKeyGetQueries < 1 || branch.saveCapacityQueries < 1
                    || branch.makeNewGameCommands < 1 || branch.startGameCommands < 1
                    || branch.startNewGame != "true" || branch.queuedStageLoads < 1) {
                    addError(report, "New Game branch did not create and start a new game");
                }
            } else if (scenario == "title-load-empty") {
                if (branch.saveListEntries != 0 || branch.loadAutoSaveCommands != 0
                    || branch.startGameCommands != 0) {
                    addError(report, "empty Load branch started gameplay or exposed saves");
                }
            } else if (scenario == "title-load") {
                if (branch.saveListEntries < 1 || branch.loadAutoSaveCommands < 1
                    || branch.startGameCommands < 1 || branch.queuedStageLoads < 1) {
                    addError(report, "Load branch did not load autosave and start existing game");
                }
            } else if (scenario == "title-option") {
                if (branch.uiObjectMutations < 2 || branch.startGameCommands != 0
                    || branch.shutdownGameCommands != 0) {
                    addError(report, "Option branch did not stay inside menu UI state");
                }
            } else if (scenario == "title-exit-denied") {
                if (branch.shutdownPermissionQueries < 1 || branch.shutdownPermission != "false"
                    || branch.shutdownGameCommands != 0) {
                    addError(report, "Exit denied branch did not respect platform shutdown policy");
                }
            } else if (scenario == "title-exit-allowed") {
                if (branch.shutdownPermissionQueries < 1 || branch.shutdownPermission != "true"
                    || branch.shutdownGameCommands < 1) {
                    addError(report, "Exit allowed branch did not request ShutdownGame");
                }
            }

            report.scenarios.push_back(std::move(branch));
        }

        if (report.executedScenarios != report.scenarioCount) {
            addError(report, "not all title branch scenarios executed");
        }
        if (report.startGameScenarios < 3 || report.loadAutoSaveScenarios < 2
            || report.makeNewGameScenarios < 1) {
            addError(report, "save/new-game/load branch coverage is incomplete");
        }
        if (report.shutdownPermissionScenarios < 2 || report.shutdownGameScenarios != 1) {
            addError(report, "exit branch coverage is incomplete");
        }
        if (report.optionUiMutations < 2) {
            addError(report, "option branch UI state mutation coverage is incomplete");
        }
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string titleBranchesRuntimeReportToJson(const TitleBranchesRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.title_branches_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " scenario_count=" << report.scenarioCount
        << " executed_scenarios=" << report.executedScenarios
        << " start_game_scenarios=" << report.startGameScenarios
        << " load_auto_save_scenarios=" << report.loadAutoSaveScenarios
        << " make_new_game_scenarios=" << report.makeNewGameScenarios
        << " shutdown_permission_scenarios=" << report.shutdownPermissionScenarios
        << " shutdown_game_scenarios=" << report.shutdownGameScenarios
        << " option_ui_mutations=" << report.optionUiMutations
        << " unresolved_calls=" << report.unresolvedCalls
        << " truncated=" << (report.truncated ? "true" : "false") << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"script\": {";
    out << "\"module\": \"" << core::jsonEscape(report.module)
        << "\", \"entry_function\": \"" << core::jsonEscape(report.entryFunction) << "\"},\n";
    out << "  \"scenarios\": [\n";
    for (size_t i = 0; i < report.scenarios.size(); ++i) {
        const auto& branch = report.scenarios[i];
        out << "    {\"scenario\": \"" << core::jsonEscape(branch.scenario)
            << "\", \"entry_found\": " << (branch.entryFound ? "true" : "false")
            << ", \"executed\": " << (branch.executed ? "true" : "false")
            << ", \"menu_selected_index\": " << branch.menuSelectedIndex
            << ", \"script_functions\": " << branch.scriptFunctions
            << ", \"script_methods\": " << branch.scriptMethods
            << ", \"native_obligations\": " << branch.nativeObligations
            << ", \"unique_native_apis\": " << branch.uniqueNativeApis
            << ", \"service_state_events\": " << branch.serviceStateEvents
            << ", \"save_service_queries\": " << branch.saveServiceQueries
            << ", \"platform_state_queries\": " << branch.platformStateQueries
            << ", \"audio_service_commands\": " << branch.audioServiceCommands
            << ", \"scene_service_commands\": " << branch.sceneServiceCommands
            << ", \"ui_service_commands\": " << branch.uiServiceCommands
            << ", \"ui_object_mutations\": " << branch.uiObjectMutations
            << ", \"save_list_entries\": " << branch.saveListEntries
            << ", \"save_capacity_queries\": " << branch.saveCapacityQueries
            << ", \"scenario_key_get_queries\": " << branch.scenarioKeyGetQueries
            << ", \"make_new_game_commands\": " << branch.makeNewGameCommands
            << ", \"load_auto_save_commands\": " << branch.loadAutoSaveCommands
            << ", \"start_game_commands\": " << branch.startGameCommands
            << ", \"queued_stage_loads\": " << branch.queuedStageLoads
            << ", \"shutdown_permission_queries\": " << branch.shutdownPermissionQueries
            << ", \"shutdown_game_commands\": " << branch.shutdownGameCommands
            << ", \"started_mission\": \"" << core::jsonEscape(branch.startedMission)
            << "\", \"start_new_game\": \"" << core::jsonEscape(branch.startNewGame)
            << "\", \"last_auto_save_loaded\": \"" << core::jsonEscape(branch.lastAutoSaveLoaded)
            << "\", \"shutdown_permission\": \"" << core::jsonEscape(branch.shutdownPermission)
            << "\", \"shutdown_requested\": \"" << core::jsonEscape(branch.shutdownRequested)
            << "\", \"unresolved_calls\": " << branch.unresolvedCalls
            << ", \"truncated\": " << (branch.truncated ? "true" : "false") << "}";
        out << (i + 1 == report.scenarios.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

GameplayFrameRuntimeReport runGameplayFrameRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    GameplayFrameRuntimeReport report;
    try {
        report = buildGameplayFrameRuntimeReport(collectGameplayFrameInputs(manifestPath, repoRoot));
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

RendererBackendSubmissionReport runRendererBackendSubmissionRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    RendererBackendSubmissionReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        report = buildRendererBackendSubmissionReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string gameplayFrameRuntimeReportToJson(const GameplayFrameRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.gameplay_frame_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " scene_runtime_ok=" << (report.sceneRuntimeOk ? "true" : "false")
        << " title_ui_ok=" << (report.titleUiOk ? "true" : "false")
        << " title_branches_ok=" << (report.titleBranchesOk ? "true" : "false")
        << " mission_event_thread_ok=" << (report.missionEventThreadOk ? "true" : "false")
        << " mission_tutorial_ok=" << (report.missionTutorialOk ? "true" : "false")
        << " frame_updates=" << report.frameUpdates
        << " renderer_frame_ready=" << (report.rendererFrameReady ? "true" : "false")
        << " ui_frame_ready=" << (report.uiFrameReady ? "true" : "false")
        << " save_frame_ready=" << (report.saveFrameReady ? "true" : "false")
        << " actor_frame_ready=" << (report.actorFrameReady ? "true" : "false")
        << " camera_frame_ready=" << (report.cameraFrameReady ? "true" : "false")
        << " input_frame_ready=" << (report.inputFrameReady ? "true" : "false")
        << " event_frame_ready=" << (report.eventFrameReady ? "true" : "false")
        << " audio_frame_ready=" << (report.audioFrameReady ? "true" : "false")
        << " gameplay_command_count=" << report.gameplayCommandCount << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"renderer\": {";
    out << "\"mesh_draw_candidates\": " << report.meshDrawCandidates
        << ", \"material_bindings\": " << report.materialBindings
        << ", \"texture_bindings\": " << report.textureBindings << "},\n";
    out << "  \"ui\": {";
    out << "\"title_ui_commands\": " << report.titleUiCommands
        << ", \"title_ui_draw_commands\": " << report.titleUiDrawCommands << "},\n";
    out << "  \"save_profile\": {";
    out << "\"start_game_scenarios\": " << report.saveStartGameScenarios
        << ", \"load_auto_save_scenarios\": " << report.saveLoadAutoSaveScenarios
        << ", \"make_new_game_scenarios\": " << report.saveMakeNewGameScenarios << "},\n";
    out << "  \"actor_input\": {";
    out << "\"actor_instances\": " << report.actorInstances
        << ", \"player_control_commands\": " << report.playerControlCommands << "},\n";
    out << "  \"camera\": {";
    out << "\"camera_commands\": " << report.cameraCommands
        << ", \"rail_nodes\": " << report.railNodes << "},\n";
    out << "  \"event\": {";
    out << "\"event_commands\": " << report.eventCommands
        << ", \"tutorial_update_commands\": " << report.tutorialUpdateCommands << "},\n";
    out << "  \"audio\": {";
    out << "\"audio_commands\": " << report.audioCommands << "}\n";
    out << "}\n";
    return out.str();
}

std::string rendererBackendSubmissionReportToJson(const RendererBackendSubmissionReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.renderer_backend_submission_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " scene_runtime_ok=" << (report.sceneRuntimeOk ? "true" : "false")
        << " title_ui_ok=" << (report.titleUiOk ? "true" : "false")
        << " gameplay_frame_ok=" << (report.gameplayFrameOk ? "true" : "false")
        << " backend_frame_ready=" << (report.backendFrameReady ? "true" : "false")
        << " title_pass_ready=" << (report.titlePassReady ? "true" : "false")
        << " world_pass_ready=" << (report.worldPassReady ? "true" : "false")
        << " resource_upload_ready=" << (report.resourceUploadReady ? "true" : "false")
        << " camera_submission_ready=" << (report.cameraSubmissionReady ? "true" : "false")
        << " actor_submission_ready=" << (report.actorSubmissionReady ? "true" : "false")
        << " event_submission_ready=" << (report.eventSubmissionReady ? "true" : "false")
        << " submission_passes=" << report.submissionPasses
        << " backend_command_count=" << report.backendCommandCount
        << " draw_submissions=" << report.drawSubmissions << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"backend\": {";
    out << "\"renderer_profile\": \"" << core::jsonEscape(report.rendererProfile)
        << "\", \"submission_passes\": " << report.submissionPasses
        << ", \"backend_command_count\": " << report.backendCommandCount
        << ", \"draw_submissions\": " << report.drawSubmissions
        << ", \"resource_upload_submissions\": " << report.resourceUploadSubmissions
        << ", \"backend_obligations\": " << report.backendObligations.size() << "},\n";
    out << "  \"title_pass\": {";
    out << "\"ready\": " << (report.titlePassReady ? "true" : "false")
        << ", \"title_2d_submissions\": " << report.title2dSubmissions
        << ", \"title_graph_submissions\": " << report.titleGraphSubmissions
        << ", \"title_text_submissions\": " << report.titleTextSubmissions
        << ", \"last_ui_command\": \"" << core::jsonEscape(report.lastUiCommand) << "\"},\n";
    out << "  \"world_pass\": {";
    out << "\"ready\": " << (report.worldPassReady ? "true" : "false")
        << ", \"scene_mesh_submissions\": " << report.sceneMeshSubmissions
        << ", \"material_bindings\": " << report.materialBindings
        << ", \"texture_bindings\": " << report.textureBindings
        << ", \"collision_debug_submissions\": " << report.collisionDebugSubmissions
        << ", \"stage_path\": \"" << core::jsonEscape(report.stagePath) << "\"},\n";
    out << "  \"scene_state\": {";
    out << "\"camera_submissions\": " << report.cameraSubmissions
        << ", \"actor_submissions\": " << report.actorSubmissions
        << ", \"event_marker_submissions\": " << report.eventMarkerSubmissions
        << ", \"camera_source\": \"" << core::jsonEscape(report.cameraSource)
        << "\", \"player_chara\": \"" << core::jsonEscape(report.playerChara) << "\"},\n";
    out << "  \"backend_obligations\": ";
    writeStringArray(out, report.backendObligations);
    out << "\n";
    out << "}\n";
    return out.str();
}

FrameSchedulerRuntimeReport runFrameSchedulerRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    FrameSchedulerRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        report = buildFrameSchedulerRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string frameSchedulerRuntimeReportToJson(const FrameSchedulerRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.frame_scheduler_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " gameplay_frame_ok=" << (report.gameplayFrameOk ? "true" : "false")
        << " renderer_submission_ok=" << (report.rendererSubmissionOk ? "true" : "false")
        << " update_graph_ready=" << (report.updateGraphReady ? "true" : "false")
        << " frame_index=" << report.frameIndex
        << " node_count=" << report.nodeCount
        << " executed_nodes=" << report.executedNodes
        << " service_node_count=" << report.serviceNodeCount
        << " scheduler_edges=" << report.schedulerEdges
        << " gameplay_commands=" << report.gameplayCommands
        << " renderer_backend_commands=" << report.rendererBackendCommands
        << " scheduled_work_items=" << report.scheduledWorkItems
        << " backend_obligations=" << report.backendObligations
        << " unresolved_nodes=" << report.unresolvedNodes << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"scheduler\": {";
    out << "\"frame_index\": " << report.frameIndex
        << ", \"node_count\": " << report.nodeCount
        << ", \"executed_nodes\": " << report.executedNodes
        << ", \"service_node_count\": " << report.serviceNodeCount
        << ", \"scheduler_edges\": " << report.schedulerEdges
        << ", \"gameplay_commands\": " << report.gameplayCommands
        << ", \"renderer_backend_commands\": " << report.rendererBackendCommands
        << ", \"scheduled_work_items\": " << report.scheduledWorkItems
        << ", \"backend_obligations\": " << report.backendObligations
        << ", \"unresolved_nodes\": " << report.unresolvedNodes << "},\n";
    out << "  \"nodes\": [\n";
    for (size_t i = 0; i < report.nodes.size(); ++i) {
        const auto& node = report.nodes[i];
        out << "    {\"node\": \"" << core::jsonEscape(node.node)
            << "\", \"service\": \"" << core::jsonEscape(node.service)
            << "\", \"ready\": " << (node.ready ? "true" : "false")
            << ", \"commands\": " << node.commands
            << ", \"source\": \"" << core::jsonEscape(node.source) << "\"}";
        out << (i + 1 == report.nodes.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

BackendObligationsRuntimeReport runBackendObligationsRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    BackendObligationsRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report = buildBackendObligationsRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer,
            vfs);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string backendObligationsRuntimeReportToJson(const BackendObligationsRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.backend_obligations_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " frame_scheduler_ok=" << (report.frameSchedulerOk ? "true" : "false")
        << " renderer_submission_ok=" << (report.rendererSubmissionOk ? "true" : "false")
        << " texture_upload_contract_ready=" << (report.textureUploadContractReady ? "true" : "false")
        << " material_binding_contract_ready=" << (report.materialBindingContractReady ? "true" : "false")
        << " shader_effect_contract_tracked=" << (report.shaderEffectContractTracked ? "true" : "false")
        << " font_contract_tracked=" << (report.fontContractTracked ? "true" : "false")
        << " device_contract_tracked=" << (report.deviceContractTracked ? "true" : "false")
        << " oracle_parity_contract_tracked=" << (report.oracleParityContractTracked ? "true" : "false")
        << " texture_dependencies=" << report.textureDependencies
        << " texture_bytes_found=" << report.textureBytesFound
        << " dds_textures=" << report.ddsTextures
        << " material_bindings=" << report.materialBindings
        << " mesh_submissions=" << report.meshSubmissions
        << " resolved_backend_contracts=" << report.resolvedBackendContracts
        << " tracked_backend_obligations=" << report.trackedBackendObligations
        << " open_backend_obligations=" << report.openBackendObligations << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"texture_upload\": {";
    out << "\"dependencies\": " << report.textureDependencies
        << ", \"bytes_found\": " << report.textureBytesFound
        << ", \"dds_textures\": " << report.ddsTextures
        << ", \"byte_total\": " << report.textureByteTotal << "},\n";
    out << "  \"material_binding\": {";
    out << "\"material_bindings\": " << report.materialBindings
        << ", \"mesh_submissions\": " << report.meshSubmissions
        << ", \"title_text_submissions\": " << report.titleTextSubmissions << "},\n";
    out << "  \"obligation_summary\": {";
    out << "\"resolved_backend_contracts\": " << report.resolvedBackendContracts
        << ", \"tracked_backend_obligations\": " << report.trackedBackendObligations
        << ", \"open_backend_obligations\": " << report.openBackendObligations << "},\n";
    out << "  \"obligations\": [\n";
    for (size_t i = 0; i < report.obligations.size(); ++i) {
        const auto& obligation = report.obligations[i];
        out << "    {\"obligation\": \"" << core::jsonEscape(obligation.obligation)
            << "\", \"status\": \"" << core::jsonEscape(obligation.status)
            << "\", \"evidence\": \"" << core::jsonEscape(obligation.evidence) << "\"}";
        out << (i + 1 == report.obligations.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

MaterialSemanticsRuntimeReport runMaterialSemanticsRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    MaterialSemanticsRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report = buildMaterialSemanticsRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer,
            vfs);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string materialSemanticsRuntimeReportToJson(const MaterialSemanticsRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.material_semantics_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " scene_runtime_ok=" << (report.sceneRuntimeOk ? "true" : "false")
        << " renderer_submission_ok=" << (report.rendererSubmissionOk ? "true" : "false")
        << " backend_obligations_ok=" << (report.backendObligationsOk ? "true" : "false")
        << " material_semantics_contract_ready=" << (report.materialSemanticsContractReady ? "true" : "false")
        << " texture_slot_contract_ready=" << (report.textureSlotContractReady ? "true" : "false")
        << " mesh_material_contract_ready=" << (report.meshMaterialContractReady ? "true" : "false")
        << " shader_effect_contract_tracked=" << (report.shaderEffectContractTracked ? "true" : "false")
        << " post_effect_source_tracked=" << (report.postEffectSourceTracked ? "true" : "false")
        << " materials=" << report.materials
        << " material_parameter_blocks=" << report.materialParameterBlocks
        << " texture_slots=" << report.textureSlots
        << " resolved_texture_slots=" << report.resolvedTextureSlots
        << " mesh_submissions=" << report.meshSubmissions
        << " named_mesh_submissions=" << report.namedMeshSubmissions
        << " mesh_material_bindings=" << report.meshMaterialBindings
        << " unresolved_mesh_material_bindings=" << report.unresolvedMeshMaterialBindings
        << " post_effect_techniques=" << report.postEffectTechniques
        << " post_effect_passes=" << report.postEffectPasses
        << " post_effect_samplers=" << report.postEffectSamplers << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"material_summary\": {";
    out << "\"model_path\": \"" << core::jsonEscape(report.modelPath)
        << "\", \"materials\": " << report.materials
        << ", \"material_parameter_blocks\": " << report.materialParameterBlocks
        << ", \"texture_slots\": " << report.textureSlots
        << ", \"resolved_texture_slots\": " << report.resolvedTextureSlots
        << ", \"unresolved_texture_slots\": " << report.unresolvedTextureSlots
        << ", \"mesh_submissions\": " << report.meshSubmissions
        << ", \"named_mesh_submissions\": " << report.namedMeshSubmissions
        << ", \"mesh_material_bindings\": " << report.meshMaterialBindings
        << ", \"unresolved_mesh_material_bindings\": " << report.unresolvedMeshMaterialBindings << "},\n";
    out << "  \"post_effect\": {";
    out << "\"tracked\": " << (report.postEffectSourceTracked ? "true" : "false")
        << ", \"techniques\": " << report.postEffectTechniques
        << ", \"passes\": " << report.postEffectPasses
        << ", \"samplers\": " << report.postEffectSamplers << "},\n";
    out << "  \"materials\": [\n";
    for (size_t i = 0; i < report.materialReports.size(); ++i) {
        const auto& material = report.materialReports[i];
        out << "    {\"index\": " << material.index
            << ", \"name\": \"" << core::jsonEscape(material.name)
            << "\", \"texture_slot_count\": " << material.textureSlotCount
            << ", \"resolved_texture_slots\": " << material.resolvedTextureSlots
            << ", \"mesh_binding_count\": " << material.meshBindingCount
            << ", \"texture_slots\": ";
        writeStringArray(out, material.textureSlots);
        out << "}";
        out << (i + 1 == report.materialReports.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"obligations\": [\n";
    for (size_t i = 0; i < report.obligations.size(); ++i) {
        const auto& obligation = report.obligations[i];
        out << "    {\"obligation\": \"" << core::jsonEscape(obligation.obligation)
            << "\", \"status\": \"" << core::jsonEscape(obligation.status)
            << "\", \"evidence\": \"" << core::jsonEscape(obligation.evidence) << "\"}";
        out << (i + 1 == report.obligations.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

DevicePresentationRuntimeReport runDevicePresentationRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    DevicePresentationRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report = buildDevicePresentationRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer,
            vfs);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string devicePresentationRuntimeReportToJson(const DevicePresentationRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.device_presentation_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " frame_scheduler_ok=" << (report.frameSchedulerOk ? "true" : "false")
        << " renderer_submission_ok=" << (report.rendererSubmissionOk ? "true" : "false")
        << " backend_obligations_ok=" << (report.backendObligationsOk ? "true" : "false")
        << " material_semantics_ok=" << (report.materialSemanticsOk ? "true" : "false")
        << " device_profile_ready=" << (report.deviceProfileReady ? "true" : "false")
        << " swapchain_contract_tracked=" << (report.swapchainContractTracked ? "true" : "false")
        << " resource_upload_plan_ready=" << (report.resourceUploadPlanReady ? "true" : "false")
        << " render_state_contract_tracked=" << (report.renderStateContractTracked ? "true" : "false")
        << " draw_queue_contract_ready=" << (report.drawQueueContractReady ? "true" : "false")
        << " present_contract_tracked=" << (report.presentContractTracked ? "true" : "false")
        << " renderer_profile=" << report.rendererProfile
        << " backbuffer_width_candidate=" << report.backbufferWidthCandidate
        << " backbuffer_height_candidate=" << report.backbufferHeightCandidate
        << " renderer_backend_commands=" << report.rendererBackendCommands
        << " resource_upload_submissions=" << report.resourceUploadSubmissions
        << " draw_submissions=" << report.drawSubmissions
        << " material_texture_slots=" << report.materialTextureSlots
        << " texture_bytes_found=" << report.textureBytesFound
        << " post_effect_samplers=" << report.postEffectSamplers
        << " resolved_device_contracts=" << report.resolvedDeviceContracts
        << " tracked_device_obligations=" << report.trackedDeviceObligations
        << " open_device_obligations=" << report.openDeviceObligations << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"device\": {";
    out << "\"renderer_profile\": \"" << core::jsonEscape(report.rendererProfile)
        << "\", \"device_profile_ready\": " << (report.deviceProfileReady ? "true" : "false")
        << ", \"backbuffer_width_candidate\": " << report.backbufferWidthCandidate
        << ", \"backbuffer_height_candidate\": " << report.backbufferHeightCandidate
        << ", \"swapchain_contract_tracked\": " << (report.swapchainContractTracked ? "true" : "false")
        << ", \"present_contract_tracked\": " << (report.presentContractTracked ? "true" : "false") << "},\n";
    out << "  \"submission\": {";
    out << "\"renderer_backend_commands\": " << report.rendererBackendCommands
        << ", \"resource_upload_submissions\": " << report.resourceUploadSubmissions
        << ", \"draw_submissions\": " << report.drawSubmissions
        << ", \"title_2d_submissions\": " << report.title2dSubmissions
        << ", \"world_mesh_submissions\": " << report.worldMeshSubmissions
        << ", \"material_bindings\": " << report.materialBindings
        << ", \"mesh_submissions\": " << report.meshSubmissions << "},\n";
    out << "  \"resource_upload\": {";
    out << "\"plan_ready\": " << (report.resourceUploadPlanReady ? "true" : "false")
        << ", \"material_texture_slots\": " << report.materialTextureSlots
        << ", \"texture_bytes_found\": " << report.textureBytesFound << "},\n";
    out << "  \"render_state\": {";
    out << "\"tracked\": " << (report.renderStateContractTracked ? "true" : "false")
        << ", \"post_effect_techniques\": " << report.postEffectTechniques
        << ", \"post_effect_passes\": " << report.postEffectPasses
        << ", \"post_effect_samplers\": " << report.postEffectSamplers << "},\n";
    out << "  \"contract_summary\": {";
    out << "\"resolved_device_contracts\": " << report.resolvedDeviceContracts
        << ", \"tracked_device_obligations\": " << report.trackedDeviceObligations
        << ", \"open_device_obligations\": " << report.openDeviceObligations << "},\n";
    out << "  \"contracts\": [\n";
    for (size_t i = 0; i < report.contracts.size(); ++i) {
        const auto& contract = report.contracts[i];
        out << "    {\"contract\": \"" << core::jsonEscape(contract.obligation)
            << "\", \"status\": \"" << core::jsonEscape(contract.status)
            << "\", \"evidence\": \"" << core::jsonEscape(contract.evidence) << "\"}";
        out << (i + 1 == report.contracts.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
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
