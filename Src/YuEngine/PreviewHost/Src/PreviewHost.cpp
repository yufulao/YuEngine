// Module: YuEngine PreviewHost
// File: Src/YuEngine/PreviewHost/Src/PreviewHost.cpp

#include "YuEngine/PreviewHost/PreviewHost.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderScene/RenderSceneRuntimeFrameBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameEntityRequest.h"

namespace yuengine::previewhost {
namespace {
using RuntimeAssetDataStatus = yuengine::runtimeasset::RuntimeAssetDataStatus;
using RuntimeAssetFileKind = yuengine::runtimeasset::RuntimeAssetFileKind;
using RuntimeAssetGraphLoadResult = yuengine::runtimeasset::RuntimeAssetGraphLoadResult;
using RuntimeAssetImportCookMissingLayer = yuengine::runtimeasset::RuntimeAssetImportCookMissingLayer;
using RuntimeAssetLoadedFile = yuengine::runtimeasset::RuntimeAssetLoadedFile;
using RuntimeAssetSceneEntityRecord = yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using RuntimeAssetSceneLoaderOutput = yuengine::runtimeasset::RuntimeAssetSceneLoaderOutput;
using RuntimeAssetSceneResourceRef = yuengine::runtimeasset::RuntimeAssetSceneResourceRef;
using ResourceBrowserDependencyState = yuengine::resourcebrowser::ResourceBrowserDependencyState;
using ResourceBrowserDiagnosticRecord = yuengine::resourcebrowser::ResourceBrowserDiagnosticRecord;
using ResourceBrowserDiagnosticSeverity = yuengine::resourcebrowser::ResourceBrowserDiagnosticSeverity;
using ResourceBrowserResourceEntry = yuengine::resourcebrowser::ResourceBrowserResourceEntry;
using ResourceBrowserSurfaceDocumentKind =
    yuengine::resourcebrowser::ResourceBrowserSurfaceDocumentKind;
using ResourceBrowserSurfacePreviewState =
    yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState;
using ResourceBrowserSurfaceSelectionState =
    yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionState;
using RenderScenePrimitiveGeometryKind = yuengine::renderscene::RenderScenePrimitiveGeometryKind;
using RenderScenePrimitiveGeometryRecord = yuengine::renderscene::RenderScenePrimitiveGeometryRecord;
using RenderSceneRuntimeFrameBuilder = yuengine::renderscene::RenderSceneRuntimeFrameBuilder;
using RenderSceneRuntimeFrameDrawRecord = yuengine::renderscene::RenderSceneRuntimeFrameDrawRecord;
using RenderSceneRuntimeFrameEntityRequest = yuengine::renderscene::RenderSceneRuntimeFrameEntityRequest;
using RenderSceneRuntimeFrameRequest = yuengine::renderscene::RenderSceneRuntimeFrameRequest;
using RenderSceneRuntimeFrameStatus = yuengine::renderscene::RenderSceneRuntimeFrameStatus;
using RenderSceneThreePrimitiveCaptureMissingLayer =
    yuengine::renderscene::RenderSceneThreePrimitiveCaptureMissingLayer;
using RenderSceneThreePrimitiveCaptureRequest =
    yuengine::renderscene::RenderSceneThreePrimitiveCaptureRequest;
using RenderSceneThreePrimitiveCaptureRoute =
    yuengine::renderscene::RenderSceneThreePrimitiveCaptureRoute;
using RenderSceneThreePrimitiveCaptureStatus =
    yuengine::renderscene::RenderSceneThreePrimitiveCaptureStatus;
using RenderSceneThreePrimitiveEntityRequest =
    yuengine::renderscene::RenderSceneThreePrimitiveEntityRequest;

bool IsAssetEqual(
    yuengine::asset::AssetHandle left,
    yuengine::asset::AssetHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool IsResourceEqual(
    yuengine::resource::ResourceHandle left,
    yuengine::resource::ResourceHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool IsPipelineValid(yuengine::rhi::RhiPipelineHandle pipeline) {
    return pipeline.generation != 0U;
}

PreviewHostStatus EmitFailure(
    const PreviewHostFrameRequest &request,
    PreviewHostFrameResult *result,
    PreviewHostStatus status,
    PreviewHostDiagnosticCode code,
    RuntimeAssetDataStatus runtime_status,
    RenderSceneRuntimeFrameStatus frame_status,
    RenderSceneThreePrimitiveCaptureMissingLayer missing_layer,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetFileKind actual_kind,
    std::uint64_t stable_id,
    std::uint32_t loaded_file_index,
    std::uint32_t resource_ref_index,
    std::uint32_t entity_index,
    RuntimeAssetImportCookMissingLayer import_cook_missing_layer =
        RuntimeAssetImportCookMissingLayer::None) {
    if (result == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    result->runtime_asset_status = runtime_status;
    result->import_cook_missing_layer = import_cook_missing_layer;
    result->render_frame.status = frame_status;
    result->capture.first_missing_layer = missing_layer;

    if (request.diagnostics.empty()) {
        result->status = PreviewHostStatus::OutputCapacityExceeded;
        result->diagnostic_count = 0U;
        return result->status;
    }

    PreviewHostDiagnostic diagnostic{};
    diagnostic.code = code;
    diagnostic.status = status;
    diagnostic.runtime_asset_status = runtime_status;
    diagnostic.import_cook_missing_layer = import_cook_missing_layer;
    diagnostic.frame_status = frame_status;
    diagnostic.missing_layer = missing_layer;
    diagnostic.expected_kind = expected_kind;
    diagnostic.actual_kind = actual_kind;
    diagnostic.stable_id = stable_id;
    diagnostic.loaded_file_index = loaded_file_index;
    diagnostic.resource_ref_index = resource_ref_index;
    diagnostic.entity_index = entity_index;
    request.diagnostics[0U] = diagnostic;
    result->diagnostic_count = 1U;
    result->status = status;
    return status;
}

bool IsDecodedPayloadRequired(RuntimeAssetFileKind kind) {
    if (kind == RuntimeAssetFileKind::Mesh) {
        return true;
    }

    if (kind == RuntimeAssetFileKind::Material) {
        return true;
    }

    return kind == RuntimeAssetFileKind::Texture;
}

PreviewHostStatus ValidateGraph(
    const PreviewHostFrameRequest &request,
    PreviewHostFrameResult *result) {
    if (request.runtime_graph == nullptr || request.scene_output == nullptr) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::InvalidArgument,
            PreviewHostDiagnosticCode::MissingRuntimeAssetGraph,
            RuntimeAssetDataStatus::InvalidArgument,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::None,
            RuntimeAssetFileKind::Unknown,
            RuntimeAssetFileKind::Unknown,
            0U,
            0U,
            0U,
            0U);
    }

    const RuntimeAssetGraphLoadResult &graph = *request.runtime_graph;
    const RuntimeAssetSceneLoaderOutput &scene_output = *request.scene_output;
    result->runtime_asset_status = graph.status;
    result->runtime_loaded_file_count = graph.loaded_file_count;
    result->resource_ref_count = scene_output.resource_ref_count;

    if (graph.status != RuntimeAssetDataStatus::Success ||
        graph.transaction_result.status != RuntimeAssetDataStatus::Success ||
        scene_output.status != RuntimeAssetDataStatus::Success) {
        RuntimeAssetDataStatus status = graph.status;
        if (status == RuntimeAssetDataStatus::Success) {
            status = graph.transaction_result.status;
        }

        if (status == RuntimeAssetDataStatus::Success) {
            status = scene_output.status;
        }

        return EmitFailure(
            request,
            result,
            PreviewHostStatus::RuntimeAssetStatusFailed,
            PreviewHostDiagnosticCode::RuntimeAssetStatusFailed,
            status,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::None,
            RuntimeAssetFileKind::Unknown,
            RuntimeAssetFileKind::Unknown,
            graph.scene.stable_id,
            0U,
            0U,
            0U);
    }

    if (!graph.scene_registered || !graph.scene_references_runtime_asset_families) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::RuntimeAssetGraphStale,
            PreviewHostDiagnosticCode::RuntimeAssetGraphStale,
            RuntimeAssetDataStatus::Success,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::None,
            RuntimeAssetFileKind::Scene,
            graph.scene.kind,
            graph.scene.stable_id,
            0U,
            0U,
            0U);
    }

