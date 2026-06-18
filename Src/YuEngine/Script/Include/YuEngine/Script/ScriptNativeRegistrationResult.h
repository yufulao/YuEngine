// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptNativeRegistrationResult.h

#pragma once

#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/Script/ScriptStatus.h"

namespace yuengine::script {
struct ScriptNativeRegistrationResult final {
    ScriptStatus status = ScriptStatus::Success;
    ScriptCallId call_id{};

    /**
     * @comment 创建成功 result。
     * @param call_id 输入 call id。
     * @return 显式操作结果。
     */
    static ScriptNativeRegistrationResult Success(ScriptCallId call_id) {
        return ScriptNativeRegistrationResult{ScriptStatus::Success, call_id};
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 status。
     * @return 显式操作结果。
     */
    static ScriptNativeRegistrationResult Failure(ScriptStatus status) {
        return ScriptNativeRegistrationResult{status, ScriptCallId{}};
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
