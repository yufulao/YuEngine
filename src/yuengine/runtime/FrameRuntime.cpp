#include "yuengine/runtime/FrameRuntime.h"

#include "yuengine/core/Json.h"
#include "yuengine/project/ProjectManifest.h"

#include <sstream>

namespace yu::runtime {
namespace {

void addError(FirstFrameRuntimeReport& report, const std::string& message)
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

} // namespace yu::runtime
