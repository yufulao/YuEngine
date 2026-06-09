#include "yuengine/runtime/EngineRuntime.h"

#include "yuengine/core/Json.h"
#include "yuengine/native/NativeRegistry.h"
#include "yuengine/native/NativeServices.h"
#include "yuengine/project/ProjectManifest.h"
#include "yuengine/resource/VirtualFileSystem.h"
#include "yuengine/script/ScriptRuntime.h"
#include "yuengine/script/SqasmModule.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>

namespace yu::runtime {
namespace {

void addError(BootReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

void addError(SceneEntryRuntimeReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

std::string pathCacheKey(const std::filesystem::path& path)
{
    try {
        return std::filesystem::weakly_canonical(path).generic_string();
    } catch (const std::exception&) {
        return path.lexically_normal().generic_string();
    }
}

std::string runtimeCacheKey(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    return pathCacheKey(manifestPath) + "|" + pathCacheKey(repoRoot);
}

bool traceEnabled()
{
#if defined(_MSC_VER)
    char* value = nullptr;
    size_t size = 0;
    _dupenv_s(&value, &size, "YUENGINE_TRACE_BOOT");
    const bool enabled = value != nullptr && value[0] != '\0';
    std::free(value);
    return enabled;
#else
    const char* value = std::getenv("YUENGINE_TRACE_BOOT");
    return value != nullptr && value[0] != '\0';
#endif
}

void traceBoot(const std::string& phase)
{
    if (traceEnabled()) {
        std::cerr << "[yuengine boot] " << phase << std::endl;
    }
}

std::string lowercase(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::vector<std::filesystem::path> manifestScriptRoots(const project::ProjectManifest& manifest)
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
    const auto roots = manifestScriptRoots(manifest);
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

int64_t fileSizeOrUnknown(const std::filesystem::path& path)
{
    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);
    return error ? -1 : static_cast<int64_t>(size);
}

std::string withExtension(const std::string& path, const std::string& extension)
{
    std::filesystem::path value(path);
    value.replace_extension(extension);
    return value.generic_string();
}

std::string eventScriptModuleName(const std::string& eventScript)
{
    if (eventScript.empty()) {
        return {};
    }
    if (eventScript.rfind("mission/", 0) == 0) {
        return eventScript.ends_with(".b64") || eventScript.ends_with(".b64.sqasm") ? eventScript
                                                                                    : eventScript + ".b64";
    }
    return "mission/" + eventScript + ".b64";
}

bool foundBinding(const SceneEntryRuntimeReport& report, const std::string& role)
{
    return std::any_of(report.bindings.begin(), report.bindings.end(), [&](const auto& binding) {
        return binding.role == role && binding.found;
    });
}

void addResourceBinding(
    SceneEntryRuntimeReport& report,
    const resource::VirtualFileSystem& vfs,
    std::string role,
    std::string query,
    bool required)
{
    RuntimeBindingReport binding;
    binding.role = std::move(role);
    binding.kind = "resource_path";
    binding.query = resource::normalizeResourcePath(query);
    binding.required = required;

    if (!query.empty()) {
        const auto resolution = vfs.resolvePath(query);
        binding.found = resolution.found;
        if (resolution.found) {
            binding.mountType = resolution.entry.mountType;
            binding.virtualPath = resolution.entry.virtualPath;
            binding.physicalPath = resolution.entry.physicalPath.string();
            binding.pack = resolution.entry.pack;
            binding.size = resolution.entry.size;
        }
    }

    ++report.resourceBindings;
    if (required && !binding.found) {
        ++report.missingResources;
        addError(report, "required runtime resource not resolved: " + binding.role + "=" + binding.query);
    }
    report.bindings.push_back(std::move(binding));
}

void addScriptBinding(
    SceneEntryRuntimeReport& report,
    const project::ProjectManifest& manifest,
    std::string role,
    std::string moduleName,
    bool required)
{
    RuntimeBindingReport binding;
    binding.role = std::move(role);
    binding.kind = "script_module";
    binding.query = std::move(moduleName);
    binding.required = required;

    if (!binding.query.empty()) {
        const auto resolved = script::resolveScriptModule(manifestScriptRoots(manifest), binding.query);
        binding.found = !resolved.empty();
        if (binding.found) {
            binding.physicalPath = resolved.string();
            binding.size = fileSizeOrUnknown(resolved);
        }
    }

    ++report.scriptBindings;
    if (required && !binding.found) {
        ++report.missingScripts;
        addError(report, "required runtime script module not resolved: " + binding.role + "=" + binding.query);
    }
    report.bindings.push_back(std::move(binding));
}

void writeJsonStringArray(std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        out << (i ? ", " : "") << "\"" << core::jsonEscape(values[i]) << "\"";
    }
    out << "]";
}

void writeBinding(std::ostringstream& out, const RuntimeBindingReport& binding)
{
    out << "{\"role\": \"" << core::jsonEscape(binding.role) << "\", "
        << "\"kind\": \"" << core::jsonEscape(binding.kind) << "\", "
        << "\"query\": \"" << core::jsonEscape(binding.query) << "\", "
        << "\"required\": " << (binding.required ? "true" : "false") << ", "
        << "\"found\": " << (binding.found ? "true" : "false");
    if (!binding.mountType.empty()) {
        out << ", \"mount_type\": \"" << core::jsonEscape(binding.mountType) << "\"";
    }
    if (!binding.virtualPath.empty()) {
        out << ", \"virtual_path\": \"" << core::jsonEscape(binding.virtualPath) << "\"";
    }
    if (!binding.physicalPath.empty()) {
        out << ", \"physical_path\": \"" << core::jsonEscape(binding.physicalPath) << "\"";
    }
    if (!binding.pack.empty()) {
        out << ", \"pack\": \"" << core::jsonEscape(binding.pack) << "\"";
    }
    if (binding.size >= 0) {
        out << ", \"size\": " << binding.size;
    }
    out << "}";
}

void loadModule(
    BootReport& report,
    const std::vector<std::filesystem::path>& roots,
    const std::string& moduleName,
    const native::NativeRegistry& registry,
    const native::NativeServiceCatalog& serviceCatalog,
    std::map<std::string, ObligationReport>& obligations)
{
    std::filesystem::path path = script::resolveScriptModule(roots, moduleName);
    if (path.empty()) {
        addError(report, "script module not found: " + moduleName);
        return;
    }
    script::SqasmModule module = script::loadSqasmModule(path);
    report.modules.push_back({
        path.string(),
        static_cast<int>(module.functions.size()),
        module.instructionCount,
        module.callCount,
        static_cast<int>(module.resourceRefs.size()),
    });
    for (const auto& function : module.functions) {
        for (const auto& call : function.calls) {
            const native::ApiSurface* surface = registry.find(call.name);
            if (!surface) {
                continue;
            }
            const auto dispatch = serviceCatalog.dispatch(*surface, {
                module.path.string(),
                function.name,
                call.sourceLine,
                call.pc,
            });
            auto& obligation = obligations[call.name];
            obligation.api = call.name;
            obligation.service = dispatch.service;
            obligation.status = dispatch.implementationStatus;
            ++obligation.calls;
        }
    }
}

size_t failedPhaseCount(const BootReport& report)
{
    size_t count = 0;
    for (const auto& phase : report.phases) {
        if (phase.status != "ok") {
            ++count;
        }
    }
    return count;
}

} // namespace

BootReport bootProject(const std::filesystem::path& manifestPath, const std::filesystem::path& repoRoot)
{
    BootReport report;
    RuntimeContext context;
    try {
        traceBoot("enter boot project");

        traceBoot("load manifest");
        context.services().setManifest(project::loadProjectManifest(manifestPath));
        const auto& manifest = context.services().manifest();
        report.projectId = manifest.projectId;
        context.recordPhase("load_project_manifest", "ok", manifest.projectId);

        traceBoot("mount vfs");
        context.services().vfs().mountProject(manifest);
        report.looseMounts = context.services().vfs().looseMountCount();
        report.packResources = context.services().vfs().packResourceCount();
        context.recordPhase(
            "mount_vfs",
            "ok",
            "loose=" + std::to_string(report.looseMounts) + " pack=" + std::to_string(report.packResources));

        traceBoot("verify required resources");
        int missingResources = 0;
        for (const auto& required : manifest.requiredResources) {
            if (required.kind == "path" && !context.services().vfs().resolvePath(required.path).found) {
                addError(report, "required resource not resolved: " + required.path);
                ++missingResources;
            }
            if (required.kind == "stem" && context.services().vfs().resolveStem(required.path).empty()) {
                addError(report, "required resource stem not resolved: " + required.path);
                ++missingResources;
            }
        }
        context.recordPhase(
            "verify_required_resources",
            missingResources == 0 ? "ok" : "failed",
            "required=" + std::to_string(manifest.requiredResources.size()) + " missing="
                + std::to_string(missingResources));

        traceBoot("load native registry");
        std::filesystem::path surfacePath = repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md";
        if (std::filesystem::exists(surfacePath)) {
            context.services().nativeRegistry().loadMarkdownSurface(surfacePath);
        } else {
            report.warnings.push_back("native surface markdown not found: " + surfacePath.string());
        }
        report.nativeApis = context.services().nativeRegistry().size();
        context.recordPhase("load_native_registry", "ok", "apis=" + std::to_string(report.nativeApis));

        traceBoot("bind native services");
        report.nativeServices = context.services().nativeServices().size();
        const auto unboundApis =
            context.services().nativeServices().unboundApis(context.services().nativeRegistry());
        report.unboundNativeApis = unboundApis.size();
        if (!unboundApis.empty()) {
            addError(report, "native APIs have no service interface: " + std::to_string(unboundApis.size()));
        }
        context.recordPhase(
            "bind_native_services",
            unboundApis.empty() ? "ok" : "failed",
            "services=" + std::to_string(report.nativeServices) + " unbound=" + std::to_string(unboundApis.size()));

        traceBoot("load script modules");
        std::map<std::string, ObligationReport> obligations;
        const auto errorsBeforeScripts = report.errors.size();
        auto roots = context.services().scriptRoots();
        for (const auto& preload : manifest.startup.preloadScripts) {
            loadModule(
                report,
                roots,
                preload,
                context.services().nativeRegistry(),
                context.services().nativeServices(),
                obligations);
        }
        for (const auto& dependency : manifest.startup.dependencyScripts) {
            loadModule(
                report,
                roots,
                dependency,
                context.services().nativeRegistry(),
                context.services().nativeServices(),
                obligations);
        }
        loadModule(
            report,
            roots,
            manifest.startup.entryModule,
            context.services().nativeRegistry(),
            context.services().nativeServices(),
            obligations);
        context.recordPhase(
            "load_startup_scripts",
            report.errors.size() == errorsBeforeScripts ? "ok" : "failed",
            "modules=" + std::to_string(report.modules.size()));

        traceBoot("collect native obligations");
        for (const auto& [_, obligation] : obligations) {
            report.obligations.push_back(obligation);
        }
        std::sort(report.obligations.begin(), report.obligations.end(), [](const auto& a, const auto& b) {
            if (a.service == b.service) {
                return a.api < b.api;
            }
            return a.service < b.service;
        });
        context.recordPhase("collect_native_obligations", "ok", "apis=" + std::to_string(report.obligations.size()));
        traceBoot("boot report ready");
    } catch (const std::exception& ex) {
        addError(report, ex.what());
        context.recordPhase("boot", "failed", ex.what());
    }
    report.phases = context.phases();
    return report;
}

SceneEntryRuntimeReport buildSceneEntryRuntimeReport(
    const project::ProjectManifest& manifest,
    const native::NativeRuntimeServiceState& titleState,
    const native::NativeRuntimeServiceState& missionState,
    const resource::VirtualFileSystem& vfs)
{
    SceneEntryRuntimeReport report;
    report.projectId = manifest.projectId;

    report.makeNewGameCommands = titleState.saveProfileScenario.makeNewGameCommands;
    report.startGameCommands = titleState.saveProfileScenario.startGameCommands;
    report.queuedStageLoads = titleState.sceneStage.queuedStageLoads;
    report.startedMission = titleState.saveProfileScenario.startedMission;
    report.startNewGame = titleState.saveProfileScenario.startNewGame;
    report.queuedMissionScript = titleState.sceneStage.currentMissionScript;
    report.queuedStage = titleState.sceneStage.currentStage;
    report.queuedRailCamera = titleState.sceneStage.currentRailCamera;

    report.missionScript = missionState.sceneStage.currentMissionScript.empty()
        ? titleState.sceneStage.currentMissionScript
        : missionState.sceneStage.currentMissionScript;
    report.eventScript = missionState.sceneStage.currentEventScript;
    report.activeLoader = missionState.sceneStage.activeLoader;
    report.stagePath = missionState.sceneStage.currentStage.empty()
        ? titleState.sceneStage.currentStage
        : missionState.sceneStage.currentStage;
    report.stageModelPath = report.stagePath.empty() ? std::string() : withExtension(report.stagePath, ".mdl");
    report.stageCollisionPath = report.stagePath.empty() ? std::string() : withExtension(report.stagePath, ".col");
    report.railCameraPath = missionState.camera.railCameraPath.empty()
        ? (missionState.sceneStage.currentRailCamera.empty() ? titleState.sceneStage.currentRailCamera
                                                             : missionState.sceneStage.currentRailCamera)
        : missionState.camera.railCameraPath;

    report.playerChara = missionState.actorTask.currentPlayerChara;
    const std::string playerStem = lowercase(report.playerChara);
    if (!playerStem.empty()) {
        report.playerScriptAsset = "player/" + playerStem + ".b64";
        report.playerPcgAsset = "player/" + playerStem + "_pcg.b64";
        report.playerScriptModule = "player/" + playerStem + ".b64";
        report.playerPcgModule = "player/" + playerStem + "_pcg.b64";
    }
    report.spawnPosition = missionState.actorTask.currentPlayerPosition;
    report.spawnRotY = missionState.actorTask.currentPlayerRotY;
    report.checkpoint = missionState.eventQuestFlag.currentCheckpoint;
    report.railCameraEnabled = missionState.camera.railCameraEnabled;
    report.autoCameraAdjustEnabled = missionState.camera.autoCameraAdjustEnabled;
    report.defaultCameraStateTarget = missionState.camera.defaultCameraStateTarget;

    addScriptBinding(report, manifest, "title_queued_mission_script", report.queuedMissionScript, true);
    addScriptBinding(report, manifest, "mission_setup_script", report.missionScript, true);
    addScriptBinding(report, manifest, "event_script", eventScriptModuleName(report.eventScript), true);
    addScriptBinding(report, manifest, "player_script_module", report.playerScriptModule, true);
    addScriptBinding(report, manifest, "player_pcg_module", report.playerPcgModule, true);

    addResourceBinding(report, vfs, "stage_sge", report.stagePath, true);
    addResourceBinding(report, vfs, "stage_model", report.stageModelPath, true);
    addResourceBinding(report, vfs, "stage_collision", report.stageCollisionPath, true);
    addResourceBinding(report, vfs, "rail_camera", report.railCameraPath, true);
    addResourceBinding(report, vfs, "player_script_asset", report.playerScriptAsset, true);
    addResourceBinding(report, vfs, "player_pcg_asset", report.playerPcgAsset, true);

    report.stageReady = missionState.sceneStage.loadedStageCommands > 0 && foundBinding(report, "stage_sge")
        && foundBinding(report, "stage_model") && foundBinding(report, "stage_collision");
    report.actorReady = missionState.actorTask.pushPlayerCharaCommands > 0 && !report.playerChara.empty()
        && !report.spawnPosition.empty() && foundBinding(report, "player_script_asset")
        && foundBinding(report, "player_pcg_asset") && foundBinding(report, "player_script_module")
        && foundBinding(report, "player_pcg_module");
    report.cameraReady = missionState.camera.pushGameCameraCommands > 0 && missionState.camera.railCameraLoads > 0
        && foundBinding(report, "rail_camera") && !report.defaultCameraStateTarget.empty();
    report.eventReady = missionState.sceneStage.loadedEventScriptCommands > 0
        && missionState.sceneStage.callSetupEventsCommands > 0 && missionState.eventQuestFlag.markerQueries > 0
        && missionState.eventQuestFlag.checkpointCommands > 0 && foundBinding(report, "mission_setup_script")
        && foundBinding(report, "event_script");

    if (report.makeNewGameCommands <= 0 || report.startGameCommands <= 0 || report.queuedStageLoads <= 0) {
        addError(report, "title new-game transition did not produce MakeNewGame/StartGame/stage queue commands");
    }
    if (!report.stageReady) {
        addError(report, "scene-entry stage contract is not ready");
    }
    if (!report.actorReady) {
        addError(report, "scene-entry actor contract is not ready");
    }
    if (!report.cameraReady) {
        addError(report, "scene-entry camera contract is not ready");
    }
    if (!report.eventReady) {
        addError(report, "scene-entry event contract is not ready");
    }

    return report;
}

SceneEntryRuntimeReport runSceneEntryRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    const auto cacheKey = runtimeCacheKey(manifestPath, repoRoot);
    static std::map<std::string, SceneEntryRuntimeReport> cache;
    const auto cached = cache.find(cacheKey);
    if (cached != cache.end()) {
        return cached->second;
    }

