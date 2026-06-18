// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonConstants.h

#pragma once

#include <cstddef>

namespace yuengine::rendercore {
/**
 * @comment 最大数量： 保留的 RenderCore render graph skeleton 记录.
 */
constexpr std::size_t MAX_RENDER_GRAPH_SKELETON_RECORDS = 8U;
/**
 * @comment 最大数量： pass declarations 接受 by 一个 RenderCore render graph skeleton 请求.
 */
constexpr std::size_t MAX_RENDER_GRAPH_SKELETON_PASSES = 8U;
/**
 * @comment 最大数量： dependency declarations 接受 by 一个 RenderCore render graph skeleton 请求.
 */
constexpr std::size_t MAX_RENDER_GRAPH_SKELETON_DEPENDENCIES = 16U;
}
