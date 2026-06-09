# L15 Gameplay Frame Runtime Status

L15 joins the already recovered title, scene, actor, camera, input, event, tutorial, audio, UI,
and save/profile contracts into a single gameplay-frame runtime report. It is a service-owned
frame contract, not a blue-screen renderer, mesh viewer, or playable game loop.

## Contract

The runtime command is:

```text
project.json
-> scene-runtime materialization
-> title UI command payload
-> title Continue/NewGame/Load/Option/Exit branch matrix
-> first mission event thread
-> first mission tutorial/updateUnits
-> gameplay-frame readiness and command-count contract
```

The frame report requires all upstream contracts to pass, then checks eight frame lanes:

- renderer: stage mesh/material/texture payloads from `scene-runtime`;
- UI: title background/logo/text/list command payloads from original `renderProc`;
- save/profile: title branch matrix with StartGame, LoadAutoSave, and MakeNewGame coverage;
- actor/task: player actor plus tutorial actor scheduling state;
- camera: rail camera handle plus mission camera restoration command;
- input: player-control commands from mission event and tutorial threads;
- event/tutorial: event-page, dialog, event-flag, and `updateUnits` state;
- audio: title-branch audio command coverage.

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe gameplay-frame samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_gameplay_frame_runtime_contract --output-on-failure
```

Current metric:

```text
ok=true
scene_runtime_ok=true
title_ui_ok=true
title_branches_ok=true
mission_event_thread_ok=true
mission_tutorial_ok=true
frame_updates=2
renderer_frame_ready=true
ui_frame_ready=true
save_frame_ready=true
actor_frame_ready=true
camera_frame_ready=true
input_frame_ready=true
event_frame_ready=true
audio_frame_ready=true
gameplay_command_count=221
```

Payload counts:

```text
mesh_draw_candidates=111
material_bindings=16
texture_bindings=39
title_ui_commands=55
title_ui_draw_commands=9
start_game_scenarios=3
load_auto_save_scenarios=2
make_new_game_scenarios=1
actor_instances=1
player_control_commands=6
camera_commands=2
rail_nodes=3
event_commands=16
tutorial_update_commands=1
audio_commands=28
```

## Boundary

L15 is not a claim that the game is playable. It does not yet provide:

- real renderer backend submission;
- swapchain/device/frame presentation;
- shader/effect/material interpretation;
- real input polling;
- actor animation/skeleton execution;
- save serialization;
- original-frame visual parity.

It prevents the previous failure mode by forcing title UI, save/load branches, scene resources,
actor/camera/input/event/tutorial state, and audio command evidence to stay in one runtime-owned
frame contract before renderer/backend work starts.

## Next Edges

- L16 has since built a backend-facing renderer submission contract that consumes title UI
  commands and scene mesh/material/texture payloads in the same frame.
- L17: replace aggregate frame readiness with a service-owned frame scheduler/update graph now
  that renderer submission has a concrete command-buffer consumer.
