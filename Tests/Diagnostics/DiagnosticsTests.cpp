// 模块：Tests Diagnostics
// 文件：Tests/Diagnostics/DiagnosticsTests.cpp

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "YuEngine/Diagnostics/BoundedDiagnosticsChannel.h"
#include "YuEngine/Diagnostics/BoundedInMemoryLogSink.h"
#include "YuEngine/Diagnostics/DiagnosticsChannelConfig.h"
#include "YuEngine/Diagnostics/DiagnosticsCounterId.h"
#include "YuEngine/Diagnostics/DiagnosticsEventId.h"
#include "YuEngine/Diagnostics/DiagnosticsLimits.h"
#include "YuEngine/Diagnostics/DiagnosticsSnapshot.h"
#include "YuEngine/Diagnostics/DiagnosticsStatus.h"
#include "YuEngine/Diagnostics/DisabledDiagnosticsChannel.h"
#include "YuEngine/Diagnostics/DisabledLogSink.h"
#include "YuEngine/Diagnostics/LogLevel.h"
#include "YuEngine/Diagnostics/RuntimeDiagnosticsCounterIds.h"
#include "YuEngine/Diagnostics/RuntimeDiagnosticsCounterRecorder.h"
#include "YuEngine/Diagnostics/RuntimeDiagnosticsCounters.h"
#include "YuEngine/Diagnostics/RuntimeDiagnosticsRecordResult.h"
#include "YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHook.h"
#include "YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHookProposal.h"
#include "YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHookResult.h"
#include "YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHookStatus.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Platform/FixedFrameClock.h"
#include "YuEngine/Platform/HeadlessHost.h"
#include "YuEngine/Platform/HostStatus.h"
#include "YuEngine/Platform/IHostRuntime.h"

using BoundedDiagnosticsChannel = yuengine::diagnostics::BoundedDiagnosticsChannel;
using BoundedInMemoryLogSink = yuengine::diagnostics::BoundedInMemoryLogSink;
using yuengine::diagnostics::DiagnosticsChannelConfig;
using yuengine::diagnostics::DiagnosticsCounterId;
using yuengine::diagnostics::DiagnosticsEventId;
using yuengine::diagnostics::DiagnosticsSnapshot;
using yuengine::diagnostics::DiagnosticsStatus;
using DisabledDiagnosticsChannel = yuengine::diagnostics::DisabledDiagnosticsChannel;
using DisabledLogSink = yuengine::diagnostics::DisabledLogSink;
using yuengine::diagnostics::LogLevel;
using yuengine::diagnostics::RuntimeDiagnosticsCounterRecorder;
using yuengine::diagnostics::RuntimeDiagnosticsCounters;
using yuengine::diagnostics::RuntimeDiagnosticsRecordResult;
using yuengine::diagnostics::RuntimeDiagnosticsOverlayHook;
using yuengine::diagnostics::RuntimeDiagnosticsOverlayHookProposal;
using yuengine::diagnostics::RuntimeDiagnosticsOverlayHookResult;
using yuengine::diagnostics::RuntimeDiagnosticsOverlayHookStatus;
using FixedFrameClock = yuengine::platform::FixedFrameClock;
using HeadlessHost = yuengine::platform::HeadlessHost;
using yuengine::platform::HeadlessHostConfig;
using yuengine::platform::HostError;
using yuengine::platform::HostStatus;
using IHostRuntime = yuengine::platform::IHostRuntime;
using yuengine::memory::MemoryAccountingStatus;
using yuengine::diagnostics::MAX_DIAGNOSTICS_COUNTERS;
using yuengine::diagnostics::RUNTIME_AUDIO_COUNT_COUNTER_ID;
using yuengine::diagnostics::RUNTIME_DIAGNOSTICS_COUNTER_COUNT;
using yuengine::diagnostics::RUNTIME_FRAME_COUNT_COUNTER_ID;
using yuengine::diagnostics::RUNTIME_FRAME_TIME_COUNTER_ID;
using yuengine::diagnostics::RUNTIME_INPUT_COUNT_COUNTER_ID;
using yuengine::diagnostics::RUNTIME_OBJECT_COUNT_COUNTER_ID;
using yuengine::diagnostics::RUNTIME_RENDER_COUNT_COUNTER_ID;
using yuengine::diagnostics::RUNTIME_RESOURCE_COUNT_COUNTER_ID;