    if (graph.loaded_file_count == 0U ||
        graph.loaded_file_count > request.loaded_files.size() ||
        scene_output.resource_ref_count > request.resource_refs.size() ||
        scene_output.entity_count > request.scene_entities.size() ||
        scene_output.entity_count > MAX_PREVIEW_HOST_FRAME_ENTITIES) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::RuntimeAssetGraphStale,
            PreviewHostDiagnosticCode::RuntimeAssetGraphStale,
            RuntimeAssetDataStatus::Success,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::None,
            RuntimeAssetFileKind::Unknown,
            RuntimeAssetFileKind::Unknown,
            graph.scene.stable_id,
            graph.loaded_file_count,
            scene_output.resource_ref_count,
            scene_output.entity_count);
    }

    result->consumed_runtime_asset_graph = true;
    result->consumed_resource_refs = scene_output.resource_ref_count > 0U;
    return PreviewHostStatus::Success;
}

PreviewHostStatus ResolveResourceRef(
    const PreviewHostFrameRequest &request,
    PreviewHostFrameResult *result,
    std::uint32_t resource_ref_index,
    RuntimeAssetFileKind expected_kind,
    std::uint32_t entity_index,
    const RuntimeAssetSceneResourceRef **out_ref,
    const RuntimeAssetLoadedFile **out_file) {
    if (out_ref == nullptr || out_file == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    *out_ref = nullptr;
    *out_file = nullptr;
    if (resource_ref_index >= request.resource_refs.size()) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::MissingResourceRef,
            PreviewHostDiagnosticCode::MissingResourceRef,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement,
            expected_kind,
            RuntimeAssetFileKind::Unknown,
            0U,
            0U,
            resource_ref_index,
            entity_index);
    }

    const RuntimeAssetSceneResourceRef &ref = request.resource_refs[resource_ref_index];
    if (ref.loaded_file_index >= request.loaded_files.size()) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::MissingResourceRef,
            PreviewHostDiagnosticCode::MissingResourceRef,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement,
            expected_kind,
            ref.kind,
            ref.stable_id,
            ref.loaded_file_index,
            resource_ref_index,
            entity_index);
    }

    const RuntimeAssetLoadedFile &file = request.loaded_files[ref.loaded_file_index];
    if (ref.kind != expected_kind || file.kind != expected_kind) {
        const RuntimeAssetFileKind actual_kind =
            ref.kind != expected_kind ? ref.kind : file.kind;
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::TypeMismatch,
            PreviewHostDiagnosticCode::TypeMismatch,
            RuntimeAssetDataStatus::TypeMismatch,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement,
            expected_kind,
            actual_kind,
            ref.stable_id,
            ref.loaded_file_index,
            resource_ref_index,
            entity_index);
    }

    if (file.stable_id != ref.stable_id ||
        !IsResourceEqual(file.resource, ref.resource) ||
        !IsAssetEqual(file.asset, ref.asset)) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::RuntimeAssetGraphStale,
            PreviewHostDiagnosticCode::StaleResourceRef,
            RuntimeAssetDataStatus::HashMismatch,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement,
            expected_kind,
            file.kind,
            ref.stable_id,
            ref.loaded_file_index,
            resource_ref_index,
            entity_index);
    }

    if (!file.resource.IsValid() || !file.asset.IsValid()) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::MissingResourceRef,
            PreviewHostDiagnosticCode::MissingResourceRef,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement,
            expected_kind,
            file.kind,
            ref.stable_id,
            ref.loaded_file_index,
            resource_ref_index,
            entity_index);
    }

    if (IsDecodedPayloadRequired(expected_kind) && !file.decoded_payload_stored) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::NotCooked,
            PreviewHostDiagnosticCode::NotCooked,
            RuntimeAssetDataStatus::DecodedPayloadStoreFailed,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::GeometryModel,
            expected_kind,
            file.kind,
            ref.stable_id,
            ref.loaded_file_index,
            resource_ref_index,
            entity_index);
    }

    *out_ref = &ref;
    *out_file = &file;
    return PreviewHostStatus::Success;
}

PreviewHostStatus StatusForImportCookLayer(RuntimeAssetImportCookMissingLayer layer) {
    switch (layer) {
        case RuntimeAssetImportCookMissingLayer::None:
            return PreviewHostStatus::InvalidCookedRecord;
        case RuntimeAssetImportCookMissingLayer::Command:
        case RuntimeAssetImportCookMissingLayer::FileVfs:
            return PreviewHostStatus::MissingCommandOutput;
        case RuntimeAssetImportCookMissingLayer::RuntimeAssetData:
            return PreviewHostStatus::InvalidCookedRecord;
        case RuntimeAssetImportCookMissingLayer::Resource:
        case RuntimeAssetImportCookMissingLayer::Asset:
            return PreviewHostStatus::UnsupportedBridgeLayer;
    }

    return PreviewHostStatus::InvalidCookedRecord;
}

PreviewHostDiagnosticCode DiagnosticForImportCookLayer(RuntimeAssetImportCookMissingLayer layer) {
    switch (StatusForImportCookLayer(layer)) {
        case PreviewHostStatus::MissingCommandOutput:
            return PreviewHostDiagnosticCode::MissingCommandOutput;
        case PreviewHostStatus::UnsupportedBridgeLayer:
            return PreviewHostDiagnosticCode::UnsupportedBridgeLayer;
        case PreviewHostStatus::InvalidCookedRecord:
            return PreviewHostDiagnosticCode::InvalidCookedRecord;
        default:
            break;
    }

    return PreviewHostDiagnosticCode::InvalidCookedRecord;
}

