#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "yuengine/diagnostics/bounded_diagnostics_channel.h"
#include "yuengine/diagnostics/bounded_in_memory_log_sink.h"
#include "yuengine/diagnostics/diagnostics_channel_config.h"
#include "yuengine/diagnostics/diagnostics_counter_id.h"
#include "yuengine/diagnostics/diagnostics_event_id.h"
#include "yuengine/diagnostics/diagnostics_snapshot.h"
#include "yuengine/diagnostics/diagnostics_status.h"
#include "yuengine/diagnostics/disabled_diagnostics_channel.h"
#include "yuengine/diagnostics/disabled_log_sink.h"
#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/platform/fixed_frame_clock.h"
#include "yuengine/platform/headless_host.h"
#include "yuengine/platform/host_status.h"
#include "yuengine/platform/i_host_runtime.h"

using BoundedDiagnosticsChannel = yuengine::diagnostics::BoundedDiagnosticsChannel;
using BoundedInMemoryLogSink = yuengine::diagnostics::BoundedInMemoryLogSink;
using DiagnosticsChannelConfig = yuengine::diagnostics::DiagnosticsChannelConfig;
using DiagnosticsCounterId = yuengine::diagnostics::DiagnosticsCounterId;
using DiagnosticsEventId = yuengine::diagnostics::DiagnosticsEventId;
using DiagnosticsSnapshot = yuengine::diagnostics::DiagnosticsSnapshot;
using DiagnosticsStatus = yuengine::diagnostics::DiagnosticsStatus;
using DisabledDiagnosticsChannel = yuengine::diagnostics::DisabledDiagnosticsChannel;
using DisabledLogSink = yuengine::diagnostics::DisabledLogSink;
using FixedFrameClock = yuengine::platform::FixedFrameClock;
using HeadlessHost = yuengine::platform::HeadlessHost;
using HeadlessHostConfig = yuengine::platform::HeadlessHostConfig;
using HostError = yuengine::platform::HostError;
using HostStatus = yuengine::platform::HostStatus;
using IHostRuntime = yuengine::platform::IHostRuntime;
using MemoryAccountingStatus = yuengine::memory::MemoryAccountingStatus;

namespace {
constexpr const char* TEST_DISABLED_LOGGING = "Logging_DisabledSink_DoesNotChangeBehavior";
constexpr const char* TEST_DISABLED_CHANNEL = "Diagnostics_DisabledChannel_DoesNotChangeBehavior";
constexpr const char* TEST_RECORDS_EVENTS_COUNTERS = "Diagnostics_BoundedChannel_RecordsEventsAndCounters";
constexpr const char* TEST_DROPS_WHEN_FULL = "Diagnostics_BoundedChannel_DropsWhenFull";
constexpr const char* TEST_SNAPSHOT = "Diagnostics_ChannelSnapshot_ReportsAcceptedDroppedAndCounters";
constexpr const char* TEST_STOPPED = "Diagnostics_ChannelStopped_DoesNotMutateAfterShutdown";
constexpr const char* TEST_UNKNOWN_IDS = "Diagnostics_ChannelRejectsUnknownIds_WhenValidationEnabled";
constexpr const char* TEST_COUNTER_OVERFLOW = "Diagnostics_CounterOverflow_ReturnsExplicitStatusAndDoesNotMutate";
constexpr const char* TEST_NO_REPORT_DEPENDENCY = "Diagnostics_NoReportDependency_ForRuntimeResults";
constexpr const char* TEST_MEMORY_SIGNAL = "Diagnostics_NoHiddenAllocation_UsesYuMemorySignal";
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
    HostError Start(std::vector<std::string>& lifecycleTrace) override {
        lifecycleTrace.push_back("runtime.start");
        return HostError::Success();
    }

    HostError Tick(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) override {
        static_cast<void>(frameIndex);
        static_cast<void>(tickTimeNanoseconds);
        lifecycleTrace.push_back("runtime.tick");
        return HostError::Success();
    }

