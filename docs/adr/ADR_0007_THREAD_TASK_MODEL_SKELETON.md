# ADR-0007: Thread And Task Model Skeleton

Status: Accepted
Owner: 博丽灵梦 with 八云紫
Reviewers: 雾雨魔理沙, 博丽灵梦 performance, 红美铃 gate/PM
Date: 2026-06-10
Depends on: ADR-0001, ADR-0002, ADR-0005, ADR-0006, docs/YUENGINE_PERFORMANCE_GATES.md, docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md

## Context

YuEngine needs a thread/task boundary before async file IO, resource loading, diagnostics queues, render submission preparation, audio streaming, or script jobs can appear. Mature engines separate task scheduling primitives from gameplay update order and resource semantics; YuEngine must do the same.

The old failure mode to avoid is introducing a background worker or unbounded queue because a later system might need async work. The first Thread/Task slice must prove deterministic queue behavior, drain behavior, shutdown behavior, and cost signals before real worker pools are allowed.

Reference inputs:

- `docs/YUENGINE_ARCHITECTURE_RESTART_PLAN.md`: Thread/Task owns worker threads, job queues, synchronization, and frame scheduling primitives; it does not own gameplay update order.
- `docs/YUENGINE_PERFORMANCE_GATES.md`: task queues and async IO need deterministic shutdown, bounded queueing, explicit cancellation or drain policy, and queue-depth signals.
- UE5 Core references sampled locally: task graph, queued thread pool, runnable thread, event primitives. These are responsibility references only, not API templates.

## Decision

YuEngine introduces a `YuThread` module as the owner of task primitive vocabulary, bounded queue semantics, deterministic drain/shutdown rules, and future worker ownership.

P1-GATE-003 does not introduce a production thread pool. The first slice is a single-thread deterministic task model:

```text
TaskId
TaskStatus
TaskResult
TaskPriority
TaskQueueCapacity
TaskRecord
InlineTaskExecutor
BoundedTaskQueue
TaskSchedulerSnapshot
```

Names may change during implementation, but the responsibilities must stay equivalent.

## First-Slice Execution Model

The first slice uses an inline/manual executor:

1. setup creates a bounded task queue with explicit capacity;
2. setup enqueues fixed test tasks;
3. drain executes queued tasks deterministically on the caller thread;
4. shutdown rejects new tasks and drains or cancels according to explicit policy;
5. tests read a snapshot of submitted, executed, rejected, canceled, max-depth, and shutdown state.

This proves scheduling semantics without introducing OS worker lifetime, race-prone synchronization, or nondeterministic timing into Phase 1.

## Ownership

`YuThread` owns:

- task queue capacity and overflow policy;
- task lifecycle states;
- submit/drain/shutdown behavior;
- cancellation or drain policy;
- task execution count and queue-depth signals;
- future worker ownership vocabulary.

`YuThread` does not own:

- gameplay update order;
- world phase scheduling;
- async file semantics;
- resource load dependency graph;
- render submission policy;
- audio callback behavior;
- script/native dispatch;
- diagnostics report or trace serialization.

Those systems may later consume `YuThread` primitives after their own gates.

## Task Lifecycle

Initial task states:

```text
Created
Queued
Running
Completed
Failed
Canceled
Rejected
```

Rules:

- A task is queued only through an explicit submit API.
- Queue overflow returns explicit rejection.
- Drain executes queued tasks in deterministic order.
- Task failure is returned as explicit result and recorded in the scheduler snapshot.
- Shutdown policy is explicit: drain queued tasks or cancel queued tasks, chosen by the caller or gate.
- After shutdown begins, new task submission is rejected.
- No task result may be observable only through logs.

## Queue And Capacity Rules

The first queue is bounded.

Required behavior:

- capacity is declared at setup;
- max queue depth is recorded;
- enqueue beyond capacity returns explicit rejection;
- no dynamic growth occurs while executing the measured fixture;
- drain order is deterministic;
- shutdown leaves no pending tasks unless the selected policy is cancel.

