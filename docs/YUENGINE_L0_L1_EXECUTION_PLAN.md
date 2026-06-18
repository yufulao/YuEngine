# YuEngine L0/L1 Execution Plan

Status: handoff plan for landing team
Owner: 八云紫, 总架构师
Requested: 2026-06-19
Current observed code checkpoint: `278d361`
Scope: progress adjustment, L0 closure plan, L1 runtime-core plan

## 1. Executive Decision

YuEngine should not continue as a sequence of isolated first-slice gates. The
current project has valuable lower-engine proof, but the next correction is to
turn proof slices into vertical runtime closure.

The immediate adjustment is:

```text
Stop saying: hardware layer is complete
Say instead: L0 hardware-facing lower layer has a closure candidate

Stop expanding: more World bridge slices as proof of engine progress
Start closing: L0 matrix, one real engine sample, then L1 runtime vertical slice

Stop optimizing for: UE/Unity ecosystem breadth
Optimize for: a clean internal commercial engine for one known game team
```

UE and Unity remain reference engines for responsibility boundaries, failure
models, production tooling, and performance discipline. YuEngine should not
copy their public ecosystem, universal plugin surface, editor extension market,
multi-industry platform scope, or generic API breadth.

## 2. Current Progress Assessment

### 2.1 What Is Solid

The current mainline has meaningful first-slice coverage in the lower engine:

- Platform window/native surface/event pump.
- RHI backend-neutral boundary and D3D11 clear/present/capture path.
- D3D11 resource/pipeline primitives, visible triangle, static mesh, texture
  sampling, and swapchain resize contract.
- Audio test backend/mixer, PCM packet, PCM stream queue, XAudio2 callback
  first slice, and Resource-to-Audio packet import bridge.
- Input replay/action snapshot, Win32 bridge, and XInput bridge.
- File/VFS, package/load-plan, async file-completion, package/resource staging,
  Resource upload queue, upload completion commit, residency, cache payload,
  decode plan/result/decoded payload ownership.
- RenderCore fixture pass, material binding, submission batch, frame packet,
  graph skeleton, graph execution plan, shader/camera/draw/view-packet
  contracts.
- Object, Serialize, Script, World, and many World bridge slices exist as
  first-slice contracts.
- Fast gate and hardware-smoke coverage have grown substantially.

This is real progress. It should not be rolled back wholesale.

### 2.2 What Is Not Yet Solid

The current state is not yet a complete hardware layer or runtime engine:

- Many modules are still fixture/value/bridge proof, not production runtime
  behavior.
- Hardware smoke still contains environment-dependent skipped paths.
- The sample/evidence path is being cleaned from `Temp/AssetSmokeDemo` into
  more appropriate sample/tool/third-party locations; that cleanup must finish
  before the sample is treated as commercial evidence.
- L1/World bridge work has advanced faster than the L0 closure and can create
  dependency sprawl if it continues unchecked.
- `Bridge` classes are numerous and must be audited so they stay narrow
  adapters instead of hidden owners of policy or lifecycle.
- There is not yet a single engine-owned vertical sample that proves:

```text
window -> input -> resource read/decode -> upload -> render -> audio -> resize -> shutdown
```

### 2.3 Immediate Progress Adjustment

The landing team should make these changes now:

1. Freeze new broad feature gates for a short window.
2. Produce and maintain an L0 completion matrix.
3. Finish repository hygiene for sample/assets/third-party binaries.
4. Convert lower-engine proof into one engine sample.
5. Pause new World/Game Adapter/UI/gameplay expansion until L0 closure gates
   pass.
6. Reframe L1 work from "more bridges" to "runtime core vertical closure".

## 3. Updated Layer Model

The old restart plan used a broader L0-L7 diagram. For the next execution
window, use this simplified delivery model.

### L0: Platform / Hardware-Facing Lower Layer

L0 owns the stable lower-engine abilities that touch OS, device, IO,
backend-specific systems, and frame submission.

Included modules:

- `YuPlatform`
- `YuRhi`
- D3D11 backend internals
- `YuAudio`
- `YuInput`
- `YuFile`
- `YuThread`
- `YuMemory`
- `YuDiagnostics`
- lower `YuPackage` load-plan primitives
- lower `YuResource` / `YuStreaming` upload/decode/cache primitives
- lower `YuRenderCore` frame/draw/material/view/graph execution primitives
- `YuHardware` / FrameHost integration shell

L0 must not own:

