# L10 First Frame Runtime Contract Status

Status: completed as renderer/input/event frame contract checkpoint on 2026-06-09.

This checkpoint consumes the L9 `scene-runtime` handles and builds a renderer-facing first-frame
runtime contract. It does not open a window and does not claim a playable rendered scene. It proves
the first frame contract remains attached to the original title new-game path and first mission
`setupProcess` path.

## Implemented

- `yu::runtime::FirstFrameRuntimeReport`
- `yu::runtime::buildFirstFrameRuntimeReport`
- `yu::runtime::runFirstFrameRuntime`
- `yuengine_cli first-frame`
- CTest `yuengine_first_frame_runtime_contract`

## Verified Contract

Command:

```powershell
build\cmake-bt143\yuengine_cli.exe first-frame samples\touhou_new_world\project.json --repo-root .
```

Metrics:

```text
ok=true
scene_runtime_ok=true
renderer_frame_ready=true
actor_frame_ready=true
camera_frame_ready=true
input_frame_ready=true
event_frame_ready=true
mesh_draw_candidates=111
texture_bindings=39
collision_triangles=150
actor_instances=1
rail_nodes=3
event_markers=1
```

Frame contract:

- Renderer frame:
  - renderer profile `d3d9_compatible`;
  - 111 mesh draw candidates;
  - 16 material bindings;
  - 39 texture bindings;
  - 150 collision triangles;
  - 42 stage dependencies.
- Actor frame:
  - 1 player actor instance;
  - player `reimuEx`;
  - spawn expression `toVec3(marker:sc01/main/ms010_0:eventMap._pos)`;
  - rotY `0`.
- Camera frame:
  - rail camera `map/doujou/doujou.rcm`;
  - default target `ev_sc01_main_ms010_0`;
  - 3 rail-node candidates.
- Input frame:
  - owner service `Actor And Task Service`;
  - control scope `player_actor_camera_scene`.
- Event frame:
  - marker `marker:sc01/main/ms010_0:eventMap`;
  - 1 event marker handle.

## Verification

```powershell
cmake -S . -B build\cmake-bt143
cmake --build build\cmake-bt143 --config Debug -- -j1
ctest --test-dir build\cmake-bt143 --output-on-failure
python -m unittest discover -s tests
```

Verified result:

```text
CTest: 16/16 passed
Python unittest: 6/6 passed
```

## Boundary

L10 proves the first-frame renderer/input/event contract can be built from original script-driven
runtime state and real resource payloads.

It is still not a playable frame. The next edge must execute or model first mission event threads
and tutorial/control behavior:

- `threadEvent0000_00` and related first mission event/page functions;
- `SetPlayerControl`, `SetPlayerPos`, `SetPlayerAngleY`, `LandPlayer`, `GetPlayerPos`;
- `SetGameCameraIfNot`, `CallSetupPages`, `EventVolume`;
- player update/input frame state beyond ownership labels;
- renderer command-buffer/upload layer after the contract is fed into a real backend.