PreviewHostStatus EmitImportCookFailure(
    const PreviewHostFrameRequest &request,
    PreviewHostFrameResult *result,
    RuntimeAssetDataStatus runtime_status,
    RuntimeAssetImportCookMissingLayer missing_layer,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetFileKind actual_kind,
    std::uint64_t stable_id,
    std::uint32_t loaded_file_index) {
    if (result == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    result->import_cook_status = runtime_status;
    const PreviewHostStatus status = StatusForImportCookLayer(missing_layer);
    return EmitFailure(
        request,
        result,
        status,
        DiagnosticForImportCookLayer(missing_layer),
        runtime_status,
        RenderSceneRuntimeFrameStatus::Success,
        RenderSceneThreePrimitiveCaptureMissingLayer::None,
        expected_kind,
        actual_kind,
        stable_id,
        loaded_file_index,
        0U,
        0U,
        missing_layer);
}

bool CommandOutputRequired(const PreviewHostFrameRequest &request) {
    if (request.command_output.require_cooked_records) {
        return true;
    }

    return request.command_output.command != nullptr;
}

std::uint32_t ResourceBrowserSeverityRank(ResourceBrowserDiagnosticSeverity severity) {
    switch (severity) {
        case ResourceBrowserDiagnosticSeverity::Info:
            return 0U;
        case ResourceBrowserDiagnosticSeverity::Warning:
            return 1U;
        case ResourceBrowserDiagnosticSeverity::Error:
            return 2U;
        case ResourceBrowserDiagnosticSeverity::Blocker:
            return 3U;
    }

    return 0U;
}

const ResourceBrowserDiagnosticRecord *HighestResourceBrowserDiagnostic(
    const PreviewHostResourceBrowserPreviewRequest &request,
    std::uint32_t *out_count) {
    if (out_count != nullptr) {
        *out_count = 0U;
    }

    const ResourceBrowserDiagnosticRecord *selected = nullptr;
    for (const ResourceBrowserDiagnosticRecord &diagnostic : request.diagnostics) {
        if (diagnostic.file_index != request.entry_index) {
            continue;
        }

        if (out_count != nullptr) {
            ++(*out_count);
        }

        if (selected == nullptr ||
            ResourceBrowserSeverityRank(diagnostic.severity) >
                ResourceBrowserSeverityRank(selected->severity)) {
            selected = &diagnostic;
        }
    }

    return selected;
}

PreviewHostDocumentKind DocumentKindForResourceBrowserKind(RuntimeAssetFileKind kind) {
    switch (kind) {
        case RuntimeAssetFileKind::Scene:
            return PreviewHostDocumentKind::Scene;
        case RuntimeAssetFileKind::Animation:
            return PreviewHostDocumentKind::Animation;
        case RuntimeAssetFileKind::Mesh:
        case RuntimeAssetFileKind::Material:
        case RuntimeAssetFileKind::Texture:
        case RuntimeAssetFileKind::Shader:
            return PreviewHostDocumentKind::Resource;
        case RuntimeAssetFileKind::Unknown:
            break;
    }

    return PreviewHostDocumentKind::Unknown;
}

PreviewHostStatus StatusForResourceBrowserDependency(ResourceBrowserDependencyState state) {
    switch (state) {
        case ResourceBrowserDependencyState::Ready:
            return PreviewHostStatus::Success;
        case ResourceBrowserDependencyState::Missing:
            return PreviewHostStatus::MissingResourceRef;
        case ResourceBrowserDependencyState::TypeMismatch:
            return PreviewHostStatus::TypeMismatch;
        case ResourceBrowserDependencyState::Duplicate:
        case ResourceBrowserDependencyState::StaleHash:
        case ResourceBrowserDependencyState::StaleSchema:
            return PreviewHostStatus::RuntimeAssetGraphStale;
        case ResourceBrowserDependencyState::Unsupported:
            return PreviewHostStatus::UnsupportedPreviewRoute;
        case ResourceBrowserDependencyState::CapacityExceeded:
        case ResourceBrowserDependencyState::BudgetExceeded:
            return PreviewHostStatus::OutputCapacityExceeded;
        case ResourceBrowserDependencyState::Unknown:
            break;
    }

    return PreviewHostStatus::RuntimeAssetGraphStale;
}

PreviewHostDiagnosticCode DiagnosticForResourceBrowserStatus(PreviewHostStatus status) {
    switch (status) {
        case PreviewHostStatus::Success:
            return PreviewHostDiagnosticCode::None;
        case PreviewHostStatus::RuntimeAssetStatusFailed:
            return PreviewHostDiagnosticCode::RuntimeAssetStatusFailed;
        case PreviewHostStatus::RuntimeAssetGraphStale:
            return PreviewHostDiagnosticCode::RuntimeAssetGraphStale;
        case PreviewHostStatus::MissingResourceRef:
            return PreviewHostDiagnosticCode::MissingResourceRef;
        case PreviewHostStatus::TypeMismatch:
            return PreviewHostDiagnosticCode::TypeMismatch;
        case PreviewHostStatus::NotCooked:
            return PreviewHostDiagnosticCode::NotCooked;
        case PreviewHostStatus::OutputCapacityExceeded:
            return PreviewHostDiagnosticCode::OutputCapacityExceeded;
        case PreviewHostStatus::UnsupportedDocumentKind:
            return PreviewHostDiagnosticCode::UnsupportedDocumentKind;
        case PreviewHostStatus::UnsupportedPreviewRoute:
            return PreviewHostDiagnosticCode::UnsupportedPreviewRoute;
        default:
            break;
    }

    return PreviewHostDiagnosticCode::RuntimeAssetStatusFailed;
}

PreviewHostStatus EmitResourceBrowserDecisionFailure(
    const PreviewHostResourceBrowserPreviewRequest &request,
    PreviewHostResourceBrowserPreviewResult *result,
    PreviewHostStatus status,
    RuntimeAssetDataStatus runtime_status,
    const ResourceBrowserDiagnosticRecord *resource_browser_diagnostic) {
    if (result == nullptr || request.entry == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    const ResourceBrowserResourceEntry &entry = *request.entry;
    result->status = status;
    result->preview_eligible = false;
    result->accepted_resource_browser_entry = true;
    result->document_kind = DocumentKindForResourceBrowserKind(entry.validation.kind);
    result->diagnostic.code = DiagnosticForResourceBrowserStatus(status);
    result->diagnostic.status = status;
    result->diagnostic.runtime_asset_status = runtime_status;
    result->diagnostic.expected_kind = entry.import_settings.target_kind;
    result->diagnostic.actual_kind = entry.validation.kind;
    result->diagnostic.stable_id = entry.import_settings.stable_id;
    result->diagnostic.resource_browser_dependency_state = entry.dependency_state;

    if (resource_browser_diagnostic != nullptr) {
        result->diagnostic.resource_browser_code = resource_browser_diagnostic->code;
        result->diagnostic.resource_browser_severity = resource_browser_diagnostic->severity;
        result->diagnostic.resource_browser_phase = resource_browser_diagnostic->phase;
        result->diagnostic.from_resource_browser_diagnostics = true;
    }

    return status;
}

PreviewHostStatus ValidateCommandOutput(
    const PreviewHostFrameRequest &request,
    PreviewHostFrameResult *result) {
    if (!CommandOutputRequired(request)) {
        return PreviewHostStatus::Success;
    }

    if (result == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    if (request.command_output.command == nullptr) {
        return EmitImportCookFailure(
            request,
            result,
            RuntimeAssetDataStatus::InvalidArgument,
            RuntimeAssetImportCookMissingLayer::Command,
            RuntimeAssetFileKind::Scene,
            RuntimeAssetFileKind::Unknown,
            0U,
            0U);
    }

    const yuengine::runtimeasset::RuntimeAssetImportCookCommandResult &command =
        *request.command_output.command;
    result->import_cook_status = command.status;
    result->import_cook_missing_layer = command.missing_layer;

    if (command.status != RuntimeAssetDataStatus::Success ||
        command.fixture.status != RuntimeAssetDataStatus::Success ||
        command.missing_layer != RuntimeAssetImportCookMissingLayer::None) {
        RuntimeAssetImportCookMissingLayer layer = command.missing_layer;
        if (layer == RuntimeAssetImportCookMissingLayer::None) {
            layer = command.fixture.missing_layer;
        }

        if (layer == RuntimeAssetImportCookMissingLayer::None) {
            layer = RuntimeAssetImportCookMissingLayer::RuntimeAssetData;
        }

        return EmitImportCookFailure(
            request,
            result,
            command.status,
            layer,
            RuntimeAssetFileKind::Scene,
            RuntimeAssetFileKind::Unknown,
            command.fixture.cooked_scene.stable_id,
            0U);
    }

    if (!command.fixture.wrote_to_disk || !command.fixture.validated_cooked_files) {
        return EmitImportCookFailure(
            request,
            result,
            RuntimeAssetDataStatus::InvalidArgument,
            RuntimeAssetImportCookMissingLayer::FileVfs,
            RuntimeAssetFileKind::Scene,
            RuntimeAssetFileKind::Unknown,
            command.fixture.cooked_scene.stable_id,
            0U);
    }

    if (request.command_output.cooked_scene == nullptr ||
        request.command_output.cooked_files.empty() ||
        command.fixture.cooked_file_count == 0U ||
        command.fixture.cooked_file_count > request.command_output.cooked_files.size()) {
        return EmitImportCookFailure(
            request,
            result,
            RuntimeAssetDataStatus::MissingDependency,
            RuntimeAssetImportCookMissingLayer::RuntimeAssetData,
            RuntimeAssetFileKind::Scene,
            RuntimeAssetFileKind::Unknown,
            command.fixture.cooked_scene.stable_id,
            command.fixture.cooked_file_count);
    }

    const yuengine::runtimeasset::RuntimeAssetFileDesc &command_scene =
        *request.command_output.cooked_scene;
    if (command_scene.kind != RuntimeAssetFileKind::Scene ||
        command_scene.path == nullptr ||
        command_scene.stable_id != command.fixture.cooked_scene.stable_id ||
        request.runtime_graph->scene.stable_id != command_scene.stable_id ||
        request.runtime_graph->scene.kind != RuntimeAssetFileKind::Scene ||
        request.runtime_graph->scene.artifact_class != yuengine::runtimeasset::RuntimeAssetArtifactClass::Cooked) {
        return EmitImportCookFailure(
            request,
            result,
            RuntimeAssetDataStatus::TypeMismatch,
            RuntimeAssetImportCookMissingLayer::RuntimeAssetData,
            RuntimeAssetFileKind::Scene,
            request.runtime_graph->scene.kind,
            command_scene.stable_id,
            0U);
    }

    if (request.runtime_graph->loaded_file_count != command.fixture.cooked_file_count) {
        return EmitImportCookFailure(
            request,
            result,
            RuntimeAssetDataStatus::InvalidBounds,
            RuntimeAssetImportCookMissingLayer::RuntimeAssetData,
            RuntimeAssetFileKind::Unknown,
            RuntimeAssetFileKind::Unknown,
            command_scene.stable_id,
            request.runtime_graph->loaded_file_count);
    }

    for (std::uint32_t index = 0U; index < command.fixture.cooked_file_count; ++index) {
        const yuengine::runtimeasset::RuntimeAssetFileDesc &desc =
            request.command_output.cooked_files[index];
        const RuntimeAssetLoadedFile &file = request.loaded_files[index];
        if (desc.path == nullptr ||
            file.kind != desc.kind ||
            file.stable_id != desc.stable_id ||
            file.resource_type.value != desc.resource_type.value ||
            file.asset_type.value != desc.asset_type.value ||
            file.artifact_class != yuengine::runtimeasset::RuntimeAssetArtifactClass::Cooked ||
            file.schema_version == 0U ||
            file.identity_hash == 0U ||
            file.payload_hash == 0U) {
            return EmitImportCookFailure(
                request,
                result,
                RuntimeAssetDataStatus::TypeMismatch,
                RuntimeAssetImportCookMissingLayer::RuntimeAssetData,
                desc.kind,
                file.kind,
                desc.stable_id,
                index);
        }
    }

    result->import_cook_status = RuntimeAssetDataStatus::Success;
    result->import_cook_missing_layer = RuntimeAssetImportCookMissingLayer::None;
    result->command_cooked_file_count = command.fixture.cooked_file_count;
    result->consumed_import_cook_command_output = true;
    return PreviewHostStatus::Success;
}

const RenderScenePrimitiveGeometryRecord *FindGeometry(
    std::span<const RenderScenePrimitiveGeometryRecord> geometry_records,
    yuengine::asset::AssetHandle asset) {
    for (const RenderScenePrimitiveGeometryRecord &geometry : geometry_records) {
        if (!geometry.is_resolved) {
            continue;
        }

        if (IsAssetEqual(geometry.geometry_asset, asset)) {
            return &geometry;
        }
    }

    return nullptr;
}

PreviewHostStatus ValidateMaterial(
    const PreviewHostFrameRequest &request,
    PreviewHostFrameResult *result,
    const RuntimeAssetSceneEntityRecord &entity,
    std::uint32_t entity_index) {
    const RuntimeAssetSceneResourceRef *material_ref = nullptr;
    const RuntimeAssetLoadedFile *material_file = nullptr;
    PreviewHostStatus status = ResolveResourceRef(
        request,
        result,
        entity.material_ref_index,
        RuntimeAssetFileKind::Material,
        entity_index,
        &material_ref,
        &material_file);
    if (status != PreviewHostStatus::Success) {
        return status;
    }

    if (material_ref == nullptr || material_file == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    if (!IsAssetEqual(request.material.material_asset, material_file->asset) ||
        request.material.material_id == 0U ||
        !request.material.is_resolved ||
        !IsPipelineValid(request.material.pipeline)) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::TypeMismatch,
            PreviewHostDiagnosticCode::TypeMismatch,
            RuntimeAssetDataStatus::TypeMismatch,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::MaterialTextureSlots,
            RuntimeAssetFileKind::Material,
            material_file->kind,
            material_file->stable_id,
            material_ref->loaded_file_index,
            entity.material_ref_index,
            entity_index);
    }

    return PreviewHostStatus::Success;
}

PreviewHostStatus ValidateTextureRefs(
    const PreviewHostFrameRequest &request,
    PreviewHostFrameResult *result,
    const RuntimeAssetSceneEntityRecord &entity,
    std::uint32_t entity_index) {
    const RuntimeAssetSceneResourceRef *texture_ref = nullptr;
    const RuntimeAssetLoadedFile *texture_file = nullptr;
    PreviewHostStatus status = ResolveResourceRef(
        request,
        result,
        entity.texture_ref_index,
        RuntimeAssetFileKind::Texture,
        entity_index,
        &texture_ref,
        &texture_file);
    if (status != PreviewHostStatus::Success) {
        return status;
    }

    if (texture_file == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    if (request.material.texture_slot_count == 0U) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::NotCooked,
            PreviewHostDiagnosticCode::NotCooked,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::MaterialTextureSlots,
            RuntimeAssetFileKind::Texture,
            RuntimeAssetFileKind::Unknown,
            texture_file->stable_id,
            texture_ref == nullptr ? 0U : texture_ref->loaded_file_index,
            entity.texture_ref_index,
            entity_index);
    }

    bool found_texture_slot = false;
    for (std::size_t slot_index = 0U; slot_index < request.material.texture_slot_count; ++slot_index) {
        if (IsAssetEqual(request.material.texture_slots[slot_index].texture_asset, texture_file->asset)) {
            found_texture_slot = true;
            break;
        }
    }

    if (!found_texture_slot) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::TypeMismatch,
            PreviewHostDiagnosticCode::TypeMismatch,
            RuntimeAssetDataStatus::TypeMismatch,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::MaterialTextureSlots,
            RuntimeAssetFileKind::Texture,
            texture_file->kind,
            texture_file->stable_id,
            texture_ref == nullptr ? 0U : texture_ref->loaded_file_index,
            entity.texture_ref_index,
            entity_index);
    }

    return PreviewHostStatus::Success;
}

