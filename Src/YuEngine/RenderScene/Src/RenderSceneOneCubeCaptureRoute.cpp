// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderSceneOneCubeCaptureRoute.cpp

#include "YuEngine/RenderScene/RenderSceneOneCubeCaptureRoute.h"

#include <array>
#include <cstddef>

#include "YuEngine/RenderCore/RenderDrawableFramePipeline.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineRequest.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryKind.h"
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

bool IsCaptureOutputValid(const RenderSceneOneCubeCaptureRequest &request) {
    if (request.capture_byte_budget == 0U) {
        return false;
    }

    if (request.capture_output.data() == nullptr) {
        return false;
    }

    return request.capture_output.size() >= request.capture_byte_budget;
}
}

RenderSceneOneCubeCaptureStatus RenderSceneOneCubeCaptureRoute::Execute(
    const RenderSceneOneCubeCaptureRequest &request,
    RenderSceneOneCubeCaptureResult *out_result) {
    if (out_result == nullptr) {
        return RenderSceneOneCubeCaptureStatus::InvalidArgument;
    }

    RenderSceneOneCubeCaptureResult result{};
    result.frame_id = request.frame_id;
    result.capture = request.camera.capture;
    result.capture.output_byte_budget = request.capture_byte_budget;

    if (!CopyOutputPath(request, &result)) {
        result.status = RenderSceneOneCubeCaptureStatus::Fail;
        result.first_missing_layer = RenderSceneOneCubeCaptureMissingLayer::OutputPath;
        *out_result = result;
        return result.status;
    }

    if (request.frame_id == 0U) {
        result.status = RenderSceneOneCubeCaptureStatus::InvalidArgument;
        *out_result = result;
        return result.status;
    }

    if (request.rhi_device == nullptr) {
        result.status = RenderSceneOneCubeCaptureStatus::BlockedByEnv;
        result.output_status = RenderSceneOneCubeCaptureOutputStatus::BlockedByEnv;
        result.first_missing_layer = RenderSceneOneCubeCaptureMissingLayer::RhiCaptureTarget;
        *out_result = result;
        return result.status;
    }

    if (!IsCaptureOutputValid(request)) {
        result.status = RenderSceneOneCubeCaptureStatus::InvalidArgument;
        *out_result = result;
        return result.status;
    }

    if (!IsCameraUsable(request.camera)) {
        *out_result = result;
        return FailWithLayer(RenderSceneOneCubeCaptureMissingLayer::Camera, out_result);
    }

    RenderScenePrimitiveGeometryBuilder geometry_builder;
    const RenderScenePrimitiveGeometryStatus geometry_status =
        geometry_builder.Validate(request.cube_geometry);
    if (geometry_status != RenderScenePrimitiveGeometryStatus::Success) {
        *out_result = result;
        return FailWithLayer(RenderSceneOneCubeCaptureMissingLayer::GeometryModel, out_result);
    }

    if (request.cube_geometry.kind != RenderScenePrimitiveGeometryKind::Cube) {
        *out_result = result;
        return FailWithLayer(RenderSceneOneCubeCaptureMissingLayer::GeometryModel, out_result);
    }

    RenderSceneRuntimeMaterialBuilder material_builder;
    const RenderSceneRuntimeMaterialStatus material_status = material_builder.Validate(request.material);
    if (material_status != RenderSceneRuntimeMaterialStatus::Success) {
        *out_result = result;
        const RenderSceneOneCubeCaptureMissingLayer layer = MapMaterialLayer(material_status);
        return FailWithLayer(layer, out_result);
    }

    RenderSceneRuntimeFrameEntityRequest entity{};
    entity.world_object_id = request.world_object_id;
    entity.transform = request.transform;
    entity.geometry = request.cube_geometry;
    entity.is_visible = true;
    entity.is_active = true;

    const std::array<RenderSceneRuntimeFrameEntityRequest, 1U> entities{entity};
    std::array<RenderSceneRuntimeFrameDrawRecord, 1U> draws{};

    RenderSceneRuntimeFrameRequest frame_request{};
    frame_request.frame_id = request.frame_id;
    frame_request.camera = request.camera;
    frame_request.material = request.material;
    frame_request.entities = entities;

    RenderSceneRuntimeFrameBuilder frame_builder;
    const RenderSceneRuntimeFrameStatus frame_status =
        frame_builder.Build(frame_request, draws, &result.frame_result);
    if (frame_status != RenderSceneRuntimeFrameStatus::Success) {
        *out_result = result;
        const RenderSceneOneCubeCaptureMissingLayer layer = MapFrameLayer(frame_status);
        return FailWithLayer(layer, out_result);
    }

    result.draw_record = draws[0U];

    const RenderSceneRuntimeMaterialTextureSlot &texture_slot = request.material.texture_slots[0U];
    yuengine::rendercore::RenderDrawableFramePipelineRequest render_request{};
    render_request.rhi_device = request.rhi_device;
    render_request.pipeline = request.material.pipeline;
    render_request.vertex_buffer = result.draw_record.draw.vertex_buffer;
    render_request.index_buffer = result.draw_record.draw.index_buffer;
    render_request.sampled_texture = texture_slot.sampled_texture;
    render_request.sampler = texture_slot.sampler;
    render_request.draw = result.draw_record.draw.draw;
    render_request.clear_color = request.camera.camera.clear_color;
    render_request.capture_output = request.capture_output;
    render_request.capture_byte_budget = request.capture_byte_budget;
    render_request.material_constant_bytes = std::span<const std::uint8_t>(
        request.material.material_constant_bytes.data(),
        request.material.material_constant_byte_count);
    render_request.frame_id = request.frame_id;
    render_request.pass_id = result.draw_record.draw.pass_id;
    render_request.material_id = request.material.material_id;

    yuengine::rendercore::RenderDrawableFramePipeline render_pipeline;
    result.render_result = render_pipeline.Execute(render_request);
    result.capture_bytes_written = result.render_result.capture_bytes_written;
    *out_result = result;
    return MapRenderCoreResult(result.render_result, out_result);
}

