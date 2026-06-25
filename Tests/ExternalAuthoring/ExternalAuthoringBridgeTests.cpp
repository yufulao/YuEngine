// Module: Tests ExternalAuthoring
// File: Tests/ExternalAuthoring/ExternalAuthoringBridgeTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "YuEngine/ExternalAuthoring/ExternalAuthoringBridge.h"
#include "YuEngine/File/FileWriteRequest.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"

namespace {
using yuengine::externalauthoring::BuildExternalAuthoringRuntimeAssetImportBridge;
using yuengine::externalauthoring::ExternalAuthoringBridgeBlockedLayer;
using yuengine::externalauthoring::ExternalAuthoringBridgeRequest;
using yuengine::externalauthoring::ExternalAuthoringBridgeResult;
using yuengine::externalauthoring::ExternalAuthoringBridgeStatus;
using yuengine::externalauthoring::ExternalAuthoringRuntimeAssetInputRow;
using yuengine::externalauthoring::ExternalAuthoringToolKind;
using yuengine::file::FileStatus;
using yuengine::file::FileWriteRequest;
using yuengine::file::MountId;
using yuengine::file::MountTable;
using yuengine::file::VirtualPath;
using yuengine::runtimeasset::ExecuteRuntimeAssetImportCookCommand;
using yuengine::runtimeasset::RuntimeAssetDataStatus;
using yuengine::runtimeasset::RuntimeAssetFileDesc;
using yuengine::runtimeasset::RuntimeAssetFileKind;
using yuengine::runtimeasset::RuntimeAssetImportCookCommandKind;
using yuengine::runtimeasset::RuntimeAssetImportCookCommandRequest;
using yuengine::runtimeasset::RuntimeAssetImportCookCommandResult;

constexpr const char *MOUNT_ID = "external-authoring";
constexpr const char *MANIFEST_PATH = "External/Export.yuauthoring";
constexpr const char *MESH_PAYLOAD_PATH = "Mesh/Cube.yumesh";
constexpr const char *SCENE_PAYLOAD_PATH = "Scene/Main.yuscene";
constexpr const char *TEST_ACCEPTED =
    "ExternalAuthoringBridge_AcceptsManifestAndEmitsRuntimeAssetImportCookInputs";
constexpr const char *TEST_MISSING_PAYLOAD =
    "ExternalAuthoringBridge_RejectsMissingPayloadWithoutMutation";
constexpr const char *TEST_INVALID_DEPENDENCY =
    "ExternalAuthoringBridge_RejectsInvalidDependencyWithoutMutation";
constexpr const char *TEST_UNSUPPORTED_FEATURE =
    "ExternalAuthoringBridge_RejectsUnsupportedFeatureWithoutMutation";
constexpr const char *TEST_UNSUPPORTED_MAPPING =
    "ExternalAuthoringBridge_RejectsUnsupportedMappingPolicyWithoutMutation";
constexpr const char *TEST_CAPACITY =
    "ExternalAuthoringBridge_RejectsOutputCapacityWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";

using TestFunction = int (*)();

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool FailStep(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return false;
}

std::filesystem::path TestRoot(std::string_view test_name) {
    std::filesystem::path root = std::filesystem::temp_directory_path();
    root /= "YuEngineExternalAuthoringBridgeTests";
    root /= std::string(test_name);
    return root;
}

std::vector<std::uint8_t> BytesFromString(const std::string &text) {
    return std::vector<std::uint8_t>(text.begin(), text.end());
}

bool WriteBytes(MountTable &table, const char *path, const std::vector<std::uint8_t> &bytes) {
    FileWriteRequest request{};
    request.mount = MountId(MOUNT_ID);
    request.path = VirtualPath(path);
    request.bytes = bytes.data();
    request.byte_count = bytes.size();
    return table.Write(request).Succeeded();
}

bool CreateMountedTable(const std::filesystem::path &root, MountTable *out_table) {
    if (out_table == nullptr) {
        return FailStep("null mount table output");
    }

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    MountTable table;
    const FileStatus mount_status = table.RegisterLooseMount(MountId(MOUNT_ID), root);
    if (mount_status != FileStatus::Success) {
        return FailStep("register loose mount failed");
    }

    *out_table = table;
    return true;
}

std::string AcceptedManifest() {
    return std::string(
        "YuExternalAuthoringExport v1\n"
        "tool=Unity\n"
        "export_id=rav8_manifest_fixture\n"
        "unit_scale=1\n"
        "handedness=right\n"
        "up_axis=y\n"
        "transform_bake=world\n"
        "material_policy=yu_material_v0\n"
        "animation_policy=sampled_clip_v0\n"
        "unsupported_feature_count=0\n"
        "entry_count=2\n"
        "entry.0.kind=Mesh\n"
        "entry.0.stable_id=1001\n"
        "entry.0.payload=Mesh/Cube.yumesh\n"
        "entry.0.content_hash=0\n"
        "entry.0.dependency_count=0\n"
        "entry.1.kind=Scene\n"
        "entry.1.stable_id=7001\n"
        "entry.1.payload=Scene/Main.yuscene\n"
        "entry.1.content_hash=0\n"
        "entry.1.dependency_count=1\n"
        "entry.1.dependency.0=1001\n");
}

std::string ReplaceFirst(std::string text, std::string_view from, std::string_view to) {
    const std::size_t found = text.find(from);
    if (found == std::string::npos) {
        return text;
    }

    text.replace(found, from.size(), to);
    return text;
}

std::string MeshPayload() {
    return std::string(
        "YUASSET MESH 1\n"
        "schema=rav0-source\n"
        "id=cube_mesh\n"
        "kind=cube\n"
        "vertices=24\n"
        "indices=36\n"
        "bounds=-1,-1,-1,1,1,1\n");
}

std::string ScenePayload() {
    return std::string(
        "YUASSET SCENE 1\n"
        "schema=rav0-source\n"
        "id=scene\n"
        "m0=Mesh/Cube.yumesh\n"
        "cam=camera:orbit\n"
        "e0=101:0,0,0\n");
}

bool WriteManifest(MountTable &table, const std::string &manifest) {
    return WriteBytes(table, MANIFEST_PATH, BytesFromString(manifest));
}

bool WriteAcceptedPayloads(MountTable &table) {
    if (!WriteBytes(table, MESH_PAYLOAD_PATH, BytesFromString(MeshPayload()))) {
        return false;
    }

    return WriteBytes(table, SCENE_PAYLOAD_PATH, BytesFromString(ScenePayload()));
}

struct BridgeHarness final {
    std::array<ExternalAuthoringRuntimeAssetInputRow, 4U> rows{};
    std::array<RuntimeAssetFileDesc, yuengine::runtimeasset::RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT>
        source_files{};
    std::array<RuntimeAssetFileDesc, yuengine::runtimeasset::RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT>
        cooked_files{};
    RuntimeAssetImportCookCommandRequest command{};
    ExternalAuthoringBridgeResult result{};
};

ExternalAuthoringBridgeRequest MakeRequest(MountTable &table, BridgeHarness &harness) {
    ExternalAuthoringBridgeRequest request{};
    request.mount_table = &table;
    request.mount = MountId(MOUNT_ID);
    request.manifest_path = VirtualPath(MANIFEST_PATH);
    request.runtime_asset_inputs = std::span<ExternalAuthoringRuntimeAssetInputRow>(harness.rows);
    request.import_cook_request = &harness.command;
    request.import_cook_source_files = harness.source_files.data();
    request.import_cook_source_file_capacity = static_cast<std::uint32_t>(harness.source_files.size());
    request.import_cook_cooked_files = harness.cooked_files.data();
    request.import_cook_cooked_file_capacity = static_cast<std::uint32_t>(harness.cooked_files.size());
    return request;
}

void SetSentinel(BridgeHarness *harness) {
    harness->rows[0].stable_id = 424242U;
    harness->rows[0].manifest_readable = true;
    harness->command.command = RuntimeAssetImportCookCommandKind::Unknown;
}

bool SentinelUnchanged(const BridgeHarness &harness) {
    if (harness.rows[0].stable_id != 424242U) {
        return false;
    }

    if (!harness.rows[0].manifest_readable) {
        return false;
    }

    return harness.command.command == RuntimeAssetImportCookCommandKind::Unknown;
}

int ExternalAuthoringBridgeAcceptsManifestAndEmitsRuntimeAssetImportCookInputs() {
    MountTable table;
    if (!CreateMountedTable(TestRoot(TEST_ACCEPTED), &table)) {
        return 1;
    }

    if (!WriteAcceptedPayloads(table)) {
        return Fail("write accepted payloads failed");
    }

    if (!WriteManifest(table, AcceptedManifest())) {
        return Fail("write manifest failed");
    }

    BridgeHarness harness{};
    const ExternalAuthoringBridgeRequest request = MakeRequest(table, harness);
    const ExternalAuthoringBridgeStatus status =
        BuildExternalAuthoringRuntimeAssetImportBridge(request, &harness.result);
    if (status != ExternalAuthoringBridgeStatus::Success) {
        return Fail("bridge rejected accepted manifest");
    }

    if (harness.result.blocked_layer != ExternalAuthoringBridgeBlockedLayer::None) {
        return Fail("success kept blocked layer");
    }

    if (harness.result.tool != ExternalAuthoringToolKind::Unity) {
        return Fail("tool kind not parsed");
    }

    if (harness.result.runtime_asset_input_count != 2U) {
        return Fail("runtime asset input count mismatch");
    }

    if (harness.result.dependency_count != 1U) {
        return Fail("dependency graph count mismatch");
    }

    if (!harness.result.emitted_import_cook_request) {
        return Fail("import cook request not emitted");
    }

    if (harness.rows[0].target_kind != RuntimeAssetFileKind::Mesh) {
        return Fail("mesh row kind mismatch");
    }

    if (std::string_view(harness.rows[0].payload_path) != MESH_PAYLOAD_PATH) {
        return Fail("mesh payload path mismatch");
    }

    if (harness.rows[0].runtime_asset_source.path != harness.rows[0].payload_path) {
        return Fail("runtime asset source path not row-owned");
    }

    if (harness.rows[1].target_kind != RuntimeAssetFileKind::Scene) {
        return Fail("scene row kind mismatch");
    }

    if (harness.rows[1].dependency_count != 1U) {
        return Fail("scene dependency count mismatch");
    }

    if (harness.command.command !=
        RuntimeAssetImportCookCommandKind::GenerateDeterministicDiskFixture) {
        return Fail("import cook command kind mismatch");
    }

    RuntimeAssetImportCookCommandResult command_result{};
    const RuntimeAssetDataStatus command_status =
        ExecuteRuntimeAssetImportCookCommand(harness.command, &command_result);
    if (command_status != RuntimeAssetDataStatus::Success) {
        return Fail("runtime asset import cook command failed");
    }

    if (command_result.fixture.source_file_count !=
        yuengine::runtimeasset::RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT) {
        return Fail("source fixture count mismatch");
    }

    if (!command_result.fixture.wrote_to_disk) {
        return Fail("import cook command did not write through file vfs");
    }

    return 0;
}

int ExternalAuthoringBridgeRejectsMissingPayloadWithoutMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot(TEST_MISSING_PAYLOAD), &table)) {
        return 1;
    }

    if (!WriteManifest(table, AcceptedManifest())) {
        return Fail("write manifest failed");
    }

    BridgeHarness harness{};
    SetSentinel(&harness);
    const ExternalAuthoringBridgeRequest request = MakeRequest(table, harness);
    const ExternalAuthoringBridgeStatus status =
        BuildExternalAuthoringRuntimeAssetImportBridge(request, &harness.result);
    if (status != ExternalAuthoringBridgeStatus::MissingPayload) {
        return Fail("missing payload status mismatch");
    }

    if (harness.result.blocked_layer != ExternalAuthoringBridgeBlockedLayer::Payload) {
        return Fail("missing payload layer mismatch");
    }

    if (!SentinelUnchanged(harness)) {
        return Fail("missing payload mutated output");
    }

    return 0;
}

