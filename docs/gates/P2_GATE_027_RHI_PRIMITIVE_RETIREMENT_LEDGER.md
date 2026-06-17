# P2-GATE-027: RHI Primitive Retirement Ledger

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: Combined lower-engine review
Depends on: P2-GATE-008, P2-GATE-006, P2-GATE-004, ADR-0011
Related decisions: ADR-0011
Source baseline: `74189da`
Proposal commit: `1c68beb`
Approval evidence: ENG-144R combined proposal review PASS.

## Layer

L3 lower-engine RHI lifecycle boundary over landed RHI primitive handles.

This gate proposes the first bounded RHI-owned primitive retirement ledger. The
first slice may accept retirement requests for existing buffer, texture,
sampler, shader, pipeline, and fence handles, record deterministic pending or
drained retirement state, invalidate handles only when a drain is accepted, and
expose counter evidence. It does not create a renderer scheduler, frame graph,
Resource residency policy, Streaming bridge, File or Package ownership,
material graph, scene traversal, UI, World, Script, report, screenshot, manual
proof, or Game Adapter behavior.

```text
landed public RHI primitive handles
-> RHI-owned retirement request records
-> bounded retirement ledger and drain proof
-> deterministic handle invalidation and counters
-> later renderer/resource lifetime integration
```

## Current Reality

P2-GATE-008 landed backend-neutral buffer, texture, sampler, shader, pipeline,
and fence primitive contracts. Later graphics gates use these handles for
visible geometry, texture sampling, RenderCore fixture pass, material binding,
submission batch, frame packet, and render graph declaration proof. Existing
RHI also has immediate `Destroy*` behavior and snapshot counters.

The missing boundary is a value-level record of primitive retirement ordering:
callers can destroy handles immediately, but there is no first-slice ledger for
requesting retirement, proving pending versus drained state, rejecting
duplicate requests, or validating fence-readiness before handle invalidation.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `858`;
- `ctest --preset windows-fast-gate -N -L RHI`: `141`;
- `ctest --preset windows-fast-gate -N -L Fast`: `858`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `91`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `262`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L RHI`: `5`.

## Approval Evidence

Approved after ENG-144R combined proposal review PASS.

Review evidence:

- proposal commit `1c68beb4b095feb7a21208da884786dde32542c6` changes only
  `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and this gate doc;
- `git diff --check 1c68beb4b095feb7a21208da884786dde32542c6^
  1c68beb4b095feb7a21208da884786dde32542c6` passed;
- review work was read-only and made no source, doc, commit, push, approval, or
  implementation changes;
- review confirmed no `NEEDS_ARCHITECTURE`, `NEEDS_IMPLEMENTABILITY`, or
  `NEEDS_TEST_POLICY` blocker;
- boundary review confirmed the proposal stays in RHI-owned primitive
  retirement request, ledger, drain, handle invalidation, and counter proof over
  landed primitive handles;
- implementability review confirmed existing public `YuRHI` handles,
  `Destroy*` APIs, generation-based invalidation behavior, snapshots, Null RHI,
  and D3D11 RHI surfaces are sufficient for the first slice;
- test-policy review confirmed deterministic default `windows-fast-gate`
  evidence, focused `RHI_PrimitiveRetirement` tests, label discovery,
  public-header/native leak scans, production dependency scans, CMakePresets
  no-drift checks, hardware-smoke isolation, and proof-shape scans are required;
- proposal discovery counts matched the reviewed baseline: default fast gate
  `858`, `RHI` `141`, `Fast` `858`, `PerformanceSmoke` `91`,
  `EvidenceOracle` `262`, default `HardwareSmoke` `0`, and
  `windows-hardware-smoke` `7` with `RHI` `5`.

## Owns

This gate owns the proposal for:

- RHI-owned primitive retirement status, request, record, drain result, and
  snapshot value contracts;
- bounded retirement ledger storage for buffer, texture, sampler, shader,
  pipeline, and fence handles;
- validation for handle generation, primitive kind, duplicate retirement,
  ledger capacity, and optional fence readiness before mutation;
- deterministic behavior for pending retirement, ready drain, rejected drain,
  and accepted drain;
- handle invalidation and destroyed counters when a ready retirement entry is
  drained;
- compatibility proof that existing immediate `Destroy*` APIs keep their
  current behavior;
- deterministic tests under default `windows-fast-gate` with `RHI`, `Fast`,
  `PerformanceSmoke`, and `EvidenceOracle` labels.

## Does Not Own

This gate does not own:

- Resource residency, Resource cache ownership, Resource upload completion,
  Streaming queues, File IO, Package parsing, asset decode, asset import, or
  original-game package behavior;
- RenderCore scheduling, render graph execution, frame graph execution,
  command-list parallelism, transient resource aliasing, material graph,
  shader source tooling, shader compiler invocation, scene traversal, UI,
  World, Script, gameplay, reports, screenshots, logs, sleeps, manual visual
  proof, or Game Adapter behavior;
- new D3D11 feature expansion, Vulkan/Metal/OpenGL abstraction, descriptor heap
  policy, bindless policy, renderer lifetime policy, or cross-frame scheduling;
- public exposure of Windows SDK, D3D11, DXGI, COM, Platform, backend-private
  native objects, or native fence primitives.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- RHI owns backend-neutral primitive handles, backend-private native objects,
  primitive lifecycle, retirement request validation, drain behavior, and
  lifecycle counters;
- RenderCore later owns render graph scheduling, pass ordering, frame packet
  use, and high-level renderer lifetime policy above public RHI values;
- Resource and Streaming later own asset residency and upload policy above
  File and Package boundaries;
- World and Game Adapter later own scene and gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep RHI primitive lifecycle,
renderer scheduling, asset residency, and gameplay meaning separate.

