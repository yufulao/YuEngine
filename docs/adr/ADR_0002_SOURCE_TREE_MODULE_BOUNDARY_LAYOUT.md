# ADR-0002: Source Tree And Module Boundary Layout

Status: Proposed
Owner: 八云紫, 总架构师
Reviewers: 红美铃, 雾雨魔理沙 when code review starts, 博丽灵梦 for performance-relevant layout
Date: 2026-06-10
Depends on: ADR-0001

## Context

ADR-0001 proposes C++20, CMake, CTest, Windows x64 first, and a headless-first executable shape. P1-GATE-001 needs a source layout decision before implementation can create files without improvising ownership.

The source tree must prevent the old YuEngine failure mode:

- monolithic runtime aggregation;
- diagnostics/report output becoming core API;
- original-game evidence shaping lower-layer interfaces;
- tools/editor code entering runtime ownership;
- tests validating report existence instead of module behavior.

The layout must make module ownership visible from paths and CMake targets.

## Decision

YuEngine will use a module-oriented native source tree:

```text
YuEngine/
  CMakeLists.txt
  CMakePresets.json
  cmake/
  src/
    yuengine/
      platform/
        include/yuengine/platform/
        src/
      kernel/
        include/yuengine/kernel/
        src/
      diagnostics/
        include/yuengine/diagnostics/
        src/
      memory/
        include/yuengine/memory/
        src/
      thread/
        include/yuengine/thread/
        src/
      file/
        include/yuengine/file/
        src/
      rhi/
      audio/
      input/
      resource/
      script/
      world/
      ui/
  apps/
    yuengine_headless/
  tests/
    platform/
    kernel/
    diagnostics/
    memory/
    thread/
    file/
  tools/
  samples/
  docs/
```

Only directories required by an approved slice should be created during implementation. The full tree above is the architectural map, not a request to create empty speculative directories.

## CMake Target Rules

Each runtime module is a separate CMake target when it contains code.

Initial target naming:

- `YuPlatform`
- `YuKernel`
- `YuDiagnostics`
- `YuMemory`
- `YuThread`
- `YuFile`
- `YuHeadless`

Test targets use the module name plus `Tests`, for example:

- `YuPlatformTests`
- `YuKernelTests`

Rules:

- Runtime targets may depend only downward according to the layer map.
- Test targets may depend on the public target they validate and local test fixtures.
- `apps/*` may assemble runtime modules, but app code must not become module implementation.
- `tools/*` is never linked into runtime targets.
- `docs/*` is not a source dependency.
- Old backup paths are not CMake inputs.

P1-GATE-001 target dependency rule:

```text
YuHeadless
  -> YuKernel
  -> YuPlatform
  -> YuDiagnostics
```

Rules for the first slice:

- `YuHeadless` may assemble the approved headless host only.
- `YuKernel` may depend on `YuPlatform` for host/platform abstractions and `YuDiagnostics` for minimal logging events.
- `YuPlatform` may depend on `YuDiagnostics` only for the minimal logging sink required by P1-GATE-001.
- `YuDiagnostics` must not depend on `YuKernel`, `YuPlatform`, `YuHeadless`, original-game evidence, tools, reports, capture, oracle, profiler dashboards, or editor code.
- No other runtime targets may be introduced by P1-GATE-001.

`YuDiagnostics` scope in P1-GATE-001:

- allowed: default logging sink, disabled logging sink, bounded in-memory test observation needed to prove disabled logging is behavior-neutral;
- blocked: diagnostics channel architecture, report schema, JSON runtime API, capture/oracle ownership, profiling API, dashboard feed, trace file format, or runtime behavior ownership.

Broader diagnostics work belongs to ADR-0004 and P1-GATE-004.

## Include Boundary

Public headers live under:

```text
src/yuengine/<module>/include/yuengine/<module>/
```

Private implementation lives under:

```text
src/yuengine/<module>/src/
```

Call sites include public headers through `yuengine/<module>/...`.

Cross-module include rule:

