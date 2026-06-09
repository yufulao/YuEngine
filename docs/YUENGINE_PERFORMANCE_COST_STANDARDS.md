# YuEngine Performance Cost Standards

Status: Phase 1 implementation-slice standard
Owner: Performance architecture
Started: 2026-06-10
Applies to: all YuEngine implementation slices after gate approval

## 1. Purpose

YuEngine uses `0GC` as a shorthand target for hot runtime paths: no hidden heap
allocation, no unbounded dynamic growth, and no diagnostics/report work that can
change frame, callback, or job behavior. C++20 does not have managed garbage
collection in the planned runtime shape, so this standard treats `0GC` as a
stricter engineering posture: every runtime allocation is either outside the hot
path, preallocated with a declared owner, or measured by a gate fixture.

This document complements:

- `docs/YUENGINE_PERFORMANCE_GATES.md`
- `docs/YUENGINE_MODULE_ENTRY_GATES.md`
- `docs/adr/ADR_0004_LOGGING_DIAGNOSTICS_BOUNDARY.md`
- project code rules in `AGENTS.md`

It does not approve new modules or broaden any gate. A slice may implement only
the scope already approved by its module gate.

## 2. Cost Classes

Every implementation review must classify work into these classes.

| Class | Meaning | Default rule |
| --- | --- | --- |
| Setup | Process startup, module construction, dependency registration, test harness setup. | Allocation is allowed only with owner, lifetime, and measurement hook or explicit deferral. |
| Load | File reads, parsing, resource discovery, decode, upload staging, cache fill. | Allocation must be budgeted by fixture size and bounded queue/cache policy. |
| Frame | Main tick, module update, input snapshot, world/render/audio handoff, per-frame diagnostics. | No heap allocation, no capacity growth, no string formatting, no report generation by default. |
| Callback | Audio callback, platform callback, backend callback, or other externally timed path. | No heap allocation, no blocking IO, no locks without an approved bounded policy. |
| Job | Worker job body and scheduler hot loop. | No hidden allocation, no unbounded queue growth, deterministic shutdown and drain behavior. |
| Tool | Offline import/export, reports, oracle, capture analysis, editor/dashboard work. | Must stay outside runtime behavior ownership. |

If a reviewer cannot identify the class, the slice is `NEEDS_PERFORMANCE`.

## 3. Universal Rules

- Hot paths are `Frame`, `Callback`, and scheduler `Job` execution unless a gate
  explicitly narrows the path.
- Hot paths must not allocate from the general heap.
- Hot paths must not grow containers, rehash maps, resize strings, build reports,
  format log strings, perform file IO, or discover services by string.
- Setup and load paths may allocate only when the owner, lifetime, maximum size,
  and teardown behavior are declared.
- Runtime behavior must not depend on logs, reports, capture, oracle, or
  diagnostics output.
- Error transport uses explicit result/status types. Logs may mirror errors, but
  tests must not need log parsing to observe behavior.
- Any rule exception must name the gate, owner, budget, measurement fixture, and
  removal or containment plan.

## 4. Memory Ownership

Each module proposal and implementation slice must state:

- allocation owner and lifetime;
- budget class touched by the allocation;
- expected peak bytes and retained bytes for the fixture;
- teardown rule and leak signal;
- whether allocator accounting is implemented, hooked, or explicitly deferred;
- whether the allocation can occur during `Frame`, `Callback`, or `Job` paths.

Blocked patterns:

- raw `new` or `delete` in hot paths;
- hidden allocation through standard-library objects in hot paths;
- global mutable caches without capacity, owner, and invalidation policy;
- allocation hidden behind diagnostics or report adapters;
- ownership transferred through unclear raw pointer lifetime.

Allowed patterns when scoped:

- setup-time allocation with fixed owner and teardown;
- load-time allocation bounded by fixture and cache policy;
- stack storage for small fixed-size temporary data;
- preallocated arenas, pools, ring buffers, or fixed-capacity containers after
  their allocator policy gate is approved.

