// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldScriptDispatchBridge.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/Script/ScriptStatus.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldPhaseTrace.h"
#include "YuEngine/World/WorldScriptDispatchBinding.h"
#include "YuEngine/World/WorldScriptDispatchBridgeDesc.h"
#include "YuEngine/World/WorldScriptDispatchConstants.h"
#include "YuEngine/World/WorldScriptDispatchResult.h"
#include "YuEngine/World/WorldScriptDispatchSnapshot.h"
#include "YuEngine/World/WorldScriptDispatchStatus.h"
#include "YuEngine/World/WorldUpdatePhase.h"

namespace yuengine::script {
class ScriptNativeRegistry;
struct ScriptValue;
}

namespace yuengine::world {
class WorldScriptDispatchBridge final {
public:
    /**
     * @comment 构造 world-script dispatch bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldScriptDispatchBridge(WorldScriptDispatchBridgeDesc desc=WorldScriptDispatchBridgeDesc{});

    /**
     * @comment 绑定 one world update phase to one script call id。
     * @param phase 输入 world update phase。
     * @param call_id 输入 script call id。
     * @return 显式操作结果。
     */
    WorldScriptDispatchResult Bind(WorldUpdatePhase phase,
        yuengine::script::ScriptCallId call_id);
    /**
     * @comment 按 trace order dispatch bound world phases。
     * @param registry dispatch 使用的 Script native registry。
     * @param phase_trace 输入 world phase trace records。
     * @param phase_trace_count 输入 trace record count。
     * @param arguments 调用方持有的 argument slots。
     * @param argument_count 输入 argument slot count。
     * @param results 调用方持有的 result slots。
     * @param result_count 输入 result slot count。
     * @return 显式操作状态。
     */
    WorldScriptDispatchStatus DispatchTrace(
        yuengine::script::ScriptNativeRegistry &registry,
        const WorldPhaseTrace *phase_trace,
        std::uint32_t phase_trace_count,
        const yuengine::script::ScriptValue *arguments,
        std::uint32_t argument_count,
        yuengine::script::ScriptValue *results,
        std::uint32_t result_count);
    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
     */
    WorldScriptDispatchSnapshot Snapshot() const;

private:
    WorldScriptDispatchStatus RecordBindFailure(WorldScriptDispatchStatus status);
    WorldScriptDispatchStatus RecordDispatchFailure(WorldScriptDispatchStatus status);
    WorldScriptDispatchStatus RecordDispatchFailure(WorldScriptDispatchStatus status,
        yuengine::script::ScriptStatus script_status);
    void RecordSuccess(yuengine::script::ScriptStatus script_status);
    WorldScriptDispatchStatus ValidateBridgeCapacity() const;
    WorldScriptDispatchStatus ValidateDispatchInputs(
        const WorldPhaseTrace *phase_trace,
        std::uint32_t phase_trace_count,
        const yuengine::script::ScriptValue *arguments,
        std::uint32_t argument_count,
        const yuengine::script::ScriptValue *results,
        std::uint32_t result_count) const;
    bool IsPhaseValid(WorldUpdatePhase phase) const;
    WorldScriptDispatchBinding *FindFreeBinding();
    const WorldScriptDispatchBinding *FindBindingByPhase(WorldUpdatePhase phase) const;

    std::array<WorldScriptDispatchBinding, MAX_WORLD_SCRIPT_DISPATCH_BINDING_COUNT> bindings_;
    WorldScriptDispatchSnapshot snapshot_;
};
}
