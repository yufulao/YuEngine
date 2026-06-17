// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonOperation.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Defines explicit RenderCore render graph skeleton operation values.
 */
enum class RenderGraphSkeletonOperation {
    None,
    Prepare,
    Query,
    Release,
    Reset
};
}
