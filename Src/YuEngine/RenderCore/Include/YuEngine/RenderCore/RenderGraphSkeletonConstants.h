// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonConstants.h

#pragma once

#include <cstddef>

namespace yuengine::rendercore {
/**
 * @comment Maximum number of retained RenderCore render graph skeleton records.
 */
constexpr std::size_t MAX_RENDER_GRAPH_SKELETON_RECORDS = 8U;
/**
 * @comment Maximum number of pass declarations accepted by one RenderCore render graph skeleton request.
 */
constexpr std::size_t MAX_RENDER_GRAPH_SKELETON_PASSES = 8U;
/**
 * @comment Maximum number of dependency declarations accepted by one RenderCore render graph skeleton request.
 */
constexpr std::size_t MAX_RENDER_GRAPH_SKELETON_DEPENDENCIES = 16U;
}
