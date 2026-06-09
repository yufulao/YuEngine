# ADR-0001: Initial Runtime, Build, And Test Shape

Status: Accepted
Owner: 八云紫, 总架构师
Reviewers: 红美铃, 博丽灵梦, 雾雨魔理沙 when code review starts
Date: 2026-06-10
Accepted: 2026-06-10

## Context

Phase 0 is complete enough to move into Phase 1 architecture proposals. The restart plan requires the initial language/build/runtime shape to be decided before code work begins.

The repository is currently clean and minimal. The old YuEngine backup was C++/CMake-based, but it is not a foundation. UE5 is a C++ commercial engine reference, but YuEngine is not copying UE5 APIs or module names.

Phase 1 needs a shape that can support:

- platform and hardware abstraction;
- deterministic headless tests;
- memory, thread, file, logging, and module lifecycle work;
- future RHI/audio/input/resource/script/world systems;
- performance smoke checks from the first implementation slice;
- GitHub-friendly review and CI-style commands.

## Decision

YuEngine's initial runtime/build/test shape is:

- Native runtime language: C++20.
- Build system: CMake.
- Test runner: CTest, with fast gate tests separated from long evidence/oracle suites.
- First platform target: Windows x64 on the current development machine.
- First executable type: headless host/test harness, not a visual demo.
- Source layout: to be finalized by ADR-0002, but implementation must keep runtime, tests, tools, and future game adapter separated.

This decision authorizes architecture review of a C++/CMake/CTest first slice. It does not authorize implementation until the matching module gate proposal is approved.

## Rationale

C++20 is the conservative runtime choice for a self-developed commercial game engine because the first layers require explicit ownership over memory, platform calls, RHI/audio integration, threading, and low-level performance behavior.

CMake and CTest match the old backup's ecosystem and keep the first slice toolchain simple, scriptable, and reviewable. Reusing the toolchain family does not reuse old architecture. Old `CMakeLists.txt` and old tests remain reference/quarantine material until a new source layout and test taxonomy are accepted.

The first target is Windows x64 because the provided original game, local UE5 source, and current workspace are Windows-based. Cross-platform shape remains an architectural requirement, but Phase 1 should not pretend to support more platforms before the platform boundary exists.

The first executable must be headless because visual output would create the same false-progress risk as the failed `Project` attempt. A headless host can prove lifecycle, logging, errors, timers, module startup/shutdown, and performance signals before renderer or UI work exists.

## Consequences

Allowed after review:

- CMake project skeleton.
- CTest fast gate command.
- Headless host executable or test harness.
- Platform/kernel public interfaces required by P1-GATE-001.

Still blocked:

- Renderer demo, blue screen, mesh viewer, or resource viewer.
- TouhouNewWorld title/save/new-game/scene/actor/camera/tutorial work.
- Report dashboard as runtime proof.
- Direct migration from old `FrameRuntime.*`.
- UI, audio business state, RHI real backend, VFS/package parsing, script VM, world/scene runtime.

## Performance Requirements

The first implementation slice must collect deterministic signals required by `docs/YUENGINE_PERFORMANCE_GATES.md`:

- startup allocation count;
- peak bytes and retained bytes if allocator accounting exists in the slice;
- shutdown leak signal;
- disabled logging cost boundary;
- module lifecycle fixture cost;
- service lookup mechanism and whether it is hot-path safe.

If exact numeric budgets are not yet justified, the code slice must still expose a measurement point and a pass/fail rule for the fixture.

## Test Requirements

The first fast gate command must be non-interactive and runnable by agents.

Expected command shape:

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The exact generator and configuration flags may be refined during implementation, but the command family must remain deterministic and documented by the implementation task.

First tests must validate public behavior:

- host can start, tick, and shut down;
- timer produces monotonic frame ticks;
- logging sink can be disabled without changing runtime behavior;
- kernel starts modules in dependency order and shuts them down deterministically;
- service registry resolves a registered service and reports a missing service without silent no-op.

## Alternatives Considered

Unity/C#:

- Rejected for engine runtime layers. It is useful as a product-boundary reference, but YuEngine is a self-developed native engine, not a Unity project.

Rust:

- Deferred. It has strong safety properties, but the existing evidence ecosystem, local references, expected RHI/audio/platform integration, and prior toolchain all point to C++ for the initial runtime.

Custom build system:

- Rejected for Phase 1. It would add tooling work before platform/kernel behavior exists.

Visual demo first:

- Rejected. It repeats the original failure pattern.

## Non-Decisions

This ADR does not decide:

- source tree layout;
- exact module descriptor format;
- allocator implementation;
- thread pool design;
- RHI backend API;
- object/world/component model;
- resource package format;
- scripting VM;
- editor/tool architecture.

Those require later ADRs and gate proposals.

## Gate Request

ADR-0001 allows P1-GATE-001 to request `APPROVED_FOR_FIRST_SLICE` for the build/test plus headless Platform/Kernel bootstrap.

This acceptance does not approve implementation by itself. P1-GATE-001 still requires module-entry and performance sign-off.
