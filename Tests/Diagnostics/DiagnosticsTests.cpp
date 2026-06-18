// 模块：Tests Diagnostics
// 文件：Tests/Diagnostics/DiagnosticsTests.cpp

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "YuEngine/Diagnostics/BoundedDiagnosticsChannel.h"
#include "YuEngine/Diagnostics/BoundedInMemoryLogSink.h"
#include "YuEngine/Diagnostics/DiagnosticsChannelConfig.h"
#include "YuEngine/Diagnostics/DiagnosticsCounterId.h"
#include "YuEngine/Diagnostics/DiagnosticsEventId.h"
#include "YuEngine/Diagnostics/DiagnosticsSnapshot.h"
#include "YuEngine/Diagnostics/DiagnosticsStatus.h"
#include "YuEngine/Diagnostics/DisabledDiagnosticsChannel.h"
#include "YuEngine/Diagnostics/DisabledLogSink.h"
#include "YuEngine/Diagnostics/LogLevel.h"
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
using FixedFrameClock = yuengine::platform::FixedFrameClock;
using HeadlessHost = yuengine::platform::HeadlessHost;
using yuengine::platform::HeadlessHostConfig;
using yuengine::platform::HostError;
using yuengine::platform::HostStatus;
using IHostRuntime = yuengine::platform::IHostRuntime;
using yuengine::memory::MemoryAccountingStatus;

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

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

DiagnosticsChannelConfig TestChannelConfig() {
    return DiagnosticsChannelConfig{EVENT_CAPACITY, COUNTER_CAPACITY, ID_CAPACITY, ID_CAPACITY, true};
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

    HostStatus explicit_runtime_status = HostStatus::Success;
    if (explicit_runtime_status != HostStatus::Success) {
        return Fail("disabled diagnostics changed explicit runtime status");
    }

    return 0;
}

int DiagnosticsBoundedChannelRecordsEventsAndCounters() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
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
        {TEST_MEMORY_SIGNAL, DiagnosticsNoHiddenAllocationUsesYuMemorySignal}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
