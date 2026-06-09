#pragma once

#include "yuengine/native/NativeServices.h"
#include "yuengine/project/ProjectManifest.h"
#include "yuengine/resource/VirtualFileSystem.h"
#include "yuengine/runtime/RuntimeContext.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace yu::runtime {

struct ModuleReport {
    std::string path;
    int functions = 0;
    int instructions = 0;
    int calls = 0;
    int resourceRefs = 0;
};

struct ObligationReport {
    std::string api;
    std::string service;
    int calls = 0;
    std::string status;
};

struct BootReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::vector<BootPhaseReport> phases;
    std::vector<ModuleReport> modules;
    std::vector<ObligationReport> obligations;
    size_t looseMounts = 0;
    size_t packResources = 0;
    size_t nativeApis = 0;
    size_t nativeServices = 0;
    size_t unboundNativeApis = 0;
};

BootReport bootProject(const std::filesystem::path& manifestPath, const std::filesystem::path& repoRoot);
std::string bootReportToJson(const BootReport& report);

struct RuntimeBindingReport {
    std::string role;
    std::string kind;
    std::string query;
    bool required = true;
    bool found = false;
    std::string mountType;
    std::string virtualPath;
    std::string physicalPath;
    std::string pack;
    int64_t size = -1;
};

struct SceneEntryRuntimeReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    bool titleNewGameExecuted = false;
    bool missionSetupExecuted = false;
    std::string titleStatus;
    std::string missionStatus;
    std::string startedMission;
    std::string startNewGame;
    int makeNewGameCommands = 0;
    int startGameCommands = 0;
    int queuedStageLoads = 0;
    std::string queuedMissionScript;
    std::string queuedStage;
    std::string queuedRailCamera;
    std::string missionScript;
    std::string eventScript;
    std::string activeLoader;
    std::string stagePath;
    std::string stageModelPath;
    std::string stageCollisionPath;
    std::string railCameraPath;
    std::string playerChara;
    std::string playerScriptAsset;
    std::string playerPcgAsset;
    std::string playerScriptModule;
    std::string playerPcgModule;
    std::string spawnPosition;
    std::string spawnRotY;
    std::string checkpoint;
    std::string railCameraEnabled;
    std::string autoCameraAdjustEnabled;
    std::string defaultCameraStateTarget;
    bool stageReady = false;
    bool actorReady = false;
    bool cameraReady = false;
    bool eventReady = false;
    int resourceBindings = 0;
    int missingResources = 0;
    int scriptBindings = 0;
    int missingScripts = 0;
    std::vector<RuntimeBindingReport> bindings;
};

SceneEntryRuntimeReport buildSceneEntryRuntimeReport(
    const project::ProjectManifest& manifest,
    const native::NativeRuntimeServiceState& titleState,
    const native::NativeRuntimeServiceState& missionState,
    const resource::VirtualFileSystem& vfs);

SceneEntryRuntimeReport runSceneEntryRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string sceneEntryRuntimeReportToJson(const SceneEntryRuntimeReport& report);

} // namespace yu::runtime