## Dependencies

Allowed dependencies:

- existing public `YuRHI` primitive handle, descriptor, device, fence, status,
  and snapshot value contracts;
- new RHI-owned retirement status, request, record, drain result, and snapshot
  value contracts;
- existing private Null and D3D11 RHI implementation files under `YuRHI`;
- `Tests/Rhi` deterministic tests plus root CMake labels;
- optional reuse of existing hardware-smoke fixtures for discovery only;
- this gate and queue documentation.

Forbidden dependencies:

- `YuResource`, `YuStreaming`, `YuPackage`, `YuFile`, `YuRenderCore`, Material,
  World, Script, UI, Game Adapter, real decoder, parser, report, screenshot,
  visual proof, or original-game evidence dependencies from RHI public headers
  or production retirement code;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public RHI retirement headers;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  hardware-only proof, or original-game output as evidence.

## Public Contract Boundary

Public RHI contracts may expose value-only descriptors such as:

- retirement status and operation values;
- primitive kind, handle generation, request id, retirement id, optional fence
  id, and drain limit values;
- bounded snapshots for retirement counters, pending count, drained count,
  rejected count, duplicate request count, invalid handle count, capacity
  rejected count, and fence-not-ready count;
- request, query, and drain functions on `IRhiDevice` or equivalent public RHI
  device surface.

Public RHI contracts must not expose:

- `ID3D11*`, `IDXGI*`, `IUnknown`, `HWND`, `HANDLE`, Windows headers, COM
  ownership, native fence pointers, native resource pointers, or backend
  storage layout;
- Resource handles, Package ids, File paths, Streaming request ids, RenderCore
  pass ids, material ids, scene ids, UI ids, report handles, screenshot
  artifacts, visual-proof types, or Game Adapter types.

Public `YuRHI` headers must remain free of Resource, Streaming, Package, File,
RenderCore, material graph, scene, UI, World, Script, and Game Adapter
dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only RHI-owned value contracts for primitive retirement status, request,
   record, drain result, primitive kind, and snapshot.
2. Add bounded retirement ledger storage inside existing RHI device ownership.
3. Validate handle generation, primitive kind, duplicate retirement, capacity,
   and optional fence readiness before mutation.
4. Reject invalid handles, stale handles, wrong primitive kind, duplicate
   request, capacity overflow, and fence-not-ready drain without mutating RHI
   primitive counts or handle generations.
5. Drain ready entries deterministically, invalidate the retired public handle
   generation, and update existing destroyed counters consistently with current
   immediate destroy semantics.
6. Keep existing immediate `Destroy*` APIs compatible and covered by regression
   tests.
7. Keep RHI core free of Resource, Streaming, Package, File, RenderCore,
   material graph, render graph, frame graph, scene, UI, World, Script, and
   Game Adapter dependencies.
8. Add deterministic fast tests for request, query, drain, invalid handle,
   stale handle, wrong kind, duplicate request, capacity overflow,
   fence-not-ready pending, fence-ready drain, no mutation on rejection,
   snapshot counters, and immediate destroy compatibility.
9. Keep all required proof in default `windows-fast-gate`; no new
   hardware-smoke admission is required for this gate.

## Test And Evidence Policy

Default `windows-fast-gate` must remain deterministic and must not require real
hardware, screenshots, reports, logs, sleeps, original-game packages, or manual
inspection.

Required implementation evidence:

- `cmake --preset windows-fast-gate`;
- `cmake --build --preset windows-fast-gate`;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- focused `RHI_PrimitiveRetirement` or equivalent tests proving the new
  boundary;
- discovery counts for `RHI`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`,
  and `HardwareSmoke`;
- proof that default `windows-fast-gate` still discovers `HardwareSmoke` as
  `0`;
- proof that `windows-hardware-smoke` remains isolated;
- public-header scan for native/backend/Resource/RenderCore/World/Game leakage;
- production dependency scan proving `YuRHI` retirement code does not gain
  Resource, Streaming, Package, File, RenderCore, material graph, render graph,
  frame graph, scene, UI, World, Script, Game Adapter, parser, report,
  screenshot, or original-game dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, hardware-only proof, and silent
  skip.

Accepted proof:

- deterministic assertions over public RHI primitive handles, retirement
  request records, drain outcomes, snapshots, and counters;
- Null RHI and existing private RHI implementation assertions through public
  RHI APIs;
- explicit validation counters proving failed paths do not mutate primitive
  counts, handle generations, existing destroy counters, RenderCore state,
  Resource state, Streaming state, Package state, File state, or upper-engine
  state.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of RHI primitive retirement proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- Resource residency, Resource upload, RenderCore scheduling, material graph,
  render graph execution, frame graph execution, scene streaming, UI, World,
  Script, or Game Adapter behavior as evidence for this gate.

## Non Goals

- No Resource residency, cache ownership, upload completion, or asset import.
- No Streaming, File IO, Package parsing, original package reader, or real
  decoder.
- No RenderCore scheduling, render graph execution, frame graph execution, or
  command-list parallelism.
- No material graph, shader compiler, shader source tooling, descriptor heap
  policy, or bindless policy.
- No scene traversal, World streaming, UI, Script, gameplay, report,
  screenshot, log, sleep, manual proof, hardware-only proof, or Game Adapter
  behavior.
- No new public native/backend type exposure.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after a combined lower-engine review
confirms the proposal stays in RHI-owned primitive retirement request, ledger,
drain, and handle invalidation proof over landed primitive handles, remains
implementable with the existing Null and D3D11 RHI device storage, preserves
current immediate destroy behavior, and keeps deterministic fast-gate proof free
of Resource, Streaming, File, Package, RenderCore, material, scene, report,
screenshot, log, sleep, manual, original-game, or hardware-only evidence.