    SceneEntryRuntimeReport report;
    try {
        traceBoot("scene-entry load manifest");
        const project::ProjectManifest manifest = project::loadProjectManifest(manifestPath);

        traceBoot("scene-entry mount vfs");
        resource::VirtualFileSystem vfs;
        vfs.mountProject(manifest);

        traceBoot("scene-entry load native registry");
        native::NativeRegistry registry;
        registry.loadMarkdownSurface(repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md");

        traceBoot("scene-entry load title script");
        native::NativeServiceCatalog titleCatalog;
        const auto titleModulePath = script::resolveScriptModule(manifestScriptRoots(manifest), manifest.startup.entryModule);
        if (titleModulePath.empty()) {
            addError(report, "title entry script module not found: " + manifest.startup.entryModule);
            return report;
        }
        const auto titleModule = script::loadSqasmModule(titleModulePath);
        const auto titleBaselineModules = loadStartupBaselineModules(manifest, manifest.startup.entryModule);

        script::ScriptRunOptions titleOptions;
        titleOptions.frames = 5;
        titleOptions.inputScenario = "title-new-game";
        titleOptions.menuSelectedIndex = 1;
        titleOptions.menuDecide = true;
        traceBoot("scene-entry execute title new-game");
        const auto titleExecution =
            script::runEntryScript(titleModule, titleBaselineModules, manifest.startup.entryFunction, registry, titleCatalog, titleOptions);
        traceBoot("scene-entry bind title state");
        const auto& titleState = titleCatalog.runtimeState();

        std::string missionModuleName = titleState.sceneStage.currentMissionScript;
        if (missionModuleName.empty()) {
            missionModuleName = manifest.oracle.firstMissionCandidate;
        }
        if (missionModuleName.empty()) {
            addError(report, "first mission script is not available from title state or oracle hints");
            return report;
        }

        traceBoot("scene-entry load mission setup script");
        native::NativeServiceCatalog missionCatalog;
        const auto missionModulePath = script::resolveScriptModule(manifestScriptRoots(manifest), missionModuleName);
        if (missionModulePath.empty()) {
            addError(report, "mission setup script module not found: " + missionModuleName);
            return report;
        }
        const auto missionModule = script::loadSqasmModule(missionModulePath);
        const auto missionBaselineModules = loadStartupBaselineModules(manifest, missionModuleName);

        script::ScriptRunOptions missionOptions;
        missionOptions.frames = 1;
        missionOptions.inputScenario = "passive";
        traceBoot("scene-entry execute mission setup");
        const auto missionExecution =
            script::runEntryScript(missionModule, missionBaselineModules, "setupProcess", registry, missionCatalog, missionOptions);
        traceBoot("scene-entry bind mission state");
        const auto& missionState = missionCatalog.runtimeState();

        traceBoot("scene-entry build report");
        report = buildSceneEntryRuntimeReport(manifest, titleState, missionState, vfs);
        report.titleNewGameExecuted = titleExecution.entryFound && titleExecution.executed;
        report.missionSetupExecuted = missionExecution.entryFound && missionExecution.executed;
        report.titleStatus = titleExecution.status;
        report.missionStatus = missionExecution.status;
        if (!report.titleNewGameExecuted) {
            addError(report, "title new-game script run did not execute");
        }
        if (!report.missionSetupExecuted) {
            addError(report, "mission setup script run did not execute");
        }
        traceBoot("scene-entry complete");
    } catch (const std::exception& ex) {
        addError(report, ex.what());
    }
    cache[cacheKey] = report;
    return report;
}

std::string bootReportToJson(const BootReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.boot_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false") << " phases=" << report.phases.size()
        << " failed_phases=" << failedPhaseCount(report) << " native_apis=" << report.nativeApis
        << " native_services=" << report.nativeServices << " obligations=" << report.obligations.size() << "\",\n";
    out << "  \"loose_mounts\": " << report.looseMounts << ",\n";
    out << "  \"pack_resources\": " << report.packResources << ",\n";
    out << "  \"native_apis\": " << report.nativeApis << ",\n";
    out << "  \"native_services\": " << report.nativeServices << ",\n";
    out << "  \"unbound_native_apis\": " << report.unboundNativeApis << ",\n";
    out << "  \"errors\": [";
    for (size_t i = 0; i < report.errors.size(); ++i) {
        out << (i ? ", " : "") << "\"" << core::jsonEscape(report.errors[i]) << "\"";
    }
    out << "],\n";
    out << "  \"warnings\": [";
    for (size_t i = 0; i < report.warnings.size(); ++i) {
        out << (i ? ", " : "") << "\"" << core::jsonEscape(report.warnings[i]) << "\"";
    }
    out << "],\n";
    out << "  \"phases\": [\n";
    for (size_t i = 0; i < report.phases.size(); ++i) {
        const auto& phase = report.phases[i];
        out << "    {\"name\": \"" << core::jsonEscape(phase.name) << "\", \"status\": \""
            << core::jsonEscape(phase.status) << "\", \"detail\": \"" << core::jsonEscape(phase.detail) << "\"}";
        out << (i + 1 == report.phases.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"modules\": [\n";
    for (size_t i = 0; i < report.modules.size(); ++i) {
        const auto& module = report.modules[i];
        out << "    {\"path\": \"" << core::jsonEscape(module.path) << "\", \"functions\": " << module.functions
            << ", \"instructions\": " << module.instructions << ", \"calls\": " << module.calls
            << ", \"resource_refs\": " << module.resourceRefs << "}";
        out << (i + 1 == report.modules.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"native_obligations\": [\n";
    for (size_t i = 0; i < report.obligations.size(); ++i) {
        const auto& obligation = report.obligations[i];
        out << "    {\"api\": \"" << core::jsonEscape(obligation.api) << "\", \"service\": \""
            << core::jsonEscape(obligation.service) << "\", \"calls\": " << obligation.calls
            << ", \"status\": \"" << core::jsonEscape(obligation.status) << "\"}";
        out << (i + 1 == report.obligations.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

std::string sceneEntryRuntimeReportToJson(const SceneEntryRuntimeReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.scene_entry_runtime_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " title_new_game_executed=" << (report.titleNewGameExecuted ? "true" : "false")
        << " mission_setup_executed=" << (report.missionSetupExecuted ? "true" : "false")
        << " stage_ready=" << (report.stageReady ? "true" : "false")
        << " actor_ready=" << (report.actorReady ? "true" : "false")
        << " camera_ready=" << (report.cameraReady ? "true" : "false")
        << " event_ready=" << (report.eventReady ? "true" : "false")
        << " resource_bindings=" << report.resourceBindings << " missing_resources=" << report.missingResources
        << " script_bindings=" << report.scriptBindings << " missing_scripts=" << report.missingScripts << "\",\n";
    out << "  \"errors\": ";
    writeJsonStringArray(out, report.errors);
    out << ",\n";
    out << "  \"warnings\": ";
    writeJsonStringArray(out, report.warnings);
    out << ",\n";
    out << "  \"title_transition\": {";
    out << "\"executed\": " << (report.titleNewGameExecuted ? "true" : "false")
        << ", \"status\": \"" << core::jsonEscape(report.titleStatus)
        << "\", \"make_new_game_commands\": " << report.makeNewGameCommands
        << ", \"start_game_commands\": " << report.startGameCommands
        << ", \"queued_stage_loads\": " << report.queuedStageLoads
        << ", \"started_mission\": \"" << core::jsonEscape(report.startedMission)
        << "\", \"start_new_game\": \"" << core::jsonEscape(report.startNewGame)
        << "\", \"queued_mission_script\": \"" << core::jsonEscape(report.queuedMissionScript)
        << "\", \"queued_stage\": \"" << core::jsonEscape(report.queuedStage)
        << "\", \"queued_rail_camera\": \"" << core::jsonEscape(report.queuedRailCamera) << "\"},\n";
    out << "  \"mission_setup\": {";
    out << "\"executed\": " << (report.missionSetupExecuted ? "true" : "false")
        << ", \"status\": \"" << core::jsonEscape(report.missionStatus)
        << "\", \"mission_script\": \"" << core::jsonEscape(report.missionScript)
        << "\", \"event_script\": \"" << core::jsonEscape(report.eventScript)
        << "\", \"active_loader\": \"" << core::jsonEscape(report.activeLoader)
        << "\", \"stage\": \"" << core::jsonEscape(report.stagePath)
        << "\", \"stage_model\": \"" << core::jsonEscape(report.stageModelPath)
        << "\", \"stage_collision\": \"" << core::jsonEscape(report.stageCollisionPath)
        << "\", \"rail_camera\": \"" << core::jsonEscape(report.railCameraPath) << "\"},\n";
    out << "  \"actor\": {";
    out << "\"ready\": " << (report.actorReady ? "true" : "false")
        << ", \"player_chara\": \"" << core::jsonEscape(report.playerChara)
        << "\", \"player_script_asset\": \"" << core::jsonEscape(report.playerScriptAsset)
        << "\", \"player_pcg_asset\": \"" << core::jsonEscape(report.playerPcgAsset)
        << "\", \"player_script_module\": \"" << core::jsonEscape(report.playerScriptModule)
        << "\", \"player_pcg_module\": \"" << core::jsonEscape(report.playerPcgModule)
        << "\", \"spawn_position\": \"" << core::jsonEscape(report.spawnPosition)
        << "\", \"spawn_rot_y\": \"" << core::jsonEscape(report.spawnRotY)
        << "\", \"checkpoint\": \"" << core::jsonEscape(report.checkpoint) << "\"},\n";
    out << "  \"camera\": {";
    out << "\"ready\": " << (report.cameraReady ? "true" : "false")
        << ", \"rail_camera_path\": \"" << core::jsonEscape(report.railCameraPath)
        << "\", \"rail_camera_enabled\": \"" << core::jsonEscape(report.railCameraEnabled)
        << "\", \"auto_camera_adjust_enabled\": \"" << core::jsonEscape(report.autoCameraAdjustEnabled)
        << "\", \"default_camera_state_target\": \"" << core::jsonEscape(report.defaultCameraStateTarget) << "\"},\n";
    out << "  \"readiness\": {";
    out << "\"stage_ready\": " << (report.stageReady ? "true" : "false")
        << ", \"actor_ready\": " << (report.actorReady ? "true" : "false")
        << ", \"camera_ready\": " << (report.cameraReady ? "true" : "false")
        << ", \"event_ready\": " << (report.eventReady ? "true" : "false") << "},\n";
    out << "  \"bindings\": [\n";
    for (size_t i = 0; i < report.bindings.size(); ++i) {
        out << "    ";
        writeBinding(out, report.bindings[i]);
        out << (i + 1 == report.bindings.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

} // namespace yu::runtime
