# P2-GATE-013: Static Mesh Fixture

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-008, P2-GATE-009, P2-GATE-012, ENG-096
Related decisions: ADR-0011
Source baseline: `58088bd`
Proposal commit: `1af7a53`

## Layer

L3 lower-engine graphics backend proof.

This gate proposes the first static-mesh-shaped geometry fixture after the
landed D3D11 visible triangle and platform input bridge slices. It is not a
Resource, Package, RenderCore, material, scene, UI, World, report, screenshot,
manual visual proof, or Game Adapter gate. The goal is to prove that a caller
can submit bounded indexed static geometry through existing RHI/D3D11 ownership
without introducing asset loading, scene traversal, or render scheduling policy.

```text
caller-owned vertex/index bytes
-> backend-neutral RHI index-buffer and indexed-draw values
-> bounded Null RHI validation and counters
-> private D3D11 index-buffer bind and indexed draw
-> capture-byte hardware-smoke proof if admitted
-> later Resource mesh loading, RenderCore pass scheduling, material system
```

## Current Reality

P2-GATE-008 landed backend-neutral resource and pipeline primitives. P2-GATE-009
landed a visible triangle proof through caller-owned vertex bytes, input layout,
pipeline bind, vertex-buffer bind, draw, present, and capture-byte assertions.
That proves a single non-indexed triangle draw, but it does not create static
mesh value contracts, index buffers, indexed draw, mesh asset identity, Resource
loading, RenderCore submission, material binding, scene traversal, reports,
screenshots, manual visual proof, or Game Adapter behavior.

P2-GATE-012 landed platform input bridge proof at `58088bd`, leaving static
mesh fixture as the next lower-engine hardware proposal in the current queue.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `713`;
- `ctest --preset windows-fast-gate -N -L RHI`: `57`;
- `ctest --preset windows-fast-gate -N -L Fast`: `713`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `48`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `118`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `5`.

## Approval Evidence

Approved after:

- ENG-117A boundary and quality review PASS with no
  `NEEDS_ARCHITECTURE` blocker;
- ENG-117B implementability review PASS with no
  `NEEDS_IMPLEMENTABILITY` blocker;
- ENG-117C test and preset review PASS with no `NEEDS_TEST_POLICY` blocker.

Review evidence:

