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

Do not stop after a single small validation if a next task is available.
