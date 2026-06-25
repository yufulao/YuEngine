// 模块: Tests ResourceBrowser
// 文件: Tests/ResourceBrowser/ResourceBrowserDiagnosticsTests.cpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "YuEngine/Asset/AssetManager.h"
#include "YuEngine/File/FileWriteRequest.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/ResourceBrowser/ResourceBrowserDiagnostics.h"
#include "YuEngine/ResourceBrowser/ResourceBrowserSurface.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"

namespace {
using yuengine::asset::AssetManager;
using yuengine::asset::AssetSnapshot;
using yuengine::asset::AssetTypeId;
using yuengine::file::FileStatus;
using yuengine::file::FileWriteRequest;
using yuengine::file::MountId;
using yuengine::file::MountTable;
using yuengine::file::VirtualPath;
using yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceSnapshot;
using yuengine::resource::ResourceTypeId;
using yuengine::resourcebrowser::BuildResourceBrowserRuntimeAssetDiagnostics;
using yuengine::resourcebrowser::ResourceBrowserDependencyState;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticCode;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticRecord;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticsRequest;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticsResult;
using yuengine::resourcebrowser::ResourceBrowserDiagnosticsStatus;
using yuengine::resourcebrowser::ResourceBrowserImportSettings;
using yuengine::resourcebrowser::ResourceBrowserResourceEntry;
using yuengine::resourcebrowser::BuildResourceBrowserNativeSurface;
using yuengine::resourcebrowser::ResourceBrowserSurfaceDocumentKind;
using yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState;
using yuengine::resourcebrowser::ResourceBrowserSurfaceRequest;
using yuengine::resourcebrowser::ResourceBrowserSurfaceResult;
using yuengine::resourcebrowser::ResourceBrowserSurfaceRow;
using yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionRequest;
using yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionResult;
using yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionStatus;
using yuengine::resourcebrowser::ResourceBrowserSurfaceSettingValidationCode;
using yuengine::resourcebrowser::ResourceBrowserSurfaceStatus;
using yuengine::resourcebrowser::BuildResourceBrowserVisibleWorkflowSurface;
using yuengine::resourcebrowser::BuildResourceBrowserDepthWorkflowSurface;
using yuengine::resourcebrowser::ResourceBrowserAssetManagerGap;
using yuengine::resourcebrowser::ResourceBrowserAssetManagerGapRow;
using yuengine::resourcebrowser::ResourceBrowserDepthCatalogRow;
using yuengine::resourcebrowser::ResourceBrowserDepthSelectionLedgerRecord;
using yuengine::resourcebrowser::ResourceBrowserDepthWorkflowRequest;
using yuengine::resourcebrowser::ResourceBrowserDepthWorkflowResult;
using yuengine::resourcebrowser::ResourceBrowserDepthWorkflowStatus;
using yuengine::resourcebrowser::ResourceBrowserImporterBoundaryRow;
using yuengine::resourcebrowser::ResourceBrowserImporterReadiness;
using yuengine::resourcebrowser::ResourceBrowserSourceBoundary;
using yuengine::resourcebrowser::ResourceBrowserVisibleDiagnosticRow;
using yuengine::resourcebrowser::ResourceBrowserVisibleImportSettingRow;
using yuengine::resourcebrowser::ResourceBrowserVisibleSelectionLedgerRecord;
using yuengine::resourcebrowser::ResourceBrowserVisibleWorkflowRequest;
using yuengine::resourcebrowser::ResourceBrowserVisibleWorkflowResult;
using yuengine::resourcebrowser::ResourceBrowserVisibleWorkflowStatus;
using yuengine::runtimeasset::LoadRuntimeAssetDataGraph;
using yuengine::runtimeasset::RuntimeAssetDataStatus;
using yuengine::runtimeasset::RuntimeAssetFileDesc;
using yuengine::runtimeasset::RuntimeAssetFileKind;
using yuengine::runtimeasset::RuntimeAssetGraphLoadRequest;
using yuengine::runtimeasset::RuntimeAssetGraphLoadResult;
using yuengine::runtimeasset::RuntimeAssetLoadedFile;
using yuengine::runtimeasset::RuntimeAssetSceneCameraRecord;
using yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using yuengine::runtimeasset::RuntimeAssetSceneLoaderOutput;
using yuengine::runtimeasset::RuntimeAssetSceneResourceRef;
using yuengine::runtimeasset::RuntimeAssetSceneTransformOutputRecord;

constexpr const char *MOUNT_ID = "resource-browser";
constexpr std::uint32_t RESOURCE_TYPE_MESH = 101U;
constexpr std::uint32_t RESOURCE_TYPE_MATERIAL = 102U;
constexpr std::uint32_t RESOURCE_TYPE_TEXTURE = 103U;
constexpr std::uint32_t RESOURCE_TYPE_SHADER = 104U;
constexpr std::uint32_t RESOURCE_TYPE_SCENE = 105U;
constexpr std::uint32_t RESOURCE_TYPE_ANIMATION = 106U;
constexpr std::uint32_t ASSET_TYPE_MESH = 201U;
constexpr std::uint32_t ASSET_TYPE_MATERIAL = 202U;
constexpr std::uint32_t ASSET_TYPE_TEXTURE = 203U;
constexpr std::uint32_t ASSET_TYPE_SHADER = 204U;
constexpr std::uint32_t ASSET_TYPE_SCENE = 205U;
constexpr std::uint32_t ASSET_TYPE_ANIMATION = 206U;
constexpr const char *SCENE_PATH = "Scene/Main.yuscene";
constexpr std::size_t FIXTURE_FILE_COUNT = 9U;

struct FixtureFile final {
    RuntimeAssetFileDesc desc{};
    const char *bytes = nullptr;
};

struct LoadedGraph final {
    std::array<RuntimeAssetLoadedFile, FIXTURE_FILE_COUNT> assets{};
    std::array<RuntimeAssetSceneResourceRef, FIXTURE_FILE_COUNT> scene_resource_refs{};
    std::array<RuntimeAssetSceneCameraRecord, 1U> scene_cameras{};
    std::array<RuntimeAssetSceneEntityRecord, 3U> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, 3U> scene_transforms{};
    RuntimeAssetSceneLoaderOutput scene_output{};
    RuntimeAssetGraphLoadResult result{};
};

using TestFunction = int (*)();

constexpr const char *TEST_ENTRIES_FROM_RUNTIME_PATH =
    "ResourceBrowserDiagnostics_EntriesComeFromRuntimeAssetFileResourceAssetPath";
constexpr const char *TEST_FAILURE_CLASSIFICATION =
    "ResourceBrowserDiagnostics_ClassifiesMissingTypeMismatchStaleSchemaAndHash";
constexpr const char *TEST_CAPACITY_BUDGET_READONLY =
    "ResourceBrowserDiagnostics_ClassifiesCapacityAndBudgetWithoutResourceAssetMutation";
constexpr const char *TEST_SURFACE_ROWS =
    "ResourceBrowserSurface_BuildsRowsWithStatusAndPreviewEligibility";
constexpr const char *TEST_SURFACE_SUFFIX_BOUNDARY =
    "ResourceBrowserSurface_DoesNotUseLocatorSuffixAsTypeTruth";
constexpr const char *TEST_SURFACE_CAPACITY =
    "ResourceBrowserSurface_RejectsSmallOutputWithoutPartialRows";
constexpr const char *TEST_SURFACE_VALID_SELECTION =
    "ResourceBrowserSurface_SelectionWorkflowAcceptsValidSelection";
constexpr const char *TEST_SURFACE_INVALID_SETTING =
    "ResourceBrowserSurface_SelectionWorkflowRejectsInvalidSetting";
constexpr const char *TEST_SURFACE_PREVIEW_GATING =
    "ResourceBrowserSurface_DiagnosticBlocksPreviewEligibility";
constexpr const char *TEST_SURFACE_SELECTION_NO_MUTATION =
    "ResourceBrowserSurface_SelectionValidationDoesNotMutateResourceAssetMapping";
constexpr const char *TEST_VISIBLE_IMPORT_ROWS =
    "ResourceBrowserVisibleWorkflow_BuildsImportSettingAndPreviewRows";
constexpr const char *TEST_VISIBLE_SELECTION_COMMIT =
    "ResourceBrowserVisibleWorkflow_CommitsSelectionForPreviewHost";
constexpr const char *TEST_VISIBLE_DIAGNOSTIC_BLOCK =
    "ResourceBrowserVisibleWorkflow_DiagnosticBlockRejectsPreview";
constexpr const char *TEST_VISIBLE_INVALID_SETTING_NO_MUTATION =
    "ResourceBrowserVisibleWorkflow_InvalidSettingDoesNotMutateOutputs";
constexpr const char *TEST_DEPTH_GAP_ROWS =
    "ResourceBrowserDepthWorkflow_EmitsCatalogImporterAndAssetGaps";
constexpr const char *TEST_DEPTH_ORIGINAL_PACKAGE =
    "ResourceBrowserDepthWorkflow_BlocksOriginalPackageBoundary";
constexpr const char *TEST_DEPTH_EXTERNAL_IMPORT =
    "ResourceBrowserDepthWorkflow_BlocksExternalImportBoundary";
constexpr const char *TEST_DEPTH_MISSING_RUNTIME_ASSET =
    "ResourceBrowserDepthWorkflow_ReportsMissingRuntimeAssetLoadAndAssetRecordGaps";
constexpr const char *TEST_DEPTH_INVALID_SETTING_NO_MUTATION =
    "ResourceBrowserDepthWorkflow_InvalidSettingDoesNotMutateOutputs";

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
    root /= "YuEngineResourceBrowserDiagnosticsTests";
    root /= std::string(test_name);
    return root;
}

std::vector<std::uint8_t> BytesFromString(const std::string &text) {
    return std::vector<std::uint8_t>(text.begin(), text.end());
}