namespace {
constexpr const char* TEST_DISABLED_LOGGING = "Logging_DisabledSink_DoesNotChangeBehavior";
constexpr const char* TEST_MODULE_FILTER = "Logging_ModuleFilterAndDynamicSwitch";
constexpr const char* TEST_DISABLED_CHANNEL = "Diagnostics_DisabledChannel_DoesNotChangeBehavior";
constexpr const char* TEST_RECORDS_EVENTS_COUNTERS = "Diagnostics_BoundedChannel_RecordsEventsAndCounters";
constexpr const char* TEST_DROPS_WHEN_FULL = "Diagnostics_BoundedChannel_DropsWhenFull";
constexpr const char* TEST_SNAPSHOT = "Diagnostics_ChannelSnapshot_ReportsAcceptedDroppedAndCounters";
constexpr const char* TEST_STOPPED = "Diagnostics_ChannelStopped_DoesNotMutateAfterShutdown";
constexpr const char* TEST_UNKNOWN_IDS = "Diagnostics_ChannelRejectsUnknownIds_WhenValidationEnabled";
constexpr const char* TEST_COUNTER_OVERFLOW = "Diagnostics_CounterOverflow_ReturnsExplicitStatusAndDoesNotMutate";
constexpr const char* TEST_NO_REPORT_DEPENDENCY = "Diagnostics_NoReportDependency_ForRuntimeResults";
constexpr const char* TEST_MEMORY_SIGNAL = "Diagnostics_NoHiddenAllocation_UsesYuMemorySignal";
constexpr const char* TEST_RUNTIME_COUNTERS = "Diagnostics_RuntimeCounters_RegisterAndRecordBoundedValues";
constexpr const char* TEST_RUNTIME_DISABLED = "Diagnostics_RuntimeCounters_DisabledChannelDoesNotChangeRuntimeValues";
constexpr const char* TEST_RUNTIME_NULL_CHANNEL = "Diagnostics_RuntimeCounters_RejectNullChannelWithoutMutation";
constexpr const char* TEST_RUNTIME_PLAIN_VALUES = "Diagnostics_RuntimeCounters_ValueTypesArePlainValues";
constexpr const char *TEST_OVERLAY_OPTIONAL = "Diagnostics_OverlayHookProposalStaysOptionalToolingPlane";
constexpr const char *TEST_OVERLAY_REJECTS_RUNTIME_DEPENDENCY =
    "Diagnostics_OverlayHookRejectsRuntimeDependency";
constexpr const char *TEST_OVERLAY_DISABLED = "Diagnostics_OverlayHookDisabledDoesNotChangeRuntimeValues";
constexpr const char* LOG_MODULE_PLATFORM = "Platform";
constexpr const char* LOG_MODULE_AUDIO = "Audio";
constexpr const char* LOG_MESSAGE_FILTERED = "filtered event";
constexpr const char* LOG_MESSAGE_ALLOWED = "allowed event";
constexpr const char* LOG_MESSAGE_DISABLED = "disabled event";
constexpr const char* LOG_MESSAGE_REENABLED = "reenabled event";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint64_t FIRST_TICK_NANOSECONDS = 2000U;
constexpr std::uint64_t STEP_NANOSECONDS = 32U;
constexpr std::uint32_t TICK_COUNT = 2U;
constexpr std::size_t LOG_CAPACITY = 8U;
constexpr std::size_t EVENT_CAPACITY = 2U;
constexpr std::size_t COUNTER_CAPACITY = 2U;
constexpr std::size_t ID_CAPACITY = 2U;
constexpr std::uint32_t EVENT_ID = 1U;
constexpr std::uint32_t SECOND_EVENT_ID = 2U;
constexpr std::uint32_t UNKNOWN_EVENT_ID = 99U;
constexpr std::uint32_t COUNTER_ID = 1U;
constexpr std::uint32_t SECOND_COUNTER_ID = 2U;
constexpr std::uint32_t UNKNOWN_COUNTER_ID = 99U;
constexpr std::uint64_t EVENT_PAYLOAD = 42U;
constexpr std::uint64_t COUNTER_DELTA = 5U;
constexpr std::uint64_t RUNTIME_FRAME_COUNT = 3U;
constexpr std::uint64_t RUNTIME_FRAME_TIME = 16666666U;
constexpr std::uint64_t RUNTIME_OBJECT_COUNT = 11U;
constexpr std::uint64_t RUNTIME_RESOURCE_COUNT = 7U;
constexpr std::uint64_t RUNTIME_RENDER_COUNT = 5U;
constexpr std::uint64_t RUNTIME_AUDIO_COUNT = 2U;
constexpr std::uint64_t RUNTIME_INPUT_COUNT = 4U;
static_assert(
    RUNTIME_DIAGNOSTICS_COUNTER_COUNT <= MAX_DIAGNOSTICS_COUNTERS,
    "runtime diagnostics counter count exceeds bounded diagnostics capacity");
using TestFunction = int (*)();

class TraceRuntime final : public IHostRuntime {
public:
    HostError Start(std::vector<std::string>& lifecycle_trace) override {
        lifecycle_trace.push_back("runtime.start");
        return HostError::Success();
    }

    HostError Tick(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) override {
        static_cast<void>(frame_index);
        static_cast<void>(tick_time_nanoseconds);
        lifecycle_trace.push_back("runtime.tick");
        return HostError::Success();
    }

