// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageLoadPlanResult.h

#pragma once

#include <cstdint>

#include "YuEngine/Package/PackageEntryId.h"
#include "YuEngine/Package/PackageId.h"
#include "YuEngine/Package/PackageLoadPlan.h"
#include "YuEngine/Package/PackageStatus.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::package {
using resource::ResourceLogicalKey;
using resource::ResourceTypeId;

struct PackageLoadPlanResult final {
    PackageStatus status;
    PackageLoadPlan plan;
    std::uint32_t required_load_plan_record_count = 0U;
    PackageId failed_load_plan_package{};
    PackageEntryId failed_load_plan_entry_id{};
    ResourceTypeId failed_load_plan_resource_type{};
    ResourceLogicalKey failed_load_plan_resource_key{};
    std::uint32_t failed_load_plan_record_capacity = 0U;
    std::uint32_t failed_load_plan_record_count = 0U;

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