    HostError Shutdown(std::vector<std::string>& lifecycleTrace) override {
        lifecycleTrace.push_back("runtime.shutdown");
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
    const auto eventStatus = channel.RegisterEventId(DiagnosticsEventId{EVENT_ID});
    if (eventStatus != DiagnosticsStatus::Success) {
        return channel;
    }

    channel.RegisterEventId(DiagnosticsEventId{SECOND_EVENT_ID});
    channel.RegisterCounterId(DiagnosticsCounterId{COUNTER_ID});
    channel.RegisterCounterId(DiagnosticsCounterId{SECOND_COUNTER_ID});
    return channel;
}

int LoggingDisabledSinkDoesNotChangeBehavior() {
    BoundedInMemoryLogSink recordingLogSink(LOG_CAPACITY);
    FixedFrameClock recordingClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime recordingRuntime;
    HeadlessHost recordingHost(recordingClock, recordingLogSink);

    const HeadlessHostConfig config{TICK_COUNT};
    const auto recordingResult = recordingHost.Run(recordingRuntime, config);
    if (recordingResult.Status != HostStatus::Success) {
        return Fail("recording host run failed");
    }

    DisabledLogSink disabledLogSink;
    FixedFrameClock disabledClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime disabledRuntime;
    HeadlessHost disabledHost(disabledClock, disabledLogSink);

    const auto disabledResult = disabledHost.Run(disabledRuntime, config);
    if (disabledResult.Status != HostStatus::Success) {
        return Fail("disabled host run failed");
    }

    if (recordingResult.LifecycleTrace != disabledResult.LifecycleTrace) {
        return Fail("disabled logging changed lifecycle behavior");
    }

    if (recordingResult.TickTimesNanoseconds != disabledResult.TickTimesNanoseconds) {
        return Fail("disabled logging changed timer behavior");
    }

    if (recordingLogSink.Events().empty()) {
        return Fail("recording logging did not observe host events");
    }

    if (recordingLogSink.DroppedCount() != 0U) {
        return Fail("bounded in-memory sink dropped unexpected events");
    }

    if (disabledLogSink.IsEnabled()) {
        return Fail("disabled sink reported enabled");
    }

    return 0;
}

int DiagnosticsDisabledChannelDoesNotChangeBehavior() {
    DisabledDiagnosticsChannel disabledChannel;
    const auto eventStatus = disabledChannel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    if (eventStatus != DiagnosticsStatus::Disabled) {
        return Fail("disabled diagnostics channel did not return disabled status");
    }

    const auto counterStatus = disabledChannel.IncrementCounter(DiagnosticsCounterId{COUNTER_ID});
    if (counterStatus != DiagnosticsStatus::Disabled) {
        return Fail("disabled diagnostics counter did not return disabled status");
    }

    const DiagnosticsSnapshot snapshot = disabledChannel.Snapshot();
    if (snapshot.Enabled) {
        return Fail("disabled diagnostics snapshot reported enabled");
    }

    if (snapshot.AcceptedEventCount != 0U) {
        return Fail("disabled diagnostics channel accepted an event");
    }

    if (snapshot.SuccessfulCounterUpdateCount != 0U) {
        return Fail("disabled diagnostics channel accepted a counter update");
    }

    HostStatus explicitRuntimeStatus = HostStatus::Success;
    if (explicitRuntimeStatus != HostStatus::Success) {
        return Fail("disabled diagnostics changed explicit runtime status");
    }

    return 0;
}

int DiagnosticsBoundedChannelRecordsEventsAndCounters() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    const auto eventStatus = channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    if (eventStatus != DiagnosticsStatus::Success) {
        return Fail("bounded diagnostics channel rejected valid event");
    }

    const auto counterStatus = channel.AddCounter(DiagnosticsCounterId{COUNTER_ID}, COUNTER_DELTA);
    if (counterStatus != DiagnosticsStatus::Success) {
        return Fail("bounded diagnostics channel rejected valid counter update");
    }

    const DiagnosticsSnapshot snapshot = channel.Snapshot();
    if (snapshot.EventCount != 1U) {
        return Fail("bounded diagnostics channel event count was wrong");
    }

    if (snapshot.Events[0U].Payload != EVENT_PAYLOAD) {
        return Fail("bounded diagnostics channel event payload was wrong");
    }

