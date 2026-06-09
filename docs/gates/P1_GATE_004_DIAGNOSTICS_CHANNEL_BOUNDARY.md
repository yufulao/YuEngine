# P1-GATE-004: Diagnostics Channel Boundary

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 红美铃, 博丽灵梦, 雾雨魔理沙
Depends on: ADR-0004
Related decisions: ADR-0005, ADR-0006, ADR-0007
Source baseline: Phase 1 through `1725931`

## Layer

L2/L7 boundary:

- L2 bounded runtime observability for counters/events/snapshots.
- L7 reports/tools are future consumers only, not runtime owners.

## Owns

This gate owns the first diagnostics-channel proposal for:

- diagnostics channel vocabulary;
- counter ID and event ID value types;
- `uint64_t` counter values and update semantics;
- synchronous bounded event/counter recording;
- disabled diagnostics behavior;
- snapshot query for tests;
- accepted/dropped counts;
- enabled/disabled behavior and cost signals.

## Does Not Own

This gate does not own:

- JSON report schema;
- file trace format;
- profiler scope API;
- capture/oracle APIs;
- dashboard feed;
- async diagnostics queue;
- background diagnostics thread;
- report serialization;
- runtime decisions that depend on diagnostics output.

## ADR Boundary

ADR-0004 separates:

```text
Logging sink
Diagnostics channel
Reports
Capture/oracle
```

P1-GATE-001 implemented only logging sinks. P1-GATE-004 may introduce the first diagnostics channel, but only as a bounded observer. It must not turn logging output, reports, or test traces into runtime behavior ownership.

## Lifecycle

First slice lifecycle:

1. Setup creates a disabled diagnostics channel or bounded in-memory diagnostics channel.
2. Setup declares channel capacity and fixed accepted event/counter IDs.
3. Runtime/test fixture records fixed counters and events synchronously, without owned string payloads.
4. Snapshot exposes accepted, dropped, counter values, and disabled state.
5. Shutdown freezes the final snapshot; later records return explicit stopped/disabled status and do not mutate state.

Failure behavior:

- event capacity overflow increments dropped count and returns explicit dropped status;
- unknown counter/event ID returns explicit error if validation is enabled and does not mutate counters/events;
- counter overflow returns explicit overflow status and does not mutate the counter value or successful counter-update count;
- disabled channel does not change host/kernel/module behavior;
- no test may pass only by parsing log text or report output.

## Inputs

- fixed diagnostics event IDs;
- fixed counter IDs;
- `uint64_t` counter update values;
- explicit channel capacity;
- fixed test fixture events/counters;
- YuMemory accounting vocabulary for allocation/byte signals in this baseline.

## Outputs

- diagnostics snapshot;
- accepted/dropped event counts;
- counter values;
- successful counter update count;
- explicit status values;
- no JSON report output as runtime API.

## Dependencies

Allowed dependencies:

- C++ standard library;
- CMake/CTest tooling;
- existing `YuDiagnostics` logging target;
- `YuMemory` for allocation/byte fixture signals only.

Target dependency expectation:

```text
YuDiagnostics
  -> YuMemory for accounting vocabulary/signal tests only
```

If P1-GATE-002 / task #14 later receives a blocking rewrite that removes the
current YuMemory vocabulary, this gate must be amended before implementation
handoff instead of silently taking an untracked allocation deferral.

`YuDiagnostics` must not depend on `YuKernel`, `YuPlatform`, `YuThread`, `YuFile`, resource/RHI/audio/input/script/world/UI modules, tools, reports, or original-game evidence.

Async diagnostics behavior requires an accepted Thread/Task implementation and a later gate. P1-GATE-004 first slice is synchronous only.

## Performance Constraints

Required deterministic signals:

- event accepted count;
- event dropped count;
- counter update count;
- snapshot query count;
- channel capacity;
- enabled versus disabled behavior equivalence;
- allocation/accounting status using `YuMemory` vocabulary;
- no report dependency.

First-slice bounds:

- event buffer capacity: 16 events maximum;
- counter capacity: 8 counters maximum;
- accepted event ID capacity: 8 IDs maximum;
- accepted counter ID capacity: 8 IDs maximum;
- event payload: fixed event ID plus at most one 64-bit value; no owned strings in measured fixtures;
- counter value type: `uint64_t`;
- counter operations: increment by one and add unsigned delta;
- counter overflow policy: if `current > UINT64_MAX - delta`, return explicit overflow status, leave the value unchanged, and do not increment successful counter-update count;
- snapshot capacity equals the declared event/counter capacity and may not grow dynamically after setup;
- allocation/accounting rule: use `YuMemory` vocabulary for allocation/read-buffer signals in this baseline; do not claim zero CRT/STL/general heap coverage.

Pass/fail rule:

- exceeding event capacity, counter capacity, accepted ID capacity, or event payload bounds is an explicit failure or dropped-status path;
- changing accepted count, dropped count, counter update count, snapshot query count, allocation/accounting status, or report-dependency status outside the declared fixture is a gate failure unless this gate is amended.

Blocking conditions:

- unbounded event buffer;
- asynchronous queue or background thread;
- hot-path string formatting before category/filter decision;
- diagnostics output required for runtime behavior;
- JSON/report/capture/oracle types in runtime APIs;
- allocation hidden behind diagnostics in a measured hot path;
- tests that validate behavior by parsing logs or reports.

## Runtime Behavior Rule

Diagnostics observes runtime behavior. It does not own it.

Rules:

- Runtime result values remain explicit host/kernel/module/file/thread/memory results.
- Diagnostics may mirror events and counters.
- Disabling diagnostics must not change behavior, result values, or lifecycle order.
- Diagnostics snapshots are test observations, not behavior transport.

## Tests

Fast gate tests required before the slice can be considered complete:

- `Diagnostics_DisabledChannel_DoesNotChangeBehavior`
- `Diagnostics_BoundedChannel_RecordsEventsAndCounters`
- `Diagnostics_BoundedChannel_DropsWhenFull`
- `Diagnostics_ChannelSnapshot_ReportsAcceptedDroppedAndCounters`
- `Diagnostics_ChannelStopped_DoesNotMutateAfterShutdown`
- `Diagnostics_ChannelRejectsUnknownIds_WhenValidationEnabled`
- `Diagnostics_CounterOverflow_ReturnsExplicitStatusAndDoesNotMutate`
- `Diagnostics_NoReportDependency_ForRuntimeResults`
- `Diagnostics_NoHiddenAllocation_UsesYuMemorySignal`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate
```

The implementation handoff must record the exact commands used.

## Allowed First Slice

If approved, the first implementation slice may update only:

```text
src/yuengine/diagnostics/include/yuengine/diagnostics/
src/yuengine/diagnostics/src/
tests/diagnostics/
```

It may update root `CMakeLists.txt` only for diagnostics tests/source files needed by this slice.

It may add no placeholder directories or targets for tools, reports, capture, oracle, profiler dashboards, thread, file, resource, RHI, audio, input, script, world, UI, or game adapter work.

## Non-Goals

- No profiler API.
- No trace file format.
- No report serialization.
- No dashboard.
- No capture/oracle.
- No async diagnostics queue.
- No background worker thread.
- No cross-module behavior ownership.
- No original-game evidence.

## Evidence Inputs

None required.

Diagnostics channel behavior is a general lower-layer engine concern. TouhouNewWorld evidence and old report counts must not shape this API.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- 红美铃 confirms the proposal satisfies module-entry gate requirements;
- 博丽灵梦 confirms bounded cost, disabled behavior, and allocation/accounting requirements;
- 雾雨魔理沙 confirms the first slice is enforceable in code review;
- YuMemory allocation/accounting vocabulary remains available from the P1-GATE-002 implementation baseline.

If those conditions are not met, return `NEEDS_ARCHITECTURE` or `NEEDS_PERFORMANCE` with exact missing fields.
