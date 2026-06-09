# P2-GATE-003: Package Manifest And Load Plan Boundary

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 红美铃, 八云蓝, 博丽灵梦, 大妖精, 射命丸文, 雾雨魔理沙 when implementation exists
Depends on: ADR-0013
Related decisions: ADR-0005, ADR-0006, ADR-0008, ADR-0009
Source baseline: Phase 2 through `e41d2ff`

## Layer

L4-L5 package/load planning boundary.

This gate proves bounded synthetic package manifests and deterministic load-plan
metadata without File reads, original package parsing, Resource mutation,
decoding, async loading, upload scheduling, tools, or Game Adapter behavior.

## Pre-Approval Blockers

P2-GATE-003 must not be approved for implementation until:

- task #21 `YuFile` code/semantic review closes without changing the public
  File/VFS vocabulary this gate references;
- task #33 `YuResource` code/semantic review closes without changing the
  resource type/logical-key vocabulary this gate references;
- task #14 `YuMemory` code/semantic review does not remove the accounting
  vocabulary required by this gate, or this gate is amended first.

These blockers do not prevent architecture review. They prevent
`APPROVED_FOR_FIRST_SLICE`.

## Owns

This gate owns the first `YuPackage` implementation slice for:

- package ID values;
- package entry IDs;
- bounded synthetic manifest descriptors;
- bounded package entry metadata;
- resource type/logical-key association;
- dependency entry declarations;
- deterministic load-plan records;
- package/load status values;
- package snapshots and counters for tests.

## Does Not Own

This gate does not own:

- File/VFS reads;
- original package parser for `.pak`, `.dat`, `.rpack`, or any original format;
- loose original resource lookup;
- Resource registry mutation, acquire/release/retire, or load completion;
- decoder, decompressor, cache, hot reload, or asset import;
- async IO, worker queues, streaming, or background loading;
- RHI texture upload, audio sample upload, script/scene/UI/gameplay binding, or
  Game Adapter behavior;
- report/profiler/capture/oracle/dashboard/tool output.

## UE/Unity Analogue

UE5 references:

- package file, asset registry, and IoStore concepts as responsibility
  references.

Unity references:

- Asset Database, Package Manager, Addressables/AssetBundle workflows as
  responsibility references.

YuEngine decision:

- Start with synthetic manifest metadata and deterministic load plans.
- Keep original package parsing, byte IO, asset import, and adapter-specific
  resource lookup outside the first slice.
- Keep Resource identity separate from package entry metadata.

## Lifecycle

First-slice lifecycle:

1. Setup creates a package manifest registry with fixed capacities.
2. Setup registers bounded synthetic package manifests.
3. Setup registers bounded entries for each package.
4. Setup declares bounded direct dependencies between entries.
5. Validation checks duplicate entries, missing dependencies, type mismatch,
   byte range bounds, and dependency cycles.
6. Resolve creates a deterministic load plan for one entry or resource key.
7. Snapshot exposes counts, last status, capacity, accepted, rejected, and
   accounting signals.

Failure behavior:

- invalid package ID, entry ID, resource type, or logical key returns explicit
  status and does not mutate state;
- duplicate package or entry registration returns explicit duplicate status and
  does not mutate state;
- manifest, entry, or dependency capacity overflow returns explicit capacity
  status and does not mutate state;
- expected resource type mismatch returns explicit mismatch status and does not
  mutate counters;
- missing dependency and dependency cycle return explicit statuses;
- byte range overflow or entry-size overflow returns explicit status and does
  not mutate state;
- resolving a missing entry returns explicit not-found status;
- disabled diagnostics/logging does not change Package results.

## Inputs

- synthetic package manifest descriptors;
- package IDs and entry IDs;
- bounded resource type IDs and logical keys;
- bounded source-key metadata;
- declared byte offsets and byte sizes;
- declared direct entry dependencies;
- optional memory tracker if P1-GATE-002 implementation vocabulary is accepted.

## Outputs

- package manifest handles or IDs;
- package entry IDs;
- deterministic load-plan records;
- package/load status values;
- package snapshots and counters;
- allocation/accounting status using `YuMemory` vocabulary or explicit
  deferral;
- no log/report text as behavior transport.

## Dependencies

Allowed dependencies:

- C++ standard library;
- CMake/CTest tooling;
- `YuMemory` for accounting vocabulary/signal tests when accepted;
- `YuResource` public value vocabulary only after task #33 code/semantic review
  closes;
- `YuDiagnostics` only for disabled-behavior observation when available.

Target dependency expectation:

```text
YuPackage
  -> YuMemory for accounting vocabulary/signal tests, or explicit deferral
  -> YuResource public value vocabulary when task #33 is accepted
  -> optional YuDiagnostics for disabled-behavior observation
```

