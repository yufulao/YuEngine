// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceRegistrationResult.h

#pragma once

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"

namespace yuengine::resource {
struct ResourceRegistrationResult final {
    ResourceStatus status;
    ResourceHandle handle;

    /**
     * @comment 创建成功结果。
     * @param handle 输入句柄。
     * @return 显式操作结果。
     */
    static ResourceRegistrationResult Success(ResourceHandle handle) {
        return ResourceRegistrationResult{ResourceStatus::Success, handle};
    }

    /**
     * @comment 创建失败结果。
     * @param status 输入 状态。
     * @return 显式操作结果。
     */
    static ResourceRegistrationResult Failure(ResourceStatus status) {
        return ResourceRegistrationResult{status, ResourceHandle{}};
    }

    /**
     * @comment 检查结果是否成功。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == ResourceStatus::Success;
    }
};
}