const char *ObjectNameForKind(RenderScenePrimitiveGeometryKind kind, std::size_t *out_byte_count) {
    if (out_byte_count == nullptr) {
        return nullptr;
    }

    if (kind == RenderScenePrimitiveGeometryKind::Cube) {
        *out_byte_count = 4U;
        return "Cube";
    }

    if (kind == RenderScenePrimitiveGeometryKind::Cylinder) {
        *out_byte_count = 8U;
        return "Cylinder";
    }

    if (kind == RenderScenePrimitiveGeometryKind::Cone) {
        *out_byte_count = 4U;
        return "Cone";
    }

    *out_byte_count = 0U;
    return nullptr;
}

PreviewHostStatus BuildEntityRequests(
    const PreviewHostFrameRequest &request,
    PreviewHostFrameResult *result,
    std::array<RenderSceneRuntimeFrameEntityRequest, MAX_PREVIEW_HOST_FRAME_ENTITIES> *out_frame_entities,
    std::array<RenderSceneThreePrimitiveEntityRequest, yuengine::renderscene::RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> *out_capture_entities) {
    if (result == nullptr || out_frame_entities == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    const RuntimeAssetSceneLoaderOutput &scene_output = *request.scene_output;
    for (std::uint32_t index = 0U; index < scene_output.entity_count; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = request.scene_entities[index];

        const RuntimeAssetSceneResourceRef *mesh_ref = nullptr;
        const RuntimeAssetLoadedFile *mesh_file = nullptr;
        PreviewHostStatus status = ResolveResourceRef(
            request,
            result,
            entity.mesh_ref_index,
            RuntimeAssetFileKind::Mesh,
            index,
            &mesh_ref,
            &mesh_file);
        if (status != PreviewHostStatus::Success) {
            return status;
        }

        if (mesh_ref == nullptr || mesh_file == nullptr) {
            return PreviewHostStatus::InvalidArgument;
        }

        status = ValidateMaterial(request, result, entity, index);
        if (status != PreviewHostStatus::Success) {
            return status;
        }

        status = ValidateTextureRefs(request, result, entity, index);
        if (status != PreviewHostStatus::Success) {
            return status;
        }

        const RenderScenePrimitiveGeometryRecord *geometry =
            FindGeometry(request.geometry_records, mesh_file->asset);
        if (geometry == nullptr) {
            return EmitFailure(
                request,
                result,
                PreviewHostStatus::MissingResourceRef,
                PreviewHostDiagnosticCode::MissingResourceRef,
                RuntimeAssetDataStatus::MissingDependency,
                RenderSceneRuntimeFrameStatus::MissingGeometryRecord,
                RenderSceneThreePrimitiveCaptureMissingLayer::GeometryModel,
                RuntimeAssetFileKind::Mesh,
                mesh_file->kind,
                mesh_file->stable_id,
                mesh_ref->loaded_file_index,
                entity.mesh_ref_index,
                index);
        }

        RenderSceneRuntimeFrameEntityRequest &frame_entity = (*out_frame_entities)[index];
        frame_entity.world_object_id = entity.world_object_id;
        frame_entity.transform = entity.transform;
        frame_entity.geometry = *geometry;
        frame_entity.is_visible = entity.is_visible;
        frame_entity.is_active = entity.is_active;

        if (out_capture_entities != nullptr &&
            index < yuengine::renderscene::RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
            std::size_t object_name_byte_count = 0U;
            const char *object_name = ObjectNameForKind(geometry->kind, &object_name_byte_count);
            if (object_name == nullptr) {
                return EmitFailure(
                    request,
                    result,
                    PreviewHostStatus::UnsupportedPreviewRoute,
                    PreviewHostDiagnosticCode::UnsupportedPreviewRoute,
                    RuntimeAssetDataStatus::UnsupportedFieldValue,
                    RenderSceneRuntimeFrameStatus::Success,
                    RenderSceneThreePrimitiveCaptureMissingLayer::GeometryModel,
                    RuntimeAssetFileKind::Mesh,
                    mesh_file->kind,
                    mesh_file->stable_id,
                    mesh_ref->loaded_file_index,
                    entity.mesh_ref_index,
                    index);
            }

            RenderSceneThreePrimitiveEntityRequest &capture_entity = (*out_capture_entities)[index];
            capture_entity.world_object_id = entity.world_object_id;
            capture_entity.object_name = object_name;
            capture_entity.object_name_byte_count = object_name_byte_count;
            capture_entity.transform = entity.transform;
            capture_entity.geometry = *geometry;
            capture_entity.is_visible = entity.is_visible;
            capture_entity.is_active = entity.is_active;
        }
    }

    return PreviewHostStatus::Success;
}

PreviewHostStatus ValidateFeedbackCapacity(
    const PreviewHostFrameRequest &request,
    PreviewHostFrameResult *result,
    std::uint32_t entity_count) {
    if (!request.hit_records.empty() && request.hit_records.size() < entity_count) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::OutputCapacityExceeded,
            PreviewHostDiagnosticCode::OutputCapacityExceeded,
            RuntimeAssetDataStatus::CapacityExceeded,
            RenderSceneRuntimeFrameStatus::OutputCapacityExceeded,
            RenderSceneThreePrimitiveCaptureMissingLayer::OutputPath,
            RuntimeAssetFileKind::Unknown,
            RuntimeAssetFileKind::Unknown,
            0U,
            0U,
            0U,
            entity_count);
    }

    if (!request.selection_records.empty() && request.selection_records.size() < entity_count) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::OutputCapacityExceeded,
            PreviewHostDiagnosticCode::OutputCapacityExceeded,
            RuntimeAssetDataStatus::CapacityExceeded,
            RenderSceneRuntimeFrameStatus::OutputCapacityExceeded,
            RenderSceneThreePrimitiveCaptureMissingLayer::OutputPath,
            RuntimeAssetFileKind::Unknown,
            RuntimeAssetFileKind::Unknown,
            0U,
            0U,
            0U,
            entity_count);
    }

    if (!request.transform_feedback.empty() && request.transform_feedback.size() < entity_count) {
        return EmitFailure(
            request,
            result,
            PreviewHostStatus::OutputCapacityExceeded,
            PreviewHostDiagnosticCode::OutputCapacityExceeded,
            RuntimeAssetDataStatus::CapacityExceeded,
            RenderSceneRuntimeFrameStatus::OutputCapacityExceeded,
            RenderSceneThreePrimitiveCaptureMissingLayer::OutputPath,
            RuntimeAssetFileKind::Unknown,
            RuntimeAssetFileKind::Unknown,
            0U,
            0U,
            0U,
            entity_count);
    }

    return PreviewHostStatus::Success;
}