std::array<FixtureFile, FIXTURE_FILE_COUNT> CanonicalFiles() {
    return std::array<FixtureFile, FIXTURE_FILE_COUNT>{
        FixtureFile{
            RuntimeAssetFileDesc{
                "Mesh/Cube.yumesh",
                RuntimeAssetFileKind::Mesh,
                ResourceTypeId{RESOURCE_TYPE_MESH},
                AssetTypeId{ASSET_TYPE_MESH},
                1001U},
            "YUASSET MESH 1\nschema=rav0-source\nid=cube_mesh\nkind=cube\nvertices=24\nindices=36\nbounds=-1,-1,-1,1,1,1\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Mesh/Cylinder.yumesh",
                RuntimeAssetFileKind::Mesh,
                ResourceTypeId{RESOURCE_TYPE_MESH},
                AssetTypeId{ASSET_TYPE_MESH},
                1002U},
            "YUASSET MESH 1\nschema=rav0-source\nid=cylinder_mesh\nkind=cylinder\nvertices=18\nindices=96\nbounds=-1,-1,-1,1,1,1\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Mesh/Cone.yumesh",
                RuntimeAssetFileKind::Mesh,
                ResourceTypeId{RESOURCE_TYPE_MESH},
                AssetTypeId{ASSET_TYPE_MESH},
                1003U},
            "YUASSET MESH 1\nschema=rav0-source\nid=cone_mesh\nkind=cone\nvertices=10\nindices=48\nbounds=-1,-1,-1,1,1,1\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Material/Shared.yumat",
                RuntimeAssetFileKind::Material,
                ResourceTypeId{RESOURCE_TYPE_MATERIAL},
                AssetTypeId{ASSET_TYPE_MATERIAL},
                2001U},
            "YUASSET MATERIAL 1\nschema=rav0-source\nid=shared_material\nshader=Shader/RuntimeProgram.yuprogram\ntexture0=Texture/Albedo.yutex\ntexture1=Texture/Normal.yutex\ntexture2=Texture/Mask.yutex\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Texture/Albedo.yutex",
                RuntimeAssetFileKind::Texture,
                ResourceTypeId{RESOURCE_TYPE_TEXTURE},
                AssetTypeId{ASSET_TYPE_TEXTURE},
                3001U},
            "YUASSET TEXTURE 1\nschema=rav0-source\nid=albedo\nformat=rgba8\nextent=2x2\npayload=checker\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Texture/Normal.yutex",
                RuntimeAssetFileKind::Texture,
                ResourceTypeId{RESOURCE_TYPE_TEXTURE},
                AssetTypeId{ASSET_TYPE_TEXTURE},
                3002U},
            "YUASSET TEXTURE 1\nschema=rav0-source\nid=normal\nformat=rgba8\nextent=2x2\npayload=normal\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Texture/Mask.yutex",
                RuntimeAssetFileKind::Texture,
                ResourceTypeId{RESOURCE_TYPE_TEXTURE},
                AssetTypeId{ASSET_TYPE_TEXTURE},
                3003U},
            "YUASSET TEXTURE 1\nschema=rav0-source\nid=mask\nformat=rgba8\nextent=2x2\npayload=mask\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Shader/RuntimeProgram.yuprogram",
                RuntimeAssetFileKind::Shader,
                ResourceTypeId{RESOURCE_TYPE_SHADER},
                AssetTypeId{ASSET_TYPE_SHADER},
                4001U},
            "YUASSET SHADER 1\nschema=rav0-source\nid=runtime_program\nstage_vs=bytecode:runtime_program_vs\nstage_ps=bytecode:runtime_program_ps\ninput=layout:position,color\ntextures=3\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Animation/Spin.yuanim",
                RuntimeAssetFileKind::Animation,
                ResourceTypeId{RESOURCE_TYPE_ANIMATION},
                AssetTypeId{ASSET_TYPE_ANIMATION},
                5001U},
            "YUASSET ANIMATION 1\nschema=rav0-source\nid=spin\nclip=1\nduration=1\ntarget=scene_entity:101\ntrack=transform:rotation_y\nkey0=0:0\nkey1=1:1\ntracks=1\nsample_rate=30\n"}};
}

std::string SceneBytes() {
    return std::string(
        "YUASSET SCENE 1\n"
        "schema=rav0-source\n"
        "id=scene\n"
        "m0=Mesh/Cube.yumesh\n"
        "m1=Mesh/Cylinder.yumesh\n"
        "m2=Mesh/Cone.yumesh\n"
        "mat=Material/Shared.yumat\n"
        "t0=Texture/Albedo.yutex\n"
        "prog=Shader/RuntimeProgram.yuprogram\n"
        "anim=Animation/Spin.yuanim\n"
        "cam=camera:orbit\n"
        "e0=101:-2,0,0\n"
        "e1=102:0,0,0\n"
        "e2=103:2,0,0\n");
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
    if (table.RegisterLooseMount(MountId(MOUNT_ID), root) != FileStatus::Success) {
        return FailStep("register loose mount failed");
    }

    *out_table = table;
    return true;
}

bool WriteCanonicalFixture(MountTable &table) {
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    for (const FixtureFile &file : files) {
        if (!WriteBytes(table, file.desc.path, BytesFromString(file.bytes))) {
            return false;
        }
    }

    return WriteBytes(table, SCENE_PATH, BytesFromString(SceneBytes()));
}

std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> CanonicalDescs() {
    const std::array<FixtureFile, FIXTURE_FILE_COUNT> files = CanonicalFiles();
    std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        descs[index] = files[index].desc;
    }

    return descs;
}

bool LoadCanonicalGraph(
    MountTable &table,
    ResourceRegistry &registry,
    AssetManager &manager,
    LoadedGraph *out_graph) {
    if (out_graph == nullptr) {
        return false;
    }

    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs = CanonicalDescs();
    RuntimeAssetGraphLoadRequest request{};
    request.mount_table = &table;
    request.mount = MountId(MOUNT_ID);
    request.scene_path = VirtualPath(SCENE_PATH);
    request.scene_resource_type = ResourceTypeId{RESOURCE_TYPE_SCENE};
    request.scene_asset_type = AssetTypeId{ASSET_TYPE_SCENE};
    request.scene_stable_id = 6001U;
    request.files = descs.data();
    request.file_count = static_cast<std::uint32_t>(descs.size());
    request.resource_registry = &registry;
    request.asset_manager = &manager;
    request.loaded_files = out_graph->assets.data();
    request.loaded_file_capacity = static_cast<std::uint32_t>(out_graph->assets.size());
    request.scene_resource_refs = out_graph->scene_resource_refs.data();
    request.scene_resource_ref_capacity = static_cast<std::uint32_t>(out_graph->scene_resource_refs.size());
    request.scene_cameras = out_graph->scene_cameras.data();
    request.scene_camera_capacity = static_cast<std::uint32_t>(out_graph->scene_cameras.size());
    request.scene_entities = out_graph->scene_entities.data();
    request.scene_entity_capacity = static_cast<std::uint32_t>(out_graph->scene_entities.size());
    request.scene_transforms = out_graph->scene_transforms.data();
    request.scene_transform_capacity = static_cast<std::uint32_t>(out_graph->scene_transforms.size());
    request.scene_output = &out_graph->scene_output;

    const RuntimeAssetDataStatus status = LoadRuntimeAssetDataGraph(request, &out_graph->result);
    return status == RuntimeAssetDataStatus::Success &&
        out_graph->result.loaded_file_count == descs.size();
}

ResourceBrowserDiagnosticsResult BuildDiagnostics(
    MountTable &table,
    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> &descs,
    const LoadedGraph *graph,
    const ResourceRegistry *registry,
    const AssetManager *manager,
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> *entries,
    std::array<ResourceBrowserDiagnosticRecord, 16U> *diagnostics) {
    ResourceBrowserDiagnosticsRequest request{};
    request.mount_table = &table;
    request.mount = MountId(MOUNT_ID);
    request.files = descs.data();
    request.file_count = static_cast<std::uint32_t>(descs.size());
    if (graph != nullptr) {
        request.loaded_files = graph->assets.data();
        request.loaded_file_count = graph->result.loaded_file_count;
    }

    request.resource_registry = registry;
    request.asset_manager = manager;
    request.entries = entries->data();
    request.entry_capacity = static_cast<std::uint32_t>(entries->size());
    request.diagnostics = diagnostics->data();
    request.diagnostic_capacity = static_cast<std::uint32_t>(diagnostics->size());

    ResourceBrowserDiagnosticsResult result{};
    BuildResourceBrowserRuntimeAssetDiagnostics(request, &result);
    return result;
}

bool HasDiagnostic(
    const std::array<ResourceBrowserDiagnosticRecord, 16U> &diagnostics,
    std::uint32_t count,
    ResourceBrowserDiagnosticCode code) {
    for (std::uint32_t index = 0U; index < count; ++index) {
        if (diagnostics[index].code == code) {
            return true;
        }
    }

    return false;
}

bool ResourceSnapshotSame(const ResourceSnapshot &left, const ResourceSnapshot &right) {
    return left.registered_resource_count == right.registered_resource_count &&
        left.dependency_edge_count == right.dependency_edge_count &&
        left.failed_operation_count == right.failed_operation_count &&
        left.load_commit_record_count == right.load_commit_record_count &&
        left.loaded_resource_count == right.loaded_resource_count;
}

bool AssetSnapshotSame(const AssetSnapshot &left, const AssetSnapshot &right) {
    return left.active_asset_count == right.active_asset_count &&
        left.active_dependency_edge_count == right.active_dependency_edge_count &&
        left.failed_operation_count == right.failed_operation_count &&
        left.registered_asset_count == right.registered_asset_count &&
        left.accepted_operation_count == right.accepted_operation_count;
}

bool ResourceHandleSame(
    yuengine::resource::ResourceHandle left,
    yuengine::resource::ResourceHandle right) {
    return left.slot == right.slot && left.generation == right.generation;
}

bool AssetHandleSame(
    yuengine::asset::AssetHandle left,
    yuengine::asset::AssetHandle right) {
    return left.slot == right.slot && left.generation == right.generation;
}

ResourceBrowserSurfaceResult BuildSurfaceRows(
    const std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> &entries,
    const std::array<ResourceBrowserDiagnosticRecord, 16U> &diagnostics,
    std::uint32_t diagnostic_count,
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> *rows) {
    ResourceBrowserSurfaceRequest request{};
    request.entries = std::span<const ResourceBrowserResourceEntry>(entries.data(), entries.size());
    request.diagnostics = std::span<const ResourceBrowserDiagnosticRecord>(diagnostics.data(), diagnostic_count);
    request.rows = std::span<ResourceBrowserSurfaceRow>(rows->data(), rows->size());

    ResourceBrowserSurfaceResult result{};
    BuildResourceBrowserNativeSurface(request, &result);
    return result;
}

