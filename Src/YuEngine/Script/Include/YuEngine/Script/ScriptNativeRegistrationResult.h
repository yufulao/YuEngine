// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptNativeRegistrationResult.h

#pragma once

#include <cstdint>

#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/Script/ScriptStatus.h"

namespace yuengine::script {
struct ScriptNativeRegistrationResult final {
    ScriptStatus status = ScriptStatus::Success;
    ScriptCallId call_id{};
    std::uint32_t required_binding_count = 0U;
    std::uint32_t binding_capacity = 0U;
    std::uint32_t binding_count = 0U;
    std::uint32_t failed_binding_index = 0U;
    ScriptCallId failed_call_id{};

    /**
     * @comment 创建成功 result。
     * @param call_id 输入 call id。
     * @return 显式操作结果。
     */
    static ScriptNativeRegistrationResult Success(ScriptCallId call_id) {
        ScriptNativeRegistrationResult result{};
        result.status = ScriptStatus::Success;
        result.call_id = call_id;
        return result;
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 status。
     * @param required_binding_count 输入 required binding 数量。
     * @return 显式操作结果。
     */
    static ScriptNativeRegistrationResult Failure(
        ScriptStatus status,
        std::uint32_t required_binding_count=0U) {
        ScriptNativeRegistrationResult result{};
        result.status = status;
        result.required_binding_count = required_binding_count;
        return result;
    }

    /**
     * @comment 检查 result 是否成功。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == ScriptStatus::Success;
    }
};
}
