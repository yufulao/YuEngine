// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiHitTestResolver.cpp

#include "YuEngine/UiCore/UiHitTestResolver.h"

#include <array>

#include "YuEngine/UiCore/UiCoreConstants.h"

namespace yuengine::uicore {
namespace {
bool ContainsPoint(const UiRect &rect, const UiVector2 &point) {
    if (point.x < rect.x) {
        return false;
    }

    if (point.y < rect.y) {
        return false;
    }

    if (point.x > (rect.x + rect.width)) {
        return false;
    }

    return point.y <= (rect.y + rect.height);
}

UiHitTestResult Miss() {
    return UiHitTestResult{UiHitTestStatus::Miss, UiNodeId{}, 0, 0U};
}

UiHitTestResult Hit(const UiNodeRecord &record) {
    return UiHitTestResult{
        UiHitTestStatus::Success,
        record.node_id,
        record.layer,
        record.sibling_order};
}

bool IsTopCandidateBetter(const UiNodeRecord &candidate, const UiNodeRecord &current) {
    if (candidate.layer > current.layer) {
        return true;
    }

    if (candidate.layer < current.layer) {
        return false;
    }

    if (candidate.sibling_order > current.sibling_order) {
        return true;
    }

    if (candidate.sibling_order < current.sibling_order) {
        return false;
    }

    return candidate.node_id.value > current.node_id.value;
}

UiHitTestResult ResolveNode(const UiNodeTree &tree, const UiHitTestRequest &request, const UiNodeRecord &record);

UiHitTestResult ResolveChildren(const UiNodeTree &tree, const UiHitTestRequest &request, UiNodeId parent_id) {
    std::array<UiNodeRecord, MAX_UI_NODE_COUNT> children{};
    std::array<bool, MAX_UI_NODE_COUNT> visited{};
    const std::uint32_t child_count = tree.ExportChildren(parent_id, children.data(), MAX_UI_NODE_COUNT);

    while (true) {
        bool found_child = false;
        std::uint32_t best_index = 0U;
        UiNodeRecord best_record;

        for (std::uint32_t index = 0U; index < child_count; ++index) {
            if (visited[index]) {
                continue;
            }

            if (!found_child || IsTopCandidateBetter(children[index], best_record)) {
                found_child = true;
                best_index = index;
                best_record = children[index];
            }
        }

        if (!found_child) {
            break;
        }

        visited[best_index] = true;
        const UiHitTestResult result = ResolveNode(tree, request, best_record);
        if (result.Hit()) {
            return result;
        }
    }

    return Miss();
}

UiHitTestResult ResolveNode(const UiNodeTree &tree, const UiHitTestRequest &request, const UiNodeRecord &record) {
    if (!record.is_visible || !record.is_enabled) {
        return Miss();
    }

    if (request.clip_to_parent_content && record.parent_id.IsValid()) {
        const UiNodeTreeResult parent_result = tree.QueryNode(record.parent_id);
        if (!parent_result.Succeeded()) {
            return UiHitTestResult{UiHitTestStatus::NodeNotFound, UiNodeId{}, 0, 0U};
        }

        if (!ContainsPoint(parent_result.record.content_rect, request.point)) {
            return Miss();
        }
    }

    const UiHitTestResult child_result = ResolveChildren(tree, request, record.node_id);
    if (child_result.Hit()) {
        return child_result;
    }

    if (!record.is_hit_testable) {
        return Miss();
    }

    if (!ContainsPoint(record.world_rect, request.point)) {
        return Miss();
    }

    return Hit(record);
}
}

UiHitTestResult UiHitTestResolver::Resolve(const UiNodeTree &tree, const UiHitTestRequest &request) {
    if (request.root_id.IsValid()) {
        const UiNodeTreeResult root_result = tree.QueryNode(request.root_id);
        if (!root_result.Succeeded()) {
            return UiHitTestResult{UiHitTestStatus::NodeNotFound, UiNodeId{}, 0, 0U};
        }

        return ResolveNode(tree, request, root_result.record);
    }

    return ResolveChildren(tree, request, UiNodeId{});
}
}