- Downward public headers may be included.
- Private `src` paths must never be included from another module.
- If a module needs private knowledge from another module, the boundary is wrong and must return to architecture review.

## Initial Build Presets

ADR-0001's non-blocking review note requires the Windows generator/config shape to be explicit. The first implementation slice should add CMake presets rather than relying on each agent's local default.

Required preset intent:

- `windows-debug`: configure a Windows debug build.
- `windows-release`: configure a Windows release build if needed later.
- `windows-fast-gate`: configure/build path used by fast CTest gate.

The implementation task may choose the exact generator based on installed tooling, but it must document the chosen generator/config command in the implementation handoff.

If Visual Studio multi-config is used, test commands must include the configuration, for example:

```text
ctest --test-dir build --build-config Debug --output-on-failure
```

If a single-config generator is used, the command may omit `--build-config`.

## First Slice Layout

P1-GATE-001 may create only:

```text
CMakeLists.txt
CMakePresets.json
cmake/
src/yuengine/platform/include/yuengine/platform/
src/yuengine/platform/src/
src/yuengine/kernel/include/yuengine/kernel/
src/yuengine/kernel/src/
src/yuengine/diagnostics/include/yuengine/diagnostics/
src/yuengine/diagnostics/src/
apps/yuengine_headless/
tests/platform/
tests/kernel/
tests/diagnostics/
```

Memory/thread/file directories are not created until their own gates are approved unless P1-GATE-001 reviewers explicitly approve a placeholder-free accounting hook.

## Runtime/Tools Separation

Runtime code:

- lives under `src/yuengine`;
- owns behavior;
- must not depend on `tools`, `samples`, `docs`, original-game evidence, or report schemas.

Tools code:

- lives under `tools`;
- consumes runtime outputs or offline inputs;
- may depend on file formats and reports;
- does not define runtime ownership.

There is no `tools_support` runtime module in Phase 1. If future tooling needs shared support code, it must enter through a later L7/tools-only gate and must not be linked into runtime targets without a separate architecture decision.

Samples:

- live under `samples`;
- are fixtures or demonstration inputs only after gates allow them;
- do not define APIs.

Game Adapter:

- is intentionally absent from the first layout;
- will be introduced only after lower runtime gates pass.

## Test Layout

Fast gate tests live under `tests/<module>`.

Rules:

- Tests validate public interfaces.
- Tests do not include module-private paths from another module.
- Fast tests do not require TouhouNewWorld data.
- Long evidence/oracle suites are not part of the fast gate.
- Test fixture data, if needed, lives under `tests/fixtures/<module>` and must be small, deterministic, and not copied from original-game assets unless a later evidence gate approves it.

## Consequences

Positive:

- Module ownership is visible in paths and targets.
- Public/private boundaries are testable by include paths.
- Tools and runtime cannot accidentally merge.
- P1-GATE-001 can proceed without inventing layout during implementation.

Costs:

- More CMake target structure than a single executable.
- More early discipline before visible output exists.
- Some future refactoring may still be needed when RHI/audio/resource/script modules become real.

## Alternatives Considered

Single `src/` plus folders:

- Rejected. It is too easy to recreate a `FrameRuntime.cpp`-style aggregation center.

Flat `include/` and `src/` split:

- Deferred. It can work for libraries, but per-module public/private layout makes ownership harder to violate in this project.

Full tree creation immediately:

- Rejected. Empty speculative directories create a false sense of architecture completion.

Unity-style `Assets/` project layout:

- Rejected. Unity remains a product/workflow reference, not the runtime source layout model.

## Non-Decisions

This ADR does not decide:

- exact class names;
- exact module descriptor format;
- allocator implementation;
- diagnostics API shape;
- package format;
- RHI backend target;
- game adapter layout.

## Gate Impact

If accepted, ADR-0002 removes the source-layout uncertainty from P1-GATE-001.

P1-GATE-001 may then create only the first-slice paths listed above and must not create placeholder directories for future systems.