- gameplay rules
- scene policy
- UI business
- old game package compatibility
- project-specific BGM/SE IDs
- TouhouNewWorld title/save/new-game logic
- editor ecosystem
- public plugin contracts

L0 exit condition:

```text
An engine-owned sample can run on the target Windows/D3D11/XAudio2/XInput path,
or explicitly skip unavailable hardware with graded reasons, while Debug,
Release, fast gate, hardware smoke, and sample smoke remain reproducible.
```

### L1: Runtime Core / Engine Framework Layer

L1 owns the project-independent runtime framework that a real game project uses
to create, update, save, load, render, and debug a scene.

Included modules and future modules:

- `YuKernel` runtime services and game loop integration
- `YuObject`
- `YuSerialize`
- `YuScript` native bridge
- `YuWorld`
- Scene lifecycle and scene assembly
- Transform/object/component model
- Asset Manager over Resource/Package/File
- Render Scene over RenderCore/RHI
- Audio Scene over Audio/Resource
- Input Mapping over Input devices/actions
- Save/Profile/Config primitives
- Runtime diagnostics surface

L1 must not own:

- D3D11/XAudio2/Win32 native details
- project-specific title flow
- old package compatibility
- editor plugin ecosystem
- broad public scripting ecosystem
- gameplay-specific object types unless promoted by repeated use

L1 exit condition:

```text
A small project scene can load, instantiate objects/components, bind resources,
respond to input, submit render/audio through L0, save/reload core state, and
shut down cleanly without project-specific shortcuts.
```

### L2: Project / Product Layer

L2 is not part of this document's implementation plan, but its boundary matters.

L2 owns:

- game-specific actors, enemies, stages, combat, UI screens, save semantics,
  project schema, project data, project-specific tools

L2 must not backflow into L0 or L1. If a project need exposes a missing engine
capability, the work returns to the owning L0/L1 gate.

### Tooling / QA Plane

Tooling is cross-cutting and not a runtime dependency layer.

It includes:

- asset cooker
- validator
- sample runner
- packager
- profiler
- debug overlay
- frame/audio/resource capture tools
- CI/test presets

Tooling may observe runtime state through explicit diagnostic/capture
interfaces. It must not own runtime behavior.

## 4. Non-Goals For The Next Window

Do not build these now:

- Public developer ecosystem.
- Marketplace or plugin store.
- Public stable plugin ABI.
- General editor extension platform.
- UE/Unity-compatible scripting ecosystem.
- Multi-backend rendering abstraction beyond the current target closure.
- Old TouhouNewWorld package/runtime compatibility layer.
- Generic ECS/DOTS/reflection universe.
- UI business or title menu.
- Gameplay systems.
- Network/cloud/account/store systems.
- Full editor.

Do build narrow internal production tools where they directly unblock the
engine sample, asset validation, profiling, or packaging.

## 5. L0 Execution Plan

### L0-0: Completion Matrix And Freeze

Duration: 1 to 2 days

Owner split:

- Architect: definitions and acceptance language
- PM/gate owner: state matrix and sequencing
- QA/code review: current risks and verification commands
- Performance owner: hot-path and allocation expectations
- Evidence owner: sample/third-party/licensing provenance

Deliverable:

`docs/YUENGINE_L0_COMPLETION_MATRIX.md`

Matrix columns:

```text
Subsystem
Current module/files
Current gate or commit evidence
State: Done / FirstSlice / BlockedByEnv / Deferred / Rework
Required closure evidence
Required test command
Allowed skips
Forbidden scope
Owner
Next action
```

Minimum subsystem rows:

- Platform window/event/native surface
- RHI backend-neutral contract
- D3D11 device/swapchain/present/capture
- D3D11 resource/pipeline primitives
- RHI visible triangle/static mesh/texture sampling
- RHI resize
- RenderCore frame packet/view packet/draw/material/graph execution
- File/VFS loose read/write and path policy
- Package load-plan/staging
- Thread worker / async file-completion
- Resource cache/decode/upload/commit/residency
- Audio mixer/test backend
- XAudio2 callback
- Audio PCM packet/stream queue
- Audio Resource import bridge
- Input replay/action
- Win32 input bridge
- XInput bridge
- Diagnostics/memory/thread cost surfaces
- HardwareFrameHost
- Engine sample
- Third-party/sample asset hygiene

Acceptance:

- Matrix is committed.
- Every row has a state and next action.
- Skipped hardware tests are graded, not buried in PASS language.
- No row claims "complete" without a reproducible command and failure model.