int ResourceBrowserDiagnosticsEntriesComeFromRuntimeAssetFileResourceAssetPath() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("EntriesFromRuntimePath"), &table) ||
        !WriteCanonicalFixture(table)) {
        return Fail("failed to create canonical fixture");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadCanonicalGraph(table, registry, manager, &graph)) {
        return Fail("canonical graph load failed");
    }

    const ResourceSnapshot resource_before = registry.Snapshot();
    const AssetSnapshot asset_before = manager.Snapshot();
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs = CanonicalDescs();
    const ResourceBrowserDiagnosticsResult result =
        BuildDiagnostics(table, descs, &graph, &registry, &manager, &entries, &diagnostics);
    const ResourceSnapshot resource_after = registry.Snapshot();
    const AssetSnapshot asset_after = manager.Snapshot();

    if (result.status != ResourceBrowserDiagnosticsStatus::Success ||
        result.entry_count != FIXTURE_FILE_COUNT ||
        result.diagnostic_count != 0U ||
        !result.used_file_vfs ||
        !result.observed_resource_registry ||
        !result.observed_asset_manager ||
        result.mutated_runtime_state ||
        !ResourceSnapshotSame(resource_before, resource_after) ||
        !AssetSnapshotSame(asset_before, asset_after)) {
        return Fail("resource browser success diagnostics did not preserve readonly runtime state");
    }

    for (std::size_t index = 0U; index < entries.size(); ++index) {
        if (!entries[index].from_file_vfs ||
            !entries[index].from_runtime_asset_validation ||
            !entries[index].from_runtime_asset_load ||
            !entries[index].from_resource_registry ||
            !entries[index].from_asset_record ||
            entries[index].dependency_state != ResourceBrowserDependencyState::Ready ||
            !entries[index].resource.IsValid() ||
            !entries[index].asset.IsValid()) {
            return Fail("resource browser entry did not expose loaded runtime path provenance");
        }
    }

    return 0;
}

int ResourceBrowserDiagnosticsClassifiesMissingTypeMismatchStaleSchemaAndHash() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("FailureClassification"), &table)) {
        return Fail("failed to create failure fixture mount");
    }

    constexpr std::size_t FAILURE_FILE_COUNT = 5U;
    const std::array<FixtureFile, FAILURE_FILE_COUNT> files{{
        FixtureFile{
            RuntimeAssetFileDesc{
                "Material/Missing.yumat",
                RuntimeAssetFileKind::Material,
                ResourceTypeId{RESOURCE_TYPE_MATERIAL},
                AssetTypeId{ASSET_TYPE_MATERIAL},
                2001U},
            "YUASSET MATERIAL 1\nschema=rav0-source\nid=missing_material\nshader=Shader/P.yuprogram\ntexture0=Texture/A.yutex\ntexture1=Texture/B.yutex\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Texture/TypeMismatch.yutex",
                RuntimeAssetFileKind::Texture,
                ResourceTypeId{RESOURCE_TYPE_TEXTURE},
                AssetTypeId{ASSET_TYPE_TEXTURE},
                3001U},
            "YUASSET TEXTURE 1\nschema=rav0-source\nid=bad_format\nformat=bc1\nextent=2x2\npayload=checker\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Mesh/StaleSchema.yumesh",
                RuntimeAssetFileKind::Mesh,
                ResourceTypeId{RESOURCE_TYPE_MESH},
                AssetTypeId{ASSET_TYPE_MESH},
                1001U},
            "YUASSET MESH 1\nschema=rav-old-source\nid=old_mesh\nkind=cube\nvertices=24\nindices=36\nbounds=-1,-1,-1,1,1,1\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Texture/StaleHash.yutex",
                RuntimeAssetFileKind::Texture,
                ResourceTypeId{RESOURCE_TYPE_TEXTURE},
                AssetTypeId{ASSET_TYPE_TEXTURE},
                3002U},
            "YUCOOKED TEXTURE 1\nschema=rav1-cooked\nid=bad_hash\nkind=texture\nsourceHash=1\npayloadHash=2\ndependencyTable=0\nrecordTable=1\nrecordBytes=64\npayloadBytes=7\npayloadAlign=4\nformat=rgba8\nextent=2x2\npayload=checker\n"},
        FixtureFile{
            RuntimeAssetFileDesc{
                "Shader/Unsupported.yuprogram",
                RuntimeAssetFileKind::Shader,
                ResourceTypeId{RESOURCE_TYPE_SHADER},
                AssetTypeId{ASSET_TYPE_SHADER},
                4001U},
            "YUASSET SHADER 1\nschema=rav0-source\nid=unsupported_shader\nstage_vs=bytecode:vs\nstage_ps=bytecode:ps\ninput=layout:position,tangent\ntextures=3\n"}}};

    std::array<RuntimeAssetFileDesc, FAILURE_FILE_COUNT> descs{};
    for (std::size_t index = 0U; index < files.size(); ++index) {
        descs[index] = files[index].desc;
        if (!WriteBytes(table, files[index].desc.path, BytesFromString(files[index].bytes))) {
            return Fail("failed to write failure fixture");
        }
    }

    std::array<ResourceBrowserResourceEntry, FAILURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    ResourceBrowserDiagnosticsRequest request{};
    request.mount_table = &table;
    request.mount = MountId(MOUNT_ID);
    request.files = descs.data();
    request.file_count = static_cast<std::uint32_t>(descs.size());
    request.entries = entries.data();
    request.entry_capacity = static_cast<std::uint32_t>(entries.size());
    request.diagnostics = diagnostics.data();
    request.diagnostic_capacity = static_cast<std::uint32_t>(diagnostics.size());

    ResourceBrowserDiagnosticsResult result{};
    BuildResourceBrowserRuntimeAssetDiagnostics(request, &result);
    if (result.status != ResourceBrowserDiagnosticsStatus::Success ||
        result.entry_count != descs.size() ||
        result.diagnostic_count != descs.size() ||
        !HasDiagnostic(diagnostics, result.diagnostic_count, ResourceBrowserDiagnosticCode::MissingDependency) ||
        !HasDiagnostic(diagnostics, result.diagnostic_count, ResourceBrowserDiagnosticCode::TypeMismatch) ||
        !HasDiagnostic(diagnostics, result.diagnostic_count, ResourceBrowserDiagnosticCode::StaleSchema) ||
        !HasDiagnostic(diagnostics, result.diagnostic_count, ResourceBrowserDiagnosticCode::StaleHash) ||
        !HasDiagnostic(diagnostics, result.diagnostic_count, ResourceBrowserDiagnosticCode::Unsupported)) {
        return Fail("resource browser did not classify validation failure diagnostics");
    }

    return 0;
}

int ResourceBrowserDiagnosticsClassifiesCapacityAndBudgetWithoutResourceAssetMutation() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("CapacityBudgetReadonly"), &table)) {
        return Fail("failed to create capacity fixture mount");
    }

    const RuntimeAssetFileDesc budget_desc{
        "Texture/Budget.yutex",
        RuntimeAssetFileKind::Texture,
        ResourceTypeId{RESOURCE_TYPE_TEXTURE},
        AssetTypeId{ASSET_TYPE_TEXTURE},
        3001U};
    const std::string budget_text =
        "YUASSET TEXTURE 1\nschema=rav0-source\nid=too_large\nformat=rgba8\nextent=4097x1\npayload=checker\n";
    if (!WriteBytes(table, budget_desc.path, BytesFromString(budget_text))) {
        return Fail("failed to write budget fixture");
    }

    ResourceRegistry registry;
    AssetManager manager;
    const ResourceSnapshot resource_before = registry.Snapshot();
    const AssetSnapshot asset_before = manager.Snapshot();
    std::array<RuntimeAssetFileDesc, 2U> descs{};
    descs[0U] = budget_desc;
    descs[1U] = budget_desc;

    {
        std::array<ResourceBrowserResourceEntry, 2U> entries{};
        std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
        ResourceBrowserDiagnosticsRequest request{};
        request.mount_table = &table;
        request.mount = MountId(MOUNT_ID);
        request.files = descs.data();
        request.file_count = 1U;
        request.resource_registry = &registry;
        request.asset_manager = &manager;
        request.entries = entries.data();
        request.entry_capacity = static_cast<std::uint32_t>(entries.size());
        request.diagnostics = diagnostics.data();
        request.diagnostic_capacity = static_cast<std::uint32_t>(diagnostics.size());

        ResourceBrowserDiagnosticsResult result{};
        BuildResourceBrowserRuntimeAssetDiagnostics(request, &result);
        if (result.status != ResourceBrowserDiagnosticsStatus::Success ||
            result.diagnostic_count != 1U ||
            diagnostics[0U].code != ResourceBrowserDiagnosticCode::BudgetExceeded ||
            entries[0U].dependency_state != ResourceBrowserDependencyState::BudgetExceeded) {
            return Fail("resource browser did not classify budget failure");
        }
    }

    {
        std::array<ResourceBrowserResourceEntry, 1U> entries{};
        std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
        ResourceBrowserDiagnosticsRequest request{};
        request.mount_table = &table;
        request.mount = MountId(MOUNT_ID);
        request.files = descs.data();
        request.file_count = 2U;
        request.entries = entries.data();
        request.entry_capacity = static_cast<std::uint32_t>(entries.size());
        request.diagnostics = diagnostics.data();
        request.diagnostic_capacity = static_cast<std::uint32_t>(diagnostics.size());

        ResourceBrowserDiagnosticsResult result{};
        BuildResourceBrowserRuntimeAssetDiagnostics(request, &result);
        if (result.status != ResourceBrowserDiagnosticsStatus::OutputCapacityExceeded ||
            result.diagnostic_count != 1U ||
            diagnostics[0U].code != ResourceBrowserDiagnosticCode::CapacityExceeded) {
            return Fail("resource browser did not classify output capacity failure");
        }
    }

    if (!ResourceSnapshotSame(resource_before, registry.Snapshot()) ||
        !AssetSnapshotSame(asset_before, manager.Snapshot())) {
        return Fail("resource browser diagnostics mutated Resource or Asset state");
    }

    return 0;
}

