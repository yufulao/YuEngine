# YuEngine Phase 0 Subsystem Reference Map

Status: phase-0 architecture input
Owner: 八云蓝, engine reference research
Task: #引擎参照 task #1
Date: 2026-06-10
Baseline: `docs/YUENGINE_ARCHITECTURE_RESTART_PLAN.md`

## Scope

This document maps UE5 and Unity subsystem boundaries to YuEngine layer and owner decisions before implementation. It compares responsibilities and coupling rules only. It is not a code-port plan and does not copy UE5 or Unity APIs.

Required subsystem scope from the task thread:

- platform
- RHI/render
- resource/package
- audio
- input
- object/world
- scene
- UI
- animation
- physics
- diagnostics
- tools/editor
- testing

## Reference Inputs

UE5 local source root:

- `D:\app\Epic Games\UE_5.5\Engine\Source`

UE5 source areas sampled for boundary shape:

- Runtime core/platform: `Runtime\Core`, `Runtime\ApplicationCore`, `Runtime\Launch`
- Object/world: `Runtime\CoreUObject`, `Runtime\Engine`
- Rendering: `Runtime\RHI`, `Runtime\RHICore`, `Runtime\RenderCore`, `Runtime\Renderer`, `Runtime\NullDrv`, `Runtime\D3D12RHI`, `Runtime\VulkanRHI`
- Resources/packages: `Runtime\PakFile`, `Runtime\AssetRegistry`, `Runtime\Projects`
- Audio: `Runtime\AudioMixer`, `Runtime\AudioMixerCore`, audio codec modules
- Input: `Runtime\InputCore`, `Runtime\InputDevice`
- UI: `Runtime\SlateCore`, `Runtime\Slate`, `Runtime\UMG`, `Runtime\SlateRHIRenderer`
- Animation: `Runtime\AnimationCore`, `Runtime\AnimGraphRuntime`
- Physics: `Runtime\PhysicsCore`
- Diagnostics/testing: `Runtime\TraceLog`, `Runtime\PerfCounters`, `Runtime\AutomationTest`
- Tools/editor separation: `Editor`, `Developer`, `Programs`

Unity official documentation anchors:

- Platform and builds: <https://docs.unity3d.com/6000.4/Documentation/Manual/PlatformSpecific.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/building-introduction.html>
- GameObjects/components/object model: <https://docs.unity3d.com/6000.4/Documentation/Manual/GameObjects.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/Components.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/fundamental-unity-types.html>
- Script lifecycle: <https://docs.unity3d.com/6000.4/Documentation/Manual/class-MonoBehaviour.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/execution-order.html>
- Scenes: <https://docs.unity3d.com/6000.4/Documentation/Manual/CreatingScenes.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/scenes-working-with.html>
- Rendering/SRP: <https://docs.unity3d.com/6000.4/Documentation/Manual/scriptable-render-pipeline-introduction.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/render-pipelines-overview.html>
- Assets/packages: <https://docs.unity3d.com/6000.4/Documentation/Manual/AssetDatabase.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/ImportingAssets.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/PackagesList.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/AssetBundlesIntro.html>
- Audio: <https://docs.unity3d.com/6000.4/Documentation/Manual/Audio.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/AudioOverview.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/AudioMixerOverview.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/class-AudioSource.html>
- Input: <https://docs.unity3d.com/6000.4/Documentation/Manual/Input.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/com.unity.inputsystem.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/UIE-Runtime-Event-System.html>
- UI: <https://docs.unity3d.com/6000.4/Documentation/Manual/UIToolkits.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/UIElements.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/UI-system-compare.html>
- Animation: <https://docs.unity3d.com/6000.4/Documentation/Manual/AnimationSection.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/AnimationOverview.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/class-AnimatorController.html>
- Physics: <https://docs.unity3d.com/6000.4/Documentation/Manual/PhysicsSection.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/PhysicsOverview.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/class-PhysicsManager.html>
- Diagnostics/profiling: <https://docs.unity3d.com/6000.4/Documentation/Manual/Profiler.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/ProfilerMemory.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/ProfilerRendering.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/log-files.html>
- Testing: <https://docs.unity3d.com/6000.4/Documentation/Manual/cus-tests.html>, <https://docs.unity3d.com/6000.4/Documentation/Manual/test-framework/edit-mode-vs-play-mode-tests.html>

