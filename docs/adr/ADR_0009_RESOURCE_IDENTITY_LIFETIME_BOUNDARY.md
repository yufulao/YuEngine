# ADR-0009: Resource Identity And Lifetime Boundary

Status: Proposed
Owner: 八云紫
Reviewers: 红美铃, 大妖精, 博丽灵梦, 射命丸文, 雾雨魔理沙 when implementation exists
Date: 2026-06-10
Depends on: ADR-0001, ADR-0002, ADR-0005, ADR-0006, ADR-0008, docs/YUENGINE_MODULE_ENTRY_GATES.md, docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md

## Context

YuEngine needs a Resource boundary before script, scene, render, audio, UI, and
Game Adapter work can rely on asset identity. The Resource module must not start
by parsing TouhouNewWorld packages or by reviving old report-shaped resource
diagnostics.

The first Resource decision therefore separates resource identity and lifetime
from file bytes, package formats, decoders, upload scheduling, and original-game
meaning.

## Decision

YuEngine introduces `YuResource` as the owner of:

- resource identity vocabulary;
- resource type identifiers;
- generation-checked resource handles;
- bounded synthetic resource registry behavior;
- direct dependency-edge declaration and validation;
- reference/lifetime counters for registered resources;
- explicit result/status values for handle and dependency failures.

The first Resource slice is a handle and dependency-lifetime skeleton only. It
does not load bytes from `YuFile`, parse packages, decode assets, upload GPU or
audio data, or interpret TouhouNewWorld resource names.

## Resource Identity

Resource identity is not a raw path string.

First-slice identity contains:

- `ResourceTypeId`;
- bounded logical key bytes;
- stable registry slot/index;
- handle generation.

Rules:

- logical keys are deterministic fixture inputs, not file paths;
- logical key comparison is case-sensitive;
- first-slice fixture keys use lowercase ASCII;
- no OS path normalization or locale-sensitive case conversion is performed;
- duplicate `(ResourceTypeId, logical key)` registration fails explicitly;
- handle equality uses slot/index and generation, not pointer value or key text.

## Handle Lifetime

`ResourceHandle` is an opaque value. It does not expose pointers, file paths,
package offsets, decoder state, or uploaded backend handles.

First-slice handle states:

- invalid;
- registered;
- acquired;
- released;
- retired.

Rules:

- acquiring an invalid or stale-generation handle fails explicitly;
- acquiring with an expected `ResourceTypeId` that differs from the registered type fails explicitly and does not change reference count;
- repeated acquire of the same valid handle increments the `uint32_t` reference count and returns success;
- acquiring when reference count is already `UINT32_MAX` fails explicitly and does not change reference count;
- releasing a handle not currently acquired fails explicitly;
- releasing an acquired handle decrements the `uint32_t` reference count;
- retiring a resource with outstanding acquisitions fails explicitly;
- retiring a resource with any live dependent edge fails explicitly;
- generation increments on retire/reuse so stale handles cannot reacquire a new resource.

## Dependency Boundary

First-slice dependencies are declared as direct edges between synthetic registered
resources.

Rules:

- missing dependency registration fails explicitly;
- self-dependency fails explicitly;
- dependency cycle validation fails explicitly;
- dependency edges are bounded and setup-time only;
- a dependency edge is live while the dependent resource is registered and not retired;
- P1-GATE-006 has no separate dependency-edge removal API;
- successfully retiring a resource clears that resource's own outbound dependency edges;
- retiring a resource with any live inbound dependent edge fails with explicit `StillDependedOn` or equivalent status;
- no automatic load graph, async job graph, streaming policy, or cache eviction is introduced.

Future Resource gates may add dependency load order and async scheduling only
after Thread/Task and File/VFS implementation gates are approved.

## Explicit Status Values

`YuResource` behavior is observable through explicit result/status values, not
log text, report output, or diagnostics side effects.

First-slice statuses include:

- `Ok`;
- `NotFound`;
- `DuplicateResource`;
- `CapacityExceeded`;
- `InvalidHandle`;
- `GenerationMismatch`;
- `TypeMismatch`;
- `NotAcquired`;
- `ReferenceCountOverflow`;
- `StillReferenced`;
- `StillDependedOn`;
- `DependencyMissing`;
- `DependencyCycle`;
- `UnsupportedInThisGate`.

The exact enum names may change during implementation review, but the semantic
cases above must stay visible and testable.

## Memory Boundary

Resource identity and dependency storage is bounded.

If the P1-GATE-002 `YuMemory` implementation is accepted before the first
Resource implementation starts, Resource allocation/accounting signals use
`YuMemory` vocabulary. This does not claim CRT/STL/general heap coverage.

If the `YuMemory` implementation receives a blocking rewrite that removes the
needed vocabulary, P1-GATE-006 must be amended before implementation handoff.

## Thread Boundary

P1-GATE-006 is synchronous and single-threaded.

Blocked:

- background loader;
- worker pool;
- async IO;
- streaming;
- lock-free dependency graph;
- callbacks that execute resource load work.

Future async Resource work requires accepted and implemented Thread/Task
semantics plus a separate gate.

## File And Package Boundary

P1-GATE-006 must not read files.

`YuFile` and package facts are future inputs for load/resolve gates. Resource
identity must be usable without a file system so higher layers can depend on
opaque handles instead of paths.

Blocked:

- file reads;
- VFS mount lookup;
- package parsing for `.pak`, `.dat`, or any original format;
- package manifest indexing;
- loose original resource lookup;
- decoder selection from extensions.

## Diagnostics Boundary

Diagnostics may observe Resource counters later. Diagnostics must not own
Resource behavior.

Rules:

- Resource results are explicit statuses.
- Disabled diagnostics/logging does not change Resource results.
- Report/capture/oracle output is not a Resource runtime API.

## Evidence Boundary

TouhouNewWorld `resource/pack*.pak`, `resource/rpack*.dat`, loose JSON, shaders,
and DB files are evidence for future package, load, adapter, renderer, script,
scene, and UI gates. They are not inputs to P1-GATE-006.

P1-GATE-006 fast tests must use synthetic descriptors only. They must not read:

- `C:\Steam\steamapps\common\TouhouNewWorld\resource`;
- old backup runtime files;
- old report/status JSON or markdown as runtime behavior fixtures.

Original resource facts may become Resource acceptance fixtures only after a
package/load gate defines the owning interface and the evidence owner approves
the fixture boundary.

## P1-GATE-006 Compatibility

P1-GATE-006 may implement:

- `YuResource` target;
- `YuResourceTests` target;
- resource type/key/handle value types;
- bounded synthetic registry;
- direct dependency-edge validation;
- acquire/release/retire behavior;
- explicit Resource status/result values;
- deterministic snapshots/counters for tests.

P1-GATE-006 must not implement:

- File/VFS reads;
- package parser;
- original resource manifest;
- asset decoder;
- resource cache eviction policy;
- hot reload;
- async load;
- GPU/audio upload scheduling;
- script, scene, UI, gameplay, or Game Adapter behavior;
- report, capture, oracle, or tool output.

## Consequences

YuEngine gains a narrow Resource identity foundation that higher layers can
target without binding to paths or original-game package details.

The cost is that no real asset loading exists yet. That is intentional. Real load
and package behavior needs separate gates after File/VFS, Thread/Task, Memory,
and Resource identity are proven.

## Requested Review

Reviewers should decide whether this ADR is accepted, rejected, or needs
amendment.

If accepted, ADR-0009 becomes the architecture input for P1-GATE-006 Resource
Identity And Lifetime Skeleton.