int ExternalAuthoringBridgeRejectsInvalidDependencyWithoutMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot(TEST_INVALID_DEPENDENCY), &table)) {
        return 1;
    }

    if (!WriteAcceptedPayloads(table)) {
        return Fail("write accepted payloads failed");
    }

    const std::string manifest = ReplaceFirst(AcceptedManifest(), "entry.1.dependency.0=1001", "entry.1.dependency.0=9999");
    if (!WriteManifest(table, manifest)) {
        return Fail("write manifest failed");
    }

    BridgeHarness harness{};
    SetSentinel(&harness);
    const ExternalAuthoringBridgeRequest request = MakeRequest(table, harness);
    const ExternalAuthoringBridgeStatus status =
        BuildExternalAuthoringRuntimeAssetImportBridge(request, &harness.result);
    if (status != ExternalAuthoringBridgeStatus::InvalidDependencyGraph) {
        return Fail("invalid dependency status mismatch");
    }

    if (harness.result.blocked_layer != ExternalAuthoringBridgeBlockedLayer::DependencyGraph) {
        return Fail("invalid dependency layer mismatch");
    }

    if (harness.result.payload_read_count != 0U) {
        return Fail("invalid dependency read payloads");
    }

    if (!SentinelUnchanged(harness)) {
        return Fail("invalid dependency mutated output");
    }

    return 0;
}

