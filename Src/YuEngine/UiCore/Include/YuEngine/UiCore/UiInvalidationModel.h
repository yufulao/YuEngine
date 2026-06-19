// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiInvalidationModel.h

#pragma once

#include <array>
#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiCoreConstants.h"
#include "YuEngine/UiCore/UiInvalidatedNode.h"
#include "YuEngine/UiCore/UiInvalidationRequest.h"
#include "YuEngine/UiCore/UiInvalidationResult.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiNodeTree.h"

namespace yuengine::uicore {
class UiInvalidationModel final {
public:
    /**
     * @comment 计算 UI invalidation 影响范围和 cache rebuild counters。
     * @param tree 输入 UI node tree。
     * @param request 输入 invalidation request。
     * @param out_nodes 调用方持有的 affected node 输出 buffer。
     * @param out_result 输出 invalidation result。
     * @return 显式 invalidation 状态。
     */
    UiInvalidationStatus Invalidate(
        const UiNodeTree &tree,
        const UiInvalidationRequest &request,
        std::span<UiInvalidatedNode> out_nodes,
        UiInvalidationResult *out_result) const;

private:
    std::uint32_t CollectAffectedNodes(
        const UiNodeTree &tree,
        UiNodeId node_id,
        UiInvalidationScope scope,
        std::array<UiNodeId, MAX_UI_NODE_COUNT> &out_node_ids) const;
    std::uint32_t CollectSubtree(
        const UiNodeTree &tree,
        UiNodeId node_id,
        std::array<UiNodeId, MAX_UI_NODE_COUNT> &out_node_ids,
        std::uint32_t write_index) const;
    UiCacheCounters BuildCacheCounters(const UiDirtyState &dirty_state, std::uint32_t affected_node_count) const;
    void WriteResult(
        const UiDirtyState &dirty_state,
        UiInvalidationStatus status,
        std::uint32_t affected_node_count,
        UiInvalidationResult *out_result) const;
};
}