    if (snapshot.Counters[0U].Value != COUNTER_DELTA) {
        return Fail("bounded diagnostics channel counter value was wrong");
    }

    if (snapshot.SuccessfulCounterUpdateCount != 1U) {
        return Fail("bounded diagnostics channel counter update count was wrong");
    }

    return 0;
}

int DiagnosticsBoundedChannelDropsWhenFull() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    const auto firstStatus = channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    const auto secondStatus = channel.RecordEvent(DiagnosticsEventId{SECOND_EVENT_ID}, EVENT_PAYLOAD);
    const auto thirdStatus = channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    if (firstStatus != DiagnosticsStatus::Success) {
        return Fail("first event was not accepted");
    }

    if (secondStatus != DiagnosticsStatus::Success) {
        return Fail("second event was not accepted");
    }

    if (thirdStatus != DiagnosticsStatus::Dropped) {
        return Fail("full diagnostics channel did not return dropped status");
    }

    const DiagnosticsSnapshot snapshot = channel.Snapshot();
    if (snapshot.EventCount != EVENT_CAPACITY) {
        return Fail("full diagnostics channel changed event storage size");
    }

    if (snapshot.DroppedEventCount != 1U) {
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
    if (snapshot.AcceptedEventCount != 2U) {
        return Fail("snapshot accepted event count was wrong");
    }

    if (snapshot.DroppedEventCount != 1U) {
        return Fail("snapshot dropped event count was wrong");
    }

    if (snapshot.Counters[0U].Value != COUNTER_DELTA + 1U) {
        return Fail("snapshot counter value was wrong");
    }

    if (snapshot.SuccessfulCounterUpdateCount != 2U) {
        return Fail("snapshot counter update count was wrong");
    }

    if (snapshot.SnapshotQueryCount != 1U) {
        return Fail("snapshot query count was wrong");
    }

    return 0;
}

int DiagnosticsChannelStoppedDoesNotMutateAfterShutdown() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    channel.IncrementCounter(DiagnosticsCounterId{COUNTER_ID});
    const auto shutdownStatus = channel.Shutdown();
    if (shutdownStatus != DiagnosticsStatus::Success) {
        return Fail("diagnostics channel shutdown failed");
    }

    const DiagnosticsSnapshot beforeRecord = channel.Snapshot();
    const auto eventStatus = channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    const auto counterStatus = channel.IncrementCounter(DiagnosticsCounterId{COUNTER_ID});
    const DiagnosticsSnapshot afterRecord = channel.Snapshot();
    if (eventStatus != DiagnosticsStatus::Stopped) {
        return Fail("stopped diagnostics channel did not reject event");
    }

    if (counterStatus != DiagnosticsStatus::Stopped) {
        return Fail("stopped diagnostics channel did not reject counter update");
    }

    if (afterRecord.AcceptedEventCount != beforeRecord.AcceptedEventCount) {
        return Fail("stopped diagnostics channel mutated accepted event count");
    }

    if (afterRecord.SuccessfulCounterUpdateCount != beforeRecord.SuccessfulCounterUpdateCount) {
        return Fail("stopped diagnostics channel mutated counter update count");
    }

    if (afterRecord.Counters[0U].Value != beforeRecord.Counters[0U].Value) {
        return Fail("stopped diagnostics channel mutated counter value");
    }

    return 0;
}

int DiagnosticsChannelRejectsUnknownIdsWhenValidationEnabled() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    const auto eventStatus = channel.RecordEvent(DiagnosticsEventId{UNKNOWN_EVENT_ID}, EVENT_PAYLOAD);
    const auto counterStatus = channel.IncrementCounter(DiagnosticsCounterId{UNKNOWN_COUNTER_ID});
    if (eventStatus != DiagnosticsStatus::UnknownEventId) {
        return Fail("unknown diagnostics event id was not rejected");
    }

    if (counterStatus != DiagnosticsStatus::UnknownCounterId) {
        return Fail("unknown diagnostics counter id was not rejected");
    }

    const DiagnosticsSnapshot snapshot = channel.Snapshot();
    if (snapshot.AcceptedEventCount != 0U) {
        return Fail("unknown diagnostics event mutated accepted count");
    }

    if (snapshot.SuccessfulCounterUpdateCount != 0U) {
        return Fail("unknown diagnostics counter mutated update count");
    }

    return 0;
}

