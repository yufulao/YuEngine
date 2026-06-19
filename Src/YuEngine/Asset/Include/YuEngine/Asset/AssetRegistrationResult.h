// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetRegistrationResult.h

#pragma once

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Asset/AssetStatus.h"

namespace yuengine::asset {
struct AssetRegistrationResult final {
    AssetStatus status = AssetStatus::Success;
    AssetHandle handle;

    /**
     * @comment 创建成功结果。
     * @param handle 输入句柄。
     * @return 显式操作结果。
     */
    static AssetRegistrationResult Success(AssetHandle handle) {
        return AssetRegistrationResult{AssetStatus::Success, handle};
    }

    /**
     * @comment 创建失败结果。
     * @param status 输入状态。
     * @return 显式操作结果。
     */
    static AssetRegistrationResult Failure(AssetStatus status) {
        return AssetRegistrationResult{status, AssetHandle{}};
    }

    /**
     * @comment 检查结果是否成功。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == AssetStatus::Success;
    }
};
}
