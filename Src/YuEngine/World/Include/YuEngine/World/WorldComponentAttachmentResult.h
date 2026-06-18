// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentResult.h

#pragma once

#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldComponentAttachmentResult final {
    WorldComponentAttachmentStatus status = WorldComponentAttachmentStatus::Success;
    WorldObjectId world_object_id{};
    WorldComponentTypeId component_type_id{};
    WorldComponentSlotId component_slot_id{};

    /**
     * @comment 创建成功 result。
     * @param world_object_id 输入 world object id。
     * @param component_type_id 输入 component type id。
     * @param component_slot_id 输入 component slot id。
     * @return 显式操作结果。
     */
    static WorldComponentAttachmentResult Success(
        WorldObjectId world_object_id,
        WorldComponentTypeId component_type_id,
        WorldComponentSlotId component_slot_id) {
        return WorldComponentAttachmentResult{
            WorldComponentAttachmentStatus::Success,
            world_object_id,
            component_type_id,
            component_slot_id};
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 attachment status。
     * @return 显式操作结果。
     */
    static WorldComponentAttachmentResult Failure(WorldComponentAttachmentStatus status) {
        return WorldComponentAttachmentResult{
            status,
            WorldObjectId{},
            WorldComponentTypeId{},
            WorldComponentSlotId{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldComponentAttachmentStatus::Success;
    }
};
}
