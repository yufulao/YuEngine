// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlanDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderGraphExecutionPlanConstants.h"

namespace yuengine::rendercore {
/**
 * @comment 描述 固定容量 RenderCore render graph execution-plan 存储.
 */
struct RenderGraphExecutionPlanDesc final {
    std::size_t plan_record_capacity = MAX_RENDER_GRAPH_EXECUTION_PLAN_RECORDS;
};
}
