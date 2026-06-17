# P2-GATE-033: RHI Swapchain Resize Public Contract

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 琪露诺
Reviewers: Combined lower-engine review with hardware evidence lane
Depends on: P2-GATE-007, P2-GATE-006, P2-GATE-005, P2-GATE-004, P2-GATE-027, ADR-0011
Related decisions: ADR-0011
Source baseline: `9323358`
Proposal commit: `4021327`
Approval evidence: ENG-156H hardware evidence PASS and ENG-156R combined proposal review PASS.

## Layer

L3 lower-engine RHI swapchain lifecycle boundary over the landed D3D11
swapchain clear, present, capture, and RHI primitive-handle generation
contracts.

This gate proposes the first bounded RHI swapchain resize public contract. The
first slice may expose a backend-neutral resize request/result, update
swapchain snapshot evidence, invalidate the old swapchain color target handle
when a real resize succeeds, and prove D3D11 backbuffer recreation through
hardware-smoke bytes and public values. It is not a Platform event gate,
renderer scheduling gate, Resource upload gate, RenderCore frame graph gate,
window-management gate, screenshot gate, report gate, manual-proof gate, or
Game Adapter gate.

```text
landed Platform native surface value
-> caller-owned RhiNativeSurfaceDesc conversion
-> landed D3D11 swapchain clear/present/capture path
-> backend-neutral swapchain resize request/result
-> refreshed swapchain snapshot and color-target generation
-> later renderer resize policy and frame-graph integration
```

## Current Reality

P2-GATE-005 landed the Platform window, native surface, and event-pump boundary.
Platform owns OS windows and native handle identity; production RHI still does
not include Platform headers or link Platform targets.

P2-GATE-006 landed the backend-neutral `IRhiDevice`, `RhiDeviceDesc`,
`RhiNativeSurfaceDesc`, `RhiCapabilities`, and Null RHI device boundary.

P2-GATE-007 landed the D3D11 device, context, swapchain, clear, present, and
capture first slice. D3D11 owns private swapchain, backbuffer,
render-target-view, and capture texture state. Public RHI exposes
`RhiSwapchainDesc`, `RhiSwapchainSnapshot`, `RhiDeviceSnapshot`, and
`GetSwapchainColorTarget`.

P2-GATE-027 landed deterministic RHI handle invalidation evidence for primitive
retirement. The swapchain color target already uses a public
`RhiTextureHandle` generation, but no public contract defines how that handle
changes when the swapchain backbuffer is resized.

The accepted current main baseline after P2-GATE-031 approval is `7204e9b`.
The RHI and hardware-smoke surface was unchanged by P2-GATE-031:

- default `windows-fast-gate` discovery remains hardware-free for
  `HardwareSmoke`;
- `windows-hardware-smoke` continues to own optional D3D11 hardware proof;
- public RHI headers remain free of Windows SDK, D3D11, DXGI, COM, Platform,
  Resource, RenderCore, Audio, File, Package, Streaming, scene, UI, World,
  Script, and Game Adapter dependencies.

The missing boundary is resize. Today a D3D11 swapchain can be created,
cleared, presented, and captured at its initial extent, but there is no
backend-neutral `ResizeSwapchain` request/result, no explicit resize capability
flag, no resize counters in the swapchain snapshot, no specified same-extent
no-op behavior, no rejected-resize accounting, and no required proof that the
old backbuffer handle is invalid after a real resize.

## Approval Evidence

Approved after ENG-156H hardware evidence PASS and ENG-156R combined proposal
review PASS.

Review evidence:

- proposal commit changes only `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and
  this gate doc;
- `git diff --check` passes for the proposal commit;
- boundary review confirms all public resize contracts stay value-only and
  backend-neutral;
- implementability review confirms existing Null RHI, D3D11 swapchain,
  snapshot, capability, status, handle-generation, present, and capture
  patterns can carry the first slice;
- hardware-evidence review confirms optional D3D11 smoke tests can prove
  backbuffer generation change, same-extent no-op behavior, and capture-byte
  sizing without requiring hardware in the default fast gate;
- dependency review confirms public headers do not leak native/backend,
  Platform, Resource, RenderCore, Audio, File, Package, Streaming, scene, UI,
  World, Script, Game Adapter, report, screenshot, or visual-proof types;
- test-policy review confirms focused fast tests, focused hardware-smoke tests,
  label discovery, `CMakePresets` no-drift, public-header leak scans, production
  dependency scans, and proof-shape scans are mandatory before landing.

## Owns

This gate owns the proposal for:

- `RhiSwapchainResizeRequest` as a value-only request with target extent only;
- `RhiSwapchainResizeResult` as a value-only result containing status, previous
  extent, previous color target, refreshed snapshot, and whether an actual
  resize occurred;
- a `supports_swapchain_resize` capability flag;
- swapchain snapshot resize counters for accepted real resizes and rejected
  resize attempts;
- one `IRhiDevice::ResizeSwapchain` public device operation with Null RHI and
  D3D11 implementations;
- validation for zero extents, fixture extent bounds, unsupported backend,
  missing hardware, invalid lifecycle, device loss, and native resize failure;
- successful D3D11 resize behavior that releases and recreates the private
  backbuffer, render target view, and capture texture without exposing native
  objects;
- public color-target handle invalidation through generation change after a
  real resize;
- same-extent resize behavior as `Success` with `resized = false`, unchanged
  generation, unchanged resize count, and unchanged snapshot extent;
- failure behavior that leaves the current extent and current color target
  stable, records rejection evidence, and exposes no partial new backbuffer;
- deterministic fast tests for default value contracts and Null RHI unsupported
  behavior;
- optional D3D11 hardware-smoke tests for resize generation, same-extent no-op,
  post-resize present, and post-resize capture bytes.

## Does Not Own

This gate does not own:

- Platform window resize messages, event-pump policy, DPI policy, swapchain
  resize scheduling from OS events, or native surface ownership;
- changing swapchain format, vsync mode, buffer count, fullscreen mode, display
  mode, adapter selection, tearing support, or frame-latency policy;
- renderer scheduling, RenderCore frame graph execution, transient resource
  aliasing, material graph behavior, shader source tooling, or command-list
  parallelism;
- Resource upload, Resource decoded payload ownership, Streaming, Package,
  File, Audio lifecycle, Audio streaming, scene traversal, UI, World, Script,
  Game Adapter, report generation, screenshots, logs, sleeps, manual proof,
  visual inspection, or original-game evidence;
- Vulkan, Metal, OpenGL, or multi-backend resize implementation beyond a clean
  backend-neutral contract and Null RHI unsupported behavior.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Platform owns OS window identity and native surface values;
- caller-owned glue converts `PlatformNativeSurface` into
  `RhiNativeSurfaceDesc`;
- RHI owns backend-neutral swapchain descriptors, resize requests/results,
  swapchain snapshots, capabilities, color-target handles, and backend-private
  D3D11 swapchain/backbuffer lifetime;
- RenderCore later owns when a renderer chooses to request resize and how
  frame graph resources react to a resized backbuffer;
- Resource, Audio, File, Package, Streaming, World, UI, Script, and Game Adapter
  remain outside this RHI lifecycle boundary.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep platform window ownership,
backend swapchain ownership, renderer resize policy, and gameplay meaning
separate.

## Dependencies

Allowed dependencies:

- existing public `YuRHI` handle, extent, format, status, device descriptor,
  native surface descriptor, swapchain descriptor, swapchain snapshot,
  capability, capture, present, and device snapshot value contracts;
- new public RHI value-only resize request/result headers;
- existing private Null RHI and D3D11 RHI implementation files;
- Windows SDK, D3D11, DXGI, and COM only inside private Windows RHI
  implementation files or private internal headers;
- `YuPlatform` only in hardware-smoke tests or caller-side conversion fixtures;
- `Tests/Rhi` deterministic tests, `Tests/Rhi` D3D11 hardware-smoke tests, root
  CMake labels, this gate doc, and queue documentation.

Forbidden dependencies:

- `YuPlatform` includes or target linkage from production `YuRHI`;
- Windows SDK, D3D11, DXGI, COM, `HWND`, `HANDLE`, `HRESULT`, `DXGI_FORMAT`,
  `D3D_FEATURE_LEVEL`, WGL, SDL, GLFW, Vulkan, OpenGL, Metal, backend-private
  pointers, or backend storage layout in public RHI headers;
- Resource, Streaming, Package, File, RenderCore, Material, Audio,
  AudioResource, scene, UI, World, Script, Game Adapter, report, screenshot,
  visual-proof, codec, parser, or original-game evidence dependencies from
  RHI public headers or production resize code;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  hardware-only proof, audible proof, or original-game output as evidence.

## Public Contract Boundary

Public RHI contracts may expose value-only descriptors such as:

- `RhiSwapchainResizeRequest` with `RhiExtent2D extent{};`;
- `RhiSwapchainResizeResult` with `RhiStatus status`, previous extent, previous
  `RhiTextureHandle`, current `RhiSwapchainSnapshot`, and `bool resized`;
- `RhiCapabilities::supports_swapchain_resize`;
- `RhiSwapchainSnapshot::resize_count` and
  `RhiSwapchainSnapshot::rejected_resize_count`;
- `IRhiDevice::ResizeSwapchain` using the request/result contracts.

Public RHI contracts must not expose:

- color format or vsync changes in the resize request;
- native window handles, `ID3D11*`, `IDXGI*`, `IUnknown`, `HWND`, `HANDLE`,
  `HRESULT`, `DXGI_FORMAT`, `D3D_FEATURE_LEVEL`, COM ownership, native surface
  objects, or backend storage layout;
- Resource handles, Package ids, File paths, Streaming request ids, RenderCore
  pass ids, material ids, Audio voice ids, scene ids, UI ids, report handles,
  screenshot artifacts, visual-proof types, or Game Adapter types.

`RhiSwapchainSnapshot::color_target.generation` remains the public backbuffer
generation evidence. The first slice must not add a second public generation
field that can drift from the handle.

## First Slice Shape

If approved, the first implementation slice should:

1. Add `RhiSwapchainResizeRequest.h` with only a requested `RhiExtent2D`.
2. Add `RhiSwapchainResizeResult.h` with status, previous extent, previous color
   target, refreshed snapshot, and `resized`.
3. Extend `RhiCapabilities`, `RhiSwapchainSnapshot`, `IRhiDevice`,
   `NullRhiDevice`, and the private D3D11 device implementation for resize.
4. Keep Null RHI unsupported: result status is `UnsupportedBackend`, result
   snapshot is invalid, output target remains empty, and capability is false.
5. Validate resize extent before mutation. Zero extents and extents above the
   current capture fixture bound are rejected without changing current
   swapchain state.
6. Treat same-extent resize as `Success` and `resized = false`, with unchanged
   extent, generation, resize count, and capture target.
7. On a real D3D11 resize, release private render-target-view, backbuffer, and
   capture texture references before the backend resize call, recreate them
   after success, increment the public color-target generation without using
   zero, update the snapshot extent and color target, increment resize count,
   and reset submitted/presented frame state.
8. On native failure or device loss, restore or preserve the last valid public
   snapshot, keep the old color target valid when possible, increment rejected
   resize evidence, and expose one concrete status.
9. Reject stale old swapchain color target handles after a successful real
   resize through existing generation validation paths.
10. Add focused fast tests for default request/result contracts, capability
    defaults, snapshot counter defaults, and Null RHI unsupported behavior.
11. Add focused D3D11 hardware-smoke tests for generation invalidation,
    same-extent no-op, post-resize present, and post-resize capture-byte size.
12. Keep all production RHI public headers free of native/backend, Platform,
    Resource, RenderCore, Audio, File, Package, Streaming, scene, UI, World,
    Script, Game Adapter, report, screenshot, or visual-proof leakage.

## Test And Evidence Policy

Implementation evidence must include:

- changed-path/offscope audit showing only RHI public headers, private RHI
  implementation files, RHI tests, root CMake test registration if needed, this
  gate doc, and queue updates changed;
- `CMakePresets.json` no-drift;
- configure and build for the Windows fast preset;
- focused fast tests for `RHI_SwapchainResize` public contracts and Null RHI
  unsupported behavior;
- full `ctest --preset windows-fast-gate` PASS;
- label discovery for `RHI`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`,
  default `HardwareSmoke`, and `windows-hardware-smoke`;