Stop conditions:

- Any row cannot name its owner.
- Any bridge row owns lifecycle for both sides.
- Any sample depends on isolated binaries not built through YuEngine.

### L0-1: Repository And Sample Hygiene

Duration: 1 to 2 days, can overlap with matrix

Goal:

Remove the current evidence weakness around temporary sample assets and
third-party binaries.

Tasks:

1. Move engine sample source under `Samples/`.
2. Move helper tools under `Tools/`.
3. Move third-party runtime binaries or source declarations under `ThirdParty/`
   with license/provenance notes.
4. Ensure generated sample binaries and capture outputs are ignored.
5. Ensure sample assets are either source-controlled intentional fixtures or
   generated by a documented tool.
6. Add sample README explaining:

```text
what is built
what uses YuEngine build outputs
what third-party artifacts are used
what command runs the sample
what result proves
what result does not prove
```

Acceptance:

- No committed executable/dll/capture output under `Temp/`.
- `git status` and `git diff --check` are clean after intended commits.
- Sample can be built or intentionally skipped by a documented command.
- License/provenance for third-party audio/image/codec artifacts is explicit.

### L0-2: Platform And Frame Pump Closure

Duration: 2 to 3 days

Goal:

Make the window/event/native-surface boundary stable enough for RHI and
FrameHost.

Tasks:

1. Confirm Win32 window lifecycle:
   - create
   - show/hide if required
   - resize
   - focus
   - close
   - destroy
2. Confirm event pump behavior:
   - bounded event queue
   - deterministic drain
   - no hidden sleeping in tests
   - close event maps to engine state
3. Confirm native surface descriptor:
   - opaque public value
   - no public HWND/HINSTANCE leak unless explicitly gated
   - D3D11 backend consumes through private adapter path
4. Confirm error behavior:
   - invalid descriptors rejected
   - double destroy is safe or explicit failure
   - unsupported headless/window mode is explicit

Acceptance tests:

- Platform create/destroy fixture
- resize/focus/close event ordering fixture
- native surface descriptor validation fixture
- FrameHost can consume Platform events without owning Platform internals

Forbidden:

- UI routing
- gameplay input
- Resource/RenderCore ownership
- public backend-native type leaks

### L0-3: RHI And D3D11 Closure

Duration: 4 to 7 days

Goal:

Complete one stable D3D11-backed path through the backend-neutral RHI boundary.

Tasks:

1. Backend-neutral device contract:
   - capability flags
   - swapchain support state
   - error statuses
   - resource handle generation
2. D3D11 device/swapchain:
   - create
   - clear
   - present
   - capture bytes
   - resize
   - shutdown
3. Resource primitives:
   - buffer
   - texture
   - sampler
   - shader
   - input layout
   - pipeline
   - fence/retirement ledger
4. Draw proof:
   - visible triangle
   - indexed static mesh
   - texture sampling
5. Resize proof:
   - backbuffer generation changes
   - stale color target rejected
   - RenderCore receives explicit resize state later, not in this gate
6. Failure proof:
   - unsupported device
   - invalid descriptor
   - stale handle
   - resource creation failure where possible

Acceptance tests:

- existing `windows-fast-gate` remains deterministic
- `windows-hardware-smoke` D3D11 rows pass on a supported machine
- capture-byte assertions prove behavior
- no screenshot/manual proof requirement

Forbidden:

- scene traversal
- material graph
- shader compiler/toolchain expansion
- World/UI/Game Adapter
- Resource owning backend-native objects
- public D3D11 header leakage

### L0-4: RenderCore Lower Closure

Duration: 3 to 5 days

Goal:

Make RenderCore a stable frame-assembly layer over RHI without becoming scene
or gameplay.

Tasks:

1. View packet:
   - camera constants
   - viewport/scissor
   - frame index
   - resize awareness as value input only
2. Draw packet:
   - geometry handles
   - material binding values
   - shader program values
   - deterministic ordering rules
3. Frame pipeline:
   - prepare
   - validate
   - submit one or more bounded batches
   - present through RHI-owned swapchain path
4. Graph skeleton/execution:
   - declared passes
   - dependencies
   - no transient aliasing yet
   - no command-list parallelism yet
5. Failure:
   - missing resource
   - stale handle
   - graph validation failure
   - unsupported backend capability

Acceptance:

