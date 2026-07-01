// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageRegistrationResult.h

#pragma once

#include <cstdint>

#include "YuEngine/Package/PackageEntryId.h"
#include "YuEngine/Package/PackageId.h"
#include "YuEngine/Package/PackageStatus.h"

namespace yuengine::package {
struct PackageRegistrationResult final {
    PackageStatus status;
    PackageId package;
    PackageEntryId entry;
    std::uint32_t required_manifest_record_count = 0U;
    std::uint32_t required_entry_record_count = 0U;

    /**
     * @comment 创建成功 manifest registration result。
     * @param package 输入 package。
     * @return 显式操作结果。
     */
    static PackageRegistrationResult ManifestSuccess(PackageId package);
    /**
     * @comment 创建成功 entry registration result。
     * @param package 输入 package。
     * @param entry 输入 entry。
     * @return 显式操作结果。
     */
    static PackageRegistrationResult EntrySuccess(PackageId package, PackageEntryId entry);
    /**
     * @comment 创建失败 result。
     * @param status 输入 status。
     * @return 显式操作结果。
     */
    static PackageRegistrationResult Failure(PackageStatus status);
    /**
     * @comment 检查 result 是否成功。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Succeeded() const;
};
}