int ExternalAuthoringBridgeRejectsUnsupportedFeatureWithoutMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot(TEST_UNSUPPORTED_FEATURE), &table)) {
        return 1;
    }

    if (!WriteAcceptedPayloads(table)) {
        return Fail("write accepted payloads failed");
    }

    const std::string manifest = ReplaceFirst(AcceptedManifest(), "unsupported_feature_count=0", "unsupported_feature_count=1");
    if (!WriteManifest(table, manifest)) {
        return Fail("write manifest failed");
    }

    BridgeHarness harness{};
    SetSentinel(&harness);
    const ExternalAuthoringBridgeRequest request = MakeRequest(table, harness);
    const ExternalAuthoringBridgeStatus status =
        BuildExternalAuthoringRuntimeAssetImportBridge(request, &harness.result);
    if (status != ExternalAuthoringBridgeStatus::UnsupportedFeature) {
        return Fail("unsupported feature status mismatch");
    }

    if (harness.result.blocked_layer != ExternalAuthoringBridgeBlockedLayer::MappingPolicy) {
        return Fail("unsupported feature layer mismatch");
    }

    if (!SentinelUnchanged(harness)) {
        return Fail("unsupported feature mutated output");
    }

    return 0;
}

int ExternalAuthoringBridgeRejectsUnsupportedMappingPolicyWithoutMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot(TEST_UNSUPPORTED_MAPPING), &table)) {
        return 1;
    }

    if (!WriteAcceptedPayloads(table)) {
        return Fail("write accepted payloads failed");
    }

    const std::string manifest = ReplaceFirst(AcceptedManifest(), "handedness=right", "handedness=left");
    if (!WriteManifest(table, manifest)) {
        return Fail("write manifest failed");
    }

    BridgeHarness harness{};
    SetSentinel(&harness);
    const ExternalAuthoringBridgeRequest request = MakeRequest(table, harness);
    const ExternalAuthoringBridgeStatus status =
        BuildExternalAuthoringRuntimeAssetImportBridge(request, &harness.result);
    if (status != ExternalAuthoringBridgeStatus::UnsupportedCoordinatePolicy) {
        return Fail("unsupported mapping status mismatch");
    }

    if (!harness.result.parsed_manifest) {
        return Fail("unsupported mapping did not parse manifest");
    }

    if (harness.result.payload_read_count != 0U) {
        return Fail("unsupported mapping read payloads");
    }

    if (!SentinelUnchanged(harness)) {
        return Fail("unsupported mapping mutated output");
    }

    return 0;
}