## 5. Dynamic Containers

Dynamic containers are allowed only when capacity behavior is explicit.

Hot-path rules:

- `std::vector`, `std::string`, `std::unordered_map`, `std::map`, and similar
  containers must not grow in hot paths.
- `reserve`, fixed capacity, or precomputed storage must happen before the hot
  path starts.
- `push_back`, `emplace_back`, insertion, and map lookup are acceptable in hot
  paths only when the review proves they cannot allocate, rehash, or grow.
- `std::unordered_map` hot-path use requires a stable reserve/load-factor policy
  and no insertion after setup.
- Per-frame scratch storage must have a reset policy that does not free and
  reallocate each frame.

Review signal:

- tests or instrumentation must capture capacity before and after the fixture for
  any container used in a hot path;
- a capacity change in a hot fixture is a performance failure unless explicitly
  approved by gate.

## 6. Strings And Formatting

Strings are setup, tools, or diagnostics data by default. They are not hot-path
control data.

Hot-path rules:

- prefer stable IDs, enum values, handles, interned names, or `std::string_view`
  over owning strings;
- do not concatenate, format, stream, allocate, lowercase, normalize, or parse
  strings in hot paths;
- do not use path normalization or file-system string operations in frame,
  callback, or job hot loops;
- log messages may use literals, but hot-path logs must be category-gated before
  any formatting or string construction;
- dynamic string creation for reports is L7 tool work, not runtime behavior.

Blocked patterns:

- `std::stringstream`, `std::format`, repeated `operator+`, or owned
  `std::string` construction in hot paths;
- string keys for per-frame service lookup, component dispatch, script/native
  calls, resource handles, or render submission;
- tests that validate runtime behavior by parsing log strings.

## 7. Closure And Lambda Capture

Lambdas and callbacks are allowed only when capture cost and lifetime are
visible. Captures can hide copies, references, type erasure, and heap allocation.

Rules:

- no default capture (`[=]` or `[&]`) in hot-path callbacks;
- captures must be explicit and small;
- do not capture owning containers, strings, large structs, or service objects by
  value in hot paths;
- do not store capturing lambdas in `std::function` on a hot path unless the gate
  proves no heap allocation and stable lifetime;
- prefer function pointers, explicit small callable types, template callables, or
  prebound setup-time callbacks when dispatch cost matters;
- callback lifetime must be tied to module lifecycle and deterministic shutdown.

Review signal:

- every persistent callback must state owner, lifetime, capture list, execution
  thread, and budget class;
- hot-path callbacks must have a fixture proving registration does not allocate
  during execution.

## 8. Dispatch And Service Lookup

Runtime dispatch must be predictable.

Rules:

- string/name lookup is setup-path only unless a gate explicitly approves and
  measures it;
- hot paths use cached handles, stable IDs, pointers, indices, or prebound call
  sites;
- multi-type dispatch follows registered tables or O(1) lookup, aligned with
  `AGENTS.md`;
- service registry misses return explicit errors and do not fall back to logs;
- script/native bridge hot paths must not use stringly dynamic dispatch.

Blocked patterns:

- per-frame service lookup by string;
- repeated map lookup where an initialization-time handle can be cached;
- reflection-like dispatch in lower layers before a cost model exists.

## 9. Logging And Diagnostics

Logging and diagnostics observe runtime behavior; they do not own it.

P1 logging rules:

- disabled logging must not change host/kernel results;
- disabled logging must have bounded cost and no report dependency;
- bounded in-memory sinks must declare capacity and overflow behavior;
- logging output is never the test oracle for runtime behavior.

Future diagnostics rules:

- enabled versus disabled overhead must be measured;
- queues must have capacity, backpressure, drop, or flush policy;
- trace/counter hot paths must avoid allocation;
- category filtering must occur before string formatting or payload building;
- report serialization remains L7 tool/adapter work.

Blocked patterns:

- JSON report objects in runtime APIs;
- profiler scopes or counters that allocate in hot paths without an approved
  diagnostics gate;