void FillFeedback(
    const PreviewHostFrameRequest &request,
    PreviewHostFrameResult *result,
    std::uint32_t entity_count) {
    if (result == nullptr) {
        return;
    }

    for (std::uint32_t index = 0U; index < entity_count; ++index) {
        const RuntimeAssetSceneEntityRecord &entity = request.scene_entities[index];
        if (!request.hit_records.empty()) {
            request.hit_records[index].world_object_id = entity.world_object_id;
            request.hit_records[index].entity_index = index;
            request.hit_records[index].hit_available = true;
        }

        if (!request.selection_records.empty()) {
            request.selection_records[index].world_object_id = entity.world_object_id;
            request.selection_records[index].entity_index = index;
            request.selection_records[index].selectable = entity.is_active && entity.is_visible;
        }

        if (!request.transform_feedback.empty()) {
            request.transform_feedback[index].world_object_id = entity.world_object_id;
            request.transform_feedback[index].transform = entity.transform;
            request.transform_feedback[index].transform_available = true;
        }
    }

    result->hit_record_count = request.hit_records.empty() ? 0U : entity_count;
    result->selection_record_count = request.selection_records.empty() ? 0U : entity_count;
    result->transform_feedback_count = request.transform_feedback.empty() ? 0U : entity_count;
}

