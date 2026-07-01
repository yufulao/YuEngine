// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptNativeRegistry.h

#pragma once

#include <array>
#include <cstddef>

#include "YuEngine/Script/ScriptConstants.h"
#include "YuEngine/Script/ScriptNativeBinding.h"
#include "YuEngine/Script/ScriptNativeRegistrationResult.h"
#include "YuEngine/Script/ScriptNativeRegistryDesc.h"
#include "YuEngine/Script/ScriptSnapshot.h"
#include "YuEngine/Script/ScriptStatus.h"
#include "YuEngine/Script/ScriptValue.h"

namespace yuengine::script {
class ScriptNativeRegistry final {
public:
    /**
     * @comment 构造 ScriptNativeRegistry 实例。
     */
    ScriptNativeRegistry();
    /**
     * @comment 构造 ScriptNativeRegistry 实例。
     * @param desc 输入 descriptor。
     */
    explicit ScriptNativeRegistry(ScriptNativeRegistryDesc desc);

    /**
     * @comment 注册 native call binding。
     * @param binding 输入 native binding。
     * @return 显式操作结果。
     */
    ScriptNativeRegistrationResult RegisterNativeCall(const ScriptNativeBinding &binding);
    /**
     * @comment 按 stable call id 调用 native call。
     * @param call_id 输入 call id。
     * @param arguments 输入 argument slots。
     * @param argument_count 输入 argument count。
     * @param results 输出 result slots。
     * @param result_count 输入 result count。
     * @return 显式操作状态。
     */
    ScriptStatus Invoke(ScriptCallId call_id,
        const ScriptValue *arguments,
        std::uint32_t argument_count,
        ScriptValue *results,
        std::uint32_t result_count);
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    ScriptSnapshot Snapshot() const;

private:
    ScriptStatus RecordRegistryFailure(
        ScriptStatus status,
        std::uint32_t required_binding_count=0U);
    ScriptStatus RecordCallFailure(ScriptStatus status);
    ScriptStatus RecordCallSuccess();
    ScriptStatus ValidateBinding(const ScriptNativeBinding &binding) const;
    ScriptStatus ValidateValueTypes(const ScriptValue *values,
        const std::array<ScriptValueType, MAX_SCRIPT_ARGUMENT_COUNT> &expected_types,
        std::uint32_t value_count) const;
    ScriptStatus ValidateResultTypes(const ScriptValue *values,
        const std::array<ScriptValueType, MAX_SCRIPT_RESULT_COUNT> &expected_types,
        std::uint32_t value_count) const;
    const ScriptNativeBinding* FindBinding(ScriptCallId call_id) const;
    bool HasBinding(ScriptCallId call_id) const;

    std::array<ScriptNativeBinding, MAX_SCRIPT_NATIVE_BINDING_COUNT> bindings_;
    ScriptSnapshot snapshot_;
};
}
