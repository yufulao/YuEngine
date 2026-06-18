// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageLoadPlanResult.h

#pragma once

#include "YuEngine/Package/PackageLoadPlan.h"
#include "YuEngine/Package/PackageStatus.h"

namespace yuengine::package {
struct PackageLoadPlanResult final {
    PackageStatus status;
    PackageLoadPlan plan;

    /**
     * @comment 创建成功 result。
     * @param plan 输入 plan。
     * @return 显式操作结果。
     */
    static PackageLoadPlanResult Success(PackageLoadPlan plan);
    /**
     * @comment 创建失败 result。
     * @param status 输入 status。
     * @return 显式操作结果。
     */
    static PackageLoadPlanResult Failure(PackageStatus status);
    /**
     * @comment 检查 result 是否成功。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Succeeded() const;
};
}
