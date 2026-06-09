# ADR-0004: Logging And Diagnostics Boundary

Status: Proposed
Owner: 八云紫, 总架构师
Reviewers: 红美铃, 博丽灵梦, 雾雨魔理沙 when code review starts
Date: 2026-06-10
Depends on: ADR-0001, ADR-0002, ADR-0003

## Context

P1-GATE-001 allows a minimal `YuDiagnostics` target only for default and disabled logging sink behavior needed by the headless host and kernel bootstrap. It does not allow diagnostics channel architecture, reports, capture, oracle, profiling API, dashboards, or runtime ownership.

Old YuEngine failed partly because diagnostics/report JSON and evidence output shaped runtime APIs. This ADR defines the early boundary so Phase 1 can log and test lifecycle behavior without recreating report-driven runtime.

## Decision

YuEngine separates logging, diagnostics, reports, and verification tools into distinct responsibilities:

```text
Logging sink
  minimal L1/L2 runtime observation for messages and test visibility

Diagnostics channel
  later bounded runtime observability API for counters/events/traces

Reports
  L7 adapters/tools that serialize observations outside runtime ownership

Capture/oracle
  L7 verification tools and adapters, not runtime owners
```

P1-GATE-001 may implement only the logging sink subset.

## P1 Logging Sink Scope

Allowed in P1-GATE-001:

- default logging sink;
- disabled logging sink;
- bounded in-memory sink for tests;
- log category or level only if needed to prove disabled logging behavior;
- explicit guarantee that disabling logging does not change host/kernel results.

Blocked in P1-GATE-001:

- JSON report schema;
- diagnostics event bus;
- capture/oracle APIs;
- profiling API;
- dashboard output;
- file trace format;
- runtime decisions that depend on log output;
- global mutable diagnostics state that modules use as behavior owner.

## Dependency Rule

For P1:

```text
YuHeadless
  -> YuKernel
  -> YuPlatform
  -> YuDiagnostics
```

`YuDiagnostics` must remain leaf-like for the first slice. It must not depend on `YuKernel`, `YuPlatform`, `YuHeadless`, tools, reports, original evidence, or future runtime modules.

Modules may emit log messages to an interface, but they must not read logging output to decide behavior.

## API Shape Constraints

P1 logging API must stay intentionally small:

- sink interface;
- default sink implementation;
- disabled sink implementation;
- optional bounded test sink;
- message submit function with explicit category/level only if used by tests.

Do not add:

- structured report objects;
- JSON helpers;
- arbitrary key/value runtime report maps;
- frame report builders;
- capture metadata;
- oracle result types;
- profiler scopes;
- global singleton logger required for behavior.

## Runtime Behavior Rule

Runtime behavior must be identical with logging enabled or disabled, except for observable log output.

Tests must prove:

- disabled logging does not change host exit status;
- disabled logging does not change kernel lifecycle result;
- disabled logging does not require report schema or file output;
- bounded test sink records expected messages without owning lifecycle state.

## Performance Requirements

P1 required signal:

- disabled logging path is bounded and has no report dependency.

Later diagnostics channel work must add:

- enabled vs disabled overhead comparison;
- queue bounds if asynchronous;
- allocation count in hot path;
- category filtering cost;
- backpressure/drop policy if relevant.

Until those later gates exist, P1 logging must avoid asynchronous queues and unbounded buffering.

## Error And Failure Reporting

Errors are returned through explicit host/kernel result types. Logs may mirror those errors, but logs are not the error transport.

For example:

- missing service returns explicit error and may log;
- startup failure returns explicit error and may log;
- shutdown failure returns explicit error and may log.

If a test can only detect failure by parsing logs, the API boundary is wrong.

## Tool And Report Boundary

Reports belong outside runtime ownership.

Future report tools may consume:

- lifecycle traces exposed by tests;
- diagnostics snapshots from a later approved diagnostics channel;
- capture/oracle output from L7 verification tools.

They must not define core runtime API shape.

## P1-GATE-001 Compatibility

P1-GATE-001 may:

- define a minimal `YuDiagnostics` target;
- use it from `YuPlatform` and `YuKernel` for logging;
- test default/disabled/bounded in-memory sinks;
- document richer diagnostics as future ADR/P1-GATE-004 work.

P1-GATE-001 must not:

- create report schemas;
- implement event tracing;
- write capture/oracle code;
- add diagnostics-owned runtime state;
- use logs as behavior control flow.

## Non-Goals

This ADR does not decide:

- full diagnostics channel API;
- profiling system;
- trace file format;
- report serialization;
- frame capture/oracle;
- editor/dashboard tooling.

Those require later ADRs and gates.

## Gate Impact

If accepted, this ADR becomes the logging boundary for P1-GATE-001 and the input for future P1-GATE-004 diagnostics channel work.

If rejected, P1-GATE-001 must keep logging even smaller: a disabled sink and test-only observation only, with no broader diagnostics target behavior.