    HostError Shutdown(std::vector<std::string>& lifecycle_trace) override {
        lifecycle_trace.push_back("runtime.shutdown");
        return HostError::Success();
    }
};

struct RuntimeProbeResult {
    HostStatus runtime_status = HostStatus::Success;
    RuntimeDiagnosticsCounters counters{};
    RuntimeDiagnosticsRecordResult diagnostics_result{};
    DiagnosticsSnapshot diagnostics_snapshot{};
};

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool DiagnosticsCapacityEntryIsClear(const DiagnosticsSnapshot &snapshot) {
    if (snapshot.failed_event_id.value != 0U) {
        return false;
    }

    if (snapshot.failed_counter_id.value != 0U) {
        return false;
    }

    if (snapshot.failed_event_id_capacity != 0U) {
        return false;
    }

    if (snapshot.failed_event_id_count != 0U) {
        return false;
    }

    if (snapshot.failed_counter_id_capacity != 0U) {
        return false;
    }

    if (snapshot.failed_counter_id_count != 0U) {
        return false;
    }

    if (snapshot.failed_counter_slot_capacity != 0U) {
        return false;
    }

    if (snapshot.failed_counter_slot_count != 0U) {
        return false;
    }

    if (snapshot.failed_event_record_capacity != 0U) {
        return false;
    }

    if (snapshot.failed_event_record_count != 0U) {
        return false;
    }

    return true;
}

DiagnosticsChannelConfig TestChannelConfig() {
    return DiagnosticsChannelConfig{EVENT_CAPACITY, COUNTER_CAPACITY, ID_CAPACITY, ID_CAPACITY, true};
}

DiagnosticsChannelConfig RuntimeChannelConfig() {
    return DiagnosticsChannelConfig{
        EVENT_CAPACITY,
        RUNTIME_DIAGNOSTICS_COUNTER_COUNT,
        ID_CAPACITY,
        RUNTIME_DIAGNOSTICS_COUNTER_COUNT,
        true};
}

BoundedDiagnosticsChannel CreateRegisteredChannel() {
    BoundedDiagnosticsChannel channel(TestChannelConfig());
    const auto event_status = channel.RegisterEventId(DiagnosticsEventId{EVENT_ID});
    if (event_status != DiagnosticsStatus::Success) {
        return channel;
    }

    channel.RegisterEventId(DiagnosticsEventId{SECOND_EVENT_ID});
    channel.RegisterCounterId(DiagnosticsCounterId{COUNTER_ID});
    channel.RegisterCounterId(DiagnosticsCounterId{SECOND_COUNTER_ID});
    return channel;
}

RuntimeDiagnosticsCounters RuntimeCounterSample() {
    RuntimeDiagnosticsCounters counters{};
    counters.frame_count = RUNTIME_FRAME_COUNT;
    counters.frame_time_nanoseconds = RUNTIME_FRAME_TIME;
    counters.object_count = RUNTIME_OBJECT_COUNT;
    counters.resource_count = RUNTIME_RESOURCE_COUNT;
    counters.render_submission_count = RUNTIME_RENDER_COUNT;
    counters.audio_submission_count = RUNTIME_AUDIO_COUNT;
    counters.input_command_count = RUNTIME_INPUT_COUNT;
    return counters;
}

bool RuntimeCountersEqual(const RuntimeDiagnosticsCounters &left, const RuntimeDiagnosticsCounters &right) {
    if (left.frame_count != right.frame_count) {
        return false;
    }

    if (left.frame_time_nanoseconds != right.frame_time_nanoseconds) {
        return false;
    }

    if (left.object_count != right.object_count) {
        return false;
    }

    if (left.resource_count != right.resource_count) {
        return false;
    }

    if (left.render_submission_count != right.render_submission_count) {
        return false;
    }

    if (left.audio_submission_count != right.audio_submission_count) {
        return false;
    }

    return left.input_command_count == right.input_command_count;
}

RuntimeProbeResult RunRuntimeProbe(bool diagnostics_enabled) {
    RuntimeProbeResult result{};
    result.runtime_status = HostStatus::Success;
    result.counters = RuntimeCounterSample();

    RuntimeDiagnosticsCounterRecorder recorder;
    if (diagnostics_enabled) {
        BoundedDiagnosticsChannel channel(RuntimeChannelConfig());
        recorder.RegisterCounters(&channel);
        result.diagnostics_result = recorder.RecordCounters(&channel, result.counters);
        result.diagnostics_snapshot = channel.Snapshot();
        return result;
    }

    DisabledDiagnosticsChannel channel;
    recorder.RegisterCounters(&channel);
    result.diagnostics_result = recorder.RecordCounters(&channel, result.counters);
    result.diagnostics_snapshot = channel.Snapshot();
    return result;
}

int LoggingDisabledSinkDoesNotChangeBehavior() {
    BoundedInMemoryLogSink recording_log_sink(LOG_CAPACITY);
    FixedFrameClock recording_clock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime recording_runtime;
    HeadlessHost recording_host(recording_clock, recording_log_sink);

    const HeadlessHostConfig config{TICK_COUNT};
    const auto recording_result = recording_host.Run(recording_runtime, config);
    if (recording_result.status != HostStatus::Success) {
        return Fail("recording host run failed");
    }

    DisabledLogSink disabled_log_sink;
    FixedFrameClock disabled_clock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime disabled_runtime;
    HeadlessHost disabled_host(disabled_clock, disabled_log_sink);

    const auto disabled_result = disabled_host.Run(disabled_runtime, config);
    if (disabled_result.status != HostStatus::Success) {
        return Fail("disabled host run failed");
    }

    if (recording_result.lifecycle_trace != disabled_result.lifecycle_trace) {
        return Fail("disabled logging changed lifecycle behavior");
    }

    if (recording_result.tick_times_nanoseconds != disabled_result.tick_times_nanoseconds) {
        return Fail("disabled logging changed timer behavior");
    }

    if (recording_log_sink.Events().empty()) {
        return Fail("recording logging did not observe host events");
    }

    if (recording_log_sink.Events()[0U].module_name != LOG_MODULE_PLATFORM) {
        return Fail("recording logging did not store host module");
    }

    if (recording_log_sink.DroppedCount() != 0U) {
        return Fail("bounded in-memory sink dropped unexpected events");
    }

    if (disabled_log_sink.IsEnabled()) {
        return Fail("disabled sink reported enabled");
    }

    return 0;
}

int LoggingModuleFilterAndDynamicSwitch() {
    BoundedInMemoryLogSink log_sink(LOG_CAPACITY);
    if (!log_sink.IsEnabled()) {
        return Fail("log sink was not initially enabled");
    }

    if (!log_sink.IsModuleEnabled(LOG_MODULE_PLATFORM)) {
        return Fail("platform module was not initially enabled");
    }

    const bool disable_platform = log_sink.SetModuleEnabled(LOG_MODULE_PLATFORM, false);
    if (!disable_platform) {
        return Fail("failed to disable platform module");
    }

    if (log_sink.IsModuleEnabled(LOG_MODULE_PLATFORM)) {
        return Fail("platform module remained enabled");
    }

    log_sink.Write(LOG_MODULE_PLATFORM, LogLevel::Info, LOG_MESSAGE_FILTERED);
    if (!log_sink.Events().empty()) {
        return Fail("disabled module recorded event");
    }

    log_sink.Write(LOG_MODULE_AUDIO, LogLevel::Warning, LOG_MESSAGE_ALLOWED);
    if (log_sink.Events().size() != 1U) {
        return Fail("enabled module did not record exactly one event");
    }

    if (log_sink.Events()[0U].module_name != LOG_MODULE_AUDIO) {
        return Fail("log event module was not recorded");
    }

    if (log_sink.Events()[0U].message != LOG_MESSAGE_ALLOWED) {
        return Fail("log event message was not recorded");
    }

    if (log_sink.Events()[0U].level != LogLevel::Warning) {
        return Fail("log event level was not recorded");
    }

    log_sink.SetEnabled(false);
    if (log_sink.IsModuleEnabled(LOG_MODULE_AUDIO)) {
        return Fail("global logging switch did not disable module");
    }

    log_sink.Write(LOG_MODULE_AUDIO, LogLevel::Error, LOG_MESSAGE_DISABLED);
    if (log_sink.Events().size() != 1U) {
        return Fail("global disabled logging recorded event");
    }

    log_sink.SetEnabled(true);
    const bool enable_platform = log_sink.SetModuleEnabled(LOG_MODULE_PLATFORM, true);
    if (!enable_platform) {
        return Fail("failed to re-enable platform module");
    }

    if (!log_sink.IsModuleEnabled(LOG_MODULE_PLATFORM)) {
        return Fail("platform module was not re-enabled");
    }

    log_sink.Write(LOG_MODULE_PLATFORM, LogLevel::Info, LOG_MESSAGE_REENABLED);
    if (log_sink.Events().size() != 2U) {
        return Fail("re-enabled module did not record event");
    }

    if (log_sink.Events()[1U].module_name != LOG_MODULE_PLATFORM) {
        return Fail("re-enabled event module was wrong");
    }

    return 0;
}

int DiagnosticsDisabledChannelDoesNotChangeBehavior() {
    DisabledDiagnosticsChannel disabled_channel;
    const auto event_status = disabled_channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    if (event_status != DiagnosticsStatus::Disabled) {
        return Fail("disabled diagnostics channel did not return disabled status");
    }

    const auto counter_status = disabled_channel.IncrementCounter(DiagnosticsCounterId{COUNTER_ID});
    if (counter_status != DiagnosticsStatus::Disabled) {
        return Fail("disabled diagnostics counter did not return disabled status");
    }

    const DiagnosticsSnapshot snapshot = disabled_channel.Snapshot();
    if (snapshot.enabled) {
        return Fail("disabled diagnostics snapshot reported enabled");
    }

    if (snapshot.accepted_event_count != 0U) {
        return Fail("disabled diagnostics channel accepted an event");
    }

    if (snapshot.successful_counter_update_count != 0U) {
        return Fail("disabled diagnostics channel accepted a counter update");
    }

    if (snapshot.last_status != DiagnosticsStatus::Disabled) {
        return Fail("disabled diagnostics snapshot status was not disabled");
    }

    HostStatus explicit_runtime_status = HostStatus::Success;
    if (explicit_runtime_status != HostStatus::Success) {
        return Fail("disabled diagnostics changed explicit runtime status");
    }

    return 0;
}

int DiagnosticsBoundedChannelRecordsEventsAndCounters() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    const auto event_capacity_status = channel.RegisterEventId(DiagnosticsEventId{UNKNOWN_EVENT_ID});
    if (event_capacity_status != DiagnosticsStatus::CapacityExceeded) {
        return Fail("bounded diagnostics channel did not report event id capacity");
    }