- proposal commit `1af7a53cb4907f21ad50f59308ff7320962568df` changes only
  `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and this gate doc;
- `git diff --check 1af7a53^ 1af7a53` passed;
- proposal review worktrees stayed clean and reviewers made no source, doc,
  commit, or push changes;
- baseline discovery is default `713`, `RHI` `57`, `Fast` `713`,
  `PerformanceSmoke` `48`, and `EvidenceOracle` `118`;
- default `HardwareSmoke`, `D3D11`, `Win32`, `StaticMesh`, and `IndexedDraw`
  discovery remains `0`;
- `windows-hardware-smoke` currently discovers `5`, with `RHI` `3`,
  `D3D11` `3`, `Win32` `5`, and no static-mesh or indexed-draw hardware test
  admitted yet.

Approval conditions:

- implementation must keep public `YuRHI` headers free of Windows, D3D11, DXGI,
  COM, Platform, RenderCore, Resource, Package, World, UI, report, screenshot,
  visual-proof, and Game Adapter types;
- implementation must keep default `windows-fast-gate` deterministic and
  no-real-device;
- indexed static-geometry proof must stay inside `YuRHI`, private D3D11,
  `Tests/Rhi`, and root CMake labels;
- optional hardware-smoke proof must be isolated in `windows-hardware-smoke`
  with `HardwareSmoke`, `RHI`, `D3D11`, and `Win32` labels;
- proof must use capture bytes plus bounded counters/statuses, not reports,
  screenshots, logs, sleeps, manual visual inspection, source tooling, or
  silent skip.

## Owns

This gate owns the proposal for:

- backend-neutral index buffer value contracts;
- backend-neutral indexed draw descriptors;
- optional static-geometry fixture descriptor made only from caller-owned
  vertex/index spans and existing RHI handles;
- Null RHI validation for index-buffer binding, indexed draw ordering, invalid
  handles, range overflow, and bounded counters;
- private D3D11 index-buffer binding and indexed draw path;
- deterministic RHI tests for descriptor values, invalid ranges, and snapshot
  counters;
- optional D3D11 hardware-smoke proof that renders a deterministic indexed
  geometry shape and validates capture bytes.

## Does Not Own

This gate does not own:

- mesh asset files, model import, Resource loading, Package streaming, File IO,
  async upload queues, mesh cache ownership, asset identity, or material
  binding;
- RenderCore pass scheduling, render graph, draw sorting, scene traversal,
  camera, transform hierarchy, culling, batching, instancing, indirect draws,
  compute, skinning, animation, or lighting;
- shader compiler, shader source tooling, material graph, texture loading,
  depth/stencil policy, blending policy, or multi-render-target expansion;
- reports, screenshots, manual visual proof, visual inspection, UI, World,
  Script, gameplay, or Game Adapter behavior.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- RHI owns GPU resource handles, index-buffer binding, indexed draw submission,
  present, capture, validation counters, and backend state;
- Resource later owns mesh asset identity, decoded mesh data, and streaming
  ownership;
- RenderCore later owns pass scheduling, material selection, draw ordering, and
  scene-to-draw translation;
- World and Game Adapter later own gameplay scene meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate only uses those engines to keep GPU draw mechanics, asset
loading, render scheduling, and gameplay scene meaning separate.

## Dependencies

Allowed dependencies:

- existing `YuRHI` public and private files;
- existing RHI D3D11 private backend files;
- existing `Tests/Rhi` deterministic and hardware-smoke fixtures;
- root CMake/CTest labels and optional `windows-hardware-smoke` admission;
- this gate and queue documentation.

Forbidden dependencies:

- production Resource, Package, File, RenderCore, UI, World, Script, Game
  Adapter, report, screenshot, visual proof, original-game evidence, shader
  compiler, shader source tooling, texture asset loading, or mesh asset
  importer code;
- Windows SDK, D3D11, DXGI, COM, Platform, RenderCore, Resource, Package,
  World, UI, report, screenshot, visual proof, or Game Adapter types in public
  RHI headers;
- proof that depends on logs, reports, screenshots, manual visual inspection,
  or original-game output.

## Public Surface Shape

The first slice should keep public API expansion small and value-based.
Suggested additions:

- `RhiIndexFormat` with fixed 16-bit and 32-bit unsigned index formats;
- `RhiIndexBufferView` with buffer handle, byte offset, byte size, and index
  format;
- `RhiDrawIndexedDesc` with index count, first index, and vertex offset if the
  implementation review confirms signed vertex offsets are needed;
- command records for binding one index buffer and issuing one indexed draw;
- snapshot counters for submitted indexed draw count, last index count, and
  rejected indexed draw count if needed for deterministic proof.

Public headers must not expose Windows, D3D11, DXGI, COM, Platform, RenderCore,
Resource, Package, World, UI, report, screenshot, visual proof, or Game Adapter
types.

## Lifecycle

The intended first-slice lifecycle is:

1. Caller creates vertex and index buffers from caller-owned byte spans.
2. Caller creates existing shader, input layout, pipeline, command list, and
   color target values.
3. Caller records target bind, clear, pipeline bind, vertex-buffer bind,
   index-buffer bind, indexed draw, present, and capture.
4. Null RHI validates lifecycle, handles, byte ranges, index format, and
   bounded command capacity.
5. D3D11 executes the indexed draw into the swapchain backbuffer if admitted to
   hardware smoke.
6. Caller destroys all RHI-owned handles explicitly.

Failure behavior:

- invalid index buffer handle returns explicit status;
- index buffer byte range overflow returns explicit status;
- invalid index format returns explicit status;
- draw indexed before valid pipeline, vertex buffer, or index buffer binding
  returns explicit status;
- rejected indexed draw does not mutate submit counters or accepted draw state;
- command-list capacity overflow returns explicit status without growing
  storage.

## Inputs

- caller-owned vertex byte span;
- caller-owned index byte span;
- caller-owned precompiled shader byte spans;
- fixed input layout, pipeline, target, command-list, and draw descriptors.

## Outputs

- explicit RHI statuses;
- bounded command records;
- Null RHI snapshot counters;
- optional D3D11 capture bytes;
- no mesh asset, material, scene, report, screenshot, log token, or manual
  visual proof.

## Test And Preset Strategy

Default `windows-fast-gate` remains deterministic and no-real-device:

- value-contract and Null RHI behavior tests are labeled `Fast`,
  `ModuleFixture`, and `RHI`;
- dependency/proof-shape scans are labeled `EvidenceOracle`;
- command-capacity and no-growth tests are labeled `PerformanceSmoke`;
- no default deterministic test receives `HardwareSmoke`, `D3D11`, or `Win32`.

Optional D3D11 proof is isolated:

- hardware tests are labeled `HardwareSmoke`, `RHI`, `D3D11`, and `Win32`;
- `windows-hardware-smoke` may discover and run one indexed static-geometry
  capture-byte test;
- default `windows-fast-gate -N -L HardwareSmoke` must remain `0`;
- proof must assert capture bytes and RHI snapshot counters, not screenshots,
  reports, logs, or manual visual inspection.

Expected deterministic first-slice growth is about 6 to 12 fast tests plus 0 to
1 optional hardware-smoke test. The gate must not remove or relabel existing
RHI tests to make the count look smaller.

## Performance Constraints

Required signals:

- declared command capacity;
- accepted indexed draw count;
- rejected indexed draw count;
- last indexed draw index count;
- bound index buffer byte range;
- no command storage growth during measured fixture;
- no Resource, RenderCore, UI, World, or Game Adapter dispatch in the draw path.

Blocking conditions:

- public Windows/D3D11/DXGI/COM or upper-layer type leakage;
- unbounded command or mesh storage;
- static mesh proof requires Resource loading, Package streaming, File IO,
  RenderCore pass scheduling, material system, report, screenshot, or manual
  visual proof;
- default fast gate depends on a real GPU, window focus, screenshot, or visual
  inspection;
- implementation adds shader compiler/source tooling in this slice.

## Required Tests And Checks

Required non-interactive checks for the first slice:

- `cmake --preset windows-fast-gate`
- `cmake --build --preset windows-fast-gate`
- `ctest --preset windows-fast-gate -N`
- `ctest --preset windows-fast-gate --output-on-failure`
- `ctest --preset windows-fast-gate -N -L RHI`
- `ctest --preset windows-fast-gate -N -L Fast`
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`
- `ctest --preset windows-hardware-smoke -N`
- `git diff --check`

