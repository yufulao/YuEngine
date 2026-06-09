# L11 First Mission Event Thread Status

Status: completed as first mission event/player-control contract checkpoint on 2026-06-09.

This checkpoint consumes the L10 first-frame scene state and executes the original first mission
event thread entry `threadEvent0000_00` from `mission/sc01/main/ms010_0.b64.sqasm`. It proves the
first post-entry gameplay/event control edge is driven by recovered bytecode and native service
state, not by a replacement tutorial script.

## Implemented

- `yu::runtime::MissionEventThreadRuntimeReport`
- `yu::runtime::runMissionEventThreadRuntime`
- `yu::runtime::missionEventThreadRuntimeReportToJson`
- `yuengine_cli mission-event-thread`
- CTest `yuengine_mission_event_thread_contract`

Runtime service coverage added for this edge:

- player control and transform APIs:
  `SetPlayerControl`, `SetPlayerPos`, `SetPlayerAngleY`, `LandPlayer`, `GetPlayerPos`;
- player/action/platform APIs:
  `resetPlayerAction`, `ResetMenuButtonHoldingTimes`;
- event/page/dialog APIs:
  `EventClass`, `GetEventUnit`, `getPage`, `CallSetupPages`, `eventPage.done`,
  `dlgReset`, `dlgHide`, `flagINI`, `GetFlag`;
- event volume and camera APIs:
  `EventVolume`, `activateVolume`, `SetGameCameraIfNot`;
- event root/table expression tracking for `ev_*`, `eventMap`, and `ev*pg*` fields.

## Verified Contract

Command:

```powershell
build\cmake-bt143\yuengine_cli.exe mission-event-thread samples\touhou_new_world\project.json --repo-root .
```

Metrics:

```text
ok=true
scene_runtime_ok=true
event_thread_found=true
event_thread_executed=true
entry=threadEvent0000_00
player_control_commands=2
player_control_enabled=true
reset_menu_button_holding_times_commands=1
dialog_reset_commands=1
dialog_hide_commands=1
reset_player_action_commands=1
event_unit_queries=1
event_page_setup_commands=1
event_page_done_commands=1
event_volume_activation_commands=1
last_event_volume_enabled=false
set_game_camera_if_not_commands=1
unresolved_calls=0
truncated=false
```

## Boundary

L11 is still a contract checkpoint, not a playable game loop.

It does not implement:

- full Squirrel VM semantics;
- full tutorial branch coverage;
- actor spawning/wait/update business-state loops;
- real renderer/audio backend submission;
- load/save/options branches beyond the current new-game path.

The next edge is L12: first mission tutorial and business-state contract. It must continue from
the same original script/runtime chain and cover tutorial actors, event flags, actor update/wait
contracts, current-player queries, and quest/event state without introducing a hand-written
tutorial replacement.

## Verification

```powershell
cmake -S . -B build\cmake-bt143
cmake --build build\cmake-bt143 --config Debug --clean-first -- -j1
ctest --test-dir build\cmake-bt143 --output-on-failure
python -m unittest discover -s tests
```
