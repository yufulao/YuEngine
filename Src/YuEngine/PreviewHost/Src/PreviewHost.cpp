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
using RuntimeAssetLoadedFile = yuengine::runtimeasset::RuntimeAssetLoadedFile;
using RuntimeAssetSceneEntityRecord = yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using RuntimeAssetSceneLoaderOutput = yuengine::runtimeasset::RuntimeAssetSceneLoaderOutput;
using RuntimeAssetSceneResourceRef = yuengine::runtimeasset::RuntimeAssetSceneResourceRef;
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
    std::uint32_t entity_index) {
    if (result == nullptr) {
        return PreviewHostStatus::InvalidArgument;
    }

    result->runtime_asset_status = runtime_status;
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
