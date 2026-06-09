#include "yuengine/runtime/SceneRuntime.h"

#include "yuengine/core/Json.h"
#include "yuengine/project/ProjectManifest.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <map>
#include <optional>
#include <set>
#include <sstream>

namespace yu::runtime {
namespace {

constexpr size_t kResourceHeaderSize = 0x100;
constexpr size_t kCollisionBodyOffset = 0x120;
constexpr uint32_t kModelMeshTag = 0x0068736D; // "msh\0"
constexpr uint32_t kModelMaterialTag = 0x0074616D; // "mat\0"

void addError(SceneRuntimeMaterializationReport& report, const std::string& message)
{
    report.ok = false;
    report.errors.push_back(message);
}

bool readU32(const std::vector<std::byte>& bytes, size_t offset, uint32_t& out)
{
    if (offset + sizeof(uint32_t) > bytes.size()) {
        return false;
    }
    std::memcpy(&out, bytes.data() + offset, sizeof(out));
    return true;
}

bool readF32(const std::vector<std::byte>& bytes, size_t offset, float& out)
{
    if (offset + sizeof(float) > bytes.size()) {
        return false;
    }
    std::memcpy(&out, bytes.data() + offset, sizeof(out));
    return std::isfinite(out);
}

std::string readFixedString(const std::vector<std::byte>& bytes, size_t offset, size_t limit)
{
    if (offset >= bytes.size()) {
        return {};
    }
    const size_t end = std::min(offset + limit, bytes.size());
    std::string out;
    for (size_t i = offset; i < end; ++i) {
        const auto c = static_cast<unsigned char>(bytes[i]);
        if (c == 0) {
            break;
        }
        if (c < 0x20 || c > 0x7E) {
            break;
        }
        out.push_back(static_cast<char>(c));
    }
    return out;
}

bool hasExtension(const std::string& path, const std::string& extension)
{
    const std::string normalized = resource::normalizeResourcePath(path);
    return normalized.ends_with(extension);
}

std::string extensionOf(const std::string& path)
{
    const std::string normalized = resource::normalizeResourcePath(path);
    const size_t dot = normalized.find_last_of('.');
    return dot == std::string::npos ? std::string() : normalized.substr(dot);
}

std::string directoryOf(const std::string& path)
{
    const std::string normalized = resource::normalizeResourcePath(path);
    const size_t slash = normalized.find_last_of('/');
    return slash == std::string::npos ? std::string() : normalized.substr(0, slash);
}

bool isPrintableAscii(const std::string& value)
{
    return std::all_of(value.begin(), value.end(), [](unsigned char c) {
        return c >= 0x20 && c <= 0x7E;
    });
}

bool hasResourceExtension(const std::string& value)
{
    static constexpr std::array<const char*, 9> extensions{
        ".sge", ".mdl", ".col", ".rcm", ".dds", ".bnt", ".b64", ".nut", ".se",
    };
    const std::string normalized = resource::normalizeResourcePath(value);
    return std::any_of(extensions.begin(), extensions.end(), [&](const char* extension) {
        return normalized.ends_with(extension);
    });
}

std::string resolveSiblingPath(
    const resource::VirtualFileSystem& vfs,
    const std::string& ownerPath,
    const std::string& token)
{
    std::string normalized = resource::normalizeResourcePath(token);
    std::vector<std::string> candidates;
    candidates.push_back(normalized);
    if (normalized.find('/') == std::string::npos) {
        const std::string ownerDir = directoryOf(ownerPath);
        if (!ownerDir.empty()) {
            candidates.push_back(ownerDir + "/" + normalized);
        }
    }
    for (const auto& candidate : candidates) {
        if (vfs.resolvePath(candidate).found) {
            return candidate;
        }
    }
    return normalized;
}

std::vector<std::string> extractAsciiTokens(
    const std::vector<std::byte>& bytes,
    size_t beginOffset,
    size_t endOffset)
{
    std::vector<std::string> tokens;
    const size_t end = std::min(endOffset, bytes.size());
    for (size_t offset = beginOffset; offset < end;) {
        const auto c = static_cast<unsigned char>(bytes[offset]);
        if (c < 0x20 || c > 0x7E) {
            ++offset;
            continue;
        }
        const size_t start = offset;
        while (offset < end) {
            const auto current = static_cast<unsigned char>(bytes[offset]);
            if (current < 0x20 || current > 0x7E) {
                break;
            }
            ++offset;
        }
        tokens.emplace_back(reinterpret_cast<const char*>(bytes.data() + start), offset - start);
    }
    return tokens;
}

RuntimeResourceHeaderReport readHeader(
    const resource::VirtualFileSystem& vfs,
    const std::string& role,
    const std::string& path,
    const std::string& expectedExtension)
{
    RuntimeResourceHeaderReport report;
    report.role = role;
    report.path = resource::normalizeResourcePath(path);
    report.expectedExtension = expectedExtension;

    const auto bytes = vfs.readBytes(path);
    report.bytesFound = bytes.found;
    report.byteSize = bytes.found ? static_cast<int64_t>(bytes.bytes.size()) : -1;
    report.physicalPath = bytes.physicalPath.string();
    if (!bytes.found || bytes.bytes.size() < kResourceHeaderSize) {
        return report;
    }

    report.actualExtension = readFixedString(bytes.bytes, 0x10, 16);
    report.resourceSet = readFixedString(bytes.bytes, 0x20, 32);
    report.valid = readFixedString(bytes.bytes, 0, 16) == "MgResourceHeader"
        && report.actualExtension == expectedExtension;
    return report;
}

void addUnique(std::vector<std::string>& values, const std::string& value)
{
    if (std::find(values.begin(), values.end(), value) == values.end()) {
        values.push_back(value);
    }
}

std::vector<std::string> extractStageDependencies(
    const resource::VirtualFileSystem& vfs,
    const std::string& stagePath,
    const std::vector<std::byte>& bytes,
    int& elementCandidates)
{
    std::vector<std::string> dependencies;
    std::set<size_t> transformAnchors;

    for (size_t offset = kResourceHeaderSize; offset + sizeof(uint32_t) < bytes.size(); ++offset) {
        uint32_t length = 0;
        if (!readU32(bytes, offset, length) || length < 5 || length > 192) {
            continue;
        }
        const size_t stringOffset = offset + sizeof(uint32_t);
        const size_t stringEnd = stringOffset + length;
        if (stringEnd >= bytes.size() || bytes[stringEnd] != std::byte{0}) {
            continue;
        }
        std::string token(reinterpret_cast<const char*>(bytes.data() + stringOffset), length);
        if (!isPrintableAscii(token) || !hasResourceExtension(token)) {
            continue;
        }
        const std::string resolved = resolveSiblingPath(vfs, stagePath, token);
        if (!vfs.resolvePath(resolved).found) {
            continue;
        }
        addUnique(dependencies, resolved);
        for (size_t probe = stringEnd + 1; probe + 0x48 <= std::min(stringEnd + 1 + 0x80, bytes.size()); ++probe) {
            uint32_t tagA = 0;
            uint32_t tagB = 0;
            float scaleX = 0.0F;
            float rotationW = 0.0F;
            if (readU32(bytes, probe, tagA) && readU32(bytes, probe + 4, tagB) && tagA == 4 && tagB == 1
                && readF32(bytes, probe + 0x20, scaleX) && readF32(bytes, probe + 0x38, rotationW)
                && std::abs(scaleX) < 1'000'000.0F && std::abs(rotationW) <= 1.1F) {
                transformAnchors.insert(probe);
                break;
            }
        }
        offset = stringEnd;
    }

    for (size_t offset = kResourceHeaderSize; offset < bytes.size();) {
        const auto c = static_cast<unsigned char>(bytes[offset]);
        if (c < 0x20 || c > 0x7E) {
            ++offset;
            continue;
        }
        const size_t start = offset;
        while (offset < bytes.size()) {
            const auto current = static_cast<unsigned char>(bytes[offset]);
            if (current < 0x20 || current > 0x7E) {
                break;
            }
            ++offset;
        }
        std::string token(reinterpret_cast<const char*>(bytes.data() + start), offset - start);
        if (token.size() >= 5 && token.size() <= 192 && hasResourceExtension(token)) {
            const std::string resolved = resolveSiblingPath(vfs, stagePath, token);
            if (vfs.resolvePath(resolved).found) {
                addUnique(dependencies, resolved);
            }
        }
    }

    std::sort(dependencies.begin(), dependencies.end());
    elementCandidates = static_cast<int>(transformAnchors.size());
    return dependencies;
}

void parseCollision(
    StageGraphRuntimeHandle& stage,
    const std::vector<std::byte>& bytes)
{
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    if (!readU32(bytes, 0x100, vertexCount) || !readU32(bytes, 0x104, indexCount)
        || vertexCount == 0 || indexCount == 0 || indexCount % 3 != 0) {
        return;
    }

    const uint64_t vertexBytes = static_cast<uint64_t>(vertexCount) * sizeof(float) * 3U;
    const uint64_t indexStart = kCollisionBodyOffset + vertexBytes;
    const uint64_t indexBytes = static_cast<uint64_t>(indexCount) * sizeof(uint32_t);
    if (indexStart + indexBytes > bytes.size()) {
        return;
    }

    RuntimeVec3 min;
    RuntimeVec3 max;
    for (uint32_t i = 0; i < vertexCount; ++i) {
        const size_t offset = kCollisionBodyOffset + static_cast<size_t>(i) * sizeof(float) * 3U;
        RuntimeVec3 vertex;
        if (!readF32(bytes, offset, vertex.x) || !readF32(bytes, offset + 4, vertex.y)
            || !readF32(bytes, offset + 8, vertex.z)) {
            return;
        }
        if (i == 0) {
            min = vertex;
            max = vertex;
        } else {
            min.x = std::min(min.x, vertex.x);
            min.y = std::min(min.y, vertex.y);
            min.z = std::min(min.z, vertex.z);
            max.x = std::max(max.x, vertex.x);
            max.y = std::max(max.y, vertex.y);
            max.z = std::max(max.z, vertex.z);
        }
    }

    stage.collisionVertexCount = static_cast<int>(vertexCount);
    stage.collisionIndexCount = static_cast<int>(indexCount);
    stage.collisionTriangleCount = static_cast<int>(indexCount / 3);
    stage.collisionBoundsMin = min;
    stage.collisionBoundsMax = max;
}

int countTag(const std::vector<std::byte>& bytes, uint32_t tag)
{
    int count = 0;
    for (size_t offset = kResourceHeaderSize; offset + sizeof(uint32_t) <= bytes.size(); ++offset) {
        uint32_t value = 0;
        if (readU32(bytes, offset, value) && value == tag) {
            ++count;
        }
    }
    return count;
}

int countPlausibleModelMeshes(const std::vector<std::byte>& bytes)
{
    int count = 0;
    for (size_t offset = kResourceHeaderSize; offset + 0x20 <= bytes.size(); ++offset) {
        uint32_t tag = 0;
        uint32_t stride = 0;
        uint32_t vertices = 0;
        uint32_t triangles = 0;
        if (!readU32(bytes, offset, tag) || tag != kModelMeshTag || !readU32(bytes, offset + 0x08, stride)
            || !readU32(bytes, offset + 0x0C, vertices) || !readU32(bytes, offset + 0x10, triangles)) {
            continue;
        }
        const uint64_t vertexStart = offset + 0x20;
        const uint64_t vertexBytes = static_cast<uint64_t>(stride) * vertices;
        const uint64_t indexBytes = static_cast<uint64_t>(triangles) * 3U * sizeof(uint16_t);
        if (stride >= 12 && stride <= 256 && vertices > 0 && vertices < 1'000'000 && triangles > 0
            && triangles < 1'000'000 && vertexStart + vertexBytes + indexBytes <= bytes.size()) {
            ++count;
        }
    }
    return count;
}

std::vector<std::string> extractModelTextures(
    const resource::VirtualFileSystem& vfs,
    const std::string& modelPath,
    const std::vector<std::byte>& bytes)
{
    std::vector<std::string> textures;
    for (const auto& token : extractAsciiTokens(bytes, kResourceHeaderSize, bytes.size())) {
        if (!hasExtension(token, ".dds")) {
            continue;
        }
        const std::string resolved = resolveSiblingPath(vfs, modelPath, token);
        if (vfs.resolvePath(resolved).found) {
            addUnique(textures, resolved);
        }
    }
    std::sort(textures.begin(), textures.end());
    return textures;
}

std::string markerFromExpression(const std::string& expression)
{
    const std::string prefix = "marker:";
    const size_t begin = expression.find(prefix);
    if (begin == std::string::npos) {
        return {};
    }
    size_t end = expression.find("._", begin);
    if (end == std::string::npos) {
        end = expression.find(')', begin);
    }
    return expression.substr(begin, end == std::string::npos ? std::string::npos : end - begin);
}

void writeStringArray(std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        out << (i == 0 ? "" : ", ") << "\"" << core::jsonEscape(values[i]) << "\"";
    }
    out << "]";
}

void writeVec3(std::ostringstream& out, const RuntimeVec3& value)
{
    out << "{\"x\": " << value.x << ", \"y\": " << value.y << ", \"z\": " << value.z << "}";
}

void writeHeader(std::ostringstream& out, const RuntimeResourceHeaderReport& header)
{
    out << "{\"role\": \"" << core::jsonEscape(header.role)
        << "\", \"path\": \"" << core::jsonEscape(header.path)
        << "\", \"bytes_found\": " << (header.bytesFound ? "true" : "false")
        << ", \"valid\": " << (header.valid ? "true" : "false")
        << ", \"expected_extension\": \"" << core::jsonEscape(header.expectedExtension)
        << "\", \"actual_extension\": \"" << core::jsonEscape(header.actualExtension)
        << "\", \"resource_set\": \"" << core::jsonEscape(header.resourceSet)
        << "\", \"physical_path\": \"" << core::jsonEscape(header.physicalPath)
        << "\", \"byte_size\": " << header.byteSize << "}";
}

void writeDependency(std::ostringstream& out, const StageDependencyHandle& dependency)
{
    out << "{\"path\": \"" << core::jsonEscape(dependency.path)
        << "\", \"extension\": \"" << core::jsonEscape(dependency.extension)
        << "\", \"found\": " << (dependency.found ? "true" : "false")
        << ", \"byte_size\": " << dependency.byteSize << "}";
}

} // namespace

