// 模块: Tests UiCore
// 文件: Tests/UiCore/UiCoreTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/UiCore/UiNodeDesc.h"
#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiDirtyDomain.h"
#include "YuEngine/UiCore/UiDirtyState.h"
#include "YuEngine/UiCore/UiDirtyTracker.h"
#include "YuEngine/UiCore/UiDrawElement.h"
#include "YuEngine/UiCore/UiDrawElementDesc.h"
#include "YuEngine/UiCore/UiDrawElementType.h"
#include "YuEngine/UiCore/UiDrawListBuilder.h"
#include "YuEngine/UiCore/UiDrawListResult.h"
#include "YuEngine/UiCore/UiDrawListStatus.h"
#include "YuEngine/UiCore/UiHitTestRequest.h"
#include "YuEngine/UiCore/UiHitTestResolver.h"
#include "YuEngine/UiCore/UiHitTestResult.h"
#include "YuEngine/UiCore/UiHitTestStatus.h"
#include "YuEngine/UiCore/UiInvalidatedNode.h"
#include "YuEngine/UiCore/UiInvalidationModel.h"
#include "YuEngine/UiCore/UiInvalidationRequest.h"
#include "YuEngine/UiCore/UiInvalidationResult.h"
#include "YuEngine/UiCore/UiInvalidationScope.h"
#include "YuEngine/UiCore/UiInvalidationStatus.h"
#include "YuEngine/UiCore/UiLayoutContainerDesc.h"
#include "YuEngine/UiCore/UiLayoutContainerType.h"
#include "YuEngine/UiCore/UiLayoutPass.h"
#include "YuEngine/UiCore/UiLayoutPassResult.h"
#include "YuEngine/UiCore/UiLayoutPassStatus.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiNodeRecord.h"
#include "YuEngine/UiCore/UiNodeTree.h"
#include "YuEngine/UiCore/UiNodeTreeDesc.h"
#include "YuEngine/UiCore/UiNodeTreeResult.h"
#include "YuEngine/UiCore/UiNodeTreeSnapshot.h"
#include "YuEngine/UiCore/UiNodeTreeStatus.h"
#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiRectMath.h"
#include "YuEngine/UiCore/UiRectMathResult.h"
#include "YuEngine/UiCore/UiRectMathStatus.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiCore/UiStackDirection.h"
#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"

using yuengine::uicore::UiDirtyChangeType;
using yuengine::uicore::UiDirtyState;
using yuengine::uicore::UiDirtyTracker;
using yuengine::uicore::UiDrawElement;
using yuengine::uicore::UiDrawElementDesc;
using yuengine::uicore::UiDrawElementType;
using yuengine::uicore::UiDrawListBuilder;
using yuengine::uicore::UiDrawListResult;
using yuengine::uicore::UiDrawListStatus;
using yuengine::uicore::UiHitTestRequest;
using yuengine::uicore::UiHitTestResolver;
using yuengine::uicore::UiHitTestResult;
using yuengine::uicore::UiHitTestStatus;
using yuengine::uicore::UiInvalidatedNode;
using yuengine::uicore::UiInvalidationModel;
using yuengine::uicore::UiInvalidationRequest;
using yuengine::uicore::UiInvalidationResult;
using yuengine::uicore::UiInvalidationScope;
using yuengine::uicore::UiInvalidationStatus;
using yuengine::uicore::UiLayoutContainerDesc;
using yuengine::uicore::UiLayoutContainerType;
using yuengine::uicore::UiLayoutPass;
using yuengine::uicore::UiLayoutPassResult;
using yuengine::uicore::UiLayoutPassStatus;
using yuengine::uicore::UiNodeDesc;
using yuengine::uicore::UiNodeId;
using yuengine::uicore::UiNodeRecord;
using yuengine::uicore::UiNodeTree;
using yuengine::uicore::UiNodeTreeDesc;
using yuengine::uicore::UiNodeTreeResult;
using yuengine::uicore::UiNodeTreeSnapshot;
using yuengine::uicore::UiNodeTreeStatus;
using yuengine::uicore::UiRect;
using yuengine::uicore::UiRectMath;
using yuengine::uicore::UiRectMathResult;
using yuengine::uicore::UiRectMathStatus;
using yuengine::uicore::UiRectTransform;
using yuengine::uicore::UiStackDirection;
using yuengine::uicore::UiStaticAtlasMetadata;
using yuengine::uicore::UiStaticAtlasMetadataDesc;
using yuengine::uicore::UiStaticAtlasPageDesc;
using yuengine::uicore::UiStaticAtlasResolveResult;
using yuengine::uicore::UiStaticAtlasSpriteDesc;
using yuengine::uicore::UiStaticAtlasStatus;
using yuengine::uicore::UI_DIRTY_HIT_TEST;
using yuengine::uicore::UI_DIRTY_LAYOUT;
using yuengine::uicore::UI_DIRTY_PAINT;

