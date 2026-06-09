# L9 Scene Runtime Materialization Status

Status: completed as runtime-handle materialization checkpoint on 2026-06-09.

This checkpoint consumes the completed L8 `scene-entry` contract and reads real resource payloads
through the YuEngine VFS. It does not draw a placeholder scene. It turns the script-driven first
mission binding into stage, actor, camera, and event-marker runtime handles with explicit failure
conditions.

## Implemented

- `yu::resource::VirtualFileSystem::readBytes`
- `yu::runtime::SceneRuntimeMaterializationReport`
- `yu::runtime::materializeSceneEntryRuntime`
- `yu::runtime::runSceneRuntimeMaterialization`
- `yuengine_cli scene-runtime`
- CTest `yuengine_scene_runtime_materialization`

## Verified Contract

Command:

```powershell
build\cmake-bt143\yuengine_cli.exe scene-runtime samples\touhou_new_world\project.json --repo-root .
```

Metrics:

```text
ok=true
scene_entry_ok=true
stage_handle_ready=true
actor_handle_ready=true
camera_handle_ready=true
event_marker_ready=true
stage_dependencies=42
missing_stage_dependencies=0
model_meshes=111
collision_triangles=150
rail_nodes=3
```

Runtime payloads confirmed:

- Stage headers:
  - `map/doujou/doujou.sge`: `MgResourceHeader`, extension `sge`, 10,176 bytes.
  - `map/doujou/doujou.mdl`: `MgResourceHeader`, extension `mdl`, 2,610,633 bytes.
  - `map/doujou/doujou.col`: `MgResourceHeader`, extension `col`, 10,304 bytes.
  - `map/doujou/doujou.rcm`: `MgResourceHeader`, extension `rcm`, 1,088 bytes.
- Stage graph:
  - 42 resolved dependencies.
  - 2 model dependencies.
  - 1 collision dependency.
  - 39 texture dependencies.
  - 0 missing dependencies.
  - 111 plausible model mesh blocks.
  - 16 material blocks.
  - Collision mesh: 272 vertices, 450 indices, 150 triangles.
- Actor handle:
  - player `reimuEx`;
  - `player/reimuex.b64`: 406 bytes;
  - `player/reimuex_pcg.b64`: 13,645 bytes;
  - spawn expression `toVec3(marker:sc01/main/ms010_0:eventMap._pos)`;
  - rotY `0`.
- Camera handle:
  - rail camera `map/doujou/doujou.rcm`;
  - 3 rail-node count candidate from payload offset `0x100`;
  - rail enabled `true`;
  - auto adjust `false`;
  - default camera target `ev_sc01_main_ms010_0`.
- Event marker handle:
  - marker `marker:sc01/main/ms010_0:eventMap`;
  - checkpoint expression matches spawn position expression.

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

L9 materializes runtime handles and reads real payloads, but it is still not a rendered playable
scene. L10 has consumed these handles into a first-frame renderer/input/event contract. The next
edge must execute or model first mission event threads and player-control behavior:

- event thread/page contract for `threadEvent0000_00` and tutorial/intro control;
- input/control contract for player movement and camera update;
- no hand-written menu, no standalone mesh preview, no bypass of title/new-game/setupProcess.
