# P3-GATE-001: Object Identity And Lifetime Registry

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 红美铃, 八云蓝, 博丽灵梦, 大妖精, 射命丸文 if evidence boundary is questioned, 雾雨魔理沙 when implementation exists
Depends on: ADR-0014
Related decisions: ADR-0002, ADR-0003, ADR-0005, ADR-0006, ADR-0009
Source baseline: Phase 3 through `5066376`

## Layer

L2-L4 core runtime object identity boundary.

This gate proves bounded synthetic object identity and lifetime behavior without
components, world/scene, script binding, Resource mutation, serialization
payloads, UI, gameplay, tools, reports, or Game Adapter behavior.

## Pre-Approval Blockers

P3-GATE-001 must not be approved for implementation until:

- ADR-0014 is accepted;
- task #14 `YuMemory` code/semantic review does not remove the accounting
  vocabulary required by this gate, or this gate is amended first;
- PM confirms sequencing against active Phase 1 and Phase 2 implementation
  reviews.

These blockers do not prevent architecture review. They prevent
`APPROVED_FOR_FIRST_SLICE`.

## Owns

This gate owns the first `YuObject` implementation slice for:

- object type ID values;
- generation-checked object handles;
- bounded synthetic object registry;
- object create and validate behavior;
- acquire/release reference counting;
- destroy and stale-handle behavior;
- object status/result values;
- deterministic registry snapshots and counters for tests.

## Does Not Own

This gate does not own:

- component model;
- transform, parent/child hierarchy, or scene graph;
- world/scene lifecycle or update phases;
- script VM, script object model, or native bridge;
- serialization payload format or reflection system;
- Resource registry mutation, package load, File reads, or asset decoding;
- render/audio/input/UI/gameplay behavior;
- report/profiler/capture/oracle/dashboard/tool output;
- TouhouNewWorld Game Adapter behavior.

## UE/Unity Analogue

UE5 references:

- CoreUObject object identity, object handles, and lifetime responsibilities as
  responsibility references.

Unity references:

- instance identity and GameObject/component separation as responsibility
  references.

YuEngine decision:

- Start with engine-owned runtime object identity only.
- Do not copy UE UObject APIs or Unity GameObject APIs.
- Keep component, world, script, reflection, and serialization behavior outside
  the first slice.

## Lifecycle

First-slice lifecycle:

1. Setup creates an object registry with fixed capacities.
2. Setup declares fixed object type IDs.
3. Runtime tests create synthetic objects and receive generation handles.
4. Runtime tests validate, acquire, release, and destroy handles explicitly.
5. Destroyed slots may be reused only with incremented generation.
6. Snapshot exposes counts, last status, capacity, accepted, rejected, and
   accounting signals.

Failure behavior:

- invalid object type returns explicit status and does not mutate state;
- registry capacity overflow returns explicit capacity status and does not
  mutate state;
- validate, acquire, release, or destroy using invalid, stale-generation, or
  already-destroyed handles returns explicit status and does not mutate
  lifecycle or reference counters;
- repeated acquire increments `uint32_t` reference count;
- acquire at `UINT32_MAX` returns explicit reference-count-overflow status and
  does not mutate reference count;
- release at zero returns explicit not-acquired status and does not mutate state;
- destroy with outstanding references returns explicit still-referenced status;
- successful destroy increments generation and invalidates old handles;
- disabled diagnostics/logging does not change object results.

## Inputs

- fixed object type IDs;
- fixed registry capacity;
- synthetic object descriptors only;
- optional setup-only synthetic initial reference count for overflow fixtures;
- optional memory tracker if P1-GATE-002 implementation vocabulary is accepted.

## Outputs

- object handles;
- object status/result values;
- object registry snapshot;
- created, alive, referenced, destroyed, and failed-operation counts;
- last explicit status;
- allocation/accounting status using `YuMemory` vocabulary or explicit deferral;
- no log/report text as behavior transport.

## Dependencies

Allowed dependencies:

- C++ standard library;
- CMake/CTest tooling;
- `YuMemory` for accounting vocabulary/signal tests when accepted;
- `YuDiagnostics` only for disabled-behavior observation when available.

Target dependency expectation:

```text
YuObject
  -> YuMemory for accounting vocabulary/signal tests, or explicit deferral
  -> optional YuDiagnostics for disabled-behavior observation
```

`YuObject` must not depend on `YuKernel`, `YuPlatform`, `YuThread`, `YuFile`,
`YuPackage`, `YuResource`, RHI, audio, input, script, scene/world, UI, tools,
reports, or TouhouNewWorld evidence in this first slice.

