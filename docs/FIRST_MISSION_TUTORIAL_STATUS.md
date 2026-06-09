# L12 First Mission Tutorial Status

Status: completed as first mission tutorial/business-state contract checkpoint on 2026-06-09.

This checkpoint consumes L9 scene runtime materialization and executes the original first mission
tutorial event entry `threadEvent0020_00` from `mission/sc01/main/ms010_0.b64.sqasm`. It also
executes `updateUnits` as the first follow-up business-state update edge. The result is still a
runtime contract checkpoint, not a finished playable loop.

## Implemented

- `yu::runtime::MissionTutorialRuntimeReport`
- `yu::runtime::runMissionTutorialRuntime`
- `yu::runtime::missionTutorialRuntimeReportToJson`
- `yuengine_cli mission-tutorial`
- CTest `yuengine_mission_tutorial_contract`

Runtime/script coverage added for this edge:

- script function values now carry their source module path, so baseline `menudef` closures and
  mission closures no longer share one ordinal namespace;
- `_OP_CLOSURE`, `_OP_NEWARRAY`, `_OP_APPENDARRAY`, and `append` are modeled enough for tutorial
  callback/page arrays;
- tutorial scheduler/runtime roots are modeled: `th`, `ms`, `ml`, and `demo`;
- tutorial actor/page APIs are service-owned: `ActorTutorial`, `addPage`, `PushActor`, `WaitActor`;
- tutorial/demo callbacks execute through recovered closure arrays, not through a hand-written
  tutorial sequence;
- dialog and message APIs are modeled: `dlgShow`, `dlgSpeakL`, `dlgWait`, `dlgHide`, `ml.dlg`,
  `ml.get`, dialog background probes;
- player/camera/event APIs for this branch are modeled: `GetCurrentPlayerName`, `GetPchar`,
  `SetPlayerControl`, `SetPlayerAngleY`, `LandPlayer`, `EnterTransition`, `LeaveTransition`,
  `flagADD`, event marker `pos`, event marker `rotYDegree`, and `UpdateUnits`.

The L12 implementation also fixes a scene-entry crash by binding `NativeRuntimeServiceState` by
reference in `runSceneEntryRuntime`; the report builder does not need to copy the full service
state object.

## Verified Contract

Command:

```powershell
build\cmake-bt143\yuengine_cli.exe mission-tutorial samples\touhou_new_world\project.json --repo-root .
```

Metrics:

```text
ok=true
scene_runtime_ok=true
tutorial_thread_found=true
tutorial_thread_executed=true
update_units_executed=true
entry=threadEvent0020_00
event_flag_add_commands=1
current_player_name_queries=3
dialog_show_commands=3
dialog_speak_commands=3
dialog_wait_commands=3
dialog_hide_commands=4
tutorial_actor_creates=1
tutorial_page_creates=1
push_actor_commands=1
wait_actor_commands=1
player_control_commands=4
player_control_enabled=true
set_player_angle_y_commands=1
land_player_commands=1
update_units_commands=1
enter_transition_commands=1
leave_transition_commands=1
unresolved_calls=0
truncated=false
```

## Boundary

L12 proves the first tutorial branch and the first mission business-state update edge through
original bytecode and service state.

It does not implement:

- full Squirrel VM semantics;
- full tutorial branch coverage beyond `threadEvent0020_00`;
- a repeated gameplay frame update loop;
- real actor animation/control simulation;
- real renderer/audio backend submission;
- save/load/options branches beyond the current new-game path.

L13 has since added script-driven title UI command payloads. Continue using original bytecode,
VFS resources, and service-owned command buffers; do not replace the title menu with a hand-written
screen or stop at a diagnostic report. The next active edges are L14 save/load/continue/options
branch coverage and L15 gameplay-frame update contracts.

## Verification

```powershell
cmake -S . -B build\cmake-bt143
cmake --build build\cmake-bt143 --config Debug --clean-first -- -j1
ctest --test-dir build\cmake-bt143 --output-on-failure
python -m unittest discover -s tests
```
