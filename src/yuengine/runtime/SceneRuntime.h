#pragma once

#include "yuengine/resource/VirtualFileSystem.h"
#include "yuengine/runtime/EngineRuntime.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace yu::runtime {

struct RuntimeVec3 {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
};

struct RuntimeResourceHeaderReport {
    std::string role;
    std::string path;
    bool bytesFound = false;
    bool valid = false;
    std::string expectedExtension;
    std::string actualExtension;
    std::string resourceSet;
    std::string physicalPath;
    int64_t byteSize = -1;
};

struct StageDependencyHandle {
    std::string path;
    std::string extension;
    bool found = false;
    int64_t byteSize = -1;
};

struct ModelTextureSlotRuntimeHandle {
    std::string slot;
    std::string token;
    std::string path;
    bool found = false;
    int64_t byteSize = -1;
};

struct ModelMaterialRuntimeHandle {
    int index = -1;
    std::string name;
    int64_t byteOffset = -1;
    int64_t byteSize = -1;
    int headerFlag0 = 0;
    int headerFlag1 = 0;
    int headerFlag2 = 0;
    RuntimeVec3 colorCandidate;
    int textureSlotCount = 0;
    int resolvedTextureSlots = 0;
    int meshBindingCount = 0;
    std::vector<ModelTextureSlotRuntimeHandle> textureSlots;
};

struct ModelMeshRuntimeHandle {
    int index = -1;
    std::string name;
    int64_t byteOffset = -1;
    int vertexStride = 0;
    int vertexCount = 0;
    int triangleCount = 0;
    int materialIndex = -1;
    std::string materialName;
    std::string materialMatchRule;
};

struct StageGraphRuntimeHandle {
    bool ready = false;
    std::string stagePath;
    int64_t stageByteSize = -1;
    std::string modelPath;
    int64_t modelByteSize = -1;
    int stageElementCandidates = 0;
    int dependencyCount = 0;
    int modelDependencyCount = 0;
    int collisionDependencyCount = 0;
    int textureDependencyCount = 0;
    int missingDependencyCount = 0;
    int modelMeshCount = 0;
    int materialCount = 0;
    int materialTextureSlotCount = 0;
    int materialTextureResolvedCount = 0;
    int namedMeshCount = 0;
    int meshMaterialBindingCount = 0;
    int unresolvedMeshMaterialBindingCount = 0;
    int collisionVertexCount = 0;
    int collisionIndexCount = 0;
    int collisionTriangleCount = 0;
    RuntimeVec3 collisionBoundsMin;
    RuntimeVec3 collisionBoundsMax;
    std::vector<StageDependencyHandle> dependencies;
    std::vector<ModelMaterialRuntimeHandle> materials;
    std::vector<ModelMeshRuntimeHandle> meshes;
};

struct ActorRuntimeHandle {
    bool ready = false;
    std::string playerChara;
    std::string playerScriptAsset;
    std::string playerPcgAsset;
    std::string spawnPositionExpression;
    std::string spawnRotY;
    int64_t playerScriptBytes = -1;
    int64_t playerPcgBytes = -1;
};

struct CameraRuntimeHandle {
    bool ready = false;
    std::string railCameraPath;
    std::string railCameraEnabled;
    std::string autoCameraAdjustEnabled;
    std::string defaultCameraStateTarget;
    int64_t railCameraBytes = -1;
    int railNodeCountCandidate = 0;
};

struct EventMarkerRuntimeHandle {
    bool ready = false;
    std::string marker;
    std::string positionExpression;
    std::string rotationYExpression;
    std::string checkpointExpression;
};

struct SceneRuntimeMaterializationReport {
    bool ok = true;
    std::string projectId;
    std::vector<std::string> errors;
    bool sceneEntryOk = false;
    StageGraphRuntimeHandle stage;
    ActorRuntimeHandle actor;
    CameraRuntimeHandle camera;
    EventMarkerRuntimeHandle eventMarker;
    std::vector<RuntimeResourceHeaderReport> resourceHeaders;
};

SceneRuntimeMaterializationReport materializeSceneEntryRuntime(
    const SceneEntryRuntimeReport& sceneEntry,
    const resource::VirtualFileSystem& vfs);

SceneRuntimeMaterializationReport runSceneRuntimeMaterialization(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot);

std::string sceneRuntimeMaterializationReportToJson(const SceneRuntimeMaterializationReport& report);

} // namespace yu::runtime