namespace {
constexpr std::string_view TEST_NODE_TREE_CREATE_ORDER =
    "UiCore_NodeTree_CreateAttachDetachOrder";
constexpr std::string_view TEST_NODE_TREE_CREATE_CAPACITY_ENTRY =
    "UiCore_NodeTree_CreateCapacityEntryReportsRejectedNode";
constexpr std::string_view TEST_NODE_TREE_DESTROY =
    "UiCore_NodeTree_DestroyRemovesDescendants";
constexpr std::string_view TEST_RECT_MATH =
    "UiCore_RectMath_ParentResizePivotMarginPaddingDpi";
constexpr std::string_view TEST_NO_FORBIDDEN_DEPENDENCY =
    "UiCore_NoLifecycleConfigEditorRenderBackendDependency";
constexpr std::string_view TEST_LAYOUT_CONTAINERS =
    "UiCore_LayoutContainers_ResolveExpectedRects";
constexpr std::string_view TEST_DIRTY_TRACKER =
    "UiCore_DirtyTracker_PaintOnlyDoesNotTriggerLayoutRebuild";
constexpr std::string_view TEST_INVALIDATION_MODEL =
    "UiCore_InvalidationModel_SubtreeRulesExposeCacheCounters";
constexpr std::string_view TEST_INVALIDATION_SMALL_OUTPUT =
    "UiCore_InvalidationModel_RejectsSmallOutputWithoutMutation";
constexpr std::string_view TEST_INVALIDATION_CAPACITY_ENTRY =
    "UiCore_InvalidationModel_OutputCapacityReportsRejectedRequest";
constexpr std::string_view TEST_HIT_TEST =
    "UiCore_HitTest_LayerClipDisabled";
constexpr std::string_view TEST_DRAW_LIST =
    "UiCore_DrawList_DeterministicElements";
constexpr std::string_view TEST_DRAW_LIST_SMALL_OUTPUT =
    "UiCore_DrawList_OutputCapacityReportsVisibleCount";
constexpr std::string_view TEST_STATIC_ATLAS_RESOLVE =
    "UiCore_StaticAtlasMetadata_ResolvesSpritePageUvNineSlice";
constexpr std::string_view TEST_STATIC_ATLAS_MISSING =
    "UiCore_StaticAtlasMetadata_MissingSpriteReportsStatus";
constexpr std::string_view TEST_STATIC_ATLAS_INVALID =
    "UiCore_StaticAtlasMetadata_RejectsInvalidSpriteMetadata";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected exactly one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
using TestFunction = int (*)();

int Fail(const std::string &message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

int RequireInvalidationCapacityEntry(const UiInvalidationResult &result,
    UiNodeId node_id,
    UiInvalidationScope scope,
    UiDirtyChangeType change_type,
    std::uint32_t output_node_capacity,
    std::uint32_t current_affected_node_count,
    std::uint32_t required_affected_node_count,
    const char *message) {
    if (result.failed_request_node_id.value != node_id.value) {
        return Fail(message);
    }

    if (result.failed_scope != scope) {
        return Fail(message);
    }

    if (result.failed_change_type != change_type) {
        return Fail(message);
    }

    if (result.failed_output_node_capacity != output_node_capacity) {
        return Fail(message);
    }

    if (result.current_affected_node_count != current_affected_node_count) {
        return Fail(message);
    }

    if (result.required_affected_node_count != required_affected_node_count) {
        return Fail(message);
    }

    return 0;
}

int RequireInvalidationCapacityEntryCleared(const UiInvalidationResult &result, const char *message) {
    if (result.failed_request_node_id.IsValid()) {
        return Fail(message);
    }

    if (result.failed_scope != UiInvalidationScope::Self) {
        return Fail(message);
    }

    if (result.failed_change_type != UiDirtyChangeType::PaintOnly) {
        return Fail(message);
    }

    if (result.failed_output_node_capacity != 0U) {
        return Fail(message);
    }

    if (result.current_affected_node_count != 0U) {
        return Fail(message);
    }

    if (result.required_affected_node_count != 0U) {
        return Fail(message);
    }

    return 0;
}

UiNodeTreeDesc MakeTreeDesc() {
    UiNodeTreeDesc desc;
    desc.node_capacity = 8U;
    desc.viewport_rect = UiRect{0.0F, 0.0F, 800.0F, 600.0F};
    return desc;
}

UiStaticAtlasPageDesc MakeAtlasPage(
    std::uint32_t page_key,
    std::uint32_t texture_key,
    std::uint32_t width,
    std::uint32_t height) {
    UiStaticAtlasPageDesc desc;
    desc.page_key = page_key;
    desc.texture_key = texture_key;
    desc.width = width;
    desc.height = height;
    return desc;
}

UiStaticAtlasSpriteDesc MakeAtlasSprite(
    std::uint32_t sprite_key,
    std::uint32_t page_key,
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t width,
    std::uint32_t height) {
    UiStaticAtlasSpriteDesc desc;
    desc.sprite_key = sprite_key;
    desc.page_key = page_key;
    desc.x = x;
    desc.y = y;
    desc.width = width;
    desc.height = height;
    return desc;
}

bool FloatClose(float left, float right) {
    float diff = left - right;
    if (diff < 0.0F) {
        diff = -diff;
    }

    return diff < 0.0001F;
}

UiRectTransform FullStretchTransform() {
    UiRectTransform transform;
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.pivot = {0.5F, 0.5F};
    return transform;
}

UiRectTransform ChildTransform(float left, float bottom, float right, float top) {
    UiRectTransform transform;
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.pivot = {0.5F, 0.5F};
    transform.offset_min = {left, bottom};
    transform.offset_max = {-right, -top};
    return transform;
}

UiRectTransform FixedChildTransform(float x, float y, float width, float height) {
    UiRectTransform transform;
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.offset_min = {x, y};
    transform.offset_max = {x + width - 800.0F, y + height - 600.0F};
    return transform;
}

UiNodeDesc MakeNodeDesc(UiNodeId node_id, UiNodeId parent_id, std::uint32_t order) {
    UiNodeDesc desc;
    desc.node_id = node_id;
    desc.parent_id = parent_id;
    desc.rect_transform = FullStretchTransform();
    desc.sibling_order = order;
    desc.layer = 3;
    desc.is_visible = true;
    desc.is_enabled = true;
    desc.is_hit_testable = true;
    return desc;
}

int ExpectSuccess(const UiNodeTreeResult &result, std::string_view message) {
    if (result.Succeeded()) {
        return 0;
    }

    return Fail(std::string(message));
}

bool FloatEquals(float left, float right) {
    const float delta = left - right;
    if (delta < 0.0F) {
        return (-delta) < 0.001F;
    }

    return delta < 0.001F;
}

bool RectEquals(const UiRect &rect, float x, float y, float width, float height) {
    if (!FloatEquals(rect.x, x)) {
        return false;
    }

    if (!FloatEquals(rect.y, y)) {
        return false;
    }

    if (!FloatEquals(rect.width, width)) {
        return false;
    }

    return FloatEquals(rect.height, height);
}

int ExpectSnapshotNodeCapacityEntryCleared(const UiNodeTreeSnapshot &snapshot) {
    if (snapshot.last_required_node_count != 0U) {
        return Fail("snapshot kept required node count");
    }

    if (snapshot.last_node_capacity_entry_node_id.IsValid()) {
        return Fail("snapshot kept node capacity entry id");
    }

    if (snapshot.last_node_capacity_entry_parent_id.IsValid()) {
        return Fail("snapshot kept node capacity entry parent");
    }

    if (snapshot.last_node_capacity_entry_sibling_order != 0U) {
        return Fail("snapshot kept node capacity entry order");
    }

    if (snapshot.last_node_capacity_entry_capacity != 0U) {
        return Fail("snapshot kept node capacity entry capacity");
    }

    if (snapshot.last_node_capacity_entry_active_count != 0U) {
        return Fail("snapshot kept node capacity entry active count");
    }

    return 0;
}

int ExpectResultNodeCapacityEntryCleared(const UiNodeTreeResult &result) {
    if (result.required_node_count != 0U) {
        return Fail("result kept required node count");
    }

    if (result.capacity_entry_node_id.IsValid()) {
        return Fail("result kept node capacity entry id");
    }

    if (result.capacity_entry_parent_id.IsValid()) {
        return Fail("result kept node capacity entry parent");
    }

    if (result.capacity_entry_sibling_order != 0U) {
        return Fail("result kept node capacity entry order");
    }

    if (result.capacity_entry_node_capacity != 0U) {
        return Fail("result kept node capacity entry capacity");
    }

    if (result.capacity_entry_active_node_count != 0U) {
        return Fail("result kept node capacity entry active count");
    }

    return 0;
}

int CreateNode(UiNodeTree &tree, const UiNodeDesc &desc, std::string_view message) {
    const UiNodeTreeResult result = tree.CreateNode(desc);
    return ExpectSuccess(result, message);
}

int QueryRectEquals(UiNodeTree &tree, UiNodeId node_id, const UiRect &expected_rect) {
    const UiNodeTreeResult result = tree.QueryNode(node_id);
    int ret_code = ExpectSuccess(result, "query rect failed");
    if (ret_code != 0) {
        return ret_code;
    }

    if (!RectEquals(result.record.world_rect, expected_rect.x, expected_rect.y, expected_rect.width, expected_rect.height)) {
        return Fail("rect did not match expected value");
    }

    return 0;
}

int CreateRootAndChildren(UiNodeTree &tree, std::uint32_t child_count) {
    const UiNodeDesc root_desc = MakeNodeDesc(NodeId(1U), UiNodeId{}, 0U);
    int ret_code = CreateNode(tree, root_desc, "root create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    for (std::uint32_t index = 0U; index < child_count; ++index) {
        const UiNodeDesc child_desc = MakeNodeDesc(NodeId(2U + index), NodeId(1U), index);
        ret_code = CreateNode(tree, child_desc, "child create failed");
        if (ret_code != 0) {
            return ret_code;
        }
    }

    return 0;
}

int UiCoreNodeTreeCreateAttachDetachOrder() {
    UiNodeTree tree(MakeTreeDesc());
    UiNodeDesc root_desc = MakeNodeDesc(NodeId(1U), UiNodeId{}, 0U);
    UiNodeDesc first_child_desc = MakeNodeDesc(NodeId(2U), NodeId(1U), 20U);
    UiNodeDesc second_child_desc = MakeNodeDesc(NodeId(3U), NodeId(1U), 10U);

    const UiNodeTreeResult root_result = tree.CreateNode(root_desc);
    int ret_code = ExpectSuccess(root_result, "root create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiNodeTreeResult first_child_result = tree.CreateNode(first_child_desc);
    ret_code = ExpectSuccess(first_child_result, "first child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiNodeTreeResult second_child_result = tree.CreateNode(second_child_desc);
    ret_code = ExpectSuccess(second_child_result, "second child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    std::array<UiNodeRecord, 2U> children{};
    const std::uint32_t child_count = tree.ExportChildren(NodeId(1U), children.data(), 2U);
    if (child_count != 2U) {
        return Fail("child count mismatch");
    }

    if (children[0U].node_id.value != 3U || children[1U].node_id.value != 2U) {
        return Fail("child order was not deterministic");
    }

    ret_code = ExpectSuccess(tree.DetachNode(NodeId(2U), 5U), "detach child failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const std::uint32_t remaining_child_count = tree.ExportChildren(NodeId(1U), children.data(), 2U);
    if (remaining_child_count != 1U || children[0U].node_id.value != 3U) {
        return Fail("detach did not remove child from parent");
    }

    std::array<UiNodeRecord, 3U> roots{};
    const std::uint32_t root_count = tree.ExportChildren(UiNodeId{}, roots.data(), 3U);
    if (root_count != 2U) {
        return Fail("detached node did not become a root");
    }

    if (roots[0U].node_id.value != 1U || roots[1U].node_id.value != 2U) {
        return Fail("root order was not deterministic after detach");
    }

    ret_code = ExpectSuccess(tree.AttachNode(NodeId(2U), NodeId(3U), 1U), "attach child failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiNodeTreeResult attached_query = tree.QueryNode(NodeId(2U));
    ret_code = ExpectSuccess(attached_query, "attached query failed");
    if (ret_code != 0) {
        return ret_code;
    }

    if (attached_query.record.parent_id.value != 3U) {
        return Fail("attach did not update parent id");
    }

    const UiNodeTreeSnapshot snapshot = tree.Snapshot();
    if (snapshot.active_node_count != 3U || snapshot.created_node_count != 3U) {
        return Fail("snapshot node counts mismatch");
    }

    return 0;
}

int UiCoreNodeTreeCreateCapacityEntryReportsRejectedNode() {
    UiNodeTreeDesc invalid_desc = MakeTreeDesc();
    invalid_desc.node_capacity = 0U;
    UiNodeTree invalid_tree(invalid_desc);
    const UiNodeTreeSnapshot invalid_snapshot = invalid_tree.Snapshot();
    if (invalid_snapshot.last_status != UiNodeTreeStatus::InvalidCapacity) {
        return Fail("invalid capacity construction did not report status");
    }

    int ret_code = ExpectSnapshotNodeCapacityEntryCleared(invalid_snapshot);
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeTreeDesc desc = MakeTreeDesc();
    desc.node_capacity = 2U;
    UiNodeTree tree(desc);

    const UiNodeDesc root_desc = MakeNodeDesc(NodeId(1U), UiNodeId{}, 0U);
    ret_code = CreateNode(tree, root_desc, "root create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiNodeDesc child_desc = MakeNodeDesc(NodeId(2U), NodeId(1U), 5U);
    ret_code = CreateNode(tree, child_desc, "child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiNodeTreeSnapshot before_snapshot = tree.Snapshot();
    UiNodeDesc rejected_desc = MakeNodeDesc(NodeId(3U), NodeId(1U), 77U);
    const UiNodeTreeResult rejected_result = tree.CreateNode(rejected_desc);
    if (rejected_result.status != UiNodeTreeStatus::CapacityExceeded || rejected_result.Succeeded()) {
        return Fail("create capacity overflow did not fail with capacity status");
    }

    if (rejected_result.capacity_entry_node_id.value != 3U ||
        rejected_result.capacity_entry_parent_id.value != 1U ||
        rejected_result.capacity_entry_sibling_order != 77U) {
        return Fail("create capacity result did not record rejected identity");
    }

    if (rejected_result.capacity_entry_node_capacity != 2U ||
        rejected_result.capacity_entry_active_node_count != 2U ||
        rejected_result.required_node_count != 3U) {
        return Fail("create capacity result did not record resize counts");
    }

    const UiNodeTreeSnapshot after_snapshot = tree.Snapshot();
    if (after_snapshot.active_node_count != before_snapshot.active_node_count ||
        after_snapshot.created_node_count != before_snapshot.created_node_count) {
        return Fail("create capacity overflow mutated node counts");
    }

    if (tree.QueryNode(NodeId(3U)).status != UiNodeTreeStatus::NodeNotFound) {
        return Fail("rejected node became queryable");
    }

    if (after_snapshot.last_node_capacity_entry_node_id.value != 3U ||
        after_snapshot.last_node_capacity_entry_parent_id.value != 1U ||
        after_snapshot.last_node_capacity_entry_sibling_order != 77U) {
        return Fail("snapshot did not record rejected node identity");
    }

    if (after_snapshot.last_node_capacity_entry_capacity != 2U ||
        after_snapshot.last_node_capacity_entry_active_count != 2U ||
        after_snapshot.last_required_node_count != 3U) {
        return Fail("snapshot did not record node capacity counts");
    }

    const UiNodeTreeResult missing_destroy_result = tree.DestroyNode(NodeId(99U));
    if (missing_destroy_result.status != UiNodeTreeStatus::NodeNotFound) {
        return Fail("missing destroy did not fail with node not found");
    }

    ret_code = ExpectSnapshotNodeCapacityEntryCleared(tree.Snapshot());
    if (ret_code != 0) {
        return ret_code;
    }

    rejected_desc = MakeNodeDesc(NodeId(4U), NodeId(1U), 88U);
    if (tree.CreateNode(rejected_desc).status != UiNodeTreeStatus::CapacityExceeded) {
        return Fail("second capacity setup did not fail");
    }

    ret_code = ExpectSuccess(tree.DestroyNode(NodeId(2U)), "destroy child after capacity failed");
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = ExpectSnapshotNodeCapacityEntryCleared(tree.Snapshot());
    if (ret_code != 0) {
        return ret_code;
    }

    const UiNodeDesc invalid_parent_desc = MakeNodeDesc(NodeId(6U), NodeId(99U), 1U);
    const UiNodeTreeResult invalid_parent_result = tree.CreateNode(invalid_parent_desc);
    if (invalid_parent_result.status != UiNodeTreeStatus::ParentNotFound) {
        return Fail("invalid parent create did not fail with parent not found");
    }

    ret_code = ExpectResultNodeCapacityEntryCleared(invalid_parent_result);
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = ExpectSnapshotNodeCapacityEntryCleared(tree.Snapshot());
    if (ret_code != 0) {
        return ret_code;
    }

    const UiNodeDesc replacement_desc = MakeNodeDesc(NodeId(5U), NodeId(1U), 90U);
    const UiNodeTreeResult replacement_result = tree.CreateNode(replacement_desc);
    ret_code = ExpectSuccess(replacement_result, "replacement create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = ExpectResultNodeCapacityEntryCleared(replacement_result);
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = ExpectSnapshotNodeCapacityEntryCleared(tree.Snapshot());
    if (ret_code != 0) {
        return ret_code;
    }

    return 0;
}

int UiCoreNodeTreeDestroyRemovesDescendants() {
    UiNodeTree tree(MakeTreeDesc());
    const UiNodeDesc root_desc = MakeNodeDesc(NodeId(1U), UiNodeId{}, 0U);
    const UiNodeTreeResult root_result = tree.CreateNode(root_desc);
    int ret_code = ExpectSuccess(root_result, "root create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiNodeDesc child_desc = MakeNodeDesc(NodeId(2U), NodeId(1U), 0U);
    const UiNodeTreeResult child_result = tree.CreateNode(child_desc);
    ret_code = ExpectSuccess(child_result, "child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiNodeDesc grandchild_desc = MakeNodeDesc(NodeId(3U), NodeId(2U), 0U);
    const UiNodeTreeResult grandchild_result = tree.CreateNode(grandchild_desc);
    ret_code = ExpectSuccess(grandchild_result, "grandchild create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = ExpectSuccess(tree.DestroyNode(NodeId(2U)), "destroy child failed");
    if (ret_code != 0) {
        return ret_code;
    }

    if (tree.QueryNode(NodeId(2U)).status != UiNodeTreeStatus::NodeNotFound) {
        return Fail("destroyed child remained queryable");
    }

    if (tree.QueryNode(NodeId(3U)).status != UiNodeTreeStatus::NodeNotFound) {
        return Fail("destroyed grandchild remained queryable");
    }

    const UiNodeTreeSnapshot snapshot = tree.Snapshot();
    if (snapshot.active_node_count != 1U || snapshot.destroyed_node_count != 2U) {
        return Fail("destroy did not update snapshot counts");
    }

    return 0;
}

int UiCoreRectMathParentResizePivotMarginPaddingDpi() {
    UiRect parent_rect{10.0F, 20.0F, 400.0F, 200.0F};
    UiRectTransform transform;
    transform.anchor_min = {0.25F, 0.25F};
    transform.anchor_max = {0.75F, 0.75F};
    transform.pivot = {0.25F, 0.75F};
    transform.offset_min = {5.0F, 7.0F};
    transform.offset_max = {-11.0F, -13.0F};
    transform.margin = {2.0F, 3.0F, 4.0F, 5.0F};
    transform.padding = {1.0F, 2.0F, 3.0F, 4.0F};
    transform.dpi_scale = 2.0F;

    const UiRectMathResult result = UiRectMath::Resolve(parent_rect, transform);
    if (!result.Succeeded()) {
        return Fail("rect math failed");
    }

    if (!RectEquals(result.rect, 124.0F, 94.0F, 156.0F, 44.0F)) {
        return Fail("resolved rect mismatch");
    }

    if (!RectEquals(result.content_rect, 126.0F, 102.0F, 148.0F, 32.0F)) {
        return Fail("content rect mismatch");
    }

    if (!FloatEquals(result.pivot_point.x, 163.0F) || !FloatEquals(result.pivot_point.y, 127.0F)) {
        return Fail("pivot point mismatch");
    }

    UiNodeTree tree(MakeTreeDesc());
    UiNodeDesc root_desc = MakeNodeDesc(NodeId(1U), UiNodeId{}, 0U);
    root_desc.rect_transform = FullStretchTransform();
    const UiNodeTreeResult root_result = tree.CreateNode(root_desc);
    int ret_code = ExpectSuccess(root_result, "root create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc child_desc = MakeNodeDesc(NodeId(2U), NodeId(1U), 0U);
    child_desc.rect_transform = ChildTransform(10.0F, 20.0F, 30.0F, 40.0F);
    const UiNodeTreeResult child_result = tree.CreateNode(child_desc);
    ret_code = ExpectSuccess(child_result, "child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeTreeResult child_query = tree.QueryNode(NodeId(2U));
    ret_code = ExpectSuccess(child_query, "child query failed");
    if (ret_code != 0) {
        return ret_code;
    }

    if (!RectEquals(child_query.record.world_rect, 10.0F, 20.0F, 760.0F, 540.0F)) {
        return Fail("child rect before resize mismatch");
    }

    root_desc.rect_transform.offset_max = {-400.0F, -300.0F};
    ret_code = ExpectSuccess(tree.SetNodeRect(NodeId(1U), root_desc.rect_transform), "root resize failed");
    if (ret_code != 0) {
        return ret_code;
    }

    child_query = tree.QueryNode(NodeId(2U));
    ret_code = ExpectSuccess(child_query, "child query after resize failed");
    if (ret_code != 0) {
        return ret_code;
    }

    if (!RectEquals(child_query.record.world_rect, 10.0F, 20.0F, 360.0F, 240.0F)) {
        return Fail("child rect after parent resize mismatch");
    }

    UiRectTransform invalid_transform = transform;
    invalid_transform.dpi_scale = 0.0F;
    const UiRectMathResult invalid_result = UiRectMath::Resolve(parent_rect, invalid_transform);
    if (invalid_result.status != UiRectMathStatus::InvalidDpiScale) {
        return Fail("invalid dpi scale did not return explicit status");
    }

    return 0;
}

int UiCoreLayoutContainersResolveExpectedRects() {
    UiLayoutPass pass;
    {
        UiNodeTree tree(MakeTreeDesc());
        int ret_code = CreateRootAndChildren(tree, 2U);
        if (ret_code != 0) {
            return ret_code;
        }

        std::array<UiLayoutContainerDesc, 1U> containers{};
        containers[0].container_id = NodeId(1U);
        containers[0].type = UiLayoutContainerType::Stack;
        containers[0].stack_direction = UiStackDirection::Vertical;
        containers[0].spacing_y = 10.0F;
        const UiLayoutPassResult result = pass.Apply(&tree, containers);
        if (!result.Succeeded()) {
            return Fail("stack layout failed");
        }

        ret_code = QueryRectEquals(tree, NodeId(2U), UiRect{0.0F, 305.0F, 800.0F, 295.0F});
        if (ret_code != 0) {
            return ret_code;
        }

        ret_code = QueryRectEquals(tree, NodeId(3U), UiRect{0.0F, 0.0F, 800.0F, 295.0F});
        if (ret_code != 0) {
            return ret_code;
        }
    }

    {
        UiNodeTree tree(MakeTreeDesc());
        int ret_code = CreateRootAndChildren(tree, 4U);
        if (ret_code != 0) {
            return ret_code;
        }

        std::array<UiLayoutContainerDesc, 1U> containers{};
        containers[0].container_id = NodeId(1U);
        containers[0].type = UiLayoutContainerType::Grid;
        containers[0].grid_column_count = 2U;
        containers[0].spacing_x = 10.0F;
        containers[0].spacing_y = 20.0F;
        const UiLayoutPassResult result = pass.Apply(&tree, containers);
        if (result.status != UiLayoutPassStatus::Success || result.arranged_node_count != 4U) {
            return Fail("grid layout result mismatch");
        }

        ret_code = QueryRectEquals(tree, NodeId(2U), UiRect{0.0F, 310.0F, 395.0F, 290.0F});
        if (ret_code != 0) {
            return ret_code;
        }

        ret_code = QueryRectEquals(tree, NodeId(5U), UiRect{405.0F, 0.0F, 395.0F, 290.0F});
        if (ret_code != 0) {
            return ret_code;
        }
    }

    {
        UiNodeTree tree(MakeTreeDesc());
        int ret_code = CreateRootAndChildren(tree, 2U);
        if (ret_code != 0) {
            return ret_code;
        }

        std::array<UiLayoutContainerDesc, 1U> containers{};
        containers[0].container_id = NodeId(1U);
        containers[0].type = UiLayoutContainerType::Overlay;
        const UiLayoutPassResult result = pass.Apply(&tree, containers);
        if (!result.Succeeded()) {
            return Fail("overlay layout failed");
        }

        ret_code = QueryRectEquals(tree, NodeId(2U), UiRect{0.0F, 0.0F, 800.0F, 600.0F});
        if (ret_code != 0) {
            return ret_code;
        }
    }

    {
        UiNodeTree tree(MakeTreeDesc());
        int ret_code = CreateRootAndChildren(tree, 1U);
        if (ret_code != 0) {
            return ret_code;
        }

        std::array<UiLayoutContainerDesc, 1U> containers{};
        containers[0].container_id = NodeId(1U);
        containers[0].type = UiLayoutContainerType::ScrollViewport;
        containers[0].item_width = 1000.0F;
        containers[0].item_height = 900.0F;
        containers[0].scroll_offset = {15.0F, 25.0F};
        const UiLayoutPassResult result = pass.Apply(&tree, containers);
        if (!result.Succeeded()) {
            return Fail("scroll viewport layout failed");
        }

        ret_code = QueryRectEquals(tree, NodeId(2U), UiRect{-15.0F, -25.0F, 1000.0F, 900.0F});
        if (ret_code != 0) {
            return ret_code;
        }
    }

    {
        UiNodeTree tree(MakeTreeDesc());
        UiNodeDesc root_desc = MakeNodeDesc(NodeId(1U), UiNodeId{}, 0U);
        int ret_code = CreateNode(tree, root_desc, "root create failed");
        if (ret_code != 0) {
            return ret_code;
        }

        UiNodeDesc child_desc = MakeNodeDesc(NodeId(2U), NodeId(1U), 0U);
        child_desc.rect_transform = ChildTransform(10.0F, 20.0F, 30.0F, 40.0F);
        ret_code = CreateNode(tree, child_desc, "absolute child create failed");
        if (ret_code != 0) {
            return ret_code;
        }

        std::array<UiLayoutContainerDesc, 1U> containers{};
        containers[0].container_id = NodeId(1U);
        containers[0].type = UiLayoutContainerType::Absolute;
        const UiLayoutPassResult result = pass.Apply(&tree, containers);
        if (!result.Succeeded()) {
            return Fail("absolute layout failed");
        }

        ret_code = QueryRectEquals(tree, NodeId(2U), UiRect{10.0F, 20.0F, 760.0F, 540.0F});
        if (ret_code != 0) {
            return ret_code;
        }
    }

    return 0;
}

int UiCoreDirtyTrackerPaintOnlyDoesNotTriggerLayoutRebuild() {
    UiDirtyTracker tracker;
    UiDirtyState state = tracker.ApplyChange(UiDirtyChangeType::PaintOnly);
    if (state.HasDomain(UI_DIRTY_LAYOUT)) {
        return Fail("paint-only dirty triggered layout domain");
    }

    if (!state.HasDomain(UI_DIRTY_PAINT) || state.layout_rebuild_count != 0U) {
        return Fail("paint-only dirty state mismatch");
    }

    if (state.paint_rebuild_count != 1U) {
        return Fail("paint-only dirty did not expose paint rebuild count");
    }

    state = tracker.ApplyChange(UiDirtyChangeType::AtlasPage);
    if (state.HasDomain(UI_DIRTY_LAYOUT) || state.layout_rebuild_count != 0U) {
        return Fail("atlas page dirty triggered layout rebuild");
    }

    if (state.paint_rebuild_count != 2U) {
        return Fail("atlas page dirty did not increment paint rebuild count");
    }

    tracker.Clear();
    state = tracker.ApplyChange(UiDirtyChangeType::ScrollOffset);
    if (state.HasDomain(UI_DIRTY_LAYOUT)) {
        return Fail("scroll dirty triggered layout domain");
    }

    if (!state.HasDomain(UI_DIRTY_HIT_TEST) || state.layout_rebuild_count != 0U) {
        return Fail("scroll dirty did not classify hit-test domain");
    }

    if (state.paint_rebuild_count != 1U) {
        return Fail("scroll dirty did not expose paint rebuild count");
    }

    tracker.Clear();
    state = tracker.ApplyChange(UiDirtyChangeType::Layout);
    if (!state.HasDomain(UI_DIRTY_LAYOUT) || state.layout_rebuild_count != 1U) {
        return Fail("layout dirty did not trigger one rebuild");
    }

    if (state.paint_rebuild_count != 1U) {
        return Fail("layout dirty did not expose paint rebuild count");
    }

    return 0;
}

int UiCoreInvalidationModelSubtreeRulesExposeCacheCounters() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateRootAndChildren(tree, 3U);
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc grandchild_desc = MakeNodeDesc(NodeId(5U), NodeId(2U), 0U);
    ret_code = CreateNode(tree, grandchild_desc, "grandchild create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiInvalidationModel model;
    UiInvalidationRequest request;
    request.node_id = NodeId(1U);
    request.change_type = UiDirtyChangeType::Layout;
    request.scope = UiInvalidationScope::Subtree;

    std::array<UiInvalidatedNode, 5U> affected_nodes{};
    UiInvalidationResult result{};
    UiInvalidationStatus status = model.Invalidate(tree, request, affected_nodes, &result);
    if (status != UiInvalidationStatus::Success || !result.Succeeded()) {
        return Fail("layout subtree invalidation failed");
    }

    if (result.affected_node_count != 5U) {
        return Fail("layout subtree affected node count mismatch");
    }

    if (result.cache_counters.layout_rebuild_count != 5U || result.cache_counters.paint_rebuild_count != 5U) {
        return Fail("layout subtree cache counters mismatch");
    }

    if (affected_nodes[0U].node_id.value != 1U || affected_nodes[1U].node_id.value != 2U) {
        return Fail("layout subtree affected order mismatch");
    }

    if (affected_nodes[2U].node_id.value != 5U || affected_nodes[3U].node_id.value != 3U) {
        return Fail("layout subtree affected order mismatch");
    }

    if (affected_nodes[4U].node_id.value != 4U) {
        return Fail("layout subtree affected order mismatch");
    }

    if ((affected_nodes[0U].domains & UI_DIRTY_LAYOUT) == 0U) {
        return Fail("layout subtree did not mark layout domain");
    }

    request.node_id = NodeId(2U);
    request.change_type = UiDirtyChangeType::PaintOnly;
    request.scope = UiInvalidationScope::Self;
    affected_nodes = {};
    result = UiInvalidationResult{};
    status = model.Invalidate(tree, request, affected_nodes, &result);
    if (status != UiInvalidationStatus::Success || result.affected_node_count != 1U) {
        return Fail("paint self invalidation failed");
    }

    if (result.cache_counters.layout_rebuild_count != 0U || result.cache_counters.paint_rebuild_count != 1U) {
        return Fail("paint self cache counters mismatch");
    }

    if ((affected_nodes[0U].domains & UI_DIRTY_PAINT) == 0U) {
        return Fail("paint self did not mark paint domain");
    }

    return 0;
}

int UiCoreInvalidationModelRejectsSmallOutputWithoutMutation() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateRootAndChildren(tree, 2U);
    if (ret_code != 0) {
        return ret_code;
    }

    UiInvalidationModel model;
    UiInvalidationRequest request;
    request.node_id = NodeId(1U);
    request.change_type = UiDirtyChangeType::Layout;
    request.scope = UiInvalidationScope::Subtree;

    std::array<UiInvalidatedNode, 1U> affected_nodes{};
    affected_nodes[0U].node_id = NodeId(99U);
    affected_nodes[0U].domains = 777U;
    UiInvalidationResult result{};
    const UiInvalidationStatus status = model.Invalidate(tree, request, affected_nodes, &result);
    if (status != UiInvalidationStatus::OutputCapacityExceeded) {
        return Fail("small invalidation output was not rejected");
    }

    if (result.affected_node_count != 3U) {
        return Fail("small invalidation output did not report required count");
    }

    if (result.cache_counters.layout_rebuild_count != 3U || result.cache_counters.paint_rebuild_count != 3U) {
        return Fail("small invalidation output did not expose cache counters");
    }

    ret_code = RequireInvalidationCapacityEntry(
        result,
        NodeId(1U),
        UiInvalidationScope::Subtree,
        UiDirtyChangeType::Layout,
        1U,
        1U,
        3U,
        "small invalidation output capacity entry wrong");
    if (ret_code != 0) {
        return ret_code;
    }

    if (affected_nodes[0U].node_id.value != 99U || affected_nodes[0U].domains != 777U) {
        return Fail("small invalidation output mutated caller storage");
    }

    return 0;
}

int UiCoreInvalidationModelOutputCapacityReportsRejectedRequest() {
    UiNodeTree tree(MakeTreeDesc());
    int ret_code = CreateRootAndChildren(tree, 2U);
    if (ret_code != 0) {
        return ret_code;
    }

    UiInvalidationModel model;
    UiInvalidationRequest request;
    request.node_id = NodeId(1U);
    request.change_type = UiDirtyChangeType::Transform;
    request.scope = UiInvalidationScope::Subtree;

    std::array<UiInvalidatedNode, 1U> affected_nodes{};
    affected_nodes[0U].node_id = NodeId(77U);
    affected_nodes[0U].domains = 88U;
    UiInvalidationResult result{};
    UiInvalidationStatus status = model.Invalidate(tree, request, affected_nodes, &result);
    if (status != UiInvalidationStatus::OutputCapacityExceeded) {
        return Fail("invalidation capacity entry status wrong");
    }

    ret_code = RequireInvalidationCapacityEntry(
        result,
        NodeId(1U),
        UiInvalidationScope::Subtree,
        UiDirtyChangeType::Transform,
        1U,
        1U,
        3U,
        "invalidation capacity entry wrong");
    if (ret_code != 0) {
        return ret_code;
    }

    if (affected_nodes[0U].node_id.value != 77U || affected_nodes[0U].domains != 88U) {
        return Fail("invalidation capacity entry mutated output");
    }

    request.node_id = NodeId(99U);
    status = model.Invalidate(tree, request, affected_nodes, &result);
    if (status != UiInvalidationStatus::NodeNotFound) {
        return Fail("invalidation capacity entry missing-node status wrong");
    }

    ret_code = RequireInvalidationCapacityEntryCleared(
        result,
        "invalidation capacity entry missing-node did not clear");
    if (ret_code != 0) {
        return ret_code;
    }

    request.node_id = NodeId(1U);
    request.scope = UiInvalidationScope::Self;
    UiInvalidatedNode *null_nodes = nullptr;
    std::span<UiInvalidatedNode> invalid_output(null_nodes, 1U);
    status = model.Invalidate(tree, request, invalid_output, &result);
    if (status != UiInvalidationStatus::InvalidOutput) {
        return Fail("invalidation capacity entry invalid-output status wrong");
    }

    ret_code = RequireInvalidationCapacityEntryCleared(
        result,
        "invalidation capacity entry invalid-output did not clear");
    if (ret_code != 0) {
        return ret_code;
    }

    request.scope = UiInvalidationScope::Subtree;
    status = model.Invalidate(tree, request, affected_nodes, &result);
    if (status != UiInvalidationStatus::OutputCapacityExceeded) {
        return Fail("invalidation capacity entry restale status wrong");
    }

    std::array<UiInvalidatedNode, 3U> full_output{};
    request.scope = UiInvalidationScope::Subtree;
    status = model.Invalidate(tree, request, full_output, &result);
    if (status != UiInvalidationStatus::Success) {
        return Fail("invalidation capacity entry success status wrong");
    }

    return RequireInvalidationCapacityEntryCleared(
        result,
        "invalidation capacity entry success did not clear");
}

int UiCoreHitTestLayerClipDisabled() {
    UiNodeTree tree(MakeTreeDesc());
    UiNodeDesc root_desc = MakeNodeDesc(NodeId(1U), UiNodeId{}, 0U);
    int ret_code = CreateNode(tree, root_desc, "root create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc low_desc = MakeNodeDesc(NodeId(2U), NodeId(1U), 0U);
    low_desc.layer = 1;
    ret_code = CreateNode(tree, low_desc, "low child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc disabled_desc = MakeNodeDesc(NodeId(3U), NodeId(1U), 1U);
    disabled_desc.layer = 5;
    disabled_desc.is_enabled = false;
    ret_code = CreateNode(tree, disabled_desc, "disabled child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc clipped_desc = MakeNodeDesc(NodeId(4U), NodeId(1U), 2U);
    clipped_desc.layer = 6;
    clipped_desc.rect_transform = FixedChildTransform(820.0F, 10.0F, 100.0F, 100.0F);
    ret_code = CreateNode(tree, clipped_desc, "clipped child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiHitTestRequest request;
    request.point = {10.0F, 10.0F};
    const UiHitTestResult hit_result = UiHitTestResolver::Resolve(tree, request);
    if (hit_result.status != UiHitTestStatus::Success || hit_result.node_id.value != 2U) {
        return Fail("hit-test did not skip disabled top child");
    }

    request.point = {850.0F, 20.0F};
    const UiHitTestResult clipped_result = UiHitTestResolver::Resolve(tree, request);
    if (clipped_result.status != UiHitTestStatus::Miss) {
        return Fail("hit-test did not respect parent clipping");
    }

    return 0;
}

int UiCoreDrawListDeterministicElements() {
    UiNodeTree tree(MakeTreeDesc());
    UiNodeDesc root_desc = MakeNodeDesc(NodeId(1U), UiNodeId{}, 0U);
    int ret_code = CreateNode(tree, root_desc, "root create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc high_desc = MakeNodeDesc(NodeId(2U), NodeId(1U), 10U);
    high_desc.layer = 2;
    ret_code = CreateNode(tree, high_desc, "high child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc low_desc = MakeNodeDesc(NodeId(3U), NodeId(1U), 20U);
    low_desc.layer = 1;
    ret_code = CreateNode(tree, low_desc, "low child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc middle_desc = MakeNodeDesc(NodeId(4U), NodeId(1U), 5U);
    middle_desc.layer = 2;
    ret_code = CreateNode(tree, middle_desc, "middle child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc hidden_desc = MakeNodeDesc(NodeId(5U), NodeId(1U), 30U);
    hidden_desc.is_visible = false;
    ret_code = CreateNode(tree, hidden_desc, "hidden child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<UiDrawElementDesc, 4U> descs{
        UiDrawElementDesc{NodeId(2U), UiDrawElementType::Rect, 11U, 21U, 31U, 0U, true},
        UiDrawElementDesc{NodeId(4U), UiDrawElementType::Text, 12U, 22U, 0U, 41U, false},
        UiDrawElementDesc{NodeId(3U), UiDrawElementType::TexturedQuad, 13U, 23U, 33U, 0U, true},
        UiDrawElementDesc{NodeId(5U), UiDrawElementType::Rect, 14U, 24U, 34U, 0U, false}};
    std::array<UiDrawElement, 3U> elements{};
    UiDrawListResult result;
    UiDrawListBuilder builder;
    const UiDrawListStatus status = builder.Build(tree, descs, elements, &result);
    if (status != UiDrawListStatus::Success || result.element_count != 3U || result.skipped_node_count != 1U) {
        return Fail("draw list result mismatch");
    }

    if (elements[0U].node_id.value != 3U || elements[1U].node_id.value != 4U || elements[2U].node_id.value != 2U) {
        return Fail("draw list order was not deterministic");
    }

    if (elements[0U].type != UiDrawElementType::TexturedQuad || elements[0U].texture_key != 33U) {
        return Fail("textured quad keys were not preserved");
    }

    if (elements[1U].type != UiDrawElementType::Text || elements[1U].text_key != 41U) {
        return Fail("text keys were not preserved");
    }

    if (!elements[2U].scissor_enabled || elements[2U].style_key != 11U || elements[2U].material_key != 21U) {
        return Fail("rect draw element keys were not preserved");
    }

    return 0;
}

int UiCoreDrawListOutputCapacityReportsVisibleCount() {
    UiNodeTree tree(MakeTreeDesc());
    UiNodeDesc root_desc = MakeNodeDesc(NodeId(1U), UiNodeId{}, 0U);
    int ret_code = CreateNode(tree, root_desc, "root create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc high_desc = MakeNodeDesc(NodeId(2U), NodeId(1U), 10U);
    high_desc.layer = 2;
    ret_code = CreateNode(tree, high_desc, "high child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc low_desc = MakeNodeDesc(NodeId(3U), NodeId(1U), 20U);
    low_desc.layer = 1;
    ret_code = CreateNode(tree, low_desc, "low child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc middle_desc = MakeNodeDesc(NodeId(4U), NodeId(1U), 5U);
    middle_desc.layer = 2;
    ret_code = CreateNode(tree, middle_desc, "middle child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    UiNodeDesc hidden_desc = MakeNodeDesc(NodeId(5U), NodeId(1U), 30U);
    hidden_desc.is_visible = false;
    ret_code = CreateNode(tree, hidden_desc, "hidden child create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const std::array<UiDrawElementDesc, 4U> descs{
        UiDrawElementDesc{NodeId(2U), UiDrawElementType::Rect, 11U, 21U, 31U, 0U, true},
        UiDrawElementDesc{NodeId(4U), UiDrawElementType::Text, 12U, 22U, 0U, 41U, false},
        UiDrawElementDesc{NodeId(3U), UiDrawElementType::TexturedQuad, 13U, 23U, 33U, 0U, true},
        UiDrawElementDesc{NodeId(5U), UiDrawElementType::Rect, 14U, 24U, 34U, 0U, false}};
    std::array<UiDrawElement, 2U> elements{};
    elements[0U].node_id = NodeId(91U);
    elements[0U].style_key = 401U;
    elements[1U].node_id = NodeId(92U);
    elements[1U].style_key = 402U;

    UiDrawListResult result;
    UiDrawListBuilder builder;
    const UiDrawListStatus status = builder.Build(tree, descs, elements, &result);
    if (status != UiDrawListStatus::OutputCapacityExceeded ||
        result.status != UiDrawListStatus::OutputCapacityExceeded ||
        result.element_count != 3U ||
        result.skipped_node_count != 1U) {
        return Fail("small draw list output did not report required visible count");
    }

    if (elements[0U].node_id.value != 91U ||
        elements[0U].style_key != 401U ||
        elements[1U].node_id.value != 92U ||
        elements[1U].style_key != 402U) {
        return Fail("small draw list output mutated output elements");
    }

    return 0;
}

int UiCoreStaticAtlasMetadataResolvesSpritePageUvNineSlice() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakeAtlasPage(7U, 77U, 256U, 128U)};
    UiStaticAtlasSpriteDesc sprite = MakeAtlasSprite(11U, 7U, 64U, 32U, 64U, 32U);
    sprite.nine_slice_enabled = true;
    sprite.border_left = 4U;
    sprite.border_top = 6U;
    sprite.border_right = 8U;
    sprite.border_bottom = 10U;
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{sprite};

    UiStaticAtlasMetadata metadata;
    const UiStaticAtlasMetadataDesc desc{pages, sprites};
    const UiStaticAtlasResolveResult result = metadata.ResolveSprite(desc, 11U);
    if (!result.Succeeded()) {
        return Fail("static atlas sprite did not resolve");
    }

    if (result.page_key != 7U || result.texture_key != 77U) {
        return Fail("static atlas page or texture key mismatch");
    }

    if (!FloatClose(result.uv_rect.u_min, 0.25F) || !FloatClose(result.uv_rect.v_min, 0.25F)) {
        return Fail("static atlas uv min mismatch");
    }

    if (!FloatClose(result.uv_rect.u_max, 0.5F) || !FloatClose(result.uv_rect.v_max, 0.5F)) {
        return Fail("static atlas uv max mismatch");
    }

    if (!result.nine_slice.enabled || result.nine_slice.border_left != 4U || result.nine_slice.border_top != 6U) {
        return Fail("static atlas nine-slice leading borders mismatch");
    }

    if (result.nine_slice.border_right != 8U || result.nine_slice.border_bottom != 10U) {
        return Fail("static atlas nine-slice trailing borders mismatch");
    }

    return 0;
}

int UiCoreStaticAtlasMetadataMissingSpriteReportsStatus() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakeAtlasPage(7U, 77U, 256U, 128U)};
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{
        MakeAtlasSprite(11U, 7U, 64U, 32U, 64U, 32U)};

    UiStaticAtlasMetadata metadata;
    const UiStaticAtlasMetadataDesc desc{pages, sprites};
    const UiStaticAtlasResolveResult result = metadata.ResolveSprite(desc, 999U);
    if (result.status != UiStaticAtlasStatus::SpriteNotFound) {
        return Fail("missing sprite did not report explicit status");
    }

    if (result.Succeeded()) {
        return Fail("missing sprite reported success");
    }

    return 0;
}

int UiCoreStaticAtlasMetadataRejectsInvalidSpriteMetadata() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakeAtlasPage(7U, 77U, 256U, 128U)};
    const std::array<UiStaticAtlasSpriteDesc, 1U> overflow_sprites{
        MakeAtlasSprite(11U, 7U, 240U, 32U, 32U, 32U)};

    UiStaticAtlasMetadata metadata;
    UiStaticAtlasMetadataDesc desc{pages, overflow_sprites};
    UiStaticAtlasStatus status = metadata.Validate(desc);
    if (status != UiStaticAtlasStatus::InvalidSpriteRect) {
        return Fail("overflow sprite rect was not rejected");
    }

    UiStaticAtlasSpriteDesc nine_slice_sprite = MakeAtlasSprite(12U, 7U, 64U, 32U, 64U, 32U);
    nine_slice_sprite.nine_slice_enabled = true;
    nine_slice_sprite.border_left = 40U;
    nine_slice_sprite.border_right = 40U;
    const std::array<UiStaticAtlasSpriteDesc, 1U> nine_slice_sprites{nine_slice_sprite};
    desc = UiStaticAtlasMetadataDesc{pages, nine_slice_sprites};
    status = metadata.Validate(desc);
    if (status != UiStaticAtlasStatus::InvalidNineSlice) {
        return Fail("invalid nine-slice metadata was not rejected");
    }

    const std::array<UiStaticAtlasSpriteDesc, 1U> missing_page_sprites{
        MakeAtlasSprite(13U, 99U, 0U, 0U, 16U, 16U)};
    desc = UiStaticAtlasMetadataDesc{pages, missing_page_sprites};
    status = metadata.Validate(desc);
    if (status != UiStaticAtlasStatus::AtlasPageNotFound) {
        return Fail("missing atlas page was not rejected");
    }

    return 0;
}

int UiCoreNoLifecycleConfigEditorRenderBackendDependency() {
    UiNodeTree tree(MakeTreeDesc());
    UiNodeDesc desc = MakeNodeDesc(NodeId(9U), UiNodeId{}, 0U);
    const UiNodeTreeResult result = tree.CreateNode(desc);
    int ret_code = ExpectSuccess(result, "node create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const UiNodeTreeResult query_result = tree.QueryNode(NodeId(9U));
    ret_code = ExpectSuccess(query_result, "query failed");
    if (ret_code != 0) {
        return ret_code;
    }

    if (!query_result.record.is_visible || !query_result.record.is_enabled || !query_result.record.is_hit_testable) {
        return Fail("node flags were not preserved as value data");
    }

    if (query_result.record.layer != 3) {
        return Fail("node layer was not preserved as value data");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_NODE_TREE_CREATE_ORDER, UiCoreNodeTreeCreateAttachDetachOrder},
        {TEST_NODE_TREE_CREATE_CAPACITY_ENTRY, UiCoreNodeTreeCreateCapacityEntryReportsRejectedNode},
        {TEST_NODE_TREE_DESTROY, UiCoreNodeTreeDestroyRemovesDescendants},
        {TEST_RECT_MATH, UiCoreRectMathParentResizePivotMarginPaddingDpi},
        {TEST_NO_FORBIDDEN_DEPENDENCY, UiCoreNoLifecycleConfigEditorRenderBackendDependency},
        {TEST_LAYOUT_CONTAINERS, UiCoreLayoutContainersResolveExpectedRects},
        {TEST_DIRTY_TRACKER, UiCoreDirtyTrackerPaintOnlyDoesNotTriggerLayoutRebuild},
        {TEST_INVALIDATION_MODEL, UiCoreInvalidationModelSubtreeRulesExposeCacheCounters},
        {TEST_INVALIDATION_SMALL_OUTPUT, UiCoreInvalidationModelRejectsSmallOutputWithoutMutation},
        {TEST_INVALIDATION_CAPACITY_ENTRY, UiCoreInvalidationModelOutputCapacityReportsRejectedRequest},
        {TEST_HIT_TEST, UiCoreHitTestLayerClipDisabled},
        {TEST_DRAW_LIST, UiCoreDrawListDeterministicElements},
        {TEST_DRAW_LIST_SMALL_OUTPUT, UiCoreDrawListOutputCapacityReportsVisibleCount},
        {TEST_STATIC_ATLAS_RESOLVE, UiCoreStaticAtlasMetadataResolvesSpritePageUvNineSlice},
        {TEST_STATIC_ATLAS_MISSING, UiCoreStaticAtlasMetadataMissingSpriteReportsStatus},
        {TEST_STATIC_ATLAS_INVALID, UiCoreStaticAtlasMetadataRejectsInvalidSpriteMetadata}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