int ExternalAuthoringBridgeRejectsOutputCapacityWithoutMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot(TEST_CAPACITY), &table)) {
        return 1;
    }

    if (!WriteAcceptedPayloads(table)) {
        return Fail("write accepted payloads failed");
    }

    if (!WriteManifest(table, AcceptedManifest())) {
        return Fail("write manifest failed");
    }

    BridgeHarness harness{};
    SetSentinel(&harness);
    ExternalAuthoringBridgeRequest request = MakeRequest(table, harness);
    request.runtime_asset_inputs =
        std::span<ExternalAuthoringRuntimeAssetInputRow>(harness.rows.data(), 1U);
    const ExternalAuthoringBridgeStatus status =
        BuildExternalAuthoringRuntimeAssetImportBridge(request, &harness.result);
    if (status != ExternalAuthoringBridgeStatus::OutputCapacityExceeded) {
        return Fail("capacity status mismatch");
    }

    if (harness.result.blocked_layer != ExternalAuthoringBridgeBlockedLayer::RuntimeAssetInput) {
        return Fail("capacity layer mismatch");
    }

    if (harness.result.payload_read_count != 0U) {
        return Fail("capacity read payloads");
    }

    if (!SentinelUnchanged(harness)) {
        return Fail("capacity mutated output");
    }

    return 0;
}

const std::unordered_map<std::string_view, TestFunction> TESTS = {
    {TEST_ACCEPTED, ExternalAuthoringBridgeAcceptsManifestAndEmitsRuntimeAssetImportCookInputs},
    {TEST_MISSING_PAYLOAD, ExternalAuthoringBridgeRejectsMissingPayloadWithoutMutation},
    {TEST_INVALID_DEPENDENCY, ExternalAuthoringBridgeRejectsInvalidDependencyWithoutMutation},
    {TEST_UNSUPPORTED_FEATURE, ExternalAuthoringBridgeRejectsUnsupportedFeatureWithoutMutation},
    {TEST_UNSUPPORTED_MAPPING, ExternalAuthoringBridgeRejectsUnsupportedMappingPolicyWithoutMutation},
    {TEST_CAPACITY, ExternalAuthoringBridgeRejectsOutputCapacityWithoutMutation},
};
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::string_view test_name(argv[1]);
    const auto test = TESTS.find(test_name);
    if (test == TESTS.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test->second();
}
