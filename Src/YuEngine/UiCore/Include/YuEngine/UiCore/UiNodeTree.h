// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiNodeTree.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/UiCore/UiCoreConstants.h"
#include "YuEngine/UiCore/UiNodeDesc.h"
#include "YuEngine/UiCore/UiNodeRecord.h"
#include "YuEngine/UiCore/UiNodeTreeDesc.h"
#include "YuEngine/UiCore/UiNodeTreeResult.h"
#include "YuEngine/UiCore/UiNodeTreeSnapshot.h"
#include "YuEngine/UiCore/UiNodeTreeStatus.h"
#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiRectTransform.h"

namespace yuengine::uicore {
class UiNodeTree final {
public:
    /**
     * @comment 构造 UI node tree。
     */
    UiNodeTree();
    /**
     * @comment 构造 UI node tree。
     * @param desc 输入 tree descriptor。
     */
    explicit UiNodeTree(UiNodeTreeDesc desc);

    /**
     * @comment 创建 UI node。
     * @param desc 输入 node descriptor。
     * @return 显式操作结果。
     */
    UiNodeTreeResult CreateNode(const UiNodeDesc &desc);
    /**
     * @comment 销毁 UI node 及其 child nodes。
     * @param node_id 输入 node id。
     * @return 显式操作结果。
     */
    UiNodeTreeResult DestroyNode(UiNodeId node_id);
    /**
     * @comment 将 UI node 挂到 parent node。
     * @param node_id 输入 node id。
     * @param parent_id 输入 parent id。
     * @param sibling_order 输入 sibling order。
     * @return 显式操作结果。
     */
    UiNodeTreeResult AttachNode(UiNodeId node_id, UiNodeId parent_id, std::uint32_t sibling_order);
    /**
     * @comment 将 UI node 从 parent node 分离。
     * @param node_id 输入 node id。
     * @param sibling_order 输入新的 root order。
     * @return 显式操作结果。
     */
    UiNodeTreeResult DetachNode(UiNodeId node_id, std::uint32_t sibling_order);
    /**
     * @comment 更新 UI node rect transform。
     * @param node_id 输入 node id。
     * @param rect_transform 输入 rect transform。
     * @return 显式操作结果。
     */
    UiNodeTreeResult SetNodeRect(UiNodeId node_id, const UiRectTransform &rect_transform);
    /**
     * @comment 更新 viewport rect 并刷新 root node cache。
     * @param viewport_rect 输入 viewport rect。
     * @return 显式操作状态。
     */
    UiNodeTreeStatus SetViewportRect(UiRect viewport_rect);
    /**
     * @comment 查询 UI node。
     * @param node_id 输入 node id。
     * @return 显式操作结果。
     */
    UiNodeTreeResult QueryNode(UiNodeId node_id) const;
    /**
     * @comment 按 sibling order 导出 child nodes。
     * @param parent_id 输入 parent id；无效 id 表示 root nodes。
     * @param output_records 调用方持有的 output buffer。
     * @param output_capacity output buffer capacity。
     * @return 匹配 child node 总数。
     */
    std::uint32_t ExportChildren(
        UiNodeId parent_id,
        UiNodeRecord *output_records,
        std::uint32_t output_capacity) const;
    /**
     * @comment 返回当前 tree 状态快照。
     * @return 快照值。
     */
    UiNodeTreeSnapshot Snapshot() const;

private:
    UiNodeTreeStatus RecordFailure(UiNodeTreeStatus status);
    void RecordSuccess();
    UiNodeTreeResult RecordNodeCapacityFailure(const UiNodeDesc &desc);
    void ClearNodeCapacityEntry();
    UiNodeTreeStatus ValidateDesc(const UiNodeDesc &desc, UiNodeRecord &out_record) const;
    UiNodeTreeStatus ResolveRecord(const UiNodeDesc &desc, UiNodeRecord &out_record) const;
    UiNodeTreeStatus ResolveRecordInPlace(UiNodeRecord &record);
    UiNodeTreeStatus ResolveDescendants(UiNodeId parent_id);
    UiNodeRecord *FindRecord(UiNodeId node_id);
    const UiNodeRecord *FindRecord(UiNodeId node_id) const;
    UiNodeRecord *FindFreeRecord();
    bool WouldCreateCycle(UiNodeId node_id, UiNodeId parent_id) const;
    bool ChildMatchesParent(const UiNodeRecord &record, UiNodeId parent_id) const;
    std::uint32_t DestroyRecordAndDescendants(UiNodeRecord &record);
    void RecountActiveNodes();

    std::array<UiNodeRecord, MAX_UI_NODE_COUNT> records_;
    UiRect viewport_rect_;
    UiNodeTreeSnapshot snapshot_;
};
}
