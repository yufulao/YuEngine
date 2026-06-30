# YuEngine Long Plan And Team Execution

Status: active commercial-engine execution plan
Owner: Architect, lead engineer
Started: 2026-06-11
Current planning checkpoint: `origin/main@50ff335fe1ddfea77a72ce20f770baa3028df4a2`
Reference product target: `C:\Steam\steamapps\common\TouhouNewWorld`
Canonical entry point: `docs/README.md`

## 0. New Team Handoff Rule

Start from `docs/README.md`, then read this file. This document is the current
long-horizon plan. Older phase, RAV, editor, bridge, preview, resource-browser,
and UI documents are historical/evidence context unless this file or
`docs/README.md` explicitly promotes them back into the active plan.

If context is compacted or a new team takes over, preserve these facts first:

- YuEngine targets a small-team native commercial engine, not a toy engine or
  a UE/Unity clone.
- Package, Resource, and RuntimeAsset are the production spine.
- RuntimeAsset must define asset-internal targets before deeper animation,
  model, scene, or WorldObject binding.
- L0/L1 is complete at
  `a4a520ffbdb1049dd4674330854033c61c91d6bb`; final VQ reports
  `COMPLETE-PASS / FINAL-L0L1-DONE-READY`.
- The current nearest stage is RuntimeAsset production spine continuation.
- The human lead has resumed continuous execution. The coordinator must keep the
  team moving through the active accepted goal until its stop condition is met.
- The architect should preserve context for architecture, dependency control,
  task design, and evidence governance, not spend the main lane on routine
  frontline implementation.

## 1. Final Product Target

YuEngine is the production engine for a small independent team. It is not a
toy engine, a technology demo, a public UE/Unity competitor, or an editor-first
research project. The final target is a stable native commercial game engine
that can ship and maintain the team's own games.

The closest local reference target is the shipped TouhouNewWorld package:

- small native runtime binaries under `bin`;
- no observed Unity runtime markers such as `UnityPlayer.dll` or
  `GameAssembly.dll`;
- resource package directory around 7.84 GB;
- `.dat` archives around 7.23 GB and `.pak` archives around 0.60 GB;
- explicit config, shader/filter, database, and resource archive surfaces.

YuEngine must eventually support the same class of product pressure:

- 20 hours or more of stable play;
- 6 GB or more of shipped content;
- deterministic startup, load, update, render, audio, save, and shutdown;
- high-performance runtime loading from packed assets;
- explicit diagnostics for missing layers and production failures;
- repeatable build, package, validation, and release evidence.

The goal is not broad feature parity with UE or Unity. The goal is a narrower
native engine with commercial discipline strong enough for the team's game
line.

## 2. Non-Negotiable Architecture Principles

1. Runtime first, editor later.
2. Package, Resource, and RuntimeAsset are the product spine.
3. Assets never bind directly to world instances or editor objects.
4. Runtime assets bind to stable asset-internal targets first.
5. World and scene instances map asset targets to runtime objects later.
6. Reports, screenshots, logs, and viewers are evidence tools, not behavior.
7. Every failure path must name the failing layer and avoid partial mutation.
8. Every direct-main implementation slice must have focused evidence.
9. Compatibility with old game packages is not a design constraint unless a
   separate accepted gate says so.
10. No public plugin ecosystem, marketplace, or generic editor extension work
    belongs in the current L0/L1 execution window.

## 3. Long-Horizon Roadmap

### Stage A: L0 Production Foundation

Purpose: make the lower engine reliable enough to run and diagnose real native
runtime work.

Required closure:

- Platform window, event pump, native surface, and shutdown.
- RHI and D3D11 device, swapchain, present, capture, resize, and stale-handle
  failures.
- RenderCore frame packet, draw packet, material values, and graph skeleton.
- File/VFS, package load-plan, resource cache, decode, upload, residency, and
  streaming commit.
- Audio mixer, PCM packet/queue, XAudio2 availability, and callback cost
  discipline.
- Input replay, Win32 input, XInput availability, and command snapshots.
- Diagnostics, memory, and thread cost surfaces.
- One engine-owned lower sample that proves window -> input -> resource ->
  render -> audio -> resize -> shutdown or reports exact blockers.

Exit standard:

```text
Debug/Release/fast/hardware/sample evidence is reproducible, or every missing
hardware path is explicitly graded as an environment blocker.
```

### Stage B: L1 Runtime Core

Purpose: make project-independent runtime systems compose into a real scene.

Required closure:

- RuntimeApp and FrameContext phases.
- Object, component, transform, and scene assembly records.
- Runtime Asset Manager over Resource/Package/File.
- RenderScene over RenderCore/RHI.
- AudioScene over Audio/Resource.
- Input mapping over L0 input devices/actions.
- Serialize/config/save value boundaries.
- Runtime diagnostics counters.
- Pure runtime visual sample with cube, cylinder, cone, shared material,
  multiple texture inputs, camera tween, animation-driven transform, and bounded
  capture output.

Exit standard:

```text
A project-independent scene can load, instantiate, render, play audio, respond
to input, serialize core state, and shut down without project-specific shortcuts.
```

### Stage C: Runtime Asset Production Spine

Purpose: turn source/cooked assets into runtime records that can sustain a
multi-GB shipped product.

Required closure:

- Source artifact schema and cooked binary artifact schema.
- Archive/package table directory, offsets, hashes, dependency tables, and
  budget limits.
- Resource index equivalent to a production asset database.
- Runtime asset family contracts for mesh, texture, material, shader, scene,
  camera, model, skeleton, animation, audio, and UI when needed.
- Validator/cook/load stages with no-mutation failures.
- Incremental asset validation and packaging commands.

Exit standard:

```text
The engine can mount, validate, load, and diagnose packed content without loose
test-only construction or editor-owned runtime behavior.
```

### Stage D: Product Runtime Feature Set

Purpose: support the first real team game.

Required closure:

- Scene/world object lifecycle and instance mapping.
- Model, skeleton, animation clips, tracks, curves, and transform application.
- Shader reflection, material binding, texture streaming, and render pipeline
  value contracts.
- Audio source/bus routing and streaming.
- Input maps and replayable command snapshots.
- Save/profile/config persistence policy outside Serialize core.
- Crash-safe shutdown and resource release.

Exit standard:

```text
A representative vertical game slice can run for long sessions with bounded
memory, stable frame pacing, reproducible loads, and actionable diagnostics.
```

### Stage E: Production Tools

Purpose: make content creation efficient only after runtime contracts are firm.

Required closure:

- Importer and cooker.
- Resource browser and validator diagnostics.
- Scene/model/material/animation preview tools.
- Package builder and release validator.
- Performance capture, asset budget reports, and crash triage.

Exit standard:

```text
Artists and designers can produce content through tools that emit the same
runtime contracts proven by the engine.
```

### Stage F: Shipping And Maintenance

Purpose: make the engine and game survive commercial release.

Required closure:

- Release packaging.
- Patch/update compatibility policy for YuEngine packages.
- Long-session soak tests.
- Save compatibility and migration gates.
- Hardware matrix and graded environment failures.
- Versioned crash diagnostics.

Exit standard:

```text
The team can ship, patch, diagnose, and maintain a commercial game without
depending on one-off manual knowledge.
```

## 4. Current State

The old P1/P2/P3 first-slice language is historical. It should no longer drive
new task selection by itself.

Current mainline has meaningful lower-engine and RuntimeAsset evidence:

- Object, Serialize, Resource, Package, RHI, RenderCore, Audio, Input, World,
  and diagnostics have useful value-contract slices.
- RuntimeAsset has disk-backed closed-loop evidence through File/VFS/Resource,
  validators, RenderScene, RenderCore, RHI, package/product run ledgers, shader
  reflection hardening, and scene-animation selected-clip proof.
- RuntimeAsset/Streaming/RHI HEAD evidence now also includes the post-008H
  payload-window and destination-range chain through
  `origin/main@50ff335fe1ddfea77a72ce20f770baa3028df4a2`.
- Scene-animation implementation and QA are complete at
  `f211f7f95299388987ccef00b4d1e8ee6f7bf0c1`.
- Docs evidence sync is complete at
  `0a9144b0e30cbede56a5dbf04b232f3e5b763802`.

The project is not blocked by lack of proof. It is at risk if proof slices keep
expanding without a production spine.

## 5. Nearest Detailed Stage

The nearest stage is not another broad feature. It is the RuntimeAsset product
spine correction.

### 5.1 Close Current Evidence Gate

Current RTSPINE evidence gate state:

- implementation: done;
- QA: done;
- docs: done;
- scene-animation VQ consistency evidence: verify as regression coverage inside
  the RTSPINE-003 VQ gate instead of reopening a separate write lane;
- RTSPINE-003 target identity implementation: done at
  `origin/main@5ea838f6fd3428e7e67b77c1ca85c41e6e1c09e4`;
- RTSPINE-003 focused QA: done, `10/10` focused target identity plus
  scene/runtime animation regression rows PASS;