## Boundary Rules Derived From References

1. UE5 uses many runtime modules rather than one frame-runtime owner. YuEngine must preserve explicit module owners and avoid a new monolith.
2. UE5 keeps backend graphics modules such as `NullDrv`, `D3D12RHI`, and `VulkanRHI` behind RHI selection. YuEngine needs a null backend and at least one real backend behind the same RHI boundary before renderer work counts.
3. UE5 separates editor/developer/program code from runtime modules. Unity similarly separates runtime concepts from Editor, Package Manager, Asset Database, build profiles, and Test Framework workflows. YuEngine tools/editor must not become runtime dependencies.
4. Unity's GameObject/component and scene model is a useful ownership reference, not an API target. YuEngine should define its own object identity, component or actor model, update phases, and scene lifecycle.
5. Unity SRP shows a useful separation between pipeline code and low-level graphics command execution. YuEngine should keep render scene and frame passes above RHI, with no gameplay code issuing backend commands.
6. Unity Asset Database, package management, and AssetBundle/Addressables documentation show that import/discovery, package distribution, and runtime lifetime are separate concerns. YuEngine should keep VFS/package lookup separate from resource handles and upload scheduling.
7. Diagnostics and profiling are observability surfaces. They may collect logs/traces/counters, but must not define runtime ownership or be required for behavior to run.

## Subsystem Map

