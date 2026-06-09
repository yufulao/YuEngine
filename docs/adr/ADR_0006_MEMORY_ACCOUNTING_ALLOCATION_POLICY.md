# ADR-0006: Memory Accounting And Allocation Policy Skeleton

Status: Accepted
Owner: 博丽灵梦 with 八云紫
Reviewers: 红美铃, 雾雨魔理沙 when implementation exists
Date: 2026-06-10
Accepted: 2026-06-10
Depends on: ADR-0001, ADR-0002, ADR-0005, docs/YUENGINE_PERFORMANCE_GATES.md, docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md

## Context

P1-GATE-001 deliberately deferred real allocation and byte accounting to `P1-GATE-002`. YuEngine now has a headless host, platform fixture, kernel lifecycle tests, logging boundary, and performance-cost standard. The next lower-layer step is to replace string-only memory deferral with a small accounting skeleton that can support later allocator work without pretending to be a complete allocator.

The memory policy must prevent these failure modes:

- claiming `0GC` or no-allocation behavior without measurement;
- hiding allocations behind diagnostics, reports, callbacks, or standard-library growth;
- introducing a global allocator override before ownership tags and fixture behavior are accepted;
- letting each subsystem invent its own memory terminology;
- treating test logs or reports as leak/accounting proof.

## Decision

YuEngine introduces a `YuMemory` module as the owner of early memory accounting vocabulary, explicit accounting hooks, and deterministic memory fixtures.

P1 does not introduce a production allocator, arena library, pool system, garbage collector, global `operator new` override, or OS heap hook. It introduces the smallest shared contract needed to count explicitly tracked allocation paths and to classify runtime cost.

The initial memory model has these concepts:

```text
MemoryBudgetClass
MemoryOwnerId
MemoryTag
MemoryAccountingEvent
MemorySnapshot
MemoryTracker
DisabledMemoryTracker
CountingMemoryTracker
```

Names may be adjusted during implementation, but the responsibilities must remain equivalent.

## Budget Classes

`YuMemory` uses the cost classes defined by `docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md`:

- Setup
- Load
- Frame
- Callback
- Job
- Tool

Rules:

- Every tracked allocation is assigned one budget class.
- Frame, Callback, and Job are hot paths by default.
- Hot-path accounting fixtures must fail if tracked allocation count or capacity growth exceeds the declared budget.
- Setup and Load allocations may be allowed only with an owner, lifetime, maximum fixture size, and teardown rule.
- Tool allocations must not become runtime behavior ownership.

## Ownership Tags

Every tracked allocation belongs to a stable owner.

Initial owner information:

```text
OwnerId
Tag
BudgetClass
Bytes
Alignment
```

Rules:

- Owner IDs are setup-path stable identifiers, not per-frame strings.
- Tags are for accounting classification and review, not behavior dispatch.
- Ownership transfer must be explicit in API shape or blocked.
- A subsystem cannot satisfy this ADR by logging owner names after the fact.
- Original-game evidence, reports, capture, or oracle output cannot own runtime memory.

## Tracker Boundary

The tracker records accounting facts; it does not allocate for runtime owners.

Required behavior:

- record allocation count;
- record free count;
- record currently retained bytes;
- record peak retained bytes;
- detect unmatched free or leak in deterministic fixtures;
- expose a snapshot through a public API.

Allowed implementations in the first slice:

- disabled tracker with behavior-neutral no-op accounting;
- counting tracker for tests;
- fixed-capacity event buffer for tests if event history is needed;
- explicit accounting calls around test-owned allocation fixtures.

Blocked implementations in the first slice:

- global `operator new` / `operator delete` replacement;
- OS heap interception;
- stack trace capture;
- async diagnostics queue;
- report or JSON output as accounting API;
- arena, pool, slab, frame allocator, scratch allocator, or full allocator family;
- per-subsystem custom allocators outside the shared `YuMemory` vocabulary;
- heap allocation from accounting callbacks in hot-path fixtures.

## API Shape Constraints

The first public API should stay narrow:

- budget class enum or equivalent stable values;
- owner/tag value types;
- allocation/free accounting entry points;
- snapshot/query entry point;
- disabled tracker;
- counting tracker or test tracker.