- RenderCore sample path submits through RHI only.
- RenderCore does not include D3D11 headers.
- RenderCore does not depend on World, UI, Script, or Game Adapter.
- Frame packet tests prove no hidden growth in hot fixture paths.

Forbidden:

- scene renderer
- camera components
- material graph editor
- shader compiler pipeline
- UI rendering

### L0-5: File, Package, Resource, Streaming Closure

Duration: 5 to 8 days

Goal:

Create a real lower asset path for the L0 sample without building a full asset
pipeline.

Tasks:

1. File/VFS:
   - loose read
   - loose write if needed by sample/tools
   - path normalization
   - mount ordering
   - small fixture bounds
2. Package:
   - manifest/load-plan values
   - staging queue
   - no original package parser expansion in L0 closure
3. Resource:
   - handle validation
   - cache payload
   - decode plan
   - decode result
   - decoded payload ownership
   - residency budget
4. Streaming:
   - upload queue
   - upload completion commit
   - deterministic counters
5. Bridges:
   - Resource decoded texture -> RHI upload request
   - Resource decoded PCM -> Audio packet request
6. Sample path:
   - at least one texture or mesh fixture travels through File/Resource/Upload
   - at least one PCM packet or stream queue fixture travels through Audio path

Acceptance:

- One sample asset path uses YuEngine modules rather than a standalone demo.
- Resource core does not own RHI device lifecycle.
- Audio core does not parse Resource payloads directly.
- File/Package does not know about World or project gameplay.

Forbidden:

- old game package compatibility
- full cooker
- hot reload
- scene loader
- game-specific resource IDs
- report-shaped APIs

### L0-6: Audio Closure

Duration: 3 to 5 days

Goal:

Make audio hardware/backend proof honest and repeatable.

Tasks:

1. Test backend/mixer remains deterministic.
2. PCM packet records are stable.
3. PCM stream queue drains into caller-owned output.
4. XAudio2 callback path:
   - initializes on supported machines
   - gives explicit unavailable status on unsupported machines
   - does not silently pass when device is missing
   - avoids allocation/blocking in callback path
5. Resource-to-Audio bridge:
   - maps metadata only
   - does not move decoded byte ownership into Audio bridge

Acceptance:

- Fast tests prove deterministic mixer/packet/queue behavior.
- Hardware smoke has at least one supported-machine XAudio2 PASS path, or a
  documented environment skip row in the L0 matrix.
- Callback path has no file IO, locks, sleeps, or unbounded allocation.

Forbidden:

- BGM/SE business IDs
- audio scene
- UI volume menu
- gameplay audio events
- codec ecosystem beyond sample need

### L0-7: Input Closure

Duration: 2 to 4 days

Goal:

Make input device events stable before L1 action/gameplay mapping expands.

Tasks:

1. Win32 keyboard/mouse bridge:
   - deterministic event translation
   - focus/close/resize separation
   - bounded queue behavior
2. XInput bridge:
   - available/unavailable
   - connect/disconnect
   - stable gamepad index behavior
   - no silent success on missing hardware
3. Input core:
   - replayable snapshots
   - action values
   - no platform-native leak

Acceptance:

- Fast tests prove replay/action behavior.
- Hardware smoke proves or explicitly skips XInput based on environment.
- L1 receives value snapshots only.

Forbidden:

- UI navigation
- text input framework
- gameplay command mapping
- World or Script callbacks

### L0-8: Diagnostics, Memory, Thread Cost Closure

Duration: 2 to 4 days

Goal:

Ensure the L0 sample and lower-engine paths can be debugged without reports or
diagnostics owning runtime behavior.

Tasks:

1. Memory:
   - owner/tag vocabulary
   - allocation counters where available
   - limitation notes for untracked CRT/STL/general heap paths
2. Thread:
   - worker lifecycle
   - async completion queue
   - deterministic shutdown
   - no sleep-based proof
3. Diagnostics:
   - bounded events/counters
   - disabled behavior neutrality
   - no report-driven core API
4. Performance smoke:
   - hot fixture paths do not grow containers unexpectedly
   - sample path has basic frame/resource/audio counters

Acceptance:

- Diagnostics can be disabled without changing behavior.
- Memory/thread/diagnostic tests stay in fast gate.
- Sample emits bounded optional diagnostics only.

Forbidden:

- JSON reports as runtime API
- unbounded logging on hot paths
- report/capture/oracle ownership inside L0 modules

### L0-9: HardwareFrameHost And Engine Sample

Duration: 4 to 7 days

Goal:

Prove L0 as an engine-owned vertical path.