`YuPackage` must not depend on `YuKernel`, `YuPlatform`, `YuThread`, `YuFile`
runtime reads, RHI, audio, input, script, scene/world, UI, tools, reports, or
TouhouNewWorld evidence in this first slice.

## Performance Constraints

Required deterministic signals:

- package capacity;
- entry capacity;
- dependency edge capacity;
- load-plan capacity;
- accepted package count;
- accepted entry count;
- accepted dependency count;
- rejected operation count;
- last status;
- load-plan record count;
- allocation/accounting status using `YuMemory` vocabulary or explicit
  deferral;
- disabled diagnostics/logging behavior equivalence.

First-slice bounds:

- package capacity: 4 manifests maximum;
- entry capacity: 32 entries total maximum;
- dependency edge capacity: 64 direct edges maximum;
- load-plan record capacity: 32 records maximum;
- package ID and entry ID are fixed numeric values;
- logical resource key length: 64 bytes maximum;
- source key length: 128 bytes maximum;
- declared byte offset and byte size use `uint32_t`;
- declared byte size maximum: 4096 bytes;
- setup/load registration may initialize fixed storage;
- resolve/load-plan creation must not allocate or grow storage.

Pass/fail rule:

- exceeding package, entry, dependency, load-plan, key, source-key, or byte-size
  bounds is an explicit failure;
- load-plan resolve allocating, growing storage, reading files, mutating
  Resource registry state, or depending on diagnostics/report output is a gate
  failure unless this gate is amended.

Blocking conditions:

- original package parser;
- File/VFS read or mount lookup in fast tests;
- Resource registry mutation or asset load completion;
- unbounded manifest, entry, dependency, or load-plan storage;
- hidden allocation in measured resolve/load-plan path;
- string lookup on a declared hot/frame path;
- diagnostics/log/report/profiler/oracle output required for behavior;
- tests that validate behavior by parsing logs, reports, original packages, or
  original resources.

## Tests

Fast gate tests required before the slice can be considered complete:

- `Package_RegisterSyntheticManifest_ReturnsStableId`
- `Package_RegisterDuplicateManifest_ReturnsExplicitStatus`
- `Package_RegisterEntry_ReturnsStableEntryId`
- `Package_RegisterDuplicateEntry_ReturnsExplicitStatus`
- `Package_ManifestCapacityOverflow_DoesNotMutate`
- `Package_EntryCapacityOverflow_DoesNotMutate`
- `Package_RegisterEntryRejectsOversizedKeysWithoutMutation`
- `Package_RegisterEntryRejectsOversizedByteRangeWithoutMutation`
- `Package_ResolveEntryByResourceKey_ReturnsDeterministicLoadPlan`
- `Package_ResolveRejectsUnknownResourceKey`
- `Package_ResolveRejectsTypeMismatchWithoutMutation`
- `Package_DependencyValidationRejectsMissingEntry`
- `Package_DependencyValidationRejectsCycle`
- `Package_DependencyPlanPreservesDeclarationOrder`
- `Package_DependencyCapacityOverflow_DoesNotMutate`
- `Package_DisabledDiagnosticsDoesNotChangeResults`
- `Package_NoFileReadOriginalPackageOrGameAdapterDependency`
- `Package_NoHiddenAllocation_UsesYuMemorySignal`

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
src/yuengine/package/include/yuengine/package/
src/yuengine/package/src/
tests/package/
```

It may update root `CMakeLists.txt` only to add `YuPackage` and
`YuPackageTests`.

It may not create placeholder directories or targets for package parsers,
File/VFS runtime reads, resource cache, async loading, streaming, decoders,
upload queues, tools, report, profiler, capture, oracle, or Game Adapter work.

## Non-Goals

- No original `.pak`, `.dat`, `.rpack`, or loose resource parser.
- No File/VFS reads.
- No Resource registry mutation.
- No asset decoder, cache, or hot reload.
- No async loading, worker queue, or streaming.
- No RHI/audio upload.
- No script/scene/UI/gameplay or Game Adapter behavior.
- No report/profiler/oracle output.

## Evidence Inputs

No original-game evidence is required for the first slice.

TouhouNewWorld package names, file layouts, resource blobs, old backup runtime
files, and old reports remain future validation evidence only. They must not be
read by P2-GATE-003 fast tests.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- ADR-0013 is accepted;
- task #21 and task #33 code/semantic reviews close or this gate is amended to
  avoid unstable vocabulary;
- 红美铃 confirms the proposal satisfies module-entry gate requirements and
  sequencing against active review work;
- 八云蓝 confirms the UE5/Unity package/load responsibility comparison is sound;
- 博丽灵梦 confirms the manifest/load-plan cost model and no-allocation resolve
  path;
- 大妖精 confirms the public surface and tests are locally implementable;
- 射命丸文 confirms original package/resource evidence is not being used as API
  shape.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_PERFORMANCE`, or `NEEDS_EVIDENCE` with exact missing fields.
