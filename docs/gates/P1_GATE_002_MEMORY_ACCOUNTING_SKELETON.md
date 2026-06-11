# P1-GATE-002: Memory Accounting Skeleton

Status: Approved
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 博丽灵梦 with 八云紫
Reviewers: 红美铃, 雾雨魔理沙 when implementation exists
Depends on: ADR-0006
Related decisions: ADR-0001, ADR-0002, ADR-0005
Source baseline: Phase 1 through `1eb7e21`

Gate decision: `APPROVED_FOR_FIRST_SLICE` after ADR-0006 acceptance,
performance/cost review, public vocabulary review, and implementation-review
baseline for task #14. Code/semantic review closure remains tracked separately
in the Phase 1 queue.

## Public Vocabulary Closure

The P1 first-slice public vocabulary is frozen for upper-gate references unless
this gate is amended:

- `MemoryAccountingStatus`
- `MemoryAccountingResult`
- `MemoryBudgetClass`
- `MemoryOwnerId`
- `MemoryTag`
- `MemoryAllocationId`
- `MemorySnapshot`
- `IMemoryTracker`
- `DisabledMemoryTracker`
- `CountingMemoryTracker`

This closure does not approve production allocators, global heap interception,
or CRT/STL/general heap coverage.

## Layer

L1-L2:

- L1 low-level runtime service vocabulary for memory accounting.
- L2 integration only where host/kernel fixtures need accounting status.

## Owns

This gate owns the first implementation slice for:

- `YuMemory` CMake target;
- memory budget class vocabulary;
- owner/tag value types;
- disabled memory tracker;
- counting/test memory tracker;
- deterministic memory snapshots;
- memory accounting tests;
- replacement of P1-GATE-001's string-only allocation/byte deferral with an explicit accounting hook or status, if implementation scope allows.

## Does Not Own

This gate does not own:

- production allocator family;
- arena, pool, slab, frame, or scratch allocator;
- global `operator new` / `operator delete` override;
- OS heap hooks;
- thread-local allocation stacks;
- resource cache memory ownership;
- RHI/audio/script/world/UI allocation policy;
- profiler UI, JSON report, dashboard, capture, or oracle output;
- game adapter behavior.

## UE/Unity Analogue

UE5 references:

- LowLevelMemTracker as a responsibility reference for tags and snapshots.
- Memory allocators as later implementation references, not copied API shape.

Unity references:

- Profiler/memory categories as observability references only.

YuEngine decision:

- Start with explicit accounting vocabulary and deterministic fixtures.
- Do not copy mature engine allocator stacks before ownership and gate tests exist.

## Lifecycle

First slice lifecycle:

1. Test or host setup creates a disabled or counting memory tracker.
2. Setup registers stable memory owners/tags for the fixture.
3. Fixture records tracked allocations and frees through the memory API.
4. Fixture snapshots allocation count, free count, peak bytes, retained bytes, and leak status.
5. Fixture shuts down and verifies retained bytes return to the expected value.

Failure behavior:

- unmatched free returns explicit error;
- owner/tag mismatch returns explicit error if the implementation tracks ownership;
- retained bytes after teardown is a leak failure;
- hot-path tracked allocation over a zero budget is a performance failure;
- diagnostics/logging absence must not hide accounting failure.

## Inputs

- fixed memory owner IDs;
- fixed memory tags;
- budget class values;
- explicit allocation/free calls from deterministic fixtures;
- optional host/platform accounting hook from P1-GATE-001 follow-up.

## Outputs

- memory snapshot;
- explicit accounting status;
- test-visible allocation/free/peak/retained/leak signals;
- no JSON report output as runtime API.

## Dependencies

Allowed dependencies:

- C++ standard library;
- CMake/CTest tooling;
- existing `YuDiagnostics` only in tests if needed to prove diagnostics independence;
- existing `YuPlatform` or `YuKernel` only for a narrow accounting-status integration test.

Target dependency expectation:

```text
YuMemory
  -> no runtime YuEngine target in the first slice
```

If host/platform integration is included, `YuPlatform` may depend on `YuMemory` only for the approved accounting status/hook. `YuMemory` must not depend on `YuPlatform`, `YuKernel`, `YuDiagnostics`, tools, reports, or original-game evidence.

Forbidden dependencies:

- TouhouNewWorld `bin` or `resource`;
- old `YuEngine_BACKUP` runtime files;
- old `FrameRuntime.*`;
- renderer/audio/input/resource/script/world/UI modules;
- diagnostics report schemas;
- profiler dashboards;
- thread/task queues.

## Performance Constraints

Required deterministic signals:

- allocation count;
- free count;
- peak bytes;
- retained bytes after teardown;
- leak status;
- hot-path tracked allocation count;
- event buffer accepted/dropped count if an event buffer exists.

Blocking conditions:

- memory accounting allocates in the measured hot path;
- accounting correctness depends on logs or reports;
- disabled tracker changes host/kernel behavior;
- implementation claims global heap coverage without an approved hook;
- a production allocator family is introduced before a later allocator gate;
- hot-path allocation is described as "small" without a zero/explicit budget fixture.

## Tests

Fast gate tests required before the slice can be considered complete:

- `Memory_TrackerCountsAllocationAndFree`
- `Memory_TrackerReportsPeakAndRetainedBytes`
- `Memory_TrackerReportsLeakOnUnreleasedBytes`
- `Memory_TrackerRejectsUnmatchedFree`
- `Memory_DisabledTracker_DoesNotChangeBehavior`
- `Memory_HotPathBudget_FailsOnTrackedAllocation`

If the implementation replaces P1-GATE-001's deferral marker in host/platform code:

- `Platform_AllocationAccountingStatus_UsesMemoryHook`

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
src/yuengine/memory/include/yuengine/memory/
src/yuengine/memory/src/
tests/memory/
```

It may update:

- root `CMakeLists.txt` only to add `YuMemory` and memory tests;
- existing host/platform performance signal types only to replace the `DEFERRED_TO_P1_GATE_002` marker with an approved memory accounting status or hook.

It may not create placeholder directories or targets for thread, file, RHI, audio, input, resource, script, world, UI, tools, or game adapter work.

## Non-Goals

- No full memory allocator.
- No global heap interception.
- No arena/pool/frame allocator.
- No per-thread scratch allocation.
- No resource cache.
- No diagnostics report schema.
- No profiler UI.
- No original-game memory reconstruction.

## Evidence Inputs

None required.

Memory accounting is a general lower-layer engine concern. TouhouNewWorld evidence must not shape this API.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- ADR-0006 is accepted;
- 红美铃 confirms the proposal satisfies module-entry gate requirements;
- 博丽灵梦 confirms the performance signals and `0GC` interpretation are sufficient;
- 雾雨魔理沙 has a code-review baseline for enforcing ownership, tests, and hidden allocation risks.

If those conditions are not met, return `NEEDS_ARCHITECTURE` or `NEEDS_PERFORMANCE` with exact missing fields.