int ResourceBrowserSurfaceBuildsRowsWithStatusAndPreviewEligibility() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("SurfaceRows"), &table) ||
        !WriteCanonicalFixture(table)) {
        return Fail("failed to create surface fixture");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadCanonicalGraph(table, registry, manager, &graph)) {
        return Fail("canonical graph load failed for surface rows");
    }

    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs = CanonicalDescs();
    const ResourceBrowserDiagnosticsResult diagnostics_result =
        BuildDiagnostics(table, descs, &graph, &registry, &manager, &entries, &diagnostics);
    if (diagnostics_result.status != ResourceBrowserDiagnosticsStatus::Success ||
        diagnostics_result.diagnostic_count != 0U) {
        return Fail("surface input diagnostics were not clean");
    }

    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> rows{};
    const ResourceBrowserSurfaceResult surface =
        BuildSurfaceRows(entries, diagnostics, diagnostics_result.diagnostic_count, &rows);
    if (surface.status != ResourceBrowserSurfaceStatus::Success ||
        surface.row_count != FIXTURE_FILE_COUNT ||
        surface.eligible_preview_count != FIXTURE_FILE_COUNT ||
        surface.blocked_preview_count != 0U ||
        surface.used_locator_path_as_type_truth) {
        return Fail("surface did not expose eligible rows from backend diagnostics");
    }

    for (const ResourceBrowserSurfaceRow &row : rows) {
        if (row.locator_path == nullptr ||
            row.declared_kind == RuntimeAssetFileKind::Unknown ||
            row.header_kind != row.declared_kind ||
            row.schema_version != 1U ||
            row.validation_status != RuntimeAssetDataStatus::Success ||
            row.dependency_state != ResourceBrowserDependencyState::Ready ||
            row.preview_state != ResourceBrowserSurfacePreviewState::Eligible ||
            row.preview_document_kind == ResourceBrowserSurfaceDocumentKind::None ||
            !row.has_runtime_loaded_record ||
            !row.has_resource_asset_record ||
            row.locator_path_is_type_truth) {
            return Fail("surface row omitted required native Resource Browser fields");
        }
    }

    return 0;
}

int ResourceBrowserSurfaceDoesNotUseLocatorSuffixAsTypeTruth() {
    std::array<ResourceBrowserResourceEntry, 1U> entries{};
    ResourceBrowserResourceEntry &entry = entries[0U];
    entry.import_settings.source_path = "Texture/LocatorLooksLikeTexture.yutex";
    entry.import_settings.target_kind = RuntimeAssetFileKind::Material;
    entry.import_settings.stable_id = 9001U;
    entry.validation.status = RuntimeAssetDataStatus::TypeMismatch;
    entry.validation.kind = RuntimeAssetFileKind::Texture;
    entry.validation.schema_version = 1U;
    entry.dependency_state = ResourceBrowserDependencyState::TypeMismatch;

    std::array<ResourceBrowserDiagnosticRecord, 1U> diagnostics{};
    diagnostics[0U].code = ResourceBrowserDiagnosticCode::TypeMismatch;
    diagnostics[0U].severity = yuengine::resourcebrowser::ResourceBrowserDiagnosticSeverity::Error;
    diagnostics[0U].phase = yuengine::resourcebrowser::ResourceBrowserDiagnosticPhase::Validate;
    diagnostics[0U].runtime_status = RuntimeAssetDataStatus::TypeMismatch;
    diagnostics[0U].source_path = entry.import_settings.source_path;
    diagnostics[0U].expected_kind = RuntimeAssetFileKind::Material;
    diagnostics[0U].file_index = 0U;

    std::array<ResourceBrowserSurfaceRow, 1U> rows{};
    ResourceBrowserSurfaceRequest request{};
    request.entries = std::span<const ResourceBrowserResourceEntry>(entries.data(), entries.size());
    request.diagnostics = std::span<const ResourceBrowserDiagnosticRecord>(
        diagnostics.data(),
        diagnostics.size());
    request.rows = std::span<ResourceBrowserSurfaceRow>(rows.data(), rows.size());

    ResourceBrowserSurfaceResult result{};
    BuildResourceBrowserNativeSurface(request, &result);
    if (result.status != ResourceBrowserSurfaceStatus::Success ||
        result.row_count != 1U ||
        result.eligible_preview_count != 0U ||
        result.blocked_preview_count != 1U ||
        result.used_locator_path_as_type_truth) {
        return Fail("surface suffix-boundary result was not blocked as expected");
    }

    const ResourceBrowserSurfaceRow &row = rows[0U];
    if (row.declared_kind != RuntimeAssetFileKind::Material ||
        row.header_kind != RuntimeAssetFileKind::Texture ||
        row.preview_state != ResourceBrowserSurfacePreviewState::BlockedByValidation ||
        row.first_diagnostic_code != ResourceBrowserDiagnosticCode::TypeMismatch ||
        !row.has_blocking_diagnostic ||
        row.locator_path_is_type_truth) {
        return Fail("surface used locator suffix instead of declared/header type truth");
    }

    return 0;
}

int ResourceBrowserSurfaceRejectsSmallOutputWithoutPartialRows() {
    std::array<ResourceBrowserResourceEntry, 2U> entries{};
    for (std::uint32_t index = 0U; index < entries.size(); ++index) {
        entries[index].validation.status = RuntimeAssetDataStatus::Success;
        entries[index].validation.kind = RuntimeAssetFileKind::Texture;
        entries[index].dependency_state = ResourceBrowserDependencyState::Ready;
    }

    std::array<ResourceBrowserSurfaceRow, 1U> rows{};
    ResourceBrowserSurfaceRequest request{};
    request.entries = std::span<const ResourceBrowserResourceEntry>(entries.data(), entries.size());
    request.rows = std::span<ResourceBrowserSurfaceRow>(rows.data(), rows.size());

    ResourceBrowserSurfaceResult result{};
    BuildResourceBrowserNativeSurface(request, &result);
    if (result.status != ResourceBrowserSurfaceStatus::OutputCapacityExceeded ||
        result.row_count != 0U ||
        rows[0U].validation_status != RuntimeAssetDataStatus::InvalidArgument) {
        return Fail("surface did not reject small output atomically");
    }

    return 0;
}

int ResourceBrowserSurfaceSelectionWorkflowAcceptsValidSelection() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("SurfaceValidSelection"), &table) ||
        !WriteCanonicalFixture(table)) {
        return Fail("failed to create valid selection fixture");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadCanonicalGraph(table, registry, manager, &graph)) {
        return Fail("canonical graph load failed for valid selection");
    }

    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs = CanonicalDescs();
    const ResourceBrowserDiagnosticsResult diagnostics_result =
        BuildDiagnostics(table, descs, &graph, &registry, &manager, &entries, &diagnostics);
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> rows{};
    const ResourceBrowserSurfaceResult surface =
        BuildSurfaceRows(entries, diagnostics, diagnostics_result.diagnostic_count, &rows);
    if (!surface.Succeeded()) {
        return Fail("surface rows failed for valid selection");
    }

    constexpr std::uint32_t SELECTED_TEXTURE = 4U;
    ResourceBrowserSurfaceSelectionRequest request{};
    request.entries = std::span<const ResourceBrowserResourceEntry>(entries.data(), entries.size());
    request.rows = std::span<const ResourceBrowserSurfaceRow>(rows.data(), rows.size());
    request.diagnostics = std::span<const ResourceBrowserDiagnosticRecord>(
        diagnostics.data(),
        diagnostics_result.diagnostic_count);
    request.selected_index = SELECTED_TEXTURE;
    request.import_settings = entries[SELECTED_TEXTURE].import_settings;
    request.import_settings.expected_source_hash = entries[SELECTED_TEXTURE].validation.source_hash;
    request.validate_import_settings = true;

    ResourceBrowserSurfaceSelectionResult result{};
    ResolveResourceBrowserSurfaceSelection(request, &result);
    if (result.status != ResourceBrowserSurfaceSelectionStatus::Success ||
        !result.state.selected ||
        !result.state.import_settings_valid ||
        !result.state.preview_eligible ||
        result.state.preview_state != ResourceBrowserSurfacePreviewState::Eligible ||
        result.state.preview_document_kind != ResourceBrowserSurfaceDocumentKind::Resource ||
        result.state.setting_validation != ResourceBrowserSurfaceSettingValidationCode::None ||
        result.state.diagnostic_blocks_preview ||
        result.state.used_locator_path_as_type_truth ||
        !result.state.resource_asset_mapping_preserved ||
        !ResourceHandleSame(result.state.resource, entries[SELECTED_TEXTURE].resource) ||
        !AssetHandleSame(result.state.asset, entries[SELECTED_TEXTURE].asset)) {
        return Fail("surface selection workflow did not accept valid selection");
    }

    return 0;
}

int ResourceBrowserSurfaceSelectionWorkflowRejectsInvalidSetting() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("SurfaceInvalidSetting"), &table) ||
        !WriteCanonicalFixture(table)) {
        return Fail("failed to create invalid setting fixture");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadCanonicalGraph(table, registry, manager, &graph)) {
        return Fail("canonical graph load failed for invalid setting");
    }

    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs = CanonicalDescs();
    const ResourceBrowserDiagnosticsResult diagnostics_result =
        BuildDiagnostics(table, descs, &graph, &registry, &manager, &entries, &diagnostics);
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> rows{};
    const ResourceBrowserSurfaceResult surface =
        BuildSurfaceRows(entries, diagnostics, diagnostics_result.diagnostic_count, &rows);
    if (!surface.Succeeded()) {
        return Fail("surface rows failed for invalid setting");
    }

    constexpr std::uint32_t SELECTED_TEXTURE = 4U;
    ResourceBrowserSurfaceSelectionRequest request{};
    request.entries = std::span<const ResourceBrowserResourceEntry>(entries.data(), entries.size());
    request.rows = std::span<const ResourceBrowserSurfaceRow>(rows.data(), rows.size());
    request.selected_index = SELECTED_TEXTURE;
    request.import_settings = entries[SELECTED_TEXTURE].import_settings;
    request.import_settings.target_kind = RuntimeAssetFileKind::Material;
    request.validate_import_settings = true;

    ResourceBrowserSurfaceSelectionResult result{};
    ResolveResourceBrowserSurfaceSelection(request, &result);
    if (result.status != ResourceBrowserSurfaceSelectionStatus::InvalidImportSettings ||
        result.state.import_settings_valid ||
        result.state.preview_eligible ||
        result.state.preview_state != ResourceBrowserSurfacePreviewState::BlockedByValidation ||
        result.state.setting_validation != ResourceBrowserSurfaceSettingValidationCode::TargetKindMismatch ||
        !result.state.resource_asset_mapping_preserved ||
        !ResourceHandleSame(result.state.resource, entries[SELECTED_TEXTURE].resource) ||
        !AssetHandleSame(result.state.asset, entries[SELECTED_TEXTURE].asset)) {
        return Fail("surface selection workflow did not reject invalid structured setting");
    }

    return 0;
}

