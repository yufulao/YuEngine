// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptRuntimePhaseDispatchAdapter.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Script/ScriptConstants.h"
#include "YuEngine/Script/ScriptRuntimePhase.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchAdapterDesc.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchBinding.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchResult.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchSnapshot.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchStatus.h"
#include "YuEngine/Script/ScriptRuntimePhaseTrace.h"
#include "YuEngine/Script/ScriptStatus.h"

namespace yuengine::script {
class ScriptNativeRegistry;
struct ScriptValue;

class ScriptRuntimePhaseDispatchAdapter final {
public:
    /**
     * @comment 构造 runtime phase dispatch adapter。
     * @param desc 输入 adapter descriptor。
     */
    explicit ScriptRuntimePhaseDispatchAdapter(
        ScriptRuntimePhaseDispatchAdapterDesc desc=ScriptRuntimePhaseDispatchAdapterDesc{});

    /**
     * @comment 将 runtime phase 绑定到 stable script call id。
     * @param phase 输入 runtime phase。
     * @param call_id 输入 script call id。
     * @return 显式操作结果。
     */
    ScriptRuntimePhaseDispatchResult Bind(ScriptRuntimePhase phase, ScriptCallId call_id);
    /**
     * @comment 按 trace order dispatch 已绑定 runtime phases。
     * @param registry dispatch 使用的 native registry。
     * @param phase_trace 输入 runtime phase trace records。
     * @param phase_trace_count 输入 trace record count。
     * @param arguments 调用方持有的 argument slots。
     * @param argument_count 输入 argument slot count。
     * @param results 调用方持有的 result slots。
     * @param result_count 输入 result slot count。
     * @return 显式操作状态。
     */
    ScriptRuntimePhaseDispatchStatus DispatchTrace(
        ScriptNativeRegistry &registry,
        const ScriptRuntimePhaseTrace *phase_trace,
        std::uint32_t phase_trace_count,
        const ScriptValue *arguments,
        std::uint32_t argument_count,
        ScriptValue *results,
        std::uint32_t result_count);
    /**
     * @comment 返回当前 adapter 状态快照。
     * @return 快照值。
     */
    ScriptRuntimePhaseDispatchSnapshot Snapshot() const;

private:
    ScriptRuntimePhaseDispatchStatus RecordBindFailure(
        ScriptRuntimePhaseDispatchStatus status,
        ScriptRuntimePhase phase,
        ScriptCallId call_id);
    ScriptRuntimePhaseDispatchStatus RecordDispatchFailure(ScriptRuntimePhaseDispatchStatus status);
    ScriptRuntimePhaseDispatchStatus RecordDispatchFailure(
        ScriptRuntimePhaseDispatchStatus status,
        ScriptStatus script_status);
    ScriptRuntimePhaseDispatchStatus RecordTraceCapacityFailure(
        const ScriptRuntimePhaseTrace *phase_trace,
        std::uint32_t phase_trace_count);
    void RecordSuccess(ScriptStatus script_status);
    void ClearCapacityIdentity();
    ScriptRuntimePhaseDispatchStatus ValidateAdapterCapacity() const;
    ScriptRuntimePhaseDispatchStatus ValidateDispatchInputs(
        const ScriptRuntimePhaseTrace *phase_trace,
        std::uint32_t phase_trace_count,
        const ScriptValue *arguments,
        std::uint32_t argument_count,
        const ScriptValue *results,
        std::uint32_t result_count) const;
    ScriptRuntimePhaseDispatchStatus MapScriptFailure(ScriptStatus script_status) const;
    bool IsPhaseValid(ScriptRuntimePhase phase) const;
    ScriptRuntimePhaseDispatchBinding *FindFreeBinding();
    const ScriptRuntimePhaseDispatchBinding *FindBindingByPhase(ScriptRuntimePhase phase) const;

    std::array<ScriptRuntimePhaseDispatchBinding, MAX_SCRIPT_RUNTIME_PHASE_DISPATCH_BINDING_COUNT> bindings_;
    ScriptRuntimePhaseDispatchSnapshot snapshot_;
};
}