| Subsystem | UE5 analogue | Unity analogue | YuEngine layer and owner | Boundary decision | Forbidden coupling | Gate before implementation |
| --- | --- | --- | --- | --- | --- | --- |
| Platform | `Runtime\ApplicationCore`, platform folders under `Runtime\Core`, `Runtime\Launch` | Platform development docs, Player/build profile docs | L0-L1, Engine core | Own process entry, window/headless host, timers, OS event pump, file handles, dynamic library loading, CPU/GPU feature discovery, and platform errors. | No game flow, resource semantics, renderer policy, UI business state, or editor-only code. | Window or headless host can start, tick, and shut down deterministically; timer, event pump, file, dynamic library, and OS error fixtures pass without dependencies above L1. |
| RHI | `Runtime\RHI`, `Runtime\RHICore`, `Runtime\NullDrv`, `Runtime\D3D12RHI`, `Runtime\VulkanRHI` | SRP `ScriptableRenderContext` as command scheduling boundary; render pipeline selection docs | L3, Rendering architecture | Own device, swapchain, command submission, resource handles, present, backend selection, and capture hook. | No material semantics, scene traversal, gameplay visibility, asset import, or UI widget knowledge. | Null RHI and one real backend expose the same interface; create/update/destroy resource, submit, present, and capture fixtures pass with bounded per-frame allocation. |
| RenderCore/Renderer | `Runtime\RenderCore`, `Runtime\Renderer` | SRP, URP/HDRP/render pipeline model, render graph concepts | L5 over L3, Rendering architecture | Own frame passes, render queues, shader/material binding contract, and render scene interface. | No direct backend/device calls outside RHI, no gameplay rules, no UI business flow, no resource import decisions. | Fixture scene renders through RenderCore into RHI and produces a captured output; render scene submits only through RHI-facing contracts. |
| Resource/package | `Runtime\PakFile`, `Runtime\AssetRegistry`, `Runtime\Projects`, package/loose-file patterns | Asset Database, importing assets, Package Manager, AssetBundles | L3-L4, Engine core for VFS/package and Runtime architecture for resource lifetime | VFS/package owns mounts, loose/pack lookup, path normalization, and async read contract; Resource owns asset handles, dependency graph, cache, load/unload, and upload scheduling. | No original-game resource meaning, diagnostics-shaped APIs, renderer/audio backend ownership, or hidden global resource maps. | Pack and loose fixture tests pass; asset dependency and lifetime tests prove handles, cache invalidation, unload, and GPU/audio upload scheduling without game scripts. |
| Audio | `Runtime\AudioMixer`, `Runtime\AudioMixerCore`, audio codec/platform modules | Audio system, AudioSource, AudioMixer, audio assets | L3-L5, Audio architecture | Own device backend, mixer graph, voices, streaming, decoding, latency policy, test sink, and audio-scene bridge. | No BGM/SE business IDs, title/menu state, resource cache ownership, blocking IO in mixer callback, or renderer/gameplay control flow. | Mixer and voice lifecycle tests pass through a test sink; streaming/decode path is explicit; callback or mixer thread has bounded allocation and no blocking IO. |
| Input | `Runtime\InputCore`, `Runtime\InputDevice`, `ApplicationCore` event bridge | Input System package, legacy Input boundary, UI event system docs | L3-L5, Runtime architecture | Own device event normalization, action mapping, focus, routing policy, per-frame input snapshots, and replay fixtures. | No title menu behavior, gameplay command semantics, widget state, or platform event pump ownership. | Recorded device events replay into deterministic action frames; focus/routing tests cover UI/world dispatch without game-specific behavior. |
| Object/world | `Runtime\CoreUObject`, `Runtime\Engine` world/actor concepts | GameObject/component model, fundamental Unity types, MonoBehaviour lifecycle, execution order | L2 and L5, 总架构师 for kernel rules and Runtime architecture for world model | Kernel owns module lifecycle, service registry, object identity, serialization base, event bus, and diagnostics channel. World owns actors/entities/components, update phases, and world lifetime. | No hard-coded title flow, script service state as object ownership, global mutable business singletons, JSON report identity, or lower-layer dependency on original evidence. | Module lifecycle tests prove startup/update/shutdown order; object identity and serialization fixtures pass; world fixture creates, updates, and unloads deterministically. |
| Scene | `Runtime\Engine` levels/world, world partition concepts | Scenes as content containers, create/load/save scenes, multi-scene workflows | L5, Runtime architecture | Own scene/world creation, load/unload, streaming boundary, active scene policy, and update integration. | No TouhouNewWorld stage logic, resource import semantics, renderer backend bypass, UI menu scripts, or gameplay adapter shortcuts. | Scene fixture loads resources through Resource, creates world objects, updates, unloads, and releases handles deterministically. |
| UI | `Runtime\SlateCore`, `Runtime\Slate`, `Runtime\UMG`, `Runtime\SlateRHIRenderer` | UI Toolkit, uGUI, IMGUI comparison, runtime UI event system | L5, Runtime architecture | Own widget tree, lifecycle, layout, focus/navigation, input routing, text path, animation hooks, draw batching, and render submission contract. | No title/save menu scripts, game adapter state, direct RHI device ownership, audio business state, or resource import decisions. | Widget fixture can layout, route input, render through RenderCore/RHI, and capture output; headless mode can run without UI behavior changes. |
| Animation | `Runtime\AnimationCore`, `Runtime\AnimGraphRuntime`, animation integration in `Runtime\Engine` | Animation system, Mecanim, Animator Controller, Animator update modes | L5, Runtime architecture | Own clip/resource model, animation graph or state machine boundary, pose evaluation, event markers, and update integration policy. | No gameplay state machine ownership, physics rule ownership, scene streaming ownership, or direct resource file IO. | Deterministic clip/graph fixture evaluates expected pose and events; physics-sync policy is explicit; animation resources load through Resource handles. |
| Physics | `Runtime\PhysicsCore`, Chaos integration points | Physics integrations, built-in 3D physics/PhysX, Physics settings | L5, Runtime architecture with performance review | Own collision world, rigid bodies/colliders, queries, fixed-step integration, layer/filter policy, and deterministic step contract. | No gameplay damage/rules, animation graph ownership, renderer culling policy, or scene lifetime ownership. | Fixed-step fixture passes deterministically; collision/query/filter tests pass; memory/threading budget and shutdown behavior are documented. |
| Diagnostics | `Runtime\TraceLog`, `Runtime\PerfCounters`, profiling hooks | Unity Profiler, memory/rendering/physics profiler modules, log files | L2 and L7, Quality architecture | Own logging, categories, trace/profiler counters, bounded report adapters, and external dashboard feed points. | No runtime ownership, public core API shape dictated by report schemas, required hot-path allocation, or behavior that changes when diagnostics are disabled. | Diagnostics can be off, category-filtered, or bounded without behavior changes; overhead smoke test exists; runtime modules use diagnostics channel only. |
| Tools/editor | UE `Editor`, `Developer`, `Programs`, UnrealEd, ContentBrowser, cook/package utilities | Unity Editor workflows, Package Manager, Asset Database, UI Builder, build profiles | L7, Tools architecture | Own import/export, packaging, editor UI, build integration, developer utilities, and versioned artifacts outside runtime. | No runtime frame ownership, runtime service initialization, gameplay authoritative state, or lower-layer API changes driven by tool reports. | Tool command or editor harness operates on versioned artifacts while runtime can build/run without editor modules linked. |
| Testing | `Runtime\AutomationTest`, `AutomationWorker`, developer test utilities | Unity Test Framework, Edit mode and Play mode tests, package tests, CI use | L7 with per-module owners, Quality architecture | Own test taxonomy, fast gates, fixture data, backend smoke tests, golden diffs, oracle tests, and performance smoke checks. | No "report exists" as pass condition, no long evidence suite in fast gates, no game adapter tests before lower gates, no tests without owning module/interface. | Each module has at least one named public-interface test before implementation; headless fast gate command is defined; long evidence/oracle suites are separated from fast architecture gates. |