Sample flow:

```text
Start process
Create platform window
Create native surface descriptor
Create RHI device/swapchain
Load small fixture asset through File/Resource path
Upload texture or mesh resource through Streaming/RHI path
Prepare RenderCore view/draw/material packet
Poll Input snapshot
Submit frame
Present
Optionally play PCM stream
Handle resize
Shutdown cleanly
```

Acceptance commands:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
ctest --preset windows-hardware-smoke --output-on-failure
cmake --preset windows-release
cmake --build --preset windows-release
ctest --test-dir build/windows-release-vs -C Release --output-on-failure
```

The exact release build directory should follow the active preset output path
used by the repository.

Acceptance:

- Sample is built from YuEngine source tree.
- Sample does not depend on committed generated binaries.
- Sample proves a real module path, not an isolated demo.
- Hardware unavailable cases are explicit skip/fail states.
- No project gameplay is introduced.

## 6. L1 Execution Plan

L1 should begin only after the L0 matrix is accepted and the L0 vertical sample
path is either complete or has documented blockers that do not affect L1 design.

### L1-0: Runtime Core Freeze

Duration: 1 to 2 days

Deliverable:

`docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`

Rows:

- Kernel/game loop
- Frame context/time
- Object registry
- Component identity
- Transform records
- World lifecycle
- Scene assembly
- Asset manager
- Render scene
- Audio scene
- Input mapping
- Script native bridge
- Serialization/save
- Config/profile
- Runtime diagnostics

Each row states:

```text
Owner
Depends on L0 modules
Allowed L1 dependencies
Forbidden dependencies
Current first-slice status
Required vertical-slice evidence
Performance risk
Test route
```

### L1-1: Runtime Kernel And Game Loop

Duration: 3 to 5 days

Goal:

Provide a runtime loop that coordinates L0 without becoming gameplay.

Tasks:

1. Define `RuntimeApp` or equivalent host over existing Kernel/FrameHost.
2. Define frame phases:

```text
BeginFrame
PollPlatform
PollInput
LoadOrCommitResources
UpdateWorld
PrepareRender
SubmitAudio
SubmitRender
Present
EndFrame
```

3. Define `FrameContext`:
   - frame index
   - delta time
   - fixed/update mode
   - input snapshot reference
   - diagnostics sink reference
4. Define pause/shutdown/error states.
5. Keep service registry stable and avoid global mutable business state.

Acceptance:

- Runtime loop can run zero-world frames.
- Runtime loop can run a small fixed-count headless test.
- Shutdown order is deterministic.
- L0 failure statuses propagate without throwing project-specific behavior.

Forbidden:

- gameplay update policy
- scene file loading
- UI/title behavior
- old runtime service state

### L1-2: Object, Component, Transform Baseline

Duration: 5 to 8 days

Goal:

Turn current `YuObject` and World sidecars into a coherent runtime identity
model without overbuilding ECS.

Tasks:

1. Object identity:
   - generation-checked handles
   - lifetime states
   - acquire/release validation
2. Component identity:
   - component type IDs
   - component slot IDs
   - attachment records
   - query bridge stays read-only
3. Transform:
   - POD transform record
   - local/world flag policy deferred unless needed
   - no hierarchy until a separate gate
4. Ownership:
   - World owns instance membership
   - Object owns object lifetime
   - Component bridge owns attachment sidecar
   - Transform bridge owns transform sidecar
5. No reflection by default.

Acceptance:

- Create/destroy object fixture.
- Attach/query component fixture.
- Set/query transform fixture.
- Destroy object invalidates relevant sidecar bindings through explicit caller
  path or documented future gate.
- No hidden growth in measured hot update/query path.

Forbidden:

- full ECS
- actor behavior lifecycle
- transform hierarchy
- reflection system
- script callbacks
- render/audio component behavior

### L1-3: World And Scene Assembly Core

Duration: 6 to 10 days

Goal:

Move from bridge fragments to a scene assembly model that remains caller-owned
and explicit.

Tasks:

1. World lifecycle:
   - create
   - update phases
   - destroy
2. Scene assembly records:
   - object identity record
   - transform record
   - component attachment record
   - component-resource binding record
3. Restore plan:
   - no-mutation preflight
   - duplicate detection
   - missing object/resource detection
   - deterministic status report
4. Active restore coordinator:
   - separate gate after proof
   - validate all before mutation
   - rollback strategy explicit before implementation
5. Manifest stream:
   - stream transport stays separate from active restore

Acceptance:

- Load a synthetic scene record set into caller-owned scratch.
- Preflight produces deterministic plan.
- Active restore only happens after an approved gate and no-mutation proof.
- World core does not depend directly on Resource/File/Package internals.

Forbidden:

- scene loader from disk as first step
- save-game policy
- object construction by Serialize
- resource loading inside scene restore
- game adapter semantics

### L1-4: Asset Manager Over Resource

Duration: 5 to 8 days

Goal:

Provide the runtime-facing asset layer without turning Resource into gameplay.

Tasks:

1. Asset handle:
   - stable ID
   - type
   - generation
   - state
2. Asset state:
   - unloaded
   - loading
   - decoded
   - uploaded/resident
   - failed
3. Dependency records:
   - explicit dependency list
   - no recursive load explosion
   - bounded traversal
4. Residency:
   - pin/unpin policy
   - budget status
   - eviction candidate query
5. L0 bridge:
   - uses Resource cache/decode/upload primitives
   - does not own RHI/Audio device lifecycle

Acceptance:

- Request asset by manifest/load-plan value.
- Decode/import a fixture payload.
- Produce texture/audio ready records.
- Query failure state.
- Release asset and observe residency/counter state.

Forbidden:

- full hot reload
- external asset ecosystem
- editor import UX
- old package compatibility
- gameplay asset semantics

### L1-5: Render Scene

Duration: 6 to 10 days

Goal:

Create a minimal runtime render scene that feeds RenderCore without owning RHI.

Tasks:

1. Render entity records:
   - object ID
   - transform reference
   - mesh/geometry asset handle
   - material asset handle
2. Camera:
   - camera record
   - active camera selection
   - view/projection values
3. Visibility:
   - initial simple full-list traversal
   - culling deferred unless needed
4. Packet assembly:
   - render records -> draw packets
   - material binding values
   - view packet
5. Resize:
   - receive L0/RHI resize state as value
   - do not own Platform event pump

Acceptance:

- Synthetic scene with one camera and one mesh produces RenderCore frame packet.
- RenderScene has no D3D11 includes.
- RenderScene can run with Null RHI or test fixture where possible.
- Missing mesh/material produces explicit status, not crash.

Forbidden:

- material graph
- lighting system
- animation/skinning
- UI rendering
- editor viewport

### L1-6: Audio Scene

Duration: 4 to 7 days

Goal:

Provide runtime audio source/bus concepts over L0 Audio without BGM/SE business
IDs.

Tasks:

1. Sound asset handle binding.
2. Audio source record:
   - play
   - stop
   - paused
   - gain
   - loop flag if required
3. Bus/mixer routing:
   - minimal fixed bus IDs
   - no editor graph
4. Runtime update:
   - source state -> PCM stream queue request
   - unavailable backend status propagated

Acceptance:

- Synthetic source plays through test backend.
- Supported hardware path can submit through callback where available.
- Missing asset/backend produces explicit status.
- No blocking IO or allocation in callback path.

Forbidden:

- BGM/SE project service IDs
- UI volume menu
- full DSP graph
- audio editor

### L1-7: Input Mapping

Duration: 3 to 5 days

Goal:

Translate L0 device snapshots into project-independent runtime commands.

Tasks:

1. Action map:
   - action ID
   - device binding
   - button/axis value
2. Input context:
   - active context list
   - focus or mode value
3. Runtime command snapshot:
   - pressed/released/held
   - axis value
   - frame index
4. Replay:
   - deterministic replay from L0 snapshots

Acceptance:

- Keyboard action fixture.
- XInput action fixture with unavailable skip/status.
- Input mapping does not call World or Script directly.

Forbidden:

- gameplay command execution
- UI navigation policy
- text input framework

### L1-8: Serialization, Save, Config

Duration: 5 to 8 days

Goal:

Use `YuSerialize` as a value stream, not as object construction or save policy.

Tasks:

1. Project-independent config record.
2. Scene assembly record stream.
3. Object/component/transform snapshot stream.
4. Versioning:
   - stream version
   - record version
   - explicit unsupported status
5. Save boundary:
   - write caller-owned buffers
   - file persistence is a separate File/Tool path

Acceptance:

- Serialize/deserialize a synthetic scene assembly record set.
- Reject unsupported version without mutation.
- No File/Package/Resource dependency inside Serialize core.

Forbidden:

- save-game business policy
- original save compatibility
- object construction in Serialize
- reflection system

### L1-9: Script Native Bridge Runtime Use

Duration: 4 to 7 days

Goal:

Keep script as a narrow call boundary before any VM or old script evidence.

Tasks:

1. Stable call IDs.
2. Caller-provided argument/value slots.
3. Native registry.
4. Error statuses.
5. Runtime phase dispatch from World remains an adapter, not World core.

Acceptance:

- Register and call a synthetic native function.
- World phase dispatch bridge maps phase trace to call ID.
- Missing call returns explicit failure.
- No stringly dispatch in hot path.

Forbidden:

- bytecode VM
- reflection
- original-game service state
- script-owned World/Resource/File access

### L1-10: Runtime Diagnostics Surface

Duration: 3 to 5 days

Goal:

Expose runtime state enough for internal debugging without turning diagnostics
into behavior.

Tasks:

1. Frame timing counter.
2. Object/world/resource/audio/render counts.
3. Last error ring or bounded event channel.
4. Optional debug overlay hook later.
5. Diagnostics-disabled equivalence tests.

Acceptance:

- Runtime runs with diagnostics disabled.
- Runtime emits bounded counters when enabled.
- No log/report output is required for behavior correctness.

Forbidden:

- report JSON as core API
- unbounded logging
- oracle/capture controlling runtime flow

### L1-11: Minimal Runtime Vertical Sample

Duration: 7 to 14 days after L1 subsystems are ready

Goal:

Prove L1 with a small project-independent scene.

Sample flow:

```text
Boot runtime
Create world
Load synthetic scene manifest
Create object records
Attach transform/component records
Bind texture/audio resource handles
Map input to runtime command
Update world for fixed frames
Assemble render scene
Submit RenderCore/RHI frame
Submit AudioScene/Audio frame
Serialize snapshot
Destroy world
Shutdown runtime
```

Acceptance:

- Runs as a CTest or sample command.
- Does not use TouhouNewWorld business logic.
- Does not parse old package runtime.
- Has deterministic output/status.
- Has Debug and Release validation.

## 7. Bridge Audit Plan

Bridge count is high enough that it needs explicit control.

Deliverable:

`docs/YUENGINE_BRIDGE_AUDIT.md`

Required fields:

```text
Bridge name
Source module
Destination module
Direction
Data handed off
Who owns source lifecycle
Who owns destination lifecycle
Failure statuses
Capacity bounds
Tests
Allowed layer crossing
Risk: Low / Medium / High
Required action
```

High-risk bridge patterns:

- crosses more than two adjacent conceptual layers
- reads and mutates multiple owner states
- has cache or lifecycle policy
- includes backend-native headers
- includes project/game-specific types
- uses reports/logs as behavior transport
- exists only to make a test pass

Required actions:

- Low risk: document and keep.
- Medium risk: add tests or split adapter from policy.
- High risk: split, move, or block future dependency until rewritten.

## 8. Team Work Breakdown

Use parallelism, but keep integration serial.

### Architecture / PM Lane

Tasks:

- L0 completion matrix.
- L1 runtime matrix.
- bridge audit.
- gate names and sequencing.
- final acceptance language.

### Render/RHI Lane

Tasks:

- D3D11 closure.
- swapchain resize proof.
- RenderCore frame sample.
- render sample integration.

### Platform/Input Lane

Tasks:

- Win32 event pump stabilization.
- Input bridge/XInput environment proof.
- L1 input mapping proposal after L0.

### Audio Lane

Tasks:

- XAudio2 callback proof.
- PCM stream queue closure.
- Audio scene proposal after L0.

### File/Resource/Streaming Lane

Tasks:

- sample asset path.
- decoded payload to RHI/Audio bridges.
- residency/cache/decode closure.
- L1 asset manager proposal.

### Runtime Core Lane

Tasks:

- kernel/game loop.
- object/component/transform.
- world/scene assembly.
- serialization/config/save.

### QA/Performance Lane

Tasks:

- fast/hardware/release commands.
- allocation/hot-path smoke.
- sample smoke.
- dirty worktree/repo hygiene.

Effective parallelism:

```text
L0 closure: 6-7 effective people out of 10
L1 runtime closure: 5-6 effective people out of 10
Final sample integration: 2-3 people, mostly serial
```

Reason:

Render, audio, input, file/resource, and QA can run in parallel. FrameHost,
sample, runtime loop, and scene integration must serialize at the end to avoid
contradictory ownership decisions.

## 9. Verification Policy

At every handoff:

```text
git status --short --branch
git diff --check
git show --check --format=short HEAD
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

