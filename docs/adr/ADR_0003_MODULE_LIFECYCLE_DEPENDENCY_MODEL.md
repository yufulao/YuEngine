# ADR-0003: Module Lifecycle And Dependency Declaration Model

Status: Proposed
Owner: 八云紫, 总架构师
Reviewers: 红美铃, 博丽灵梦, 雾雨魔理沙 when code review starts
Date: 2026-06-10
Depends on: ADR-0001, ADR-0002

## Context

P1-GATE-001 authorizes the first headless host and kernel bootstrap slice. That slice includes minimal module lifecycle and service registry behavior. Before the kernel grows beyond that slice, YuEngine needs an explicit lifecycle and dependency model.

The model must prevent these old failure modes:

- one central runtime object owning unrelated behavior;
- implicit initialization order;
- global mutable business state;
- silent missing services;
- diagnostics/report output shaping runtime APIs;
- string lookups being treated as hot-path safe without a cached handle rule.

## Decision

YuEngine modules are kernel-owned runtime units with explicit identity, dependencies, lifecycle callbacks, and service publication.

The initial lifecycle states are:

```text
Declared
-> Constructed
-> Starting
-> Started
-> Updating
-> Stopping
-> Stopped
-> Failed
```

Allowed transitions:

- `Declared -> Constructed`
- `Constructed -> Starting`
- `Starting -> Started`
- `Started -> Updating`
- `Updating -> Started`
- `Started -> Stopping`
- `Stopping -> Stopped`
- any startup/update/shutdown failure path -> `Failed`

The kernel owns lifecycle transitions. A module must not transition itself.

## Module Descriptor

Each module has a descriptor:

```text
ModuleId
DisplayName
Layer
RequiredDependencies
OptionalDependencies
PublishedServices
LifecycleCallbacks
```

Rules:

- `ModuleId` is stable and unique.
- Required dependencies must be present before startup.
- Optional dependencies must be queried explicitly and cannot silently alter ownership.
- Published services must be declared before startup.
- Descriptor creation is setup-path only.

## Dependency Order

Startup order is topologically sorted from required dependencies.

Shutdown order is deterministic reverse startup order for modules that reached `Started` or beyond.

Failure behavior:

- Missing required dependency blocks startup before callbacks run.
- Cycle in required dependencies blocks startup.
- Failure during module startup triggers teardown of already-started modules in reverse order.
- Failure during shutdown is recorded and returned as explicit result.
- No failure path may silently continue as success.

## Lifecycle Callbacks

Initial callbacks:

```text
OnStart(context)
OnUpdate(context)
OnStop(context)
```

Callback rules:

- `OnStart` may publish services declared in the descriptor.
- `OnUpdate` must not perform setup-path string service lookups repeatedly.
- `OnStop` releases module-owned runtime resources.
- Callbacks return explicit success/failure.
- Exceptions or platform-specific faults must be translated to explicit failure at the host/kernel boundary.

## Service Registry

Service registration:

- Services are registered by stable service ID during startup.
- Service IDs are setup-path names, not hot-path dynamic dispatch.
- A missing required service is an explicit error.
- Duplicate service registration is an explicit error unless replacement semantics are later approved by ADR.

Service lookup:

- Setup/configuration code may look up by service ID.
- Hot/update paths must use cached stable handles, pointers, or another later-approved bounded mechanism.
- Uncached string/name lookup is not hot-path safe.

## Diagnostics Boundary

The kernel may emit diagnostics events for:

- module state transitions;
- dependency order;
- startup failure;
- shutdown failure;
- missing service;
- duplicate service.

Diagnostics cannot own lifecycle behavior. Disabling diagnostics must not change lifecycle results.

No JSON report schema is a kernel API.

## Performance Requirements

The first measurable signals are:

- module count;
- dependency edge count;
- startup order count;
- update count;
- shutdown order count;
- service registration count;
- service lookup count in tests;
- failure teardown count.

Performance rules:

- Dependency sorting is setup-path.
- Descriptor and service ID string work is setup-path.
- Hot/update paths must use stable cached access.
- No unbounded background queue is introduced by this ADR.
- Allocation/bytes accounting may be `DEFERRED_TO_P1_GATE_002` until the Memory gate exists.

## Test Requirements

Kernel lifecycle tests must cover:

- dependency-order startup;
- reverse-order shutdown;
- missing required dependency;
- dependency cycle;
- startup failure teardown of already-started modules;
- shutdown failure reporting;
- service registration and resolution;
- missing service returns explicit error;
- duplicate service registration returns explicit error;
- diagnostics-disabled behavior equivalence for lifecycle result.

## P1-GATE-001 Compatibility

P1-GATE-001 may implement a minimal subset:

- fixed test module descriptors;
- required dependency sorting;
- `OnStart`, `OnUpdate`, `OnStop`;
- service registration/resolution;
- explicit failure results;
- lifecycle trace used by tests.

It does not need:

- dynamic plugin loading;
- module hot reload;
- optional dependency policies beyond explicit query behavior;
- async start/stop;
- thread/task scheduling;
- allocator implementation;
- reflection or serialization.

## Non-Goals

This ADR does not decide:

- plugin binary loading;
- editor module model;
- game adapter module model;
- task scheduler;
- memory allocator;
- dependency injection framework beyond the minimal service registry;
- scripting/native service ABI.

## Consequences

Positive:

- Kernel behavior remains deterministic and testable.
- Modules cannot silently control lifecycle ownership.
- Service lookup rules protect future hot paths from stringly dispatch.
- Diagnostics remain observational.

Costs:

- More structure than ad hoc startup functions.
- Implementers must write descriptors and tests before adding module behavior.
- Some later features, such as optional modules and plugins, need separate ADRs.

## Gate Impact

If accepted, this ADR becomes the lifecycle model for P1-GATE-001 and later Kernel slices.

If rejected, P1-GATE-001 may still complete its minimal approved slice, but any broader Kernel work must wait for a replacement lifecycle/dependency model.
