// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderSceneContractQueue.cpp

#include "YuEngine/RenderScene/RenderSceneContractQueue.h"

namespace yuengine::renderscene {
RenderSceneStatus RenderSceneContractQueue::BuildRenderCorePackets(
    const RenderSceneSubmitRequest &request,
    std::span<yuengine::rendercore::RenderViewPacketRequest> out_packets,
    RenderSceneSubmitResult *out_result) {
    RenderSceneSubmitResult result{};
    result.frame_id = request.frame_id;

    if (out_result == nullptr) {
        return RecordFailure(RenderSceneStatus::NullPointer, result);
    }

    const RenderSceneCameraRecord *camera = nullptr;
    std::size_t visible_entity_count = 0U;
    const RenderSceneStatus validate_status =
        ValidateRequest(request, out_packets, &camera, &visible_entity_count);
    if (validate_status != RenderSceneStatus::Success) {
        result.status = validate_status;
        *out_result = result;
        return RecordFailure(validate_status, result);
    }

    result.camera_id = camera->camera_id;
    result.visible_entity_count = visible_entity_count;

    std::size_t output_index = 0U;
    for (const RenderSceneEntityRecord &entity : request.entities) {
        if (!IsActiveEntity(entity)) {
            ++result.skipped_entity_count;
            continue;
        }

        FillPacket(request, *camera, entity, &out_packets[output_index]);
        ++output_index;
    }

    result.output_packet_count = output_index;
    result.status = RenderSceneStatus::Success;
    *out_result = result;
    return RecordSuccess(result);
}

RenderSceneSnapshot RenderSceneContractQueue::Snapshot() const {
    return snapshot_;
}

RenderSceneStatus RenderSceneContractQueue::ValidateRequest(
    const RenderSceneSubmitRequest &request,
    std::span<yuengine::rendercore::RenderViewPacketRequest> out_packets,
    const RenderSceneCameraRecord **out_camera,
    std::size_t *out_visible_entity_count) const {
    if (out_camera == nullptr) {
        return RenderSceneStatus::NullPointer;
    }

    if (out_visible_entity_count == nullptr) {
        return RenderSceneStatus::NullPointer;
    }

    if (request.frame_id == 0U) {
        return RenderSceneStatus::InvalidFrameId;
    }

    const RenderSceneCameraRecord *camera = FindCamera(request);
    if (camera == nullptr) {
        return RenderSceneStatus::MissingCamera;
    }

    std::size_t visible_entity_count = 0U;
    for (const RenderSceneEntityRecord &entity : request.entities) {
        if (!IsActiveEntity(entity)) {
            continue;
        }

        const RenderSceneStatus entity_status = ValidateEntity(entity);
        if (entity_status != RenderSceneStatus::Success) {
            return entity_status;
        }

        ++visible_entity_count;
    }

    if (visible_entity_count == 0U) {
        return RenderSceneStatus::MissingEntity;
    }

    if (visible_entity_count > out_packets.size()) {
        return RenderSceneStatus::OutputCapacityExceeded;
    }

    *out_camera = camera;
    *out_visible_entity_count = visible_entity_count;
    return RenderSceneStatus::Success;
}

RenderSceneStatus RenderSceneContractQueue::ValidateEntity(const RenderSceneEntityRecord &entity) const {
    if (!entity.world_object_id.IsValid()) {
        return RenderSceneStatus::InvalidEntityRecord;
    }

    if (!entity.mesh_asset.IsValid()) {
        return RenderSceneStatus::MissingMeshAsset;
    }

    if (!entity.material_asset.IsValid()) {
        return RenderSceneStatus::MissingMaterialAsset;
    }

    if (!entity.texture_ready.is_ready) {
        return RenderSceneStatus::MissingTextureReadyRecord;
    }

    if (entity.material.material_id == 0U) {
        return RenderSceneStatus::InvalidMaterialRecord;
    }

    if (entity.material.pass_id == 0U) {
        return RenderSceneStatus::InvalidMaterialRecord;
    }

    if (entity.material.program_id == 0U) {
        return RenderSceneStatus::InvalidMaterialRecord;
    }

    if (entity.draw.draw_id == 0U) {
        return RenderSceneStatus::InvalidDrawRecord;
    }

    if (entity.draw.pass_id != entity.material.pass_id) {
        return RenderSceneStatus::InvalidDrawRecord;
    }

    if (entity.draw.material_id != entity.material.material_id) {
        return RenderSceneStatus::InvalidDrawRecord;
    }

    if (entity.draw.draw.index_count == 0U) {
        return RenderSceneStatus::InvalidDrawRecord;
    }

    return RenderSceneStatus::Success;
}

const RenderSceneCameraRecord *RenderSceneContractQueue::FindCamera(
    const RenderSceneSubmitRequest &request) const {
    for (const RenderSceneCameraRecord &camera : request.cameras) {
        if (!camera.is_active) {
            continue;
        }

        if (camera.camera_id != request.active_camera_id) {
            continue;
        }

        return &camera;
    }

    return nullptr;
}

bool RenderSceneContractQueue::IsActiveEntity(const RenderSceneEntityRecord &entity) const {
    if (!entity.is_active) {
        return false;
    }

    return entity.is_visible;
}

void RenderSceneContractQueue::FillPacket(
    const RenderSceneSubmitRequest &request,
    const RenderSceneCameraRecord &camera,
    const RenderSceneEntityRecord &entity,
    yuengine::rendercore::RenderViewPacketRequest *out_packet) const {
    if (out_packet == nullptr) {
        return;
    }

    yuengine::rendercore::RenderMaterialRequest material = entity.material;
    material.sampled_texture = entity.texture_ready.sampled_texture;

    out_packet->view_id = entity.draw.draw_id;
    out_packet->frame_id = request.frame_id;
    out_packet->target = camera.target;
    out_packet->clear_color = camera.clear_color;
    out_packet->capture_output = request.capture_output;
    out_packet->capture_byte_budget = request.capture_byte_budget;
    out_packet->material = material;
    out_packet->draw = entity.draw;
}

RenderSceneStatus RenderSceneContractQueue::RecordSuccess(const RenderSceneSubmitResult &result) {
    ++snapshot_.submit_count;
    snapshot_.last_frame_id = result.frame_id;
    snapshot_.last_camera_id = result.camera_id;
    snapshot_.last_output_packet_count = result.output_packet_count;
    snapshot_.last_visible_entity_count = result.visible_entity_count;
    snapshot_.last_skipped_entity_count = result.skipped_entity_count;
    snapshot_.last_status = RenderSceneStatus::Success;
    return RenderSceneStatus::Success;
}

RenderSceneStatus RenderSceneContractQueue::RecordFailure(
    RenderSceneStatus status,
    const RenderSceneSubmitResult &result) {
    ++snapshot_.failed_submit_count;
    snapshot_.last_frame_id = result.frame_id;
    snapshot_.last_camera_id = result.camera_id;
    snapshot_.last_output_packet_count = result.output_packet_count;
    snapshot_.last_visible_entity_count = result.visible_entity_count;
    snapshot_.last_skipped_entity_count = result.skipped_entity_count;
    snapshot_.last_status = status;
    return status;
}
}
