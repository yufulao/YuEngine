// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldIdentityBaselineResult.h

#pragma once

#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldIdentityBaselineRecord.h"
#include "YuEngine/World/WorldIdentityBaselineStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldStatus.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldIdentityBaselineResult final {
    WorldIdentityBaselineStatus status = WorldIdentityBaselineStatus::Success;
    WorldIdentityBaselineRecord record{};
    yuengine::object::ObjectStatus object_status = yuengine::object::ObjectStatus::Success;
    WorldStatus world_status = WorldStatus::Success;
    WorldObjectIdentityStatus identity_status = WorldObjectIdentityStatus::Success;
    WorldTransformStatus transform_status = WorldTransformStatus::Success;
    WorldComponentAttachmentStatus component_status = WorldComponentAttachmentStatus::Success;

    /**
     * @comment 创建成功 result。
     * @param record 输入 baseline record。
     * @return 显式操作结果。
     */
    static WorldIdentityBaselineResult Success(const WorldIdentityBaselineRecord &record) {
        return WorldIdentityBaselineResult{
            WorldIdentityBaselineStatus::Success,
            record,
            yuengine::object::ObjectStatus::Success,
            WorldStatus::Success,
            WorldObjectIdentityStatus::Success,
            WorldTransformStatus::Success,
            WorldComponentAttachmentStatus::Success};
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 baseline status。
     * @return 显式操作结果。
     */
    static WorldIdentityBaselineResult Failure(WorldIdentityBaselineStatus status) {
        return WorldIdentityBaselineResult{
            status,
            WorldIdentityBaselineRecord{},
            yuengine::object::ObjectStatus::Success,
            WorldStatus::Success,
            WorldObjectIdentityStatus::Success,
            WorldTransformStatus::Success,
            WorldComponentAttachmentStatus::Success};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldIdentityBaselineStatus::Success;
    }
};
}