For hardware closure:

```text
ctest --preset windows-hardware-smoke --output-on-failure
```

For release closure:

```text
cmake --preset windows-release
cmake --build --preset windows-release
ctest --test-dir <active release build dir> -C Release --output-on-failure
```

The exact release build dir must be read from the active preset or current repo
convention before execution.

Do not weaken fast gate to make hardware tests pass. Hardware tests are isolated
through labels/presets and explicit skip grading.

## 10. Done Definitions

### L0 Closure Candidate

L0 can be called a closure candidate when:

- L0 completion matrix is committed.
- Sample/repo hygiene is complete.
- Debug fast gate passes.
- Release tests pass.
- Hardware smoke has no unexpected failures.
- Environment skips are documented and graded.
- One YuEngine-owned sample exercises platform, RHI, input, resource, render,
  audio or explicit audio skip, resize, and shutdown.
- Bridge audit has no unresolved high-risk L0 bridge.

### L0 Done For Current Product

L0 can be called done for the current product when:

- Required hardware paths pass on the target production machine.
- Sample uses intended asset path.
- Failure states are recoverable or explicitly fatal.
- Hot-path budget smoke exists for render submit, audio callback, input poll,
  resource upload, diagnostics disabled/enabled, and thread/file completion.
- No temporary demo or evidence artifact is required for proof.

