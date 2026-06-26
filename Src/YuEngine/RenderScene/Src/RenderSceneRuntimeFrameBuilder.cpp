// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderSceneRuntimeFrameBuilder.cpp

#include "YuEngine/RenderScene/RenderSceneRuntimeFrameBuilder.h"

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h"

namespace yuengine::renderscene {
namespace {
constexpr std::uint32_t RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX = 0xFFFFFFFFU;

RenderSceneRuntimeFrameStatus MapRuntimeFrameMaterialStatus(RenderSceneRuntimeMaterialStatus status) {
    if (status == RenderSceneRuntimeMaterialStatus::MissingMaterialRecord) {
        return RenderSceneRuntimeFrameStatus::MissingMaterialRecord;
    }

    return RenderSceneRuntimeFrameStatus::InvalidMaterialRecord;
}

void SetFrameFailure(
    RenderSceneRuntimeFrameResult *result,
    RenderSceneRuntimeFrameStatus status,
    std::uint32_t entity_index,
    std::uint32_t material_index) {
    if (result == nullptr) {
        return;
    }

    result->status = status;
    result->first_failed_entity_index = entity_index;
    result->first_failed_material_index = material_index;
}
}

RenderSceneRuntimeFrameStatus RenderSceneRuntimeFrameBuilder::Build(
    const RenderSceneRuntimeFrameRequest &request,
    std::span<RenderSceneRuntimeFrameDrawRecord> out_draws,
    RenderSceneRuntimeFrameResult *out_result) const {
    RenderSceneRuntimeFrameResult result{};
    result.frame_id = request.frame_id;

    if (out_result == nullptr) {
        return RenderSceneRuntimeFrameStatus::NullPointer;
    }

    const std::span<const RenderSceneRuntimeMaterialRecord> materials = MaterialTableFor(request);
    result.material_count = materials.size();
    result.material_variant_count = materials.size();

    std::size_t visible_entity_count = 0U;
    const RenderSceneRuntimeFrameStatus status =
        ValidateRequest(request, out_draws, materials, &visible_entity_count, &result);
    if (status != RenderSceneRuntimeFrameStatus::Success) {
        result.status = status;
        *out_result = result;
        return status;
    }

    result.camera_id = request.camera.camera.camera_id;
    result.material_id = materials[0U].material_id;
    result.material_texture_slot_count = materials[0U].texture_slot_count;
    result.submitted_entity_count = visible_entity_count;

    std::size_t output_index = 0U;
    for (const RenderSceneRuntimeFrameEntityRequest &entity : request.entities) {
        if (!IsActiveEntity(entity)) {
            ++result.skipped_entity_count;
            continue;
        }

        const RenderSceneRuntimeMaterialRecord *material = MaterialForEntity(entity, materials);
        if (material == nullptr) {
            SetFrameFailure(
                &result,
                RenderSceneRuntimeFrameStatus::MaterialIndexOutOfRange,
                RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX,
                entity.material_table_index);
            *out_result = result;
            return result.status;
        }

        FillDrawRecord(entity, *material, &out_draws[output_index]);
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
    std::span<const RenderSceneRuntimeMaterialRecord> materials,
    std::size_t *out_visible_entity_count,
    RenderSceneRuntimeFrameResult *out_result) const {
    if (out_visible_entity_count == nullptr || out_result == nullptr) {
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

    const RenderSceneRuntimeFrameStatus material_status =
        ValidateMaterials(materials, out_result);
    if (material_status != RenderSceneRuntimeFrameStatus::Success) {
        return material_status;
    }

    std::size_t visible_entity_count = 0U;
    for (std::size_t index = 0U; index < request.entities.size(); ++index) {
        const RenderSceneRuntimeFrameEntityRequest &entity = request.entities[index];
        if (!IsActiveEntity(entity)) {
            continue;
        }

        const std::uint32_t entity_index = static_cast<std::uint32_t>(index);
        const RenderSceneRuntimeFrameStatus status =
            ValidateEntity(entity, materials, entity_index, out_result);
        if (status != RenderSceneRuntimeFrameStatus::Success) {
            return status;
        }

        if (HasDuplicateWorldObject(request, entity.world_object_id, index)) {
            SetFrameFailure(
                out_result,
                RenderSceneRuntimeFrameStatus::DuplicateWorldObject,
                entity_index,
                RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX);
            return RenderSceneRuntimeFrameStatus::DuplicateWorldObject;
        }

        if (HasDuplicateTransform(request, entity.transform, index)) {
            SetFrameFailure(
                out_result,
                RenderSceneRuntimeFrameStatus::DuplicateTransform,
                entity_index,
                RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX);
            return RenderSceneRuntimeFrameStatus::DuplicateTransform;
        }

        ++visible_entity_count;
    }

    if (visible_entity_count == 0U) {
        SetFrameFailure(
            out_result,
            RenderSceneRuntimeFrameStatus::MissingEntity,
            RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX,
            RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX);
        return RenderSceneRuntimeFrameStatus::MissingEntity;
    }

    if (visible_entity_count > out_draws.size()) {
        SetFrameFailure(
            out_result,
            RenderSceneRuntimeFrameStatus::OutputCapacityExceeded,
            RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX,
            RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX);
        return RenderSceneRuntimeFrameStatus::OutputCapacityExceeded;
    }

    *out_visible_entity_count = visible_entity_count;
    return RenderSceneRuntimeFrameStatus::Success;
}

RenderSceneRuntimeFrameStatus RenderSceneRuntimeFrameBuilder::ValidateMaterials(
    std::span<const RenderSceneRuntimeMaterialRecord> materials,
    RenderSceneRuntimeFrameResult *out_result) const {
    if (materials.empty() || materials.data() == nullptr) {
        SetFrameFailure(
            out_result,
            RenderSceneRuntimeFrameStatus::MissingMaterialRecord,
            RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX,
            RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX);
        return RenderSceneRuntimeFrameStatus::MissingMaterialRecord;
    }

    RenderSceneRuntimeMaterialBuilder material_builder;
    for (std::size_t index = 0U; index < materials.size(); ++index) {
        const RenderSceneRuntimeMaterialStatus material_status =
            material_builder.Validate(materials[index]);
        if (material_status != RenderSceneRuntimeMaterialStatus::Success) {
            const RenderSceneRuntimeFrameStatus frame_status =
                MapRuntimeFrameMaterialStatus(material_status);
            SetFrameFailure(
                out_result,
                frame_status,
                RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX,
                static_cast<std::uint32_t>(index));
            return frame_status;
        }

        if (HasDuplicateMaterialId(materials, index)) {
            SetFrameFailure(
                out_result,
                RenderSceneRuntimeFrameStatus::DuplicateMaterialRecord,
                RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX,
                static_cast<std::uint32_t>(index));
            return RenderSceneRuntimeFrameStatus::DuplicateMaterialRecord;
        }
    }

    return RenderSceneRuntimeFrameStatus::Success;
}

RenderSceneRuntimeFrameStatus RenderSceneRuntimeFrameBuilder::ValidateEntity(
    const RenderSceneRuntimeFrameEntityRequest &entity,
    std::span<const RenderSceneRuntimeMaterialRecord> materials,
    std::uint32_t entity_index,
    RenderSceneRuntimeFrameResult *out_result) const {
    if (!entity.world_object_id.IsValid()) {
        SetFrameFailure(
            out_result,
            RenderSceneRuntimeFrameStatus::InvalidEntityRecord,
            entity_index,
            RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX);
        return RenderSceneRuntimeFrameStatus::InvalidEntityRecord;
    }

    const std::size_t material_index = static_cast<std::size_t>(entity.material_table_index);
    if (material_index >= materials.size()) {
        SetFrameFailure(
            out_result,
            RenderSceneRuntimeFrameStatus::MaterialIndexOutOfRange,
            entity_index,
            entity.material_table_index);
        return RenderSceneRuntimeFrameStatus::MaterialIndexOutOfRange;
    }

    RenderScenePrimitiveGeometryBuilder geometry_builder;
    const RenderScenePrimitiveGeometryStatus geometry_status = geometry_builder.Validate(entity.geometry);
    if (geometry_status == RenderScenePrimitiveGeometryStatus::MissingGeometryRecord) {
        SetFrameFailure(
            out_result,
            RenderSceneRuntimeFrameStatus::MissingGeometryRecord,
            entity_index,
            RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX);
        return RenderSceneRuntimeFrameStatus::MissingGeometryRecord;
    }

    if (geometry_status != RenderScenePrimitiveGeometryStatus::Success) {
        SetFrameFailure(
            out_result,
            RenderSceneRuntimeFrameStatus::InvalidGeometryRecord,
            entity_index,
            RENDER_SCENE_RUNTIME_FRAME_INVALID_INDEX);
        return RenderSceneRuntimeFrameStatus::InvalidGeometryRecord;
    }

    return RenderSceneRuntimeFrameStatus::Success;
}

std::span<const RenderSceneRuntimeMaterialRecord> RenderSceneRuntimeFrameBuilder::MaterialTableFor(
    const RenderSceneRuntimeFrameRequest &request) const {
    if (!request.materials.empty()) {
        return request.materials;
    }

    return std::span<const RenderSceneRuntimeMaterialRecord>(&request.material, 1U);
}

const RenderSceneRuntimeMaterialRecord *RenderSceneRuntimeFrameBuilder::MaterialForEntity(
    const RenderSceneRuntimeFrameEntityRequest &entity,
    std::span<const RenderSceneRuntimeMaterialRecord> materials) const {
    const std::size_t material_index = static_cast<std::size_t>(entity.material_table_index);
    if (material_index >= materials.size()) {
        return nullptr;
    }

    return &materials[material_index];
}

bool RenderSceneRuntimeFrameBuilder::HasDuplicateMaterialId(
    std::span<const RenderSceneRuntimeMaterialRecord> materials,
    std::size_t current_index) const {
    const std::uint32_t material_id = materials[current_index].material_id;
    for (std::size_t index = current_index + 1U; index < materials.size(); ++index) {
        if (materials[index].material_id == material_id) {
            return true;
        }
    }

    return false;
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