- capture/oracle APIs that alter frame behavior when disabled.

## 10. Per-Path Rules

### Frame Path

- no heap allocation;
- no capacity growth;
- no string formatting or report building;
- no service lookup by string;
- no file IO;
- no unbounded diagnostics emission.

### Callback Path

- no heap allocation;
- no blocking IO;
- no unbounded locks or waits;
- no container growth;
- no diagnostics formatting;
- no ownership transfer.

### Job Path

- no hidden allocation in the job body;
- no unbounded queue growth;
- job inputs are owned or stable for the job lifetime;
- cancellation/drain behavior is deterministic;
- logging is bounded and optional.

### Load Path

- allocation is allowed only with fixture-size budget;
- caches declare capacity, invalidation, and teardown;
- upload staging declares max in-flight requests and buffer lifetime;
- load path cannot smuggle behavior into lower layers from original-game facts.

## 11. Measurement Fixtures

Every implementation slice that touches runtime cost must include at least one
deterministic signal.

Accepted signal types:

- allocation count;
- peak bytes;
- retained bytes;
- leak count;
- container capacity before/after hot fixture;
- log events accepted/dropped by bounded sink;
- queue depth;
- fixed tick count;
- callback invocation count;
- lifecycle order and count;
- enabled/disabled diagnostics cost comparison when diagnostics is in scope.

Minimum pass/fail shape:

```text
Given fixed fixture input
When the approved runtime path executes
Then allocation count, capacity growth, queue depth, or diagnostics overhead stays within the declared budget
And runtime behavior is unchanged when diagnostics/logging is disabled
```

If allocator accounting is not part of the approved slice, the implementation
must expose a hook or mark the rule as `DEFERRED_TO_<gate id>` with the owning
future gate.

## 12. Review Checklist

Performance review must block the slice when any answer is missing:

- Which cost class does this code execute under?
- Can this code allocate, grow capacity, format strings, rehash, or block?
- Which owner releases the memory or drains the queue?
- What is the maximum fixture size, queue depth, or retained byte count?
- Does disabled logging/diagnostics change behavior?
- Are callback captures explicit, small, and lifetime-safe?
- Are service lookups cached before hot paths?
- Which deterministic test proves the budget?

Code review should treat the following as performance blockers:

- hot-path `new`, `delete`, `std::make_unique`, or `std::make_shared`;
- hot-path dynamic string construction or formatting;
- hot-path container growth or map insertion without reserve proof;
- `std::function` storage of capturing lambdas in hot paths without proof;
- default lambda captures in hot callbacks;
- diagnostics/report/capture/oracle ownership inside runtime behavior;
- uncached string lookup in frame/update/callback/job paths;
- unbounded in-memory logs, queues, caches, or scratch buffers.

## 13. Gate Outcomes

Use `NEEDS_PERFORMANCE` when:

- the slice cannot classify its paths;
- the fixture lacks a deterministic budget signal;
- allocation or growth is described as "small" without proof;
- diagnostics behavior is not defined when disabled;
- callback capture or lifetime is unclear;
- a hot path relies on string lookup, report objects, or unbounded containers.

Use `APPROVE_PERFORMANCE` only when:

- hot paths are identified;
- allocation/growth policy is explicit;
- measurement fixture and pass/fail rule are named;
- diagnostics/logging disabled behavior is behavior-neutral;
- code review can enforce the rules locally.

## 14. Immediate Application

For P1-GATE-001 and adjacent first-slice work:

- allocation and byte accounting may be exposed as hooks or explicitly
  `DEFERRED_TO_P1_GATE_002`;
- disabled logging must be behavior-neutral and bounded;
- lifecycle tests must record order and counts;
- service lookup strings are setup-only;
- fixed tick and monotonic timer behavior are required;
- no memory allocator, thread/task queue, file/VFS, report, capture/oracle,
  renderer, audio, input, UI, script, world, or game adapter work is approved by
  this standard.