    const DiagnosticsSnapshot event_capacity_snapshot = channel.Snapshot();
    if (event_capacity_snapshot.last_status != DiagnosticsStatus::CapacityExceeded) {
        return Fail("event id capacity status was not recorded");
    }

    if (event_capacity_snapshot.required_event_id_count != ID_CAPACITY + 1U) {
        return Fail("event id capacity did not expose required count");
    }

    if (event_capacity_snapshot.failed_event_id.value != UNKNOWN_EVENT_ID) {
        return Fail("event id capacity did not expose failed event id");
    }

    if (event_capacity_snapshot.failed_event_id_capacity != ID_CAPACITY) {
        return Fail("event id capacity did not expose capacity");
    }

    if (event_capacity_snapshot.failed_event_id_count != ID_CAPACITY) {
        return Fail("event id capacity did not expose current count");
    }

    if (event_capacity_snapshot.failed_counter_id.value != 0U) {
        return Fail("event id capacity left stale failed counter id");
    }

    if (event_capacity_snapshot.event_count != 0U) {
        return Fail("event id capacity mutated event records");
    }

    const auto counter_capacity_status = channel.RegisterCounterId(DiagnosticsCounterId{UNKNOWN_COUNTER_ID});
    if (counter_capacity_status != DiagnosticsStatus::CapacityExceeded) {
        return Fail("bounded diagnostics channel did not report counter id capacity");
    }

    const DiagnosticsSnapshot counter_capacity_snapshot = channel.Snapshot();
    if (counter_capacity_snapshot.last_status != DiagnosticsStatus::CapacityExceeded) {
        return Fail("counter id capacity status was not recorded");
    }

    if (counter_capacity_snapshot.required_counter_id_count != ID_CAPACITY + 1U) {
        return Fail("counter id capacity did not expose required count");
    }

    if (counter_capacity_snapshot.failed_counter_id.value != UNKNOWN_COUNTER_ID) {
        return Fail("counter id capacity did not expose failed counter id");
    }

    if (counter_capacity_snapshot.failed_counter_id_capacity != ID_CAPACITY) {
        return Fail("counter id capacity did not expose capacity");
    }

    if (counter_capacity_snapshot.failed_counter_id_count != ID_CAPACITY) {
        return Fail("counter id capacity did not expose current count");
    }

    if (counter_capacity_snapshot.failed_event_id.value != 0U) {
        return Fail("counter id capacity left stale failed event id");
    }

    if (counter_capacity_snapshot.counter_count != COUNTER_CAPACITY) {
        return Fail("counter id capacity mutated counter slots");
    }

    DiagnosticsChannelConfig counter_slot_config = TestChannelConfig();
    counter_slot_config.counter_capacity = 1U;
    BoundedDiagnosticsChannel counter_slot_channel(counter_slot_config);
    counter_slot_channel.RegisterCounterId(DiagnosticsCounterId{COUNTER_ID});
    const auto counter_slot_status =
        counter_slot_channel.RegisterCounterId(DiagnosticsCounterId{SECOND_COUNTER_ID});
    if (counter_slot_status != DiagnosticsStatus::CapacityExceeded) {
        return Fail("bounded diagnostics channel did not report counter slot capacity");
    }

