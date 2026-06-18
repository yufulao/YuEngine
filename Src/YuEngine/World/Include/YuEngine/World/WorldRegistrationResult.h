// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldRegistrationResult.h

#pragma once

#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldStatus.h"

namespace yuengine::world {
struct WorldRegistrationResult final {
    WorldStatus status = WorldStatus::Success;
    WorldObjectId id{};

    /**
     * @comment 创建成功 result。
     * @param id 输入 world object id。
     * @return 显式操作结果。
     */
    static WorldRegistrationResult Success(WorldObjectId id) {
        return WorldRegistrationResult{WorldStatus::Success, id};
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 status。
     * @return 显式操作结果。
     */
    static WorldRegistrationResult Failure(WorldStatus status) {
        return WorldRegistrationResult{status, WorldObjectId{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldStatus::Success;
    }
};
}
