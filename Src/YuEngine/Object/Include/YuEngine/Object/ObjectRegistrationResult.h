// 模块: YuEngine Object
// 文件: Src/YuEngine/Object/Include/YuEngine/Object/ObjectRegistrationResult.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Object/ObjectStatus.h"

namespace yuengine::object {
struct ObjectRegistrationResult final {
    ObjectStatus status = ObjectStatus::InvalidType;
    ObjectHandle handle{};
    std::uint32_t required_object_count = 0U;
    std::uint32_t required_type_count = 0U;

    /**
     * @comment 创建成功 result。
     * @param handle 输入 handle。
     * @param required_object_count 输入 required object 数量。
     * @param required_type_count 输入 required type 数量。
     * @return 显式操作结果。
     */
    static ObjectRegistrationResult Success(
        ObjectHandle handle,
        std::uint32_t required_object_count=0U,
        std::uint32_t required_type_count=0U) {
        ObjectRegistrationResult result{};
        result.status = ObjectStatus::Success;
        result.handle = handle;
        result.required_object_count = required_object_count;
        result.required_type_count = required_type_count;
        return result;
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 status。
     * @param required_object_count 输入 required object 数量。
     * @param required_type_count 输入 required type 数量。
     * @return 显式操作结果。
     */
    static ObjectRegistrationResult Failure(
        ObjectStatus status,
        std::uint32_t required_object_count=0U,
        std::uint32_t required_type_count=0U) {
        ObjectRegistrationResult result{};
        result.status = status;
        result.required_object_count = required_object_count;
        result.required_type_count = required_type_count;
        return result;
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
