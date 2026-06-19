// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Src/RuntimeDiagnosticsCounterRecorder.cpp

#include "YuEngine/Diagnostics/RuntimeDiagnosticsCounterRecorder.h"

#include "YuEngine/Diagnostics/RuntimeDiagnosticsCounterIds.h"

namespace yuengine::diagnostics {
DiagnosticsStatus RuntimeDiagnosticsCounterRecorder::RegisterCounters(BoundedDiagnosticsChannel *channel) const {
    if (channel == nullptr) {
        return DiagnosticsStatus::InvalidArgument;
    }

    DiagnosticsStatus status = RegisterCounter(channel, RUNTIME_FRAME_COUNT_COUNTER_ID);
    if (status != DiagnosticsStatus::Success) {
        return status;
    }

    status = RegisterCounter(channel, RUNTIME_FRAME_TIME_COUNTER_ID);
    if (status != DiagnosticsStatus::Success) {
        return status;
    }

    status = RegisterCounter(channel, RUNTIME_OBJECT_COUNT_COUNTER_ID);
    if (status != DiagnosticsStatus::Success) {
        return status;
    }

    status = RegisterCounter(channel, RUNTIME_RESOURCE_COUNT_COUNTER_ID);
    if (status != DiagnosticsStatus::Success) {
        return status;
    }

    status = RegisterCounter(channel, RUNTIME_RENDER_COUNT_COUNTER_ID);
    if (status != DiagnosticsStatus::Success) {
        return status;
    }

    status = RegisterCounter(channel, RUNTIME_AUDIO_COUNT_COUNTER_ID);
    if (status != DiagnosticsStatus::Success) {
        return status;
    }

    status = RegisterCounter(channel, RUNTIME_INPUT_COUNT_COUNTER_ID);
    return status;
}

DiagnosticsStatus RuntimeDiagnosticsCounterRecorder::RegisterCounters(DisabledDiagnosticsChannel *channel) const {
    if (channel == nullptr) {
        return DiagnosticsStatus::InvalidArgument;
    }

    return channel->RegisterCounterId(RUNTIME_FRAME_COUNT_COUNTER_ID);
}

RuntimeDiagnosticsRecordResult RuntimeDiagnosticsCounterRecorder::RecordCounters(
    BoundedDiagnosticsChannel *channel,
    const RuntimeDiagnosticsCounters &counters) const {
    RuntimeDiagnosticsRecordResult result{};
    result.contract_counter_count = static_cast<std::uint32_t>(RUNTIME_DIAGNOSTICS_COUNTER_COUNT);

    if (channel == nullptr) {
        result.status = DiagnosticsStatus::InvalidArgument;
        return result;
    }

    if (!RecordCounter(channel, RUNTIME_FRAME_COUNT_COUNTER_ID, counters.frame_count, &result)) {
        return result;
    }

    if (!RecordCounter(channel, RUNTIME_FRAME_TIME_COUNTER_ID, counters.frame_time_nanoseconds, &result)) {
        return result;
    }

    if (!RecordCounter(channel, RUNTIME_OBJECT_COUNT_COUNTER_ID, counters.object_count, &result)) {
        return result;
    }

    if (!RecordCounter(channel, RUNTIME_RESOURCE_COUNT_COUNTER_ID, counters.resource_count, &result)) {
        return result;
    }

    if (!RecordCounter(channel, RUNTIME_RENDER_COUNT_COUNTER_ID, counters.render_submission_count, &result)) {
        return result;
    }

    if (!RecordCounter(channel, RUNTIME_AUDIO_COUNT_COUNTER_ID, counters.audio_submission_count, &result)) {
        return result;
    }

    RecordCounter(channel, RUNTIME_INPUT_COUNT_COUNTER_ID, counters.input_command_count, &result);
    return result;
}

RuntimeDiagnosticsRecordResult RuntimeDiagnosticsCounterRecorder::RecordCounters(
    DisabledDiagnosticsChannel *channel,
    const RuntimeDiagnosticsCounters &counters) const {
    RuntimeDiagnosticsRecordResult result{};
    result.contract_counter_count = static_cast<std::uint32_t>(RUNTIME_DIAGNOSTICS_COUNTER_COUNT);
    static_cast<void>(counters);

    if (channel == nullptr) {
        result.status = DiagnosticsStatus::InvalidArgument;
        return result;
    }

    result.status = channel->AddCounter(RUNTIME_FRAME_COUNT_COUNTER_ID, 0U);
    return result;
}

DiagnosticsStatus RuntimeDiagnosticsCounterRecorder::RegisterCounter(
    BoundedDiagnosticsChannel *channel,
    DiagnosticsCounterId counter_id) const {
    if (channel == nullptr) {
        return DiagnosticsStatus::InvalidArgument;
    }

    return channel->RegisterCounterId(counter_id);
}

bool RuntimeDiagnosticsCounterRecorder::RecordCounter(
    BoundedDiagnosticsChannel *channel,
    DiagnosticsCounterId counter_id,
    std::uint64_t value,
    RuntimeDiagnosticsRecordResult *result) const {
    if (result == nullptr) {
        return false;
    }

    if (channel == nullptr) {
        result->status = DiagnosticsStatus::InvalidArgument;
        result->failed_counter_id = counter_id;
        return false;
    }

    const DiagnosticsStatus status = channel->AddCounter(counter_id, value);
    if (status != DiagnosticsStatus::Success) {
        result->status = status;
        result->failed_counter_id = counter_id;
        return false;
    }

    result->status = DiagnosticsStatus::Success;
    ++result->recorded_counter_count;
    return true;
}
}
