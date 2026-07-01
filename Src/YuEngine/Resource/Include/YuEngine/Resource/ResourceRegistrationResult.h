// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceRegistrationResult.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"

namespace yuengine::resource {
struct ResourceRegistrationResult final {
    ResourceStatus status = ResourceStatus::InvalidDescriptor;
    ResourceHandle handle{};
    std::uint32_t required_resource_count = 0U;
    std::uint32_t required_type_count = 0U;
    std::uint32_t required_dependency_edge_count = 0U;

    /**
     * @comment 创建成功结果。
     * @param handle 输入句柄。
     * @return 显式操作结果。
     */
    static ResourceRegistrationResult Success(ResourceHandle handle) {
        ResourceRegistrationResult result{};
        result.status = ResourceStatus::Success;
        result.handle = handle;
        return result;
    }

    /**
     * @comment 创建失败结果。
     * @param status 输入状态。
     * @return 显式操作结果。
     */
    static ResourceRegistrationResult Failure(ResourceStatus status) {
        return Failure(status, 0U, 0U, 0U);
    }

    /**
     * @comment 创建带容量诊断的失败结果。
     * @param status 输入状态。
     * @param required_resource_count 所需 resource 容量。
     * @param required_type_count 所需 type 容量。
     * @param required_dependency_edge_count 所需 dependency edge 容量。
     * @return 显式操作结果。
     */
    static ResourceRegistrationResult Failure(
        ResourceStatus status,
        std::uint32_t required_resource_count,
        std::uint32_t required_type_count,
        std::uint32_t required_dependency_edge_count) {
        ResourceRegistrationResult result{};
        result.status = status;
        result.required_resource_count = required_resource_count;
        result.required_type_count = required_type_count;
        result.required_dependency_edge_count = required_dependency_edge_count;
        return result;
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
