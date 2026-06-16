# P2-GATE-004: Test Tier Labels And Hardware Smoke Presets

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: ADR-0005, ENG-096
Related decisions: ADR-0002, ADR-0006
Source baseline: `0bbcd26`

## Layer

L7 verification support for L0-L3 hardware and lower-engine gates.

This gate prepares the test execution structure needed before real Platform,
RHI, D3D11, Audio, Input, Thread, or async IO gates start landing. It does not
add runtime behavior. It does not approve real devices, real graphics backends,
new lower-layer APIs, or removal of deterministic tests.

The first slice should make the existing test fleet easier to target:

```text
existing deterministic tests
-> CTest labels by tier and module
-> default windows-fast-gate remains deterministic
-> optional hardware-smoke preset can exist without blocking fast gate
-> future D3D11/WASAPI/window tests have a clear landing lane
```

## Owns

This gate owns a first slice for:

- CTest labels for existing tests;
- tier labels: `Fast`, `ModuleFixture`, `PerformanceSmoke`, `HardwareSmoke`,
  and `EvidenceOracle`;
- module labels: `Platform`, `Kernel`, `Memory`, `Diagnostics`, `Thread`,
  `File`, `RHI`, `Audio`, `Input`, `Resource`, `Package`, `Object`,
  `Serialize`, `Script`, and `World`;
- future backend labels: `D3D11`, `WASAPI`, `Win32`, and `AsyncIO`;
- preserving the existing `windows-fast-gate` configure/build/test preset as
  the default deterministic required gate;
- adding an optional hardware-smoke preset or documented preset slot that can be
  used by real-device gates after they exist;
- documenting when targeted tests, full fast gate, and optional hardware smoke
  are required during implementation, QA, and landing;
- CMake helper or grouped registration changes if they reduce label drift
  without changing test behavior.

## Does Not Own

This gate does not own:

- real Platform window, native surface, or OS event pump behavior;
- D3D, DXGI, swapchain, GPU device, shader, pipeline, buffer, mesh, or draw
  behavior;
- real audio devices, callback threads, codecs, or streaming;
- OS input devices or focus policy;
- worker thread pool, job graph, async IO, package streaming, or upload queue;
- RenderCore, material system, UI, World, scene policy, tools, reports, oracle,
  or Game Adapter behavior;
- deleting deterministic tests to reduce count;
- moving tests out of `windows-fast-gate` without an explicit approved reason;
- treating skipped hardware smoke tests as proof that a real backend works.

## Dependencies

Allowed dependencies:

- root `CMakeLists.txt`;
- `CMakePresets.json`;
- existing CTest test names and executable targets;
- existing ADR-0005 test taxonomy;
- existing ENG-096 hardware-first sequencing decision.

Forbidden dependencies:

- changes under `Src/YuEngine` for runtime behavior;
- new Platform, RHI, Audio, Input, Thread, File, Resource, Package, RenderCore,
  World, UI, tools, report, or Game Adapter implementation;
- third-party test framework adoption;
- screenshots, log parsing, JSON report parsing, or external evidence files as
  proof of test behavior.

## Lifecycle

The first-slice lifecycle is:

1. Configure `windows-fast-gate`.
2. Register every existing test with at least one tier label and one module
   label.
3. Keep `windows-fast-gate` execution behavior equivalent to the current full
   deterministic test set.
4. Allow focused local execution by CTest label and test-name filter.
5. Provide an optional hardware-smoke preset or reserved preset shape for future
   tests that require real OS/GPU/audio devices.
6. Keep optional hardware smoke separate from default deterministic fast gate.

Failure behavior:

- label assignment errors must fail configure or discovery clearly;
- the default fast gate must still discover all deterministic tests;
- optional hardware-smoke discovery may report no tests before real hardware
  smoke tests exist, but it must not mask deterministic fast-gate failures;
- unsupported hardware environments must not be counted as proof for promoting a
  real backend milestone.

## Inputs

- current CTest test registry;
- current `windows-fast-gate` configure/build/test presets;
- module ownership implied by each test executable and test name;
- ENG-096 ordering and test-cost recommendations.

## Outputs

- labeled CTest registry;
- preserved `windows-fast-gate` default preset;
- optional hardware-smoke preset or documented preset slot;
- documented command family for focused module/tier execution;
- explicit rule for when full fast gate and optional hardware smoke are required.

## Performance Constraints

Required deterministic signals:

- full deterministic test discovery count remains stable unless a later
  implementation change intentionally adds tests;
- label-filtered discovery works for at least `Fast`, `RHI`, `Audio`,
  `Platform`, and `World`;
- configure/build/test preset changes do not add runtime dependencies;
- CMake registration remains readable and reviewable.

Blocking conditions:

- default fast gate stops running existing deterministic tests;
- real-device tests become required by default without an explicit hardware
  environment contract;
- tests are removed, skipped, or hidden to reduce count;
- labels are broad enough to lose module ownership;
- hardware proof depends on screenshots, logs, reports, or manual visual checks
  instead of public-interface assertions and capture/oracle fixtures owned by
  later gates.

## Required Tests And Checks

Required non-interactive checks for the first slice:

- `cmake --preset windows-fast-gate`
- `cmake --build --preset windows-fast-gate`
- `ctest --preset windows-fast-gate -N`
- `ctest --preset windows-fast-gate`
- label discovery for `Fast`, `RHI`, `Audio`, `Platform`, and `World`
- optional hardware-smoke preset discovery, if the preset is added in this slice
- `git diff --check`

The implementation handoff must record:

- full deterministic discovery count before and after;
- label-filtered discovery counts for changed label families;
- targeted command examples for one lower module and one upper module;
- whether an optional hardware-smoke preset is present and whether it currently
  has zero or more tests.

## Allowed First Slice

If approved, the first implementation slice may modify only:

```text
CMakeLists.txt
CMakePresets.json
docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md
docs/YUENGINE_PHASE3_ARCHITECTURE_QUEUE.md
```

It may add no production source files and no runtime test behavior unless the
review explicitly finds a tiny CMake-only verification target necessary.

## Non-Goals

- No D3D11 implementation.
- No OS window implementation.
- No real audio implementation.
- No async IO or worker implementation.
- No RenderCore, mesh, material, UI, World, or Game Adapter expansion.
- No replacement of CTest.
- No reduction of test evidence by deletion or silent skip.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- 八云蓝 confirms the label tiers match mature-engine responsibility boundaries
  and do not blur Platform/RHI/RenderCore/Game responsibilities;
- 博丽灵梦 confirms the policy lowers execution friction without weakening
  boundary and performance evidence;
- 雾雨魔理沙 confirms the required checks are implementable and the default fast
  gate remains deterministic;
- 八云紫 confirms this gate stays ahead of real hardware gates in the Phase 2
  sequence.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_TEST_POLICY`, or `NEEDS_IMPLEMENTABILITY` with exact missing fields.