bool RenderSceneOneCubeCaptureRoute::CopyOutputPath(
    const RenderSceneOneCubeCaptureRequest &request,
    RenderSceneOneCubeCaptureResult *out_result) const {
    if (out_result == nullptr) {
        return false;
    }

    if (request.output_path == nullptr) {
        return false;
    }

    if (request.output_path_byte_count == 0U) {
        return false;
    }

    if (request.output_path_byte_count >= MAX_RENDER_SCENE_ONE_CUBE_CAPTURE_OUTPUT_PATH_BYTES) {
        return false;
    }

    for (std::size_t index = 0U; index < request.output_path_byte_count; ++index) {
        out_result->output_path[index] = request.output_path[index];
    }

    out_result->output_path[request.output_path_byte_count] = '\0';
    out_result->output_path_byte_count = request.output_path_byte_count;
    out_result->output_status = RenderSceneOneCubeCaptureOutputStatus::PathRecorded;
    return true;
}

RenderSceneOneCubeCaptureStatus RenderSceneOneCubeCaptureRoute::FailWithLayer(
    RenderSceneOneCubeCaptureMissingLayer layer,
    RenderSceneOneCubeCaptureResult *out_result) const {
    if (out_result == nullptr) {
        return RenderSceneOneCubeCaptureStatus::InvalidArgument;
    }

    out_result->status = RenderSceneOneCubeCaptureStatus::Fail;
    out_result->first_missing_layer = layer;
    return out_result->status;
}

RenderSceneOneCubeCaptureMissingLayer RenderSceneOneCubeCaptureRoute::MapMaterialLayer(
    RenderSceneRuntimeMaterialStatus status) const {
    if (status == RenderSceneRuntimeMaterialStatus::InvalidPipeline) {
        return RenderSceneOneCubeCaptureMissingLayer::ShaderPipeline;
    }

    if (status == RenderSceneRuntimeMaterialStatus::MissingMaterialRecord) {
        return RenderSceneOneCubeCaptureMissingLayer::MaterialTextureSlots;
    }

    if (status == RenderSceneRuntimeMaterialStatus::MissingTextureSlot) {
        return RenderSceneOneCubeCaptureMissingLayer::MaterialTextureSlots;
    }

    return RenderSceneOneCubeCaptureMissingLayer::MaterialTextureSlots;
}

