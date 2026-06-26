// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeFrameBuilder.h

#pragma once

#include <span>

#include "YuEngine/RenderScene/RenderSceneRuntimeFrameDrawRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameResult.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameStatus.h"

namespace yuengine::renderscene {
class RenderSceneRuntimeFrameBuilder final {
public:
    /**
     * @comment 构建 runtime multi-entity frame draw record 列表。
     * @param request 输入 runtime frame request。
     * @param out_draws 调用方持有的输出 draw record storage。
     * @param out_result 调用方持有的输出结果。
     * @return 显式构建状态。
     */
    RenderSceneRuntimeFrameStatus Build(
        const RenderSceneRuntimeFrameRequest &request,
        std::span<RenderSceneRuntimeFrameDrawRecord> out_draws,
        RenderSceneRuntimeFrameResult *out_result) const;

private:
    RenderSceneRuntimeFrameStatus ValidateRequest(
        const RenderSceneRuntimeFrameRequest &request,
        std::span<RenderSceneRuntimeFrameDrawRecord> out_draws,
        std::span<const RenderSceneRuntimeMaterialRecord> materials,
        std::size_t *out_visible_entity_count,
        RenderSceneRuntimeFrameResult *out_result) const;
    RenderSceneRuntimeFrameStatus ValidateMaterials(
        std::span<const RenderSceneRuntimeMaterialRecord> materials,
        RenderSceneRuntimeFrameResult *out_result) const;
    RenderSceneRuntimeFrameStatus ValidateEntity(
        const RenderSceneRuntimeFrameEntityRequest &entity,
        std::span<const RenderSceneRuntimeMaterialRecord> materials,
        std::uint32_t entity_index,
        RenderSceneRuntimeFrameResult *out_result) const;
    std::span<const RenderSceneRuntimeMaterialRecord> MaterialTableFor(
        const RenderSceneRuntimeFrameRequest &request) const;
    const RenderSceneRuntimeMaterialRecord *MaterialForEntity(
        const RenderSceneRuntimeFrameEntityRequest &entity,
        std::span<const RenderSceneRuntimeMaterialRecord> materials) const;
    bool HasDuplicateMaterialId(
        std::span<const RenderSceneRuntimeMaterialRecord> materials,
        std::size_t current_index) const;
    bool IsActiveEntity(const RenderSceneRuntimeFrameEntityRequest &entity) const;
    bool HasDuplicateWorldObject(
        const RenderSceneRuntimeFrameRequest &request,
        yuengine::world::WorldObjectId world_object_id,
        std::size_t current_index) const;
    bool HasDuplicateTransform(
        const RenderSceneRuntimeFrameRequest &request,
        const yuengine::world::WorldTransformState &transform,
        std::size_t current_index) const;
    bool IsSameTransform(
        const yuengine::world::WorldTransformState &left,
        const yuengine::world::WorldTransformState &right) const;
    void FillDrawRecord(
        const RenderSceneRuntimeFrameEntityRequest &entity,
        const RenderSceneRuntimeMaterialRecord &material,
        RenderSceneRuntimeFrameDrawRecord *out_draw) const;
};
}
