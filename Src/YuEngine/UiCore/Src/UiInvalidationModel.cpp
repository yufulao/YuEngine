// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiInvalidationModel.cpp

#include "YuEngine/UiCore/UiInvalidationModel.h"

#include <array>

#include "YuEngine/UiCore/UiDirtyTracker.h"
#include "YuEngine/UiCore/UiNodeRecord.h"
#include "YuEngine/UiCore/UiNodeTreeResult.h"

namespace yuengine::uicore {
UiInvalidationStatus UiInvalidationModel::Invalidate(
    const UiNodeTree &tree,
    const UiInvalidationRequest &request,
    std::span<UiInvalidatedNode> out_nodes,
    UiInvalidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiInvalidationStatus::InvalidOutput;
    }

    *out_result = UiInvalidationResult{};
    const UiNodeTreeResult query_result = tree.QueryNode(request.node_id);
    if (!query_result.Succeeded()) {
        out_result->status = UiInvalidationStatus::NodeNotFound;
        return UiInvalidationStatus::NodeNotFound;
    }

    UiDirtyTracker tracker;
    const UiDirtyState dirty_state = tracker.ApplyChange(request.change_type);
    std::array<UiNodeId, MAX_UI_NODE_COUNT> affected_node_ids{};
    const std::uint32_t affected_node_count =
        CollectAffectedNodes(tree, request.node_id, request.scope, affected_node_ids);
    if (out_nodes.size() < static_cast<std::size_t>(affected_node_count)) {
        const std::uint32_t output_node_capacity = static_cast<std::uint32_t>(out_nodes.size());
        WriteResult(dirty_state, UiInvalidationStatus::OutputCapacityExceeded, affected_node_count, out_result);
        out_result->failed_request_node_id = request.node_id;
        out_result->failed_scope = request.scope;
        out_result->failed_change_type = request.change_type;
        out_result->failed_output_node_capacity = output_node_capacity;
        out_result->current_affected_node_count = output_node_capacity;
        out_result->required_affected_node_count = affected_node_count;
        return UiInvalidationStatus::OutputCapacityExceeded;
    }

    if ((affected_node_count > 0U) && (out_nodes.data() == nullptr)) {
        WriteResult(dirty_state, UiInvalidationStatus::InvalidOutput, affected_node_count, out_result);
        return UiInvalidationStatus::InvalidOutput;
    }

    for (std::uint32_t index = 0U; index < affected_node_count; ++index) {
        out_nodes[index].node_id = affected_node_ids[index];
        out_nodes[index].domains = dirty_state.domains;
    }

    WriteResult(dirty_state, UiInvalidationStatus::Success, affected_node_count, out_result);
    return UiInvalidationStatus::Success;
}

std::uint32_t UiInvalidationModel::CollectAffectedNodes(
    const UiNodeTree &tree,
    UiNodeId node_id,
    UiInvalidationScope scope,
    std::array<UiNodeId, MAX_UI_NODE_COUNT> &out_node_ids) const {
    out_node_ids[0U] = node_id;
    if (scope == UiInvalidationScope::Self) {
        return 1U;
    }

    return CollectSubtree(tree, node_id, out_node_ids, 1U);
}

std::uint32_t UiInvalidationModel::CollectSubtree(
    const UiNodeTree &tree,
    UiNodeId node_id,
    std::array<UiNodeId, MAX_UI_NODE_COUNT> &out_node_ids,
    std::uint32_t write_index) const {
    std::array<UiNodeRecord, MAX_UI_NODE_COUNT> children{};
    const std::uint32_t child_count = tree.ExportChildren(node_id, children.data(), MAX_UI_NODE_COUNT);
    std::uint32_t current_index = write_index;
    for (std::uint32_t index = 0U; index < child_count; ++index) {
        if (current_index >= MAX_UI_NODE_COUNT) {
            return current_index;
        }

        out_node_ids[current_index] = children[index].node_id;
        ++current_index;
        current_index = CollectSubtree(tree, children[index].node_id, out_node_ids, current_index);
    }

    return current_index;
}

UiCacheCounters UiInvalidationModel::BuildCacheCounters(
    const UiDirtyState &dirty_state,
    std::uint32_t affected_node_count) const {
    UiCacheCounters counters;
    counters.layout_rebuild_count = dirty_state.layout_rebuild_count * affected_node_count;
    counters.paint_rebuild_count = dirty_state.paint_rebuild_count * affected_node_count;
    return counters;
}

void UiInvalidationModel::WriteResult(
    const UiDirtyState &dirty_state,
    UiInvalidationStatus status,
    std::uint32_t affected_node_count,
    UiInvalidationResult *out_result) const {
    out_result->status = status;
    out_result->dirty_state = dirty_state;
    out_result->cache_counters = BuildCacheCounters(dirty_state, affected_node_count);
    out_result->affected_node_count = affected_node_count;
}
}
