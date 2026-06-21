// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderSceneThreePrimitiveCaptureRoute.cpp

#include "YuEngine/RenderScene/RenderSceneThreePrimitiveCaptureRoute.h"

#include <array>
#include <cstddef>

#include "YuEngine/RenderCore/RenderDrawableFramePipeline.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineRequest.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameEntityRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h"

namespace yuengine::renderscene {
namespace {
bool IsCameraUsable(const RenderSceneCameraBindingResult &camera) {
    if (camera.status != RenderSceneStatus::Success) {
        return false;
    }

    if (!camera.camera.is_active) {
        return false;
    }

    return camera.camera.camera_id != 0U;
}

bool IsCaptureOutputValid(const RenderSceneThreePrimitiveCaptureRequest &request) {
    if (request.capture_byte_budget_per_entity == 0U) {
        return false;
    }

    if (request.capture_output.data() == nullptr) {
        return false;
    }

    const std::size_t max_entity_budget =
        request.capture_output.size() / RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    return request.capture_byte_budget_per_entity <= max_entity_budget;
}

RenderScenePrimitiveGeometryKind ExpectedPrimitiveKind(std::size_t index) {
    if (index == 0U) {
        return RenderScenePrimitiveGeometryKind::Cube;
    }

    if (index == 1U) {
        return RenderScenePrimitiveGeometryKind::Cylinder;
    }

    return RenderScenePrimitiveGeometryKind::Cone;
}

bool IsEntityPlacementUsable(const RenderSceneThreePrimitiveEntityRequest &entity) {
    if (!entity.world_object_id.IsValid()) {
        return false;
    }

    if (entity.object_name == nullptr) {
        return false;
    }

    if (entity.object_name_byte_count == 0U) {
        return false;
    }

    if (entity.object_name_byte_count >= MAX_RENDER_SCENE_THREE_PRIMITIVE_OBJECT_NAME_BYTES) {
        return false;
    }

    if (!entity.is_active) {
        return false;
    }

    return entity.is_visible;
}

bool CopyObjectName(
    const RenderSceneThreePrimitiveEntityRequest &entity,
    RenderSceneThreePrimitiveEntityReport *out_report) {
    if (out_report == nullptr) {
        return false;
    }

    if (entity.object_name == nullptr) {
        return false;
    }

    if (entity.object_name_byte_count >= MAX_RENDER_SCENE_THREE_PRIMITIVE_OBJECT_NAME_BYTES) {
        return false;
    }

    for (std::size_t index = 0U; index < entity.object_name_byte_count; ++index) {
        out_report->object_name[index] = entity.object_name[index];
    }

    out_report->object_name[entity.object_name_byte_count] = '\0';
    out_report->object_name_byte_count = entity.object_name_byte_count;
    return true;
}
}

RenderSceneThreePrimitiveCaptureStatus RenderSceneThreePrimitiveCaptureRoute::Execute(
    const RenderSceneThreePrimitiveCaptureRequest &request,
    RenderSceneThreePrimitiveCaptureResult *out_result) {
    if (out_result == nullptr) {
        return RenderSceneThreePrimitiveCaptureStatus::InvalidArgument;
    }

    RenderSceneThreePrimitiveCaptureResult result{};
    result.frame_id = request.frame_id;
    result.capture = request.camera.capture;
    result.capture.output_byte_budget =
        request.capture_byte_budget_per_entity * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;

    if (!CopyOutputPath(request, &result)) {
        result.status = RenderSceneThreePrimitiveCaptureStatus::Fail;
        result.first_missing_layer = RenderSceneThreePrimitiveCaptureMissingLayer::OutputPath;
        *out_result = result;
        return result.status;
    }

    if (request.frame_id == 0U) {
        result.status = RenderSceneThreePrimitiveCaptureStatus::InvalidArgument;
        *out_result = result;
        return result.status;
    }

    if (request.rhi_device == nullptr) {
        result.status = RenderSceneThreePrimitiveCaptureStatus::BlockedByEnv;
        result.output_status = RenderSceneThreePrimitiveCaptureOutputStatus::BlockedByEnv;
        result.first_missing_layer = RenderSceneThreePrimitiveCaptureMissingLayer::RhiCaptureTarget;
        *out_result = result;
        return result.status;
    }

    if (!IsCaptureOutputValid(request)) {
        result.status = RenderSceneThreePrimitiveCaptureStatus::InvalidArgument;
        *out_result = result;
        return result.status;
    }

    if (!IsCameraUsable(request.camera)) {
        *out_result = result;
        return FailWithLayer(RenderSceneThreePrimitiveCaptureMissingLayer::Camera, out_result);
    }

    RenderSceneRuntimeMaterialBuilder material_builder;
    const RenderSceneRuntimeMaterialStatus material_status = material_builder.Validate(request.material);
    if (material_status != RenderSceneRuntimeMaterialStatus::Success) {
        *out_result = result;
        const RenderSceneThreePrimitiveCaptureMissingLayer layer = MapMaterialLayer(material_status);
        return FailWithLayer(layer, out_result);
    }

    if (request.entities.size() != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        *out_result = result;
        return FailWithLayer(RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement, out_result);
    }

    std::array<RenderSceneRuntimeFrameEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        frame_entities{};
    for (std::size_t index = 0U; index < request.entities.size(); ++index) {
        const RenderSceneThreePrimitiveEntityRequest &entity = request.entities[index];
        if (!IsEntityPlacementUsable(entity)) {
            *out_result = result;
            return FailWithLayer(RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement, out_result);
        }

        if (entity.geometry.kind != ExpectedPrimitiveKind(index)) {
            *out_result = result;
            return FailWithLayer(RenderSceneThreePrimitiveCaptureMissingLayer::GeometryModel, out_result);
        }

        frame_entities[index].world_object_id = entity.world_object_id;
        frame_entities[index].transform = entity.transform;
        frame_entities[index].geometry = entity.geometry;
        frame_entities[index].is_visible = entity.is_visible;
        frame_entities[index].is_active = entity.is_active;
    }

    std::array<RenderSceneRuntimeFrameDrawRecord, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> draws{};
    RenderSceneRuntimeFrameRequest frame_request{};
    frame_request.frame_id = request.frame_id;
    frame_request.camera = request.camera;
    frame_request.material = request.material;
    frame_request.entities = frame_entities;

    RenderSceneRuntimeFrameBuilder frame_builder;
    const RenderSceneRuntimeFrameStatus frame_status =
        frame_builder.Build(frame_request, draws, &result.frame_result);
    if (frame_status != RenderSceneRuntimeFrameStatus::Success) {
        *out_result = result;
        const RenderSceneThreePrimitiveCaptureMissingLayer layer = MapFrameLayer(frame_status);
        return FailWithLayer(layer, out_result);
    }

    const RenderSceneRuntimeMaterialTextureSlot &texture_slot = request.material.texture_slots[0U];
    for (std::size_t index = 0U; index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT; ++index) {
        const RenderSceneThreePrimitiveEntityRequest &entity = request.entities[index];
        RenderSceneThreePrimitiveEntityReport &report = result.entity_reports[index];
        report.world_object_id = entity.world_object_id;
        report.transform = entity.transform;
        report.primitive_kind = entity.geometry.kind;
        report.draw_record = draws[index];
        report.submitted = true;
        if (!CopyObjectName(entity, &report)) {
            *out_result = result;
            return FailWithLayer(RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement, out_result);
        }

        const std::size_t capture_offset = index * request.capture_byte_budget_per_entity;
        const std::span<std::uint8_t> capture_output =
            request.capture_output.subspan(capture_offset, request.capture_byte_budget_per_entity);

        yuengine::rendercore::RenderDrawableFramePipelineRequest render_request{};
        render_request.rhi_device = request.rhi_device;
        render_request.pipeline = request.material.pipeline;
        render_request.vertex_buffer = draws[index].draw.vertex_buffer;
        render_request.index_buffer = draws[index].draw.index_buffer;
        render_request.sampled_texture = texture_slot.sampled_texture;
        render_request.sampler = texture_slot.sampler;
        render_request.draw = draws[index].draw.draw;
        render_request.clear_color = request.camera.camera.clear_color;
        render_request.capture_output = capture_output;
        render_request.capture_byte_budget = request.capture_byte_budget_per_entity;
        render_request.frame_id = request.frame_id + static_cast<std::uint32_t>(index);
        render_request.pass_id = draws[index].draw.pass_id + static_cast<std::uint32_t>(index);
        render_request.material_id = request.material.material_id;

        yuengine::rendercore::RenderDrawableFramePipeline render_pipeline;
        result.render_results[index] = render_pipeline.Execute(render_request);
        ++result.render_result_count;
        result.capture_bytes_written += result.render_results[index].capture_bytes_written;
        const RenderSceneThreePrimitiveCaptureStatus render_status =
            MapRenderCoreResult(result.render_results[index], &result);
        if (render_status != RenderSceneThreePrimitiveCaptureStatus::Success) {
            *out_result = result;
            return render_status;
        }
    }

    result.entity_report_count = RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    result.status = RenderSceneThreePrimitiveCaptureStatus::Success;
    result.first_missing_layer = RenderSceneThreePrimitiveCaptureMissingLayer::None;
    result.output_status = RenderSceneThreePrimitiveCaptureOutputStatus::CaptureAvailable;
    *out_result = result;
    return result.status;
}

bool RenderSceneThreePrimitiveCaptureRoute::CopyOutputPath(
    const RenderSceneThreePrimitiveCaptureRequest &request,
    RenderSceneThreePrimitiveCaptureResult *out_result) const {
    if (out_result == nullptr) {
        return false;
    }

    if (request.output_path == nullptr) {
        return false;
    }

    if (request.output_path_byte_count == 0U) {
        return false;
    }

    if (request.output_path_byte_count >= MAX_RENDER_SCENE_THREE_PRIMITIVE_CAPTURE_OUTPUT_PATH_BYTES) {
        return false;
    }

    for (std::size_t index = 0U; index < request.output_path_byte_count; ++index) {
        out_result->output_path[index] = request.output_path[index];
    }

    out_result->output_path[request.output_path_byte_count] = '\0';
    out_result->output_path_byte_count = request.output_path_byte_count;
    out_result->output_status = RenderSceneThreePrimitiveCaptureOutputStatus::PathRecorded;
    return true;
}

RenderSceneThreePrimitiveCaptureStatus RenderSceneThreePrimitiveCaptureRoute::FailWithLayer(
    RenderSceneThreePrimitiveCaptureMissingLayer layer,
    RenderSceneThreePrimitiveCaptureResult *out_result) const {
    if (out_result == nullptr) {
        return RenderSceneThreePrimitiveCaptureStatus::InvalidArgument;
    }

    out_result->status = RenderSceneThreePrimitiveCaptureStatus::Fail;
    out_result->first_missing_layer = layer;
    return out_result->status;
}

RenderSceneThreePrimitiveCaptureMissingLayer RenderSceneThreePrimitiveCaptureRoute::MapMaterialLayer(
    RenderSceneRuntimeMaterialStatus status) const {
    if (status == RenderSceneRuntimeMaterialStatus::InvalidPipeline) {
        return RenderSceneThreePrimitiveCaptureMissingLayer::ShaderPipeline;
    }

    if (status == RenderSceneRuntimeMaterialStatus::MissingMaterialRecord) {
        return RenderSceneThreePrimitiveCaptureMissingLayer::MaterialTextureSlots;
    }

    if (status == RenderSceneRuntimeMaterialStatus::MissingTextureSlot) {
        return RenderSceneThreePrimitiveCaptureMissingLayer::MaterialTextureSlots;
    }

    return RenderSceneThreePrimitiveCaptureMissingLayer::MaterialTextureSlots;
}

RenderSceneThreePrimitiveCaptureMissingLayer RenderSceneThreePrimitiveCaptureRoute::MapFrameLayer(
    RenderSceneRuntimeFrameStatus status) const {
    if (status == RenderSceneRuntimeFrameStatus::MissingCamera) {
        return RenderSceneThreePrimitiveCaptureMissingLayer::Camera;
    }

    if (status == RenderSceneRuntimeFrameStatus::MissingGeometryRecord) {
        return RenderSceneThreePrimitiveCaptureMissingLayer::GeometryModel;
    }

    if (status == RenderSceneRuntimeFrameStatus::MissingMaterialRecord) {
        return RenderSceneThreePrimitiveCaptureMissingLayer::MaterialTextureSlots;
    }

    if (status == RenderSceneRuntimeFrameStatus::InvalidGeometryRecord) {
        return RenderSceneThreePrimitiveCaptureMissingLayer::GeometryModel;
    }

    if (status == RenderSceneRuntimeFrameStatus::InvalidMaterialRecord) {
        return RenderSceneThreePrimitiveCaptureMissingLayer::MaterialTextureSlots;
    }

    if (status == RenderSceneRuntimeFrameStatus::MissingEntity) {
        return RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement;
    }

    return RenderSceneThreePrimitiveCaptureMissingLayer::RenderSceneSubmission;
}

RenderSceneThreePrimitiveCaptureStatus RenderSceneThreePrimitiveCaptureRoute::MapRenderCoreResult(
    const yuengine::rendercore::RenderDrawableFramePipelineResult &render_result,
    RenderSceneThreePrimitiveCaptureResult *out_result) const {
    if (out_result == nullptr) {
        return RenderSceneThreePrimitiveCaptureStatus::InvalidArgument;
    }

    if (render_result.status == yuengine::rendercore::RenderDrawableFramePipelineStatus::Success) {
        out_result->status = RenderSceneThreePrimitiveCaptureStatus::Success;
        out_result->first_missing_layer = RenderSceneThreePrimitiveCaptureMissingLayer::None;
        out_result->output_status = RenderSceneThreePrimitiveCaptureOutputStatus::CaptureAvailable;
        return out_result->status;
    }

    if (render_result.status == yuengine::rendercore::RenderDrawableFramePipelineStatus::InvalidSwapchain) {
        out_result->status = RenderSceneThreePrimitiveCaptureStatus::BlockedByEnv;
        out_result->first_missing_layer = RenderSceneThreePrimitiveCaptureMissingLayer::RhiCaptureTarget;
        out_result->output_status = RenderSceneThreePrimitiveCaptureOutputStatus::BlockedByEnv;
        return out_result->status;
    }

    if (render_result.status == yuengine::rendercore::RenderDrawableFramePipelineStatus::RhiFailure) {
        out_result->status = RenderSceneThreePrimitiveCaptureStatus::BlockedByEnv;
        out_result->first_missing_layer = RenderSceneThreePrimitiveCaptureMissingLayer::RhiCaptureTarget;
        out_result->output_status = RenderSceneThreePrimitiveCaptureOutputStatus::BlockedByEnv;
        return out_result->status;
    }

    if (render_result.status == yuengine::rendercore::RenderDrawableFramePipelineStatus::MaterialBindingFailed) {
        out_result->status = RenderSceneThreePrimitiveCaptureStatus::Fail;
        out_result->first_missing_layer = RenderSceneThreePrimitiveCaptureMissingLayer::ShaderPipeline;
        return out_result->status;
    }

    out_result->status = RenderSceneThreePrimitiveCaptureStatus::Fail;
    out_result->first_missing_layer = RenderSceneThreePrimitiveCaptureMissingLayer::RenderCoreRhiDrawCapture;
    return out_result->status;
}
}
