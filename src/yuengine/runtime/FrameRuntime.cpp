#include "yuengine/runtime/FrameRuntime.h"

#include "yuengine/core/Json.h"
#include "yuengine/native/NativeRegistry.h"
#include "yuengine/project/ProjectManifest.h"
#include "yuengine/script/ScriptRuntime.h"
#include "yuengine/script/SqasmModule.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
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

void addError(TextureUploadRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(BackendStateRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(BackendResourceAllocationRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(BackendDeviceExecutionRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(BackendPresentationOracleRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(BackendPlatformBridgeRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(BackendExecutorRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(BackendDeviceAdapterRuntimeReport& report, const std::string& message)
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

uint32_t readLe32(const std::vector<std::byte>& bytes, size_t offset)
{
    if (offset + 4 > bytes.size()) {
        return 0;
    }
    return static_cast<uint32_t>(std::to_integer<unsigned char>(bytes[offset]))
        | (static_cast<uint32_t>(std::to_integer<unsigned char>(bytes[offset + 1])) << 8)
        | (static_cast<uint32_t>(std::to_integer<unsigned char>(bytes[offset + 2])) << 16)
        | (static_cast<uint32_t>(std::to_integer<unsigned char>(bytes[offset + 3])) << 24);
}

std::string readDdsFourCc(const std::vector<std::byte>& bytes, size_t offset)
{
    std::string value;
    for (size_t i = 0; i < 4 && offset + i < bytes.size(); ++i) {
        const char ch = static_cast<char>(std::to_integer<unsigned char>(bytes[offset + i]));
        if (ch == '\0') {
            break;
        }
        value.push_back(ch);
    }
    return value;
}

int cubeFaceCountFromCaps2(uint32_t caps2)
{
    constexpr uint32_t ddsCaps2Cubemap = 0x00000200;
    constexpr uint32_t ddsCaps2CubemapPositiveX = 0x00000400;
    constexpr uint32_t ddsCaps2CubemapNegativeX = 0x00000800;
    constexpr uint32_t ddsCaps2CubemapPositiveY = 0x00001000;
    constexpr uint32_t ddsCaps2CubemapNegativeY = 0x00002000;
    constexpr uint32_t ddsCaps2CubemapPositiveZ = 0x00004000;
    constexpr uint32_t ddsCaps2CubemapNegativeZ = 0x00008000;

    if ((caps2 & ddsCaps2Cubemap) == 0) {
        return 1;
    }

    int faces = 0;
    for (const auto faceBit : {
             ddsCaps2CubemapPositiveX,
             ddsCaps2CubemapNegativeX,
             ddsCaps2CubemapPositiveY,
             ddsCaps2CubemapNegativeY,
             ddsCaps2CubemapPositiveZ,
             ddsCaps2CubemapNegativeZ,
         }) {
        if ((caps2 & faceBit) != 0) {
            ++faces;
        }
    }
    return faces == 0 ? 6 : faces;
}

int64_t ddsCompressedPayloadBytes(int width, int height, int mipCount, int blockBytes, int faces)
{
    if (width <= 0 || height <= 0 || mipCount <= 0 || blockBytes <= 0 || faces <= 0) {
        return 0;
    }

    int64_t total = 0;
    int levelWidth = width;
    int levelHeight = height;
    for (int level = 0; level < mipCount; ++level) {
        const int blocksWide = std::max(1, (levelWidth + 3) / 4);
        const int blocksHigh = std::max(1, (levelHeight + 3) / 4);
        total += static_cast<int64_t>(blocksWide) * static_cast<int64_t>(blocksHigh) * blockBytes;
        levelWidth = std::max(1, levelWidth / 2);
        levelHeight = std::max(1, levelHeight / 2);
    }
    return total * faces;
}

bool decodeDdsUploadRecord(
    const std::string& path,
    const std::vector<std::byte>& bytes,
    int materialConsumerCount,
    const std::string& role,
    TextureUploadRecordReport& record)
{
    record.path = path;
    record.role = role;
    record.found = true;
    record.byteSize = static_cast<int64_t>(bytes.size());
    record.materialConsumerCount = materialConsumerCount;

    if (!isDdsPayload(bytes) || bytes.size() < 128) {
        return false;
    }

    const uint32_t headerSize = readLe32(bytes, 4);
    const uint32_t height = readLe32(bytes, 12);
    const uint32_t width = readLe32(bytes, 16);
    const uint32_t mipCount = readLe32(bytes, 28);
    const uint32_t pixelFormatSize = readLe32(bytes, 76);
    const std::string fourCc = readDdsFourCc(bytes, 84);
    const uint32_t caps2 = readLe32(bytes, 112);

    record.width = static_cast<int>(width);
    record.height = static_cast<int>(height);
    record.mipCount = static_cast<int>(mipCount);
    record.format = fourCc;
    record.cubeFaces = cubeFaceCountFromCaps2(caps2);
    record.cubeMap = record.cubeFaces > 1;
    record.payloadBytes = static_cast<int64_t>(bytes.size()) - 128;

    if (fourCc == "DXT1") {
        record.blockBytes = 8;
    } else if (fourCc == "DXT3" || fourCc == "DXT5") {
        record.blockBytes = 16;
    }

    record.expectedPayloadBytes =
        ddsCompressedPayloadBytes(record.width, record.height, record.mipCount, record.blockBytes, record.cubeFaces);
    record.compressedPayloadMatches =
        record.expectedPayloadBytes > 0 && record.expectedPayloadBytes == record.payloadBytes;
    record.validDds = headerSize == 124 && pixelFormatSize == 32 && record.width > 0 && record.height > 0
        && record.mipCount > 0 && record.blockBytes > 0 && record.compressedPayloadMatches;
    return record.validDds;
}

struct TextureConsumerSummary {
    int count = 0;
    std::string role;
};

std::map<std::string, TextureConsumerSummary> materialTextureConsumerMap(const StageGraphRuntimeHandle& stage)
{
    std::map<std::string, TextureConsumerSummary> consumers;
    for (const auto& material : stage.materials) {
        for (const auto& slot : material.textureSlots) {
            const std::string key = resource::normalizeResourcePath(slot.path);
            auto& summary = consumers[key];
            ++summary.count;
            if (summary.role.empty()) {
                summary.role = slot.slot;
            }
        }
    }
    return consumers;
}

std::string inferTextureUploadRole(
    const std::string& path,
    const TextureConsumerSummary& consumer)
{
    if (!consumer.role.empty()) {
        return consumer.role;
    }
    const std::string normalized = resource::normalizeResourcePath(path);
    if (normalized.rfind("cubeenvmap/", 0) == 0) {
        return "cube_environment_candidate";
    }
    if (normalized.find("_n.dds") != std::string::npos) {
        return "normal_candidate";
    }
    if (normalized.find("_s.dds") != std::string::npos) {
        return "specular_candidate";
    }
    if (normalized.find("_e.dds") != std::string::npos) {
        return "emissive_candidate";
    }
    if (normalized.find("_lm.dds") != std::string::npos) {
        return "lightmap_candidate";
    }
    return "base_color_candidate";
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

std::string trimText(const std::string& value)
{
    size_t begin = 0;
    while (begin < value.size() && (value[begin] == ' ' || value[begin] == '\t'
        || value[begin] == '\r' || value[begin] == '\n')) {
        ++begin;
    }
    size_t end = value.size();
    while (end > begin && (value[end - 1] == ' ' || value[end - 1] == '\t'
        || value[end - 1] == '\r' || value[end - 1] == '\n')) {
        --end;
    }
    return value.substr(begin, end - begin);
}

std::string readIdentifierAfter(const std::string& text, size_t offset)
{
    while (offset < text.size() && (text[offset] == ' ' || text[offset] == '\t')) {
        ++offset;
    }
    const size_t begin = offset;
    while (offset < text.size()) {
        const char ch = text[offset];
        if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')
                || ch == '_')) {
            break;
        }
        ++offset;
    }
    return text.substr(begin, offset - begin);
}

size_t findMatchingBrace(const std::string& text, size_t openBrace)
{
    int depth = 0;
    for (size_t offset = openBrace; offset < text.size(); ++offset) {
        if (text[offset] == '{') {
            ++depth;
        } else if (text[offset] == '}') {
            --depth;
            if (depth == 0) {
                return offset;
            }
        }
    }
    return std::string::npos;
}

std::string extractFxAssignment(const std::string& block, const std::string& key)
{
    size_t offset = block.find(key);
    if (offset == std::string::npos) {
        return {};
    }
    offset = block.find('=', offset + key.size());
    if (offset == std::string::npos) {
        return {};
    }
    const size_t begin = offset + 1;
    size_t end = block.find(';', begin);
    if (end == std::string::npos) {
        end = block.size();
    }
    std::string value = trimText(block.substr(begin, end - begin));
    if (value.size() >= 2 && value.front() == '<' && value.back() == '>') {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

std::string shaderProfileFromAssignment(const std::string& block, const std::string& key)
{
    const std::string value = extractFxAssignment(block, key);
    if (value.find("vs_3_0") != std::string::npos) {
        return "vs_3_0";
    }
    if (value.find("ps_3_0") != std::string::npos) {
        return "ps_3_0";
    }
    return {};
}

std::vector<BackendSamplerStateRecord> parseSmaaSamplerStateRecords(const std::string& text)
{
    std::vector<BackendSamplerStateRecord> records;
    const std::string marker = "\nsampler2D ";
    size_t offset = 0;
    while ((offset = text.find(marker, offset)) != std::string::npos) {
        const size_t nameBegin = offset + marker.size();
        const std::string name = readIdentifierAfter(text, nameBegin);
        const size_t openBrace = text.find('{', nameBegin);
        if (name.empty() || openBrace == std::string::npos) {
            offset = nameBegin;
            continue;
        }
        const size_t closeBrace = findMatchingBrace(text, openBrace);
        if (closeBrace == std::string::npos) {
            offset = openBrace + 1;
            continue;
        }
        const std::string block = text.substr(openBrace + 1, closeBrace - openBrace - 1);
        BackendSamplerStateRecord record;
        record.name = name;
        record.texture = extractFxAssignment(block, "Texture");
        record.addressU = extractFxAssignment(block, "AddressU");
        record.addressV = extractFxAssignment(block, "AddressV");
        record.addressW = extractFxAssignment(block, "AddressW");
        record.mipFilter = extractFxAssignment(block, "MipFilter");
        record.minFilter = extractFxAssignment(block, "MinFilter");
        record.magFilter = extractFxAssignment(block, "MagFilter");
        record.srgbTexture = extractFxAssignment(block, "SRGBTexture");
        record.ready = !record.name.empty() && !record.texture.empty() && !record.addressU.empty()
            && !record.addressV.empty() && !record.mipFilter.empty() && !record.minFilter.empty()
            && !record.magFilter.empty() && !record.srgbTexture.empty();
        records.push_back(std::move(record));
        offset = closeBrace + 1;
    }
    return records;
}

std::vector<BackendPassStateRecord> parseSmaaPassStateRecords(const std::string& text)
{
    std::vector<BackendPassStateRecord> records;
    const std::string passMarker = "\n    pass ";
    size_t offset = 0;
    while ((offset = text.find(passMarker, offset)) != std::string::npos) {
        const size_t nameBegin = offset + passMarker.size();
        const std::string passName = readIdentifierAfter(text, nameBegin);
        const size_t openBrace = text.find('{', nameBegin);
        if (passName.empty() || openBrace == std::string::npos) {
            offset = nameBegin;
            continue;
        }
        const size_t closeBrace = findMatchingBrace(text, openBrace);
        if (closeBrace == std::string::npos) {
            offset = openBrace + 1;
            continue;
        }
        std::string techniqueName;
        const size_t techniquePos = text.rfind("\ntechnique ", offset);
        if (techniquePos != std::string::npos) {
            techniqueName = readIdentifierAfter(text, techniquePos + std::string("\ntechnique ").size());
        }
        const std::string block = text.substr(openBrace + 1, closeBrace - openBrace - 1);
        BackendPassStateRecord record;
        record.technique = techniqueName;
        record.pass = passName;
        record.vertexShaderProfile = shaderProfileFromAssignment(block, "VertexShader");
        record.pixelShaderProfile = shaderProfileFromAssignment(block, "PixelShader");
        record.zEnable = extractFxAssignment(block, "ZEnable");
        record.srgbWriteEnable = extractFxAssignment(block, "SRGBWriteEnable");
        record.alphaBlendEnable = extractFxAssignment(block, "AlphaBlendEnable");
        record.alphaTestEnable = extractFxAssignment(block, "AlphaTestEnable");
        record.stencilEnable = extractFxAssignment(block, "StencilEnable");
        record.stencilPass = extractFxAssignment(block, "StencilPass");
        record.stencilFunc = extractFxAssignment(block, "StencilFunc");
        record.stencilRef = extractFxAssignment(block, "StencilRef");
        record.ready = !record.technique.empty() && !record.pass.empty()
            && record.vertexShaderProfile == "vs_3_0" && record.pixelShaderProfile == "ps_3_0"
            && !record.zEnable.empty() && !record.srgbWriteEnable.empty()
            && !record.alphaBlendEnable.empty() && !record.alphaTestEnable.empty()
            && !record.stencilEnable.empty();
        records.push_back(std::move(record));
        offset = closeBrace + 1;
    }
    return records;
}

std::vector<std::string> parseSmaaTextureDeclarations(const std::string& text)
{
    std::vector<std::string> declarations;
    const std::string marker = "\ntexture2D ";
    size_t offset = 0;
    while ((offset = text.find(marker, offset)) != std::string::npos) {
        const size_t nameBegin = offset + marker.size();
        const std::string name = readIdentifierAfter(text, nameBegin);
        if (!name.empty()) {
            declarations.push_back(name);
        }
        offset = nameBegin;
    }
    return declarations;
}

std::string d3dFormatFromDdsFourCc(const std::string& fourCc)
{
    if (fourCc == "DXT1") {
        return "D3DFMT_DXT1";
    }
    if (fourCc == "DXT5") {
        return "D3DFMT_DXT5";
    }
    return "unsupported";
}

int bytesPerPixelForDdsLookup(const std::string& d3dFormat)
{
    if (d3dFormat == "D3DFMT_L8") {
        return 1;
    }
    if (d3dFormat == "D3DFMT_A8L8") {
        return 2;
    }
    return 0;
}

std::string d3dLookupFormatFromDdsHeader(const std::vector<std::byte>& bytes)
{
    constexpr uint32_t ddsPfAlphaPixels = 0x00000001;
    constexpr uint32_t ddsPfLuminance = 0x00020000;

    const std::string fourCc = readDdsFourCc(bytes, 84);
    const uint32_t flags = readLe32(bytes, 80);
    const uint32_t rgbBitCount = readLe32(bytes, 88);
    const uint32_t alphaMask = readLe32(bytes, 104);
    if (!fourCc.empty() || (flags & ddsPfLuminance) == 0) {
        return {};
    }
    if (rgbBitCount == 8) {
        return "D3DFMT_L8";
    }
    if (rgbBitCount == 16 && (flags & ddsPfAlphaPixels) != 0 && alphaMask != 0) {
        return "D3DFMT_A8L8";
    }
    return {};
}

BackendResourceAllocationRecord makeSmaaLookupAllocationRecord(
    const std::string& name,
    const std::string& path,
    const std::vector<std::byte>& bytes)
{
    BackendResourceAllocationRecord record;
    record.name = name;
    record.path = path;
    record.resourceKind = "smaa_lookup_texture";
    record.usage = "post_effect_lookup";
    record.status = "blocked";
    record.byteSize = static_cast<int64_t>(bytes.size());
    record.payloadBytes = bytes.size() >= 128 ? static_cast<int64_t>(bytes.size()) - 128 : 0;

    if (!isDdsPayload(bytes) || bytes.size() < 128) {
        return record;
    }

    const uint32_t headerSize = readLe32(bytes, 4);
    const uint32_t height = readLe32(bytes, 12);
    const uint32_t width = readLe32(bytes, 16);
    const uint32_t mipCount = readLe32(bytes, 28);
    const uint32_t pixelFormatSize = readLe32(bytes, 76);
    record.width = static_cast<int>(width);
    record.height = static_cast<int>(height);
    record.mipLevels = static_cast<int>(mipCount == 0 ? 1 : mipCount);
    record.format = d3dLookupFormatFromDdsHeader(bytes);
    record.expectedPayloadBytes =
        static_cast<int64_t>(record.width) * record.height * bytesPerPixelForDdsLookup(record.format);
    record.ready = headerSize == 124 && pixelFormatSize == 32 && record.width > 0 && record.height > 0
        && record.mipLevels == 1 && !record.format.empty() && record.payloadBytes == record.expectedPayloadBytes;
    record.status = record.ready ? "contract_ready" : "blocked";
    return record;
}

BackendResourceAllocationRecord makeStageTextureAllocationRecord(const TextureUploadRecordReport& upload)
{
    BackendResourceAllocationRecord record;
    record.name = resource::normalizeResourcePath(upload.path);
    record.path = upload.path;
    record.resourceKind = upload.cubeMap ? "cube_texture" : "texture_2d";
    record.usage = upload.cubeMap ? "stage_environment_cube" : "stage_material_texture";
    record.format = d3dFormatFromDdsFourCc(upload.format);
    record.status = upload.validDds && record.format != "unsupported" ? "contract_ready" : "blocked";
    record.width = upload.width;
    record.height = upload.height;
    record.mipLevels = upload.mipCount;
    record.cubeFaces = upload.cubeFaces;
    record.materialConsumerCount = upload.materialConsumerCount;
    record.byteSize = upload.byteSize;
    record.payloadBytes = upload.payloadBytes;
    record.expectedPayloadBytes = upload.expectedPayloadBytes;
    record.ready = record.status == "contract_ready";
    return record;
}

BackendResourceAllocationRecord makeTransientSurfaceAllocationRecord(
    const std::string& name,
    int width,
    int height)
{
    BackendResourceAllocationRecord record;
    record.name = name;
    record.resourceKind = "smaa_transient_surface";
    record.usage = "post_effect_input_or_intermediate";
    record.format = "unknown_d3d9_surface_format";
    record.status = "tracked_open";
    record.width = width;
    record.height = height;
    record.mipLevels = 1;
    record.ready = false;
    return record;
}

BackendResourceAllocationRecord makeFontAtlasAllocationPlaceholder()
{
    BackendResourceAllocationRecord record;
    record.name = "font_atlas_placeholder";
    record.resourceKind = "font_atlas_texture";
    record.usage = "title_text";
    record.format = "unknown_font_atlas_format";
    record.status = "tracked_open";
    record.mipLevels = 1;
    record.ready = false;
    return record;
}

BackendDeviceResourceExecutionRecord makeDeviceResourceExecutionRecord(
    const BackendResourceAllocationRecord& allocation)
{
    BackendDeviceResourceExecutionRecord record;
    record.name = allocation.name;
    record.path = allocation.path;
    record.resourceKind = allocation.resourceKind;
    record.format = allocation.format;
    record.status = allocation.status;
    record.width = allocation.width;
    record.height = allocation.height;
    record.mipLevels = allocation.mipLevels;
    record.cubeFaces = allocation.cubeFaces;
    record.materialBindingSlots = allocation.materialConsumerCount;
    record.byteSize = allocation.byteSize;
    record.payloadBytes = allocation.payloadBytes;
    record.ready = allocation.ready;
    record.subresourceCount = allocation.ready ? allocation.mipLevels * allocation.cubeFaces : 0;

    if (allocation.resourceKind == "cube_texture") {
        record.operation = "CreateCubeTexture";
    } else if (allocation.resourceKind == "texture_2d" || allocation.resourceKind == "smaa_lookup_texture") {
        record.operation = "CreateTexture";
    } else if (allocation.resourceKind == "smaa_transient_surface" && allocation.name == "depthTex2D") {
        record.operation = "CreateDepthStencilSurfaceCandidate";
    } else if (allocation.resourceKind == "smaa_transient_surface") {
        record.operation = "CreateRenderTargetCandidate";
    } else if (allocation.resourceKind == "font_atlas_texture") {
        record.operation = "CreateTextureCandidate";
    } else {
        record.operation = "CreateResourceCandidate";
    }

    return record;
}

BackendDeviceStateBindingRecord makeMaterialTextureBindingRecord(
    const BackendResourceAllocationRecord& allocation)
{
    BackendDeviceStateBindingRecord record;
    record.name = allocation.name;
    record.operation = "SetTextureCandidate";
    record.target = "material_texture_slot";
    record.source = allocation.path;
    record.status = "tracked_open";
    record.bindingSlots = allocation.materialConsumerCount;
    record.ready = false;
    return record;
}

BackendDeviceStateBindingRecord makeSmaaSamplerTextureBindingRecord(
    const BackendSamplerStateRecord& sampler)
{
    BackendDeviceStateBindingRecord record;
    record.name = sampler.name;
    record.operation = sampler.texture == "areaTex2D" || sampler.texture == "searchTex2D"
        ? "SetTexture"
        : "SetTextureCandidate";
    record.target = sampler.texture;
    record.source = "SMAA.fx sampler2D";
    record.status = record.operation == "SetTexture" ? "contract_ready" : "tracked_open";
    record.bindingSlots = 1;
    record.ready = record.status == "contract_ready";
    return record;
}

BackendDeviceStateBindingRecord makeSamplerStateBindingRecord(const BackendSamplerStateRecord& sampler)
{
    BackendDeviceStateBindingRecord record;
    record.name = sampler.name;
    record.operation = "SetSamplerState";
    record.target = sampler.texture;
    record.source = "SMAA.fx sampler state";
    record.status = sampler.ready ? "contract_ready" : "blocked";
    record.bindingSlots = 1;
    record.ready = sampler.ready;
    return record;
}

BackendDeviceStateBindingRecord makeRenderStateBindingRecord(const BackendPassStateRecord& pass)
{
    BackendDeviceStateBindingRecord record;
    record.name = pass.technique + "." + pass.pass;
    record.operation = "SetRenderStateBundle";
    record.target = pass.pass;
    record.source = "SMAA.fx pass state";
    record.status = pass.ready ? "contract_ready" : "blocked";
    record.bindingSlots = 1;
    record.ready = pass.ready;
    return record;
}

BackendPresentationOracleRecord makePresentationOracleRecord(
    const std::string& name,
    const std::string& operation,
    const std::string& source,
    const std::string& status,
    int width,
    int height,
    int linkedRecordCount)
{
    BackendPresentationOracleRecord record;
    record.name = name;
    record.operation = operation;
    record.source = source;
    record.status = status;
    record.width = width;
    record.height = height;
    record.linkedRecordCount = linkedRecordCount;
    record.ready = status == "contract_ready";
    return record;
}

BackendPlatformBridgeCallRecord makePlatformBridgeCallRecord(
    const std::string& name,
    const std::string& api,
    const std::string& source,
    const std::string& status,
    int inputRecords,
    int readyInputs,
    int trackedOpenInputs,
    int callCount,
    int width,
    int height)
{
    BackendPlatformBridgeCallRecord record;
    record.name = name;
    record.api = api;
    record.source = source;
    record.status = status;
    record.inputRecords = inputRecords;
    record.readyInputs = readyInputs;
    record.trackedOpenInputs = trackedOpenInputs;
    record.callCount = callCount;
    record.width = width;
    record.height = height;
    record.ready = status == "contract_ready";
    return record;
}

std::string executorObligationForBridgeRecord(const BackendPlatformBridgeCallRecord& record)
{
    if (record.name == "platform_window_surface") {
        return "requires a concrete HWND/window surface before execution";
    }
    if (record.name == "d3d9_interface_creation" || record.name == "d3d9_device_creation") {
        return "requires Direct3DCreate9/CreateDevice concrete backend implementation";
    }
    if (record.name == "d3d9_draw_submission_queue") {
        return "requires vertex/index buffer binding and draw-call executor";
    }
    if (record.name == "d3d9_present_call") {
        return "requires an IDirect3DDevice9 Present-capable device";
    }
    if (record.name == "frame_capture_oracle_queue") {
        return "requires captured YuEngine frame artifact and original oracle trace";
    }
    return "diagnostic adapter accepted the ready bridge call batch";
}

BackendExecutorResultRecord makeExecutorResultRecord(
    const BackendPlatformBridgeCallRecord& source,
    const std::string& adapter,
    const std::string& resultStatus)
{
    BackendExecutorResultRecord result;
    result.name = source.name + "_result";
    result.sourceBridgeRecord = source.name;
    result.api = source.api;
    result.adapter = adapter;
    result.resultStatus = resultStatus;
    result.obligation = executorObligationForBridgeRecord(source);
    result.inputRecords = source.inputRecords;
    result.callCount = source.callCount;
    result.executedCalls = resultStatus == "diagnostic_success" ? source.callCount : 0;
    result.preservedOpenCalls = resultStatus == "tracked_open" ? source.callCount : 0;
    result.ready = resultStatus == "diagnostic_success";
    return result;
}

std::string deviceAdapterStageForExecutorResult(const BackendExecutorResultRecord& result)
{
    if (result.sourceBridgeRecord == "diagnostic_backend_bridge") {
        return "diagnostic_context";
    }
    if (result.sourceBridgeRecord == "platform_window_surface") {
        return "window_surface";
    }
    if (result.sourceBridgeRecord == "d3d9_interface_creation") {
        return "d3d9_interface";
    }
    if (result.sourceBridgeRecord == "d3d9_device_creation") {
        return "d3d9_device";
    }
    if (result.sourceBridgeRecord == "d3d9_resource_creation_queue"
        || result.sourceBridgeRecord == "d3d9_texture_upload_queue"
        || result.sourceBridgeRecord == "d3d9_state_binding_queue") {
        return "downstream_resource_queue";
    }
    return "downstream_render_queue";
}

std::string deviceAdapterStatusForExecutorResult(const BackendExecutorResultRecord& result)
{
    if (result.sourceBridgeRecord == "diagnostic_backend_bridge") {
        return "contract_ready";
    }
    if (result.sourceBridgeRecord == "platform_window_surface"
        || result.sourceBridgeRecord == "d3d9_interface_creation"
        || result.sourceBridgeRecord == "d3d9_device_creation") {
        return "tracked_open";
    }
    return "blocked_until_device";
}

std::string deviceAdapterObligationForExecutorResult(const BackendExecutorResultRecord& result)
{
    if (result.sourceBridgeRecord == "diagnostic_backend_bridge") {
        return "executor diagnostic context is preserved for backend adapter dispatch";
    }
    if (result.sourceBridgeRecord == "platform_window_surface") {
        return "requires a real HWND/window surface owned by the YuEngine runtime";
    }
    if (result.sourceBridgeRecord == "d3d9_interface_creation") {
        return "requires a real Direct3DCreate9 interface result";
    }
    if (result.sourceBridgeRecord == "d3d9_device_creation") {
        return "requires a real IDirect3DDevice9 created with the recovered backbuffer contract";
    }
    if (result.sourceBridgeRecord == "d3d9_resource_creation_queue") {
        return "resource creation is blocked until a concrete IDirect3DDevice9 handle exists";
    }
    if (result.sourceBridgeRecord == "d3d9_texture_upload_queue") {
        return "texture uploads are blocked until concrete D3D texture resources exist";
    }
    if (result.sourceBridgeRecord == "d3d9_state_binding_queue") {
        return "state binding is blocked until a concrete D3D device handle exists";
    }
    if (result.sourceBridgeRecord == "d3d9_draw_submission_queue") {
        return "draw execution is blocked until concrete buffers and device state exist";
    }
    if (result.sourceBridgeRecord == "d3d9_present_call") {
        return "present is blocked until a concrete device and swapchain exist";
    }
    return "capture/oracle work is blocked until YuEngine can present a concrete frame";
}

BackendDeviceAdapterRecord makeDeviceAdapterRecord(
    const BackendExecutorResultRecord& source,
    int width,
    int height)
{
    BackendDeviceAdapterRecord record;
    record.name = source.sourceBridgeRecord + "_adapter";
    record.sourceExecutorResult = source.name;
    record.sourceBridgeRecord = source.sourceBridgeRecord;
    record.api = source.api;
    record.adapter = "real_hwnd_d3d9_device_adapter";
    record.stage = deviceAdapterStageForExecutorResult(source);
    record.status = deviceAdapterStatusForExecutorResult(source);
    record.obligation = deviceAdapterObligationForExecutorResult(source);
    record.inputRecords = source.inputRecords;
    record.callCount = source.callCount;
    record.width = width;
    record.height = height;
    record.inheritedExecutedCalls = source.executedCalls;
    record.inheritedPreservedOpenCalls = source.preservedOpenCalls;
    record.realExecutedCalls = 0;
    record.blockedRealCalls = record.status == "contract_ready" ? 0 : source.callCount;
    record.sourceReady = source.ready;
    record.deviceHandleRequired = record.status != "contract_ready";
    record.deviceHandleReady = false;
    return record;
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

TextureUploadRuntimeReport buildTextureUploadRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile,
    const resource::VirtualFileSystem& vfs)
{
    TextureUploadRuntimeReport report;
    const auto backendObligations = buildBackendObligationsRuntimeReport(inputs, rendererProfile, vfs);
    const auto materialSemantics = buildMaterialSemanticsRuntimeReport(inputs, rendererProfile, vfs);
    const auto devicePresentation = buildDevicePresentationRuntimeReport(inputs, rendererProfile, vfs);
    const auto& stage = inputs.sceneRuntime.stage;
    const auto consumers = materialTextureConsumerMap(stage);

    report.projectId = inputs.sceneRuntime.projectId;
    report.sceneRuntimeOk = inputs.sceneRuntime.ok;
    report.backendObligationsOk = backendObligations.ok;
    report.materialSemanticsOk = materialSemantics.ok;
    report.devicePresentationOk = devicePresentation.ok;
    report.postEffectSamplers = materialSemantics.postEffectSamplers;
    report.resourceUploadSubmissions = devicePresentation.resourceUploadSubmissions;
    report.titleTextSubmissions = inputs.titleUi.textDrawCommands + inputs.titleUi.graphStringCommands;
    report.titleStringSizeQueries = inputs.titleUi.stringSizeQueries;
    report.localizedMenuTextCommands = inputs.titleUi.localizedMenuTextCommands;
    report.textureWidthMin = std::numeric_limits<int>::max();
    report.textureHeightMin = std::numeric_limits<int>::max();

    for (const auto& dependency : stage.dependencies) {
        if (dependency.extension != ".dds") {
            continue;
        }

        ++report.stageTextureDependencies;
        TextureUploadRecordReport record;
        const std::string normalized = resource::normalizeResourcePath(dependency.path);
        const auto consumerIt = consumers.find(normalized);
        const TextureConsumerSummary consumer = consumerIt == consumers.end() ? TextureConsumerSummary{} : consumerIt->second;
        const std::string role = inferTextureUploadRole(dependency.path, consumer);
        const auto bytes = vfs.readBytes(dependency.path);
        if (bytes.found) {
            decodeDdsUploadRecord(dependency.path, bytes.bytes, consumer.count, role, record);
        } else {
            record.path = dependency.path;
            record.role = role;
            record.materialConsumerCount = consumer.count;
        }

        ++report.textureUploadRecords;
        if (record.found && isDdsPayload(bytes.bytes)) {
            ++report.ddsMagicRecords;
        }
        if (record.validDds) {
            ++report.validDdsHeaders;
        }
        if (record.format == "DXT1") {
            ++report.dxt1Textures;
        } else if (record.format == "DXT5") {
            ++report.dxt5Textures;
        } else {
            ++report.unsupportedTextureFormats;
        }
        if (record.cubeMap) {
            ++report.cubeMapTextures;
            report.cubeMapFaces += record.cubeFaces;
        }
        if (record.compressedPayloadMatches) {
            ++report.compressedPayloadMatches;
        }
        if (record.materialConsumerCount > 0) {
            ++report.uniqueMaterialTextureUploads;
            report.materialSlotConsumers += record.materialConsumerCount;
            report.duplicateMaterialConsumers += std::max(0, record.materialConsumerCount - 1);
        } else {
            ++report.stageOnlyTextureUploads;
        }

        if (record.width > 0) {
            report.textureWidthMin = std::min(report.textureWidthMin, record.width);
            report.textureWidthMax = std::max(report.textureWidthMax, record.width);
        }
        if (record.height > 0) {
            report.textureHeightMin = std::min(report.textureHeightMin, record.height);
            report.textureHeightMax = std::max(report.textureHeightMax, record.height);
        }
        if (record.mipCount == 9) {
            ++report.mip9Textures;
        } else if (record.mipCount == 10) {
            ++report.mip10Textures;
        } else if (record.mipCount == 11) {
            ++report.mip11Textures;
        }
        if (record.byteSize > 0) {
            report.textureByteTotal += record.byteSize;
        }
        report.payloadByteTotal += record.payloadBytes;
        report.expectedPayloadByteTotal += record.expectedPayloadBytes;
        report.records.push_back(std::move(record));
    }

    if (report.textureWidthMin == std::numeric_limits<int>::max()) {
        report.textureWidthMin = 0;
    }
    if (report.textureHeightMin == std::numeric_limits<int>::max()) {
        report.textureHeightMin = 0;
    }

    report.ddsHeaderContractReady = report.stageTextureDependencies == 39
        && report.textureUploadRecords == 39 && report.ddsMagicRecords == 39
        && report.validDdsHeaders == 39 && report.unsupportedTextureFormats == 0
        && report.dxt1Textures == 31 && report.dxt5Textures == 8;
    report.payloadLayoutContractReady = report.ddsHeaderContractReady
        && report.compressedPayloadMatches == 39
        && report.payloadByteTotal == report.expectedPayloadByteTotal
        && report.payloadByteTotal > 0;
    report.materialConsumerContractReady = materialSemantics.textureSlotContractReady
        && report.materialSlotConsumers == 39 && report.uniqueMaterialTextureUploads == 38
        && report.duplicateMaterialConsumers == 1 && report.stageOnlyTextureUploads == 1
        && report.cubeMapTextures == 1 && report.cubeMapFaces == 6;
    report.samplerStateGateTracked =
        devicePresentation.renderStateContractTracked && report.postEffectSamplers == 7;
    report.blendDepthStateGateTracked = devicePresentation.renderStateContractTracked;
    report.fontAtlasGateTracked = backendObligations.fontContractTracked
        && report.titleTextSubmissions == 11 && report.titleStringSizeQueries == 5
        && report.localizedMenuTextCommands == 10;
    report.oracleParityGateTracked =
        backendObligations.oracleParityContractTracked && devicePresentation.presentContractTracked;
    report.textureUploadRuntimeReady = report.sceneRuntimeOk && report.backendObligationsOk
        && report.materialSemanticsOk && report.devicePresentationOk && report.ddsHeaderContractReady
        && report.payloadLayoutContractReady && report.materialConsumerContractReady;

    report.contracts.push_back(makeBackendObligation(
        "dds_header_decode",
        report.ddsHeaderContractReady ? "contract_ready" : "blocked",
        "39 stage texture dependencies decode to DDS DXT1/DXT5 upload records"));
    report.contracts.push_back(makeBackendObligation(
        "dds_compressed_payload_layout",
        report.payloadLayoutContractReady ? "contract_ready" : "blocked",
        "computed mip-chain payload bytes match the DDS payload size for every record"));
    report.contracts.push_back(makeBackendObligation(
        "texture_material_consumer_map",
        report.materialConsumerContractReady ? "contract_ready" : "blocked",
        "39 material slots consume 38 texture uploads plus one cube environment upload"));
    report.contracts.push_back(makeBackendObligation(
        "cube_environment_upload_record",
        report.materialConsumerContractReady ? "contract_ready" : "blocked",
        "cubeenvmap/doujou_1.dds is tracked as a six-face DXT1 upload"));
    report.contracts.push_back(makeBackendObligation(
        "sampler_state_values",
        report.samplerStateGateTracked ? "tracked_open" : "blocked",
        "SMAA sampler count is known, but sampler filter/address state values are not bound"));
    report.contracts.push_back(makeBackendObligation(
        "blend_depth_state_values",
        report.blendDepthStateGateTracked ? "tracked_open" : "blocked",
        "renderer obligations carry blend/depth state, but device render states are not bound"));
    report.contracts.push_back(makeBackendObligation(
        "font_atlas_glyph_metrics",
        report.fontAtlasGateTracked ? "tracked_open" : "blocked",
        "title text commands and string-size queries require font atlas and glyph metrics"));
    report.contracts.push_back(makeBackendObligation(
        "original_frame_oracle_trace",
        report.oracleParityGateTracked ? "tracked_open" : "blocked",
        "upload records still lack original-frame screenshot or graphics API trace parity"));

    for (const auto& contract : report.contracts) {
        if (contract.status == "contract_ready") {
            ++report.resolvedUploadContracts;
        } else if (contract.status == "tracked_open") {
            ++report.trackedUploadObligations;
            ++report.openUploadObligations;
        } else {
            ++report.openUploadObligations;
        }
    }

    if (!report.sceneRuntimeOk) {
        addError(report, "scene runtime contract is not ready for texture upload records");
    }
    if (!report.backendObligationsOk) {
        addError(report, "backend obligations contract is not ready for texture upload records");
    }
    if (!report.materialSemanticsOk) {
        addError(report, "material semantics contract is not ready for texture upload records");
    }
    if (!report.devicePresentationOk) {
        addError(report, "device presentation contract is not ready for texture upload records");
    }
    if (!report.ddsHeaderContractReady) {
        addError(report, "DDS header upload records are incomplete");
    }
    if (!report.payloadLayoutContractReady) {
        addError(report, "DDS compressed payload layout does not match upload records");
    }
    if (!report.materialConsumerContractReady) {
        addError(report, "texture material consumer map is incomplete");
    }
    if (!report.samplerStateGateTracked || !report.blendDepthStateGateTracked
        || !report.fontAtlasGateTracked || !report.oracleParityGateTracked) {
        addError(report, "render-state/font/oracle gates are not fully tracked");
    }
    if (report.resolvedUploadContracts < 4 || report.trackedUploadObligations < 4
        || report.openUploadObligations != 4) {
        addError(report, "texture upload obligation accounting is inconsistent");
    }

    return report;
}

BackendStateRuntimeReport buildBackendStateRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile,
    const resource::VirtualFileSystem& vfs)
{
    BackendStateRuntimeReport report;
    const auto textureUpload = buildTextureUploadRuntimeReport(inputs, rendererProfile, vfs);
    const auto materialSemantics = buildMaterialSemanticsRuntimeReport(inputs, rendererProfile, vfs);

    report.projectId = inputs.sceneRuntime.projectId;
    report.textureUploadOk = textureUpload.ok;
    report.devicePresentationOk = textureUpload.devicePresentationOk;
    report.materialSemanticsOk = materialSemantics.ok;
    report.titleUiOk = inputs.titleUi.ok;
    report.textureUploadRecords = textureUpload.textureUploadRecords;
    report.materialTextureConsumers = textureUpload.materialSlotConsumers;

    const auto smaaFx = vfs.readBytes("SMAA.fx");
    if (smaaFx.found) {
        const std::string text = bytesToText(smaaFx.bytes);
        report.samplerRecords = parseSmaaSamplerStateRecords(text);
        report.passRecords = parseSmaaPassStateRecords(text);
    }

    for (const auto& sampler : report.samplerRecords) {
        ++report.samplerStateRecords;
        if (!sampler.texture.empty()) {
            ++report.samplerTextureBindings;
        }
        if (sampler.addressU == "Clamp" && sampler.addressV == "Clamp"
            && (sampler.addressW.empty() || sampler.addressW == "Clamp")) {
            ++report.samplerClampAddressRecords;
        }
        if (sampler.minFilter == "Linear") {
            ++report.samplerLinearMinFilters;
        } else if (sampler.minFilter == "Point") {
            ++report.samplerPointMinFilters;
        }
        if (sampler.srgbTexture == "true") {
            ++report.samplerSrgbTrueRecords;
        } else if (sampler.srgbTexture == "false") {
            ++report.samplerSrgbFalseRecords;
        }
    }

    for (const auto& pass : report.passRecords) {
        ++report.passStateRecords;
        if (pass.vertexShaderProfile == "vs_3_0") {
            ++report.passVs30Shaders;
        }
        if (pass.pixelShaderProfile == "ps_3_0") {
            ++report.passPs30Shaders;
        }
        if (pass.zEnable == "false") {
            ++report.zDisabledPasses;
        }
        if (pass.alphaBlendEnable == "false") {
            ++report.alphaBlendDisabledPasses;
        }
        if (pass.alphaTestEnable == "false") {
            ++report.alphaTestDisabledPasses;
        }
        if (pass.srgbWriteEnable == "true") {
            ++report.srgbWriteEnabledPasses;
        } else if (pass.srgbWriteEnable == "false") {
            ++report.srgbWriteDisabledPasses;
        }
        if (pass.stencilEnable == "true") {
            ++report.stencilEnabledPasses;
        } else if (pass.stencilEnable == "false") {
            ++report.stencilDisabledPasses;
        }
        if (pass.stencilPass == "REPLACE") {
            ++report.stencilReplacePasses;
        } else if (pass.stencilPass == "KEEP") {
            ++report.stencilKeepPasses;
        }
        if (pass.stencilFunc == "EQUAL") {
            ++report.stencilEqualPasses;
        }
    }

    report.fontRecord.source = "title-ui";
    report.fontRecord.fontQueries = inputs.titleUi.fontQueryCommands;
    report.fontRecord.fontScaleLimits = inputs.titleUi.fontScaleLimitCommands;
    report.fontRecord.textDrawCommands = inputs.titleUi.textDrawCommands;
    report.fontRecord.graphStringCommands = inputs.titleUi.graphStringCommands;
    report.fontRecord.stringSizeQueries = inputs.titleUi.stringSizeQueries;
    report.fontRecord.localizedMenuTextCommands = inputs.titleUi.localizedMenuTextCommands;
    report.fontRecord.drawListItemCommands = inputs.titleUi.drawListItemCommands;
    report.fontRecord.glyphMetricInputsReady = report.fontRecord.fontQueries >= 6
        && report.fontRecord.textDrawCommands == 6 && report.fontRecord.graphStringCommands == 5
        && report.fontRecord.stringSizeQueries == 5 && report.fontRecord.localizedMenuTextCommands == 10
        && report.fontRecord.drawListItemCommands == 5;
    report.fontRecord.atlasImplementationTracked = report.fontRecord.glyphMetricInputsReady;

    report.fontQueryRecords = report.fontRecord.fontQueries;
    report.fontScaleLimitRecords = report.fontRecord.fontScaleLimits;
    report.textDrawCommands = report.fontRecord.textDrawCommands;
    report.graphStringCommands = report.fontRecord.graphStringCommands;
    report.stringSizeQueries = report.fontRecord.stringSizeQueries;
    report.localizedMenuTextCommands = report.fontRecord.localizedMenuTextCommands;
    report.drawListItemCommands = report.fontRecord.drawListItemCommands;

    report.samplerStateRecordsReady = report.samplerStateRecords == 7
        && report.samplerTextureBindings == 7 && report.samplerClampAddressRecords == 7
        && report.samplerLinearMinFilters == 6 && report.samplerPointMinFilters == 1
        && report.samplerSrgbTrueRecords == 1 && report.samplerSrgbFalseRecords == 6;
    report.passRenderStateRecordsReady = report.passStateRecords == 5
        && report.passVs30Shaders == 5 && report.passPs30Shaders == 5
        && report.zDisabledPasses == 5 && report.alphaBlendDisabledPasses == 5
        && report.alphaTestDisabledPasses == 5 && report.srgbWriteEnabledPasses == 1
        && report.srgbWriteDisabledPasses == 4 && report.stencilEnabledPasses == 4
        && report.stencilDisabledPasses == 1 && report.stencilReplacePasses == 3
        && report.stencilKeepPasses == 1 && report.stencilEqualPasses == 1;
    report.fontAtlasRecordsReady = inputs.titleUi.ok && report.fontRecord.glyphMetricInputsReady
        && report.fontRecord.atlasImplementationTracked;
    report.materialShaderProgramGateTracked = materialSemantics.shaderEffectContractTracked;
    report.gpuStateBindingGateTracked =
        textureUpload.samplerStateGateTracked && textureUpload.blendDepthStateGateTracked;
    report.oracleParityGateTracked = textureUpload.oracleParityGateTracked;
    report.backendStateRuntimeReady = report.textureUploadOk && report.devicePresentationOk
        && report.materialSemanticsOk && report.titleUiOk && report.samplerStateRecordsReady
        && report.passRenderStateRecordsReady && report.fontAtlasRecordsReady
        && report.textureUploadRecords == 39 && report.materialTextureConsumers == 39;

    report.contracts.push_back(makeBackendObligation(
        "smaa_sampler_state_records",
        report.samplerStateRecordsReady ? "contract_ready" : "blocked",
        "SMAA.fx sampler blocks expose texture, address, filter, and SRGB state"));
    report.contracts.push_back(makeBackendObligation(
        "smaa_pass_render_state_records",
        report.passRenderStateRecordsReady ? "contract_ready" : "blocked",
        "SMAA.fx pass blocks expose shader profiles, alpha, depth, SRGB, and stencil state"));
    report.contracts.push_back(makeBackendObligation(
        "title_font_glyph_metric_inputs",
        report.fontAtlasRecordsReady ? "contract_ready" : "blocked",
        "title UI exposes font queries, text draw commands, graph strings, and string-size queries"));
    report.contracts.push_back(makeBackendObligation(
        "backend_state_consumes_texture_uploads",
        report.backendStateRuntimeReady ? "contract_ready" : "blocked",
        "backend state records consume 39 texture uploads and 39 material slot consumers"));
    report.contracts.push_back(makeBackendObligation(
        "material_shader_program_binding",
        report.materialShaderProgramGateTracked ? "tracked_open" : "blocked",
        "model materials still expose texture slots but no per-material shader program token"));
    report.contracts.push_back(makeBackendObligation(
        "gpu_state_binding_execution",
        report.gpuStateBindingGateTracked ? "tracked_open" : "blocked",
        "sampler/pass state records are decoded but not submitted to a GPU device"));
    report.contracts.push_back(makeBackendObligation(
        "font_atlas_texture_implementation",
        report.fontAtlasRecordsReady ? "tracked_open" : "blocked",
        "glyph metric inputs exist but no atlas texture, glyph cache, or text draw backend exists"));
    report.contracts.push_back(makeBackendObligation(
        "original_frame_oracle_trace",
        report.oracleParityGateTracked ? "tracked_open" : "blocked",
        "backend state records still lack original-frame screenshot or graphics API trace parity"));

    for (const auto& contract : report.contracts) {
        if (contract.status == "contract_ready") {
            ++report.resolvedBackendStateContracts;
        } else if (contract.status == "tracked_open") {
            ++report.trackedBackendStateObligations;
            ++report.openBackendStateObligations;
        } else {
            ++report.openBackendStateObligations;
        }
    }

    if (!report.textureUploadOk) {
        addError(report, "texture upload contract is not ready for backend state records");
    }
    if (!report.devicePresentationOk) {
        addError(report, "device presentation contract is not ready for backend state records");
    }
    if (!report.materialSemanticsOk) {
        addError(report, "material semantics contract is not ready for backend state records");
    }
    if (!report.titleUiOk) {
        addError(report, "title UI contract is not ready for backend state records");
    }
    if (!report.samplerStateRecordsReady) {
        addError(report, "SMAA sampler state records are incomplete");
    }
    if (!report.passRenderStateRecordsReady) {
        addError(report, "SMAA pass render-state records are incomplete");
    }
    if (!report.fontAtlasRecordsReady) {
        addError(report, "font atlas/glyph metric records are incomplete");
    }
    if (!report.materialShaderProgramGateTracked || !report.gpuStateBindingGateTracked
        || !report.oracleParityGateTracked) {
        addError(report, "backend state open gates are not fully tracked");
    }
    if (report.resolvedBackendStateContracts < 4 || report.trackedBackendStateObligations < 4
        || report.openBackendStateObligations != 4) {
        addError(report, "backend state obligation accounting is inconsistent");
    }

    return report;
}

BackendResourceAllocationRuntimeReport buildBackendResourceAllocationRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile,
    const resource::VirtualFileSystem& vfs)
{
    BackendResourceAllocationRuntimeReport report;
    const auto textureUpload = buildTextureUploadRuntimeReport(inputs, rendererProfile, vfs);
    const auto backendState = buildBackendStateRuntimeReport(inputs, rendererProfile, vfs);

    report.projectId = inputs.sceneRuntime.projectId;
    report.textureUploadOk = textureUpload.ok;
    report.backendStateOk = backendState.ok;

    for (const auto& upload : textureUpload.records) {
        report.records.push_back(makeStageTextureAllocationRecord(upload));
    }

    const auto smaaFx = vfs.readBytes("SMAA.fx");
    std::vector<std::string> textureDeclarations;
    int transientWidth = 0;
    int transientHeight = 0;
    if (smaaFx.found) {
        const std::string text = bytesToText(smaaFx.bytes);
        textureDeclarations = parseSmaaTextureDeclarations(text);
        extractSmaaPixelSizeCandidate(text, transientWidth, transientHeight);
    }
    report.samplerTextureDeclarations = static_cast<int>(textureDeclarations.size());

    const std::map<std::string, std::string> lookupPaths = {
        {"areaTex2D", "system/glsl/smaa/areatexdx9.dds"},
        {"searchTex2D", "system/glsl/smaa/searchtex.dds"},
    };
    const std::set<std::string> transientNames = {
        "colorTex2D",
        "depthTex2D",
        "edgesTex2D",
        "blendTex2D",
    };

    for (const auto& declaration : textureDeclarations) {
        const auto lookup = lookupPaths.find(declaration);
        if (lookup != lookupPaths.end()) {
            const auto bytes = vfs.readBytes(lookup->second);
            BackendResourceAllocationRecord record;
            if (bytes.found) {
                record = makeSmaaLookupAllocationRecord(declaration, lookup->second, bytes.bytes);
            } else {
                record.name = declaration;
                record.path = lookup->second;
                record.resourceKind = "smaa_lookup_texture";
                record.usage = "post_effect_lookup";
                record.status = "blocked";
            }
            report.records.push_back(std::move(record));
            continue;
        }
        if (transientNames.find(declaration) != transientNames.end()) {
            report.records.push_back(makeTransientSurfaceAllocationRecord(
                declaration,
                transientWidth,
                transientHeight));
        }
    }

    if (backendState.fontAtlasRecordsReady) {
        report.records.push_back(makeFontAtlasAllocationPlaceholder());
    }

    for (const auto& record : report.records) {
        ++report.allocationRecords;
        if (record.ready) {
            ++report.readyAllocationRecords;
            report.readyAllocationByteTotal += record.byteSize;
            report.readyAllocationPayloadBytes += record.payloadBytes;
            report.readyExpectedPayloadBytes += record.expectedPayloadBytes;
        } else if (record.status == "tracked_open") {
            ++report.trackedOpenAllocationRecords;
        }

        if (record.resourceKind == "texture_2d" || record.resourceKind == "cube_texture") {
            ++report.stageTextureAllocations;
            report.materialTextureConsumers += record.materialConsumerCount;
            if (record.format == "D3DFMT_DXT1") {
                ++report.d3dDxt1Allocations;
            } else if (record.format == "D3DFMT_DXT5") {
                ++report.d3dDxt5Allocations;
            }
            if (record.resourceKind == "cube_texture") {
                ++report.cubeTextureAllocations;
            }
        } else if (record.resourceKind == "smaa_lookup_texture") {
            ++report.smaaLookupAllocations;
            if (record.format == "D3DFMT_L8") {
                ++report.lookupL8Allocations;
            } else if (record.format == "D3DFMT_A8L8") {
                ++report.lookupA8L8Allocations;
            }
        } else if (record.resourceKind == "smaa_transient_surface") {
            ++report.transientSurfaceCandidates;
        } else if (record.resourceKind == "font_atlas_texture") {
            ++report.fontAtlasPlaceholders;
        }
    }

    report.stageTextureAllocationRecordsReady = textureUpload.textureUploadRuntimeReady
        && report.stageTextureAllocations == 39 && report.d3dDxt1Allocations == 31
        && report.d3dDxt5Allocations == 8 && report.cubeTextureAllocations == 1
        && report.materialTextureConsumers == 39;
    report.smaaLookupAllocationRecordsReady = report.smaaLookupAllocations == 2
        && report.lookupL8Allocations == 1 && report.lookupA8L8Allocations == 1
        && report.readyAllocationPayloadBytes == report.readyExpectedPayloadBytes
        && report.readyAllocationPayloadBytes > textureUpload.payloadByteTotal;
    report.transientSurfaceAllocationGateTracked = report.transientSurfaceCandidates == 4
        && report.samplerTextureDeclarations == 6 && transientWidth == 1280 && transientHeight == 720;
    report.fontAtlasAllocationGateTracked =
        backendState.fontAtlasRecordsReady && report.fontAtlasPlaceholders == 1;
    report.d3dResourceCreationGateTracked =
        textureUpload.devicePresentationOk && backendState.gpuStateBindingGateTracked;
    report.oracleParityGateTracked = backendState.oracleParityGateTracked;
    report.resourceAllocationRuntimeReady = report.textureUploadOk && report.backendStateOk
        && report.stageTextureAllocationRecordsReady && report.smaaLookupAllocationRecordsReady
        && report.transientSurfaceAllocationGateTracked && report.fontAtlasAllocationGateTracked
        && report.d3dResourceCreationGateTracked && report.oracleParityGateTracked
        && report.allocationRecords == 46 && report.readyAllocationRecords == 41
        && report.trackedOpenAllocationRecords == 5;

    report.contracts.push_back(makeBackendObligation(
        "stage_texture_allocation_records",
        report.stageTextureAllocationRecordsReady ? "contract_ready" : "blocked",
        "39 DDS stage uploads map to D3D9 texture/cube allocation records"));
    report.contracts.push_back(makeBackendObligation(
        "smaa_lookup_texture_allocation_records",
        report.smaaLookupAllocationRecordsReady ? "contract_ready" : "blocked",
        "SMAA area/search lookup DDS payloads map to D3DFMT_A8L8 and D3DFMT_L8"));
    report.contracts.push_back(makeBackendObligation(
        "smaa_transient_surface_allocation_candidates",
        report.transientSurfaceAllocationGateTracked ? "tracked_open" : "blocked",
        "SMAA color/depth/edges/blend texture declarations are tracked as 1280x720 surface candidates"));
    report.contracts.push_back(makeBackendObligation(
        "font_atlas_allocation_placeholder",
        report.fontAtlasAllocationGateTracked ? "tracked_open" : "blocked",
        "title font glyph metric inputs exist, but atlas texture dimensions and cache ownership are unknown"));
    report.contracts.push_back(makeBackendObligation(
        "d3d_resource_creation_execution",
        report.d3dResourceCreationGateTracked ? "tracked_open" : "blocked",
        "resource allocation records are typed but no IDirect3DDevice9 CreateTexture/CreateRenderTarget calls exist"));
    report.contracts.push_back(makeBackendObligation(
        "material_shader_resource_binding",
        backendState.materialShaderProgramGateTracked ? "tracked_open" : "blocked",
        "stage texture allocations have material consumers but no recovered material shader program token"));
    report.contracts.push_back(makeBackendObligation(
        "original_frame_oracle_trace",
        report.oracleParityGateTracked ? "tracked_open" : "blocked",
        "allocation records still lack original graphics API trace or frame capture parity"));

    for (const auto& contract : report.contracts) {
        if (contract.status == "contract_ready") {
            ++report.resolvedAllocationContracts;
        } else if (contract.status == "tracked_open") {
            ++report.trackedAllocationObligations;
            ++report.openAllocationObligations;
        } else {
            ++report.openAllocationObligations;
        }
    }

    if (!report.textureUploadOk) {
        addError(report, "texture upload contract is not ready for resource allocation records");
    }
    if (!report.backendStateOk) {
        addError(report, "backend state contract is not ready for resource allocation records");
    }
    if (!report.stageTextureAllocationRecordsReady) {
        addError(report, "stage texture allocation records are incomplete");
    }
    if (!report.smaaLookupAllocationRecordsReady) {
        addError(report, "SMAA lookup texture allocation records are incomplete");
    }
    if (!report.transientSurfaceAllocationGateTracked || !report.fontAtlasAllocationGateTracked
        || !report.d3dResourceCreationGateTracked || !report.oracleParityGateTracked) {
        addError(report, "resource allocation open gates are not fully tracked");
    }
    if (report.resolvedAllocationContracts != 2 || report.trackedAllocationObligations != 5
        || report.openAllocationObligations != 5) {
        addError(report, "resource allocation obligation accounting is inconsistent");
    }

    return report;
}

BackendDeviceExecutionRuntimeReport buildBackendDeviceExecutionRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile,
    const resource::VirtualFileSystem& vfs)
{
    BackendDeviceExecutionRuntimeReport report;
    const auto resourceAllocation = buildBackendResourceAllocationRuntimeReport(inputs, rendererProfile, vfs);
    const auto backendState = buildBackendStateRuntimeReport(inputs, rendererProfile, vfs);
    const auto devicePresentation = buildDevicePresentationRuntimeReport(inputs, rendererProfile, vfs);

    report.projectId = inputs.sceneRuntime.projectId;
    report.resourceAllocationOk = resourceAllocation.ok;
    report.backendStateOk = backendState.ok;
    report.devicePresentationOk = devicePresentation.ok;

    for (const auto& allocation : resourceAllocation.records) {
        report.resourceRecords.push_back(makeDeviceResourceExecutionRecord(allocation));
        if (allocation.materialConsumerCount > 0) {
            report.bindingRecordsDetail.push_back(makeMaterialTextureBindingRecord(allocation));
        }
    }

    for (const auto& sampler : backendState.samplerRecords) {
        report.bindingRecordsDetail.push_back(makeSmaaSamplerTextureBindingRecord(sampler));
        report.bindingRecordsDetail.push_back(makeSamplerStateBindingRecord(sampler));
    }
    for (const auto& pass : backendState.passRecords) {
        report.bindingRecordsDetail.push_back(makeRenderStateBindingRecord(pass));
    }

    for (const auto& record : report.resourceRecords) {
        ++report.resourceCreationRecords;
        if (record.ready) {
            ++report.readyResourceCreationRecords;
            ++report.textureUploadExecutionRecords;
            report.uploadSubresourceRecords += record.subresourceCount;
            report.readyUploadPayloadBytes += record.payloadBytes;
        } else if (record.status == "tracked_open") {
            ++report.trackedOpenResourceCreationRecords;
        }

        if (record.operation == "CreateTexture") {
            ++report.createTextureRecords;
        } else if (record.operation == "CreateCubeTexture") {
            ++report.createCubeTextureRecords;
        } else if (record.operation == "CreateRenderTargetCandidate") {
            ++report.renderTargetCreationCandidates;
        } else if (record.operation == "CreateDepthStencilSurfaceCandidate") {
            ++report.depthStencilCreationCandidates;
        } else if (record.operation == "CreateTextureCandidate"
            && record.resourceKind == "font_atlas_texture") {
            ++report.fontAtlasCreationPlaceholders;
        }
    }

    for (const auto& record : report.bindingRecordsDetail) {
        ++report.bindingRecords;
        if (record.ready) {
            ++report.readyBindingRecords;
        } else if (record.status == "tracked_open") {
            ++report.trackedOpenBindingRecords;
        }

        if (record.target == "material_texture_slot") {
            ++report.materialTextureBindingRecords;
            report.materialTextureBindingSlots += record.bindingSlots;
        } else if (record.operation == "SetTexture" || record.operation == "SetTextureCandidate") {
            ++report.samplerTextureBindingRecords;
            if (record.status == "contract_ready") {
                ++report.lookupTextureBindingRecords;
            } else if (record.status == "tracked_open") {
                ++report.transientSamplerBindingCandidates;
            }
        } else if (record.operation == "SetSamplerState") {
            ++report.samplerStateBindingRecords;
        } else if (record.operation == "SetRenderStateBundle") {
            ++report.renderStateBindingRecords;
        }
    }

    report.resourceCreationRecordsReady = resourceAllocation.resourceAllocationRuntimeReady
        && report.resourceCreationRecords == 46 && report.readyResourceCreationRecords == 41
        && report.trackedOpenResourceCreationRecords == 5 && report.createTextureRecords == 40
        && report.createCubeTextureRecords == 1 && report.renderTargetCreationCandidates == 3
        && report.depthStencilCreationCandidates == 1 && report.fontAtlasCreationPlaceholders == 1;
    report.uploadExecutionRecordsReady = report.textureUploadExecutionRecords == 41
        && report.uploadSubresourceRecords == 458
        && report.readyUploadPayloadBytes == resourceAllocation.readyAllocationPayloadBytes
        && report.readyUploadPayloadBytes == 23949794;
    report.stateBindingRecordsReady = backendState.samplerStateRecordsReady
        && backendState.passRenderStateRecordsReady && report.samplerStateBindingRecords == 7
        && report.renderStateBindingRecords == 5;
    report.lookupTextureBindingRecordsReady =
        report.samplerTextureBindingRecords == 7 && report.lookupTextureBindingRecords == 2;
    report.materialTextureBindingGateTracked =
        report.materialTextureBindingRecords == 38 && report.materialTextureBindingSlots == 39;
    report.transientSurfaceBindingGateTracked =
        report.renderTargetCreationCandidates == 3 && report.depthStencilCreationCandidates == 1
        && report.transientSamplerBindingCandidates == 5;
    report.fontAtlasExecutionGateTracked =
        backendState.fontAtlasRecordsReady && report.fontAtlasCreationPlaceholders == 1;
    report.d3dApiCallSubmissionGateTracked =
        devicePresentation.deviceProfileReady && resourceAllocation.d3dResourceCreationGateTracked;
    report.presentOracleGateTracked =
        devicePresentation.presentContractTracked && resourceAllocation.oracleParityGateTracked;
    report.deviceExecutionRuntimeReady = report.resourceAllocationOk && report.backendStateOk
        && report.devicePresentationOk && report.resourceCreationRecordsReady
        && report.uploadExecutionRecordsReady && report.stateBindingRecordsReady
        && report.lookupTextureBindingRecordsReady && report.materialTextureBindingGateTracked
        && report.transientSurfaceBindingGateTracked && report.fontAtlasExecutionGateTracked
        && report.d3dApiCallSubmissionGateTracked && report.presentOracleGateTracked
        && report.bindingRecords == 57 && report.readyBindingRecords == 14
        && report.trackedOpenBindingRecords == 43;

    report.contracts.push_back(makeBackendObligation(
        "device_resource_creation_records",
        report.resourceCreationRecordsReady ? "contract_ready" : "blocked",
        "46 allocation records map to D3D9 CreateTexture/CreateCubeTexture/render-target/depth/font creation records"));
    report.contracts.push_back(makeBackendObligation(
        "texture_upload_subresource_records",
        report.uploadExecutionRecordsReady ? "contract_ready" : "blocked",
        "41 ready textures expand to 458 upload subresources with payload byte parity"));
    report.contracts.push_back(makeBackendObligation(
        "smaa_lookup_texture_bindings",
        report.lookupTextureBindingRecordsReady ? "contract_ready" : "blocked",
        "SMAA area/search samplers bind ready lookup texture records"));
    report.contracts.push_back(makeBackendObligation(
        "sampler_and_render_state_value_records",
        report.stateBindingRecordsReady ? "contract_ready" : "blocked",
        "7 sampler state records and 5 pass render-state records are device-binding ready"));
    report.contracts.push_back(makeBackendObligation(
        "material_texture_shader_binding",
        report.materialTextureBindingGateTracked ? "tracked_open" : "blocked",
        "39 material texture slots have texture records, but material shader program ownership is not recovered"));
    report.contracts.push_back(makeBackendObligation(
        "smaa_transient_surface_creation_and_binding",
        report.transientSurfaceBindingGateTracked ? "tracked_open" : "blocked",
        "SMAA transient render/depth targets are typed candidates but exact D3D formats and ownership are unknown"));
    report.contracts.push_back(makeBackendObligation(
        "font_atlas_device_resource",
        report.fontAtlasExecutionGateTracked ? "tracked_open" : "blocked",
        "font metric inputs exist, but atlas dimensions, glyph cache, and texture upload are not implemented"));
    report.contracts.push_back(makeBackendObligation(
        "d3d_api_call_submission",
        report.d3dApiCallSubmissionGateTracked ? "tracked_open" : "blocked",
        "device execution records exist, but YuEngine does not call IDirect3DDevice9 APIs yet"));
    report.contracts.push_back(makeBackendObligation(
        "present_and_original_frame_oracle",
        report.presentOracleGateTracked ? "tracked_open" : "blocked",
        "resource/state execution records still lack Present execution and original-frame oracle parity"));

    for (const auto& contract : report.contracts) {
        if (contract.status == "contract_ready") {
            ++report.resolvedDeviceExecutionContracts;
        } else if (contract.status == "tracked_open") {
            ++report.trackedDeviceExecutionObligations;
            ++report.openDeviceExecutionObligations;
        } else {
            ++report.openDeviceExecutionObligations;
        }
    }

    if (!report.resourceAllocationOk) {
        addError(report, "resource allocation contract is not ready for device execution records");
    }
    if (!report.backendStateOk) {
        addError(report, "backend state contract is not ready for device execution records");
    }
    if (!report.devicePresentationOk) {
        addError(report, "device presentation contract is not ready for device execution records");
    }
    if (!report.resourceCreationRecordsReady) {
        addError(report, "device resource creation records are incomplete");
    }
    if (!report.uploadExecutionRecordsReady) {
        addError(report, "texture upload execution records are incomplete");
    }
    if (!report.stateBindingRecordsReady || !report.lookupTextureBindingRecordsReady) {
        addError(report, "device state/lookup binding records are incomplete");
    }
    if (!report.materialTextureBindingGateTracked || !report.transientSurfaceBindingGateTracked
        || !report.fontAtlasExecutionGateTracked || !report.d3dApiCallSubmissionGateTracked
        || !report.presentOracleGateTracked) {
        addError(report, "device execution open gates are not fully tracked");
    }
    if (report.resolvedDeviceExecutionContracts != 4 || report.trackedDeviceExecutionObligations != 5
        || report.openDeviceExecutionObligations != 5) {
        addError(report, "device execution obligation accounting is inconsistent");
    }

    return report;
}

BackendPresentationOracleRuntimeReport buildBackendPresentationOracleRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile,
    const resource::VirtualFileSystem& vfs)
{
    BackendPresentationOracleRuntimeReport report;
    const auto deviceExecution = buildBackendDeviceExecutionRuntimeReport(inputs, rendererProfile, vfs);
    const auto devicePresentation = buildDevicePresentationRuntimeReport(inputs, rendererProfile, vfs);

    report.projectId = inputs.sceneRuntime.projectId;
    report.deviceExecutionOk = deviceExecution.ok;
    report.devicePresentationOk = devicePresentation.ok;
    report.backbufferWidth = devicePresentation.backbufferWidthCandidate;
    report.backbufferHeight = devicePresentation.backbufferHeightCandidate;
    report.rendererBackendCommands = devicePresentation.rendererBackendCommands;
    report.drawSubmissions = devicePresentation.drawSubmissions;
    report.linkedDeviceExecutionRecords =
        deviceExecution.resourceCreationRecords + deviceExecution.bindingRecords;
    report.linkedDeviceReadyRecords =
        deviceExecution.readyResourceCreationRecords + deviceExecution.readyBindingRecords;
    report.linkedDeviceOpenRecords =
        deviceExecution.trackedOpenResourceCreationRecords + deviceExecution.trackedOpenBindingRecords;

    report.records.push_back(makePresentationOracleRecord(
        "swapchain_backbuffer_extent",
        "BackbufferExtent",
        "SMAA_PIXEL_SIZE",
        devicePresentation.swapchainContractTracked ? "contract_ready" : "blocked",
        report.backbufferWidth,
        report.backbufferHeight,
        1));
    report.records.push_back(makePresentationOracleRecord(
        "device_execution_frame_input",
        "DeviceExecutionFrameInput",
        "yuengine_cli device-execution",
        deviceExecution.deviceExecutionRuntimeReady ? "contract_ready" : "blocked",
        report.backbufferWidth,
        report.backbufferHeight,
        report.linkedDeviceExecutionRecords));
    report.records.push_back(makePresentationOracleRecord(
        "window_surface_candidate",
        "CreateWindowSurfaceCandidate",
        "device-presentation",
        devicePresentation.presentContractTracked ? "tracked_open" : "blocked",
        report.backbufferWidth,
        report.backbufferHeight,
        1));
    report.records.push_back(makePresentationOracleRecord(
        "swapchain_creation_candidate",
        "CreateSwapChainCandidate",
        "device-presentation",
        devicePresentation.swapchainContractTracked ? "tracked_open" : "blocked",
        report.backbufferWidth,
        report.backbufferHeight,
        1));
    report.records.push_back(makePresentationOracleRecord(
        "present_call_candidate",
        "PresentCandidate",
        "renderer scheduler present gate",
        devicePresentation.presentContractTracked ? "tracked_open" : "blocked",
        report.backbufferWidth,
        report.backbufferHeight,
        report.rendererBackendCommands));
    report.records.push_back(makePresentationOracleRecord(
        "frame_capture_candidate",
        "FrameCaptureCandidate",
        "oracle title boot readiness",
        deviceExecution.presentOracleGateTracked ? "tracked_open" : "blocked",
        report.backbufferWidth,
        report.backbufferHeight,
        report.drawSubmissions));
    report.records.push_back(makePresentationOracleRecord(
        "original_frame_oracle_trace",
        "OriginalFrameOracleTraceCandidate",
        "oracle title boot readiness",
        deviceExecution.presentOracleGateTracked ? "tracked_open" : "blocked",
        report.backbufferWidth,
        report.backbufferHeight,
        0));

    for (const auto& record : report.records) {
        ++report.presentationRecords;
        if (record.ready) {
            ++report.readyPresentationRecords;
        } else if (record.status == "tracked_open") {
            ++report.trackedOpenPresentationRecords;
        }

        if (record.operation == "BackbufferExtent") {
            ++report.backbufferExtentRecords;
        } else if (record.operation == "DeviceExecutionFrameInput") {
            ++report.deviceExecutionFrameInputs;
        } else if (record.operation == "CreateWindowSurfaceCandidate") {
            ++report.windowSurfaceCandidates;
        } else if (record.operation == "CreateSwapChainCandidate") {
            ++report.swapchainCreationCandidates;
        } else if (record.operation == "PresentCandidate") {
            ++report.presentCallCandidates;
        } else if (record.operation == "FrameCaptureCandidate") {
            ++report.frameCaptureCandidates;
        } else if (record.operation == "OriginalFrameOracleTraceCandidate") {
            ++report.oracleTraceCandidates;
        }
    }

    report.backbufferExtentRecordsReady = devicePresentation.swapchainContractTracked
        && report.backbufferExtentRecords == 1 && report.backbufferWidth == 1280
        && report.backbufferHeight == 720;
    report.deviceExecutionInputRecordsReady = deviceExecution.deviceExecutionRuntimeReady
        && report.deviceExecutionFrameInputs == 1 && report.linkedDeviceExecutionRecords == 103
        && report.linkedDeviceReadyRecords == 55 && report.linkedDeviceOpenRecords == 48;
    report.windowSurfaceGateTracked =
        devicePresentation.presentContractTracked && report.windowSurfaceCandidates == 1;
    report.swapchainCreationGateTracked =
        devicePresentation.swapchainContractTracked && report.swapchainCreationCandidates == 1;
    report.presentCallGateTracked =
        devicePresentation.presentContractTracked && report.presentCallCandidates == 1
        && report.rendererBackendCommands == 181;
    report.frameCaptureGateTracked =
        deviceExecution.presentOracleGateTracked && report.frameCaptureCandidates == 1
        && report.drawSubmissions == 121;
    report.originalFrameOracleGateTracked =
        deviceExecution.presentOracleGateTracked && report.oracleTraceCandidates == 1;
    report.presentationOracleRuntimeReady = report.deviceExecutionOk && report.devicePresentationOk
        && report.backbufferExtentRecordsReady && report.deviceExecutionInputRecordsReady
        && report.windowSurfaceGateTracked && report.swapchainCreationGateTracked
        && report.presentCallGateTracked && report.frameCaptureGateTracked
        && report.originalFrameOracleGateTracked && report.presentationRecords == 7
        && report.readyPresentationRecords == 2 && report.trackedOpenPresentationRecords == 5;

    report.contracts.push_back(makeBackendObligation(
        "backbuffer_extent_record",
        report.backbufferExtentRecordsReady ? "contract_ready" : "blocked",
        "SMAA_PIXEL_SIZE and device presentation expose a 1280x720 backbuffer extent"));
    report.contracts.push_back(makeBackendObligation(
        "device_execution_frame_input",
        report.deviceExecutionInputRecordsReady ? "contract_ready" : "blocked",
        "L24 supplies 103 device execution records, including 55 ready and 48 tracked-open records"));
    report.contracts.push_back(makeBackendObligation(
        "window_surface_creation",
        report.windowSurfaceGateTracked ? "tracked_open" : "blocked",
        "present requires an HWND/window surface, but no platform surface is created"));
    report.contracts.push_back(makeBackendObligation(
        "swapchain_creation",
        report.swapchainCreationGateTracked ? "tracked_open" : "blocked",
        "backbuffer extent is known, but no swapchain object is created"));
    report.contracts.push_back(makeBackendObligation(
        "present_call_execution",
        report.presentCallGateTracked ? "tracked_open" : "blocked",
        "renderer scheduler reaches present gate, but no Present call is executed"));
    report.contracts.push_back(makeBackendObligation(
        "frame_capture_artifact",
        report.frameCaptureGateTracked ? "tracked_open" : "blocked",
        "draw submissions exist, but no captured frame artifact exists for parity"));
    report.contracts.push_back(makeBackendObligation(
        "original_frame_oracle_trace",
        report.originalFrameOracleGateTracked ? "tracked_open" : "blocked",
        "no original D3D/render trace or screenshot parity has been recorded"));

    for (const auto& contract : report.contracts) {
        if (contract.status == "contract_ready") {
            ++report.resolvedPresentationContracts;
        } else if (contract.status == "tracked_open") {
            ++report.trackedPresentationObligations;
            ++report.openPresentationObligations;
        } else {
            ++report.openPresentationObligations;
        }
    }

    if (!report.deviceExecutionOk) {
        addError(report, "device execution contract is not ready for presentation/oracle records");
    }
    if (!report.devicePresentationOk) {
        addError(report, "device presentation contract is not ready for presentation/oracle records");
    }
    if (!report.backbufferExtentRecordsReady) {
        addError(report, "backbuffer extent record is incomplete");
    }
    if (!report.deviceExecutionInputRecordsReady) {
        addError(report, "device execution frame input record is incomplete");
    }
    if (!report.windowSurfaceGateTracked || !report.swapchainCreationGateTracked
        || !report.presentCallGateTracked || !report.frameCaptureGateTracked
        || !report.originalFrameOracleGateTracked) {
        addError(report, "presentation/oracle open gates are not fully tracked");
    }
    if (report.resolvedPresentationContracts != 2 || report.trackedPresentationObligations != 5
        || report.openPresentationObligations != 5) {
        addError(report, "presentation/oracle obligation accounting is inconsistent");
    }

    return report;
}

BackendPlatformBridgeRuntimeReport buildBackendPlatformBridgeRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile,
    const resource::VirtualFileSystem& vfs)
{
    BackendPlatformBridgeRuntimeReport report;
    const auto deviceExecution = buildBackendDeviceExecutionRuntimeReport(inputs, rendererProfile, vfs);
    const auto presentationOracle = buildBackendPresentationOracleRuntimeReport(inputs, rendererProfile, vfs);

    report.projectId = inputs.sceneRuntime.projectId;
    report.deviceExecutionOk = deviceExecution.ok;
    report.presentationOracleOk = presentationOracle.ok;
    report.backbufferWidth = presentationOracle.backbufferWidth;
    report.backbufferHeight = presentationOracle.backbufferHeight;
    report.rendererBackendCommands = presentationOracle.rendererBackendCommands;
    report.drawSubmissions = presentationOracle.drawSubmissions;
    report.linkedDeviceExecutionRecords =
        deviceExecution.resourceCreationRecords + deviceExecution.bindingRecords;
    report.linkedPresentationRecords = presentationOracle.presentationRecords;
    report.readyPlatformInputRecords = deviceExecution.readyResourceCreationRecords
        + deviceExecution.readyBindingRecords + presentationOracle.readyPresentationRecords;
    report.trackedOpenPlatformInputRecords = deviceExecution.trackedOpenResourceCreationRecords
        + deviceExecution.trackedOpenBindingRecords + presentationOracle.trackedOpenPresentationRecords;

    report.records.push_back(makePlatformBridgeCallRecord(
        "diagnostic_backend_bridge",
        "BackendBridge::submitFrame",
        "L24 device execution + L25 presentation/oracle",
        deviceExecution.deviceExecutionRuntimeReady && presentationOracle.presentationOracleRuntimeReady
            ? "contract_ready"
            : "blocked",
        report.linkedDeviceExecutionRecords + report.linkedPresentationRecords,
        report.readyPlatformInputRecords,
        report.trackedOpenPlatformInputRecords,
        1,
        report.backbufferWidth,
        report.backbufferHeight));
    report.records.push_back(makePlatformBridgeCallRecord(
        "platform_window_surface",
        "CreateWindowSurface",
        "L25 window_surface_candidate",
        presentationOracle.windowSurfaceGateTracked ? "tracked_open" : "blocked",
        presentationOracle.windowSurfaceCandidates,
        0,
        presentationOracle.windowSurfaceCandidates,
        presentationOracle.windowSurfaceCandidates,
        report.backbufferWidth,
        report.backbufferHeight));
    report.records.push_back(makePlatformBridgeCallRecord(
        "d3d9_interface_creation",
        "Direct3DCreate9",
        "L20 renderer_profile=d3d9_compatible",
        presentationOracle.presentationOracleRuntimeReady ? "tracked_open" : "blocked",
        1,
        0,
        1,
        1,
        0,
        0));
    report.records.push_back(makePlatformBridgeCallRecord(
        "d3d9_device_creation",
        "IDirect3D9::CreateDevice",
        "L25 swapchain_backbuffer_extent",
        presentationOracle.swapchainCreationGateTracked ? "tracked_open" : "blocked",
        presentationOracle.backbufferExtentRecords,
        0,
        presentationOracle.backbufferExtentRecords,
        1,
        report.backbufferWidth,
        report.backbufferHeight));
    report.records.push_back(makePlatformBridgeCallRecord(
        "d3d9_resource_creation_queue",
        "CreateTexture/CreateCubeTexture/CreateRenderTarget/CreateDepthStencilSurface",
        "L24 resource_records",
        deviceExecution.resourceCreationRecordsReady ? "contract_ready" : "blocked",
        deviceExecution.resourceCreationRecords,
        deviceExecution.readyResourceCreationRecords,
        deviceExecution.trackedOpenResourceCreationRecords,
        deviceExecution.resourceCreationRecords,
        0,
        0));
    report.records.push_back(makePlatformBridgeCallRecord(
        "d3d9_texture_upload_queue",
        "LockRect/UnlockRect",
        "L24 upload_subresource_records",
        deviceExecution.uploadExecutionRecordsReady ? "contract_ready" : "blocked",
        deviceExecution.textureUploadExecutionRecords,
        deviceExecution.textureUploadExecutionRecords,
        0,
        deviceExecution.uploadSubresourceRecords,
        0,
        0));
    report.records.push_back(makePlatformBridgeCallRecord(
        "d3d9_state_binding_queue",
        "SetTexture/SetSamplerState/SetRenderState",
        "L24 binding_records",
        deviceExecution.stateBindingRecordsReady && deviceExecution.lookupTextureBindingRecordsReady
            ? "contract_ready"
            : "blocked",
        deviceExecution.bindingRecords,
        deviceExecution.readyBindingRecords,
        deviceExecution.trackedOpenBindingRecords,
        deviceExecution.bindingRecords,
        0,
        0));
    report.records.push_back(makePlatformBridgeCallRecord(
        "d3d9_draw_submission_queue",
        "DrawIndexedPrimitive/DrawPrimitiveUP",
        "L16 renderer submissions via L25 present gate",
        presentationOracle.presentCallGateTracked ? "tracked_open" : "blocked",
        presentationOracle.drawSubmissions,
        0,
        presentationOracle.drawSubmissions,
        presentationOracle.drawSubmissions,
        0,
        0));
    report.records.push_back(makePlatformBridgeCallRecord(
        "d3d9_present_call",
        "IDirect3DDevice9::Present",
        "L25 present_call_candidate",
        presentationOracle.presentCallGateTracked ? "tracked_open" : "blocked",
        presentationOracle.presentCallCandidates,
        0,
        presentationOracle.presentCallCandidates,
        presentationOracle.presentCallCandidates,
        report.backbufferWidth,
        report.backbufferHeight));
    report.records.push_back(makePlatformBridgeCallRecord(
        "frame_capture_oracle_queue",
        "GetFrontBufferData/oracle_compare",
        "L25 frame_capture_candidate + original_frame_oracle_trace",
        presentationOracle.frameCaptureGateTracked && presentationOracle.originalFrameOracleGateTracked
            ? "tracked_open"
            : "blocked",
        presentationOracle.frameCaptureCandidates + presentationOracle.oracleTraceCandidates,
        0,
        presentationOracle.frameCaptureCandidates + presentationOracle.oracleTraceCandidates,
        presentationOracle.frameCaptureCandidates + presentationOracle.oracleTraceCandidates,
        report.backbufferWidth,
        report.backbufferHeight));

    for (const auto& record : report.records) {
        ++report.bridgeCallRecords;
        if (record.ready) {
            ++report.readyBridgeCallRecords;
        } else if (record.status == "tracked_open") {
            ++report.trackedOpenBridgeCallRecords;
        }

        if (record.name == "diagnostic_backend_bridge") {
            report.diagnosticCallBatches += record.callCount;
        } else if (record.name == "platform_window_surface") {
            report.platformSurfaceRecords += record.callCount;
        } else if (record.name == "d3d9_interface_creation") {
            report.d3dInterfaceRecords += record.callCount;
        } else if (record.name == "d3d9_device_creation") {
            report.createDeviceRecords += record.callCount;
        } else if (record.name == "d3d9_resource_creation_queue") {
            report.resourceCreationCallRecords += record.callCount;
        } else if (record.name == "d3d9_texture_upload_queue") {
            report.uploadSubresourceCalls += record.callCount;
        } else if (record.name == "d3d9_state_binding_queue") {
            report.stateBindingCallRecords += record.callCount;
        } else if (record.name == "d3d9_draw_submission_queue") {
            report.drawSubmissionCalls += record.callCount;
        } else if (record.name == "d3d9_present_call") {
            report.presentCallRecords += record.callCount;
        } else if (record.name == "frame_capture_oracle_queue") {
            report.captureOracleCallRecords += record.callCount;
        }
    }

    report.textureCreateCalls = deviceExecution.createTextureRecords;
    report.cubeTextureCreateCalls = deviceExecution.createCubeTextureRecords;
    report.renderTargetCreateCandidates = deviceExecution.renderTargetCreationCandidates;
    report.depthStencilCreateCandidates = deviceExecution.depthStencilCreationCandidates;
    report.fontAtlasCreateCandidates = deviceExecution.fontAtlasCreationPlaceholders;
    report.uploadCallRecords = deviceExecution.textureUploadExecutionRecords;
    report.readyStateBindingCalls = deviceExecution.readyBindingRecords;
    report.trackedOpenStateBindingCalls = deviceExecution.trackedOpenBindingRecords;
    report.setTextureCalls =
        deviceExecution.materialTextureBindingRecords + deviceExecution.samplerTextureBindingRecords;
    report.setSamplerStateCalls = deviceExecution.samplerStateBindingRecords;
    report.setRenderStateBundleCalls = deviceExecution.renderStateBindingRecords;

    report.diagnosticBridgeReady = report.linkedDeviceExecutionRecords == 103
        && report.linkedPresentationRecords == 7 && report.readyPlatformInputRecords == 57
        && report.trackedOpenPlatformInputRecords == 53 && report.diagnosticCallBatches == 1;
    report.creationSubmissionQueueReady = deviceExecution.resourceCreationRecordsReady
        && report.resourceCreationCallRecords == 46 && report.textureCreateCalls == 40
        && report.cubeTextureCreateCalls == 1 && report.renderTargetCreateCandidates == 3
        && report.depthStencilCreateCandidates == 1 && report.fontAtlasCreateCandidates == 1;
    report.uploadSubmissionQueueReady = deviceExecution.uploadExecutionRecordsReady
        && report.uploadCallRecords == 41 && report.uploadSubresourceCalls == 458;
    report.stateSubmissionQueueReady = deviceExecution.stateBindingRecordsReady
        && deviceExecution.lookupTextureBindingRecordsReady && report.stateBindingCallRecords == 57
        && report.readyStateBindingCalls == 14 && report.trackedOpenStateBindingCalls == 43
        && report.setTextureCalls == 45 && report.setSamplerStateCalls == 7
        && report.setRenderStateBundleCalls == 5;
    report.presentationSubmissionQueueTracked = presentationOracle.presentationOracleRuntimeReady
        && report.platformSurfaceRecords == 1 && report.d3dInterfaceRecords == 1
        && report.createDeviceRecords == 1 && report.presentCallRecords == 1
        && report.backbufferWidth == 1280 && report.backbufferHeight == 720;
    report.d3dConcreteBackendGateTracked = report.presentationSubmissionQueueTracked
        && report.resourceCreationCallRecords == 46 && report.stateBindingCallRecords == 57;
    report.frameCaptureGateTracked =
        presentationOracle.frameCaptureGateTracked && report.captureOracleCallRecords == 2;
    report.originalOracleGateTracked =
        presentationOracle.originalFrameOracleGateTracked && report.captureOracleCallRecords == 2;
    report.platformBridgeRuntimeReady = report.deviceExecutionOk && report.presentationOracleOk
        && report.diagnosticBridgeReady && report.creationSubmissionQueueReady
        && report.uploadSubmissionQueueReady && report.stateSubmissionQueueReady
        && report.presentationSubmissionQueueTracked && report.d3dConcreteBackendGateTracked
        && report.frameCaptureGateTracked && report.originalOracleGateTracked
        && report.bridgeCallRecords == 10 && report.readyBridgeCallRecords == 4
        && report.trackedOpenBridgeCallRecords == 6;

    report.contracts.push_back(makeBackendObligation(
        "diagnostic_backend_bridge",
        report.diagnosticBridgeReady ? "contract_ready" : "blocked",
        "backend bridge receives 103 L24 device records and 7 L25 presentation/oracle records"));
    report.contracts.push_back(makeBackendObligation(
        "resource_creation_submission_queue",
        report.creationSubmissionQueueReady ? "contract_ready" : "blocked",
        "46 resource creation calls map to L24 CreateTexture/CreateCubeTexture/render/depth/font records"));
    report.contracts.push_back(makeBackendObligation(
        "texture_upload_submission_queue",
        report.uploadSubmissionQueueReady ? "contract_ready" : "blocked",
        "41 ready textures expand to 458 LockRect/UnlockRect style upload calls"));
    report.contracts.push_back(makeBackendObligation(
        "state_binding_submission_queue",
        report.stateSubmissionQueueReady ? "contract_ready" : "blocked",
        "57 binding calls map to SetTexture, SetSamplerState, and SetRenderState bundles"));
    report.contracts.push_back(makeBackendObligation(
        "platform_window_surface",
        report.presentationSubmissionQueueTracked ? "tracked_open" : "blocked",
        "window surface creation is queued but not executed"));
    report.contracts.push_back(makeBackendObligation(
        "d3d9_interface_and_device_creation",
        report.d3dConcreteBackendGateTracked ? "tracked_open" : "blocked",
        "Direct3DCreate9/CreateDevice are queued but no concrete D3D device exists"));
    report.contracts.push_back(makeBackendObligation(
        "draw_submission_backend",
        report.drawSubmissionCalls == 121 ? "tracked_open" : "blocked",
        "121 draw submissions exist but vertex/index buffer binding and draw calls are not executed"));
    report.contracts.push_back(makeBackendObligation(
        "present_execution",
        report.presentCallRecords == 1 ? "tracked_open" : "blocked",
        "Present is queued but not called on an IDirect3DDevice9"));
    report.contracts.push_back(makeBackendObligation(
        "frame_capture_artifact",
        report.frameCaptureGateTracked ? "tracked_open" : "blocked",
        "frame capture is queued but no YuEngine frame artifact exists"));
    report.contracts.push_back(makeBackendObligation(
        "original_frame_oracle_parity",
        report.originalOracleGateTracked ? "tracked_open" : "blocked",
        "original screenshot/API trace parity is still absent"));

    for (const auto& contract : report.contracts) {
        if (contract.status == "contract_ready") {
            ++report.resolvedPlatformBridgeContracts;
        } else if (contract.status == "tracked_open") {
            ++report.trackedPlatformBridgeObligations;
            ++report.openPlatformBridgeObligations;
        } else {
            ++report.openPlatformBridgeObligations;
        }
    }

    if (!report.deviceExecutionOk) {
        addError(report, "device execution contract is not ready for platform bridge");
    }
    if (!report.presentationOracleOk) {
        addError(report, "presentation/oracle contract is not ready for platform bridge");
    }
    if (!report.diagnosticBridgeReady || !report.creationSubmissionQueueReady
        || !report.uploadSubmissionQueueReady || !report.stateSubmissionQueueReady) {
        addError(report, "platform bridge ready submission queues are incomplete");
    }
    if (!report.presentationSubmissionQueueTracked || !report.d3dConcreteBackendGateTracked
        || !report.frameCaptureGateTracked || !report.originalOracleGateTracked) {
        addError(report, "platform bridge open gates are not fully tracked");
    }
    if (report.resolvedPlatformBridgeContracts != 4 || report.trackedPlatformBridgeObligations != 6
        || report.openPlatformBridgeObligations != 6) {
        addError(report, "platform bridge obligation accounting is inconsistent");
    }

    return report;
}

BackendExecutorRuntimeReport buildBackendExecutorRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile,
    const resource::VirtualFileSystem& vfs)
{
    BackendExecutorRuntimeReport report;
    const auto platformBridge = buildBackendPlatformBridgeRuntimeReport(inputs, rendererProfile, vfs);

    report.projectId = inputs.sceneRuntime.projectId;
    report.platformBridgeOk = platformBridge.ok;
    report.consumedBridgeCallRecords = platformBridge.bridgeCallRecords;
    report.readyBridgeCallRecords = platformBridge.readyBridgeCallRecords;
    report.trackedOpenBridgeCallRecords = platformBridge.trackedOpenBridgeCallRecords;
    report.readyPlatformInputRecords = platformBridge.readyPlatformInputRecords;
    report.trackedOpenPlatformInputRecords = platformBridge.trackedOpenPlatformInputRecords;
    report.linkedPlatformInputRecords =
        report.readyPlatformInputRecords + report.trackedOpenPlatformInputRecords;
    report.backbufferWidth = platformBridge.backbufferWidth;
    report.backbufferHeight = platformBridge.backbufferHeight;

    for (const auto& call : platformBridge.records) {
        const std::string resultStatus = call.ready ? "diagnostic_success" : call.status;
        report.results.push_back(makeExecutorResultRecord(call, "diagnostic_d3d9_adapter", resultStatus));
    }

    for (const auto& result : report.results) {
        ++report.executionResultRecords;
        report.resultCallCountTotal += result.callCount;
        report.diagnosticExecutedCalls += result.executedCalls;
        report.preservedOpenCalls += result.preservedOpenCalls;

        if (result.resultStatus == "diagnostic_success") {
            ++report.diagnosticSuccessRecords;
        } else if (result.resultStatus == "tracked_open") {
            ++report.trackedOpenExecutionRecords;
        } else {
            ++report.blockedExecutionRecords;
        }

        if (result.sourceBridgeRecord == "diagnostic_backend_bridge") {
            report.submittedDiagnosticBatches += result.executedCalls;
        } else if (result.sourceBridgeRecord == "d3d9_resource_creation_queue") {
            report.executedResourceCreationCalls += result.executedCalls;
        } else if (result.sourceBridgeRecord == "d3d9_texture_upload_queue") {
            report.executedUploadSubresourceCalls += result.executedCalls;
        } else if (result.sourceBridgeRecord == "d3d9_state_binding_queue") {
            report.executedStateBindingCalls += result.executedCalls;
        } else if (result.sourceBridgeRecord == "platform_window_surface") {
            report.preservedPlatformSurfaceGates += result.preservedOpenCalls;
        } else if (result.sourceBridgeRecord == "d3d9_interface_creation"
            || result.sourceBridgeRecord == "d3d9_device_creation") {
            report.preservedDeviceCreationGates += result.preservedOpenCalls;
        } else if (result.sourceBridgeRecord == "d3d9_draw_submission_queue") {
            report.preservedDrawSubmissionGates += result.preservedOpenCalls;
        } else if (result.sourceBridgeRecord == "d3d9_present_call") {
            report.preservedPresentGates += result.preservedOpenCalls;
        } else if (result.sourceBridgeRecord == "frame_capture_oracle_queue") {
            report.preservedCaptureOracleGates += result.preservedOpenCalls;
        }
    }

    report.diagnosticExecutorReady = platformBridge.platformBridgeRuntimeReady
        && report.submittedDiagnosticBatches == 1 && report.diagnosticSuccessRecords == 4
        && report.diagnosticExecutedCalls == 562;
    report.oneToOneBridgeMappingReady = report.executionResultRecords == platformBridge.bridgeCallRecords
        && report.consumedBridgeCallRecords == 10 && report.executionResultRecords == 10;
    report.readyCallExecutionResultsReady = report.readyBridgeCallRecords == 4
        && report.diagnosticSuccessRecords == 4 && report.executedResourceCreationCalls == 46
        && report.executedUploadSubresourceCalls == 458 && report.executedStateBindingCalls == 57;
    report.trackedOpenCallResultsReady = report.trackedOpenBridgeCallRecords == 6
        && report.trackedOpenExecutionRecords == 6 && report.blockedExecutionRecords == 0
        && report.preservedOpenCalls == 127;
    report.hwndDeviceGateTracked =
        report.preservedPlatformSurfaceGates == 1 && report.preservedDeviceCreationGates == 2;
    report.drawExecutionGateTracked = report.preservedDrawSubmissionGates == 121;
    report.presentExecutionGateTracked = report.preservedPresentGates == 1;
    report.frameCaptureGateTracked = report.preservedCaptureOracleGates == 2;
    report.originalOracleGateTracked = report.preservedCaptureOracleGates == 2;
    report.concreteD3DExecutionGateTracked = report.hwndDeviceGateTracked
        && report.drawExecutionGateTracked && report.presentExecutionGateTracked;
    report.executorRuntimeReady = report.platformBridgeOk && report.diagnosticExecutorReady
        && report.oneToOneBridgeMappingReady && report.readyCallExecutionResultsReady
        && report.trackedOpenCallResultsReady && report.concreteD3DExecutionGateTracked
        && report.frameCaptureGateTracked && report.originalOracleGateTracked
        && report.resultCallCountTotal == 689 && report.linkedPlatformInputRecords == 110
        && report.readyPlatformInputRecords == 57 && report.trackedOpenPlatformInputRecords == 53
        && report.backbufferWidth == 1280 && report.backbufferHeight == 720;

    report.contracts.push_back(makeBackendObligation(
        "executor_consumes_platform_bridge",
        report.oneToOneBridgeMappingReady ? "contract_ready" : "blocked",
        "10 executor result records map one-to-one to 10 L26 bridge call records"));
    report.contracts.push_back(makeBackendObligation(
        "diagnostic_success_result_mapping",
        report.diagnosticExecutorReady ? "contract_ready" : "blocked",
        "diagnostic adapter accepts 4 ready bridge records and 562 diagnostic calls"));
    report.contracts.push_back(makeBackendObligation(
        "ready_call_execution_results",
        report.readyCallExecutionResultsReady ? "contract_ready" : "blocked",
        "resource creation, upload, and state binding ready queues produce diagnostic success"));
    report.contracts.push_back(makeBackendObligation(
        "tracked_open_result_preservation",
        report.trackedOpenCallResultsReady ? "contract_ready" : "blocked",
        "6 tracked-open bridge records preserve 127 concrete/platform/oracle calls"));
    report.contracts.push_back(makeBackendObligation(
        "concrete_hwnd_and_d3d_device_execution",
        report.hwndDeviceGateTracked ? "tracked_open" : "blocked",
        "HWND, Direct3DCreate9, and CreateDevice execution remain unimplemented"));
    report.contracts.push_back(makeBackendObligation(
        "draw_execution_backend",
        report.drawExecutionGateTracked ? "tracked_open" : "blocked",
        "121 draw submissions remain queued until vertex/index buffers and draw calls exist"));
    report.contracts.push_back(makeBackendObligation(
        "present_execution",
        report.presentExecutionGateTracked ? "tracked_open" : "blocked",
        "Present remains queued until a concrete device and swapchain exist"));
    report.contracts.push_back(makeBackendObligation(
        "frame_capture_artifact",
        report.frameCaptureGateTracked ? "tracked_open" : "blocked",
        "frame capture remains queued until a YuEngine frame artifact exists"));
    report.contracts.push_back(makeBackendObligation(
        "original_frame_oracle_parity",
        report.originalOracleGateTracked ? "tracked_open" : "blocked",
        "original screenshot/API trace parity is still absent"));
    report.contracts.push_back(makeBackendObligation(
        "real_d3d9_adapter",
        report.concreteD3DExecutionGateTracked ? "tracked_open" : "blocked",
        "diagnostic adapter exists, but real D3D9 calls are not executed"));

    for (const auto& contract : report.contracts) {
        if (contract.status == "contract_ready") {
            ++report.resolvedExecutorContracts;
        } else if (contract.status == "tracked_open") {
            ++report.trackedExecutorObligations;
            ++report.openExecutorObligations;
        } else {
            ++report.openExecutorObligations;
        }
    }

    if (!report.platformBridgeOk) {
        addError(report, "platform bridge contract is not ready for backend executor");
    }
    if (!report.diagnosticExecutorReady || !report.oneToOneBridgeMappingReady
        || !report.readyCallExecutionResultsReady || !report.trackedOpenCallResultsReady) {
        addError(report, "backend executor result mapping is incomplete");
    }
    if (!report.concreteD3DExecutionGateTracked || !report.frameCaptureGateTracked
        || !report.originalOracleGateTracked) {
        addError(report, "backend executor open gates are not fully tracked");
    }
    if (report.resolvedExecutorContracts != 4 || report.trackedExecutorObligations != 6
        || report.openExecutorObligations != 6) {
        addError(report, "backend executor obligation accounting is inconsistent");
    }

    return report;
}

BackendDeviceAdapterRuntimeReport buildBackendDeviceAdapterRuntimeReport(
    const GameplayFrameInputs& inputs,
    const std::string& rendererProfile,
    const resource::VirtualFileSystem& vfs)
{
    BackendDeviceAdapterRuntimeReport report;
    const auto executor = buildBackendExecutorRuntimeReport(inputs, rendererProfile, vfs);

    report.projectId = inputs.sceneRuntime.projectId;
    report.backendExecutorOk = executor.ok;
    report.consumedExecutorResultRecords = executor.executionResultRecords;
    report.linkedPlatformInputRecords = executor.linkedPlatformInputRecords;
    report.readyPlatformInputRecords = executor.readyPlatformInputRecords;
    report.trackedOpenPlatformInputRecords = executor.trackedOpenPlatformInputRecords;
    report.backbufferWidth = executor.backbufferWidth;
    report.backbufferHeight = executor.backbufferHeight;

    for (const auto& result : executor.results) {
        report.records.push_back(
            makeDeviceAdapterRecord(result, executor.backbufferWidth, executor.backbufferHeight));
    }

    for (const auto& record : report.records) {
        ++report.adapterRecordCount;
        report.realExecutedCalls += record.realExecutedCalls;
        report.blockedRealCallsTotal += record.blockedRealCalls;
        report.inheritedDiagnosticExecutedCalls += record.inheritedExecutedCalls;
        report.inheritedPreservedOpenCalls += record.inheritedPreservedOpenCalls;

        if (record.sourceReady) {
            ++report.sourceDiagnosticSuccessRecords;
        } else {
            ++report.sourceTrackedOpenRecords;
        }

        if (record.stage == "diagnostic_context") {
            ++report.diagnosticContextRecords;
        } else if (record.stage == "window_surface") {
            ++report.platformDeviceAdapterRecords;
            ++report.windowSurfaceAdapterRecords;
            report.platformDevicePreconditionCalls += record.callCount;
        } else if (record.stage == "d3d9_interface") {
            ++report.platformDeviceAdapterRecords;
            ++report.d3dInterfaceAdapterRecords;
            report.platformDevicePreconditionCalls += record.callCount;
        } else if (record.stage == "d3d9_device") {
            ++report.platformDeviceAdapterRecords;
            ++report.createDeviceAdapterRecords;
            report.platformDevicePreconditionCalls += record.callCount;
        } else if (record.stage == "downstream_resource_queue") {
            ++report.downstreamBlockedRecords;
            ++report.resourceQueueBlockedRecords;
            report.downstreamRealCallsBlockedUntilDevice += record.callCount;
        } else {
            ++report.downstreamBlockedRecords;
            ++report.renderQueueBlockedRecords;
            report.downstreamRealCallsBlockedUntilDevice += record.callCount;
        }

        if (record.sourceBridgeRecord == "d3d9_resource_creation_queue") {
            report.realResourceCreationCallsBlocked += record.callCount;
        } else if (record.sourceBridgeRecord == "d3d9_texture_upload_queue") {
            report.realUploadSubresourceCallsBlocked += record.callCount;
        } else if (record.sourceBridgeRecord == "d3d9_state_binding_queue") {
            report.realStateBindingCallsBlocked += record.callCount;
        } else if (record.sourceBridgeRecord == "d3d9_draw_submission_queue") {
            report.realDrawCallsBlocked += record.callCount;
        } else if (record.sourceBridgeRecord == "d3d9_present_call") {
            report.realPresentCallsBlocked += record.callCount;
        } else if (record.sourceBridgeRecord == "frame_capture_oracle_queue") {
            report.realCaptureOracleCallsBlocked += record.callCount;
        }
    }

    report.executorResultsConsumedReady = report.backendExecutorOk
        && report.adapterRecordCount == executor.executionResultRecords
        && report.consumedExecutorResultRecords == 10 && report.adapterRecordCount == 10;
    report.realWindowSurfaceGateTracked = report.windowSurfaceAdapterRecords == 1;
    report.realD3DInterfaceGateTracked = report.d3dInterfaceAdapterRecords == 1;
    report.realD3DDeviceGateTracked = report.createDeviceAdapterRecords == 1;
    report.platformDevicePreconditionsTracked = report.platformDeviceAdapterRecords == 3
        && report.platformDevicePreconditionCalls == 3 && report.realWindowSurfaceGateTracked
        && report.realD3DInterfaceGateTracked && report.realD3DDeviceGateTracked;
    report.resourceExecutionRequiresDevice = report.resourceQueueBlockedRecords == 3
        && report.realResourceCreationCallsBlocked == 46
        && report.realUploadSubresourceCallsBlocked == 458
        && report.realStateBindingCallsBlocked == 57;
    report.drawPresentCaptureRequiresDevice = report.renderQueueBlockedRecords == 3
        && report.realDrawCallsBlocked == 121 && report.realPresentCallsBlocked == 1
        && report.realCaptureOracleCallsBlocked == 2;
    report.downstreamExecutionBlockedUntilDevice = report.downstreamBlockedRecords == 6
        && report.resourceExecutionRequiresDevice && report.drawPresentCaptureRequiresDevice
        && report.downstreamRealCallsBlockedUntilDevice == 685
        && report.blockedRealCallsTotal == 688 && report.realExecutedCalls == 0;
    report.backbufferExtentCarried =
        report.backbufferWidth == 1280 && report.backbufferHeight == 720;
    report.realDeviceHandleReady = false;
    report.deviceAdapterRuntimeReady = report.executorResultsConsumedReady
        && report.platformDevicePreconditionsTracked
        && report.downstreamExecutionBlockedUntilDevice
        && report.backbufferExtentCarried
        && report.sourceDiagnosticSuccessRecords == 4
        && report.sourceTrackedOpenRecords == 6
        && report.diagnosticContextRecords == 1
        && report.inheritedDiagnosticExecutedCalls == 562
        && report.inheritedPreservedOpenCalls == 127
        && report.linkedPlatformInputRecords == 110
        && report.readyPlatformInputRecords == 57
        && report.trackedOpenPlatformInputRecords == 53
        && !report.realDeviceHandleReady;

    report.contracts.push_back(makeBackendObligation(
        "device_adapter_consumes_executor_results",
        report.executorResultsConsumedReady ? "contract_ready" : "blocked",
        "10 L28 adapter records map one-to-one to 10 L27 executor results"));
    report.contracts.push_back(makeBackendObligation(
        "platform_device_preconditions_tracked",
        report.platformDevicePreconditionsTracked ? "contract_ready" : "blocked",
        "window surface, Direct3DCreate9, and CreateDevice gates are distinct records"));
    report.contracts.push_back(makeBackendObligation(
        "downstream_execution_blocked_until_device",
        report.downstreamExecutionBlockedUntilDevice ? "contract_ready" : "blocked",
        "685 downstream resource/draw/present/capture calls cannot execute without a device"));
    report.contracts.push_back(makeBackendObligation(
        "backbuffer_extent_carried_to_adapter",
        report.backbufferExtentCarried ? "contract_ready" : "blocked",
        "1280x720 backbuffer extent is inherited from L25/L26/L27"));
    report.contracts.push_back(makeBackendObligation(
        "real_hwnd_surface_creation",
        report.realWindowSurfaceGateTracked ? "tracked_open" : "blocked",
        "no YuEngine-owned HWND has been created yet"));
    report.contracts.push_back(makeBackendObligation(
        "real_direct3d9_interface_creation",
        report.realD3DInterfaceGateTracked ? "tracked_open" : "blocked",
        "Direct3DCreate9 is not called yet"));
    report.contracts.push_back(makeBackendObligation(
        "real_d3d9_device_creation",
        report.realD3DDeviceGateTracked ? "tracked_open" : "blocked",
        "IDirect3DDevice9::CreateDevice is not called yet"));
    report.contracts.push_back(makeBackendObligation(
        "real_resource_execution_after_device",
        report.resourceExecutionRequiresDevice ? "tracked_open" : "blocked",
        "resource creation, upload, and state binding are blocked until device creation"));
    report.contracts.push_back(makeBackendObligation(
        "real_draw_present_capture_after_device",
        report.drawPresentCaptureRequiresDevice ? "tracked_open" : "blocked",
        "draw, present, and capture are blocked until a concrete frame can be produced"));
    report.contracts.push_back(makeBackendObligation(
        "original_frame_oracle_after_capture",
        report.drawPresentCaptureRequiresDevice ? "tracked_open" : "blocked",
        "original-frame parity remains blocked until YuEngine capture exists"));

    for (const auto& contract : report.contracts) {
        if (contract.status == "contract_ready") {
            ++report.resolvedDeviceAdapterContracts;
        } else if (contract.status == "tracked_open") {
            ++report.trackedDeviceAdapterObligations;
            ++report.openDeviceAdapterObligations;
        } else {
            ++report.openDeviceAdapterObligations;
        }
    }

    if (!report.backendExecutorOk) {
        addError(report, "backend executor contract is not ready for device adapter");
    }
    if (!report.executorResultsConsumedReady || !report.platformDevicePreconditionsTracked) {
        addError(report, "device adapter does not consume executor/device preconditions");
    }
    if (!report.downstreamExecutionBlockedUntilDevice || report.realExecutedCalls != 0) {
        addError(report, "downstream backend calls are not blocked behind device creation");
    }
    if (!report.backbufferExtentCarried) {
        addError(report, "device adapter lost the recovered backbuffer extent");
    }
    if (report.resolvedDeviceAdapterContracts != 4 || report.trackedDeviceAdapterObligations != 6
        || report.openDeviceAdapterObligations != 6) {
        addError(report, "device adapter obligation accounting is inconsistent");
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
        report.fontQueryCommands = uiCommandContainsCount(uiState, "font_query");
        report.fontScaleLimitCommands = uiCommandContainsCount(uiState, "font_scale_limit");
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
        if (report.fontQueryCommands < 5) {
            addError(report, "title menu font query payload is incomplete");
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
        << " truncated=" << (report.truncated ? "true" : "false")
        << " font_query_commands=" << report.fontQueryCommands
        << " font_scale_limit_commands=" << report.fontScaleLimitCommands << "\",\n";
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
        << ", \"font_query_commands\": " << report.fontQueryCommands
        << ", \"font_scale_limit_commands\": " << report.fontScaleLimitCommands
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

TextureUploadRuntimeReport runTextureUploadRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    TextureUploadRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report = buildTextureUploadRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer,
            vfs);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string textureUploadRuntimeReportToJson(const TextureUploadRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.texture_upload_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " scene_runtime_ok=" << (report.sceneRuntimeOk ? "true" : "false")
        << " backend_obligations_ok=" << (report.backendObligationsOk ? "true" : "false")
        << " material_semantics_ok=" << (report.materialSemanticsOk ? "true" : "false")
        << " device_presentation_ok=" << (report.devicePresentationOk ? "true" : "false")
        << " texture_upload_runtime_ready=" << (report.textureUploadRuntimeReady ? "true" : "false")
        << " dds_header_contract_ready=" << (report.ddsHeaderContractReady ? "true" : "false")
        << " payload_layout_contract_ready=" << (report.payloadLayoutContractReady ? "true" : "false")
        << " material_consumer_contract_ready=" << (report.materialConsumerContractReady ? "true" : "false")
        << " sampler_state_gate_tracked=" << (report.samplerStateGateTracked ? "true" : "false")
        << " blend_depth_state_gate_tracked=" << (report.blendDepthStateGateTracked ? "true" : "false")
        << " font_atlas_gate_tracked=" << (report.fontAtlasGateTracked ? "true" : "false")
        << " oracle_parity_gate_tracked=" << (report.oracleParityGateTracked ? "true" : "false")
        << " stage_texture_dependencies=" << report.stageTextureDependencies
        << " texture_upload_records=" << report.textureUploadRecords
        << " valid_dds_headers=" << report.validDdsHeaders
        << " dxt1_textures=" << report.dxt1Textures
        << " dxt5_textures=" << report.dxt5Textures
        << " cube_map_textures=" << report.cubeMapTextures
        << " cube_map_faces=" << report.cubeMapFaces
        << " material_slot_consumers=" << report.materialSlotConsumers
        << " unique_material_texture_uploads=" << report.uniqueMaterialTextureUploads
        << " duplicate_material_consumers=" << report.duplicateMaterialConsumers
        << " stage_only_texture_uploads=" << report.stageOnlyTextureUploads
        << " compressed_payload_matches=" << report.compressedPayloadMatches
        << " payload_byte_total=" << report.payloadByteTotal
        << " expected_payload_byte_total=" << report.expectedPayloadByteTotal
        << " mip9_textures=" << report.mip9Textures
        << " mip10_textures=" << report.mip10Textures
        << " mip11_textures=" << report.mip11Textures
        << " texture_width_min=" << report.textureWidthMin
        << " texture_width_max=" << report.textureWidthMax
        << " post_effect_samplers=" << report.postEffectSamplers
        << " title_text_submissions=" << report.titleTextSubmissions
        << " resolved_upload_contracts=" << report.resolvedUploadContracts
        << " tracked_upload_obligations=" << report.trackedUploadObligations
        << " open_upload_obligations=" << report.openUploadObligations << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"dds_summary\": {";
    out << "\"stage_texture_dependencies\": " << report.stageTextureDependencies
        << ", \"texture_upload_records\": " << report.textureUploadRecords
        << ", \"dds_magic_records\": " << report.ddsMagicRecords
        << ", \"valid_dds_headers\": " << report.validDdsHeaders
        << ", \"dxt1_textures\": " << report.dxt1Textures
        << ", \"dxt5_textures\": " << report.dxt5Textures
        << ", \"unsupported_texture_formats\": " << report.unsupportedTextureFormats
        << ", \"texture_width_min\": " << report.textureWidthMin
        << ", \"texture_width_max\": " << report.textureWidthMax
        << ", \"texture_height_min\": " << report.textureHeightMin
        << ", \"texture_height_max\": " << report.textureHeightMax
        << ", \"mip9_textures\": " << report.mip9Textures
        << ", \"mip10_textures\": " << report.mip10Textures
        << ", \"mip11_textures\": " << report.mip11Textures << "},\n";
    out << "  \"payload_layout\": {";
    out << "\"texture_byte_total\": " << report.textureByteTotal
        << ", \"payload_byte_total\": " << report.payloadByteTotal
        << ", \"expected_payload_byte_total\": " << report.expectedPayloadByteTotal
        << ", \"compressed_payload_matches\": " << report.compressedPayloadMatches << "},\n";
    out << "  \"material_consumers\": {";
    out << "\"material_slot_consumers\": " << report.materialSlotConsumers
        << ", \"unique_material_texture_uploads\": " << report.uniqueMaterialTextureUploads
        << ", \"duplicate_material_consumers\": " << report.duplicateMaterialConsumers
        << ", \"stage_only_texture_uploads\": " << report.stageOnlyTextureUploads
        << ", \"cube_map_textures\": " << report.cubeMapTextures
        << ", \"cube_map_faces\": " << report.cubeMapFaces << "},\n";
    out << "  \"gates\": {";
    out << "\"sampler_state_gate_tracked\": " << (report.samplerStateGateTracked ? "true" : "false")
        << ", \"blend_depth_state_gate_tracked\": " << (report.blendDepthStateGateTracked ? "true" : "false")
        << ", \"font_atlas_gate_tracked\": " << (report.fontAtlasGateTracked ? "true" : "false")
        << ", \"oracle_parity_gate_tracked\": " << (report.oracleParityGateTracked ? "true" : "false")
        << ", \"post_effect_samplers\": " << report.postEffectSamplers
        << ", \"resource_upload_submissions\": " << report.resourceUploadSubmissions
        << ", \"title_text_submissions\": " << report.titleTextSubmissions
        << ", \"title_string_size_queries\": " << report.titleStringSizeQueries
        << ", \"localized_menu_text_commands\": " << report.localizedMenuTextCommands << "},\n";
    out << "  \"contract_summary\": {";
    out << "\"resolved_upload_contracts\": " << report.resolvedUploadContracts
        << ", \"tracked_upload_obligations\": " << report.trackedUploadObligations
        << ", \"open_upload_obligations\": " << report.openUploadObligations << "},\n";
    out << "  \"contracts\": [\n";
    for (size_t i = 0; i < report.contracts.size(); ++i) {
        const auto& contract = report.contracts[i];
        out << "    {\"contract\": \"" << core::jsonEscape(contract.obligation)
            << "\", \"status\": \"" << core::jsonEscape(contract.status)
            << "\", \"evidence\": \"" << core::jsonEscape(contract.evidence) << "\"}";
        out << (i + 1 == report.contracts.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"records\": [\n";
    for (size_t i = 0; i < report.records.size(); ++i) {
        const auto& record = report.records[i];
        out << "    {\"path\": \"" << core::jsonEscape(record.path)
            << "\", \"role\": \"" << core::jsonEscape(record.role)
            << "\", \"found\": " << (record.found ? "true" : "false")
            << ", \"valid_dds\": " << (record.validDds ? "true" : "false")
            << ", \"format\": \"" << core::jsonEscape(record.format)
            << "\", \"width\": " << record.width
            << ", \"height\": " << record.height
            << ", \"mip_count\": " << record.mipCount
            << ", \"block_bytes\": " << record.blockBytes
            << ", \"cube_faces\": " << record.cubeFaces
            << ", \"material_consumer_count\": " << record.materialConsumerCount
            << ", \"byte_size\": " << record.byteSize
            << ", \"payload_bytes\": " << record.payloadBytes
            << ", \"expected_payload_bytes\": " << record.expectedPayloadBytes
            << ", \"compressed_payload_matches\": " << (record.compressedPayloadMatches ? "true" : "false")
            << "}";
        out << (i + 1 == report.records.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

BackendStateRuntimeReport runBackendStateRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    BackendStateRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report = buildBackendStateRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer,
            vfs);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string backendStateRuntimeReportToJson(const BackendStateRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.backend_state_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " texture_upload_ok=" << (report.textureUploadOk ? "true" : "false")
        << " device_presentation_ok=" << (report.devicePresentationOk ? "true" : "false")
        << " material_semantics_ok=" << (report.materialSemanticsOk ? "true" : "false")
        << " title_ui_ok=" << (report.titleUiOk ? "true" : "false")
        << " backend_state_runtime_ready=" << (report.backendStateRuntimeReady ? "true" : "false")
        << " sampler_state_records_ready=" << (report.samplerStateRecordsReady ? "true" : "false")
        << " pass_render_state_records_ready=" << (report.passRenderStateRecordsReady ? "true" : "false")
        << " font_atlas_records_ready=" << (report.fontAtlasRecordsReady ? "true" : "false")
        << " material_shader_program_gate_tracked=" << (report.materialShaderProgramGateTracked ? "true" : "false")
        << " gpu_state_binding_gate_tracked=" << (report.gpuStateBindingGateTracked ? "true" : "false")
        << " oracle_parity_gate_tracked=" << (report.oracleParityGateTracked ? "true" : "false")
        << " sampler_state_records=" << report.samplerStateRecords
        << " sampler_texture_bindings=" << report.samplerTextureBindings
        << " sampler_clamp_address_records=" << report.samplerClampAddressRecords
        << " sampler_linear_min_filters=" << report.samplerLinearMinFilters
        << " sampler_point_min_filters=" << report.samplerPointMinFilters
        << " sampler_srgb_true_records=" << report.samplerSrgbTrueRecords
        << " sampler_srgb_false_records=" << report.samplerSrgbFalseRecords
        << " pass_state_records=" << report.passStateRecords
        << " pass_vs30_shaders=" << report.passVs30Shaders
        << " pass_ps30_shaders=" << report.passPs30Shaders
        << " z_disabled_passes=" << report.zDisabledPasses
        << " alpha_blend_disabled_passes=" << report.alphaBlendDisabledPasses
        << " alpha_test_disabled_passes=" << report.alphaTestDisabledPasses
        << " srgb_write_enabled_passes=" << report.srgbWriteEnabledPasses
        << " srgb_write_disabled_passes=" << report.srgbWriteDisabledPasses
        << " stencil_enabled_passes=" << report.stencilEnabledPasses
        << " stencil_disabled_passes=" << report.stencilDisabledPasses
        << " stencil_replace_passes=" << report.stencilReplacePasses
        << " stencil_keep_passes=" << report.stencilKeepPasses
        << " stencil_equal_passes=" << report.stencilEqualPasses
        << " font_query_records=" << report.fontQueryRecords
        << " text_draw_commands=" << report.textDrawCommands
        << " graph_string_commands=" << report.graphStringCommands
        << " string_size_queries=" << report.stringSizeQueries
        << " localized_menu_text_commands=" << report.localizedMenuTextCommands
        << " texture_upload_records=" << report.textureUploadRecords
        << " material_texture_consumers=" << report.materialTextureConsumers
        << " resolved_backend_state_contracts=" << report.resolvedBackendStateContracts
        << " tracked_backend_state_obligations=" << report.trackedBackendStateObligations
        << " open_backend_state_obligations=" << report.openBackendStateObligations << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"sampler_summary\": {";
    out << "\"records\": " << report.samplerStateRecords
        << ", \"texture_bindings\": " << report.samplerTextureBindings
        << ", \"clamp_address_records\": " << report.samplerClampAddressRecords
        << ", \"linear_min_filters\": " << report.samplerLinearMinFilters
        << ", \"point_min_filters\": " << report.samplerPointMinFilters
        << ", \"srgb_true_records\": " << report.samplerSrgbTrueRecords
        << ", \"srgb_false_records\": " << report.samplerSrgbFalseRecords << "},\n";
    out << "  \"pass_state_summary\": {";
    out << "\"records\": " << report.passStateRecords
        << ", \"vs30_shaders\": " << report.passVs30Shaders
        << ", \"ps30_shaders\": " << report.passPs30Shaders
        << ", \"z_disabled_passes\": " << report.zDisabledPasses
        << ", \"alpha_blend_disabled_passes\": " << report.alphaBlendDisabledPasses
        << ", \"alpha_test_disabled_passes\": " << report.alphaTestDisabledPasses
        << ", \"srgb_write_enabled_passes\": " << report.srgbWriteEnabledPasses
        << ", \"srgb_write_disabled_passes\": " << report.srgbWriteDisabledPasses
        << ", \"stencil_enabled_passes\": " << report.stencilEnabledPasses
        << ", \"stencil_disabled_passes\": " << report.stencilDisabledPasses
        << ", \"stencil_replace_passes\": " << report.stencilReplacePasses
        << ", \"stencil_keep_passes\": " << report.stencilKeepPasses
        << ", \"stencil_equal_passes\": " << report.stencilEqualPasses << "},\n";
    out << "  \"font_atlas_record\": {";
    out << "\"source\": \"" << core::jsonEscape(report.fontRecord.source)
        << "\", \"font_queries\": " << report.fontRecord.fontQueries
        << ", \"font_scale_limits\": " << report.fontRecord.fontScaleLimits
        << ", \"text_draw_commands\": " << report.fontRecord.textDrawCommands
        << ", \"graph_string_commands\": " << report.fontRecord.graphStringCommands
        << ", \"string_size_queries\": " << report.fontRecord.stringSizeQueries
        << ", \"localized_menu_text_commands\": " << report.fontRecord.localizedMenuTextCommands
        << ", \"draw_list_item_commands\": " << report.fontRecord.drawListItemCommands
        << ", \"glyph_metric_inputs_ready\": " << (report.fontRecord.glyphMetricInputsReady ? "true" : "false")
        << ", \"atlas_implementation_tracked\": " << (report.fontRecord.atlasImplementationTracked ? "true" : "false")
        << "},\n";
    out << "  \"contract_summary\": {";
    out << "\"resolved_backend_state_contracts\": " << report.resolvedBackendStateContracts
        << ", \"tracked_backend_state_obligations\": " << report.trackedBackendStateObligations
        << ", \"open_backend_state_obligations\": " << report.openBackendStateObligations << "},\n";
    out << "  \"contracts\": [\n";
    for (size_t i = 0; i < report.contracts.size(); ++i) {
        const auto& contract = report.contracts[i];
        out << "    {\"contract\": \"" << core::jsonEscape(contract.obligation)
            << "\", \"status\": \"" << core::jsonEscape(contract.status)
            << "\", \"evidence\": \"" << core::jsonEscape(contract.evidence) << "\"}";
        out << (i + 1 == report.contracts.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"samplers\": [\n";
    for (size_t i = 0; i < report.samplerRecords.size(); ++i) {
        const auto& sampler = report.samplerRecords[i];
        out << "    {\"name\": \"" << core::jsonEscape(sampler.name)
            << "\", \"texture\": \"" << core::jsonEscape(sampler.texture)
            << "\", \"address_u\": \"" << core::jsonEscape(sampler.addressU)
            << "\", \"address_v\": \"" << core::jsonEscape(sampler.addressV)
            << "\", \"address_w\": \"" << core::jsonEscape(sampler.addressW)
            << "\", \"mip_filter\": \"" << core::jsonEscape(sampler.mipFilter)
            << "\", \"min_filter\": \"" << core::jsonEscape(sampler.minFilter)
            << "\", \"mag_filter\": \"" << core::jsonEscape(sampler.magFilter)
            << "\", \"srgb_texture\": \"" << core::jsonEscape(sampler.srgbTexture)
            << "\", \"ready\": " << (sampler.ready ? "true" : "false") << "}";
        out << (i + 1 == report.samplerRecords.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"passes\": [\n";
    for (size_t i = 0; i < report.passRecords.size(); ++i) {
        const auto& pass = report.passRecords[i];
        out << "    {\"technique\": \"" << core::jsonEscape(pass.technique)
            << "\", \"pass\": \"" << core::jsonEscape(pass.pass)
            << "\", \"vertex_shader_profile\": \"" << core::jsonEscape(pass.vertexShaderProfile)
            << "\", \"pixel_shader_profile\": \"" << core::jsonEscape(pass.pixelShaderProfile)
            << "\", \"z_enable\": \"" << core::jsonEscape(pass.zEnable)
            << "\", \"srgb_write_enable\": \"" << core::jsonEscape(pass.srgbWriteEnable)
            << "\", \"alpha_blend_enable\": \"" << core::jsonEscape(pass.alphaBlendEnable)
            << "\", \"alpha_test_enable\": \"" << core::jsonEscape(pass.alphaTestEnable)
            << "\", \"stencil_enable\": \"" << core::jsonEscape(pass.stencilEnable)
            << "\", \"stencil_pass\": \"" << core::jsonEscape(pass.stencilPass)
            << "\", \"stencil_func\": \"" << core::jsonEscape(pass.stencilFunc)
            << "\", \"stencil_ref\": \"" << core::jsonEscape(pass.stencilRef)
            << "\", \"ready\": " << (pass.ready ? "true" : "false") << "}";
        out << (i + 1 == report.passRecords.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

BackendResourceAllocationRuntimeReport runBackendResourceAllocationRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    BackendResourceAllocationRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report = buildBackendResourceAllocationRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer,
            vfs);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string backendResourceAllocationRuntimeReportToJson(
    const BackendResourceAllocationRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.backend_resource_allocation_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " texture_upload_ok=" << (report.textureUploadOk ? "true" : "false")
        << " backend_state_ok=" << (report.backendStateOk ? "true" : "false")
        << " resource_allocation_runtime_ready=" << (report.resourceAllocationRuntimeReady ? "true" : "false")
        << " stage_texture_allocation_records_ready=" << (report.stageTextureAllocationRecordsReady ? "true" : "false")
        << " smaa_lookup_allocation_records_ready=" << (report.smaaLookupAllocationRecordsReady ? "true" : "false")
        << " transient_surface_allocation_gate_tracked=" << (report.transientSurfaceAllocationGateTracked ? "true" : "false")
        << " font_atlas_allocation_gate_tracked=" << (report.fontAtlasAllocationGateTracked ? "true" : "false")
        << " d3d_resource_creation_gate_tracked=" << (report.d3dResourceCreationGateTracked ? "true" : "false")
        << " oracle_parity_gate_tracked=" << (report.oracleParityGateTracked ? "true" : "false")
        << " allocation_records=" << report.allocationRecords
        << " ready_allocation_records=" << report.readyAllocationRecords
        << " tracked_open_allocation_records=" << report.trackedOpenAllocationRecords
        << " stage_texture_allocations=" << report.stageTextureAllocations
        << " d3d_dxt1_allocations=" << report.d3dDxt1Allocations
        << " d3d_dxt5_allocations=" << report.d3dDxt5Allocations
        << " cube_texture_allocations=" << report.cubeTextureAllocations
        << " smaa_lookup_allocations=" << report.smaaLookupAllocations
        << " lookup_l8_allocations=" << report.lookupL8Allocations
        << " lookup_a8l8_allocations=" << report.lookupA8L8Allocations
        << " transient_surface_candidates=" << report.transientSurfaceCandidates
        << " font_atlas_placeholders=" << report.fontAtlasPlaceholders
        << " sampler_texture_declarations=" << report.samplerTextureDeclarations
        << " material_texture_consumers=" << report.materialTextureConsumers
        << " ready_allocation_byte_total=" << report.readyAllocationByteTotal
        << " ready_allocation_payload_bytes=" << report.readyAllocationPayloadBytes
        << " ready_expected_payload_bytes=" << report.readyExpectedPayloadBytes
        << " resolved_allocation_contracts=" << report.resolvedAllocationContracts
        << " tracked_allocation_obligations=" << report.trackedAllocationObligations
        << " open_allocation_obligations=" << report.openAllocationObligations << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"allocation_summary\": {";
    out << "\"allocation_records\": " << report.allocationRecords
        << ", \"ready_allocation_records\": " << report.readyAllocationRecords
        << ", \"tracked_open_allocation_records\": " << report.trackedOpenAllocationRecords
        << ", \"stage_texture_allocations\": " << report.stageTextureAllocations
        << ", \"smaa_lookup_allocations\": " << report.smaaLookupAllocations
        << ", \"transient_surface_candidates\": " << report.transientSurfaceCandidates
        << ", \"font_atlas_placeholders\": " << report.fontAtlasPlaceholders << "},\n";
    out << "  \"format_summary\": {";
    out << "\"d3d_dxt1_allocations\": " << report.d3dDxt1Allocations
        << ", \"d3d_dxt5_allocations\": " << report.d3dDxt5Allocations
        << ", \"lookup_l8_allocations\": " << report.lookupL8Allocations
        << ", \"lookup_a8l8_allocations\": " << report.lookupA8L8Allocations
        << ", \"cube_texture_allocations\": " << report.cubeTextureAllocations << "},\n";
    out << "  \"ready_bytes\": {";
    out << "\"ready_allocation_byte_total\": " << report.readyAllocationByteTotal
        << ", \"ready_allocation_payload_bytes\": " << report.readyAllocationPayloadBytes
        << ", \"ready_expected_payload_bytes\": " << report.readyExpectedPayloadBytes << "},\n";
    out << "  \"gates\": {";
    out << "\"stage_texture_allocation_records_ready\": "
        << (report.stageTextureAllocationRecordsReady ? "true" : "false")
        << ", \"smaa_lookup_allocation_records_ready\": "
        << (report.smaaLookupAllocationRecordsReady ? "true" : "false")
        << ", \"transient_surface_allocation_gate_tracked\": "
        << (report.transientSurfaceAllocationGateTracked ? "true" : "false")
        << ", \"font_atlas_allocation_gate_tracked\": "
        << (report.fontAtlasAllocationGateTracked ? "true" : "false")
        << ", \"d3d_resource_creation_gate_tracked\": "
        << (report.d3dResourceCreationGateTracked ? "true" : "false")
        << ", \"oracle_parity_gate_tracked\": " << (report.oracleParityGateTracked ? "true" : "false")
        << "},\n";
    out << "  \"contract_summary\": {";
    out << "\"resolved_allocation_contracts\": " << report.resolvedAllocationContracts
        << ", \"tracked_allocation_obligations\": " << report.trackedAllocationObligations
        << ", \"open_allocation_obligations\": " << report.openAllocationObligations << "},\n";
    out << "  \"contracts\": [\n";
    for (size_t i = 0; i < report.contracts.size(); ++i) {
        const auto& contract = report.contracts[i];
        out << "    {\"contract\": \"" << core::jsonEscape(contract.obligation)
            << "\", \"status\": \"" << core::jsonEscape(contract.status)
            << "\", \"evidence\": \"" << core::jsonEscape(contract.evidence) << "\"}";
        out << (i + 1 == report.contracts.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"records\": [\n";
    for (size_t i = 0; i < report.records.size(); ++i) {
        const auto& record = report.records[i];
        out << "    {\"name\": \"" << core::jsonEscape(record.name)
            << "\", \"path\": \"" << core::jsonEscape(record.path)
            << "\", \"resource_kind\": \"" << core::jsonEscape(record.resourceKind)
            << "\", \"usage\": \"" << core::jsonEscape(record.usage)
            << "\", \"format\": \"" << core::jsonEscape(record.format)
            << "\", \"status\": \"" << core::jsonEscape(record.status)
            << "\", \"width\": " << record.width
            << ", \"height\": " << record.height
            << ", \"mip_levels\": " << record.mipLevels
            << ", \"cube_faces\": " << record.cubeFaces
            << ", \"material_consumer_count\": " << record.materialConsumerCount
            << ", \"byte_size\": " << record.byteSize
            << ", \"payload_bytes\": " << record.payloadBytes
            << ", \"expected_payload_bytes\": " << record.expectedPayloadBytes
            << ", \"ready\": " << (record.ready ? "true" : "false") << "}";
        out << (i + 1 == report.records.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

BackendDeviceExecutionRuntimeReport runBackendDeviceExecutionRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    BackendDeviceExecutionRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report = buildBackendDeviceExecutionRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer,
            vfs);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string backendDeviceExecutionRuntimeReportToJson(
    const BackendDeviceExecutionRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.backend_device_execution_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " resource_allocation_ok=" << (report.resourceAllocationOk ? "true" : "false")
        << " backend_state_ok=" << (report.backendStateOk ? "true" : "false")
        << " device_presentation_ok=" << (report.devicePresentationOk ? "true" : "false")
        << " device_execution_runtime_ready=" << (report.deviceExecutionRuntimeReady ? "true" : "false")
        << " resource_creation_records_ready=" << (report.resourceCreationRecordsReady ? "true" : "false")
        << " upload_execution_records_ready=" << (report.uploadExecutionRecordsReady ? "true" : "false")
        << " state_binding_records_ready=" << (report.stateBindingRecordsReady ? "true" : "false")
        << " lookup_texture_binding_records_ready=" << (report.lookupTextureBindingRecordsReady ? "true" : "false")
        << " material_texture_binding_gate_tracked=" << (report.materialTextureBindingGateTracked ? "true" : "false")
        << " transient_surface_binding_gate_tracked=" << (report.transientSurfaceBindingGateTracked ? "true" : "false")
        << " font_atlas_execution_gate_tracked=" << (report.fontAtlasExecutionGateTracked ? "true" : "false")
        << " d3d_api_call_submission_gate_tracked=" << (report.d3dApiCallSubmissionGateTracked ? "true" : "false")
        << " present_oracle_gate_tracked=" << (report.presentOracleGateTracked ? "true" : "false")
        << " resource_creation_records=" << report.resourceCreationRecords
        << " ready_resource_creation_records=" << report.readyResourceCreationRecords
        << " tracked_open_resource_creation_records=" << report.trackedOpenResourceCreationRecords
        << " create_texture_records=" << report.createTextureRecords
        << " create_cube_texture_records=" << report.createCubeTextureRecords
        << " render_target_creation_candidates=" << report.renderTargetCreationCandidates
        << " depth_stencil_creation_candidates=" << report.depthStencilCreationCandidates
        << " font_atlas_creation_placeholders=" << report.fontAtlasCreationPlaceholders
        << " texture_upload_execution_records=" << report.textureUploadExecutionRecords
        << " upload_subresource_records=" << report.uploadSubresourceRecords
        << " ready_upload_payload_bytes=" << report.readyUploadPayloadBytes
        << " binding_records=" << report.bindingRecords
        << " ready_binding_records=" << report.readyBindingRecords
        << " tracked_open_binding_records=" << report.trackedOpenBindingRecords
        << " material_texture_binding_records=" << report.materialTextureBindingRecords
        << " material_texture_binding_slots=" << report.materialTextureBindingSlots
        << " sampler_texture_binding_records=" << report.samplerTextureBindingRecords
        << " lookup_texture_binding_records=" << report.lookupTextureBindingRecords
        << " transient_sampler_binding_candidates=" << report.transientSamplerBindingCandidates
        << " sampler_state_binding_records=" << report.samplerStateBindingRecords
        << " render_state_binding_records=" << report.renderStateBindingRecords
        << " resolved_device_execution_contracts=" << report.resolvedDeviceExecutionContracts
        << " tracked_device_execution_obligations=" << report.trackedDeviceExecutionObligations
        << " open_device_execution_obligations=" << report.openDeviceExecutionObligations << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"resource_creation_summary\": {";
    out << "\"records\": " << report.resourceCreationRecords
        << ", \"ready_records\": " << report.readyResourceCreationRecords
        << ", \"tracked_open_records\": " << report.trackedOpenResourceCreationRecords
        << ", \"create_texture_records\": " << report.createTextureRecords
        << ", \"create_cube_texture_records\": " << report.createCubeTextureRecords
        << ", \"render_target_candidates\": " << report.renderTargetCreationCandidates
        << ", \"depth_stencil_candidates\": " << report.depthStencilCreationCandidates
        << ", \"font_atlas_placeholders\": " << report.fontAtlasCreationPlaceholders << "},\n";
    out << "  \"upload_execution_summary\": {";
    out << "\"texture_upload_execution_records\": " << report.textureUploadExecutionRecords
        << ", \"upload_subresource_records\": " << report.uploadSubresourceRecords
        << ", \"ready_upload_payload_bytes\": " << report.readyUploadPayloadBytes << "},\n";
    out << "  \"binding_summary\": {";
    out << "\"binding_records\": " << report.bindingRecords
        << ", \"ready_binding_records\": " << report.readyBindingRecords
        << ", \"tracked_open_binding_records\": " << report.trackedOpenBindingRecords
        << ", \"material_texture_binding_records\": " << report.materialTextureBindingRecords
        << ", \"material_texture_binding_slots\": " << report.materialTextureBindingSlots
        << ", \"sampler_texture_binding_records\": " << report.samplerTextureBindingRecords
        << ", \"lookup_texture_binding_records\": " << report.lookupTextureBindingRecords
        << ", \"transient_sampler_binding_candidates\": " << report.transientSamplerBindingCandidates
        << ", \"sampler_state_binding_records\": " << report.samplerStateBindingRecords
        << ", \"render_state_binding_records\": " << report.renderStateBindingRecords << "},\n";
    out << "  \"contract_summary\": {";
    out << "\"resolved_device_execution_contracts\": " << report.resolvedDeviceExecutionContracts
        << ", \"tracked_device_execution_obligations\": " << report.trackedDeviceExecutionObligations
        << ", \"open_device_execution_obligations\": " << report.openDeviceExecutionObligations << "},\n";
    out << "  \"contracts\": [\n";
    for (size_t i = 0; i < report.contracts.size(); ++i) {
        const auto& contract = report.contracts[i];
        out << "    {\"contract\": \"" << core::jsonEscape(contract.obligation)
            << "\", \"status\": \"" << core::jsonEscape(contract.status)
            << "\", \"evidence\": \"" << core::jsonEscape(contract.evidence) << "\"}";
        out << (i + 1 == report.contracts.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"resource_records\": [\n";
    for (size_t i = 0; i < report.resourceRecords.size(); ++i) {
        const auto& record = report.resourceRecords[i];
        out << "    {\"name\": \"" << core::jsonEscape(record.name)
            << "\", \"path\": \"" << core::jsonEscape(record.path)
            << "\", \"operation\": \"" << core::jsonEscape(record.operation)
            << "\", \"resource_kind\": \"" << core::jsonEscape(record.resourceKind)
            << "\", \"format\": \"" << core::jsonEscape(record.format)
            << "\", \"status\": \"" << core::jsonEscape(record.status)
            << "\", \"width\": " << record.width
            << ", \"height\": " << record.height
            << ", \"mip_levels\": " << record.mipLevels
            << ", \"cube_faces\": " << record.cubeFaces
            << ", \"subresource_count\": " << record.subresourceCount
            << ", \"material_binding_slots\": " << record.materialBindingSlots
            << ", \"byte_size\": " << record.byteSize
            << ", \"payload_bytes\": " << record.payloadBytes
            << ", \"ready\": " << (record.ready ? "true" : "false") << "}";
        out << (i + 1 == report.resourceRecords.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"binding_records_detail\": [\n";
    for (size_t i = 0; i < report.bindingRecordsDetail.size(); ++i) {
        const auto& record = report.bindingRecordsDetail[i];
        out << "    {\"name\": \"" << core::jsonEscape(record.name)
            << "\", \"operation\": \"" << core::jsonEscape(record.operation)
            << "\", \"target\": \"" << core::jsonEscape(record.target)
            << "\", \"source\": \"" << core::jsonEscape(record.source)
            << "\", \"status\": \"" << core::jsonEscape(record.status)
            << "\", \"binding_slots\": " << record.bindingSlots
            << ", \"ready\": " << (record.ready ? "true" : "false") << "}";
        out << (i + 1 == report.bindingRecordsDetail.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

BackendPresentationOracleRuntimeReport runBackendPresentationOracleRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    BackendPresentationOracleRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report = buildBackendPresentationOracleRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer,
            vfs);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string backendPresentationOracleRuntimeReportToJson(
    const BackendPresentationOracleRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.backend_presentation_oracle_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " device_execution_ok=" << (report.deviceExecutionOk ? "true" : "false")
        << " device_presentation_ok=" << (report.devicePresentationOk ? "true" : "false")
        << " presentation_oracle_runtime_ready=" << (report.presentationOracleRuntimeReady ? "true" : "false")
        << " backbuffer_extent_records_ready=" << (report.backbufferExtentRecordsReady ? "true" : "false")
        << " device_execution_input_records_ready=" << (report.deviceExecutionInputRecordsReady ? "true" : "false")
        << " window_surface_gate_tracked=" << (report.windowSurfaceGateTracked ? "true" : "false")
        << " swapchain_creation_gate_tracked=" << (report.swapchainCreationGateTracked ? "true" : "false")
        << " present_call_gate_tracked=" << (report.presentCallGateTracked ? "true" : "false")
        << " frame_capture_gate_tracked=" << (report.frameCaptureGateTracked ? "true" : "false")
        << " original_frame_oracle_gate_tracked=" << (report.originalFrameOracleGateTracked ? "true" : "false")
        << " presentation_records=" << report.presentationRecords
        << " ready_presentation_records=" << report.readyPresentationRecords
        << " tracked_open_presentation_records=" << report.trackedOpenPresentationRecords
        << " backbuffer_extent_records=" << report.backbufferExtentRecords
        << " backbuffer_width=" << report.backbufferWidth
        << " backbuffer_height=" << report.backbufferHeight
        << " device_execution_frame_inputs=" << report.deviceExecutionFrameInputs
        << " linked_device_execution_records=" << report.linkedDeviceExecutionRecords
        << " linked_device_ready_records=" << report.linkedDeviceReadyRecords
        << " linked_device_open_records=" << report.linkedDeviceOpenRecords
        << " window_surface_candidates=" << report.windowSurfaceCandidates
        << " swapchain_creation_candidates=" << report.swapchainCreationCandidates
        << " present_call_candidates=" << report.presentCallCandidates
        << " frame_capture_candidates=" << report.frameCaptureCandidates
        << " oracle_trace_candidates=" << report.oracleTraceCandidates
        << " renderer_backend_commands=" << report.rendererBackendCommands
        << " draw_submissions=" << report.drawSubmissions
        << " resolved_presentation_contracts=" << report.resolvedPresentationContracts
        << " tracked_presentation_obligations=" << report.trackedPresentationObligations
        << " open_presentation_obligations=" << report.openPresentationObligations << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"presentation_summary\": {";
    out << "\"records\": " << report.presentationRecords
        << ", \"ready_records\": " << report.readyPresentationRecords
        << ", \"tracked_open_records\": " << report.trackedOpenPresentationRecords
        << ", \"backbuffer_width\": " << report.backbufferWidth
        << ", \"backbuffer_height\": " << report.backbufferHeight
        << ", \"renderer_backend_commands\": " << report.rendererBackendCommands
        << ", \"draw_submissions\": " << report.drawSubmissions << "},\n";
    out << "  \"device_execution_link\": {";
    out << "\"device_execution_frame_inputs\": " << report.deviceExecutionFrameInputs
        << ", \"linked_device_execution_records\": " << report.linkedDeviceExecutionRecords
        << ", \"linked_device_ready_records\": " << report.linkedDeviceReadyRecords
        << ", \"linked_device_open_records\": " << report.linkedDeviceOpenRecords << "},\n";
    out << "  \"oracle_gates\": {";
    out << "\"window_surface_candidates\": " << report.windowSurfaceCandidates
        << ", \"swapchain_creation_candidates\": " << report.swapchainCreationCandidates
        << ", \"present_call_candidates\": " << report.presentCallCandidates
        << ", \"frame_capture_candidates\": " << report.frameCaptureCandidates
        << ", \"oracle_trace_candidates\": " << report.oracleTraceCandidates << "},\n";
    out << "  \"contract_summary\": {";
    out << "\"resolved_presentation_contracts\": " << report.resolvedPresentationContracts
        << ", \"tracked_presentation_obligations\": " << report.trackedPresentationObligations
        << ", \"open_presentation_obligations\": " << report.openPresentationObligations << "},\n";
    out << "  \"contracts\": [\n";
    for (size_t i = 0; i < report.contracts.size(); ++i) {
        const auto& contract = report.contracts[i];
        out << "    {\"contract\": \"" << core::jsonEscape(contract.obligation)
            << "\", \"status\": \"" << core::jsonEscape(contract.status)
            << "\", \"evidence\": \"" << core::jsonEscape(contract.evidence) << "\"}";
        out << (i + 1 == report.contracts.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"records\": [\n";
    for (size_t i = 0; i < report.records.size(); ++i) {
        const auto& record = report.records[i];
        out << "    {\"name\": \"" << core::jsonEscape(record.name)
            << "\", \"operation\": \"" << core::jsonEscape(record.operation)
            << "\", \"source\": \"" << core::jsonEscape(record.source)
            << "\", \"status\": \"" << core::jsonEscape(record.status)
            << "\", \"width\": " << record.width
            << ", \"height\": " << record.height
            << ", \"linked_record_count\": " << record.linkedRecordCount
            << ", \"ready\": " << (record.ready ? "true" : "false") << "}";
        out << (i + 1 == report.records.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

BackendPlatformBridgeRuntimeReport runBackendPlatformBridgeRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    BackendPlatformBridgeRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report = buildBackendPlatformBridgeRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer,
            vfs);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string backendPlatformBridgeRuntimeReportToJson(
    const BackendPlatformBridgeRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.backend_platform_bridge_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " device_execution_ok=" << (report.deviceExecutionOk ? "true" : "false")
        << " presentation_oracle_ok=" << (report.presentationOracleOk ? "true" : "false")
        << " platform_bridge_runtime_ready=" << (report.platformBridgeRuntimeReady ? "true" : "false")
        << " diagnostic_bridge_ready=" << (report.diagnosticBridgeReady ? "true" : "false")
        << " creation_submission_queue_ready=" << (report.creationSubmissionQueueReady ? "true" : "false")
        << " upload_submission_queue_ready=" << (report.uploadSubmissionQueueReady ? "true" : "false")
        << " state_submission_queue_ready=" << (report.stateSubmissionQueueReady ? "true" : "false")
        << " presentation_submission_queue_tracked="
        << (report.presentationSubmissionQueueTracked ? "true" : "false")
        << " d3d_concrete_backend_gate_tracked="
        << (report.d3dConcreteBackendGateTracked ? "true" : "false")
        << " frame_capture_gate_tracked=" << (report.frameCaptureGateTracked ? "true" : "false")
        << " original_oracle_gate_tracked=" << (report.originalOracleGateTracked ? "true" : "false")
        << " bridge_call_records=" << report.bridgeCallRecords
        << " ready_bridge_call_records=" << report.readyBridgeCallRecords
        << " tracked_open_bridge_call_records=" << report.trackedOpenBridgeCallRecords
        << " diagnostic_call_batches=" << report.diagnosticCallBatches
        << " platform_surface_records=" << report.platformSurfaceRecords
        << " d3d_interface_records=" << report.d3dInterfaceRecords
        << " create_device_records=" << report.createDeviceRecords
        << " resource_creation_call_records=" << report.resourceCreationCallRecords
        << " texture_create_calls=" << report.textureCreateCalls
        << " cube_texture_create_calls=" << report.cubeTextureCreateCalls
        << " render_target_create_candidates=" << report.renderTargetCreateCandidates
        << " depth_stencil_create_candidates=" << report.depthStencilCreateCandidates
        << " font_atlas_create_candidates=" << report.fontAtlasCreateCandidates
        << " upload_call_records=" << report.uploadCallRecords
        << " upload_subresource_calls=" << report.uploadSubresourceCalls
        << " state_binding_call_records=" << report.stateBindingCallRecords
        << " ready_state_binding_calls=" << report.readyStateBindingCalls
        << " tracked_open_state_binding_calls=" << report.trackedOpenStateBindingCalls
        << " set_texture_calls=" << report.setTextureCalls
        << " set_sampler_state_calls=" << report.setSamplerStateCalls
        << " set_render_state_bundle_calls=" << report.setRenderStateBundleCalls
        << " draw_submission_calls=" << report.drawSubmissionCalls
        << " present_call_records=" << report.presentCallRecords
        << " capture_oracle_call_records=" << report.captureOracleCallRecords
        << " linked_device_execution_records=" << report.linkedDeviceExecutionRecords
        << " linked_presentation_records=" << report.linkedPresentationRecords
        << " ready_platform_input_records=" << report.readyPlatformInputRecords
        << " tracked_open_platform_input_records=" << report.trackedOpenPlatformInputRecords
        << " renderer_backend_commands=" << report.rendererBackendCommands
        << " draw_submissions=" << report.drawSubmissions
        << " backbuffer_width=" << report.backbufferWidth
        << " backbuffer_height=" << report.backbufferHeight
        << " resolved_platform_bridge_contracts=" << report.resolvedPlatformBridgeContracts
        << " tracked_platform_bridge_obligations=" << report.trackedPlatformBridgeObligations
        << " open_platform_bridge_obligations=" << report.openPlatformBridgeObligations << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"bridge_summary\": {";
    out << "\"records\": " << report.bridgeCallRecords
        << ", \"ready_records\": " << report.readyBridgeCallRecords
        << ", \"tracked_open_records\": " << report.trackedOpenBridgeCallRecords
        << ", \"diagnostic_call_batches\": " << report.diagnosticCallBatches
        << ", \"renderer_backend_commands\": " << report.rendererBackendCommands
        << ", \"draw_submissions\": " << report.drawSubmissions
        << ", \"backbuffer_width\": " << report.backbufferWidth
        << ", \"backbuffer_height\": " << report.backbufferHeight << "},\n";
    out << "  \"input_link\": {";
    out << "\"linked_device_execution_records\": " << report.linkedDeviceExecutionRecords
        << ", \"linked_presentation_records\": " << report.linkedPresentationRecords
        << ", \"ready_platform_input_records\": " << report.readyPlatformInputRecords
        << ", \"tracked_open_platform_input_records\": " << report.trackedOpenPlatformInputRecords
        << "},\n";
    out << "  \"platform_calls\": {";
    out << "\"platform_surface_records\": " << report.platformSurfaceRecords
        << ", \"d3d_interface_records\": " << report.d3dInterfaceRecords
        << ", \"create_device_records\": " << report.createDeviceRecords
        << ", \"resource_creation_call_records\": " << report.resourceCreationCallRecords
        << ", \"texture_create_calls\": " << report.textureCreateCalls
        << ", \"cube_texture_create_calls\": " << report.cubeTextureCreateCalls
        << ", \"render_target_create_candidates\": " << report.renderTargetCreateCandidates
        << ", \"depth_stencil_create_candidates\": " << report.depthStencilCreateCandidates
        << ", \"font_atlas_create_candidates\": " << report.fontAtlasCreateCandidates
        << ", \"upload_call_records\": " << report.uploadCallRecords
        << ", \"upload_subresource_calls\": " << report.uploadSubresourceCalls
        << ", \"state_binding_call_records\": " << report.stateBindingCallRecords
        << ", \"set_texture_calls\": " << report.setTextureCalls
        << ", \"set_sampler_state_calls\": " << report.setSamplerStateCalls
        << ", \"set_render_state_bundle_calls\": " << report.setRenderStateBundleCalls
        << ", \"draw_submission_calls\": " << report.drawSubmissionCalls
        << ", \"present_call_records\": " << report.presentCallRecords
        << ", \"capture_oracle_call_records\": " << report.captureOracleCallRecords << "},\n";
    out << "  \"contract_summary\": {";
    out << "\"resolved_platform_bridge_contracts\": " << report.resolvedPlatformBridgeContracts
        << ", \"tracked_platform_bridge_obligations\": " << report.trackedPlatformBridgeObligations
        << ", \"open_platform_bridge_obligations\": " << report.openPlatformBridgeObligations << "},\n";
    out << "  \"contracts\": [\n";
    for (size_t i = 0; i < report.contracts.size(); ++i) {
        const auto& contract = report.contracts[i];
        out << "    {\"contract\": \"" << core::jsonEscape(contract.obligation)
            << "\", \"status\": \"" << core::jsonEscape(contract.status)
            << "\", \"evidence\": \"" << core::jsonEscape(contract.evidence) << "\"}";
        out << (i + 1 == report.contracts.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"records\": [\n";
    for (size_t i = 0; i < report.records.size(); ++i) {
        const auto& record = report.records[i];
        out << "    {\"name\": \"" << core::jsonEscape(record.name)
            << "\", \"api\": \"" << core::jsonEscape(record.api)
            << "\", \"source\": \"" << core::jsonEscape(record.source)
            << "\", \"status\": \"" << core::jsonEscape(record.status)
            << "\", \"input_records\": " << record.inputRecords
            << ", \"ready_inputs\": " << record.readyInputs
            << ", \"tracked_open_inputs\": " << record.trackedOpenInputs
            << ", \"call_count\": " << record.callCount
            << ", \"width\": " << record.width
            << ", \"height\": " << record.height
            << ", \"ready\": " << (record.ready ? "true" : "false") << "}";
        out << (i + 1 == report.records.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

BackendExecutorRuntimeReport runBackendExecutorRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    BackendExecutorRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report = buildBackendExecutorRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer,
            vfs);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string backendExecutorRuntimeReportToJson(
    const BackendExecutorRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.backend_executor_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " platform_bridge_ok=" << (report.platformBridgeOk ? "true" : "false")
        << " executor_runtime_ready=" << (report.executorRuntimeReady ? "true" : "false")
        << " diagnostic_executor_ready=" << (report.diagnosticExecutorReady ? "true" : "false")
        << " one_to_one_bridge_mapping_ready=" << (report.oneToOneBridgeMappingReady ? "true" : "false")
        << " ready_call_execution_results_ready="
        << (report.readyCallExecutionResultsReady ? "true" : "false")
        << " tracked_open_call_results_ready="
        << (report.trackedOpenCallResultsReady ? "true" : "false")
        << " concrete_d3d_execution_gate_tracked="
        << (report.concreteD3DExecutionGateTracked ? "true" : "false")
        << " hwnd_device_gate_tracked=" << (report.hwndDeviceGateTracked ? "true" : "false")
        << " draw_execution_gate_tracked=" << (report.drawExecutionGateTracked ? "true" : "false")
        << " present_execution_gate_tracked=" << (report.presentExecutionGateTracked ? "true" : "false")
        << " frame_capture_gate_tracked=" << (report.frameCaptureGateTracked ? "true" : "false")
        << " original_oracle_gate_tracked=" << (report.originalOracleGateTracked ? "true" : "false")
        << " execution_result_records=" << report.executionResultRecords
        << " diagnostic_success_records=" << report.diagnosticSuccessRecords
        << " tracked_open_execution_records=" << report.trackedOpenExecutionRecords
        << " blocked_execution_records=" << report.blockedExecutionRecords
        << " consumed_bridge_call_records=" << report.consumedBridgeCallRecords
        << " ready_bridge_call_records=" << report.readyBridgeCallRecords
        << " tracked_open_bridge_call_records=" << report.trackedOpenBridgeCallRecords
        << " result_call_count_total=" << report.resultCallCountTotal
        << " diagnostic_executed_calls=" << report.diagnosticExecutedCalls
        << " preserved_open_calls=" << report.preservedOpenCalls
        << " submitted_diagnostic_batches=" << report.submittedDiagnosticBatches
        << " executed_resource_creation_calls=" << report.executedResourceCreationCalls
        << " executed_upload_subresource_calls=" << report.executedUploadSubresourceCalls
        << " executed_state_binding_calls=" << report.executedStateBindingCalls
        << " preserved_platform_surface_gates=" << report.preservedPlatformSurfaceGates
        << " preserved_device_creation_gates=" << report.preservedDeviceCreationGates
        << " preserved_draw_submission_gates=" << report.preservedDrawSubmissionGates
        << " preserved_present_gates=" << report.preservedPresentGates
        << " preserved_capture_oracle_gates=" << report.preservedCaptureOracleGates
        << " linked_platform_input_records=" << report.linkedPlatformInputRecords
        << " ready_platform_input_records=" << report.readyPlatformInputRecords
        << " tracked_open_platform_input_records=" << report.trackedOpenPlatformInputRecords
        << " backbuffer_width=" << report.backbufferWidth
        << " backbuffer_height=" << report.backbufferHeight
        << " resolved_executor_contracts=" << report.resolvedExecutorContracts
        << " tracked_executor_obligations=" << report.trackedExecutorObligations
        << " open_executor_obligations=" << report.openExecutorObligations << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"executor_summary\": {";
    out << "\"execution_result_records\": " << report.executionResultRecords
        << ", \"diagnostic_success_records\": " << report.diagnosticSuccessRecords
        << ", \"tracked_open_execution_records\": " << report.trackedOpenExecutionRecords
        << ", \"blocked_execution_records\": " << report.blockedExecutionRecords
        << ", \"result_call_count_total\": " << report.resultCallCountTotal
        << ", \"diagnostic_executed_calls\": " << report.diagnosticExecutedCalls
        << ", \"preserved_open_calls\": " << report.preservedOpenCalls << "},\n";
    out << "  \"bridge_link\": {";
    out << "\"consumed_bridge_call_records\": " << report.consumedBridgeCallRecords
        << ", \"ready_bridge_call_records\": " << report.readyBridgeCallRecords
        << ", \"tracked_open_bridge_call_records\": " << report.trackedOpenBridgeCallRecords
        << ", \"linked_platform_input_records\": " << report.linkedPlatformInputRecords
        << ", \"ready_platform_input_records\": " << report.readyPlatformInputRecords
        << ", \"tracked_open_platform_input_records\": " << report.trackedOpenPlatformInputRecords
        << "},\n";
    out << "  \"diagnostic_execution\": {";
    out << "\"submitted_diagnostic_batches\": " << report.submittedDiagnosticBatches
        << ", \"executed_resource_creation_calls\": " << report.executedResourceCreationCalls
        << ", \"executed_upload_subresource_calls\": " << report.executedUploadSubresourceCalls
        << ", \"executed_state_binding_calls\": " << report.executedStateBindingCalls
        << ", \"backbuffer_width\": " << report.backbufferWidth
        << ", \"backbuffer_height\": " << report.backbufferHeight << "},\n";
    out << "  \"preserved_gates\": {";
    out << "\"preserved_platform_surface_gates\": " << report.preservedPlatformSurfaceGates
        << ", \"preserved_device_creation_gates\": " << report.preservedDeviceCreationGates
        << ", \"preserved_draw_submission_gates\": " << report.preservedDrawSubmissionGates
        << ", \"preserved_present_gates\": " << report.preservedPresentGates
        << ", \"preserved_capture_oracle_gates\": " << report.preservedCaptureOracleGates << "},\n";
    out << "  \"contract_summary\": {";
    out << "\"resolved_executor_contracts\": " << report.resolvedExecutorContracts
        << ", \"tracked_executor_obligations\": " << report.trackedExecutorObligations
        << ", \"open_executor_obligations\": " << report.openExecutorObligations << "},\n";
    out << "  \"contracts\": [\n";
    for (size_t i = 0; i < report.contracts.size(); ++i) {
        const auto& contract = report.contracts[i];
        out << "    {\"contract\": \"" << core::jsonEscape(contract.obligation)
            << "\", \"status\": \"" << core::jsonEscape(contract.status)
            << "\", \"evidence\": \"" << core::jsonEscape(contract.evidence) << "\"}";
        out << (i + 1 == report.contracts.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"results\": [\n";
    for (size_t i = 0; i < report.results.size(); ++i) {
        const auto& result = report.results[i];
        out << "    {\"name\": \"" << core::jsonEscape(result.name)
            << "\", \"source_bridge_record\": \"" << core::jsonEscape(result.sourceBridgeRecord)
            << "\", \"api\": \"" << core::jsonEscape(result.api)
            << "\", \"adapter\": \"" << core::jsonEscape(result.adapter)
            << "\", \"result_status\": \"" << core::jsonEscape(result.resultStatus)
            << "\", \"obligation\": \"" << core::jsonEscape(result.obligation)
            << "\", \"input_records\": " << result.inputRecords
            << ", \"call_count\": " << result.callCount
            << ", \"executed_calls\": " << result.executedCalls
            << ", \"preserved_open_calls\": " << result.preservedOpenCalls
            << ", \"ready\": " << (result.ready ? "true" : "false") << "}";
        out << (i + 1 == report.results.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

BackendDeviceAdapterRuntimeReport runBackendDeviceAdapterRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    BackendDeviceAdapterRuntimeReport report;
    try {
        const auto manifest = project::loadProjectManifest(manifestPath);
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);
        report = buildBackendDeviceAdapterRuntimeReport(
            collectGameplayFrameInputs(manifestPath, repoRoot),
            manifest.renderer,
            vfs);
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    return report;
}

std::string backendDeviceAdapterRuntimeReportToJson(
    const BackendDeviceAdapterRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.backend_device_adapter_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " backend_executor_ok=" << (report.backendExecutorOk ? "true" : "false")
        << " device_adapter_runtime_ready=" << (report.deviceAdapterRuntimeReady ? "true" : "false")
        << " executor_results_consumed_ready="
        << (report.executorResultsConsumedReady ? "true" : "false")
        << " platform_device_preconditions_tracked="
        << (report.platformDevicePreconditionsTracked ? "true" : "false")
        << " downstream_execution_blocked_until_device="
        << (report.downstreamExecutionBlockedUntilDevice ? "true" : "false")
        << " backbuffer_extent_carried=" << (report.backbufferExtentCarried ? "true" : "false")
        << " real_window_surface_gate_tracked="
        << (report.realWindowSurfaceGateTracked ? "true" : "false")
        << " real_d3d_interface_gate_tracked="
        << (report.realD3DInterfaceGateTracked ? "true" : "false")
        << " real_d3d_device_gate_tracked="
        << (report.realD3DDeviceGateTracked ? "true" : "false")
        << " real_device_handle_ready=" << (report.realDeviceHandleReady ? "true" : "false")
        << " resource_execution_requires_device="
        << (report.resourceExecutionRequiresDevice ? "true" : "false")
        << " draw_present_capture_requires_device="
        << (report.drawPresentCaptureRequiresDevice ? "true" : "false")
        << " adapter_record_count=" << report.adapterRecordCount
        << " consumed_executor_result_records=" << report.consumedExecutorResultRecords
        << " source_diagnostic_success_records=" << report.sourceDiagnosticSuccessRecords
        << " source_tracked_open_records=" << report.sourceTrackedOpenRecords
        << " diagnostic_context_records=" << report.diagnosticContextRecords
        << " platform_device_adapter_records=" << report.platformDeviceAdapterRecords
        << " window_surface_adapter_records=" << report.windowSurfaceAdapterRecords
        << " d3d_interface_adapter_records=" << report.d3dInterfaceAdapterRecords
        << " create_device_adapter_records=" << report.createDeviceAdapterRecords
        << " downstream_blocked_records=" << report.downstreamBlockedRecords
        << " resource_queue_blocked_records=" << report.resourceQueueBlockedRecords
        << " render_queue_blocked_records=" << report.renderQueueBlockedRecords
        << " platform_device_precondition_calls=" << report.platformDevicePreconditionCalls
        << " downstream_real_calls_blocked_until_device="
        << report.downstreamRealCallsBlockedUntilDevice
        << " blocked_real_calls_total=" << report.blockedRealCallsTotal
        << " real_executed_calls=" << report.realExecutedCalls
        << " real_resource_creation_calls_blocked=" << report.realResourceCreationCallsBlocked
        << " real_upload_subresource_calls_blocked=" << report.realUploadSubresourceCallsBlocked
        << " real_state_binding_calls_blocked=" << report.realStateBindingCallsBlocked
        << " real_draw_calls_blocked=" << report.realDrawCallsBlocked
        << " real_present_calls_blocked=" << report.realPresentCallsBlocked
        << " real_capture_oracle_calls_blocked=" << report.realCaptureOracleCallsBlocked
        << " inherited_diagnostic_executed_calls=" << report.inheritedDiagnosticExecutedCalls
        << " inherited_preserved_open_calls=" << report.inheritedPreservedOpenCalls
        << " linked_platform_input_records=" << report.linkedPlatformInputRecords
        << " ready_platform_input_records=" << report.readyPlatformInputRecords
        << " tracked_open_platform_input_records=" << report.trackedOpenPlatformInputRecords
        << " backbuffer_width=" << report.backbufferWidth
        << " backbuffer_height=" << report.backbufferHeight
        << " resolved_device_adapter_contracts=" << report.resolvedDeviceAdapterContracts
        << " tracked_device_adapter_obligations=" << report.trackedDeviceAdapterObligations
        << " open_device_adapter_obligations=" << report.openDeviceAdapterObligations << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"adapter_summary\": {";
    out << "\"adapter_record_count\": " << report.adapterRecordCount
        << ", \"consumed_executor_result_records\": " << report.consumedExecutorResultRecords
        << ", \"source_diagnostic_success_records\": " << report.sourceDiagnosticSuccessRecords
        << ", \"source_tracked_open_records\": " << report.sourceTrackedOpenRecords
        << ", \"diagnostic_context_records\": " << report.diagnosticContextRecords
        << ", \"platform_device_adapter_records\": " << report.platformDeviceAdapterRecords
        << ", \"downstream_blocked_records\": " << report.downstreamBlockedRecords << "},\n";
    out << "  \"device_preconditions\": {";
    out << "\"window_surface_adapter_records\": " << report.windowSurfaceAdapterRecords
        << ", \"d3d_interface_adapter_records\": " << report.d3dInterfaceAdapterRecords
        << ", \"create_device_adapter_records\": " << report.createDeviceAdapterRecords
        << ", \"platform_device_precondition_calls\": " << report.platformDevicePreconditionCalls
        << ", \"real_device_handle_ready\": "
        << (report.realDeviceHandleReady ? "true" : "false")
        << ", \"backbuffer_width\": " << report.backbufferWidth
        << ", \"backbuffer_height\": " << report.backbufferHeight << "},\n";
    out << "  \"blocked_execution\": {";
    out << "\"resource_queue_blocked_records\": " << report.resourceQueueBlockedRecords
        << ", \"render_queue_blocked_records\": " << report.renderQueueBlockedRecords
        << ", \"downstream_real_calls_blocked_until_device\": "
        << report.downstreamRealCallsBlockedUntilDevice
        << ", \"blocked_real_calls_total\": " << report.blockedRealCallsTotal
        << ", \"real_executed_calls\": " << report.realExecutedCalls
        << ", \"real_resource_creation_calls_blocked\": "
        << report.realResourceCreationCallsBlocked
        << ", \"real_upload_subresource_calls_blocked\": "
        << report.realUploadSubresourceCallsBlocked
        << ", \"real_state_binding_calls_blocked\": " << report.realStateBindingCallsBlocked
        << ", \"real_draw_calls_blocked\": " << report.realDrawCallsBlocked
        << ", \"real_present_calls_blocked\": " << report.realPresentCallsBlocked
        << ", \"real_capture_oracle_calls_blocked\": "
        << report.realCaptureOracleCallsBlocked << "},\n";
    out << "  \"executor_link\": {";
    out << "\"inherited_diagnostic_executed_calls\": " << report.inheritedDiagnosticExecutedCalls
        << ", \"inherited_preserved_open_calls\": " << report.inheritedPreservedOpenCalls
        << ", \"linked_platform_input_records\": " << report.linkedPlatformInputRecords
        << ", \"ready_platform_input_records\": " << report.readyPlatformInputRecords
        << ", \"tracked_open_platform_input_records\": "
        << report.trackedOpenPlatformInputRecords << "},\n";
    out << "  \"contract_summary\": {";
    out << "\"resolved_device_adapter_contracts\": " << report.resolvedDeviceAdapterContracts
        << ", \"tracked_device_adapter_obligations\": "
        << report.trackedDeviceAdapterObligations
        << ", \"open_device_adapter_obligations\": "
        << report.openDeviceAdapterObligations << "},\n";
    out << "  \"contracts\": [\n";
    for (size_t i = 0; i < report.contracts.size(); ++i) {
        const auto& contract = report.contracts[i];
        out << "    {\"contract\": \"" << core::jsonEscape(contract.obligation)
            << "\", \"status\": \"" << core::jsonEscape(contract.status)
            << "\", \"evidence\": \"" << core::jsonEscape(contract.evidence) << "\"}";
        out << (i + 1 == report.contracts.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"records\": [\n";
    for (size_t i = 0; i < report.records.size(); ++i) {
        const auto& record = report.records[i];
        out << "    {\"name\": \"" << core::jsonEscape(record.name)
            << "\", \"source_executor_result\": \"" << core::jsonEscape(record.sourceExecutorResult)
            << "\", \"source_bridge_record\": \"" << core::jsonEscape(record.sourceBridgeRecord)
            << "\", \"api\": \"" << core::jsonEscape(record.api)
            << "\", \"adapter\": \"" << core::jsonEscape(record.adapter)
            << "\", \"stage\": \"" << core::jsonEscape(record.stage)
            << "\", \"status\": \"" << core::jsonEscape(record.status)
            << "\", \"obligation\": \"" << core::jsonEscape(record.obligation)
            << "\", \"input_records\": " << record.inputRecords
            << ", \"call_count\": " << record.callCount
            << ", \"width\": " << record.width
            << ", \"height\": " << record.height
            << ", \"inherited_executed_calls\": " << record.inheritedExecutedCalls
            << ", \"inherited_preserved_open_calls\": " << record.inheritedPreservedOpenCalls
            << ", \"real_executed_calls\": " << record.realExecutedCalls
            << ", \"blocked_real_calls\": " << record.blockedRealCalls
            << ", \"source_ready\": " << (record.sourceReady ? "true" : "false")
            << ", \"device_handle_required\": "
            << (record.deviceHandleRequired ? "true" : "false")
            << ", \"device_handle_ready\": "
            << (record.deviceHandleReady ? "true" : "false") << "}";
        out << (i + 1 == report.records.size() ? "\n" : ",\n");
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