int ResourceBrowserSurfaceDiagnosticBlocksPreviewEligibility() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("SurfacePreviewGate"), &table) ||
        !WriteCanonicalFixture(table)) {
        return Fail("failed to create preview gate fixture");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadCanonicalGraph(table, registry, manager, &graph)) {
        return Fail("canonical graph load failed for preview gate");
    }

    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs = CanonicalDescs();
    const ResourceBrowserDiagnosticsResult diagnostics_result =
        BuildDiagnostics(table, descs, &graph, &registry, &manager, &entries, &diagnostics);
    if (diagnostics_result.diagnostic_count != 0U) {
        return Fail("preview gate fixture unexpectedly produced diagnostics");
    }

    constexpr std::uint32_t SELECTED_TEXTURE = 4U;
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> baseline_rows{};
    const ResourceBrowserSurfaceResult baseline_surface =
        BuildSurfaceRows(entries, diagnostics, diagnostics_result.diagnostic_count, &baseline_rows);
    if (baseline_surface.status != ResourceBrowserSurfaceStatus::Success ||
        baseline_rows[SELECTED_TEXTURE].preview_state != ResourceBrowserSurfacePreviewState::Eligible) {
        return Fail("preview gate baseline row was not eligible");
    }

    diagnostics[0U].code = ResourceBrowserDiagnosticCode::StaleHash;
    diagnostics[0U].severity = yuengine::resourcebrowser::ResourceBrowserDiagnosticSeverity::Error;
    diagnostics[0U].phase = yuengine::resourcebrowser::ResourceBrowserDiagnosticPhase::Validate;
    diagnostics[0U].runtime_status = RuntimeAssetDataStatus::HashMismatch;
    diagnostics[0U].source_path = entries[SELECTED_TEXTURE].import_settings.source_path;
    diagnostics[0U].expected_kind = RuntimeAssetFileKind::Texture;
    diagnostics[0U].file_index = SELECTED_TEXTURE;

    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> gated_rows{};
    const ResourceBrowserSurfaceResult surface = BuildSurfaceRows(entries, diagnostics, 1U, &gated_rows);
    if (surface.status != ResourceBrowserSurfaceStatus::Success ||
        surface.eligible_preview_count != FIXTURE_FILE_COUNT - 1U ||
        surface.blocked_preview_count != 1U ||
        gated_rows[SELECTED_TEXTURE].preview_state != ResourceBrowserSurfacePreviewState::BlockedByDiagnostic ||
        !gated_rows[SELECTED_TEXTURE].has_blocking_diagnostic) {
        return Fail("surface rows did not convert blocking diagnostic into preview gate");
    }

    ResourceBrowserSurfaceSelectionRequest request{};
    request.entries = std::span<const ResourceBrowserResourceEntry>(entries.data(), entries.size());
    request.rows = std::span<const ResourceBrowserSurfaceRow>(baseline_rows.data(), baseline_rows.size());
    request.diagnostics = std::span<const ResourceBrowserDiagnosticRecord>(diagnostics.data(), 1U);
    request.selected_index = SELECTED_TEXTURE;
    request.import_settings = entries[SELECTED_TEXTURE].import_settings;
    request.validate_import_settings = true;

    ResourceBrowserSurfaceSelectionResult result{};
    ResolveResourceBrowserSurfaceSelection(request, &result);
    if (result.status != ResourceBrowserSurfaceSelectionStatus::PreviewBlocked ||
        !result.state.import_settings_valid ||
        result.state.preview_eligible ||
        !result.state.diagnostic_blocks_preview ||
        result.state.preview_state != ResourceBrowserSurfacePreviewState::BlockedByDiagnostic ||
        result.state.blocking_diagnostic_code != ResourceBrowserDiagnosticCode::StaleHash) {
        return Fail("surface selection did not block preview from diagnostic");
    }

    return 0;
}

int ResourceBrowserSurfaceSelectionValidationDoesNotMutateResourceAssetMapping() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("SurfaceSelectionNoMutation"), &table) ||
        !WriteCanonicalFixture(table)) {
        return Fail("failed to create selection no-mutation fixture");
    }

    ResourceRegistry registry;
    AssetManager manager;
    LoadedGraph graph{};
    if (!LoadCanonicalGraph(table, registry, manager, &graph)) {
        return Fail("canonical graph load failed for selection no-mutation");
    }

    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs = CanonicalDescs();
    const ResourceBrowserDiagnosticsResult diagnostics_result =
        BuildDiagnostics(table, descs, &graph, &registry, &manager, &entries, &diagnostics);
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> rows{};
    const ResourceBrowserSurfaceResult surface =
        BuildSurfaceRows(entries, diagnostics, diagnostics_result.diagnostic_count, &rows);
    if (!surface.Succeeded()) {
        return Fail("surface rows failed for selection no-mutation");
    }

    constexpr std::uint32_t SELECTED_TEXTURE = 4U;
    const auto before_entry_resource = entries[SELECTED_TEXTURE].resource;
    const auto before_entry_asset = entries[SELECTED_TEXTURE].asset;
    const auto before_row_resource = rows[SELECTED_TEXTURE].resource;
    const auto before_row_asset = rows[SELECTED_TEXTURE].asset;
    const ResourceSnapshot resource_before = registry.Snapshot();
    const AssetSnapshot asset_before = manager.Snapshot();

    ResourceBrowserSurfaceSelectionRequest request{};
    request.entries = std::span<const ResourceBrowserResourceEntry>(entries.data(), entries.size());
    request.rows = std::span<const ResourceBrowserSurfaceRow>(rows.data(), rows.size());
    request.selected_index = SELECTED_TEXTURE;
    request.import_settings = entries[SELECTED_TEXTURE].import_settings;
    request.import_settings.expected_source_hash = entries[SELECTED_TEXTURE].validation.source_hash + 1U;
    request.validate_import_settings = true;

    ResourceBrowserSurfaceSelectionResult result{};
    ResolveResourceBrowserSurfaceSelection(request, &result);
    if (result.status != ResourceBrowserSurfaceSelectionStatus::InvalidImportSettings ||
        result.state.setting_validation != ResourceBrowserSurfaceSettingValidationCode::SourceHashMismatch ||
        result.state.mutated_runtime_state ||
        !result.state.resource_asset_mapping_preserved ||
        !ResourceHandleSame(before_entry_resource, entries[SELECTED_TEXTURE].resource) ||
        !AssetHandleSame(before_entry_asset, entries[SELECTED_TEXTURE].asset) ||
        !ResourceHandleSame(before_row_resource, rows[SELECTED_TEXTURE].resource) ||
        !AssetHandleSame(before_row_asset, rows[SELECTED_TEXTURE].asset) ||
        !ResourceSnapshotSame(resource_before, registry.Snapshot()) ||
        !AssetSnapshotSame(asset_before, manager.Snapshot())) {
        return Fail("surface setting validation mutated Resource or Asset mapping");
    }

    return 0;
}

ResourceBrowserVisibleWorkflowResult BuildVisibleWorkflow(
    const std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> &entries,
    const ResourceBrowserDiagnosticRecord *diagnostics,
    std::uint32_t diagnostic_count,
    std::uint32_t selected_index,
    const ResourceBrowserImportSettings &import_settings,
    std::array<ResourceBrowserVisibleImportSettingRow, FIXTURE_FILE_COUNT> *setting_rows,
    std::array<ResourceBrowserVisibleDiagnosticRow, 16U> *diagnostic_rows,
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> *preview_rows,
    std::array<ResourceBrowserVisibleSelectionLedgerRecord, 1U> *ledger) {
    ResourceBrowserVisibleWorkflowRequest request{};
    request.entries = std::span<const ResourceBrowserResourceEntry>(entries.data(), entries.size());
    request.diagnostics = std::span<const ResourceBrowserDiagnosticRecord>(
        diagnostics,
        diagnostic_count);
    request.selected_index = selected_index;
    request.import_settings = import_settings;
    request.validate_import_settings = true;
    request.import_setting_rows =
        std::span<ResourceBrowserVisibleImportSettingRow>(setting_rows->data(), setting_rows->size());
    request.diagnostic_rows =
        std::span<ResourceBrowserVisibleDiagnosticRow>(diagnostic_rows->data(), diagnostic_rows->size());
    request.preview_rows =
        std::span<ResourceBrowserSurfaceRow>(preview_rows->data(), preview_rows->size());
    request.selection_ledger =
        std::span<ResourceBrowserVisibleSelectionLedgerRecord>(ledger->data(), ledger->size());

    ResourceBrowserVisibleWorkflowResult result{};
    BuildResourceBrowserVisibleWorkflowSurface(request, &result);
    return result;
}

ResourceBrowserDepthWorkflowResult BuildDepthWorkflow(
    const std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> &entries,
    const ResourceBrowserDiagnosticRecord *diagnostics,
    std::uint32_t diagnostic_count,
    std::uint32_t selected_index,
    const ResourceBrowserImportSettings &import_settings,
    std::array<ResourceBrowserDepthCatalogRow, FIXTURE_FILE_COUNT> *catalog_rows,
    std::array<ResourceBrowserImporterBoundaryRow, FIXTURE_FILE_COUNT> *importer_rows,
    std::array<ResourceBrowserAssetManagerGapRow, FIXTURE_FILE_COUNT> *asset_gap_rows,
    std::array<ResourceBrowserDepthSelectionLedgerRecord, 1U> *ledger) {
    ResourceBrowserDepthWorkflowRequest request{};
    request.entries = std::span<const ResourceBrowserResourceEntry>(entries.data(), entries.size());
    request.diagnostics = std::span<const ResourceBrowserDiagnosticRecord>(
        diagnostics,
        diagnostic_count);
    request.selected_index = selected_index;
    request.import_settings = import_settings;
    request.validate_import_settings = true;
    request.catalog_rows =
        std::span<ResourceBrowserDepthCatalogRow>(catalog_rows->data(), catalog_rows->size());
    request.importer_rows =
        std::span<ResourceBrowserImporterBoundaryRow>(importer_rows->data(), importer_rows->size());
    request.asset_gap_rows =
        std::span<ResourceBrowserAssetManagerGapRow>(asset_gap_rows->data(), asset_gap_rows->size());
    request.selection_ledger =
        std::span<ResourceBrowserDepthSelectionLedgerRecord>(ledger->data(), ledger->size());

    ResourceBrowserDepthWorkflowResult result{};
    BuildResourceBrowserDepthWorkflowSurface(request, &result);
    return result;
}

