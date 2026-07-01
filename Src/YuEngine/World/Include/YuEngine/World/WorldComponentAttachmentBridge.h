// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentBridge.h

#pragma once

#include <array>

#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldComponentAttachmentBridgeDesc.h"
#include "YuEngine/World/WorldComponentAttachmentResult.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshot.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldComponentAttachmentExportResult final {
    WorldComponentAttachmentStatus status = WorldComponentAttachmentStatus::Success;
    std::uint32_t required_attachment_count = 0U;
    std::uint32_t exported_attachment_count = 0U;

    /**
     * @comment 检查 export result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldComponentAttachmentStatus::Success;
    }
};

class WorldComponentAttachmentBridge final {
public:
    /**
     * @comment 构造 world component attachment bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldComponentAttachmentBridge(
        WorldComponentAttachmentBridgeDesc desc=WorldComponentAttachmentBridgeDesc{});

    /**
     * @comment 添加一个调用方提供的 component slot attachment。
     * @param world_object_id 输入 world object id。
     * @param component_type_id 输入 component type id。
     * @param component_slot_id 输入 component slot id。
     * @return 显式操作结果。
     */
    WorldComponentAttachmentResult Add(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id);
    /**
     * @comment 按 world object id 和 component type id 查询一个 component attachment。
     * @param world_object_id 输入 world object id。
     * @param component_type_id 输入 component type id。
     * @return 显式操作结果。
     */
    WorldComponentAttachmentResult Query(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id);
    /**
     * @comment 移除一个 component attachment。
     * @param world_object_id 输入 world object id。
     * @param component_type_id 输入 component type id。
     * @return 显式操作状态。
     */
    WorldComponentAttachmentStatus Remove(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id);
    /**
     * @comment 清空 all component attachments in deterministic slot order。
     * @return 显式操作状态。
     */
    WorldComponentAttachmentStatus Clear();
    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
     */
    WorldComponentAttachmentSnapshot Snapshot() const;
    /**
     * @comment 校验 explicit restore preflight 的 destination state。
     * @param required_attachment_count Required attachment 容量。
     * @return 显式操作状态。
     */
    WorldComponentAttachmentStatus ValidateRestoreDestination(
        std::uint32_t required_attachment_count) const;
    /**
     * @comment 按 deterministic slot order 复制 active attachment records。
     * @param output_attachments 调用方持有的 output attachment buffer。
     * @param output_capacity 输出 attachment buffer capacity。
     * @return active attachment 总数。
     */
    std::uint32_t ExportAttachments(
        WorldComponentAttachment *output_attachments,
        std::uint32_t output_capacity) const;
    /**
     * @comment 按 deterministic slot order 复制 active attachment records，容量不足时不写 output buffer。
     * @param output_attachments 调用方持有的 output attachment buffer。
     * @param output_capacity 输出 attachment buffer capacity。
     * @return 显式 export 结果，包含 required attachment count。
     */
    WorldComponentAttachmentExportResult ExportAttachmentsChecked(
        WorldComponentAttachment *output_attachments,
        std::uint32_t output_capacity) const;

private:
    WorldComponentAttachmentResult RecordFailureResult(WorldComponentAttachmentStatus status);
    WorldComponentAttachmentResult RecordDuplicateFailureResult();
    WorldComponentAttachmentStatus RecordFailure(WorldComponentAttachmentStatus status);
    void RecordSuccess();
    WorldComponentAttachmentStatus ValidateBridgeCapacity() const;
    WorldComponentAttachment *FindAttachment(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id);
    const WorldComponentAttachment *FindAttachment(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id) const;
    WorldComponentAttachment *FindFreeAttachment();
    void ClearAttachment(WorldComponentAttachment &attachment);
    void RecountActiveAttachments();

    std::array<WorldComponentAttachment, MAX_WORLD_OBJECT_COUNT> attachments_;
    WorldComponentAttachmentSnapshot snapshot_;
};
}
