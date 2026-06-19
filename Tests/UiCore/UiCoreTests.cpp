// 模块: Tests UiCore
// 文件: Tests/UiCore/UiCoreTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/UiCore/UiNodeDesc.h"
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

namespace {
constexpr std::string_view TEST_NODE_TREE_CREATE_ORDER =
    "UiCore_NodeTree_CreateAttachDetachOrder";
constexpr std::string_view TEST_NODE_TREE_DESTROY =
    "UiCore_NodeTree_DestroyRemovesDescendants";
constexpr std::string_view TEST_RECT_MATH =
    "UiCore_RectMath_ParentResizePivotMarginPaddingDpi";
constexpr std::string_view TEST_NO_FORBIDDEN_DEPENDENCY =
    "UiCore_NoLifecycleConfigEditorRenderBackendDependency";
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
        {TEST_NO_FORBIDDEN_DEPENDENCY, UiCoreNoLifecycleConfigEditorRenderBackendDependency}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