    const DiagnosticsSnapshot counter_slot_snapshot = counter_slot_channel.Snapshot();
    if (counter_slot_snapshot.required_counter_slot_count != 2U) {
        return Fail("counter slot capacity did not expose required count");
    }

    if (counter_slot_snapshot.failed_counter_id.value != SECOND_COUNTER_ID) {
        return Fail("counter slot capacity did not expose failed counter id");
    }

    if (counter_slot_snapshot.failed_counter_slot_capacity != 1U) {
        return Fail("counter slot capacity did not expose capacity");
    }

    if (counter_slot_snapshot.failed_counter_slot_count != 1U) {
        return Fail("counter slot capacity did not expose current count");
    }

    if (counter_slot_snapshot.counter_count != 1U) {
        return Fail("counter slot capacity mutated counter slot count");
    }

    const auto event_status = channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    if (event_status != DiagnosticsStatus::Success) {
        return Fail("bounded diagnostics channel rejected valid event");
    }

    const auto counter_status = channel.AddCounter(DiagnosticsCounterId{COUNTER_ID}, COUNTER_DELTA);
    if (counter_status != DiagnosticsStatus::Success) {
        return Fail("bounded diagnostics channel rejected valid counter update");
    }

    const DiagnosticsSnapshot snapshot = channel.Snapshot();
    if (snapshot.event_count != 1U) {
        return Fail("bounded diagnostics channel event count was wrong");
    }

    if (snapshot.events[0U].payload != EVENT_PAYLOAD) {
        return Fail("bounded diagnostics channel event payload was wrong");
    }

    if (snapshot.counters[0U].value != COUNTER_DELTA) {
        return Fail("bounded diagnostics channel counter value was wrong");
    }

    if (snapshot.successful_counter_update_count != 1U) {
        return Fail("bounded diagnostics channel counter update count was wrong");
    }

    if (snapshot.last_status != DiagnosticsStatus::Success) {
        return Fail("bounded diagnostics channel success status was not recorded");
    }

    if (!DiagnosticsCapacityEntryIsClear(snapshot)) {
        return Fail("bounded diagnostics channel success left stale capacity entry");
    }

    return 0;
}

int DiagnosticsBoundedChannelDropsWhenFull() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    const auto first_status = channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    const auto second_status = channel.RecordEvent(DiagnosticsEventId{SECOND_EVENT_ID}, EVENT_PAYLOAD);
    const auto third_status = channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    if (first_status != DiagnosticsStatus::Success) {
        return Fail("first event was not accepted");
    }

    if (second_status != DiagnosticsStatus::Success) {
        return Fail("second event was not accepted");
    }

    if (third_status != DiagnosticsStatus::Dropped) {
        return Fail("full diagnostics channel did not return dropped status");
    }

    const DiagnosticsSnapshot snapshot = channel.Snapshot();
    if (snapshot.event_count != EVENT_CAPACITY) {
        return Fail("full diagnostics channel changed event storage size");
    }

    if (snapshot.dropped_event_count != 1U) {
        return Fail("full diagnostics channel did not count dropped event");
    }

    if (snapshot.required_event_record_count != EVENT_CAPACITY + 1U) {
        return Fail("full diagnostics channel did not expose required event record count");
    }

    if (snapshot.failed_event_id.value != EVENT_ID) {
        return Fail("full diagnostics channel did not expose failed event id");
    }

    if (snapshot.failed_event_record_capacity != EVENT_CAPACITY) {
        return Fail("full diagnostics channel did not expose event record capacity");
    }

    if (snapshot.failed_event_record_count != EVENT_CAPACITY) {
        return Fail("full diagnostics channel did not expose event record count");
    }

    if (snapshot.last_status != DiagnosticsStatus::Dropped) {
        return Fail("full diagnostics channel did not record dropped status");
    }

    if (channel.RecordEvent(DiagnosticsEventId{UNKNOWN_EVENT_ID}, EVENT_PAYLOAD) != DiagnosticsStatus::UnknownEventId) {
        return Fail("unknown event did not clear stale drop identity");
    }

    const DiagnosticsSnapshot invalid_snapshot = channel.Snapshot();
    if (!DiagnosticsCapacityEntryIsClear(invalid_snapshot)) {
        return Fail("unknown event left stale capacity entry");
    }

    if (invalid_snapshot.event_count != EVENT_CAPACITY) {
        return Fail("unknown event mutated full diagnostics storage");
    }

    return 0;
}

int DiagnosticsChannelSnapshotReportsAcceptedDroppedAndCounters() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    channel.RecordEvent(DiagnosticsEventId{SECOND_EVENT_ID}, EVENT_PAYLOAD);
    channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    channel.IncrementCounter(DiagnosticsCounterId{COUNTER_ID});
    channel.AddCounter(DiagnosticsCounterId{COUNTER_ID}, COUNTER_DELTA);

    const DiagnosticsSnapshot snapshot = channel.Snapshot();
    if (snapshot.accepted_event_count != 2U) {
        return Fail("snapshot accepted event count was wrong");
    }

    if (snapshot.dropped_event_count != 1U) {
        return Fail("snapshot dropped event count was wrong");
    }

    if (snapshot.counters[0U].value != COUNTER_DELTA + 1U) {
        return Fail("snapshot counter value was wrong");
    }

    if (snapshot.successful_counter_update_count != 2U) {
        return Fail("snapshot counter update count was wrong");
    }

    if (snapshot.snapshot_query_count != 1U) {
        return Fail("snapshot query count was wrong");
    }

    if (snapshot.last_status != DiagnosticsStatus::Success) {
        return Fail("snapshot last status was not success after counter update");
    }

    return 0;
}