Blocked behavior:

- unbounded queue;
- background worker thread;
- lock-free queue without a testable capacity policy;
- task queue owned by diagnostics, reports, gameplay, resource, or file modules;
- hidden allocation in task execution hot path;
- waiting on external OS objects in the first slice.

## Callback And Capture Rules

Tasks are callbacks and follow `docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md`.

Rules:

- no default lambda capture in hot-path task bodies;
- persistent callbacks must state owner and lifetime;
- stored capturing callables must prove setup-time allocation only or use an approved fixed-capacity callable policy;
- task bodies must not allocate, format strings, block on IO, or query services by string in hot-path fixtures;
- task failure returns status, not log text.

The first slice may use small test callables only where lifetime is local and deterministic. It must not standardize `std::function` as a hot-path storage type unless allocation behavior is proved and bounded.

## Synchronization Policy

The first slice uses caller-thread drain and does not require locks or condition variables.

Future worker gates must define:

- worker count;
- wake strategy;
- synchronization primitive;
- blocking points;
- queue depth/backpressure;
- cancellation/drain behavior;
- shutdown timeout and failure path.

Until then, synchronization remains a documented future boundary.

## Memory And Diagnostics Boundary

Memory:

- setup-time queue storage must be owned and bounded;
- hot-path task execution has zero tracked allocation by default;
- any fixture allocation must be reported through the ADR-0006 memory vocabulary when available;
- queue capacity before/after fixture must be stable.

Diagnostics:

- diagnostics may observe submitted/executed/rejected/canceled counts;
- diagnostics must be disabled without changing task behavior;
- reports and traces do not define scheduler state;
- tests must read scheduler snapshots or task results, not logs.

## Test Requirements

First-slice tests must cover:

- enqueue within capacity succeeds;
- enqueue beyond capacity returns explicit rejection;
- drain executes tasks in deterministic FIFO order unless a priority policy is explicitly approved;
- task failure returns explicit failed result and is reflected in snapshot counts;
- shutdown rejects new submission;
- drain-on-shutdown executes queued tasks and leaves no pending tasks;
- cancel-on-shutdown cancels queued tasks and records canceled count;
- max queue depth is recorded;
- queue capacity does not grow during the fixture;
- disabled diagnostics/logging does not change task results;
- hot-path fixture has zero tracked allocation or an explicit approved budget.

## Performance Requirements

Required deterministic signals:

- submitted count;
- executed count;
- rejected count;
- failed count;
- canceled count;
- maximum queue depth;
- queue capacity before and after fixture;
- drain operation count;
- tracked allocation count for the task execution fixture;
- shutdown result.

No benchmark claim is required in the first slice. The goal is bounded behavior and measurable signals.

## P1-GATE-003 Compatibility

P1-GATE-003 may implement:

- `YuThread` target;
- bounded task queue;
- inline/manual executor;
- task result/status types;
- scheduler snapshot;
- tests under `tests/thread`;
- optional memory-accounting integration only if P1-GATE-002 has landed and the integration stays narrow.

P1-GATE-003 must not implement:

- production worker pool;
- real OS worker thread lifecycle;
- async file IO;
- resource loading jobs;
- render/audio/input/script/world scheduling;
- gameplay update phases;
- diagnostics event bus;
- unbounded task queues;
- lock-free data structures without an accepted capacity and memory policy.

## Non-Goals

This ADR does not decide:

- final job system API;
- worker pool implementation;
- work stealing;
- job dependencies;
- fibers/coroutines;
- async IO;
- frame graph or world phase scheduling;
- render submission threading;
- audio callback threading.

Those require later ADRs and gates.

## Gate Impact

If accepted, ADR-0007 becomes the architecture input for P1-GATE-003 Thread/Task Primitive Skeleton.

If rejected, no thread/task implementation work may proceed and downstream async gates remain blocked.