RenderSceneOneCubeCaptureMissingLayer RenderSceneOneCubeCaptureRoute::MapFrameLayer(
    RenderSceneRuntimeFrameStatus status) const {
    if (status == RenderSceneRuntimeFrameStatus::MissingCamera) {
        return RenderSceneOneCubeCaptureMissingLayer::Camera;
    }

    if (status == RenderSceneRuntimeFrameStatus::MissingGeometryRecord) {
        return RenderSceneOneCubeCaptureMissingLayer::GeometryModel;
    }

    if (status == RenderSceneRuntimeFrameStatus::MissingMaterialRecord) {
        return RenderSceneOneCubeCaptureMissingLayer::MaterialTextureSlots;
    }

    if (status == RenderSceneRuntimeFrameStatus::InvalidGeometryRecord) {
        return RenderSceneOneCubeCaptureMissingLayer::GeometryModel;
    }

    if (status == RenderSceneRuntimeFrameStatus::InvalidMaterialRecord) {
        return RenderSceneOneCubeCaptureMissingLayer::MaterialTextureSlots;
    }

    if (status == RenderSceneRuntimeFrameStatus::MissingEntity) {
        return RenderSceneOneCubeCaptureMissingLayer::ScenePlacement;
    }

    return RenderSceneOneCubeCaptureMissingLayer::RenderSceneSubmission;
}

RenderSceneOneCubeCaptureStatus RenderSceneOneCubeCaptureRoute::MapRenderCoreResult(
    const yuengine::rendercore::RenderDrawableFramePipelineResult &render_result,
    RenderSceneOneCubeCaptureResult *out_result) const {
    if (out_result == nullptr) {
        return RenderSceneOneCubeCaptureStatus::InvalidArgument;
    }

    if (render_result.status == yuengine::rendercore::RenderDrawableFramePipelineStatus::Success) {
        out_result->status = RenderSceneOneCubeCaptureStatus::Success;
        out_result->first_missing_layer = RenderSceneOneCubeCaptureMissingLayer::None;
        out_result->output_status = RenderSceneOneCubeCaptureOutputStatus::CaptureAvailable;
        return out_result->status;
    }

    if (render_result.status == yuengine::rendercore::RenderDrawableFramePipelineStatus::InvalidSwapchain) {
        out_result->status = RenderSceneOneCubeCaptureStatus::BlockedByEnv;
        out_result->first_missing_layer = RenderSceneOneCubeCaptureMissingLayer::RhiCaptureTarget;
        out_result->output_status = RenderSceneOneCubeCaptureOutputStatus::BlockedByEnv;
        return out_result->status;
    }

    if (render_result.status == yuengine::rendercore::RenderDrawableFramePipelineStatus::RhiFailure) {
        out_result->status = RenderSceneOneCubeCaptureStatus::BlockedByEnv;
        out_result->first_missing_layer = RenderSceneOneCubeCaptureMissingLayer::RhiCaptureTarget;
        out_result->output_status = RenderSceneOneCubeCaptureOutputStatus::BlockedByEnv;
        return out_result->status;
    }

    if (render_result.status == yuengine::rendercore::RenderDrawableFramePipelineStatus::MaterialBindingFailed) {
        out_result->status = RenderSceneOneCubeCaptureStatus::Fail;
        out_result->first_missing_layer = RenderSceneOneCubeCaptureMissingLayer::ShaderPipeline;
        return out_result->status;
    }

    out_result->status = RenderSceneOneCubeCaptureStatus::Fail;
    out_result->first_missing_layer = RenderSceneOneCubeCaptureMissingLayer::RenderCoreRhiDrawCapture;
    return out_result->status;
}
}
