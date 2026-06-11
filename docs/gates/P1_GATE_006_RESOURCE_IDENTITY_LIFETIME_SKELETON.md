# P1-GATE-006: Resource Identity And Lifetime Skeleton

Status: Approved
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 红美铃, 大妖精, 博丽灵梦, 射命丸文, 雾雨魔理沙 when implementation exists
Depends on: ADR-0009
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0008
Source baseline: Phase 1 through `7ce457e`

Gate decision: `APPROVED_FOR_FIRST_SLICE` after ADR-0009 acceptance,
resource-boundary review, performance/cost review, evidence-boundary review,
and implementation-review baseline for task #33. Code/semantic review closure
remains tracked separately in the Phase 1 queue.

## Public Vocabulary Closure

The P1 first-slice public Resource vocabulary is frozen for upper-gate
references unless this gate is amended:

- `ResourceTypeId`
- `ResourceLogicalKey`
- `ResourceDescriptor`
- `ResourceDependencyEdge`
- `ResourceHandle`
- `ResourceStatus`
- `ResourceRegistrationResult`
- `ResourceRegistryDesc`
- `ResourceSnapshot`

Allocation/accounting signals use `YuMemory::MemoryAccountingStatus`. This
closure does not approve File/package reads, Resource mutation by Package, asset
decoding, async loading, upload scheduling, or game-adapter behavior.

## Layer

L4 Resource identity boundary over lower-layer runtime primitives.

This gate proves resource handles and dependency lifetime without file IO,
package parsing, decoders, upload, or original-game meaning.

## Owns

This gate owns the first `YuResource` implementation slice for:

- resource type IDs;
- bounded logical resource keys;
- generation-checked resource handles;
- bounded synthetic registry;
- direct dependency-edge validation;
- acquire/release/retire behavior;
- dependency-edge lifetime and retire blocking;
- explicit resource statuses/results;
- deterministic snapshots/counters for tests.

## Does Not Own

This gate does not own:

- File/VFS reads;
- package parser or package manifest;
- TouhouNewWorld resource lookup;
- asset decoding;
- resource cache eviction;
- hot reload;
- async loading or streaming;
- GPU/audio upload scheduling;
- script, scene, UI, gameplay, or Game Adapter behavior;
- report/capture/oracle/tool output.

## Inputs

- fixed `ResourceTypeId` values;
- fixed lowercase ASCII logical keys;
- explicit registry capacity;
- explicit dependency-edge capacity;
- synthetic test descriptors only;
- YuMemory accounting vocabulary from the P1-GATE-002 implementation baseline.

## Outputs

- resource handles;
- resource result/status values;
- registry snapshot;
- registered/acquired/released/retired counts;
- dependency edge count;
- validation failure counts;
- allocation/accounting status using `YuMemory` vocabulary;
- no log/report text as behavior transport.

## Lifecycle

First-slice lifecycle:

1. Setup creates a bounded registry with fixed capacities.
2. Setup registers synthetic resource descriptors.
3. Setup validates direct dependency edges.
4. Tests acquire and release handles explicitly.
5. Tests retire resources only when reference/dependency rules allow it.
6. Snapshot exposes counts and last explicit failure status.

Failure behavior:

- duplicate resource registration returns explicit duplicate status;
- capacity overflow returns explicit capacity status and does not mutate registry state;
- missing dependency returns explicit missing-dependency status;
- self-dependency returns explicit dependency-cycle or invalid-dependency status;
- dependency cycle returns explicit dependency-cycle status;
- stale generation handle returns explicit generation-mismatch status;
- acquire with mismatched expected `ResourceTypeId` returns explicit type-mismatch status and does not mutate reference count;
- repeated acquire of a valid handle increments `uint32_t` reference count and returns success;
- acquire when reference count is already `UINT32_MAX` returns explicit reference-count-overflow status and does not mutate reference count;
- release decrements reference count, and release at zero returns explicit not-acquired status without mutation;
- retiring an acquired resource or an acquired dependency returns explicit still-referenced status;
- retiring a resource with any live inbound dependent edge returns explicit still-depended-on status and does not mutate registry state;
- successfully retiring a resource clears that resource's own outbound dependency edges;
- disabled diagnostics/logging does not change any Resource result.

## Dependencies

Allowed dependencies:

- C++ standard library;
- CMake/CTest tooling;
- `YuMemory` for accounting vocabulary/signal tests;
- `YuDiagnostics` only if P1-GATE-004 is implemented before this slice, and only for disabled-behavior observation.

Target dependency expectation:

```text
YuResource
  -> YuMemory for accounting vocabulary/signal tests
  -> optional YuDiagnostics observation after P1-GATE-004 implementation
```

`YuResource` must not depend on `YuKernel`, `YuPlatform`, `YuThread`, `YuFile`,
RHI, audio, input, script, world, UI, tools, reports, or original-game evidence in
this first slice.

If P1-GATE-002 later receives a blocking rewrite that removes the needed
YuMemory vocabulary, this gate must be amended before implementation handoff.

## Performance Constraints

Required deterministic signals:

- registry capacity;
- dependency-edge capacity;
- registered resource count;
- acquired handle count;
- retired resource count;
- dependency validation count;
- failed operation count;
- allocation/accounting status using `YuMemory` vocabulary;
- disabled diagnostics/logging behavior equivalence.

First-slice bounds:

- resource capacity: 32 resources maximum;
- resource type capacity: 8 types maximum;
- dependency edge capacity: 64 direct edges maximum;
- logical key length: 64 bytes maximum;
- handle generation: `uint32_t`;
- resource slot/index: `uint32_t`;
- reference count: `uint32_t`;
- all registry and dependency storage capacity is fixed at setup and may not grow dynamically in measured fixtures.

Pass/fail rule:

- exceeding resource, type, dependency-edge, or logical-key bounds is an explicit failure;
- changing registered, acquired, retired, dependency-validation, failure, or allocation/accounting counts outside the declared fixture is a gate failure unless this gate is amended.

Blocking conditions:

- unbounded resource map or global mutable cache;
- file path as primary handle identity;
- package manifest or original resource lookup;
- async load, worker queue, streaming, or background thread;
- decoder or upload ownership;
- hidden allocation in measured handle acquire/release paths;
- diagnostics/log/report output required for behavior;
- tests that validate behavior by parsing logs or reports.

## Tests

Fast gate tests required before the slice can be considered complete:

- `Resource_RegisterSyntheticDescriptor_ReturnsGenerationHandle`
- `Resource_RegisterDuplicate_ReturnsExplicitStatus`
- `Resource_RegistryRejectsCapacityOverflowWithoutMutation`
- `Resource_HandleRejectsWrongGeneration`
- `Resource_HandleRejectsTypeMismatch`
- `Resource_AcquireRelease_TracksReferenceCount`
- `Resource_RepeatedAcquire_IncrementsReferenceCount`
- `Resource_AcquireRejectsReferenceCountOverflow`
- `Resource_RetireRejectsOutstandingAcquire`
- `Resource_RetireRejectsLiveDependentEdge`
- `Resource_DependencyValidationRejectsMissingDependency`
- `Resource_DependencyValidationRejectsCycle`
- `Resource_NoFileOrPackageDependency_ForHandleRegistry`
- `Resource_DisabledDiagnosticsDoesNotChangeResults`
- `Resource_NoHiddenAllocation_UsesYuMemorySignal`

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
src/yuengine/resource/include/yuengine/resource/
src/yuengine/resource/src/
tests/resource/
```

It may update root `CMakeLists.txt` only for `YuResource` and `YuResourceTests`
targets needed by this slice.

It may add no placeholder directories or targets for file, package, decoder,
render, audio, input, script, world, UI, tools, report, capture, oracle, or Game
Adapter work.

## Evidence Inputs

None required.

P1-GATE-006 uses synthetic descriptors only. TouhouNewWorld resource files,
package blobs, old backup runtime code, and old report/status files are not
inputs to this gate.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- ADR-0009 is accepted;
- 红美铃 confirms the proposal satisfies module-entry gate requirements;
- 大妖精 confirms the handle/dependency semantics are implementation-reviewable;
- 博丽灵梦 confirms bounded cost and allocation/accounting requirements;
- 射命丸文 confirms original-game evidence is not being used as API shape;
- task #14 / P1-GATE-002 memory implementation is not in a blocking rewrite state.

If those conditions are not met, return `NEEDS_ARCHITECTURE`, `NEEDS_PERFORMANCE`,
or `NEEDS_EVIDENCE` with exact missing fields.
