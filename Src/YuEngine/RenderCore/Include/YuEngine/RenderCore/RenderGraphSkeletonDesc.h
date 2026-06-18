// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderGraphSkeletonConstants.h"

namespace yuengine::rendercore {
/**
 * @comment 配置 固定容量 存储 用于 一个 RenderCore render graph skeleton.
 */
struct RenderGraphSkeletonDesc final {
    std::size_t graph_record_capacity = MAX_RENDER_GRAPH_SKELETON_RECORDS;
    std::size_t pass_record_capacity = MAX_RENDER_GRAPH_SKELETON_PASSES;
    std::size_t dependency_record_capacity = MAX_RENDER_GRAPH_SKELETON_DEPENDENCIES;
};
}