int DiagnosticsChannelStoppedDoesNotMutateAfterShutdown() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    channel.IncrementCounter(DiagnosticsCounterId{COUNTER_ID});
    const auto shutdown_status = channel.Shutdown();
    if (shutdown_status != DiagnosticsStatus::Success) {
        return Fail("diagnostics channel shutdown failed");
    }

    const DiagnosticsSnapshot before_record = channel.Snapshot();
    if (before_record.last_status != DiagnosticsStatus::Success) {
        return Fail("shutdown success status was not recorded");
    }

    const auto event_status = channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    const auto counter_status = channel.IncrementCounter(DiagnosticsCounterId{COUNTER_ID});
    const DiagnosticsSnapshot after_record = channel.Snapshot();
    if (event_status != DiagnosticsStatus::Stopped) {
        return Fail("stopped diagnostics channel did not reject event");
    }

    if (counter_status != DiagnosticsStatus::Stopped) {
        return Fail("stopped diagnostics channel did not reject counter update");
    }

    if (after_record.accepted_event_count != before_record.accepted_event_count) {
        return Fail("stopped diagnostics channel mutated accepted event count");
    }

    if (after_record.successful_counter_update_count != before_record.successful_counter_update_count) {
        return Fail("stopped diagnostics channel mutated counter update count");
    }

    if (after_record.counters[0U].value != before_record.counters[0U].value) {
        return Fail("stopped diagnostics channel mutated counter value");
    }

    if (after_record.last_status != DiagnosticsStatus::Stopped) {
        return Fail("stopped diagnostics channel did not record stopped status");
    }

    return 0;
}

int DiagnosticsChannelRejectsUnknownIdsWhenValidationEnabled() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    const auto event_status = channel.RecordEvent(DiagnosticsEventId{UNKNOWN_EVENT_ID}, EVENT_PAYLOAD);
    const auto counter_status = channel.IncrementCounter(DiagnosticsCounterId{UNKNOWN_COUNTER_ID});
    if (event_status != DiagnosticsStatus::UnknownEventId) {
        return Fail("unknown diagnostics event id was not rejected");
    }

    if (counter_status != DiagnosticsStatus::UnknownCounterId) {
        return Fail("unknown diagnostics counter id was not rejected");
    }

    const DiagnosticsSnapshot snapshot = channel.Snapshot();
    if (snapshot.accepted_event_count != 0U) {
        return Fail("unknown diagnostics event mutated accepted count");
    }

    if (snapshot.successful_counter_update_count != 0U) {
        return Fail("unknown diagnostics counter mutated update count");
    }

    if (snapshot.last_status != DiagnosticsStatus::UnknownCounterId) {
        return Fail("unknown diagnostics counter status was not recorded");
    }

    return 0;
}

int DiagnosticsCounterOverflowReturnsExplicitStatusAndDoesNotMutate() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    const std::uint64_t max_value = std::numeric_limits<std::uint64_t>::max();
    const auto first_status = channel.AddCounter(DiagnosticsCounterId{COUNTER_ID}, max_value);
    const DiagnosticsSnapshot before_overflow = channel.Snapshot();
    const auto overflow_status = channel.AddCounter(DiagnosticsCounterId{COUNTER_ID}, 1U);
    const DiagnosticsSnapshot after_overflow = channel.Snapshot();
    if (first_status != DiagnosticsStatus::Success) {
        return Fail("counter max setup update failed");
    }

    if (overflow_status != DiagnosticsStatus::CounterOverflow) {
        return Fail("counter overflow did not return explicit status");
    }

    if (after_overflow.counters[0U].value != before_overflow.counters[0U].value) {
        return Fail("counter overflow mutated counter value");
    }

    if (after_overflow.successful_counter_update_count != before_overflow.successful_counter_update_count) {
        return Fail("counter overflow mutated successful update count");
    }

    if (after_overflow.last_status != DiagnosticsStatus::CounterOverflow) {
        return Fail("counter overflow status was not recorded");
    }

    return 0;
}

int DiagnosticsNoReportDependencyForRuntimeResults() {
    DisabledLogSink log_sink;
    FixedFrameClock frame_clock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime runtime;
    HeadlessHost host(frame_clock, log_sink);
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();

    const HeadlessHostConfig config{TICK_COUNT};
    const auto result = host.Run(runtime, config);
    const auto event_status = channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    if (result.status != HostStatus::Success) {
        return Fail("host result depended on diagnostics output");
    }

    if (event_status != DiagnosticsStatus::Success) {
        return Fail("diagnostics event fixture was not accepted");
    }

    if (result.lifecycle_trace.empty()) {
        return Fail("runtime result was not explicit without reports");
    }

    return 0;
}

int DiagnosticsNoHiddenAllocationUsesYuMemorySignal() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    const DiagnosticsSnapshot snapshot = channel.Snapshot();
    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("diagnostics channel did not expose YuMemory explicit-tracking status");
    }

    if (snapshot.enabled != true) {
        return Fail("diagnostics channel snapshot did not report enabled state");
    }

    if (snapshot.last_status != DiagnosticsStatus::Success) {
        return Fail("diagnostics channel snapshot did not report success status");
    }

    return 0;
}

