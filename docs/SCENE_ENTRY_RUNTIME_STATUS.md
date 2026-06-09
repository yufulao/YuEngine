# L8 Scene Entry Runtime Contract Status

Status: scene-entry runtime contract checkpoint completed on 2026-06-09.

This is not a visual demo and not a handcrafted stage launcher. The checkpoint runs the original
title new-game bytecode first, uses its `StartGame` service state to locate the first mission,
then runs the original first mission `setupProcess` bytecode and validates the resulting runtime
contract against project VFS/script roots.

## Implemented

- `yu::runtime::SceneEntryRuntimeReport`
- `yu::runtime::runSceneEntryRuntime`
- `yuengine_cli scene-entry`
- CTest `yuengine_scene_entry_title_new_game`

The runtime report merges two proven service-state edges:

```text
script/menu/titlemenu.b64 setupProc --frames 5 --input-scenario title-new-game
-> MakeNewGame(sc01)
-> StartGame(mission:sc01/main/ms010_0, true)
-> queue_scene_stage_load(
     mission/sc01/main/ms010_0.b64.sqasm,
     map/Doujou/doujou.sge,
     map/Doujou/doujou.rcm)

mission/sc01/main/ms010_0.b64 setupProcess --frames 1
-> LoadStage(map/Doujou/doujou.sge)
-> LoadEventsScriptViaMission(sc01/main/ms010_0)
-> PushPlayerChara(reimuEx, marker pos, rotY 0)
-> PushTaskGameCamera
-> LoadRailCamera(map/Doujou/doujou.rcm)
-> SetDefaultCameraState(ev_sc01_main_ms010_0)
```

## Verified Contract

Command:

```powershell
build\cmake-bt143\yuengine_cli.exe scene-entry samples\touhou_new_world\project.json --repo-root .
```

Metrics:

```text
ok=true
title_new_game_executed=true
mission_setup_executed=true
stage_ready=true
actor_ready=true
camera_ready=true
event_ready=true
resource_bindings=6
missing_resources=0
script_bindings=5
missing_scripts=0
```

Resolved required resource bindings:

- `stage_sge`: `map/doujou/doujou.sge`
- `stage_model`: `map/doujou/doujou.mdl`
- `stage_collision`: `map/doujou/doujou.col`
- `rail_camera`: `map/doujou/doujou.rcm`
- `player_script_asset`: `player/reimuex.b64`
- `player_pcg_asset`: `player/reimuex_pcg.b64`

Resolved required script bindings:

- `title_queued_mission_script`: `mission/sc01/main/ms010_0.b64.sqasm`
- `mission_setup_script`: `mission/sc01/main/ms010_0.b64.sqasm`
- `event_script`: `mission/sc01/main/ms010_0.b64`
- `player_script_module`: `player/reimuex.b64`
- `player_pcg_module`: `player/reimuex_pcg.b64`

## Verification

```powershell
cmake -S . -B build\cmake-bt143
cmake --build build\cmake-bt143 --config Debug --clean-first -- -j1
ctest --test-dir build\cmake-bt143 --output-on-failure
python -m unittest discover -s tests
```

Verified result:

```text
CTest: 16/16 passed
Python unittest: 6/6 passed
```

## Boundary

This checkpoint proves that original scripts now drive a coherent scene-entry contract through
project manifest, VFS, script roots, native service state, stage resources, player script assets,
and camera resources.

It is still not a rendered playable scene. L9 has consumed these bindings into runtime handles,
and L10 has consumed those handles into renderer/input/event first-frame contracts. The next edge
is first mission event thread/tutorial/player-control behavior.

L9 consumed the resolved bindings into runtime subsystems:

- parse or model SGE/RCM/COL/MDL headers and dependency payloads;
- build stage graph/resource handles rather than raw path strings;
- materialize event marker data for `eventMap._pos/_rot`;
- materialize actor task state from `PushPlayerChara` and player scripts;
- materialize camera task stack from `PushTaskGameCamera`, rail camera, and default camera state;
- continue toward renderer/audio/input contracts without hand-written visual placeholders.
