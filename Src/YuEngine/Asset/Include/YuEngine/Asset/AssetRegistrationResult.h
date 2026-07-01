// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetRegistrationResult.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Asset/AssetStatus.h"
#include "YuEngine/Asset/AssetTypeId.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::asset {
struct AssetRegistrationResult final {
    AssetStatus status = AssetStatus::Success;
    AssetHandle handle;
    std::uint32_t required_asset_count = 0U;
    std::uint32_t required_type_count = 0U;
    std::uint32_t required_dependency_edge_count = 0U;
    std::uint64_t capacity_entry_asset_id = 0U;
    yuengine::resource::ResourceHandle capacity_entry_resource_handle;
    yuengine::resource::ResourceTypeId capacity_entry_resource_type;
    AssetTypeId capacity_entry_asset_type;
    std::uint32_t capacity_entry_asset_capacity = 0U;
    std::uint32_t capacity_entry_type_capacity = 0U;
    std::uint32_t capacity_entry_dependency_edge_capacity = 0U;
    std::uint32_t capacity_entry_asset_count = 0U;
    std::uint32_t capacity_entry_type_count = 0U;
    std::uint32_t capacity_entry_dependency_edge_count = 0U;

    /**
     * @comment 创建成功结果。
     * @param handle 输入句柄。
     * @return 显式操作结果。
     */
    static AssetRegistrationResult Success(AssetHandle handle) {
        AssetRegistrationResult result{};
        result.status = AssetStatus::Success;
        result.handle = handle;
        return result;
    }

    /**
     * @comment 创建失败结果。
     * @param status 输入状态。
     * @param required_asset_count 输入 required asset 数量。
     * @param required_type_count 输入 required type 数量。
     * @param required_dependency_edge_count 输入 required dependency edge 数量。
     * @return 显式操作结果。
     */
    static AssetRegistrationResult Failure(
        AssetStatus status,
        std::uint32_t required_asset_count=0U,
        std::uint32_t required_type_count=0U,
        std::uint32_t required_dependency_edge_count=0U) {
        AssetRegistrationResult result{};
        result.status = status;
        result.required_asset_count = required_asset_count;
        result.required_type_count = required_type_count;
        result.required_dependency_edge_count = required_dependency_edge_count;
        return result;
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
