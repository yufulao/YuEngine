// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h

#pragma once

#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRecord.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRequest.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryStatus.h"

namespace yuengine::renderscene {
class RenderScenePrimitiveGeometryBuilder final {
public:
    /**
     * @comment 构建 cube/cylinder/cone runtime primitive geometry record。
     * @param request 输入 primitive geometry request。
     * @param out_record 调用方持有的输出 record。
     * @return 显式构建状态。
     */
    RenderScenePrimitiveGeometryStatus Build(
        const RenderScenePrimitiveGeometryRequest &request,
        RenderScenePrimitiveGeometryRecord *out_record) const;

    /**
     * @comment 验证 primitive geometry record 是否可供 RenderScene 消费。
     * @param record 输入 primitive geometry record。
     * @return 显式验证状态。
     */
    RenderScenePrimitiveGeometryStatus Validate(
        const RenderScenePrimitiveGeometryRecord &record) const;

private:
    RenderScenePrimitiveGeometryStatus ValidateRequest(
        const RenderScenePrimitiveGeometryRequest &request) const;
    void FillCounts(
        const RenderScenePrimitiveGeometryRequest &request,
        RenderScenePrimitiveGeometryRecord *out_record) const;
};
}
