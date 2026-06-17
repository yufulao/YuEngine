// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlanDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderGraphExecutionPlanConstants.h"

namespace yuengine::rendercore {
/**
 * @comment Describes bounded RenderCore render graph execution-plan storage.
 */
struct RenderGraphExecutionPlanDesc final {
    std::size_t plan_record_capacity = MAX_RENDER_GRAPH_EXECUTION_PLAN_RECORDS;
};
}