## YuEngine Layer Placement Summary

| YuEngine layer | Subsystems from this map |
| --- | --- |
| L0 Host and Hardware | platform host, OS process/window/timer/hardware discovery |
| L1 Platform Services | platform services, file handles, low-level input/audio/graphics device discovery |
| L2 Engine Kernel | module lifecycle, service registry, object identity, diagnostics channel |
| L3 Low-Level Runtime Interfaces | RHI, audio backend/mixer, input action layer, VFS/package, async IO |
| L4 Core Asset and Script Framework | resource handles, dependency graph, serialization/script bridge inputs |
| L5 Runtime World Systems | world, scene, render scene, UI, animation, physics, audio scene |
| L6 Game Adapter Layer | no direct implementation authorized by this map |
| L7 Verification and Tools | diagnostics adapters, tools/editor, import/export, packaging, testing |

## Gate Checklist Template

Every implementation slice derived from this map must state:

1. Owning module and owner role.
2. Layer and allowed downward dependencies.
3. Inputs and outputs.
4. Lifecycle: create, update/use, shutdown/release.
5. Performance constraint: allocation, threading, IO, or hot-path budget.
6. Test fixture or harness that proves behavior.
7. UE5/Unity reference analogue from this document.
8. Forbidden coupling check.

If any item is missing, the slice returns to architecture or gate review before code starts.

## Immediate Architecture Decisions For YuEngine

1. Treat UE5 module layout as evidence for ownership separation, not as a module-name template.
2. Treat Unity GameObject/scene/SRP/input/UI systems as product-boundary references, not API references.
3. Put RHI, audio backend, input replay, VFS/package, and diagnostics channel below world systems before gameplay exists.
4. Keep Resource package lookup, asset lifetime, and backend upload as separate concerns.
5. Require null/test backends for RHI, audio, input, and resource fixtures before real backend progress is counted.
6. Keep tools/editor and reports outside runtime ownership.
7. Do not allow TouhouNewWorld evidence or old `FrameRuntime.cpp` shape to define lower-layer interfaces.

## Residual Risks

- Unity documentation is public boundary documentation, not source-level implementation. Use it for product and workflow boundaries only.
- UE5 source layout is large; this map samples major module descriptors and directories relevant to the task scope. Detailed ADRs should inspect the exact module before adopting a specific boundary.
- Godot was not included because the task acceptance explicitly requested UE5 local source plus official Unity docs. A later comparison can add Godot if architecture wants a third reference.
