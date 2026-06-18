// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DisabledDiagnosticsChannel.h

#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsCounterId.h"
#include "YuEngine/Diagnostics/DiagnosticsEventId.h"
#include "YuEngine/Diagnostics/DiagnosticsSnapshot.h"
#include "YuEngine/Diagnostics/DiagnosticsStatus.h"

namespace yuengine::diagnostics {
class DisabledDiagnosticsChannel final {
public:
    /**
     * @comment 注册 event id。
     * @param event_id 输入 event id。
     * @return 显式操作状态。
     */
    DiagnosticsStatus RegisterEventId(DiagnosticsEventId event_id);
    /**
     * @comment 注册 counter id。
     * @param counter_id 输入 counter id。
     * @return 显式操作状态。
     */
    DiagnosticsStatus RegisterCounterId(DiagnosticsCounterId counter_id);
    /**
     * @comment 记录 event。
     * @param event_id 输入 event id。
     * @param payload 输入 payload。
     * @return 显式操作状态。
     */
    DiagnosticsStatus RecordEvent(DiagnosticsEventId event_id, std::uint64_t payload);
    /**
     * @comment 递增 counter。
     * @param counter_id 输入 counter id。
     * @return 显式操作状态。
     */
    DiagnosticsStatus IncrementCounter(DiagnosticsCounterId counter_id);
    /**
     * @comment 增加 counter。
     * @param counter_id 输入 counter id。
     * @param delta 输入 delta。
     * @return 显式操作状态。
     */
    DiagnosticsStatus AddCounter(DiagnosticsCounterId counter_id, std::uint64_t delta);
    /**
     * @comment 关闭 component。
     * @return 显式操作状态。
     */
    DiagnosticsStatus Shutdown();
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    DiagnosticsSnapshot Snapshot() const;
};
}