- RTSPINE-003 docs: synchronized by workspace task
  `d9dc3692-aa12-4f5c-872a-5b7293a92ceb`;
- RTSPINE-003 VQ consistency audit: done / COMPLETE-PASS by workspace task
  `fdd78da4-da12-4956-b6ac-63ff9e377121`.

This gate is closed. RTSPINE-004 may open as the only implementation write lane.
Independent read-only architecture, code-surface scouting, and pressure-audit
lanes may run in parallel when they cannot mutate the same evidence gate.

### 5.2 Correct RuntimeAsset Dependency Order

The next planning and implementation queue must use this order:

```text
RuntimeAsset container and family identity
-> package/resource index and dependency tables
-> asset-internal node/model/skeleton target contract
-> animation clip/track/channel binding to those targets
-> Step/Linear interpolation sampler
-> transform application to runtime instance records
-> world object mapping only after instance contracts exist
-> editor/importer authoring surfaces after runtime contracts pass
```

This order is mandatory. Animation must not bind directly to WorldObject,
editor object, scene instance, raw pointer, display name, or file path.

### 5.3 Immediate Backlog

| ID | Work item | Acceptance |
| --- | --- | --- |
| RTSPINE-001 | Final VQ for scene-animation evidence gate | Implementation, QA, docs, `origin/main`, and evidence matrices agree; no next lane opens before PASS |
| RTSPINE-002 | RuntimeAsset production target addendum | Long-term target, TouhouNewWorld package reference, and no-compatibility policy are written into planning docs |
| RTSPINE-003 | Asset-internal target identity contract | PASS at `origin/main@5ea838f6fd3428e7e67b77c1ca85c41e6e1c09e4`: scene node, model node, and skeleton joint target IDs are bounded output records independent from WorldObject; duplicate id, missing parent, and capacity overflow fail without mutation |
| RTSPINE-004 | Animation track target binding contract | Tracks bind to target ID plus property, not world instance or editor object |
| RTSPINE-005 | Minimal interpolation contract | PASS at `origin/main@2bfe7e37d36ca711dd706728f21b1e4caecfd3db` with focused QA at `origin/main@d18f1679ebd389ecec506055764602591f5b9ab6`: Step and Linear scalar/vector/transform sampling pass fixed-time tests with no hidden global time; unsupported interpolation and sample output capacity fail without mutation |
| RTSPINE-006 | Invalid target failure contract | PASS at `origin/main@96e0c024435f670c39ced019ff825b819a6830a3`; target-family mismatch and sample failure diagnostics fail without output mutation, and focused QA task `6d02c260-936a-456b-917b-5c2802bbb666` reports focused RuntimeAsset regex `8/8` PASS plus exact new rows `2/2` PASS without broad/full CTest |
| RTSPINE-007 | Runtime instance mapping contract | PASS at `origin/main@37a112549190ac2123abcd72b5c688cdfa5b01e5`: asset target records map to caller-owned runtime instance rows for scene entities before any WorldObject/editor binding; focused QA task `6b6baf5f-2381-4b9c-89b1-4411fba53d23` reports exact RuntimeInstanceMapping rows `5/5` PASS without broad/full CTest |
| RTSPINE-008 | Package/resource index pressure gate | 008A spec, 008B byte-range/index, 008C Package hash/dependency integrity, 008D File/VFS ranged IO, 008E Resource payload window/reference budget, 008F Package dependency closure/budgeted load plan, 008G RuntimeAsset packaged validation bridge, 008H RuntimeAsset transaction rollback/proof, and 008I Package archive range to RuntimeAsset Resource payload-window handoff are represented in current HEAD evidence; cite VQ-closed status only for lanes with workspace COMPLETE-PASS VQ |
| RTSPINE-009 | Payload-window and destination-range follow-through ledger | Current HEAD `50ff335` records `e2e8c3c`/`1658639` RuntimeAssetWorldAdapter bridge/handoff with exact marker aliases `RuntimeAssetWorldObjectAdapter` and `RuntimeAssetWorldObjectRestoreHandoff`, `0d2021c` Streaming U64 staging, `bc6d0ee` Resource U64 `payload_window`, `2c93ddf` RuntimeAsset payload logical count, `6ac7ff9` Streaming cache payload bridge, `08b1ccd`/`35a84c3` Package payload metadata and legacy compatibility, `50a654e`/`baae22d` Streaming pipeline cache payload consumer plus rejection/no-mutation coverage, `e5cd6ee` Package-to-Streaming artifact fixture, `10f7b30` RuntimeAssetData package payload-window consumer, `c3cf022` RHI update destination range contract, and `50ff335` ResourceUpload destination range consumer; this is implementation/focused evidence unless a scoped VQ says COMPLETE-PASS; next-gate matrix marker `a5aad608` / `NEXT-GATE-MATRIX-READY` is docs-only VQ evidence routing, not implementation closure |