Do not add:

- resource cache policy;
- string interning;
- container library;
- allocator-aware runtime object model;
- thread-local allocator stacks;
- per-frame scratch allocator;
- scripting/native memory bridge;
- serialization/report schemas.

Those need later ADRs and gates.

## Integration With P1-GATE-001

P1-GATE-001 currently exposes `DEFERRED_TO_P1_GATE_002` for allocation and byte accounting. P1-GATE-002 should replace that with an explicit accounting hook or status that proves:

- allocator/byte accounting is no longer an unnamed deferral for the approved fixture;
- the fixture can report allocation count, peak bytes, retained bytes, and leak status for explicitly tracked paths;
- disabled accounting does not change host/kernel behavior;
- any still-untracked general heap path is named as a limitation rather than counted as zero.

This ADR does not require retroactive full tracking of all standard-library or CRT allocations from P1-GATE-001. It requires the deferral to become an explicit `YuMemory` boundary and testable fixture.

## Hot-Path Rules

For a path classified as Frame, Callback, or Job:

- tracked allocation budget defaults to zero;
- tracked capacity growth budget defaults to zero;
- string formatting, path normalization, report building, and service lookup by string are not memory-safe hot-path behavior;
- any exception must be named in the owning gate with fixture, budget, and owner.

Accounting code itself must not hide allocations in the hot path. If a tracker needs history, it must use setup-time fixed capacity or test-only storage outside the measured hot path.

## Diagnostics Boundary

Diagnostics may observe memory snapshots after the fact. Diagnostics must not be required for accounting correctness.

Rules:

- leak/accounting tests read memory APIs, not log strings;
- disabled logging must not alter memory accounting results;
- reports may serialize snapshots later, outside runtime ownership;
- no JSON/report schema is part of `YuMemory` runtime API.

## Test Requirements

First-slice memory tests must cover:

- allocation count increments for a tracked allocation fixture;
- free count increments and retained bytes return to zero;
- peak bytes records the highest retained byte count;
- leak/unreleased bytes are reported as failure in a fixture;
- unmatched free or owner/tag mismatch returns explicit error;
- disabled tracker is behavior-neutral and reports no retained bytes;
- budget class is recorded and queryable;
- hot-path fixture fails on tracked allocation when budget is zero;
- accounting does not require diagnostics/report/log output.

If integrated with P1-GATE-001 host/platform signals, tests must also prove:

- the old `DEFERRED_TO_P1_GATE_002` marker is replaced by a memory accounting status or snapshot;
- host start/tick/shutdown can expose a deterministic memory accounting result for explicitly tracked paths.

## Performance Requirements

Required deterministic signals:

- allocation count;
- free count;
- peak bytes;
- retained bytes after teardown;
- leak count or leak status;
- hot-path tracked allocation count;
- event buffer accepted/dropped count if an event buffer exists.

The first slice may use small deterministic fixtures. It must not claim global allocation coverage unless the implementation actually intercepts those paths under an accepted future gate.

## P1-GATE-002 Compatibility

P1-GATE-002 may implement:

- `YuMemory` target;
- public memory accounting value types;
- disabled tracker;
- counting/test tracker;
- memory tests under `tests/memory`;
- optional integration point that replaces the P1-GATE-001 string deferral with a typed accounting status.

P1-GATE-002 must not implement:

- full allocator family;
- production arena/pool/frame allocator;
- global heap override;
- thread-local allocator stack;
- resource cache;
- diagnostics/report ownership;
- game adapter memory policy.

## Non-Goals

This ADR does not decide:

- final allocator implementation;
- arena or pool layout;
- per-thread scratch storage;
- resource cache memory ownership;
- RHI/audio/script allocation APIs;
- platform-specific heap tracking;
- memory profiling UI or report format.

## Gate Impact

ADR-0006 is the architecture input for P1-GATE-002 Memory Accounting Skeleton.

Implementation watch items:

- Owner IDs and tags must stay setup-time stable values, not per-frame owning string construction.
- Any event history buffer must be fixed-capacity/setup-time or test-only storage with deterministic accepted/dropped counts.
- Untracked CRT, STL, or general heap paths must be named as limitations, not counted as zero.