bool BuildCanonicalResourceBrowserInputs(
    std::string_view test_name,
    MountTable *table,
    ResourceRegistry *registry,
    AssetManager *manager,
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> *entries,
    std::array<ResourceBrowserDiagnosticRecord, 16U> *diagnostics,
    ResourceBrowserDiagnosticsResult *out_diagnostics_result) {
    if (table == nullptr ||
        registry == nullptr ||
        manager == nullptr ||
        entries == nullptr ||
        diagnostics == nullptr ||
        out_diagnostics_result == nullptr) {
        return false;
    }

    if (!CreateMountedTable(TestRoot(test_name), table) ||
        !WriteCanonicalFixture(*table)) {
        return false;
    }

    LoadedGraph graph{};
    if (!LoadCanonicalGraph(*table, *registry, *manager, &graph)) {
        return false;
    }

    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs = CanonicalDescs();
    *out_diagnostics_result =
        BuildDiagnostics(*table, descs, &graph, registry, manager, entries, diagnostics);
    return out_diagnostics_result->status == ResourceBrowserDiagnosticsStatus::Success;
}

int ResourceBrowserVisibleWorkflowBuildsImportSettingAndPreviewRows() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    ResourceBrowserDiagnosticsResult diagnostics_result{};
    if (!BuildCanonicalResourceBrowserInputs(
            "VisibleWorkflowImportRows",
            &table,
            &registry,
            &manager,
            &entries,
            &diagnostics,
            &diagnostics_result)) {
        return Fail("failed to build visible workflow canonical inputs");
    }

    constexpr std::uint32_t SELECTED_TEXTURE = 4U;
    std::array<ResourceBrowserVisibleImportSettingRow, FIXTURE_FILE_COUNT> setting_rows{};
    std::array<ResourceBrowserVisibleDiagnosticRow, 16U> diagnostic_rows{};
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> preview_rows{};
    std::array<ResourceBrowserVisibleSelectionLedgerRecord, 1U> ledger{};
    ResourceBrowserImportSettings import_settings = entries[SELECTED_TEXTURE].import_settings;
    import_settings.expected_source_hash = entries[SELECTED_TEXTURE].validation.source_hash;

    const ResourceBrowserVisibleWorkflowResult result =
        BuildVisibleWorkflow(
            entries,
            diagnostics.data(),
            diagnostics_result.diagnostic_count,
            SELECTED_TEXTURE,
            import_settings,
            &setting_rows,
            &diagnostic_rows,
            &preview_rows,
            &ledger);
    if (result.status != ResourceBrowserVisibleWorkflowStatus::Success ||
        result.import_setting_row_count != FIXTURE_FILE_COUNT ||
        result.preview_row_count != FIXTURE_FILE_COUNT ||
        result.diagnostic_row_count != 0U ||
        !result.emitted_import_settings ||
        !result.emitted_preview_rows ||
        result.emitted_diagnostics ||
        result.blocked_preview_count != 0U ||
        result.eligible_preview_count != FIXTURE_FILE_COUNT) {
        return Fail("visible workflow did not emit import settings and preview rows");
    }

    const ResourceBrowserVisibleImportSettingRow &setting = setting_rows[SELECTED_TEXTURE];
    const ResourceBrowserSurfaceRow &preview = preview_rows[SELECTED_TEXTURE];
    if (!setting.selected ||
        setting.source_path != entries[SELECTED_TEXTURE].import_settings.source_path ||
        setting.target_kind != RuntimeAssetFileKind::Texture ||
        setting.validation != ResourceBrowserSurfaceSettingValidationCode::None ||
        !setting.source_hash_matches ||
        !setting.target_kind_matches_header ||
        preview.preview_state != ResourceBrowserSurfacePreviewState::Eligible ||
        preview.preview_document_kind != ResourceBrowserSurfaceDocumentKind::Resource ||
        preview.stable_id != entries[SELECTED_TEXTURE].import_settings.stable_id) {
        return Fail("visible workflow rows omitted import setting or preview eligibility fields");
    }

    return 0;
}

int ResourceBrowserVisibleWorkflowCommitsSelectionForPreviewHost() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    ResourceBrowserDiagnosticsResult diagnostics_result{};
    if (!BuildCanonicalResourceBrowserInputs(
            "VisibleWorkflowSelectionCommit",
            &table,
            &registry,
            &manager,
            &entries,
            &diagnostics,
            &diagnostics_result)) {
        return Fail("failed to build selection commit canonical inputs");
    }

    constexpr std::uint32_t SELECTED_MATERIAL = 3U;
    std::array<ResourceBrowserVisibleImportSettingRow, FIXTURE_FILE_COUNT> setting_rows{};
    std::array<ResourceBrowserVisibleDiagnosticRow, 16U> diagnostic_rows{};
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> preview_rows{};
    std::array<ResourceBrowserVisibleSelectionLedgerRecord, 1U> ledger{};
    ResourceBrowserImportSettings import_settings = entries[SELECTED_MATERIAL].import_settings;
    import_settings.expected_source_hash = entries[SELECTED_MATERIAL].validation.source_hash;

    const ResourceBrowserVisibleWorkflowResult result =
        BuildVisibleWorkflow(
            entries,
            diagnostics.data(),
            diagnostics_result.diagnostic_count,
            SELECTED_MATERIAL,
            import_settings,
            &setting_rows,
            &diagnostic_rows,
            &preview_rows,
            &ledger);
    if (!result.Succeeded() ||
        result.selection_status != ResourceBrowserSurfaceSelectionStatus::Success ||
        !result.selection_committed ||
        result.selection_rejected ||
        !result.consumed_preview_host_ready_selection ||
        !result.selection_state.preview_eligible ||
        result.selection_state.preview_state != ResourceBrowserSurfacePreviewState::Eligible ||
        !result.selection_state.resource_asset_mapping_preserved ||
        !ledger[0U].committed_to_preview_host ||
        ledger[0U].rejected_preview_request ||
        !ledger[0U].preview_eligible ||
        !ResourceHandleSame(ledger[0U].resource, entries[SELECTED_MATERIAL].resource) ||
        !AssetHandleSame(ledger[0U].asset, entries[SELECTED_MATERIAL].asset)) {
        return Fail("visible workflow did not commit a Preview Host-ready selection");
    }

    return 0;
}

int ResourceBrowserVisibleWorkflowDiagnosticBlockRejectsPreview() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    ResourceBrowserDiagnosticsResult diagnostics_result{};
    if (!BuildCanonicalResourceBrowserInputs(
            "VisibleWorkflowDiagnosticBlock",
            &table,
            &registry,
            &manager,
            &entries,
            &diagnostics,
            &diagnostics_result)) {
        return Fail("failed to build diagnostic block canonical inputs");
    }

    constexpr std::uint32_t SELECTED_TEXTURE = 4U;
    diagnostics[0U].code = ResourceBrowserDiagnosticCode::StaleHash;
    diagnostics[0U].severity = yuengine::resourcebrowser::ResourceBrowserDiagnosticSeverity::Error;
    diagnostics[0U].phase = yuengine::resourcebrowser::ResourceBrowserDiagnosticPhase::Validate;
    diagnostics[0U].runtime_status = RuntimeAssetDataStatus::HashMismatch;
    diagnostics[0U].source_path = entries[SELECTED_TEXTURE].import_settings.source_path;
    diagnostics[0U].expected_kind = RuntimeAssetFileKind::Texture;
    diagnostics[0U].file_index = SELECTED_TEXTURE;

    std::array<ResourceBrowserVisibleImportSettingRow, FIXTURE_FILE_COUNT> setting_rows{};
    std::array<ResourceBrowserVisibleDiagnosticRow, 16U> diagnostic_rows{};
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> preview_rows{};
    std::array<ResourceBrowserVisibleSelectionLedgerRecord, 1U> ledger{};
    ResourceBrowserImportSettings import_settings = entries[SELECTED_TEXTURE].import_settings;
    import_settings.expected_source_hash = entries[SELECTED_TEXTURE].validation.source_hash;

    const ResourceBrowserVisibleWorkflowResult result =
        BuildVisibleWorkflow(
            entries,
            diagnostics.data(),
            1U,
            SELECTED_TEXTURE,
            import_settings,
            &setting_rows,
            &diagnostic_rows,
            &preview_rows,
            &ledger);
    if (!result.Succeeded() ||
        result.selection_status != ResourceBrowserSurfaceSelectionStatus::PreviewBlocked ||
        result.selection_committed ||
        !result.selection_rejected ||
        result.consumed_preview_host_ready_selection ||
        result.blocked_preview_count != 1U ||
        result.eligible_preview_count != FIXTURE_FILE_COUNT - 1U ||
        preview_rows[SELECTED_TEXTURE].preview_state != ResourceBrowserSurfacePreviewState::BlockedByDiagnostic ||
        !diagnostic_rows[0U].selected ||
        !diagnostic_rows[0U].blocks_preview ||
        !ledger[0U].rejected_preview_request ||
        ledger[0U].committed_to_preview_host ||
        ledger[0U].preview_eligible) {
        return Fail("visible workflow did not reject diagnostic-blocked preview");
    }

    return 0;
}