SceneRuntimeMaterializationReport materializeSceneEntryRuntime(
    const SceneEntryRuntimeReport& sceneEntry,
    const resource::VirtualFileSystem& vfs)
{
    SceneRuntimeMaterializationReport report;
    report.projectId = sceneEntry.projectId;
    report.sceneEntryOk = sceneEntry.ok;
    if (!sceneEntry.ok) {
        addError(report, "scene-entry contract is not ready");
    }

    auto addHeader = [&](const std::string& role, const std::string& path, const std::string& extension) {
        report.resourceHeaders.push_back(readHeader(vfs, role, path, extension));
        if (!report.resourceHeaders.back().valid) {
            addError(report, "runtime resource header is not valid: " + role + "=" + path);
        }
    };

    addHeader("stage_sge", sceneEntry.stagePath, "sge");
    addHeader("stage_model", sceneEntry.stageModelPath, "mdl");
    addHeader("stage_collision", sceneEntry.stageCollisionPath, "col");
    addHeader("rail_camera", sceneEntry.railCameraPath, "rcm");

    const auto stageBytes = vfs.readBytes(sceneEntry.stagePath);
    const auto modelBytes = vfs.readBytes(sceneEntry.stageModelPath);
    const auto collisionBytes = vfs.readBytes(sceneEntry.stageCollisionPath);
    const auto railBytes = vfs.readBytes(sceneEntry.railCameraPath);
    const auto playerScriptBytes = vfs.readBytes(sceneEntry.playerScriptAsset);
    const auto playerPcgBytes = vfs.readBytes(sceneEntry.playerPcgAsset);

    report.stage.stagePath = resource::normalizeResourcePath(sceneEntry.stagePath);
    report.stage.stageByteSize = stageBytes.found ? static_cast<int64_t>(stageBytes.bytes.size()) : -1;
    if (stageBytes.found) {
        auto dependencyPaths =
            extractStageDependencies(vfs, sceneEntry.stagePath, stageBytes.bytes, report.stage.stageElementCandidates);
        addUnique(dependencyPaths, resource::normalizeResourcePath(sceneEntry.stageModelPath));
        addUnique(dependencyPaths, resource::normalizeResourcePath(sceneEntry.stageCollisionPath));
        if (modelBytes.found) {
            for (const auto& texture : extractModelTextures(vfs, sceneEntry.stageModelPath, modelBytes.bytes)) {
                addUnique(dependencyPaths, texture);
            }
        }
        std::sort(dependencyPaths.begin(), dependencyPaths.end());

        for (const auto& path : dependencyPaths) {
            const auto bytes = vfs.readBytes(path);
            StageDependencyHandle dependency;
            dependency.path = resource::normalizeResourcePath(path);
            dependency.extension = extensionOf(path);
            dependency.found = bytes.found || vfs.resolvePath(path).found;
            dependency.byteSize = bytes.found ? static_cast<int64_t>(bytes.bytes.size()) : -1;
            if (!dependency.found) {
                ++report.stage.missingDependencyCount;
            }
            if (dependency.extension == ".mdl") {
                ++report.stage.modelDependencyCount;
            } else if (dependency.extension == ".col") {
                ++report.stage.collisionDependencyCount;
            } else if (dependency.extension == ".dds") {
                ++report.stage.textureDependencyCount;
            }
            report.stage.dependencies.push_back(std::move(dependency));
        }
        report.stage.dependencyCount = static_cast<int>(report.stage.dependencies.size());
    }

    if (modelBytes.found) {
        report.stage.modelMeshCount = countPlausibleModelMeshes(modelBytes.bytes);
        report.stage.materialCount = countTag(modelBytes.bytes, kModelMaterialTag);
    }
    if (collisionBytes.found) {
        parseCollision(report.stage, collisionBytes.bytes);
    }
    report.stage.ready = stageBytes.found && modelBytes.found && collisionBytes.found
        && report.stage.modelDependencyCount > 0 && report.stage.collisionDependencyCount > 0
        && report.stage.missingDependencyCount == 0 && report.stage.collisionTriangleCount > 0;
    if (!report.stage.ready) {
        addError(report, "stage graph runtime handle is not ready");
    }

    report.actor.playerChara = sceneEntry.playerChara;
    report.actor.playerScriptAsset = resource::normalizeResourcePath(sceneEntry.playerScriptAsset);
    report.actor.playerPcgAsset = resource::normalizeResourcePath(sceneEntry.playerPcgAsset);
    report.actor.spawnPositionExpression = sceneEntry.spawnPosition;
    report.actor.spawnRotY = sceneEntry.spawnRotY;
    report.actor.playerScriptBytes = playerScriptBytes.found ? static_cast<int64_t>(playerScriptBytes.bytes.size()) : -1;
    report.actor.playerPcgBytes = playerPcgBytes.found ? static_cast<int64_t>(playerPcgBytes.bytes.size()) : -1;
    report.actor.ready = !report.actor.playerChara.empty() && playerScriptBytes.found && playerPcgBytes.found
        && !report.actor.spawnPositionExpression.empty() && !report.actor.spawnRotY.empty();
    if (!report.actor.ready) {
        addError(report, "actor runtime handle is not ready");
    }

    report.camera.railCameraPath = resource::normalizeResourcePath(sceneEntry.railCameraPath);
    report.camera.railCameraEnabled = sceneEntry.railCameraEnabled;
    report.camera.autoCameraAdjustEnabled = sceneEntry.autoCameraAdjustEnabled;
    report.camera.defaultCameraStateTarget = sceneEntry.defaultCameraStateTarget;
    report.camera.railCameraBytes = railBytes.found ? static_cast<int64_t>(railBytes.bytes.size()) : -1;
    if (railBytes.found) {
        uint32_t count = 0;
        if (readU32(railBytes.bytes, 0x100, count) && count < 100000) {
            report.camera.railNodeCountCandidate = static_cast<int>(count);
        }
    }
    report.camera.ready = railBytes.found && report.camera.railNodeCountCandidate > 0
        && !report.camera.defaultCameraStateTarget.empty();
    if (!report.camera.ready) {
        addError(report, "camera runtime handle is not ready");
    }

    report.eventMarker.positionExpression = sceneEntry.spawnPosition;
    report.eventMarker.rotationYExpression = sceneEntry.spawnRotY;
    report.eventMarker.checkpointExpression = sceneEntry.checkpoint;
    report.eventMarker.marker = markerFromExpression(sceneEntry.spawnPosition);
    report.eventMarker.ready = !report.eventMarker.marker.empty() && !report.eventMarker.positionExpression.empty()
        && !report.eventMarker.checkpointExpression.empty();
    if (!report.eventMarker.ready) {
        addError(report, "event marker runtime handle is not ready");
    }

    return report;
}