int DiagnosticsRuntimeCountersRegisterAndRecordBoundedValues() {
    RuntimeDiagnosticsCounterRecorder recorder;
    BoundedDiagnosticsChannel channel(RuntimeChannelConfig());
    const DiagnosticsStatus register_status = recorder.RegisterCounters(&channel);
    if (register_status != DiagnosticsStatus::Success) {
        return Fail("runtime diagnostics counters did not register");
    }

    const RuntimeDiagnosticsCounters counters = RuntimeCounterSample();
    const RuntimeDiagnosticsRecordResult record_result = recorder.RecordCounters(&channel, counters);
    if (record_result.status != DiagnosticsStatus::Success) {
        return Fail("runtime diagnostics counters did not record");
    }

    const std::uint32_t expected_counter_count = static_cast<std::uint32_t>(RUNTIME_DIAGNOSTICS_COUNTER_COUNT);
    if (record_result.contract_counter_count != expected_counter_count) {
        return Fail("runtime diagnostics contract counter count was wrong");
    }

    if (record_result.recorded_counter_count != expected_counter_count) {
        return Fail("runtime diagnostics recorded counter count was wrong");
    }

    const DiagnosticsSnapshot snapshot = channel.Snapshot();
    if (snapshot.counter_count != RUNTIME_DIAGNOSTICS_COUNTER_COUNT) {
        return Fail("runtime diagnostics snapshot counter count was wrong");
    }

    if (snapshot.successful_counter_update_count != RUNTIME_DIAGNOSTICS_COUNTER_COUNT) {
        return Fail("runtime diagnostics update count was wrong");
    }

    if (snapshot.last_status != DiagnosticsStatus::Success) {
        return Fail("runtime diagnostics update status was not recorded");
    }

    if (snapshot.counters[0U].id.value != RUNTIME_FRAME_COUNT_COUNTER_ID.value) {
        return Fail("runtime frame counter id was wrong");
    }

    if (snapshot.counters[0U].value != counters.frame_count) {
        return Fail("runtime frame counter value was wrong");
    }

    if (snapshot.counters[1U].id.value != RUNTIME_FRAME_TIME_COUNTER_ID.value) {
        return Fail("runtime frame time counter id was wrong");
    }

    if (snapshot.counters[1U].value != counters.frame_time_nanoseconds) {
        return Fail("runtime frame time counter value was wrong");
    }

    if (snapshot.counters[2U].id.value != RUNTIME_OBJECT_COUNT_COUNTER_ID.value) {
        return Fail("runtime object counter id was wrong");
    }

    if (snapshot.counters[2U].value != counters.object_count) {
        return Fail("runtime object counter value was wrong");
    }

    if (snapshot.counters[3U].id.value != RUNTIME_RESOURCE_COUNT_COUNTER_ID.value) {
        return Fail("runtime resource counter id was wrong");
    }

    if (snapshot.counters[3U].value != counters.resource_count) {
        return Fail("runtime resource counter value was wrong");
    }

    if (snapshot.counters[4U].id.value != RUNTIME_RENDER_COUNT_COUNTER_ID.value) {
        return Fail("runtime render counter id was wrong");
    }

    if (snapshot.counters[4U].value != counters.render_submission_count) {
        return Fail("runtime render counter value was wrong");
    }

    if (snapshot.counters[5U].id.value != RUNTIME_AUDIO_COUNT_COUNTER_ID.value) {
        return Fail("runtime audio counter id was wrong");
    }

    if (snapshot.counters[5U].value != counters.audio_submission_count) {
        return Fail("runtime audio counter value was wrong");
    }

    if (snapshot.counters[6U].id.value != RUNTIME_INPUT_COUNT_COUNTER_ID.value) {
        return Fail("runtime input counter id was wrong");
    }

    if (snapshot.counters[6U].value != counters.input_command_count) {
        return Fail("runtime input counter value was wrong");
    }

    return 0;
}

int DiagnosticsRuntimeCountersDisabledChannelDoesNotChangeRuntimeValues() {
    const RuntimeProbeResult enabled_result = RunRuntimeProbe(true);
    const RuntimeProbeResult disabled_result = RunRuntimeProbe(false);

    if (enabled_result.runtime_status != disabled_result.runtime_status) {
        return Fail("disabled runtime diagnostics changed runtime status");
    }

    if (!RuntimeCountersEqual(enabled_result.counters, disabled_result.counters)) {
        return Fail("disabled runtime diagnostics changed runtime counters");
    }

    if (enabled_result.diagnostics_result.status != DiagnosticsStatus::Success) {
        return Fail("enabled runtime diagnostics did not record");
    }

    if (disabled_result.diagnostics_result.status != DiagnosticsStatus::Disabled) {
        return Fail("disabled runtime diagnostics did not report disabled status");
    }

    if (disabled_result.diagnostics_result.recorded_counter_count != 0U) {
        return Fail("disabled runtime diagnostics recorded counters");
    }

    if (disabled_result.diagnostics_snapshot.counter_count != 0U) {
        return Fail("disabled runtime diagnostics snapshot stored counters");
    }

    return 0;
}

int DiagnosticsRuntimeCountersRejectNullChannelWithoutMutation() {
    RuntimeDiagnosticsCounterRecorder recorder;
    RuntimeDiagnosticsCounters counters = RuntimeCounterSample();
    const RuntimeDiagnosticsCounters before_counters = counters;

    const DiagnosticsStatus register_status = recorder.RegisterCounters(static_cast<BoundedDiagnosticsChannel *>(nullptr));
    if (register_status != DiagnosticsStatus::InvalidArgument) {
        return Fail("runtime diagnostics null register did not return invalid argument");
    }

    const RuntimeDiagnosticsRecordResult result = recorder.RecordCounters(
        static_cast<BoundedDiagnosticsChannel *>(nullptr),
        counters);
    if (result.status != DiagnosticsStatus::InvalidArgument) {
        return Fail("runtime diagnostics null record did not return invalid argument");
    }

    if (result.recorded_counter_count != 0U) {
        return Fail("runtime diagnostics null record mutated result count");
    }

    if (!RuntimeCountersEqual(counters, before_counters)) {
        return Fail("runtime diagnostics null record mutated counters");
    }

    return 0;
}

int DiagnosticsRuntimeCountersValueTypesArePlainValues() {
    if (!std::is_standard_layout_v<RuntimeDiagnosticsCounters>) {
        return Fail("runtime diagnostics counters are not standard layout");
    }

    if (!std::is_trivially_copyable_v<RuntimeDiagnosticsCounters>) {
        return Fail("runtime diagnostics counters are not trivially copyable");
    }

    if (!std::is_standard_layout_v<RuntimeDiagnosticsRecordResult>) {
        return Fail("runtime diagnostics record result is not standard layout");
    }

    if (!std::is_trivially_copyable_v<RuntimeDiagnosticsRecordResult>) {
        return Fail("runtime diagnostics record result is not trivially copyable");
    }

    if (!std::is_standard_layout_v<RuntimeDiagnosticsOverlayHookProposal>) {
        return Fail("runtime diagnostics overlay proposal is not standard layout");
    }

    if (!std::is_trivially_copyable_v<RuntimeDiagnosticsOverlayHookProposal>) {
        return Fail("runtime diagnostics overlay proposal is not trivially copyable");
    }

    if (!std::is_standard_layout_v<RuntimeDiagnosticsOverlayHookResult>) {
        return Fail("runtime diagnostics overlay result is not standard layout");
    }

    if (!std::is_trivially_copyable_v<RuntimeDiagnosticsOverlayHookResult>) {
        return Fail("runtime diagnostics overlay result is not trivially copyable");
    }

    return 0;
}

