// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderSceneRuntimeFrameBuilder.cpp

#include "YuEngine/RenderScene/RenderSceneRuntimeFrameBuilder.h"

#include <cstddef>

#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h"

namespace yuengine::renderscene {
RenderSceneRuntimeFrameStatus RenderSceneRuntimeFrameBuilder::Build(
    const RenderSceneRuntimeFrameRequest &request,
    std::span<RenderSceneRuntimeFrameDrawRecord> out_draws,
    RenderSceneRuntimeFrameResult *out_result) const {
    RenderSceneRuntimeFrameResult result{};
    result.frame_id = request.frame_id;

    if (out_result == nullptr) {
        return RenderSceneRuntimeFrameStatus::NullPointer;
    }

    std::size_t visible_entity_count = 0U;
    const RenderSceneRuntimeFrameStatus status =
        ValidateRequest(request, out_draws, &visible_entity_count);
    if (status != RenderSceneRuntimeFrameStatus::Success) {
        result.status = status;
        *out_result = result;
        return status;
    }

    result.camera_id = request.camera.camera.camera_id;
    result.material_id = request.material.material_id;
    result.material_texture_slot_count = request.material.texture_slot_count;
    result.submitted_entity_count = visible_entity_count;

    std::size_t output_index = 0U;
    for (const RenderSceneRuntimeFrameEntityRequest &entity : request.entities) {
        if (!IsActiveEntity(entity)) {
            ++result.skipped_entity_count;
            continue;
        }

        FillDrawRecord(entity, request.material, &out_draws[output_index]);
        ++output_index;
    }

    result.output_draw_count = output_index;
    result.status = RenderSceneRuntimeFrameStatus::Success;
    *out_result = result;
    return RenderSceneRuntimeFrameStatus::Success;
}

RenderSceneRuntimeFrameStatus RenderSceneRuntimeFrameBuilder::ValidateRequest(
    const RenderSceneRuntimeFrameRequest &request,
    std::span<RenderSceneRuntimeFrameDrawRecord> out_draws,
    std::size_t *out_visible_entity_count) const {
    if (out_visible_entity_count == nullptr) {
        return RenderSceneRuntimeFrameStatus::NullPointer;
    }

    if (request.frame_id == 0U) {
        return RenderSceneRuntimeFrameStatus::InvalidFrameId;
    }

    if (request.camera.status != RenderSceneStatus::Success) {
        return RenderSceneRuntimeFrameStatus::MissingCamera;
    }

    if (!request.camera.camera.is_active || request.camera.camera.camera_id == 0U) {
        return RenderSceneRuntimeFrameStatus::MissingCamera;
    }

    RenderSceneRuntimeMaterialBuilder material_builder;
    const RenderSceneRuntimeMaterialStatus material_status = material_builder.Validate(request.material);
    if (material_status == RenderSceneRuntimeMaterialStatus::MissingMaterialRecord) {
        return RenderSceneRuntimeFrameStatus::MissingMaterialRecord;
    }

    if (material_status != RenderSceneRuntimeMaterialStatus::Success) {
        return RenderSceneRuntimeFrameStatus::InvalidMaterialRecord;
    }

    std::size_t visible_entity_count = 0U;
    for (std::size_t index = 0U; index < request.entities.size(); ++index) {
        const RenderSceneRuntimeFrameEntityRequest &entity = request.entities[index];
        if (!IsActiveEntity(entity)) {
            continue;
        }

        const RenderSceneRuntimeFrameStatus status = ValidateEntity(entity);
        if (status != RenderSceneRuntimeFrameStatus::Success) {
            return status;
        }

        if (HasDuplicateWorldObject(request, entity.world_object_id, index)) {
            return RenderSceneRuntimeFrameStatus::DuplicateWorldObject;
        }

        if (HasDuplicateTransform(request, entity.transform, index)) {
            return RenderSceneRuntimeFrameStatus::DuplicateTransform;
        }

        ++visible_entity_count;
    }

    if (visible_entity_count == 0U) {
        return RenderSceneRuntimeFrameStatus::MissingEntity;
    }

    if (visible_entity_count > out_draws.size()) {
        return RenderSceneRuntimeFrameStatus::OutputCapacityExceeded;
    }

    *out_visible_entity_count = visible_entity_count;
    return RenderSceneRuntimeFrameStatus::Success;
}

RenderSceneRuntimeFrameStatus RenderSceneRuntimeFrameBuilder::ValidateEntity(
    const RenderSceneRuntimeFrameEntityRequest &entity) const {
    if (!entity.world_object_id.IsValid()) {
        return RenderSceneRuntimeFrameStatus::InvalidEntityRecord;
    }

    RenderScenePrimitiveGeometryBuilder geometry_builder;
    const RenderScenePrimitiveGeometryStatus geometry_status = geometry_builder.Validate(entity.geometry);
    if (geometry_status == RenderScenePrimitiveGeometryStatus::MissingGeometryRecord) {
        return RenderSceneRuntimeFrameStatus::MissingGeometryRecord;
    }

    if (geometry_status != RenderScenePrimitiveGeometryStatus::Success) {
        return RenderSceneRuntimeFrameStatus::InvalidGeometryRecord;
    }

    return RenderSceneRuntimeFrameStatus::Success;
}

bool RenderSceneRuntimeFrameBuilder::IsActiveEntity(
    const RenderSceneRuntimeFrameEntityRequest &entity) const {
    if (!entity.is_active) {
        return false;
    }

    return entity.is_visible;
}

bool RenderSceneRuntimeFrameBuilder::HasDuplicateWorldObject(
    const RenderSceneRuntimeFrameRequest &request,
    yuengine::world::WorldObjectId world_object_id,
    std::size_t current_index) const {
    for (std::size_t index = current_index + 1U; index < request.entities.size(); ++index) {
        const RenderSceneRuntimeFrameEntityRequest &entity = request.entities[index];
        if (!IsActiveEntity(entity)) {
            continue;
        }

        if (entity.world_object_id.value == world_object_id.value) {
            return true;
        }
    }

    return false;
}

bool RenderSceneRuntimeFrameBuilder::HasDuplicateTransform(
    const RenderSceneRuntimeFrameRequest &request,
    const yuengine::world::WorldTransformState &transform,
    std::size_t current_index) const {
    for (std::size_t index = current_index + 1U; index < request.entities.size(); ++index) {
        const RenderSceneRuntimeFrameEntityRequest &entity = request.entities[index];
        if (!IsActiveEntity(entity)) {
            continue;
        }

        if (IsSameTransform(entity.transform, transform)) {
            return true;
        }
    }

    return false;
}

bool RenderSceneRuntimeFrameBuilder::IsSameTransform(
    const yuengine::world::WorldTransformState &left,
    const yuengine::world::WorldTransformState &right) const {
    if (left.translation_x != right.translation_x) {
        return false;
    }

    if (left.translation_y != right.translation_y) {
        return false;
    }

    if (left.translation_z != right.translation_z) {
        return false;
    }

    if (left.rotation_x != right.rotation_x) {
        return false;
    }

    if (left.rotation_y != right.rotation_y) {
        return false;
    }

    if (left.rotation_z != right.rotation_z) {
        return false;
    }

    if (left.rotation_w != right.rotation_w) {
        return false;
    }

    if (left.scale_x != right.scale_x) {
        return false;
    }

    if (left.scale_y != right.scale_y) {
        return false;
    }

    return left.scale_z == right.scale_z;
}

void RenderSceneRuntimeFrameBuilder::FillDrawRecord(
    const RenderSceneRuntimeFrameEntityRequest &entity,
    const RenderSceneRuntimeMaterialRecord &material,
    RenderSceneRuntimeFrameDrawRecord *out_draw) const {
    if (out_draw == nullptr) {
        return;
    }

    out_draw->world_object_id = entity.world_object_id;
    out_draw->transform = entity.transform;
    out_draw->geometry_kind = entity.geometry.kind;
    out_draw->draw = entity.geometry.draw;
    out_draw->draw.material_id = material.material_id;
}
}