int ResourceBrowserVisibleWorkflowInvalidSettingDoesNotMutateOutputs() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    ResourceBrowserDiagnosticsResult diagnostics_result{};
    if (!BuildCanonicalResourceBrowserInputs(
            "VisibleWorkflowInvalidSetting",
            &table,
            &registry,
            &manager,
            &entries,
            &diagnostics,
            &diagnostics_result)) {
        return Fail("failed to build invalid setting canonical inputs");
    }

    constexpr std::uint32_t SELECTED_TEXTURE = 4U;
    diagnostics[0U].code = ResourceBrowserDiagnosticCode::Unsupported;
    diagnostics[0U].severity = yuengine::resourcebrowser::ResourceBrowserDiagnosticSeverity::Info;
    diagnostics[0U].phase = yuengine::resourcebrowser::ResourceBrowserDiagnosticPhase::ImportSettings;
    diagnostics[0U].file_index = SELECTED_TEXTURE;

    std::array<ResourceBrowserVisibleImportSettingRow, FIXTURE_FILE_COUNT> setting_rows{};
    std::array<ResourceBrowserVisibleDiagnosticRow, 16U> diagnostic_rows{};
    std::array<ResourceBrowserSurfaceRow, FIXTURE_FILE_COUNT> preview_rows{};
    std::array<ResourceBrowserVisibleSelectionLedgerRecord, 1U> ledger{};
    setting_rows[0U].source_path = "sentinel";
    diagnostic_rows[0U].code = ResourceBrowserDiagnosticCode::BudgetExceeded;
    preview_rows[0U].stable_id = 7007U;
    ledger[0U].stable_id = 8008U;
    ledger[0U].committed_to_preview_host = true;

    ResourceBrowserImportSettings import_settings = entries[SELECTED_TEXTURE].import_settings;
    import_settings.target_kind = RuntimeAssetFileKind::Material;

    const ResourceBrowserVisibleWorkflowResult result =
        BuildVisibleWorkflow(
            entries,
            diagnostics.data(),
            1U,
            SELECTED_TEXTURE,
            import_settings,
            &setting_rows,
            &diagnostic_rows,
            &preview_rows,
            &ledger);
    if (result.status != ResourceBrowserVisibleWorkflowStatus::InvalidImportSettings ||
        result.selection_status != ResourceBrowserSurfaceSelectionStatus::InvalidImportSettings ||
        result.import_setting_row_count != 0U ||
        result.diagnostic_row_count != 0U ||
        result.preview_row_count != 0U ||
        result.selection_ledger_count != 0U ||
        std::string_view(setting_rows[0U].source_path) != std::string_view("sentinel") ||
        diagnostic_rows[0U].code != ResourceBrowserDiagnosticCode::BudgetExceeded ||
        preview_rows[0U].stable_id != 7007U ||
        ledger[0U].stable_id != 8008U ||
        !ledger[0U].committed_to_preview_host) {
        return Fail("visible workflow invalid setting mutated caller outputs");
    }

    return 0;
}

int ResourceBrowserDepthWorkflowEmitsCatalogImporterAndAssetGaps() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    ResourceBrowserDiagnosticsResult diagnostics_result{};
    if (!BuildCanonicalResourceBrowserInputs(
            "DepthWorkflowGapRows",
            &table,
            &registry,
            &manager,
            &entries,
            &diagnostics,
            &diagnostics_result)) {
        return Fail("failed to build depth workflow canonical inputs");
    }

    constexpr std::uint32_t SELECTED_TEXTURE = 4U;
    std::array<ResourceBrowserDepthCatalogRow, FIXTURE_FILE_COUNT> catalog_rows{};
    std::array<ResourceBrowserImporterBoundaryRow, FIXTURE_FILE_COUNT> importer_rows{};
    std::array<ResourceBrowserAssetManagerGapRow, FIXTURE_FILE_COUNT> asset_gap_rows{};
    std::array<ResourceBrowserDepthSelectionLedgerRecord, 1U> ledger{};
    ResourceBrowserImportSettings import_settings = entries[SELECTED_TEXTURE].import_settings;
    import_settings.expected_source_hash = entries[SELECTED_TEXTURE].validation.source_hash;

    const ResourceBrowserDepthWorkflowResult result =
        BuildDepthWorkflow(
            entries,
            diagnostics.data(),
            diagnostics_result.diagnostic_count,
            SELECTED_TEXTURE,
            import_settings,
            &catalog_rows,
            &importer_rows,
            &asset_gap_rows,
            &ledger);
    if (!result.Succeeded() ||
        result.catalog_row_count != FIXTURE_FILE_COUNT ||
        result.importer_row_count != FIXTURE_FILE_COUNT ||
        result.asset_gap_row_count != FIXTURE_FILE_COUNT ||
        result.importer_ready_count != FIXTURE_FILE_COUNT ||
        result.asset_manager_ready_count != FIXTURE_FILE_COUNT ||
        result.preview_request_ready_count != FIXTURE_FILE_COUNT ||
        !result.selection_committed ||
        result.selection_rejected ||
        result.mutated_runtime_state ||
        result.opened_native_window ||
        result.used_web_ui) {
        return Fail("depth workflow did not emit ready catalog/importer/asset gap rows");
    }

    if (!catalog_rows[SELECTED_TEXTURE].selected ||
        std::string_view(catalog_rows[SELECTED_TEXTURE].source_path) !=
            std::string_view(import_settings.source_path) ||
        catalog_rows[SELECTED_TEXTURE].target_kind != import_settings.target_kind ||
        catalog_rows[SELECTED_TEXTURE].source_boundary !=
            ResourceBrowserSourceBoundary::RuntimeAssetSource ||
        catalog_rows[SELECTED_TEXTURE].importer_readiness !=
            ResourceBrowserImporterReadiness::Ready ||
        catalog_rows[SELECTED_TEXTURE].asset_manager_gap !=
            ResourceBrowserAssetManagerGap::None ||
        !catalog_rows[SELECTED_TEXTURE].preview_request_ready ||
        !importer_rows[SELECTED_TEXTURE].importer_ready ||
        !asset_gap_rows[SELECTED_TEXTURE].asset_manager_ready ||
        !ledger[0U].selection_committed ||
        ledger[0U].blocked_by_importer_gap ||
        ledger[0U].blocked_by_asset_manager_gap) {
        return Fail("depth workflow selected ready row mismatch");
    }

    return 0;
}

int ResourceBrowserDepthWorkflowBlocksOriginalPackageBoundary() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    ResourceBrowserDiagnosticsResult diagnostics_result{};
    if (!BuildCanonicalResourceBrowserInputs(
            "DepthWorkflowOriginalPackage",
            &table,
            &registry,
            &manager,
            &entries,
            &diagnostics,
            &diagnostics_result)) {
        return Fail("failed to build original package canonical inputs");
    }

    constexpr std::uint32_t SELECTED_MATERIAL = 3U;
    std::array<ResourceBrowserDepthCatalogRow, FIXTURE_FILE_COUNT> catalog_rows{};
    std::array<ResourceBrowserImporterBoundaryRow, FIXTURE_FILE_COUNT> importer_rows{};
    std::array<ResourceBrowserAssetManagerGapRow, FIXTURE_FILE_COUNT> asset_gap_rows{};
    std::array<ResourceBrowserDepthSelectionLedgerRecord, 1U> ledger{};
    ResourceBrowserImportSettings import_settings = entries[SELECTED_MATERIAL].import_settings;
    import_settings.source_path = "original-package:th075.dat";
    import_settings.expected_source_hash = entries[SELECTED_MATERIAL].validation.source_hash;

    const ResourceBrowserDepthWorkflowResult result =
        BuildDepthWorkflow(
            entries,
            diagnostics.data(),
            diagnostics_result.diagnostic_count,
            SELECTED_MATERIAL,
            import_settings,
            &catalog_rows,
            &importer_rows,
            &asset_gap_rows,
            &ledger);
    if (!result.Succeeded() ||
        result.selection_committed ||
        !result.selection_rejected ||
        result.original_package_boundary_count != 1U ||
        !result.blocked_by_original_package ||
        result.blocked_by_external_import ||
        !result.blocked_by_importer_gap ||
        result.blocked_by_asset_manager_gap) {
        return Fail("depth workflow did not block original package boundary");
    }

    if (catalog_rows[SELECTED_MATERIAL].source_boundary !=
            ResourceBrowserSourceBoundary::OriginalPackageBoundary ||
        std::string_view(catalog_rows[SELECTED_MATERIAL].source_path) !=
            std::string_view("original-package:th075.dat") ||
        catalog_rows[SELECTED_MATERIAL].importer_readiness !=
            ResourceBrowserImporterReadiness::OriginalPackageBoundary ||
        importer_rows[SELECTED_MATERIAL].source_boundary !=
            ResourceBrowserSourceBoundary::OriginalPackageBoundary ||
        std::string_view(importer_rows[SELECTED_MATERIAL].source_path) !=
            std::string_view("original-package:th075.dat") ||
        !importer_rows[SELECTED_MATERIAL].original_package_boundary ||
        importer_rows[SELECTED_MATERIAL].external_import_boundary ||
        ledger[0U].source_boundary !=
            ResourceBrowserSourceBoundary::OriginalPackageBoundary ||
        !ledger[0U].blocked_by_original_package ||
        ledger[0U].preview_request_ready) {
        return Fail("depth workflow original package ledger mismatch");
    }

    return 0;
}

int ResourceBrowserDepthWorkflowBlocksExternalImportBoundary() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    ResourceBrowserDiagnosticsResult diagnostics_result{};
    if (!BuildCanonicalResourceBrowserInputs(
            "DepthWorkflowExternalImport",
            &table,
            &registry,
            &manager,
            &entries,
            &diagnostics,
            &diagnostics_result)) {
        return Fail("failed to build external import canonical inputs");
    }

    constexpr std::uint32_t SELECTED_MESH = 0U;
    std::array<ResourceBrowserDepthCatalogRow, FIXTURE_FILE_COUNT> catalog_rows{};
    std::array<ResourceBrowserImporterBoundaryRow, FIXTURE_FILE_COUNT> importer_rows{};
    std::array<ResourceBrowserAssetManagerGapRow, FIXTURE_FILE_COUNT> asset_gap_rows{};
    std::array<ResourceBrowserDepthSelectionLedgerRecord, 1U> ledger{};
    ResourceBrowserImportSettings import_settings = entries[SELECTED_MESH].import_settings;
    import_settings.source_path = "external-import:Blender/Cube.fbx";
    import_settings.expected_source_hash = entries[SELECTED_MESH].validation.source_hash;

    const ResourceBrowserDepthWorkflowResult result =
        BuildDepthWorkflow(
            entries,
            diagnostics.data(),
            diagnostics_result.diagnostic_count,
            SELECTED_MESH,
            import_settings,
            &catalog_rows,
            &importer_rows,
            &asset_gap_rows,
            &ledger);
    if (!result.Succeeded() ||
        result.selection_committed ||
        !result.selection_rejected ||
        result.external_import_boundary_count != 1U ||
        result.blocked_by_original_package ||
        !result.blocked_by_external_import ||
        !result.blocked_by_importer_gap ||
        result.blocked_by_asset_manager_gap) {
        return Fail("depth workflow did not block external import boundary");
    }

    if (catalog_rows[SELECTED_MESH].source_boundary !=
            ResourceBrowserSourceBoundary::ExternalImportBoundary ||
        std::string_view(catalog_rows[SELECTED_MESH].source_path) !=
            std::string_view("external-import:Blender/Cube.fbx") ||
        catalog_rows[SELECTED_MESH].importer_readiness !=
            ResourceBrowserImporterReadiness::ExternalImportBoundary ||
        importer_rows[SELECTED_MESH].source_boundary !=
            ResourceBrowserSourceBoundary::ExternalImportBoundary ||
        std::string_view(importer_rows[SELECTED_MESH].source_path) !=
            std::string_view("external-import:Blender/Cube.fbx") ||
        importer_rows[SELECTED_MESH].original_package_boundary ||
        !importer_rows[SELECTED_MESH].external_import_boundary ||
        ledger[0U].source_boundary !=
            ResourceBrowserSourceBoundary::ExternalImportBoundary ||
        !ledger[0U].blocked_by_external_import ||
        ledger[0U].preview_request_ready) {
        return Fail("depth workflow external import ledger mismatch");
    }

    return 0;
}

