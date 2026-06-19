// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptRuntimePhaseDispatchResult.h

#pragma once

#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/Script/ScriptRuntimePhase.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchStatus.h"

namespace yuengine::script {
struct ScriptRuntimePhaseDispatchResult final {
    ScriptRuntimePhaseDispatchStatus status = ScriptRuntimePhaseDispatchStatus::Success;
    ScriptRuntimePhase phase = ScriptRuntimePhase::BeginFrame;
    ScriptCallId call_id{};

    /**
     * @comment 创建成功结果。
     * @param phase 输入 runtime phase。
     * @param call_id 输入 script call id。
     * @return 显式操作结果。
     */
    static ScriptRuntimePhaseDispatchResult Success(ScriptRuntimePhase phase, ScriptCallId call_id) {
        return ScriptRuntimePhaseDispatchResult{ScriptRuntimePhaseDispatchStatus::Success, phase, call_id};
    }

    /**
     * @comment 创建失败结果。
     * @param status 输入失败状态。
     * @return 显式操作结果。
     */
    static ScriptRuntimePhaseDispatchResult Failure(ScriptRuntimePhaseDispatchStatus status) {
        return ScriptRuntimePhaseDispatchResult{status, ScriptRuntimePhase::BeginFrame, ScriptCallId{}};
    }

    /**
     * @comment 判断操作是否成功。
     * @return 成功返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == ScriptRuntimePhaseDispatchStatus::Success;
    }
};
}