int DiagnosticsOverlayHookProposalStaysOptionalToolingPlane() {
    RuntimeDiagnosticsOverlayHookProposal proposal{};
    proposal.is_enabled = true;
    proposal.requested_line_capacity = 4U;
    proposal.observed_counter_count = static_cast<std::uint32_t>(RUNTIME_DIAGNOSTICS_COUNTER_COUNT);

    RuntimeDiagnosticsOverlayHook hook;
    const RuntimeDiagnosticsOverlayHookResult result = hook.ValidateProposal(proposal);
    if (result.status != RuntimeDiagnosticsOverlayHookStatus::Success) {
        return Fail("runtime diagnostics overlay proposal was not accepted");
    }

    if (!result.is_optional_tooling_plane) {
        return Fail("runtime diagnostics overlay stopped being optional tooling");
    }

    if (result.runtime_dependency_required) {
        return Fail("runtime diagnostics overlay required runtime dependency");
    }

    if (result.accepted_line_capacity != proposal.requested_line_capacity) {
        return Fail("runtime diagnostics overlay line capacity mismatch");
    }

    if (result.observed_counter_count != proposal.observed_counter_count) {
        return Fail("runtime diagnostics overlay counter count mismatch");
    }

    return 0;
}

int DiagnosticsOverlayHookRejectsRuntimeDependency() {
    RuntimeDiagnosticsOverlayHookProposal proposal{};
    proposal.is_enabled = true;
    proposal.requires_runtime_dependency = true;
    proposal.requested_line_capacity = 1U;

    RuntimeDiagnosticsOverlayHook hook;
    const RuntimeDiagnosticsOverlayHookResult result = hook.ValidateProposal(proposal);
    if (result.status != RuntimeDiagnosticsOverlayHookStatus::RuntimeDependencyRejected) {
        return Fail("runtime diagnostics overlay accepted runtime dependency");
    }

    if (!result.is_optional_tooling_plane) {
        return Fail("runtime diagnostics overlay lost optional tooling status");
    }

    if (!result.runtime_dependency_required) {
        return Fail("runtime diagnostics overlay did not report rejected dependency");
    }

    return 0;
}

int DiagnosticsOverlayHookDisabledDoesNotChangeRuntimeValues() {
    RuntimeDiagnosticsOverlayHookProposal proposal{};
    RuntimeDiagnosticsOverlayHook hook;
    const RuntimeDiagnosticsOverlayHookResult hook_result = hook.ValidateProposal(proposal);
    if (hook_result.status != RuntimeDiagnosticsOverlayHookStatus::Disabled) {
        return Fail("runtime diagnostics overlay disabled status mismatch");
    }

    const RuntimeProbeResult enabled_result = RunRuntimeProbe(true);
    const RuntimeProbeResult disabled_result = RunRuntimeProbe(false);
    if (enabled_result.runtime_status != disabled_result.runtime_status) {
        return Fail("disabled overlay diagnostics changed runtime status");
    }

    if (!RuntimeCountersEqual(enabled_result.counters, disabled_result.counters)) {
        return Fail("disabled overlay diagnostics changed runtime counters");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_DISABLED_LOGGING, LoggingDisabledSinkDoesNotChangeBehavior},
        {TEST_MODULE_FILTER, LoggingModuleFilterAndDynamicSwitch},
        {TEST_DISABLED_CHANNEL, DiagnosticsDisabledChannelDoesNotChangeBehavior},
        {TEST_RECORDS_EVENTS_COUNTERS, DiagnosticsBoundedChannelRecordsEventsAndCounters},
        {TEST_DROPS_WHEN_FULL, DiagnosticsBoundedChannelDropsWhenFull},
        {TEST_SNAPSHOT, DiagnosticsChannelSnapshotReportsAcceptedDroppedAndCounters},
        {TEST_STOPPED, DiagnosticsChannelStoppedDoesNotMutateAfterShutdown},
        {TEST_UNKNOWN_IDS, DiagnosticsChannelRejectsUnknownIdsWhenValidationEnabled},
        {TEST_COUNTER_OVERFLOW, DiagnosticsCounterOverflowReturnsExplicitStatusAndDoesNotMutate},
        {TEST_NO_REPORT_DEPENDENCY, DiagnosticsNoReportDependencyForRuntimeResults},
        {TEST_MEMORY_SIGNAL, DiagnosticsNoHiddenAllocationUsesYuMemorySignal},
        {TEST_RUNTIME_COUNTERS, DiagnosticsRuntimeCountersRegisterAndRecordBoundedValues},
        {TEST_RUNTIME_DISABLED, DiagnosticsRuntimeCountersDisabledChannelDoesNotChangeRuntimeValues},
        {TEST_RUNTIME_NULL_CHANNEL, DiagnosticsRuntimeCountersRejectNullChannelWithoutMutation},
        {TEST_RUNTIME_PLAIN_VALUES, DiagnosticsRuntimeCountersValueTypesArePlainValues},
        {TEST_OVERLAY_OPTIONAL, DiagnosticsOverlayHookProposalStaysOptionalToolingPlane},
        {TEST_OVERLAY_REJECTS_RUNTIME_DEPENDENCY, DiagnosticsOverlayHookRejectsRuntimeDependency},
        {TEST_OVERLAY_DISABLED, DiagnosticsOverlayHookDisabledDoesNotChangeRuntimeValues}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