If a hardware-smoke indexed static-geometry test is admitted, the implementation
handoff must also record:

- `ctest --preset windows-hardware-smoke --output-on-failure`;
- D3D11/RHI hardware test discovery count;
- capture-byte proof shape;
- unavailable-device behavior if the machine cannot provide the accepted D3D11
  backend.

The implementation handoff must record:

- deterministic discovery before and after;
- `RHI`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and `HardwareSmoke`
  discovery counts;
- proof that default `windows-fast-gate` stays deterministic;
- public-header dependency scan;
- production dependency scan for forbidden modules and platform types;
- indexed draw lifecycle proof shape;
- regression evidence for existing visible-triangle RHI tests.

## Allowed First Slice

If approved, the first implementation slice may modify or create:

```text
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiIndexFormat.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiIndexBufferView.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDrawIndexedDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandRecord.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandType.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandList.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceSnapshot.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/IRhiDevice.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/NullRhiDevice.h
Src/YuEngine/Rhi/Src/RhiCommandList.cpp
Src/YuEngine/Rhi/Src/NullRhiDevice.cpp
Src/YuEngine/Rhi/Src/D3D11RhiDeviceInternal.h
Src/YuEngine/Rhi/Src/D3D11RhiDeviceWindows.cpp
Tests/Rhi/RhiTests.cpp
Tests/Rhi/RhiD3D11HardwareSmokeTests.cpp
CMakeLists.txt
docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md
docs/gates/P2_GATE_013_STATIC_MESH_FIXTURE.md
```

`CMakePresets.json` is not expected to change. Any CMake change must preserve
the default `windows-fast-gate` behavior.

Implementation review may narrow file names or proof shape, but it may not
expand beyond `YuRHI`, private D3D11 RHI implementation, `Tests/Rhi`, root CMake
labels, and this gate/queue documentation without returning
`NEEDS_ARCHITECTURE`.

## Non-Goals

- No mesh asset files.
- No Resource or Package streaming.
- No File IO.
- No RenderCore.
- No material system.
- No shader compiler or shader source tooling.
- No scene traversal, camera, transform hierarchy, culling, batching,
  instancing, indirect draw, compute, animation, or skinning.
- No UI, World, Script, or Game Adapter.
- No reports, screenshots, manual visual proof, or visual inspection.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- 八云蓝 confirms the boundary keeps static geometry proof separate from
  Resource, Package, RenderCore, material, scene, UI, World, Game Adapter,
  reports, screenshots, and manual visual proof;
- 博丽灵梦 confirms the proposed RHI/D3D11 indexed draw shape is locally
  implementable without public backend-type leakage, unbounded storage, shader
  compiler work, Resource/File coupling, or RenderCore dispatch;
- 雾雨魔理沙 confirms the test and preset policy is enforceable, keeps default
  `windows-fast-gate` deterministic, isolates any D3D11 proof in
  `windows-hardware-smoke`, and does not rely on logs, reports, screenshots, or
  manual visual inspection;
- 八云紫 confirms this gate is the next lower-engine proposal after
  P2-GATE-012 and before Resource/Package streaming, RenderCore, UI, World, or
  Game Adapter tasks.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_IMPLEMENTABILITY`, or `NEEDS_TEST_POLICY` with exact missing fields.
