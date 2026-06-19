// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/RuntimeDiagnosticsCounterRecorder.h

#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/BoundedDiagnosticsChannel.h"
#include "YuEngine/Diagnostics/DisabledDiagnosticsChannel.h"
#include "YuEngine/Diagnostics/RuntimeDiagnosticsCounters.h"
#include "YuEngine/Diagnostics/RuntimeDiagnosticsRecordResult.h"

namespace yuengine::diagnostics {
class RuntimeDiagnosticsCounterRecorder final {
public:
    /**
     * @comment 注册 L1 runtime diagnostics counter id。
     * @param channel 输入可写 diagnostics channel。
     * @return 显式操作状态。
     */
    DiagnosticsStatus RegisterCounters(BoundedDiagnosticsChannel *channel) const;

    /**
     * @comment 访问 disabled diagnostics channel，保持禁用语义。
     * @param channel 输入 disabled diagnostics channel。
     * @return 显式操作状态。
     */
    DiagnosticsStatus RegisterCounters(DisabledDiagnosticsChannel *channel) const;

    /**
     * @comment 记录 L1 runtime diagnostics counter 值。
     * @param channel 输入可写 diagnostics channel。
     * @param counters 输入 runtime counter 值。
     * @return 显式记录结果。
     */
    RuntimeDiagnosticsRecordResult RecordCounters(
        BoundedDiagnosticsChannel *channel,
        const RuntimeDiagnosticsCounters &counters) const;

    /**
     * @comment 访问 disabled diagnostics channel，保持 runtime 计数值不变。
     * @param channel 输入 disabled diagnostics channel。
     * @param counters 输入 runtime counter 值。
     * @return 显式记录结果。
     */
    RuntimeDiagnosticsRecordResult RecordCounters(
        DisabledDiagnosticsChannel *channel,
        const RuntimeDiagnosticsCounters &counters) const;

private:
    DiagnosticsStatus RegisterCounter(
        BoundedDiagnosticsChannel *channel,
        DiagnosticsCounterId counter_id) const;

    bool RecordCounter(
        BoundedDiagnosticsChannel *channel,
        DiagnosticsCounterId counter_id,
        std::uint64_t value,
        RuntimeDiagnosticsRecordResult *result) const;
};
}
