// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlanOperation.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 定义显式 RenderCore render graph execution-plan 操作。
 */
enum class RenderGraphExecutionPlanOperation {
    None,
    Execute,
    Query,
    Release,
    Reset
};
}