SceneRuntimeMaterializationReport runSceneRuntimeMaterialization(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot)
{
    const auto manifest = project::loadProjectManifest(manifestPath);
    resource::VirtualFileSystem vfs;
    vfs.mountProject(manifest);
    auto sceneEntry = runSceneEntryRuntime(manifestPath, repoRoot);
    return materializeSceneEntryRuntime(sceneEntry, vfs);
}

std::string sceneRuntimeMaterializationReportToJson(const SceneRuntimeMaterializationReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.scene_runtime_materialization_report.v1\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"project_id\": \"" << core::jsonEscape(report.projectId) << "\",\n";
    out << "  \"metrics\": \"ok=" << (report.ok ? "true" : "false")
        << " scene_entry_ok=" << (report.sceneEntryOk ? "true" : "false")
        << " stage_handle_ready=" << (report.stage.ready ? "true" : "false")
        << " actor_handle_ready=" << (report.actor.ready ? "true" : "false")
        << " camera_handle_ready=" << (report.camera.ready ? "true" : "false")
        << " event_marker_ready=" << (report.eventMarker.ready ? "true" : "false")
        << " stage_dependencies=" << report.stage.dependencyCount
        << " missing_stage_dependencies=" << report.stage.missingDependencyCount
        << " model_meshes=" << report.stage.modelMeshCount
        << " collision_triangles=" << report.stage.collisionTriangleCount
        << " rail_nodes=" << report.camera.railNodeCountCandidate << "\",\n";
    out << "  \"errors\": ";
    writeStringArray(out, report.errors);
    out << ",\n";
    out << "  \"resource_headers\": [\n";
    for (size_t i = 0; i < report.resourceHeaders.size(); ++i) {
        out << "    ";
        writeHeader(out, report.resourceHeaders[i]);
        out << (i + 1 == report.resourceHeaders.size() ? "\n" : ",\n");
    }
    out << "  ],\n";
    out << "  \"stage\": {";
    out << "\"ready\": " << (report.stage.ready ? "true" : "false")
        << ", \"stage_path\": \"" << core::jsonEscape(report.stage.stagePath)
        << "\", \"stage_byte_size\": " << report.stage.stageByteSize
        << ", \"stage_element_candidates\": " << report.stage.stageElementCandidates
        << ", \"dependency_count\": " << report.stage.dependencyCount
        << ", \"model_dependency_count\": " << report.stage.modelDependencyCount
        << ", \"collision_dependency_count\": " << report.stage.collisionDependencyCount
        << ", \"texture_dependency_count\": " << report.stage.textureDependencyCount
        << ", \"missing_dependency_count\": " << report.stage.missingDependencyCount
        << ", \"model_mesh_count\": " << report.stage.modelMeshCount
        << ", \"material_count\": " << report.stage.materialCount
        << ", \"collision_vertex_count\": " << report.stage.collisionVertexCount
        << ", \"collision_index_count\": " << report.stage.collisionIndexCount
        << ", \"collision_triangle_count\": " << report.stage.collisionTriangleCount
        << ", \"collision_bounds_min\": ";
    writeVec3(out, report.stage.collisionBoundsMin);
    out << ", \"collision_bounds_max\": ";
    writeVec3(out, report.stage.collisionBoundsMax);
    out << ", \"dependencies\": [";
    for (size_t i = 0; i < report.stage.dependencies.size(); ++i) {
        out << (i == 0 ? "" : ", ");
        writeDependency(out, report.stage.dependencies[i]);
    }
    out << "]},\n";
    out << "  \"actor\": {";
    out << "\"ready\": " << (report.actor.ready ? "true" : "false")
        << ", \"player_chara\": \"" << core::jsonEscape(report.actor.playerChara)
        << "\", \"player_script_asset\": \"" << core::jsonEscape(report.actor.playerScriptAsset)
        << "\", \"player_pcg_asset\": \"" << core::jsonEscape(report.actor.playerPcgAsset)
        << "\", \"player_script_bytes\": " << report.actor.playerScriptBytes
        << ", \"player_pcg_bytes\": " << report.actor.playerPcgBytes
        << ", \"spawn_position_expression\": \"" << core::jsonEscape(report.actor.spawnPositionExpression)
        << "\", \"spawn_rot_y\": \"" << core::jsonEscape(report.actor.spawnRotY) << "\"},\n";
    out << "  \"camera\": {";
    out << "\"ready\": " << (report.camera.ready ? "true" : "false")
        << ", \"rail_camera_path\": \"" << core::jsonEscape(report.camera.railCameraPath)
        << "\", \"rail_camera_bytes\": " << report.camera.railCameraBytes
        << ", \"rail_node_count_candidate\": " << report.camera.railNodeCountCandidate
        << ", \"rail_camera_enabled\": \"" << core::jsonEscape(report.camera.railCameraEnabled)
        << "\", \"auto_camera_adjust_enabled\": \"" << core::jsonEscape(report.camera.autoCameraAdjustEnabled)
        << "\", \"default_camera_state_target\": \"" << core::jsonEscape(report.camera.defaultCameraStateTarget)
        << "\"},\n";
    out << "  \"event_marker\": {";
    out << "\"ready\": " << (report.eventMarker.ready ? "true" : "false")
        << ", \"marker\": \"" << core::jsonEscape(report.eventMarker.marker)
        << "\", \"position_expression\": \"" << core::jsonEscape(report.eventMarker.positionExpression)
        << "\", \"rotation_y_expression\": \"" << core::jsonEscape(report.eventMarker.rotationYExpression)
        << "\", \"checkpoint_expression\": \"" << core::jsonEscape(report.eventMarker.checkpointExpression)
        << "\"}\n";
    out << "}\n";
    return out.str();
}

} // namespace yu::runtime
