// 模块: YuEngine Object
// 文件: Src/YuEngine/Object/Include/YuEngine/Object/ObjectRegistrationResult.h

#pragma once

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Object/ObjectStatus.h"

namespace yuengine::object {
struct ObjectRegistrationResult final {
    ObjectStatus status;
    ObjectHandle handle;

    /**
     * @comment 创建成功 result。
     * @param handle 输入 handle。
     * @return 显式操作结果。
     */
    static ObjectRegistrationResult Success(ObjectHandle handle) {
        return ObjectRegistrationResult{ObjectStatus::Success, handle};
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 status。
     * @return 显式操作结果。
     */
    static ObjectRegistrationResult Failure(ObjectStatus status) {
        return ObjectRegistrationResult{status, ObjectHandle{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == ObjectStatus::Success;
    }
};
}