int ResourceBrowserDepthWorkflowReportsMissingRuntimeAssetLoadAndAssetRecordGaps() {
    MountTable table;
    if (!CreateMountedTable(TestRoot("DepthWorkflowMissingRuntimeAsset"), &table) ||
        !WriteCanonicalFixture(table)) {
        return Fail("failed to build missing runtime asset fixture");
    }

    ResourceRegistry registry;
    AssetManager manager;
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    const std::array<RuntimeAssetFileDesc, FIXTURE_FILE_COUNT> descs = CanonicalDescs();
    const ResourceBrowserDiagnosticsResult diagnostics_result =
        BuildDiagnostics(table, descs, nullptr, &registry, &manager, &entries, &diagnostics);

    constexpr std::uint32_t SELECTED_MESH = 0U;
    std::array<ResourceBrowserDepthCatalogRow, FIXTURE_FILE_COUNT> catalog_rows{};
    std::array<ResourceBrowserImporterBoundaryRow, FIXTURE_FILE_COUNT> importer_rows{};
    std::array<ResourceBrowserAssetManagerGapRow, FIXTURE_FILE_COUNT> asset_gap_rows{};
    std::array<ResourceBrowserDepthSelectionLedgerRecord, 1U> ledger{};
    ResourceBrowserImportSettings import_settings = entries[SELECTED_MESH].import_settings;
    import_settings.expected_source_hash = entries[SELECTED_MESH].validation.source_hash;

    const ResourceBrowserDepthWorkflowResult result =
        BuildDepthWorkflow(
            entries,
            diagnostics.data(),
            diagnostics_result.diagnostic_count,
            SELECTED_MESH,
            import_settings,
            &catalog_rows,
            &importer_rows,
            &asset_gap_rows,
            &ledger);
    if (!result.Succeeded() ||
        result.selection_committed ||
        !result.selection_rejected ||
        result.importer_ready_count != FIXTURE_FILE_COUNT ||
        result.asset_manager_ready_count != 0U ||
        result.preview_request_ready_count != 0U ||
        result.blocked_by_importer_gap ||
        !result.blocked_by_asset_manager_gap) {
        return Fail("depth workflow missing runtime asset gap counters mismatch");
    }

    if (catalog_rows[SELECTED_MESH].asset_manager_gap !=
            ResourceBrowserAssetManagerGap::MissingRuntimeLoadRecord ||
        catalog_rows[SELECTED_MESH].preview_request_ready ||
        asset_gap_rows[SELECTED_MESH].has_runtime_loaded_record ||
        asset_gap_rows[SELECTED_MESH].has_resource_registry_record ||
        asset_gap_rows[SELECTED_MESH].has_asset_record ||
        asset_gap_rows[SELECTED_MESH].resource_handle_valid ||
        asset_gap_rows[SELECTED_MESH].asset_handle_valid ||
        ledger[0U].asset_manager_gap !=
            ResourceBrowserAssetManagerGap::MissingRuntimeLoadRecord ||
        !ledger[0U].blocked_by_asset_manager_gap) {
        return Fail("depth workflow missing runtime asset selected row mismatch");
    }

    return 0;
}

int ResourceBrowserDepthWorkflowInvalidSettingDoesNotMutateOutputs() {
    MountTable table;
    ResourceRegistry registry;
    AssetManager manager;
    std::array<ResourceBrowserResourceEntry, FIXTURE_FILE_COUNT> entries{};
    std::array<ResourceBrowserDiagnosticRecord, 16U> diagnostics{};
    ResourceBrowserDiagnosticsResult diagnostics_result{};
    if (!BuildCanonicalResourceBrowserInputs(
            "DepthWorkflowInvalidSetting",
            &table,
            &registry,
            &manager,
            &entries,
            &diagnostics,
            &diagnostics_result)) {
        return Fail("failed to build depth invalid setting inputs");
    }

    constexpr std::uint32_t SELECTED_TEXTURE = 4U;
    std::array<ResourceBrowserDepthCatalogRow, FIXTURE_FILE_COUNT> catalog_rows{};
    std::array<ResourceBrowserImporterBoundaryRow, FIXTURE_FILE_COUNT> importer_rows{};
    std::array<ResourceBrowserAssetManagerGapRow, FIXTURE_FILE_COUNT> asset_gap_rows{};
    std::array<ResourceBrowserDepthSelectionLedgerRecord, 1U> ledger{};
    catalog_rows[0U].stable_id = 7007U;
    catalog_rows[0U].preview_request_ready = true;
    importer_rows[0U].source_path = "sentinel";
    importer_rows[0U].importer_ready = true;
    asset_gap_rows[0U].stable_id = 8008U;
    asset_gap_rows[0U].asset_manager_ready = true;
    ledger[0U].stable_id = 9009U;
    ledger[0U].selection_committed = true;

    ResourceBrowserImportSettings import_settings = entries[SELECTED_TEXTURE].import_settings;
    import_settings.target_kind = RuntimeAssetFileKind::Material;

    const ResourceBrowserDepthWorkflowResult result =
        BuildDepthWorkflow(
            entries,
            diagnostics.data(),
            diagnostics_result.diagnostic_count,
            SELECTED_TEXTURE,
            import_settings,
            &catalog_rows,
            &importer_rows,
            &asset_gap_rows,
            &ledger);
    if (result.status != ResourceBrowserDepthWorkflowStatus::InvalidImportSettings ||
        result.selection_status != ResourceBrowserSurfaceSelectionStatus::InvalidImportSettings ||
        result.catalog_row_count != 0U ||
        result.importer_row_count != 0U ||
        result.asset_gap_row_count != 0U ||
        result.selection_ledger_count != 0U ||
        catalog_rows[0U].stable_id != 7007U ||
        !catalog_rows[0U].preview_request_ready ||
        std::string_view(importer_rows[0U].source_path) != std::string_view("sentinel") ||
        !importer_rows[0U].importer_ready ||
        asset_gap_rows[0U].stable_id != 8008U ||
        !asset_gap_rows[0U].asset_manager_ready ||
        ledger[0U].stable_id != 9009U ||
        !ledger[0U].selection_committed) {
        return Fail("depth workflow invalid setting mutated caller outputs");
    }

    return 0;
}

const std::unordered_map<std::string_view, TestFunction> &Tests() {
    static const std::unordered_map<std::string_view, TestFunction> tests{
        {TEST_ENTRIES_FROM_RUNTIME_PATH, ResourceBrowserDiagnosticsEntriesComeFromRuntimeAssetFileResourceAssetPath},
        {TEST_FAILURE_CLASSIFICATION, ResourceBrowserDiagnosticsClassifiesMissingTypeMismatchStaleSchemaAndHash},
        {TEST_CAPACITY_BUDGET_READONLY, ResourceBrowserDiagnosticsClassifiesCapacityAndBudgetWithoutResourceAssetMutation},
        {TEST_SURFACE_ROWS, ResourceBrowserSurfaceBuildsRowsWithStatusAndPreviewEligibility},
        {TEST_SURFACE_SUFFIX_BOUNDARY, ResourceBrowserSurfaceDoesNotUseLocatorSuffixAsTypeTruth},
        {TEST_SURFACE_CAPACITY, ResourceBrowserSurfaceRejectsSmallOutputWithoutPartialRows},
        {TEST_SURFACE_VALID_SELECTION, ResourceBrowserSurfaceSelectionWorkflowAcceptsValidSelection},
        {TEST_SURFACE_INVALID_SETTING, ResourceBrowserSurfaceSelectionWorkflowRejectsInvalidSetting},
        {TEST_SURFACE_PREVIEW_GATING, ResourceBrowserSurfaceDiagnosticBlocksPreviewEligibility},
        {TEST_SURFACE_SELECTION_NO_MUTATION, ResourceBrowserSurfaceSelectionValidationDoesNotMutateResourceAssetMapping},
        {TEST_VISIBLE_IMPORT_ROWS, ResourceBrowserVisibleWorkflowBuildsImportSettingAndPreviewRows},
        {TEST_VISIBLE_SELECTION_COMMIT, ResourceBrowserVisibleWorkflowCommitsSelectionForPreviewHost},
        {TEST_VISIBLE_DIAGNOSTIC_BLOCK, ResourceBrowserVisibleWorkflowDiagnosticBlockRejectsPreview},
        {TEST_VISIBLE_INVALID_SETTING_NO_MUTATION, ResourceBrowserVisibleWorkflowInvalidSettingDoesNotMutateOutputs},
        {TEST_DEPTH_GAP_ROWS, ResourceBrowserDepthWorkflowEmitsCatalogImporterAndAssetGaps},
        {TEST_DEPTH_ORIGINAL_PACKAGE, ResourceBrowserDepthWorkflowBlocksOriginalPackageBoundary},
        {TEST_DEPTH_EXTERNAL_IMPORT, ResourceBrowserDepthWorkflowBlocksExternalImportBoundary},
        {TEST_DEPTH_MISSING_RUNTIME_ASSET, ResourceBrowserDepthWorkflowReportsMissingRuntimeAssetLoadAndAssetRecordGaps},
        {TEST_DEPTH_INVALID_SETTING_NO_MUTATION, ResourceBrowserDepthWorkflowInvalidSettingDoesNotMutateOutputs},
    };
    return tests;
}

}

int main(int argc, char **argv) {
    if (argc < 2) {
        return Fail("missing test name");
    }

    const std::string_view test_name(argv[1]);
    const auto &tests = Tests();
    const auto found = tests.find(test_name);
    if (found == tests.end()) {
        return Fail("unknown test name");
    }

    return found->second();
}
