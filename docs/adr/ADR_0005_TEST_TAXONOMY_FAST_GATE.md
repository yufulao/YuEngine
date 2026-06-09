# ADR-0005: Test Taxonomy And Fast Gate Command

Status: Proposed
Owner: 八云紫, 总架构师
Reviewers: 红美铃, 雾雨魔理沙, 博丽灵梦 for performance signals
Date: 2026-06-10
Depends on: ADR-0001, ADR-0002, docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md

## Context

YuEngine must avoid the old pattern where contract/report count becomes a false progress metric. Tests must validate module-owned behavior through public interfaces, with fast deterministic gates separated from long evidence/oracle suites.

P1-GATE-001 needs a concrete test taxonomy and command model for the first headless host and kernel bootstrap implementation.

## Decision

YuEngine uses four test categories:

```text
Fast Gate Tests
Module Fixture Tests
Performance Smoke Tests
Evidence/Oracle Suites
```

Only Fast Gate Tests are required for the first implementation slice. Module Fixture and Performance Smoke behavior can be included when small and deterministic. Evidence/Oracle suites are explicitly excluded from P1-GATE-001.

## Fast Gate Tests

Purpose:

- Validate public behavior of approved modules.
- Run non-interactively on an agent machine.
- Stay small, deterministic, and independent from TouhouNewWorld assets.

Initial command family:

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

If using a multi-config generator, the implementation handoff must include config-specific commands, for example:

```text
cmake --build build --config Debug
ctest --test-dir build --build-config Debug --output-on-failure
```

P1-GATE-001 must document the exact command used.

When a repository preset exists, the preset is the preferred fast gate command.
For the current P1 implementation shape, the expected command family is:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate
```

Any replacement preset must preserve non-interactive configure, build, and test
behavior and must state the generator/configuration it uses.

## Module Fixture Tests

Purpose:

- Validate one module boundary with controlled inputs.
- Avoid private implementation reach-through.
- Avoid original-game data unless a future evidence gate approves fixture use.

Rules:

- Fixture tests live under `tests/<module>` or `tests/fixtures/<module>`.
- Tests include public module headers only.
- Tests may use local test doubles only for external boundaries such as time or platform APIs.
- Tests must not mock internal YuEngine modules that are part of the approved slice.

## Performance Smoke Tests

Purpose:

- Provide deterministic performance signals required by `docs/YUENGINE_PERFORMANCE_GATES.md` and `docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md`.

P1-GATE-001 required signals:

- fixed tick count;
- monotonic timer assertion;
- lifecycle order/counts;
- disabled logging behavior and bounded/no report dependency;
- service lookup rule, including setup-path string lookup and future hot-path cached-handle policy;
- allocation/bytes hooks or explicit `DEFERRED_TO_P1_GATE_002`.

Performance smoke tests are not benchmark claims. They are guardrails that make cost and measurement points visible.

They must classify the path being exercised as Setup, Load, Frame, Callback,
Job, or Tool when the performance cost standard applies. If allocator accounting
is deferred, the test or handoff must name the owning future gate instead of
treating the cost as unmeasured success.

## Evidence/Oracle Suites

Purpose:

- Validate original-game facts, frame capture, adapter behavior, or long-running reconstruction evidence.

Rules:

- Excluded from fast gate.
- Excluded from P1-GATE-001.
- Must never define runtime ownership.
- Must run only after the relevant lower-layer module gates pass.
- Must not be counted as engine completion by volume.

## Test Naming

Test names should express behavior:

- `<Module>_<Behavior>_<ExpectedResult>`

Examples:

- `Host_StartTickShutdown_Deterministic`
- `Host_TimerMonotonic_ForFixedTicks`
- `Logging_DisabledSink_DoesNotChangeBehavior`
- `Kernel_ModuleLifecycle_DependencyOrder`
- `Kernel_ModuleStartupFailure_TearsDownStartedModules`
- `Kernel_ServiceRegistry_ResolveAndMissingService`

## Test Result Rule

A test passes only if public behavior matches expectation.

Not valid pass conditions:

- report file exists;
- JSON schema serializes;
- old evidence count increased;
- visual demo opens;
- placeholder object was created;
- implementation private state was inspected directly.

Reports may be outputs of tests, but report existence is not the behavior under test.

## P1-GATE-001 Test Requirements

The first implementation task must provide:

- non-interactive configure/build/test commands;
- CTest registration for fast tests;
- host start/tick/shutdown test;
- monotonic fixed tick timer test;
- disabled logging behavior test;
- module dependency-order lifecycle test;
- startup-failure teardown test;
- service resolve/missing-service test.

If ADR-0003 is accepted or used by implementation:

- required service preflight test;
- partial-start service cleanup test;
- update-failure dependent-stop test;
- service deregistration after stop/failure test.

If a test is deferred because the approved slice is smaller, the implementation handoff must name the deferral and the gate that will own it.

For P1-GATE-001, `DEFERRED_TO_P1_GATE_002` is valid only for real allocator and
byte accounting. It does not defer disabled logging behavior, lifecycle order,
service lookup policy, or fixed tick determinism.

## CI And Local Agent Use

Phase 1 does not require remote CI before the first code slice lands, but all commands must be suitable for future CI:

- no interactive prompts;
- no absolute machine-specific paths in build scripts;
- build outputs under ignored build directories;
- deterministic test data;
- clear failure output.

The exact command used for review must be recorded in the implementation
handoff. A human or agent reviewer must be able to run the same command without
interactive setup or private machine state beyond the documented toolchain.

## Non-Goals

This ADR does not decide:

- full CI provider;
- coverage tooling;
- fuzz testing;
- frame oracle;
- asset corpus management;
- performance benchmark thresholds.

Those require later ADRs and gates.

## Gate Impact

If accepted, this ADR becomes the test taxonomy for P1-GATE-001 and later module gate proposals.

If rejected, P1-GATE-001 must still provide the tests named in its approved gate, but broader test taxonomy remains unresolved before the next implementation slice.