### 5.4 Forbidden Work In This Stage

- no editor-first animation timeline;
- no gameplay animation state machine;
- no blend tree, montage, skeletal skinning, or physics coupling;
- no WorldObject direct binding inside asset files;
- no old TouhouNewWorld package parser as L0/L1 proof;
- no broad shader/material work as a substitute for target identity;
- no direct WorldObject/editor binding beyond caller-owned scene-entity rows and
  the narrow RuntimeAssetWorldAdapter bridge/handoff evidence before its own
  evidence gate is released;
- no broader Resource/File/VFS expansion beyond the narrow Resource/Streaming
  payload-window and destination-range follow-through already represented at
  `50ff335` before its own gate is released.

## 6. Team Execution Model

The lead architect owns order, dependency, and stop conditions.

Specialist agents execute only when a task has:

- exact scope;
- exact files or module surface;
- exact non-goals;
- expected evidence;
- failure/blocker policy.

### 6.1 Continuous Multi-Agent Execution

The coordinator must not stop after a single task while accepted work remains.
The default behavior is to keep the goal active, set timers, and keep assigning
the next independent work surface until the stop condition is met or a real
blocker is recorded.

Every shared task must include:

- AI ETA;
- owner and role fit;
- exact scope;
- exact read/write surface;
- non-goals;
- expected evidence;
- stale-owner timeout and reroute rule.

The architect must avoid routine frontline implementation unless the work is a
small architecture patch, an unblocker, or there is no viable owner. This keeps
the architect's context budget available for global dependency control, risk
triage, task decomposition, and evidence review.

### 6.2 True Parallelism Rule

Parallel work is useful only when it reduces calendar time without corrupting
contracts. A lane can run in parallel when it is read-only, writes to disjoint
files, or owns an isolated module boundary with no shared-contract mutation.

Do not split one serial dependency into fake parallel work such as separate
"test 1", "test 2", and "full test" lanes when each lane must wait for the same
implementation or produces duplicate evidence. That burns tokens and does not
increase throughput.

Allowed parallel lanes in the current stage:

- VQ evidence consistency audit for the completed scene-animation gate;
- read-only RuntimeAsset target-identity design surface;
- read-only package/resource index pressure audit;
- read-only implementation-surface scouting for the next RTSPINE contracts;
- read-only quality and risk audit that identifies safe implementation splits.

Forbidden parallel lanes in the current stage:

- any write lane that changes RuntimeAsset contracts before VQ accepts the
  current evidence gate;
- editor, UI, gameplay, or WorldObject binding expansion;
- duplicate QA lanes that only repeat the same focused evidence without a new
  risk question;
- broad full-suite testing without an explicit shared-contract or release
  reason.

QA and evidence tasks may run in parallel only when they are read-only or when
their write surfaces are disjoint. Implementation lanes do not overlap if they
touch shared contracts.

## 7. Stop Conditions

Stop and return to architecture when any of these happen:

- a lower module depends upward on World, UI, Script, Project, GameAdapter, or
  editor types;
- a runtime asset references world instance IDs or editor object IDs;
- a feature bypasses Package/Resource/RuntimeAsset and claims production asset
  proof;
- a report, viewer, screenshot, or log becomes required for runtime behavior;
- old game package compatibility shapes the first production contract;
- a bridge owns both sides' lifecycle;
- hot paths allocate, grow containers, format strings, or perform file IO
  without an accepted gate exception;
- a new implementation lane opens before the active evidence gate closes;
- stage evidence cannot name exact commands, commits, and failure statuses.

## 8. Summary

YuEngine must become a compact native production engine for the team, with the
same kind of reliability and asset pressure as the TouhouNewWorld shipped
package. The spine is:

```text
Package -> Resource -> RuntimeAsset -> Runtime records -> World/Scene instance
mapping -> Render/Audio/Input/Save -> Tools -> Shipping
```

The next correct move is not feature breadth. The current RuntimeAsset
production spine is evidence-rich through `50ff335`, but next implementation
must wait for the canonical docs/VQ boundary to name the next scoped gate rather
than reopening broad WorldObject, Resource/File/VFS, editor, or gameplay work.
