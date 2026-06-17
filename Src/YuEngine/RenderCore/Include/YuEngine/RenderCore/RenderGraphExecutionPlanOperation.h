// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlanOperation.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Defines explicit RenderCore render graph execution-plan operations.
 */
enum class RenderGraphExecutionPlanOperation {
    None,
    Execute,
    Query,
    Release,
    Reset
};
}