### L1 Closure Candidate

L1 can be called a closure candidate when:

- L1 matrix is committed.
- Runtime loop runs.
- Object/component/transform baseline is stable.
- Scene assembly preflight and controlled restore exist.
- Asset manager can bind runtime assets over L0.
- RenderScene and AudioScene can consume runtime records and submit to L0.
- Input mapping produces runtime command snapshots.
- Serialization/config can save and read a synthetic scene snapshot.
- Runtime diagnostics is bounded and optional.
- One project-independent vertical sample passes.

### Not Done

Do not call the engine commercially complete until:

- L2 project playable exists.
- Packaging/cook/release flow exists.
- Internal tools can validate assets and diagnose failures.
- Performance budgets are measured on realistic content.
- Regression tests cover startup, scene load, resource errors, device issues,
  and shutdown.

## 11. Required Future Documents

Create these documents in order:

1. `docs/YUENGINE_L0_COMPLETION_MATRIX.md`
2. `docs/YUENGINE_BRIDGE_AUDIT.md`
3. `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`
4. `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md`
5. `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md`

Do not create new broad implementation tasks until document 1 and document 2
exist and are accepted.

## 12. First 10 Landing Tasks

These are the recommended next tasks for the landing team.

1. Create L0 completion matrix.
2. Finish sample/repo hygiene cleanup.
3. Create bridge audit.
4. Close hardware skip grading for XAudio2 and XInput.
5. Build YuEngine-owned L0 sample skeleton under `Samples/`.
6. Wire Platform + RHI + RenderCore frame path in the sample.
7. Wire File/Resource/Streaming texture or mesh fixture path in the sample.
8. Wire Audio PCM stream path or explicit unavailable-device proof.
9. Run Debug/Release/Fast/HardwareSmoke/sample-smoke verification.
10. Create L1 runtime core matrix based on the accepted L0 result.

After these, begin L1 runtime loop and object/component/transform closure.

## 13. Guardrails

Stop and return to architecture if any of these happen:

- A lower module starts depending on World, UI, Script, or Project types.
- A bridge starts owning both sides' lifecycle.
- A report/log/capture becomes required for runtime behavior.
- A sample bypasses YuEngine modules with standalone demo code.
- A test uses sleeps/manual screenshots/manual audio listening as primary proof.
- A hardware skip is counted as done without grading.
- A project-specific need changes L0/L1 API without a gate.
- Old TouhouNewWorld package compatibility appears in L0 or L1.
- New World bridge work continues before L0 closure matrix is accepted.

## 14. Summary For Landing Team

The project is not behind enough to roll back. The project is ahead in several
module proofs but needs execution discipline now.

The correct move is not to add more broad gates. The correct move is:

```text
Matrix -> Bridge Audit -> L0 Engine Sample -> L1 Runtime Matrix -> L1 Vertical Sample
```

Keep YuEngine narrower than UE/Unity, but not weaker in production discipline.
Avoid developer ecosystem work. Preserve clean ownership, deterministic tests,
failure semantics, and performance budgets.

