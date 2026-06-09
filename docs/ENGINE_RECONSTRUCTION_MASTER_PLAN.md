# Engine Reconstruction Master Plan

This plan corrects the previous failure mode: do not confuse small validations with engine
reconstruction progress.

## Objective

Build YuEngine as a reusable C++ engine runtime extracted from the Touhou New World evidence
corpus, with the original game as the first sample/oracle project and `samples/empty_project`
as the generic project proof.

The target is not a mesh viewer, a hand-written menu, or a Python-only analysis toolkit. The
target is a C++ runtime that can eventually execute:

```text
project.json
-> project boot
-> VFS/resource mount
-> script VM and module loader
-> native/API service registry
-> original title script
-> save/new-game/profile flow
-> scene/stage load
-> actor/player/camera/input
-> early mission/tutorial flow
-> renderer/audio/save baseline
```

## Corrected Execution Rule

Runtime work must start as a full engine spine, not as isolated visual prototypes.

The previous `Project` failed because it chased visible gaps and built a replacement shell.
`YuEngine` must stay bottom-up and contract-driven. Small slices are acceptable only as verified
edges of the full runtime path; they are never a license to ship a minimal entry, fake menu, or
scene preview.

The word "slice" here means a contract edge inside the final runtime, not a separate miniature
engine. A valid edge must preserve the original layers on both sides: project manifest, VFS,
script module/VM state, native service ownership, runtime-owned state, and sample behavior.
If a change cannot be extended directly toward original title/menu/save/scene/actor/camera/input
flow, it is not a valid phase endpoint.

Allowed now:

- build C++ project/runtime infrastructure;
- wire project manifest, VFS, script module loading, native obligation registry, diagnostics;
- implement services as explicit tracked modules;
- keep unconfirmed native behavior as obligations that fail loudly.

Still forbidden:

- hand-written replacement title menu;
- blue-screen scene placeholders presented as progress;
- silent no-op native calls;
- hard-coding `map/Doujou`, `titlemenu`, or first mission into engine code;
- claiming original behavior without oracle/static evidence;
- using Steam/login bypasses.
- stopping after plans, readiness reports, parser statistics, or call binding diagnostics when a
  non-blocked runtime edge remains.
- marking a smallest launcher, smallest VM, smallest renderer, or smallest menu as a completed
  reconstruction phase.

## Completion Meaning

The project is not "done" until all of these are true:

- C++ runtime builds.
- Original sample project boots through `project.json`.
- Original title menu is driven by original script behavior, not replacement UI.
- New Game / Load / Save flow follows sampled `MakeNewGame` and `StartGame` behavior.
- Scene load reaches the correct stage through service-owned APIs.
- Player, camera, input, actor/task, and early tutorial/event flow run.
- Generic empty sample can boot through the same runtime without original game-specific code.
- Residual mismatches are explicitly tracked.

## Engine Spine Order

1. Core: logging, diagnostics, paths, JSON, command-line app.
2. Project: manifest load/validate, project context, lifecycle.
3. VFS: loose mount, pack-manifest mount, resource existence and stem resolution.
4. Script: `.sqasm` module loader now; Squirrel VM later.
5. Native/API: service registry and obligation table from evidence.
6. Runtime: boot sequence, preload modules, entry module, service diagnostics.
7. UI/render/audio/save/scene/actor/camera services: implemented behind service interfaces.
8. Oracle integration: captured traces upgrade obligations into behavior specs.
9. Script execution: original scripts drive title/new-game/scene through services.

## Engineering Standard

Every loop must either:

- add C++ runtime surface area;
- wire original/generic sample data into runtime;
- reduce unknown native/API behavior with evidence;
- add tests/diagnostics that prevent fake progress.

Do not stop after a single small validation if a next task is available. The standing loop is:

```text
evidence
-> runtime/service contract
-> implementation
-> regression test or diagnostic gate
-> exact unknowns
-> commit
-> next contract edge
```

Current target state remains original-script driven title, save/new-game, scene entry, actor,
camera, input, tutorial, and then generic reusable runtime. Anything below that is an in-progress
checkpoint, not a finished engine.

Do not stop after the current edge if the full target is not reached. The operative completion
bar is:

```text
engine runtime source exists
-> original sample boots through that runtime
-> original scripts drive title/menu/new-game/load
-> scene load reaches real stage resources
-> player/camera/input/tutorial/event state advances through services
-> renderer/audio/save baselines consume the same state
```

Every smaller result is a checkpoint only. A checkpoint may be committed, but the next action is
to advance the next non-blocked runtime edge. This is especially important after `script-run`,
`scene-entry`, `scene-runtime`, and `first-frame`: those commands are regression gates and evidence
producers, not substitutes for the game loop.

The current L7 route is: original title bytecode -> cross-module class/method inheritance ->
runtime-owned script/service state -> title scene dispatch -> original menu state -> save/new-game
services -> scene/stage load. Each checkpoint must move one of those arrows forward or harden a
regression gate around an already verified arrow.

Current latest checkpoint: L26 now defines the platform/backend bridge through
`yuengine_cli platform-bridge`. The runtime consumes L24 device execution records and L25
presentation/oracle records before it can mark the bridge ready. It materializes 10 D3D9-style
call batches, including 46 resource creation calls, 458 upload subresource calls, 57 state-binding
calls, 121 draw submission calls, 1 present call, and 2 capture/oracle calls. The bridge links
103 L24 records and 7 L25 records, with 57 ready platform inputs and 53 tracked-open inputs. This
is still not a playable loop or concrete D3D backend; the next edge is L27 concrete backend
executor interface and diagnostic D3D9 adapter.

The current route is no longer allowed to stop at menu visuals:

```text
original title bytecode
-> UI command payload
-> save/load/options branch behavior
-> scene/actor/camera/input/event frame state
-> renderer/audio/save backend submission
-> service-owned frame scheduler/update graph
-> concrete backend obligations
-> shader/effect/material semantics
-> device/swapchain/render-state presentation
-> texture upload/render-state/font/oracle parity gates
-> backend render-state/font atlas records
-> D3D9-compatible resource allocation records
-> device resource creation/state-binding execution records
-> swapchain/present/original-frame oracle parity records
-> real platform D3D API submission/backend bridge
-> concrete backend executor interface and diagnostic adapter
```

The Project failure rule is now stricter: no new loop may be framed as "minimal." The loop unit
is a layer contract with implementation and regression coverage. A checkpoint is allowed only
when it leaves the next contract edge executable and the agent continues into that edge. L27 must
consume L26 bridge records; a standalone launcher, clear-screen window, or mesh preview remains a
regression to the failed `Project` route.