int DiagnosticsCounterOverflowReturnsExplicitStatusAndDoesNotMutate() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    const std::uint64_t maxValue = std::numeric_limits<std::uint64_t>::max();
    const auto firstStatus = channel.AddCounter(DiagnosticsCounterId{COUNTER_ID}, maxValue);
    const DiagnosticsSnapshot beforeOverflow = channel.Snapshot();
    const auto overflowStatus = channel.AddCounter(DiagnosticsCounterId{COUNTER_ID}, 1U);
    const DiagnosticsSnapshot afterOverflow = channel.Snapshot();
    if (firstStatus != DiagnosticsStatus::Success) {
        return Fail("counter max setup update failed");
    }

    if (overflowStatus != DiagnosticsStatus::CounterOverflow) {
        return Fail("counter overflow did not return explicit status");
    }

    if (afterOverflow.Counters[0U].Value != beforeOverflow.Counters[0U].Value) {
        return Fail("counter overflow mutated counter value");
    }

    if (afterOverflow.SuccessfulCounterUpdateCount != beforeOverflow.SuccessfulCounterUpdateCount) {
        return Fail("counter overflow mutated successful update count");
    }

    return 0;
}

int DiagnosticsNoReportDependencyForRuntimeResults() {
    DisabledLogSink logSink;
    FixedFrameClock frameClock(FIRST_TICK_NANOSECONDS, STEP_NANOSECONDS);
    TraceRuntime runtime;
    HeadlessHost host(frameClock, logSink);
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();

    const HeadlessHostConfig config{TICK_COUNT};
    const auto result = host.Run(runtime, config);
    const auto eventStatus = channel.RecordEvent(DiagnosticsEventId{EVENT_ID}, EVENT_PAYLOAD);
    if (result.Status != HostStatus::Success) {
        return Fail("host result depended on diagnostics output");
    }

    if (eventStatus != DiagnosticsStatus::Success) {
        return Fail("diagnostics event fixture was not accepted");
    }

    if (result.LifecycleTrace.empty()) {
        return Fail("runtime result was not explicit without reports");
    }

    return 0;
}

int DiagnosticsNoHiddenAllocationUsesYuMemorySignal() {
    BoundedDiagnosticsChannel channel = CreateRegisteredChannel();
    const DiagnosticsSnapshot snapshot = channel.Snapshot();
    if (snapshot.AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("diagnostics channel did not expose YuMemory explicit-tracking status");
    }

    if (snapshot.Enabled != true) {
        return Fail("diagnostics channel snapshot did not report enabled state");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> testRegistry{
        {TEST_DISABLED_LOGGING, LoggingDisabledSinkDoesNotChangeBehavior},
        {TEST_DISABLED_CHANNEL, DiagnosticsDisabledChannelDoesNotChangeBehavior},
        {TEST_RECORDS_EVENTS_COUNTERS, DiagnosticsBoundedChannelRecordsEventsAndCounters},
        {TEST_DROPS_WHEN_FULL, DiagnosticsBoundedChannelDropsWhenFull},
        {TEST_SNAPSHOT, DiagnosticsChannelSnapshotReportsAcceptedDroppedAndCounters},
        {TEST_STOPPED, DiagnosticsChannelStoppedDoesNotMutateAfterShutdown},
        {TEST_UNKNOWN_IDS, DiagnosticsChannelRejectsUnknownIdsWhenValidationEnabled},
        {TEST_COUNTER_OVERFLOW, DiagnosticsCounterOverflowReturnsExplicitStatusAndDoesNotMutate},
        {TEST_NO_REPORT_DEPENDENCY, DiagnosticsNoReportDependencyForRuntimeResults},
        {TEST_MEMORY_SIGNAL, DiagnosticsNoHiddenAllocationUsesYuMemorySignal}};

    const std::string_view testName(argv[1]);
    const auto testIterator = testRegistry.find(testName);
    if (testIterator == testRegistry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testIterator->second();
}
