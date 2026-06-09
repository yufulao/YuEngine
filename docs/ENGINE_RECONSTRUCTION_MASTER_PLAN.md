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

Current latest checkpoint: L12 now executes the original first mission tutorial branch
`threadEvent0020_00` through `yuengine_cli mission-tutorial`, and executes `updateUnits` as the
first mission business-state update edge. Tutorial actor/page creation, scheduler push/wait,
dialog lifecycle, current-player queries, event flag add, marker placement, player-control
recovery, and transition lifecycle are recorded in service-owned state with `unresolved_calls=0`.
This is still not a playable loop; the next edge is L13 script-driven title UI command payloads.
