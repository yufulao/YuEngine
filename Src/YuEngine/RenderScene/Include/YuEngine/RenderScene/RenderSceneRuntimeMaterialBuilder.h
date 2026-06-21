// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h

#pragma once

#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialTextureSlot.h"

namespace yuengine::renderscene {
class RenderSceneRuntimeMaterialBuilder final {
public:
    /**
     * @comment 构建 runtime material texture-slot record。
     * @param request 输入 material request。
     * @param out_record 调用方持有的输出 record。
     * @return 显式构建状态。
     */
    RenderSceneRuntimeMaterialStatus Build(
        const RenderSceneRuntimeMaterialRequest &request,
        RenderSceneRuntimeMaterialRecord *out_record) const;

    /**
     * @comment 验证 runtime material record 是否可供后续 RenderScene/RenderCore 消费。
     * @param record 输入 material record。
     * @return 显式验证状态。
     */
    RenderSceneRuntimeMaterialStatus Validate(
        const RenderSceneRuntimeMaterialRecord &record) const;

private:
    RenderSceneRuntimeMaterialStatus ValidateRequest(
        const RenderSceneRuntimeMaterialRequest &request) const;
    RenderSceneRuntimeMaterialStatus ValidateTextureSlot(
        const RenderSceneRuntimeMaterialTextureSlot &texture_slot) const;
    bool HasDuplicateTextureSlot(
        const RenderSceneRuntimeMaterialRequest &request,
        std::uint32_t slot,
        std::size_t current_index) const;
    void InsertTextureSlotSorted(
        const RenderSceneRuntimeMaterialTextureSlot &texture_slot,
        RenderSceneRuntimeMaterialRecord *out_record) const;
};
}
