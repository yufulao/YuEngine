// Module: YuEngine Script
// File: Src/YuEngine/Script/Include/YuEngine/Script/ScriptNativeRegistrationResult.h

#pragma once

#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/Script/ScriptStatus.h"

namespace yuengine::script {
struct ScriptNativeRegistrationResult final {
    ScriptStatus status = ScriptStatus::Success;
    ScriptCallId call_id{};

    /**
     * @comment Creates a successful result.
     * @param call_id Input call id.
     * @return Explicit operation result.
     */
    static ScriptNativeRegistrationResult Success(ScriptCallId call_id) {
        return ScriptNativeRegistrationResult{ScriptStatus::Success, call_id};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input status.
     * @return Explicit operation result.
     */
    static ScriptNativeRegistrationResult Failure(ScriptStatus status) {
        return ScriptNativeRegistrationResult{status, ScriptCallId{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool Succeeded() const {
        return status == ScriptStatus::Success;
    }
};
}