## Performance Constraints

Required deterministic signals:

- object capacity;
- object type capacity;
- alive object count;
- referenced object count;
- destroyed object count;
- failed operation count;
- last status;
- allocation/accounting status using `YuMemory` vocabulary or explicit
  deferral;
- disabled diagnostics/logging behavior equivalence.

First-slice bounds:

- object capacity: 64 objects maximum;
- object type capacity: 16 types maximum;
- object type ID: `uint32_t`;
- object slot/index: `uint32_t`;
- handle generation: `uint32_t`;
- reference count: `uint32_t`;
- all registry storage capacity is fixed at setup and may not grow dynamically
  in measured fixtures.

Overflow fixture rule:

- `Object_AcquireRejectsReferenceCountOverflow` may use a setup-only synthetic
  descriptor field to seed an object's reference count to `UINT32_MAX`;
- the seed field is not a runtime API and must not be callable after object
  creation;
- the overflow test must prove acquire returns explicit overflow status without
  changing reference count or lifecycle counters.

Pass/fail rule:

- exceeding object, type, or reference-count bounds is an explicit failure;
- create, validate, acquire, release, and destroy must not allocate or grow
  storage in measured fixtures;
- changing lifecycle, reference, failure, or accounting counts outside the
  declared fixture is a gate failure unless this gate is amended.

Blocking conditions:

- raw pointer or string as primary object identity;
- global mutable object cache without capacity;
- component, world, scene, script, UI, gameplay, or Game Adapter ownership;
- Resource handles standing in for object handles;
- hidden allocation in measured handle lifecycle paths;
- diagnostics/log/report output required for behavior;
- tests that validate behavior by parsing logs, reports, old runtime files, or
  original-game evidence.

## Tests

Fast gate tests required before the slice can be considered complete:

- `Object_CreateSyntheticObject_ReturnsGenerationHandle`
- `Object_CreateRejectsInvalidTypeWithoutMutation`
- `Object_RegistryCapacityOverflow_DoesNotMutate`
- `Object_ValidateRejectsInvalidOrStaleHandle`
- `Object_InvalidOrStaleHandleOperations_ReturnExplicitStatusWithoutMutation`
- `Object_DestroyIncrementsGenerationAndInvalidatesOldHandle`
- `Object_ReusesFreedSlotWithNewGeneration`
- `Object_AcquireRelease_TracksReferenceCount`
- `Object_RepeatedAcquire_IncrementsReferenceCount`
- `Object_AcquireRejectsReferenceCountOverflow`
- `Object_ReleaseAtZero_DoesNotMutate`
- `Object_DestroyRejectsOutstandingReference`
- `Object_RegistrySnapshot_ReportsCountsAndLastStatus`
- `Object_DisabledDiagnosticsDoesNotChangeResults`
- `Object_NoWorldScriptResourceOrGameAdapterDependency`
- `Object_NoHiddenAllocation_UsesYuMemorySignal`

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
src/yuengine/object/include/yuengine/object/
src/yuengine/object/src/
tests/object/
```

It may update root `CMakeLists.txt` only to add `YuObject` and
`YuObjectTests`.

It may not create placeholder directories or targets for components, world,
scene, script, serialization, reflection, UI, gameplay, tools, report, profiler,
capture, oracle, or Game Adapter work.

## Non-Goals

- No component model.
- No scene graph, transform hierarchy, or world lifecycle.
- No script object binding or native bridge.
- No serialization payload or reflection metadata.
- No Resource registry mutation.
- No File/VFS or package reads.
- No render/audio/input/UI/gameplay behavior.
- No report/profiler/oracle/tool output.
- No Game Adapter behavior.

## Evidence Inputs

No original-game evidence is required for the first slice.

TouhouNewWorld actors, scripts, saves, scenes, object names, old backup runtime
files, and old reports remain future validation evidence only. They must not be
read by P3-GATE-001 fast tests.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- ADR-0014 is accepted;
- task #14 code/semantic review closes without removing required `YuMemory`
  vocabulary, or this gate is amended to avoid unstable vocabulary;
- 红美铃 confirms module-entry fit and sequencing against active review work;
- 八云蓝 confirms the mature-engine object identity responsibility comparison is
  sound;
- 博丽灵梦 confirms the object handle/lifetime cost model and no-allocation
  lifecycle path;
- 大妖精 confirms the public surface and tests are locally implementable;
- 射命丸文 confirms original-game evidence is not being used as API shape if the
  evidence boundary is questioned.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_PERFORMANCE`, or `NEEDS_EVIDENCE` with exact missing fields.
