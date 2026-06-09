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

FirstFrameRuntimeReport buildFirstFrameRuntimeReport(
    const SceneRuntimeMaterializationReport& sceneRuntime,
    const std::string& rendererProfile);

FirstFrameRuntimeReport runFirstFrameRuntime(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string firstFrameRuntimeReportToJson(const FirstFrameRuntimeReport& report);

} // namespace yu::runtime
