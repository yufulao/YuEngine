// 模块: Tests UiCore
// 文件: Tests/UiCore/UiCoreTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
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
constexpr std::string_view TEST_HIT_TEST =
    "UiCore_HitTest_LayerClipDisabled";
constexpr std::string_view TEST_DRAW_LIST =
    "UiCore_DrawList_DeterministicElements";
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

    state = tracker.ApplyChange(UiDirtyChangeType::AtlasPage);
    if (state.HasDomain(UI_DIRTY_LAYOUT) || state.layout_rebuild_count != 0U) {
        return Fail("atlas page dirty triggered layout rebuild");
    }

    tracker.Clear();
    state = tracker.ApplyChange(UiDirtyChangeType::ScrollOffset);
    if (state.HasDomain(UI_DIRTY_LAYOUT)) {
        return Fail("scroll dirty triggered layout domain");
    }

    if (!state.HasDomain(UI_DIRTY_HIT_TEST) || state.layout_rebuild_count != 0U) {
        return Fail("scroll dirty did not classify hit-test domain");
    }

    tracker.Clear();
    state = tracker.ApplyChange(UiDirtyChangeType::Layout);
    if (!state.HasDomain(UI_DIRTY_LAYOUT) || state.layout_rebuild_count != 1U) {
        return Fail("layout dirty did not trigger one rebuild");
    }

    return 0;
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
        {TEST_NODE_TREE_DESTROY, UiCoreNodeTreeDestroyRemovesDescendants},
        {TEST_RECT_MATH, UiCoreRectMathParentResizePivotMarginPaddingDpi},
        {TEST_NO_FORBIDDEN_DEPENDENCY, UiCoreNoLifecycleConfigEditorRenderBackendDependency},
        {TEST_LAYOUT_CONTAINERS, UiCoreLayoutContainersResolveExpectedRects},
        {TEST_DIRTY_TRACKER, UiCoreDirtyTrackerPaintOnlyDoesNotTriggerLayoutRebuild},
        {TEST_HIT_TEST, UiCoreHitTestLayerClipDisabled},
        {TEST_DRAW_LIST, UiCoreDrawListDeterministicElements},
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
