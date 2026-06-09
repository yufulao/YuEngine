# P1-GATE-003: Thread Task Primitive Skeleton

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 博丽灵梦 with 八云紫
Reviewers: 红美铃, 雾雨魔理沙 when implementation exists
Depends on: ADR-0007
Related decisions: ADR-0001, ADR-0002, ADR-0005, ADR-0006
Source baseline: Phase 1 through `f9e59ec`

## Layer

L1-L2:

- L1 low-level task primitive vocabulary and bounded queue behavior.
- L2 integration later only when kernel/module scheduling needs approved task primitives.

## Owns

This gate owns the first implementation slice for:

- `YuThread` CMake target;
- bounded task queue;
- inline/manual task executor;
- task status/result values;
- deterministic drain and shutdown policy;
- scheduler snapshot/counters;
- task queue tests.

## Does Not Own

This gate does not own:

- production worker pool;
- OS worker thread lifecycle;
- work stealing;
- fibers or coroutines;
- async file IO;
- resource loading;
- render submission;
- audio callbacks;
- input polling;
- script execution;
- world/gameplay update order;
- diagnostics event bus or reports.

## UE/Unity Analogue

UE5 references:

- Task graph, queued thread pool, runnable thread, and event primitives as responsibility references.

Unity references:

- Job system concepts as product-boundary references.

YuEngine decision:

- Start with deterministic bounded queue and inline execution.
- Defer real worker/thread pool behavior until queue semantics, memory policy, and shutdown behavior are proven.

## Lifecycle

First slice lifecycle:

1. Setup creates a bounded queue with declared capacity.
2. Setup submits fixed task records.
3. Drain executes tasks deterministically on the caller thread.
4. Shutdown either drains queued tasks or cancels queued tasks according to explicit policy.
5. Snapshot reports submitted, executed, rejected, failed, canceled, max queue depth, and shutdown state.

Failure behavior:

- queue overflow returns explicit rejected result;
- failed task returns explicit failed result;
- submit after shutdown returns explicit rejected result;
- shutdown failure is surfaced explicitly;
- no behavior depends on diagnostics/log output.

## Inputs

- fixed task queue capacity;
- fixed test tasks;
- shutdown policy;
- optional memory tracker/snapshot if P1-GATE-002 implementation exists.

## Outputs

- task results;
- scheduler snapshot;
- deterministic task execution trace for tests;
- no JSON report output as runtime API.

## Dependencies

Allowed dependencies:

- C++ standard library;
- CMake/CTest tooling;
- `YuMemory` only if P1-GATE-002 implementation has landed and the integration is limited to capacity/allocation fixture signals;
- `YuDiagnostics` only in tests if needed to prove diagnostics-disabled behavior.

Target dependency expectation:

```text
YuThread
  -> optional YuMemory after P1-GATE-002 implementation
```

`YuThread` must not depend on `YuKernel`, `YuPlatform`, file/resource/RHI/audio/input/script/world/UI modules, tools, reports, or original-game evidence.

Forbidden dependencies:

- TouhouNewWorld `bin` or `resource`;
- old `YuEngine_BACKUP` runtime files;
- old `FrameRuntime.*`;
- gameplay/world/resource/render/audio/input/script modules;
- diagnostics report schemas;
- thread/task behavior owned by reports or tools.

## Performance Constraints

Required deterministic signals:

- submitted count;
- executed count;
- rejected count;
- failed count;
- canceled count;
- maximum queue depth;
- queue capacity before and after fixture;
- drain operation count;
- tracked allocation count or explicit deferral if P1-GATE-002 implementation is not yet present;
- shutdown result.

Blocking conditions:

- unbounded queue;
- dynamic queue growth in the measured fixture;
- hidden hot-path allocation in task execution;
- default lambda capture in persistent/hot task callbacks;
- task behavior observable only through logs;
- background worker thread introduced by this slice;
- gameplay update order or async resource loading enters the task API.

## Tests

Fast gate tests required before the slice can be considered complete:

- `Thread_QueueEnqueueWithinCapacity_Succeeds`
- `Thread_QueueEnqueueBeyondCapacity_Rejects`
- `Thread_DrainExecutesTasks_InDeterministicOrder`
- `Thread_TaskFailure_ReturnsFailedResult`
- `Thread_ShutdownRejectsNewSubmission`
- `Thread_ShutdownDrainPolicy_ExecutesQueuedTasks`
- `Thread_ShutdownCancelPolicy_CancelsQueuedTasks`
- `Thread_QueueCapacity_DoesNotGrowDuringFixture`
- `Thread_DiagnosticsDisabled_DoesNotChangeBehavior`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate
```

The implementation handoff must record the exact commands used.

## Allowed First Slice

If approved, the first implementation slice may create:

```text
src/yuengine/thread/include/yuengine/thread/
src/yuengine/thread/src/
tests/thread/
```

It may update root `CMakeLists.txt` only to add `YuThread` and `YuThreadTests`.

It may not create placeholder directories or targets for file, RHI, audio, input, resource, script, world, UI, tools, or game adapter work.

## Non-Goals

- No worker pool.
- No OS thread lifecycle.
- No work stealing.
- No async IO.
- No resource loading job graph.
- No render/audio/input/script/world scheduling.
- No gameplay update phases.
- No profiler or report schema.

## Evidence Inputs

None required.

Thread/task primitives are general lower-layer engine infrastructure. TouhouNewWorld evidence must not shape this API.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- ADR-0007 is accepted;
- 红美铃 confirms the proposal satisfies module-entry gate requirements;
- 博丽灵梦 confirms queue capacity, shutdown, hot-path allocation, and callback/capture rules;
- 雾雨魔理沙 confirms the first slice is enforceable in code review.

If those conditions are not met, return `NEEDS_ARCHITECTURE` or `NEEDS_PERFORMANCE` with exact missing fields.