PreviewHostStatus StatusForSurfacePreviewState(ResourceBrowserSurfacePreviewState state) {
    switch (state) {
        case ResourceBrowserSurfacePreviewState::Eligible:
            return PreviewHostStatus::Success;
        case ResourceBrowserSurfacePreviewState::BlockedByValidation:
        case ResourceBrowserSurfacePreviewState::BlockedByDiagnostic:
            return PreviewHostStatus::RuntimeAssetStatusFailed;
        case ResourceBrowserSurfacePreviewState::BlockedByDependency:
        case ResourceBrowserSurfacePreviewState::BlockedByResourceAssetRecord:
            return PreviewHostStatus::MissingResourceRef;
        case ResourceBrowserSurfacePreviewState::BlockedByLoadRecord:
            return PreviewHostStatus::RuntimeAssetGraphStale;
        case ResourceBrowserSurfacePreviewState::BlockedByUnsupportedKind:
            return PreviewHostStatus::UnsupportedPreviewRoute;
        case ResourceBrowserSurfacePreviewState::Unknown:
            break;
    }

    return PreviewHostStatus::RuntimeAssetGraphStale;
}

RuntimeAssetDataStatus RuntimeStatusForSurfaceSelection(
    const ResourceBrowserSurfaceSelectionState &selection) {
    if (selection.validation_status != RuntimeAssetDataStatus::Success) {
        return selection.validation_status;
    }

    if (!selection.import_settings_valid) {
        return RuntimeAssetDataStatus::InvalidArgument;
    }

    if (selection.preview_state == ResourceBrowserSurfacePreviewState::BlockedByDependency ||
        selection.preview_state == ResourceBrowserSurfacePreviewState::BlockedByLoadRecord ||
        selection.preview_state == ResourceBrowserSurfacePreviewState::BlockedByResourceAssetRecord) {
        return RuntimeAssetDataStatus::MissingDependency;
    }

    if (selection.preview_state == ResourceBrowserSurfacePreviewState::BlockedByUnsupportedKind) {
        return RuntimeAssetDataStatus::UnsupportedFieldValue;
    }

    if (selection.preview_state == ResourceBrowserSurfacePreviewState::BlockedByDiagnostic) {
        return RuntimeAssetDataStatus::InvalidDependency;
    }

    return RuntimeAssetDataStatus::Success;
}

void CopySurfaceSelectionToViewportResult(
    const ResourceBrowserSurfaceSelectionState &selection,
    PreviewHostViewportSessionResult *result) {
    if (result == nullptr) {
        return;
    }

    result->resource_browser_preview_state = selection.preview_state;
    result->resource_browser_document_kind = selection.preview_document_kind;
    result->matched_resource_browser_diagnostic_count = selection.matched_diagnostic_count;
    result->consumed_resource_browser_selection = selection.selected;
    result->resource_browser_preview_eligible = selection.preview_eligible;
    result->resource_asset_mapping_preserved = selection.resource_asset_mapping_preserved;
    result->used_locator_path_as_type_truth = selection.used_locator_path_as_type_truth;
}

PreviewHostStatus EmitViewportSelectionFailure(
    const PreviewHostViewportSessionRequest &request,
    PreviewHostViewportSessionResult *result,
    PreviewHostStatus status,
    RuntimeAssetDataStatus runtime_status) {
    if (result == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    result->status = status;
    result->frame.status = status;
    result->frame.frame = request.frame_request.frame;
    result->frame.camera_state = request.frame_request.camera_state;
    result->frame.runtime_asset_status = runtime_status;

    const ResourceBrowserSurfaceSelectionState *selection =
        request.resource_browser_selection;
    if (selection != nullptr) {
        CopySurfaceSelectionToViewportResult(*selection, result);
    }

    if (request.frame_request.diagnostics.empty()) {
        return status;
    }

    PreviewHostDiagnostic diagnostic{};
    diagnostic.status = status;
    diagnostic.code = DiagnosticForResourceBrowserStatus(status);
    diagnostic.runtime_asset_status = runtime_status;
    if (selection != nullptr) {
        diagnostic.resource_browser_code = selection->blocking_diagnostic_code;
        diagnostic.resource_browser_severity = selection->blocking_diagnostic_severity;
        diagnostic.resource_browser_phase = selection->blocking_diagnostic_phase;
        diagnostic.resource_browser_dependency_state = selection->dependency_state;
        diagnostic.stable_id = selection->stable_id;
        diagnostic.resource_ref_index = selection->selected_index;
        diagnostic.from_resource_browser_diagnostics =
            selection->matched_diagnostic_count > 0U ||
            selection->diagnostic_blocks_preview;
    }

    request.frame_request.diagnostics[0U] = diagnostic;
    result->frame.diagnostic_count = 1U;
    return status;
}
}

