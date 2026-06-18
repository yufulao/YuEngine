// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/BoundedDiagnosticsChannel.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsChannelConfig.h"
#include "YuEngine/Diagnostics/DiagnosticsCounterId.h"
#include "YuEngine/Diagnostics/DiagnosticsCounterSnapshot.h"
#include "YuEngine/Diagnostics/DiagnosticsEvent.h"
#include "YuEngine/Diagnostics/DiagnosticsEventId.h"
#include "YuEngine/Diagnostics/DiagnosticsLimits.h"
#include "YuEngine/Diagnostics/DiagnosticsSnapshot.h"
#include "YuEngine/Diagnostics/DiagnosticsStatus.h"

namespace yuengine::diagnostics {
class BoundedDiagnosticsChannel final {
public:
    /**
     * @comment 构造 BoundedDiagnosticsChannel 实例。
     * @param config 输入 configuration。
     */
    explicit BoundedDiagnosticsChannel(DiagnosticsChannelConfig config);

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
    DiagnosticsSnapshot Snapshot();

private:
    DiagnosticsStatus ValidateConfig(DiagnosticsChannelConfig config) const;
    bool HasAcceptedEventId(DiagnosticsEventId event_id) const;
    bool HasAcceptedCounterId(DiagnosticsCounterId counter_id) const;
    std::size_t CounterIndex(DiagnosticsCounterId counter_id) const;

    DiagnosticsChannelConfig config_;
    DiagnosticsStatus configuration_status_;
    DiagnosticsSnapshot snapshot_;
    std::array<DiagnosticsEventId, MAX_DIAGNOSTICS_EVENT_IDS> accepted_event_ids_;
    std::array<DiagnosticsCounterId, MAX_DIAGNOSTICS_COUNTER_IDS> accepted_counter_ids_;
    std::size_t accepted_event_id_count_;
    std::size_t accepted_counter_id_count_;
};
}