- default `windows-fast-gate -L HardwareSmoke` discovery remains `0`;
- `windows-hardware-smoke` D3D11 RHI resize tests either PASS on available
  hardware or skip with the existing explicit unavailable-device policy;
- focused hardware-smoke evidence for:
  - real resize changes extent and color-target generation;
  - stale pre-resize target rejects after real resize;
  - same-extent resize keeps generation and resize count stable;
  - post-resize present and capture produce exactly
    `width * height * 4` RGBA8 bytes;
- public-header leak scan for Windows SDK, D3D11, DXGI, COM, Platform,
  Resource, RenderCore, Audio, File, Package, Streaming, scene, UI, World,
  Script, Game Adapter, report, screenshot, and visual-proof symbols;
- production dependency scan proving `YuRHI` does not depend on `YuPlatform`,
  Resource, RenderCore, Audio, File, Package, Streaming, scene, UI, World,
  Script, or Game Adapter;
- proof-shape scan rejecting reports, screenshots, logs, sleeps, manual proof,
  visual inspection, hardware-only proof, audible proof, and original-game
  output;
- style scan for the new public contracts and D3D11 resize implementation.

## Non Goals

This proposal does not approve:

- automatic resize from Platform events;
- changing swapchain format, vsync, buffer count, fullscreen mode, display
  mode, tearing policy, adapter selection, or frame-latency policy;
- RenderCore resize scheduling, frame graph transient-resource aliasing, or
  renderer frame pacing;
- Resource upload integration or asset import paths;
- Audio streaming, File, Package, Streaming, scene, UI, World, Script, or Game
  Adapter behavior;
- screenshots, reports, logs, sleeps, manual visual proof, or original-game
  output as evidence.

## Required Review Before Approval

Before this gate is approved for implementation:

- boundary review must confirm the public resize request/result and snapshot
  fields are backend-neutral values only;
- implementability review must confirm D3D11 resize rollback, COM release
  ordering, capture texture recreation, handle-generation invalidation, and
  same-extent no-op semantics are implementable without new public native
  leakage;
- hardware-evidence review must confirm the optional D3D11 smoke proof shape,
  label isolation, skip policy, and byte-evidence checks;
- test-policy review must confirm default fast-gate determinism and hardware
  isolation remain intact;
- dependency review must confirm production RHI remains independent from
  Platform, Resource, RenderCore, Audio, File, Package, Streaming, scene, UI,
  World, Script, and Game Adapter.