PreviewHostStatus PreviewHost::StartSession(
    const PreviewHostSessionDesc &desc,
    PreviewHostSessionResult *out_result) {
    if (out_result == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    PreviewHostSessionResult result{};
    if (desc.document_kind == PreviewHostDocumentKind::Unknown) {
        result.status = PreviewHostStatus::UnsupportedDocumentKind;
        result.active_session_count = ActiveSessionCount();
        *out_result = result;
        return result.status;
    }

    for (std::size_t index = 0U; index < sessions_.size(); ++index) {
        SessionSlot &slot = sessions_[index];
        if (slot.active) {
            continue;
        }

        PreviewHostSessionId session{};
        session.slot = static_cast<std::uint32_t>(index + 1U);
        session.generation = next_generation_;
        ++next_generation_;
        if (next_generation_ == 0U) {
            next_generation_ = 1U;
        }

        slot.id = session;
        slot.document_kind = desc.document_kind;
        slot.active = true;

        result.status = PreviewHostStatus::Success;
        result.session = session;
        result.active_session_count = ActiveSessionCount();
        *out_result = result;
        return result.status;
    }

    result.status = PreviewHostStatus::SessionCapacityExceeded;
    result.active_session_count = ActiveSessionCount();
    *out_result = result;
    return result.status;
}

PreviewHostStatus PreviewHost::StopSession(
    PreviewHostSessionId session,
    PreviewHostSessionResult *out_result) {
    if (out_result == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    PreviewHostSessionResult result{};
    if (!session.IsValid() || session.slot > sessions_.size()) {
        result.status = PreviewHostStatus::StaleSession;
        result.active_session_count = ActiveSessionCount();
        *out_result = result;
        return result.status;
    }

    SessionSlot &slot = sessions_[session.slot - 1U];
    if (!slot.active ||
        slot.id.generation != session.generation ||
        slot.id.slot != session.slot) {
        result.status = PreviewHostStatus::StaleSession;
        result.active_session_count = ActiveSessionCount();
        *out_result = result;
        return result.status;
    }

    slot = SessionSlot{};
    result.status = PreviewHostStatus::Success;
    result.session = session;
    result.active_session_count = ActiveSessionCount();
    *out_result = result;
    return result.status;
}

PreviewHostStatus PreviewHost::BuildFrame(
    const PreviewHostFrameRequest &request,
    PreviewHostFrameResult *out_result) const {
    if (out_result == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    PreviewHostFrameResult result{};
    result.frame = request.frame;
    result.camera_state = request.camera_state;
    result.runtime_asset_status = RuntimeAssetDataStatus::Success;
    result.import_cook_status = RuntimeAssetDataStatus::Success;
    result.import_cook_missing_layer = RuntimeAssetImportCookMissingLayer::None;

    if (!IsSessionActive(request.session, request.document_kind)) {
        const PreviewHostStatus status = EmitFailure(
            request,
            &result,
            PreviewHostStatus::StaleSession,
            PreviewHostDiagnosticCode::StaleSession,
            RuntimeAssetDataStatus::Success,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::None,
            RuntimeAssetFileKind::Unknown,
            RuntimeAssetFileKind::Unknown,
            0U,
            0U,
            0U,
            0U);
        *out_result = result;
        return status;
    }

    if (request.document_kind != PreviewHostDocumentKind::Scene) {
        const PreviewHostStatus status = EmitFailure(
            request,
            &result,
            PreviewHostStatus::UnsupportedDocumentKind,
            PreviewHostDiagnosticCode::UnsupportedDocumentKind,
            RuntimeAssetDataStatus::UnsupportedFieldValue,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::None,
            RuntimeAssetFileKind::Unknown,
            RuntimeAssetFileKind::Unknown,
            0U,
            0U,
            0U,
            0U);
        *out_result = result;
        return status;
    }

    if (request.frame.frame_id == 0U) {
        const PreviewHostStatus status = EmitFailure(
            request,
            &result,
            PreviewHostStatus::InvalidArgument,
            PreviewHostDiagnosticCode::OutputCapacityExceeded,
            RuntimeAssetDataStatus::InvalidArgument,
            RenderSceneRuntimeFrameStatus::InvalidFrameId,
            RenderSceneThreePrimitiveCaptureMissingLayer::None,
            RuntimeAssetFileKind::Unknown,
            RuntimeAssetFileKind::Unknown,
            0U,
            0U,
            0U,
            0U);
        *out_result = result;
        return status;
    }

    PreviewHostStatus status = ValidateGraph(request, &result);
    if (status != PreviewHostStatus::Success) {
        *out_result = result;
        return status;
    }

    status = ValidateCommandOutput(request, &result);
    if (status != PreviewHostStatus::Success) {
        *out_result = result;
        return status;
    }

    const std::uint32_t entity_count = request.scene_output->entity_count;
    status = ValidateFeedbackCapacity(request, &result, entity_count);
    if (status != PreviewHostStatus::Success) {
        *out_result = result;
        return status;
    }

    std::array<RenderSceneRuntimeFrameEntityRequest, MAX_PREVIEW_HOST_FRAME_ENTITIES>
        frame_entities{};
    std::array<
        RenderSceneThreePrimitiveEntityRequest,
        yuengine::renderscene::RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> capture_entities{};
    status = BuildEntityRequests(request, &result, &frame_entities, &capture_entities);
    if (status != PreviewHostStatus::Success) {
        *out_result = result;
        return status;
    }

    if (request.camera.status != yuengine::renderscene::RenderSceneStatus::Success ||
        !request.camera.camera.is_active ||
        request.camera.camera.camera_id == 0U) {
        status = EmitFailure(
            request,
            &result,
            PreviewHostStatus::MissingCamera,
            PreviewHostDiagnosticCode::MissingCamera,
            RuntimeAssetDataStatus::MissingDependency,
            RenderSceneRuntimeFrameStatus::MissingCamera,
            RenderSceneThreePrimitiveCaptureMissingLayer::Camera,
            RuntimeAssetFileKind::Scene,
            RuntimeAssetFileKind::Unknown,
            request.runtime_graph->scene.stable_id,
            0U,
            0U,
            0U);
        *out_result = result;
        return status;
    }

    if (!request.frame.capture_requested) {
        RenderSceneRuntimeFrameRequest frame_request{};
        frame_request.frame_id = request.frame.frame_id;
        frame_request.camera = request.camera;
        frame_request.material = request.material;
        frame_request.entities = std::span<const RenderSceneRuntimeFrameEntityRequest>(
            frame_entities.data(),
            entity_count);

        std::array<RenderSceneRuntimeFrameDrawRecord, MAX_PREVIEW_HOST_FRAME_ENTITIES> draws{};
        RenderSceneRuntimeFrameBuilder builder;
        const RenderSceneRuntimeFrameStatus frame_status =
            builder.Build(
                frame_request,
                std::span<RenderSceneRuntimeFrameDrawRecord>(draws.data(), entity_count),
                &result.render_frame);
        if (frame_status != RenderSceneRuntimeFrameStatus::Success) {
            status = EmitFailure(
                request,
                &result,
                PreviewHostStatus::RenderSceneFailed,
                PreviewHostDiagnosticCode::RenderSceneFailed,
                RuntimeAssetDataStatus::Success,
                frame_status,
                RenderSceneThreePrimitiveCaptureMissingLayer::RenderSceneSubmission,
                RuntimeAssetFileKind::Scene,
                RuntimeAssetFileKind::Scene,
                request.runtime_graph->scene.stable_id,
                0U,
                0U,
                0U);
            *out_result = result;
            return status;
        }

        result.status = PreviewHostStatus::Success;
        result.submitted_entity_count = static_cast<std::uint32_t>(result.render_frame.submitted_entity_count);
        result.submitted_render_scene_frame = true;
        result.headless_output = true;
        FillFeedback(request, &result, entity_count);
        *out_result = result;
        return result.status;
    }

    if (entity_count != yuengine::renderscene::RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        status = EmitFailure(
            request,
            &result,
            PreviewHostStatus::UnsupportedPreviewRoute,
            PreviewHostDiagnosticCode::UnsupportedPreviewRoute,
            RuntimeAssetDataStatus::UnsupportedFieldValue,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement,
            RuntimeAssetFileKind::Scene,
            RuntimeAssetFileKind::Scene,
            request.runtime_graph->scene.stable_id,
            0U,
            0U,
            entity_count);
        *out_result = result;
        return status;
    }

    if (request.rhi_device == nullptr) {
        status = EmitFailure(
            request,
            &result,
            PreviewHostStatus::RenderCoreRhiFailed,
            PreviewHostDiagnosticCode::RenderCoreRhiFailed,
            RuntimeAssetDataStatus::Success,
            RenderSceneRuntimeFrameStatus::Success,
            RenderSceneThreePrimitiveCaptureMissingLayer::RhiCaptureTarget,
            RuntimeAssetFileKind::Scene,
            RuntimeAssetFileKind::Scene,
            request.runtime_graph->scene.stable_id,
            0U,
            0U,
            0U);
        *out_result = result;
        return status;
    }

    RenderSceneThreePrimitiveCaptureRequest capture_request{};
    capture_request.frame_id = request.frame.frame_id;
    capture_request.camera = request.camera;
    capture_request.material = request.material;
    capture_request.entities = std::span<const RenderSceneThreePrimitiveEntityRequest>(
        capture_entities.data(),
        capture_entities.size());
    capture_request.rhi_device = request.rhi_device;
    capture_request.output_path = request.output_path;
    capture_request.output_path_byte_count = request.output_path_byte_count;
    capture_request.capture_output = request.capture_output;
    capture_request.capture_byte_budget_per_entity = request.capture_byte_budget_per_entity;

    RenderSceneThreePrimitiveCaptureRoute capture_route;
    const RenderSceneThreePrimitiveCaptureStatus capture_status =
        capture_route.Execute(capture_request, &result.capture);
    result.render_frame = result.capture.frame_result;
    result.capture_bytes_written = result.capture.capture_bytes_written;
    if (capture_status != RenderSceneThreePrimitiveCaptureStatus::Success) {
        status = EmitFailure(
            request,
            &result,
            PreviewHostStatus::RenderCoreRhiFailed,
            PreviewHostDiagnosticCode::RenderCoreRhiFailed,
            RuntimeAssetDataStatus::Success,
            result.capture.frame_result.status,
            result.capture.first_missing_layer,
            RuntimeAssetFileKind::Scene,
            RuntimeAssetFileKind::Scene,
            request.runtime_graph->scene.stable_id,
            0U,
            0U,
            0U);
        *out_result = result;
        return status;
    }

    result.status = PreviewHostStatus::Success;
    result.submitted_entity_count = static_cast<std::uint32_t>(result.render_frame.submitted_entity_count);
    result.submitted_render_scene_frame = true;
    result.captured_through_render_core_rhi = result.capture.capture_bytes_written > 0U;
    FillFeedback(request, &result, entity_count);
    *out_result = result;
    return result.status;
}

PreviewHostStatus PreviewHost::BuildViewportSessionSurface(
    const PreviewHostViewportSessionRequest &request,
    PreviewHostViewportSessionResult *out_result) const {
    if (out_result == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    PreviewHostViewportSessionResult result{};
    result.frame.frame = request.frame_request.frame;
    result.frame.camera_state = request.frame_request.camera_state;
    result.camera_state = request.frame_request.camera_state;
    result.viewport_width = request.frame_request.frame.width;
    result.viewport_height = request.frame_request.frame.height;
    result.selected_entity_index = request.selected_entity_index;
    result.consumed_viewport_controls =
        request.frame_request.frame.frame_id != 0U &&
        request.frame_request.frame.width != 0U &&
        request.frame_request.frame.height != 0U &&
        request.frame_request.camera_state.camera_id != 0U;

    if (request.resource_browser_selection == nullptr) {
        const PreviewHostStatus status = EmitViewportSelectionFailure(
            request,
            &result,
            PreviewHostStatus::InvalidArgument,
            RuntimeAssetDataStatus::InvalidArgument);
        *out_result = result;
        return status;
    }

    const ResourceBrowserSurfaceSelectionState &selection =
        *request.resource_browser_selection;
    CopySurfaceSelectionToViewportResult(selection, &result);

    if (!selection.selected) {
        const PreviewHostStatus status = EmitViewportSelectionFailure(
            request,
            &result,
            PreviewHostStatus::InvalidArgument,
            RuntimeAssetDataStatus::InvalidArgument);
        *out_result = result;
        return status;
    }

    if (!selection.import_settings_valid) {
        const PreviewHostStatus status = EmitViewportSelectionFailure(
            request,
            &result,
            PreviewHostStatus::RuntimeAssetStatusFailed,
            RuntimeAssetDataStatus::InvalidArgument);
        *out_result = result;
        return status;
    }

    if (selection.used_locator_path_as_type_truth) {
        const PreviewHostStatus status = EmitViewportSelectionFailure(
            request,
            &result,
            PreviewHostStatus::RuntimeAssetGraphStale,
            RuntimeAssetDataStatus::HashMismatch);
        *out_result = result;
        return status;
    }

    if (!selection.resource_asset_mapping_preserved) {
        const PreviewHostStatus status = EmitViewportSelectionFailure(
            request,
            &result,
            PreviewHostStatus::MissingResourceRef,
            RuntimeAssetDataStatus::MissingDependency);
        *out_result = result;
        return status;
    }

    if (!selection.preview_eligible) {
        const PreviewHostStatus status =
            StatusForSurfacePreviewState(selection.preview_state);
        const RuntimeAssetDataStatus runtime_status =
            RuntimeStatusForSurfaceSelection(selection);
        const PreviewHostStatus emitted_status = EmitViewportSelectionFailure(
            request,
            &result,
            status,
            runtime_status);
        *out_result = result;
        return emitted_status;
    }

    if (request.require_selected_entity) {
        if (request.frame_request.scene_output == nullptr ||
            request.selected_entity_index >= request.frame_request.scene_output->entity_count) {
            const PreviewHostStatus status = EmitViewportSelectionFailure(
                request,
                &result,
                PreviewHostStatus::MissingResourceRef,
                RuntimeAssetDataStatus::MissingDependency);
            *out_result = result;
            return status;
        }
    }

    PreviewHostFrameResult frame_result{};
    const PreviewHostStatus frame_status =
        BuildFrame(request.frame_request, &frame_result);
    result.frame = frame_result;
    result.camera_state = frame_result.camera_state;
    result.status = frame_status;
    if (frame_status != PreviewHostStatus::Success) {
        *out_result = result;
        return frame_status;
    }

    result.built_frame = true;
    result.selected_entity_available =
        request.selected_entity_index < frame_result.submitted_entity_count;
    result.emitted_hit_feedback = frame_result.hit_record_count > 0U;
    result.emitted_selection_feedback = frame_result.selection_record_count > 0U;
    result.emitted_transform_feedback = frame_result.transform_feedback_count > 0U;
    *out_result = result;
    return result.status;
}

PreviewHostStatus PreviewHost::ResolveResourceBrowserPreview(
    const PreviewHostResourceBrowserPreviewRequest &request,
    PreviewHostResourceBrowserPreviewResult *out_result) const {
    if (out_result == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    PreviewHostResourceBrowserPreviewResult result{};
    if (request.entry == nullptr) {
        *out_result = result;
        return result.status;
    }

    std::uint32_t diagnostic_count = 0U;
    const ResourceBrowserDiagnosticRecord *diagnostic =
        HighestResourceBrowserDiagnostic(request, &diagnostic_count);
    result.resource_browser_diagnostic_count = diagnostic_count;

    const ResourceBrowserResourceEntry &entry = *request.entry;
    result.accepted_resource_browser_entry = true;
    result.document_kind = DocumentKindForResourceBrowserKind(entry.validation.kind);
    result.used_locator_path_as_type_truth = false;

    if (entry.validation.status != RuntimeAssetDataStatus::Success) {
        const PreviewHostStatus status = EmitResourceBrowserDecisionFailure(
            request,
            &result,
            PreviewHostStatus::RuntimeAssetStatusFailed,
            entry.validation.status,
            diagnostic);
        *out_result = result;
        return status;
    }

    if (entry.dependency_state != ResourceBrowserDependencyState::Ready) {
        const PreviewHostStatus status = EmitResourceBrowserDecisionFailure(
            request,
            &result,
            StatusForResourceBrowserDependency(entry.dependency_state),
            RuntimeAssetDataStatus::InvalidDependency,
            diagnostic);
        *out_result = result;
        return status;
    }

    if (!entry.from_runtime_asset_load) {
        const PreviewHostStatus status = EmitResourceBrowserDecisionFailure(
            request,
            &result,
            PreviewHostStatus::RuntimeAssetGraphStale,
            RuntimeAssetDataStatus::MissingDependency,
            diagnostic);
        *out_result = result;
        return status;
    }

    if (!entry.from_resource_registry ||
        !entry.from_asset_record ||
        !entry.resource.IsValid() ||
        !entry.asset.IsValid()) {
        const PreviewHostStatus status = EmitResourceBrowserDecisionFailure(
            request,
            &result,
            PreviewHostStatus::MissingResourceRef,
            RuntimeAssetDataStatus::MissingDependency,
            diagnostic);
        *out_result = result;
        return status;
    }

    if (result.document_kind == PreviewHostDocumentKind::Unknown) {
        const PreviewHostStatus status = EmitResourceBrowserDecisionFailure(
            request,
            &result,
            PreviewHostStatus::UnsupportedDocumentKind,
            RuntimeAssetDataStatus::UnsupportedFieldValue,
            diagnostic);
        *out_result = result;
        return status;
    }

    result.status = PreviewHostStatus::Success;
    result.preview_eligible = true;
    result.diagnostic.resource_browser_dependency_state = entry.dependency_state;
    *out_result = result;
    return result.status;
}

std::size_t PreviewHost::ActiveSessionCount() const {
    std::size_t count = 0U;
    for (const SessionSlot &slot : sessions_) {
        if (slot.active) {
            ++count;
        }
    }

    return count;
}

bool PreviewHost::IsSessionActive(
    PreviewHostSessionId session,
    PreviewHostDocumentKind document_kind) const {
    if (!session.IsValid()) {
        return false;
    }

    if (session.slot > sessions_.size()) {
        return false;
    }

    const SessionSlot &slot = sessions_[session.slot - 1U];
    if (!slot.active) {
        return false;
    }

    if (slot.id.generation != session.generation) {
        return false;
    }

    if (slot.id.slot != session.slot) {
        return false;
    }

    return slot.document_kind == document_kind;
}
}
