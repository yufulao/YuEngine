// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonOperation.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 定义显式 RenderCore render graph skeleton 操作值。
 */
enum class RenderGraphSkeletonOperation {
    None,
    Prepare,
    Query,
    Release,
    Reset
};
}
