# Autonomous Loop Tasks

This file is the queue for long-running agents. Work top to bottom. When a task is completed,
commit it and continue to the next task without waiting for a new prompt.

## Execution Doctrine

Each loop closes one engine layer contract. It is not a minimal demo, a temporary entrypoint,
or a visual placeholder. A loop is only complete when it has:

- a named runtime/service contract;
- evidence inputs from scripts/resources/native/API/oracle where available;
- implementation in the C++ runtime spine or an explicitly staged tool feeding that spine;
- diagnostics or tests that fail on the important regression;
- documentation of remaining unknowns;
- a git commit.

After a loop is committed, continue to the next non-blocked loop. Do not stop at CLI statistics,
resource previews, mesh previews, handwritten UI, or blue-screen runtime output. Those can be
intermediate diagnostics only when they are connected to the next runtime layer.

Partial diagnostics are not loop completion. If a task produces a plan, table, report, parser,
or readiness check, the next action is to use that output to advance the runtime contract in the
same lane. A commit can checkpoint the work, but it is not a stopping condition.

Do not interpret "small verified slice" as "minimal engine." A slice is valid only when it is a
bounded edge of the full engine route and preserves the original project's contract. If the slice
cannot be connected to project boot, VFS, script VM, native services, save/new-game, scene, actor,
camera, input, or tutorial flow, it is not a YuEngine loop task.

Hard current directive:

- no handwritten substitute title menu;
- no blue-screen or mesh-preview progress claims;
- no smallest-entry replacement runtime;
- no "minimal engine" stage or "first make a tiny launcher" detour;
- no silent native/API stubs;
- no stop after docs, readiness, statistics, binding reports, or test-only maintenance;
- every checkpoint must leave a named next contract edge and then continue to that edge.
- no stop after a successful `script-run`, `scene-entry`, `scene-runtime`, or `first-frame`
  command unless it is immediately followed by implementation, regression coverage, docs, commit,
  and the next non-blocked contract edge.

Autonomous cycle:

```text
audit current layer evidence
-> encode the contract in runtime/tooling
-> wire it into CLI/tests
-> document exact remaining unknowns
-> commit
-> immediately continue the next non-blocked contract edge
```

The current long target remains:

```text
project.json
-> VFS/resource system
-> script module model
-> script VM/native service registry
-> script-driven title menu
-> save/new-game/profile flow
-> scene/stage load
-> actor/player/camera/input/tutorial flow
-> first playable sample flow
-> generic reusable engine runtime
```

## Completed Loops

### L1: C++ Runtime Spine

Status: completed as diagnostic runtime spine on 2026-06-09.

Deliver:

- `CMakeLists.txt`
- `src/yuengine/...`
- `apps/yuengine_cli`
- runtime boot diagnostics for both sample projects

Acceptance:

- CMake configure/build passes.
- `yuengine_cli validate samples/touhou_new_world/project.json` passes.
- `yuengine_cli boot samples/touhou_new_world/project.json` loads project, mounts VFS, loads
  preload and title `.sqasm`, and reports native/API obligations.
- `yuengine_cli boot samples/empty_project/project.json` boots without original resources.

Verified:

- CMake BuildTools 14.43/Ninja build passes.
- CTest passes 4/4.
- Python unittest suite passes 6/6.
- Original sample diagnostic boot reports:
  - loose mounts: 2
  - pack resources: 13,028
  - native registry APIs: 84
  - loaded modules: preload and titlemenu
  - title/preload obligations: 36, all `not_started`
- Empty sample diagnostic boot reports:
  - loose mounts: 1
  - native registry APIs: 84
  - native obligations: 0

Boundary: this is diagnostic boot only. It is not script execution or gameplay.

### L2: VFS And Pack Manifest Depth

Status: completed as resource dependency diagnostics on 2026-06-09.

Deliver:

- resource registry with normalized paths;
- pack-manifest indexed resources;
- loose resource lookup;
- stem expansion;
- title and first mission resource dependency diagnostics.

Acceptance:

- title background/logo/DLC stems resolve to language variants.
- `map/doujou/doujou.sge` and `map/doujou/doujou.rcm` resolve through project mounts.

Verified:

- `yuengine_cli resources samples/touhou_new_world/project.json` passes.
- `yuengine_cli resources samples/empty_project/project.json` passes.
- CTest passes 6/6 with resources tests included.
- Original sample required resources have 0 missing.
- Title background/logo/DLC stems resolve to pack and merged loose variants.
- Title script resource refs: 8, unresolved: 0.
- First mission script resource refs: 3, unresolved: 0.
- `map/doujou/doujou.sge` and `map/doujou/doujou.rcm` resolve as pack resources.
- Resource literal pollution from `/`, date placeholders, and script keys is filtered.

Boundary: this is VFS/dependency diagnostics only. It is not asset payload parsing or rendering.

### L3: Script Module Model

Status: completed as C++ script module diagnostics on 2026-06-09.

Deliver:

- `.sqasm` parser in C++ preserves functions, literals, instructions, callsites, resource refs;
- module dependency/call summary;
- obligation extraction by service owner.

Acceptance:

- title module reports 81 functions, 4466 instructions, 593 calls.
- first mission module reports 62 functions, 4068 instructions, 640 calls.

Verified:

- `yuengine_cli script samples/touhou_new_world/project.json script/menu/titlemenu.b64` reports
  81 functions, 4,466 instructions, 593 calls, 146 unique calls, 8 resource refs, 80 closure bindings.
- `yuengine_cli script samples/touhou_new_world/project.json mission/sc01/main/ms010_0.b64` reports
  62 functions, 4,068 instructions, 640 calls, 123 unique calls, 7 resource refs, 61 closure bindings.
- CTest passes 8/8 with script tests included.
- Python unittest suite passes 6/6.

Boundary: this is script module diagnostics only. It is not Squirrel VM execution or native side effects.

### L4: Native Service Interfaces

Status: completed as C++ native service contract baseline on 2026-06-09.

Deliver:

- C++ service interfaces for the 11 P6 services;
- native registry maps API name -> service -> implementation state;
- unimplemented calls raise tracked obligations.

Acceptance:

- 84 title/first-mission APIs are present in registry.
- no call used by current boot scope is unowned.

Verified:

- `yuengine_cli native-services samples/touhou_new_world/project.json --repo-root .` reports
  11 services, 84 native APIs, 0 unowned APIs, 0 unbound APIs, 84 `not_started` obligations.
- Original sample diagnostic boot remains `ok: true` and still reports 36 title/preload
  native obligations through the service catalog.
- CTest passes 9/9.
- Python unittest suite passes 6/6.

Boundary: this is service ownership and obligation dispatch only. It is not native behavior
implementation, argument/return confirmation, or oracle-diff acceptance.

### L5: Service-Backed Runtime Lifecycle

Status: completed as service-backed diagnostic boot lifecycle on 2026-06-09.

Deliver:

- service container for core/project/VFS/script/native layers;
- deterministic boot phases with explicit diagnostics;
- preload and entry-module ownership by Script Service;
- native/API obligation emission through Native Service interfaces.

Acceptance:

- original and empty sample still pass boot diagnostics.
- no script-visible native/API call can disappear as a silent no-op.
- R1 remains diagnostic boot until original scripts execute; do not mark R2 here.

Verified:

- Boot report now has deterministic phases:
  `load_project_manifest`, `mount_vfs`, `verify_required_resources`, `load_native_registry`,
  `bind_native_services`, `load_startup_scripts`, `collect_native_obligations`.
- Original sample reports `ok=true phases=7 failed_phases=0 native_apis=84 native_services=11 obligations=36`.
- Empty sample reports `ok=true phases=7 failed_phases=0 native_apis=84 native_services=11 obligations=0`.
- CTest passes 9/9.
- Python unittest suite passes 6/6.

Boundary: this is service-backed diagnostic boot only. It is not script VM execution, title UI,
native behavior implementation, or gameplay.

### L6: Oracle Capture Execution

Status: completed as oracle capture readiness on 2026-06-09. P1 title boot sampling remains incomplete.

Deliver:

- actual title boot captures if capture tools or user-driven run are available;
- three-run stability report.

Acceptance:

- title boot trace maps to `titlemenu.b64`.
- save list/resource/audio/render observations are recorded or explicitly blocked.

Verified:

- `oracle_title_boot.py readiness` reports `can_launch_original_exe=true` and `safe_to_bypass_platform=false`.
- Current blockers are explicit: no file-IO capture tool, no D3D/render capture tool, and no
  user-driven title boot run recorded in this workspace.
- `inspect`, `readiness`, `snapshot`, and `runbook` commands complete.
- Python unittest suite passes 6/6.

Boundary: P1 is not complete. No original-game process was launched and no title frame/file-IO/
render/audio trace was sampled in this loop.

## Active Loop

### L7: Title Script Execution

Status: active. Title bytecode-state and runtime input scenario checkpoints are implemented on
2026-06-09; L7 is not complete and must continue without waiting.

Deliver:

- Squirrel VM plan/embedding or bytecode execution bridge;
- title script dispatch through native service registry;
- UI/render command buffer begins from script calls.

Acceptance:

- no hand-written replacement menu.
- every missing native call is a named obligation.

Progress:

- `yuengine_cli script-plan` resolves the manifest startup entry.
- Title entry `setupProc` is found with 2 direct calls.
- `print` is resolved as `builtin`.
- title root recovers 8 class method tables and 64 method bindings.
- `modTitle` is bound to `ModuleTitle` through root object construction evidence.
- `setupProc -> modTitle.init` resolves to `ModuleTitle.init`, function ordinal `78`.
- name-only ambiguity among `init` ordinals `15, 30, 61, 78` is removed for this entry.
- `yuengine_cli script-run` executes the original title boot path:
  - `modTitle -> ModuleTitle`;
  - `modTitle._scenes[0] -> TitleScene`;
  - `modTitle._scenes[1] -> NewGameScene`;
  - `modTitle._scenes[2] -> LoadScene`;
  - `modTitle._scenes[3] -> OverwriteSaveScene`;
  - `setupProc -> print + modTitle.init`;
  - `ModuleTitle.init` foreach scene init;
  - optional first boot-frame `main` wrapper with `--frames 1`.
- `script-run --frames 2` now dispatches the original scene path:
  `ModuleTitle.main -> modTitle._scenes[0].main -> TitleSceneBase.main -> TitleScene.state0`.
- `TitleSceneBase.main` now uses virtual owner dispatch over the concrete scene object, so
  `TitleScene.state0` runs instead of the base placeholder state.
- `_OP_CALL` argument capture is wired into recovered method/constructor execution. The title
  menu window records `script.ScrollWindow@constructor:50._elemCount=4` from
  `selectCursorY(titleMenuCount)`.
- `root.gMenu=table#1` is recognized as receiver `gMenu` through root table aliasing. Passive
  title-frame input returns `gMenu.isDecide(false)`, so the idle branch is followed and the old
  false `TitleScene.state0 -> PlaySE(3)` fallthrough is gone.
- `script-run` now accepts `--input-scenario`; the `title-new-game` profile supplies runtime
  input state (`menu_selected_index=1`, `menu_decide=true`) and lets original bytecode drive the
  branch.
- `script-run --frames 5 --input-scenario title-new-game` now reaches:
  `TitleScene.state0 -> NewGameScene.state0 -> NewGameScene.state1 -> IsSaveFull(false) ->
  setMissionKey -> SetDifficultyMode -> return 200 -> ModuleTitle.fadeOut ->
  gMenu.startGame4Menu -> MakeNewGame(sc01) -> StartGame(mission:sc01/main/ms010_0, true)`.
- `StartGame` now queues the first scene/stage load request from original resource evidence:
  `mission/sc01/main/ms010_0.b64.sqasm`, `map/Doujou/doujou.sge`, and
  `map/Doujou/doujou.rcm`.
- Branch-only service calls now appear only when selected by input/runtime state: `PlaySE(2)`,
  `GetScenarioKeys`, `GetCountActiveDLC`, `IsSaveFull`, `SetDifficultyMode`, `fadeOut`, and
  `startGame4Menu`, `MakeNewGame`, and `StartGame`.
- Current title trace has 0 unresolved calls after runtime gap classification:
  - 15 engine object calls in the two-frame passive trace;
  - 17 runtime-owned UI helper object calls;
  - 6 value helper calls;
  - 6 value method calls;
  - 0 Module lifecycle hooks.
- `script-run --frames 1` now executes a scoped multi-module bytecode state pass over the title
  boot path with 2 baseline modules: `preload.b64` and `script/menu/menudef.b64`.
- The bytecode state pass is PC-branch aware for `_OP_JZ/_OP_JNZ/_OP_JMP`, handles
  `_OP_FOREACH/_OP_POSTFOREACH` for the title `_scenes` loop, and has
  `control_flow_unknown=0`, `unresolved_calls=0`, `truncated=false`.
- Method-context object lookup now falls back to root/global slots, so original bytecode resolves
  `gMenu` from `ModuleTitle.main` without a hard-coded shortcut.
- `_OP_NEWSLOT/_OP_NEWSLOTA` now follow slot-write semantics and no longer overwrite register
  `a0`; this is required for `menudef` to publish `ModuleBase` into the root table.
- `ModuleTitle : ModuleBase` now inherits `ModuleBase._fadeInTime`; `FadeIn` decodes as
  `duration=0.7; blend=0`.
- Cross-module method dispatch now walks class-base chains across `titlemenu.b64`,
  `menudef.b64`, and preload baseline modules. `ModuleBase.stateInit` executes as
  `script_inherited_owner_method`, mutating `modTitle._nextState` to `0`; the older
  `module_lifecycle_hook` fallback is no longer part of the current trace.
- Current passive `--frames 2` state counters:
  - 99 state-scanned functions;
  - 4,207 state-scanned instructions;
  - 31 root slot writes;
  - 405 class slot writes;
  - 191 object field writes;
  - 199 table slot writes;
  - 167 typed call returns;
  - 0 UI object mutations.
- Service calls now mutate runtime-owned service state while still emitting event evidence:
  - 43 service state mutations in passive two-frame title idle;
  - 4 save/profile queries in passive title init/idle;
  - 3 platform state queries in passive title init/idle;
  - 1 audio command;
  - 1 scene fade command;
  - 20 tracked UI objects;
  - 0 UI service commands;
  - 4 save/value state queries;
  - 8 decoded service argument payloads.
- Runtime service state snapshot now records:
  - `save.empty_save_list_queries=4`;
  - `save.save_list_count_queries=4`;
  - `save.save_list_entries=0`;
  - `audio.current_bgm_id=3`;
  - `scene.fade_in_duration=0.7`;
  - `scene.fade_in_blend=0`;
  - `ui.created_objects=20`;
  - `ui.command_count=0`.
- Runtime-owned new-game state now records:
  - `scenario_key_queries=2`;
  - `scenario_key_count_queries=1`;
  - `scenario_key_get_queries=1`;
  - `make_new_game_commands=1`;
  - `start_game_commands=1`;
  - `started_mission=mission:sc01/main/ms010_0`;
  - `difficulty_mode=1`;
  - `scene.queued_stage_loads=1`;
  - `scene.current_mission_script=mission/sc01/main/ms010_0.b64.sqasm`;
  - `scene.current_stage=map/Doujou/doujou.sge`;
  - `scene.current_rail_camera=map/Doujou/doujou.rcm`.
- Runtime script state snapshot now records:
  - `root_field_count=31`;
  - `object_count=26`;
  - `table_count=17`;
  - `class_slot_table_count=24`;
  - `class_base_count=16`;
  - `root.gMenu=table#1` with 87 table fields;
  - `modTitle._nextState=0`;
  - `modTitle._scenes[0..3]=TitleScene/NewGameScene/LoadScene/OverwriteSaveScene`;
  - `script.ScrollWindow@constructor:50._elemCount=4`;
  - `script.ScrollWindow@constructor:50._sel=0`.

Current next edge:

- consume the completed scene-entry runtime contract into deeper runtime subsystems:
  resource load handles, stage graph, actor component/task state, camera task stack, event-map
  marker data, and renderer/audio/input contracts;
- expand runtime input scenarios for `gMenu` beyond `title-new-game`: Continue enabled/disabled,
  Load empty/non-empty, Option, Exit denied/allowed, cursor up/down;
- extend concrete UI helper objects into decoded UI command payloads;
- finish argument payload decoding for the current service state events, especially `MenuObject`,
  `_menuWindow`, `_listWindow`, `setSelectCursor`, `bl/tr`, `float2`, and `renderHorizontal`;
- after scene-entry binding exists, materialize runtime handles instead of remaining at path
  validation or visual preview.

Boundary: L7 is not complete. The current checkpoint is bytecode-state execution for the title
boot/new-game edge, not a full Squirrel VM, title UI, scene runtime, or gameplay.

### L8: Save/New Game And Scene Entry

Status: completed as scene-entry runtime contract checkpoint on 2026-06-09.

Deliver:

- save/profile/scenario service behavior from evidence/oracle;
- `MakeNewGame` / `StartGame` path;
- scene/stage service load diagnostics.

Acceptance:

- StartGame selects a mission candidate through data/service state, not hard-coded scene boot.
- Mission setupProcess executes through original bytecode and produces stage, player, camera,
  event-script, and checkpoint service state.

Progress:

- `script-run samples/touhou_new_world/project.json mission/sc01/main/ms010_0.b64 setupProcess
  --frames 1` now executes the original first mission `setupProcess` entry with 0 unresolved
  calls.
- Current first mission metrics:
  `script_functions=2`, `native_obligations=18`, `unique_native_apis=18`,
  `engine_object_calls=9`, `typed_call_returns=35`, `service_state_events=27`,
  `audio_service_commands=1`, `scene_service_commands=10`, `decoded_service_arguments=19`,
  `unresolved_calls=0`, `truncated=false`.
- Runtime-owned first mission service state records:
  - loader: `loader:sc01/main/ms010_0`;
  - stage: `map/Doujou/doujou.sge`;
  - event script: `sc01/main/ms010_0`;
  - player: `reimuEx` at `toVec3(marker:sc01/main/ms010_0:eventMap._pos)`;
  - camera: `map/Doujou/doujou.rcm`, rail enabled true, auto adjust false;
  - checkpoint: `toVec3(marker:sc01/main/ms010_0:eventMap._pos)`;
  - place params: label `<tid=plac.0001>`, return-to-world enabled, slave display mode 0.
- `yuengine_cli scene-entry samples/touhou_new_world/project.json --repo-root .` now runs
  original title new-game bytecode and original first mission setup bytecode into one runtime
  contract.
- Scene-entry metrics:
  `ok=true`, `title_new_game_executed=true`, `mission_setup_executed=true`,
  `stage_ready=true`, `actor_ready=true`, `camera_ready=true`, `event_ready=true`,
  `resource_bindings=6`, `missing_resources=0`, `script_bindings=5`, `missing_scripts=0`.
- Required scene-entry resource bindings:
  `map/doujou/doujou.sge`, `map/doujou/doujou.mdl`, `map/doujou/doujou.col`,
  `map/doujou/doujou.rcm`, `player/reimuex.b64`, `player/reimuex_pcg.b64`.
- Required scene-entry script bindings:
  `mission/sc01/main/ms010_0.b64.sqasm`, `mission/sc01/main/ms010_0.b64`,
  `player/reimuex.b64`, `player/reimuex_pcg.b64`.
- CTest passes 16/16 and Python unittest passes 6/6.

Boundary: L8 proves script-driven new-game and scene-entry binding. It is not a rendered playable
scene and not a generic stage/actor/camera implementation.

### L9: Stage, Actor, Camera Runtime Materialization

Status: completed as runtime-handle materialization checkpoint on 2026-06-09.

Deliver:

- consume `scene-entry` binding into runtime handles, not raw strings;
- parse or model SGE/RCM/COL/MDL headers and dependency payloads needed by the first mission;
- build stage graph state from `map/Doujou/doujou.sge` and sibling resources;
- materialize event-map marker data for `eventMap._pos/_rot`;
- materialize player actor/task state from `PushPlayerChara(reimuEx, pos, rotY)`;
- materialize camera task stack from `PushTaskGameCamera`, `LoadRailCamera`, rail enable, auto
  adjust, and default camera state;
- expose diagnostics/tests that fail if any of those runtime handles are missing.

Acceptance:

- no hand-written visual placeholder or mesh-only preview;
- no direct hard-coded first scene startup that bypasses title/new-game/setupProcess;
- scene-entry output feeds stage/actor/camera runtime data structures;
- missing or malformed SGE/RCM/COL/MDL/player script inputs fail explicitly.

Next edge:

- `yuengine_cli scene-runtime samples/touhou_new_world/project.json --repo-root .` now consumes
  the L8 scene-entry contract and emits stage/actor/camera/event runtime handles.
- Scene-runtime metrics:
  `ok=true`, `scene_entry_ok=true`, `stage_handle_ready=true`, `actor_handle_ready=true`,
  `camera_handle_ready=true`, `event_marker_ready=true`, `stage_dependencies=42`,
  `missing_stage_dependencies=0`, `model_meshes=111`, `collision_triangles=150`,
  `rail_nodes=3`.
- Resource payloads confirmed:
  `map/doujou/doujou.sge` 10,176 bytes, `map/doujou/doujou.mdl` 2,610,633 bytes,
  `map/doujou/doujou.col` 10,304 bytes, `map/doujou/doujou.rcm` 1,088 bytes.
- Stage graph handle now records 42 resolved dependencies, 2 model dependencies,
  1 collision dependency, 39 texture dependencies, 0 missing dependencies, 111 plausible model
  mesh blocks, 16 material blocks, and collision mesh counts 272 vertices / 450 indices /
  150 triangles.
- Actor handle records player `reimuEx`, `player/reimuex.b64` 406 bytes,
  `player/reimuex_pcg.b64` 13,645 bytes, spawn expression
  `toVec3(marker:sc01/main/ms010_0:eventMap._pos)`, and rotY `0`.
- Camera handle records rail camera `map/doujou/doujou.rcm`, 3 rail-node count candidate,
  rail enabled true, auto adjust false, and default target `ev_sc01_main_ms010_0`.
- Event marker handle records `marker:sc01/main/ms010_0:eventMap`.
- CTest passes 16/16 and Python unittest passes 6/6.

Boundary: L9 proves resource payload materialization into runtime handles. It is not a rendered
playable frame.

### L10: Renderer, Input, And First Gameplay Frame Contract

Status: completed as renderer/input/event first-frame contract checkpoint on 2026-06-09.

Deliver:

- consume L9 stage/actor/camera/event handles into renderer-facing scene frame state;
- define renderer resource upload contract for model meshes, material/texture bindings, collision
  debug/collision geometry, rail/default camera state, and actor spawn;
- define first gameplay frame input/control contract from script/native service state;
- connect event thread/page setup for first mission intro/tutorial control without hand-written
  scene flow;
- expose diagnostics/tests that fail if the rendered-frame contract is detached from
  title/new-game/setupProcess/scene-runtime handles.

Acceptance:

- no blue-screen placeholder;
- no mesh-only preview as completion;
- no T-pose actor claim without script/task state;
- renderer contract consumes `scene-runtime`, not hard-coded `map/Doujou`;
- first frame state includes stage resources, player actor spawn, camera state, event marker, and
  input/control ownership.

Next edge:

- `yuengine_cli first-frame samples/touhou_new_world/project.json --repo-root .` now consumes
  L9 scene-runtime handles and emits a first-frame renderer/input/event contract.
- First-frame metrics:
  `ok=true`, `scene_runtime_ok=true`, `renderer_frame_ready=true`, `actor_frame_ready=true`,
  `camera_frame_ready=true`, `input_frame_ready=true`, `event_frame_ready=true`,
  `mesh_draw_candidates=111`, `texture_bindings=39`, `collision_triangles=150`,
  `actor_instances=1`, `rail_nodes=3`, `event_markers=1`.
- Renderer frame records profile `d3d9_compatible`, 111 mesh draw candidates, 16 material
  bindings, 39 texture bindings, 150 collision triangles, and 42 stage dependencies.
- Actor frame records player `reimuEx`, 1 actor instance, spawn expression
  `toVec3(marker:sc01/main/ms010_0:eventMap._pos)`, and rotY `0`.
- Camera frame records source `map/doujou/doujou.rcm`, default target `ev_sc01_main_ms010_0`,
  and 3 rail-node candidates.
- Input frame records owner `Actor And Task Service` and control scope
  `player_actor_camera_scene`.
- Event frame records marker `marker:sc01/main/ms010_0:eventMap`.
- CTest passes 16/16 and Python unittest passes 6/6.

Boundary: L10 proves the renderer-facing first-frame contract. It is not a rendered playable
frame and not a real backend upload.

### L11: First Mission Event Thread And Player Control Contract

Status: completed as first mission event/player-control contract checkpoint on 2026-06-09.

Deliver:

- execute or model first mission event thread/page setup beyond `setupProcess`;
- recover runtime behavior for `threadEvent0000_00` and related first mission intro/tutorial
  functions;
- implement service state for player control and player transform calls:
  `SetPlayerControl`, `SetPlayerPos`, `SetPlayerAngleY`, `LandPlayer`, `GetPlayerPos`;
- implement camera/event page contract calls such as `SetGameCameraIfNot`, `CallSetupPages`,
  and `EventVolume`;
- feed the resulting player/control/camera/event state back into `first-frame` or a subsequent
  gameplay-frame contract.

Acceptance:

- no manual tutorial sequence: satisfied for `threadEvent0000_00`;
- no fake control state detached from original mission script: satisfied through service mutations;
- first-frame/gameplay-frame state changes only through original bytecode and native service
  mutations: satisfied for this edge;
- `mission-event-thread` reports the first mission event thread contract without dumping unrelated
  full VM state;
- the report proves, from original `threadEvent0000_00` bytecode, player-control gating, menu-hold
  reset, dialog reset/hide, event-page setup/done, event-volume activation, and camera restoration
  service mutations;
- L10 `first-frame` still passes after L11 state is modeled.

Verified metric:

```text
ok=true scene_runtime_ok=true event_thread_found=true event_thread_executed=true entry=threadEvent0000_00 player_control_commands=2 player_control_enabled=true reset_menu_button_holding_times_commands=1 dialog_reset_commands=1 dialog_hide_commands=1 reset_player_action_commands=1 event_unit_queries=1 event_page_setup_commands=1 event_page_done_commands=1 event_volume_creates=0 event_volume_activation_commands=1 last_event_volume_enabled=false set_game_camera_if_not_commands=1 unresolved_calls=0 truncated=false
```

Next edge:

- continue immediately into L12 tutorial/business-state functions connected to first mission event
  pages;
- inspect actor/tutorial/event flag call graph beyond `threadEvent0000_00`;
- keep new APIs as runtime service contracts with CTest gates, not visual or scripted stand-ins.

### L12: First Mission Tutorial And Business-State Contract

Status: completed as first mission tutorial/business-state contract checkpoint on 2026-06-09.

Deliver:

- model or execute the early tutorial/event actor branches connected to first mission event pages;
- implement the next set of runtime-owned actor/event APIs such as `ActorTutorial`, `PushActor`,
  `WaitActor`, `UpdateUnits`, `GetPlayerControl`, and current-player/player-name queries;
- keep tutorial completion, event flags, quest markers, and actor handles in service-owned state.

Acceptance:

- no scripted tutorial replacement outside original bytecode: satisfied for `threadEvent0020_00`;
- tutorial actor/page state mutates only through ScriptRuntime/native service contracts:
  satisfied through `ActorTutorial`, `addPage`, `PushActor`, and `WaitActor`;
- unconfirmed effect/actor APIs remain visible obligations: satisfied by retaining unresolved-free
  script execution while keeping real backend behavior as later runtime work.

Verified metric:

```text
ok=true scene_runtime_ok=true tutorial_thread_found=true tutorial_thread_executed=true update_units_executed=true entry=threadEvent0020_00 event_flag_add_commands=1 current_player_name_queries=3 dialog_show_commands=3 dialog_speak_commands=3 dialog_wait_commands=3 dialog_hide_commands=4 tutorial_actor_creates=1 tutorial_page_creates=1 push_actor_commands=1 wait_actor_commands=1 player_control_commands=4 player_control_enabled=true set_player_angle_y_commands=1 land_player_commands=1 update_units_commands=1 enter_transition_commands=1 leave_transition_commands=1 unresolved_calls=0 truncated=false
```

Additional engine hardening:

- Squirrel `Function` values now carry source module paths so baseline module closures and mission
  closures do not collide by ordinal.
- Scene-entry binds service runtime state by reference instead of copying the full runtime-state
  graph after title/mission execution.

Next edge:

- continue immediately into L15 gameplay-frame work using title branch, scene, actor, camera,
  input, event, and UI service state;
- preserve L13 title UI command payloads and L14 title branch matrix as regression gates;
- begin L16 renderer/backend command buffer work when it can consume these service contracts.

### L13: Script-Driven Title UI Command Payload Contract

Status: completed as title/menu render command payload on 2026-06-09.

Deliver:

- `yuengine_cli title-ui` executes original `script/menu/titlemenu.b64.sqasm` setup/render;
- `ScriptRunOptions.renderFrames` drives original `renderProc` after setup/main state;
- `ScrollWindow.drawList` materializes recovered per-row `MenuObject` instances and invokes the
  original callback, rather than drawing a replacement menu;
- title background/logo/menu text/text-size/color/graph calls are recorded in
  UI And 2D Render Service command state;
- `yuengine_title_ui_command_payload_contract` guards the command payload.

Acceptance:

- no hand-written title menu;
- title menu layout and selectable state come from original title bytecode and UI service payload;
- command buffers are regression-tested and do not regress title/new-game script flow;
- metrics: `created_objects=26`, `command_count=55`, `draw_commands=9`,
  `graph_string_commands=5`, `string_size_queries=5`, `text_draw_commands=6`,
  `graph_draw_commands=3`, `color_commands=11`, `draw_list_item_commands=5`,
  `background_resource_bound=true`, `logo_resource_bound=true`, `unresolved_calls=0`.

Boundary:

- this is not a renderer backend or a playable menu;
- UI global/native obligations remain behavior specs until backend submission is implemented;
- next work must use this payload for branch/UI/backend progress, not replace it.

### L14: Save/Load/Continue/Options Branch Contract

Status: completed as title branch matrix contract on 2026-06-09.

Deliver:

- execute original title bytecode branches for Continue disabled, Continue, New Game, Load empty,
  Load, Option, Exit denied, and Exit allowed;
- expand Save/Profile/Scenario Service state for save-list entries, autosave load, capacity query,
  MakeNewGame, StartGame, and queued stage load;
- expand Platform Service state for CanShutdown and ShutdownGame;
- keep Steam/login/entitlement as replaceable Platform Service state, not bypass logic.

Acceptance:

- no fake save menu state;
- branch transitions and enabled/disabled states are driven by title scripts and service inputs;
- every unsupported platform/save behavior is an explicit runtime obligation.
- metrics: `scenario_count=8`, `executed_scenarios=8`, `start_game_scenarios=3`,
  `load_auto_save_scenarios=2`, `make_new_game_scenarios=1`,
  `shutdown_permission_scenarios=2`, `shutdown_game_scenarios=1`,
  `option_ui_mutations=2`, `unresolved_calls=0`, `truncated=false`.

Boundary:

- Continue/Load currently prove autosave-load branch reachability and StartGame/scene queue
  side effects; their `StartGame` argument shape is preserved as recovered runtime evidence, not
  forced into New Game's mission-label schema;
- this is still not a real save browser, save serializer, or interactive menu backend;
- branch contracts must feed L15 gameplay-frame state rather than becoming a stopping point.

### L15: Gameplay Frame Update Loop Contract

Status: completed as service-owned gameplay-frame contract checkpoint on 2026-06-09.

Deliver:

- join input, actor/task, camera, event, renderer, audio, and save services into a repeated
  gameplay-frame update contract;
- consume scene-runtime handles and mission event state in one frame report;
- begin replacing diagnostics-only frame reports with backend-ready command buffers.

Acceptance:

- no blue-screen, mesh-only, or T-pose completion claim;
- frame state is sourced from project/VFS/script/native services;
- residual mismatches are named and become the next loop, not hidden.

Verified metric:

```text
ok=true scene_runtime_ok=true title_ui_ok=true title_branches_ok=true mission_event_thread_ok=true mission_tutorial_ok=true frame_updates=2 renderer_frame_ready=true ui_frame_ready=true save_frame_ready=true actor_frame_ready=true camera_frame_ready=true input_frame_ready=true event_frame_ready=true audio_frame_ready=true gameplay_command_count=221
```

Payload counts:

- renderer: 111 mesh draw candidates, 16 material bindings, 39 texture bindings;
- UI: 55 title UI commands, 9 draw commands;
- save/profile: 3 StartGame scenarios, 2 LoadAutoSave scenarios, 1 MakeNewGame scenario;
- actor/input: 1 actor instance, 6 player-control commands;
- camera: 2 camera commands, 3 rail nodes;
- event/tutorial: 16 event/dialog/tutorial commands, 1 `updateUnits` command;
- audio: 28 branch audio commands.

Boundary:

- L15 is not a playable loop and not a renderer backend;
- it is the first joined frame-level runtime contract across title UI, title branch, scene,
  actor, camera, input, event, tutorial, audio, and save/profile service state;
- the next non-blocked work is backend-facing renderer submission, not another isolated
  diagnostic report.

### L16: Renderer/Backend Submission Contract

Status: completed as backend-facing renderer submission contract checkpoint on 2026-06-09.

Deliver:

- consume title UI command payloads and scene-runtime mesh/material/texture handles through a
  backend-ready renderer command buffer;
- keep D3D9-compatible concepts where the original evidence implies them, but do not hard-code
  a blue screen or mesh preview as completion;
- connect renderer output to project/runtime lifecycle and service-owned state.

Acceptance:

- title background/logo/text draw commands and scene mesh/material commands are both present in
  one backend-facing frame contract;
- no renderer path bypasses project.json, VFS, original scripts, or Native Service state;
- unsupported shader/effect/material semantics remain explicit obligations.

First implementation cycle:

- define a renderer command-buffer model owned by Runtime/Render Service;
- consume L13 title UI commands as 2D draw submissions;
- consume L9/L15 stage mesh/material/texture payloads as scene draw submissions;
- expose one CLI/CTest gate that fails if title and scene submissions drift into separate,
  disconnected reports;
- keep unsupported shader, blend, effect, texture format, font, and device behavior as explicit
  backend obligations.

Verified metric:

```text
ok=true scene_runtime_ok=true title_ui_ok=true gameplay_frame_ok=true backend_frame_ready=true title_pass_ready=true world_pass_ready=true resource_upload_ready=true camera_submission_ready=true actor_submission_ready=true event_submission_ready=true submission_passes=3 backend_command_count=181 draw_submissions=121
```

Payload counts:

- backend: `d3d9_compatible`, 3 passes, 181 backend commands, 121 draw submissions;
- resource uploads: 57 submissions, 6 explicit backend obligations;
- title pass: 55 2D submissions, 3 graph submissions, 11 text submissions;
- world pass: 111 scene mesh submissions, 16 material bindings, 39 texture bindings, 1 collision
  debug submission;
- scene state: 1 camera submission, 1 actor submission, 1 event marker submission.

Boundary:

- L16 is not a real device backend and not visual parity;
- it proves title UI and scene/world renderer submissions share one backend-facing frame
  contract;
- the next non-blocked work is a service-owned frame scheduler/update graph, not another
  standalone CLI aggregation.

### L17: Service-Owned Frame Scheduler And Update Graph

Status: completed as service-owned frame scheduler/update graph checkpoint on 2026-06-09.

Deliver:

- move frame assembly from CLI-report composition toward a runtime-owned frame scheduler;
- define explicit update nodes for project lifecycle, script tick/render, native service mutation,
  scene update, actor/task update, camera update, event/tutorial update, audio update, renderer
  submission, and save/profile service state;
- make `renderer-submit` consume scheduler output rather than reconstructing state ad hoc;
- keep every node tied to existing evidence/service contracts and expose unresolved native/API
  behavior as obligations.

Acceptance:

- no isolated renderer report can pass if scheduler nodes are missing;
- title UI, title branch, scene-runtime, mission event, tutorial, and renderer submissions flow
  through one frame graph;
- frame graph diagnostics identify which service node owns each command and which next backend
  obligation remains.

Verified metric:

```text
ok=true gameplay_frame_ok=true renderer_submission_ok=true update_graph_ready=true frame_index=0 node_count=10 executed_nodes=10 service_node_count=10 scheduler_edges=12 gameplay_commands=221 renderer_backend_commands=181 scheduled_work_items=469 backend_obligations=6 unresolved_nodes=0
```

Scheduler nodes:

- `project_lifecycle`: Project Service, 1 command;
- `script_tick_render`: Script Service, 55 commands;
- `save_profile_update`: Save/Profile/Scenario Service, 6 commands;
- `scene_resource_update`: Scene And Stage Service, 166 commands;
- `actor_task_update`: Actor And Task Service, 7 commands;
- `camera_update`: Camera Service, 2 commands;
- `event_tutorial_update`: Event/Quest/Flag Service, 17 commands;
- `audio_update`: Audio Service, 28 commands;
- `renderer_submission`: UI And 2D Render Service, 181 commands;
- `backend_obligation_tracking`: Resource Service, 6 commands.

Boundary:

- L17 is not a real game loop or device backend;
- it replaces ad hoc CLI report composition with a runtime-owned frame graph;
- the next non-blocked work is to resolve concrete backend obligations behind scheduler nodes.

### L18: Backend Texture, Shader, Material, Font, And Device Obligations

Status: completed as backend obligation split and texture/material contract checkpoint on 2026-06-09.

Deliver:

- split the six backend obligations into concrete contracts:
  texture upload format, shader/effect semantics, material binding, blend/depth state, font atlas
  and glyph metrics, device/swapchain presentation, and oracle parity;
- start with texture upload and material/shader evidence because L16 already has 39 textures and
  16 material bindings;
- keep every implementation attached to scheduler nodes and renderer submission output.

Acceptance:

- no obligation can be silently removed;
- each backend obligation has a service owner, source evidence, runtime structure, CLI/CTest gate,
  and residual mismatch list;
- renderer submission and frame scheduler must fail if texture/material obligations detach from
  scene-runtime handles.

Verified metric:

```text
ok=true frame_scheduler_ok=true renderer_submission_ok=true texture_upload_contract_ready=true material_binding_contract_ready=true shader_effect_contract_tracked=true font_contract_tracked=true device_contract_tracked=true oracle_parity_contract_tracked=true texture_dependencies=39 texture_bytes_found=39 dds_textures=39 material_bindings=16 mesh_submissions=111 resolved_backend_contracts=2 tracked_backend_obligations=4 open_backend_obligations=4
```

Payload evidence:

- 39 DDS texture dependencies;
- 39 texture payloads found through VFS;
- 39 DDS magic headers confirmed;
- 23,773,408 total texture bytes;
- 16 material tags;
- 111 scene mesh submissions;
- 11 title text submissions.

Boundary:

- L18 does not create a GPU device and does not upload textures;
- texture upload format and material binding are now contract-ready evidence;
- shader/effect, font atlas/glyph metrics, device/swapchain, and original-frame parity remain
  open obligations.

### L19: Shader/Effect And Material Semantics Contract

Status: completed as material semantics and shader/effect tracking checkpoint on 2026-06-09.

Deliver:

- inspect model/material/effect evidence behind the 16 material tags and 111 mesh submissions;
- identify shader/effect resource references or binary payload structures if present;
- define material binding payload shape for mesh submission -> material -> texture slots;
- keep unresolved shader program, constant buffer, blend/depth, and sampler semantics as explicit
  obligations.

Acceptance:

- material semantics must consume the L18 texture/material contract;
- renderer submission and frame scheduler must fail if shader/effect obligations are dropped;
- no fake shader or hard-coded material defaults can satisfy the contract without evidence.

Verified metric:

```text
ok=true scene_runtime_ok=true renderer_submission_ok=true backend_obligations_ok=true material_semantics_contract_ready=true texture_slot_contract_ready=true mesh_material_contract_ready=true shader_effect_contract_tracked=true post_effect_source_tracked=true materials=16 material_parameter_blocks=16 texture_slots=39 resolved_texture_slots=39 mesh_submissions=111 named_mesh_submissions=110 mesh_material_bindings=110 unresolved_mesh_material_bindings=1 post_effect_techniques=5 post_effect_passes=5 post_effect_samplers=7
```

Payload evidence:

- `doujou.mdl` now materializes 16 `mat` blocks as runtime material handles;
- 39 material texture slots resolve through VFS;
- material names and texture-slot roles are recovered from model bytes, not hard-coded defaults;
- 110 named mesh submissions bind to material handles through original mesh-name suffix evidence;
- one mesh has no length-prefixed name before the `msh` tag and remains a tracked open gap;
- `resource/SMAA.fx` is tracked as a DX9 HLSL postprocess effect source with 5 techniques, 5
  passes, and 7 samplers.

Boundary:

- L19 does not create a GPU device and does not compile or bind shader bytecode;
- per-material shader/effect program semantics are still open because model material blocks expose
  texture slots but no shader/effect program token;
- the material semantics are now structured runtime handles and can be consumed by a device/backend
  layer without falling back to mesh viewers or hard-coded materials.

### L20: Device/Swapchain And Render-State Presentation Contract

Status: completed as device/swapchain/render-state presentation contract checkpoint on 2026-06-09.

Deliver:

- define the D3D9-compatible device/swapchain/presentation contract that consumes L16 renderer
  submissions, L18 texture upload evidence, and L19 material semantics;
- separate device creation, resource upload, render-state binding, draw submission, and present
  obligations instead of treating them as one renderer placeholder;
- keep blend/depth/sampler/font/oracle parity as explicit failure gates.

Acceptance:

- no blue-screen/window-only launcher can satisfy this stage;
- device contract must consume renderer submission, material semantics, and scheduler state;
- every unimplemented GPU behavior remains a named obligation with source evidence and tests.

Verified metric:

```text
ok=true frame_scheduler_ok=true renderer_submission_ok=true backend_obligations_ok=true material_semantics_ok=true device_profile_ready=true swapchain_contract_tracked=true resource_upload_plan_ready=true render_state_contract_tracked=true draw_queue_contract_ready=true present_contract_tracked=true renderer_profile=d3d9_compatible backbuffer_width_candidate=1280 backbuffer_height_candidate=720 renderer_backend_commands=181 resource_upload_submissions=57 draw_submissions=121 material_texture_slots=39 texture_bytes_found=39 post_effect_samplers=7 resolved_device_contracts=3 tracked_device_obligations=5 open_device_obligations=5
```

Payload evidence:

- device profile is `d3d9_compatible` from project manifest and renderer submission;
- renderer backend frame has 181 commands, 57 resource uploads, and 121 draw submissions;
- draw queue includes 55 title 2D submissions and 111 world mesh submissions;
- resource upload plan consumes 39 texture payloads and 39 material texture slots;
- `SMAA_PIXEL_SIZE` gives a 1280x720 backbuffer candidate;
- 7 SMAA samplers are tracked for render-state obligations.

Boundary:

- L20 does not create an HWND, D3D device, swapchain, or real `Present` call;
- sampler, blend/depth, material shader program, font atlas, and original-frame parity remain open;
- L20 prevents device work from becoming a blue-window demo by requiring scheduler, renderer,
  backend obligations, and material semantics to be present in the same contract.

### L21: Texture Upload, Render-State, Font, And Oracle Parity Edges

Status: completed as typed texture upload/runtime gate checkpoint on 2026-06-09.

Deliver:

- turn the L20 resource upload plan into typed texture upload records with DDS format, dimensions,
  mip/count candidates, and material slot consumers;
- split sampler, blend/depth, font atlas/glyph metrics, and original-frame parity into separate
  runtime gates;
- keep device/window creation blocked until upload/state/font contracts have failure evidence.

Acceptance:

- no texture can be marked uploaded without format/dimension evidence;
- font and oracle parity cannot be hidden behind generic renderer readiness;
- every stage remains attached to scheduler, renderer submission, backend obligations, material
  semantics, and device presentation.

Verification:

```text
ok=true scene_runtime_ok=true backend_obligations_ok=true material_semantics_ok=true device_presentation_ok=true texture_upload_runtime_ready=true dds_header_contract_ready=true payload_layout_contract_ready=true material_consumer_contract_ready=true sampler_state_gate_tracked=true blend_depth_state_gate_tracked=true font_atlas_gate_tracked=true oracle_parity_gate_tracked=true stage_texture_dependencies=39 texture_upload_records=39 valid_dds_headers=39 dxt1_textures=31 dxt5_textures=8 cube_map_textures=1 cube_map_faces=6 material_slot_consumers=39 unique_material_texture_uploads=38 duplicate_material_consumers=1 stage_only_texture_uploads=1 compressed_payload_matches=39 payload_byte_total=23768416 expected_payload_byte_total=23768416 mip9_textures=2 mip10_textures=14 mip11_textures=23 texture_width_min=256 texture_width_max=1024 post_effect_samplers=7 title_text_submissions=11 resolved_upload_contracts=4 tracked_upload_obligations=4 open_upload_obligations=4
```

Evidence now locked:

- 39 stage DDS dependencies become typed upload records, not directory-scan guesses;
- DDS formats are 31 DXT1 and 8 DXT5;
- mip distribution is 2 with 9 mips, 14 with 10 mips, and 23 with 11 mips;
- compressed payload totals match computed DDS mip-chain sizes: 23,768,416 bytes;
- 39 material slot consumers map to 38 unique material texture uploads;
- `kamidana_d.dds` has 2 material consumers;
- `cubeenvmap/doujou_1.dds` is a stage-only six-face DXT1 cube upload record;
- sampler, blend/depth, font atlas/glyph metrics, and original-frame oracle trace are separate
  tracked-open gates.

Boundary:

- L21 does not allocate GPU textures or call D3D;
- L21 does not bind sampler/blend/depth states;
- L21 does not implement font atlas rendering;
- L21 does not prove visual parity.

### L22: Backend Render-State And Font Atlas Records

Status: completed as backend state/font record checkpoint on 2026-06-09.

Deliver:

- consume L21 texture upload records and L20 render-state gates in a backend-facing state record;
- split material texture sampling, SMAA sampler declarations, blend/depth obligations, and title
  font metrics into typed backend records;
- keep actual D3D device allocation blocked until render-state and font records can fail with
  concrete missing evidence.

Acceptance:

- sampler/blend/depth cannot remain a single generic obligation;
- title text rendering cannot be marked ready without font atlas/glyph metric evidence;
- backend state records must consume scheduler, renderer submission, device presentation, and L21
  texture upload reports.

Verification:

```text
ok=true texture_upload_ok=true device_presentation_ok=true material_semantics_ok=true title_ui_ok=true backend_state_runtime_ready=true sampler_state_records_ready=true pass_render_state_records_ready=true font_atlas_records_ready=true material_shader_program_gate_tracked=true gpu_state_binding_gate_tracked=true oracle_parity_gate_tracked=true sampler_state_records=7 sampler_texture_bindings=7 sampler_clamp_address_records=7 sampler_linear_min_filters=6 sampler_point_min_filters=1 sampler_srgb_true_records=1 sampler_srgb_false_records=6 pass_state_records=5 pass_vs30_shaders=5 pass_ps30_shaders=5 z_disabled_passes=5 alpha_blend_disabled_passes=5 alpha_test_disabled_passes=5 srgb_write_enabled_passes=1 srgb_write_disabled_passes=4 stencil_enabled_passes=4 stencil_disabled_passes=1 stencil_replace_passes=3 stencil_keep_passes=1 stencil_equal_passes=1 font_query_records=6 text_draw_commands=6 graph_string_commands=5 string_size_queries=5 localized_menu_text_commands=10 texture_upload_records=39 material_texture_consumers=39 resolved_backend_state_contracts=4 tracked_backend_state_obligations=4 open_backend_state_obligations=4
```

Evidence now locked:

- `SMAA.fx` contributes 7 sampler state records with texture, address, filter, and SRGB fields;
- `SMAA.fx` contributes 5 pass state records with `vs_3_0`/`ps_3_0`, alpha, depth, SRGB, and
  stencil fields;
- title UI contributes 6 font queries, 6 text draws, 5 graph strings, and 5 string-size queries;
- backend state records consume 39 texture uploads and 39 material slot consumers from L21;
- GPU state binding, font atlas texture implementation, material shader program binding, and
  original-frame oracle trace remain tracked-open gates.

Boundary:

- L22 does not call D3D state APIs;
- L22 does not allocate font atlas textures;
- L22 does not compile material shaders;
- L22 does not render or compare frames.

### L23: D3D9-Compatible Resource Allocation Records

Status: completed as backend resource allocation record checkpoint on 2026-06-09.

Deliver:

- define backend resource allocation records for textures, cube maps, transient SMAA render
  targets, depth/stencil, and font atlas placeholders;
- keep actual device creation/present blocked until allocation records have typed failure gates;
- consume L21 texture uploads and L22 sampler/pass/font state records.

Acceptance:

- texture allocation records must preserve DDS format, dimensions, mip count, cube faces, and
  payload size;
- transient render targets must map to SMAA pass needs rather than generic fullscreen buffers;
- font atlas allocation must stay tracked-open unless glyph atlas dimensions and glyph cache
  evidence exist.

Verification:

```text
ok=true texture_upload_ok=true backend_state_ok=true resource_allocation_runtime_ready=true stage_texture_allocation_records_ready=true smaa_lookup_allocation_records_ready=true transient_surface_allocation_gate_tracked=true font_atlas_allocation_gate_tracked=true d3d_resource_creation_gate_tracked=true oracle_parity_gate_tracked=true allocation_records=46 ready_allocation_records=41 tracked_open_allocation_records=5 stage_texture_allocations=39 d3d_dxt1_allocations=31 d3d_dxt5_allocations=8 cube_texture_allocations=1 smaa_lookup_allocations=2 lookup_l8_allocations=1 lookup_a8l8_allocations=1 transient_surface_candidates=4 font_atlas_placeholders=1 sampler_texture_declarations=6 material_texture_consumers=39 ready_allocation_byte_total=23955042 ready_allocation_payload_bytes=23949794 ready_expected_payload_bytes=23949794 resolved_allocation_contracts=2 tracked_allocation_obligations=5 open_allocation_obligations=5
```

Evidence now locked:

- 39 stage DDS upload records map to D3D9 allocation records with format, dimensions, mip levels,
  cube faces, payload bytes, and material consumer counts preserved from L21;
- stage formats are 31 `D3DFMT_DXT1` and 8 `D3DFMT_DXT5`;
- `cubeenvmap/doujou_1.dds` is a six-face `D3DFMT_DXT1` cube texture allocation record;
- `system/glsl/smaa/areatexdx9.dds` is a ready 160x560 `D3DFMT_A8L8` lookup texture;
- `system/glsl/smaa/searchtex.dds` is a ready 66x33 `D3DFMT_L8` lookup texture;
- `colorTex2D`, `depthTex2D`, `edgesTex2D`, and `blendTex2D` are 1280x720 tracked-open transient
  surface candidates;
- `font_atlas_placeholder` remains tracked-open because no atlas dimensions or glyph cache
  ownership are recovered.

Boundary:

- L23 does not create an HWND, D3D device, swapchain, or real GPU resource;
- L23 does not upload texture levels or bind sampler/pass states;
- L23 does not allocate a font atlas or prove visual parity;
- the next non-blocked work is device resource creation/state-binding execution, not a visual demo.

### L24: D3D9-Compatible Device Resource Creation And Binding Execution

Status: completed as device execution record checkpoint on 2026-06-09.

Deliver:

- consume L23 allocation records into a device-facing creation plan for textures, cube textures,
  lookup textures, transient surfaces, and font atlas placeholders;
- define execution records for `CreateTexture`, `CreateCubeTexture`, `CreateRenderTarget`,
  `CreateDepthStencilSurface`, `SetTexture`, `SetSamplerState`, and `SetRenderState` equivalents;
- keep HWND/swapchain/present/oracle parity blocked until resource creation and state binding can
  fail independently.

Acceptance:

- no blue-window, clear-screen, or mesh-preview output can satisfy L24;
- every execution record must point back to L21/L22/L23 evidence;
- uncreated resources must stay blocked/tracked-open with named reasons, not disappear as silent
  backend defaults.

Verification:

```text
ok=true resource_allocation_ok=true backend_state_ok=true device_presentation_ok=true device_execution_runtime_ready=true resource_creation_records_ready=true upload_execution_records_ready=true state_binding_records_ready=true lookup_texture_binding_records_ready=true material_texture_binding_gate_tracked=true transient_surface_binding_gate_tracked=true font_atlas_execution_gate_tracked=true d3d_api_call_submission_gate_tracked=true present_oracle_gate_tracked=true resource_creation_records=46 ready_resource_creation_records=41 tracked_open_resource_creation_records=5 create_texture_records=40 create_cube_texture_records=1 render_target_creation_candidates=3 depth_stencil_creation_candidates=1 font_atlas_creation_placeholders=1 texture_upload_execution_records=41 upload_subresource_records=458 ready_upload_payload_bytes=23949794 binding_records=57 ready_binding_records=14 tracked_open_binding_records=43 material_texture_binding_records=38 material_texture_binding_slots=39 sampler_texture_binding_records=7 lookup_texture_binding_records=2 transient_sampler_binding_candidates=5 sampler_state_binding_records=7 render_state_binding_records=5 resolved_device_execution_contracts=4 tracked_device_execution_obligations=5 open_device_execution_obligations=5
```

Evidence now locked:

- 46 L23 allocation records map to device-facing creation operations;
- 41 ready textures expand to 458 upload subresources;
- `cubeenvmap/doujou_1.dds` maps to `CreateCubeTexture` with 54 upload subresources;
- stage and lookup 2D textures map to 40 `CreateTexture` records;
- SMAA transient surfaces split into 3 render-target candidates and 1 depth/stencil candidate;
- 57 binding records include 38 material texture candidates, 7 SMAA sampler texture bindings,
  7 sampler-state records, and 5 pass render-state records;
- material shader ownership, transient surface formats/ownership, font atlas implementation,
  real D3D API calls, and present/oracle parity remain tracked-open gates.

Boundary:

- L24 does not create a D3D device or call any `IDirect3DDevice9` method;
- L24 does not upload GPU memory, draw, present, or compare frames;
- the next non-blocked work is swapchain/present/original-frame oracle parity records, not a
  blue-window shortcut.

### L25: Swapchain, Present, And Original-Frame Oracle Parity Records

Status: completed after L24.

Deliver:

- consumed L20 device presentation and L24 device execution records into 7 presentation/oracle
  records;
- defined HWND/window surface, swapchain, present, frame capture, and original-frame oracle trace
  gates as independent records;
- kept real platform API submission blocked until present/oracle evidence can fail cleanly.

Acceptance:

- `yuengine_cli present-oracle samples/touhou_new_world/project.json --repo-root .` reports
  `presentation_oracle_runtime_ready=true`;
- CTest `yuengine_present_oracle_contract` locks 7 records, 2 ready records, 5 tracked-open
  obligations, 1280x720 backbuffer extent, 103 linked L24 device execution records, 181 renderer
  backend commands, and 121 draw submissions;
- missing window surface, swapchain creation, present execution, frame capture artifact, and
  original graphics trace remain explicit obligations.

Boundary:

- L25 does not create an HWND, swapchain, D3D device, captured frame, or original graphics trace;
- no clear-screen or mesh-only output can satisfy L25;
- L25 only closes the presentation/oracle record layer so L26 can submit real platform API calls
  without guessing.

### L26: Real Platform D3D API Submission And Backend Bridge

Status: completed after L25.

Deliver:

- introduced a platform/backend bridge that consumes L24 creation/upload/binding execution records
  and L25 presentation/oracle records;
- defined 10 D3D9-style call batches for window surface creation, Direct3D interface/device
  creation, resource creation, upload, state binding, draw, present, and capture/oracle;
- kept concrete D3D calls, frame capture, and original-frame parity as explicit gates.

Acceptance:

- `yuengine_cli platform-bridge samples/touhou_new_world/project.json --repo-root .` reports
  `platform_bridge_runtime_ready=true`;
- CTest `yuengine_platform_bridge_contract` locks 10 call records, 4 ready records, 6 tracked-open
  records, 103 L24 inputs, 7 L25 inputs, 57 ready platform inputs, 53 tracked-open platform inputs,
  46 creation calls, 458 upload subresource calls, 57 state-binding calls, 121 draw submissions,
  1 present call, and 2 capture/oracle calls;
- no bridge path bypasses scheduler, renderer submission, material semantics, resource allocation,
  device execution, or presentation/oracle records.

Boundary:

- L26 does not execute `Direct3DCreate9`, `CreateDevice`, `CreateTexture`, `SetTexture`, draw,
  `Present`, frame capture, or oracle comparison;
- L26 only defines the platform bridge queue and ready/open accounting;
- concrete execution must consume these records through an executor interface.

### L27: Concrete Backend Executor Interface And Diagnostic D3D9 Adapter

Status: completed after L26.

Deliver:

- introduced a backend executor interface that consumes L26 call records and returns per-call
  execution results;
- implemented a diagnostic D3D9 adapter mode for CTest that preserves API names, ordering, input
  counts, and failure obligations without requiring a real HWND;
- kept the real HWND/D3D9 mode gated until platform window creation, device creation, and frame
  capture requirements are explicitly satisfied.

Acceptance:

- `yuengine_cli backend-executor samples/touhou_new_world/project.json --repo-root .` reports
  `executor_runtime_ready=true`;
- CTest `yuengine_backend_executor_contract` locks 10 executor result records, 4 diagnostic-success
  records, 6 tracked-open execution records, 562 diagnostic executed calls, 127 preserved open
  calls, 110 linked platform input records, and 1280x720 backbuffer extent;
- no standalone renderer, manual mesh path, or clear-screen path satisfies L27.

Boundary:

- L27 does not create an HWND or D3D9 device;
- L27 does not execute draw, present, frame capture, or oracle comparison;
- L27 only closes the executor result/accounting layer so L28 can implement the real platform
  device path without guessing.

### L28: Real HWND And D3D9 Device Creation Adapter Records

Status: completed after L27.

Deliver:

- consumed L27 executor results and split the concrete platform gates into typed adapter records;
- explicitly gated the real HWND/window surface, `Direct3DCreate9`, and `CreateDevice`
  preconditions through the same runtime path;
- carried the original 1280x720 backbuffer, renderer profile, and L26/L27 call ordering into the
  adapter contract;
- kept resource creation/upload/draw/present/capture blocked until a concrete device handle exists.

Acceptance:

- `yuengine_cli backend-device-adapter samples/touhou_new_world/project.json --repo-root .`
  reports `device_adapter_runtime_ready=true`;
- CTest `yuengine_backend_device_adapter_contract` locks 10 adapter records, 3 platform/device
  precondition records, 6 downstream blocked records, 688 blocked real calls, 0 real executed
  calls, and the inherited 1280x720 backbuffer extent;
- no platform/device record is invented without a source L27 result and source L26 bridge record;
- no blue-window, clear-screen, mesh preview, or separate sample app satisfies L28.

Boundary:

- L28 does not create HWND, Direct3D interface, or D3D9 device handles;
- L28 does not execute resource creation/upload/state/draw/present/capture;
- L28 only closes the device-adapter precondition layer so L29 can create or explicitly fail the
  real platform device path.

### L29: Concrete HWND And D3D9 Device Creation Execution

Status: completed after L28.

Deliver:

- consumed the 3 L28 platform/device precondition records;
- implemented hidden Win32 HWND creation, `Direct3DCreate9`, and
  `IDirect3D9::CreateDevice` execution result records;
- preserved the inherited 1280x720 backbuffer and renderer profile;
- kept all downstream resource/upload/state/draw/present/capture queues preserved for the next
  device-service layer.

Acceptance:

- `yuengine_cli backend-device-create samples/touhou_new_world/project.json --repo-root .`
  reports `device_creation_runtime_ready=true`;
- CTest `yuengine_backend_device_creation_contract` locks 3 execution records, 3 consumed adapter
  preconditions, the inherited 1280x720 backbuffer, and 685 deferred downstream real calls;
- on the current Windows test machine, the hidden HWND, `IDirect3D9`, and `IDirect3DDevice9`
  creation records all report `real_success`;
- no standalone launcher, preview window, or manual render path satisfies L29.

Boundary:

- L29 creates and releases a concrete device during the creation attempt; it does not persist a
  runtime-owned device service handle;
- L29 does not execute resource creation/upload/state/draw/present/capture;
- L29 only closes the platform device creation edge so L30 can own the device service and execute
  resource queues without bypassing recovered backend records.

### L30: Persistent Device Service And Resource Creation Queue Execution

Status: completed after L29.

Deliver:

- introduce a runtime-owned backend device service handle that preserves the L29 HWND/D3D9 device
  creation result beyond the creation attempt;
- consume L24/L26/L28/L29 resource creation records through the service, not through a standalone
  sample renderer;
- execute ready `CreateTexture`, `CreateCubeTexture`, SMAA lookup texture creation, transient
  render target/depth candidates, and font atlas placeholder handling against the real device;
- keep upload/state/draw/present/capture blocked until resource handles are real and accounted.

Acceptance:

- every created resource handle must point back to its source allocation/execution record;
- failed resource creation must be explicit and cannot downgrade to diagnostic success;
- CTest must lock resource handle counts, ready/open accounting, D3D formats, payload sizes, and
  downstream blocked queues.

Verified:

- `yuengine_cli backend-resource-create samples/touhou_new_world/project.json --repo-root .`
  reports `resource_creation_runtime_ready=true`;
- CTest `yuengine_backend_resource_creation_contract` locks 46 source records, 41 ready records,
  5 tracked-open records, 41 service-retained D3D9 resource handles, 40 texture creations, 1 cube
  texture creation, 2 SMAA lookup texture creations, 23,949,794 ready payload bytes, and the
  inherited 1280x720 backbuffer extent;
- on the current Windows test machine, all 41 ready resource creations report `real_success`;
- transient surfaces and the font atlas remain explicit tracked-open records.

Boundary:

- L30 creates resources but does not upload DDS payloads;
- L30 does not bind texture/sampler/render state;
- L30 does not issue draw, present, capture, or oracle comparison calls;
- L30 only closes the persistent device and resource creation edge so L31 can upload and bind
  through real resource handles.

### L31: Texture Upload And Backend State Binding Execution

Status: completed after L30.

Deliver:

- keep the L30 backend device service and resource handles alive through upload and state binding;
- execute ready `LockRect`/`UnlockRect` upload records for DXT stage textures, cube faces/mips,
  and SMAA lookup textures;
- execute ready `SetTexture`, `SetSamplerState`, and render-state bundle records from the recovered
  backend state chain;
- preserve material shader ownership, transient surface bindings, font atlas bindings,
  draw/present/capture, and oracle parity as explicit downstream gates.

Acceptance:

- every upload record must point back to its created resource handle and source DDS payload;
- upload byte counts, mip/face indexing, and ready/open accounting must be locked by CTest;
- every state binding must point back to the recovered sampler/pass/material records;
- failed upload or state calls must be explicit and cannot be downgraded to diagnostic success;
- draw/present/capture remain blocked until upload and state binding are actually complete.

Verified:

- `yuengine_cli backend-upload-bind samples/touhou_new_world/project.json --repo-root .`
  reports `upload_binding_runtime_ready=true`;
- CTest `yuengine_backend_upload_binding_contract` locks 41 retained resource handles, 458 uploaded
  subresources, 23,949,794 uploaded payload bytes, 14 executed ready binding records, 43 preserved
  tracked-open binding records, and the inherited 1280x720 backbuffer extent;
- real D3D9 calls executed on this machine: 41 texture upload records through `LockRect`/
  `UnlockRect`, 2 `SetTexture` lookup binds, 7 `SetSamplerState` bundles, and 5 `SetRenderState`
  bundles;
- material shader slot ownership, transient SMAA surface bindings, font atlas binding, draw,
  present, capture, and oracle parity remain explicit open gates.

Boundary:

- L31 uploads and binds ready backend state but does not issue draw calls;
- L31 does not bind stage material textures until material shader slot ownership is recovered;
- L31 does not create transient render targets/depth surfaces or a font atlas;
- L31 only closes the upload/binding edge so L32 can resolve remaining render-target/material/font
  gates before draw execution.

### L32: Transient Surfaces, Material Shader Binding, And Font Atlas Resolution

Status: completed as backend surface/material/font gate checkpoint on 2026-06-09.

Deliver:

- resolve SMAA transient render target/depth surface formats and create real D3D9 surfaces;
- strengthen material shader evidence and preserve material texture binding until pass/sampler slot
  ownership is recovered;
- preserve font atlas creation as an explicit blocker with title font metric evidence;
- preserve draw/present/capture/oracle as downstream gates until material, depth, and font gates are
  closed.

Acceptance:

- `yuengine_cli backend-surface-material-font samples/touhou_new_world/project.json --repo-root .`
  reports `surface_material_font_runtime_ready=true`;
- CTest `yuengine_backend_surface_material_font_contract` locks 3 render-target textures, 1
  depth/stencil surface, 4 transient render-target `SetTexture` calls, 38 preserved material
  texture bindings, 1 preserved depth texture binding, and the inherited 1280x720 extent;
- real D3D9 calls executed on this machine: `CreateTexture(D3DUSAGE_RENDERTARGET)` for
  `colorTex2D`, `edgesTex2D`, and `blendTex2D`; `CreateDepthStencilSurface` for `depthTex2D`;
  `SetTexture` for `colorTex`, `colorTexG`, `edgesTex`, and `blendTex`;
- `.bfx` material shader evidence is tracked, but per-material shader pass/sampler slot ownership
  remains open;
- font query/string-size evidence is tracked, but font atlas dimensions/cache ownership remain open.

Boundary:

- L32 does not bind stage material textures to guessed shader slots;
- L32 does not treat the D24S8 depth surface as a sampleable depth texture;
- L32 does not create or populate a font atlas;
- L32 does not issue draw/present/capture/oracle calls.

### L33: Material Shader Slot, Depth Texture, And Font Atlas Recovery

Status: completed as backend shader sampler reflection checkpoint on 2026-06-09.

Deliver:

- recover `.bfx` CTAB sampler reflection from shipped shader binaries;
- map material texture roles to recovered shader sampler registers when evidence exists;
- preserve material program/pass selection, lightmap sampler, DX9 depth texture sampler, font atlas,
  draw, present, capture, and oracle as explicit downstream gates.

Acceptance:

- `yuengine_cli backend-shader-sampler samples/touhou_new_world/project.json --repo-root .`
  reports `shader_ctab_reflection_ready=true` and `material_sampler_role_map_ready=true`;
- CTest `yuengine_backend_shader_sampler_contract` locks 39 scanned `.bfx` files, 39 files with
  CTAB, 1105 CTAB chunks, 337 unique sampler records, 107 material-compatible sampler records,
  24 shader files with material-compatible samplers, 39 material texture slots, 38 resolved sampler
  slots, 1 unresolved lightmap slot, and the inherited 1280x720 extent;
- `kabe1` base/normal/specular slots resolve to `SamplerDiffuse` s0, `SamplerNormal` s1, and
  `SamplerSpecular` s2 in `system/shader/mesh30.bfx` `ps_3_0`;
- `ki:tree` lightmap remains `tracked_open`;
- material program/pass selection, depth texture sampler behavior, font atlas/cache ownership, and
  draw/present/capture/oracle remain blocked until L34+.

Boundary:

- L33 does not choose exact shader programs/passes for mesh draw calls;
- L33 does not treat the lightmap slot as resolved;
- L33 does not treat the D24S8 depth/stencil surface as a sampleable depth texture;
- L33 does not create or populate a font atlas;
- L33 does not issue draw/present/capture/oracle calls.

### L34: Material Program, Lightmap, Depth, And Font Evidence Gate

Status: completed as backend program/depth/font evidence checkpoint on 2026-06-09.

Deliver:

- inventory recovered mesh/grass/deferred shader program candidates without choosing a guessed draw
  program;
- prove material program/pass tokens are absent from parsed material records and keep exact program
  selection blocked;
- prove lightmap/depth sampler token evidence while keeping exact lightmap binding and sampleable
  depth implementation blocked;
- recover title/menu font atlas FMP/DDS resource ownership, dimensions, mip count, format, and
  payload parity;
- keep draw/present/capture/oracle deferred.

Acceptance:

- `yuengine_cli backend-program-depth-font samples/touhou_new_world/project.json --repo-root .`
  reports `shader_program_candidate_evidence_ready=true`,
  `font_atlas_resource_evidence_ready=true`, and `downstream_draw_present_deferred=true`;
- CTest `yuengine_backend_program_depth_font_contract` locks 12 shader program candidate files, 0
  material program token hits, 3 `SamplerLight` records, 33 depth sampler records, 4 FMP font maps,
  7 font atlas links, 7 ready `D3DFMT_A8` 4096x4096 two-mip atlas DDS payloads, and ready/open
  accounting of 5 resolved and 6 tracked-open obligations;
- exact material program/pass selection, lightmap material binding, sampleable depth texture,
  glyph-record layout/font texture upload, draw/present/capture/oracle remain tracked-open.

Boundary:

- L34 does not select a scene mesh shader program/pass;
- L34 does not bind the lightmap slot to `SamplerLight`;
- L34 does not sample from `depthTex2D`;
- L34 does not create/upload/bind font atlas textures or decode the FMP glyph record stride;
- L34 does not issue draw/present/capture/oracle calls.

### L35: Font Atlas Texture And Glyph Layout Runtime

Status: completed as backend font atlas runtime checkpoint on 2026-06-09.

Deliver:

- use L34 FMP/DDS evidence to define and execute font atlas D3D9 texture creation and DDS payload
  upload records;
- recover enough FMP glyph record layout to map title text/string-size commands to atlas regions, or
  keep the exact missing fields as tracked-open;
- keep scene material program selection, lightmap program binding, sampleable depth texture, and
  draw/present/capture/oracle deferred until their evidence gates close.

Acceptance:

- `yuengine_cli backend-font-atlas samples/touhou_new_world/project.json --repo-root .` reports
  `font_atlas_d3d_texture_creation_executed=true`,
  `font_atlas_payload_upload_executed=true`, and `fmp_glyph_layout_probe_ready=true`;
- CTest `yuengine_backend_font_atlas_contract` locks 4 FMP files, 20854 32-byte glyph records, 7
  tail atlas links, 7 real `D3DFMT_A8` texture creations, 14 uploaded mip subresources,
  146800640 uploaded payload bytes, and ready/open accounting of 5 resolved and 6 tracked-open
  obligations;
- exact non-ASCII codepoint encoding, glyph quad/text draw backend, material program selection,
  sampleable depth, draw/present/capture/oracle remain tracked-open.

Boundary:

- L35 does not fully name non-ASCII FMP codepoint encoding;
- L35 does not build glyph quads or execute a text draw backend;
- L35 does not choose material shader programs/passes;
- L35 does not bind lightmaps or make D24S8 depth sampleable;
- L35 does not issue draw/present/capture/oracle calls.

### L36: Material Program, Lightmap, And Depth Closure

Status: completed as backend material-program evidence checkpoint on 2026-06-09, with exact
selector/depth implementation deliberately still tracked-open.

Deliver:

- turn material program ownership from a loose open gate into per-material records driven by original
  material blocks, texture-role patterns, `.bfx` CTAB tokens, and renderer profile evidence;
- bind the `ki:tree` lightmap material slot only when its selected program and `SamplerLight`
  ownership are proven;
- prove whether the shipped resources contain a sampleable depth texture path separately from the
  D24S8 depth/stencil surface;
- keep draw/present/capture/oracle deferred until these gates close.

Acceptance:

- `yuengine_cli backend-material-program samples/touhou_new_world/project.json --repo-root .`
  reports `material_program_rule_selection_ready=true`,
  `material_sampler_slot_closure_ready=true`, `lightmap_program_binding_ready=true`, and
  `sampleable_depth_negative_evidence_ready=true`;
- CTest `yuengine_backend_material_program_contract` locks 16 material program records, 16 original
  material block scans, 0 material shader/pass token hits, 12 `deferred`, 2 `deferredGrass`, and 2
  `deferredMulti` program records, 39/39 resolved sampler slots, 1 `SamplerLight` binding, 1 SMAA
  depth technique pass, 0 sampleable-depth format tokens, and ready/open accounting of 8 resolved
  and 4 tracked-open obligations;
- exact original C++ program selector function, sampleable depth implementation,
  draw/present/capture/oracle remain tracked-open.

Boundary:

- L36 records per-material rule-derived shader choices; it does not claim the original C++ selector
  function has been recovered;
- L36 binds `ki:tree` to `SamplerLight` through selected deferred CTAB evidence;
- L36 does not treat `depthTex2D` as sampleable;
- L36 does not issue draw/present/capture/oracle calls.

### L36b: Binary Selector And Depth Candidate Evidence

Status: completed as binary-evidence checkpoint on 2026-06-09, with selector CFG and depth runtime
selection deliberately still tracked-open.

Deliver:

- consume original `bin/game.exe` as read-only binary evidence;
- prove the L36 deferred/deferredGrass/deferredMulti selected programs are present in the original
  `mgRenderMesh.cpp` shader table;
- prove the original binary contains SMAA depth and DX9 sampleable-depth candidate format evidence;
- keep draw/present/capture/oracle deferred until selector CFG and depth runtime selection are
  reconstructed.

Acceptance:

- `yuengine_cli backend-material-program-binary samples/touhou_new_world/project.json --repo-root .`
  reports `binary_deferred_selector_table_ready=true`,
  `selected_programs_backed_by_binary_table=true`,
  `sampleable_depth_binary_candidate_ready=true`,
  `selector_control_flow_still_open=true`, and
  `sampleable_depth_selection_still_open=true`;
- CTest `yuengine_backend_material_program_binary_contract` locks 28 original binary shader path
  tokens, 10 filter path tokens, 3 deferred selector tokens, 2 SMAA depth tokens, 1 packed
  `INTZ/RAWZ/DF24/DF16` post-filter format record, 6 render depth format tokens, 2 RSM depth
  labels, and ready/open accounting of 9 resolved and 4 tracked-open obligations;
- draw/present/capture/oracle remain tracked-open.

Boundary:

- L36b proves binary string-table ownership, not control-flow ownership;
- L36b proves sampleable-depth candidates exist, not which candidate is selected for the running
  frame;
- L36b does not issue draw/present/capture/oracle calls.

### L36c0: Binary Dispatch Table And Xref Probe Before CFG

Status: completed as PE dispatch evidence checkpoint on 2026-06-10.

Deliver:

- parse original `game.exe` PE64 layout and `.pdata` function ranges inside YuEngine tooling;
- prove whether shader/depth binary tokens have direct VA/RVA/rel32 executable xrefs;
- bind packed depth-format anchors to adjacent function-table candidates without claiming selected
  runtime path;
- probe D3D/D3DX import presence and direct IAT call edges.

Acceptance:

- `yuengine_cli backend-material-program-binary-dispatch samples/touhou_new_world/project.json --repo-root .`
  reports `pe_image_ready=true`, `pe_pdata_ready=true`,
  `binary_string_direct_xref_probe_ready=true`, `selector_direct_xrefs_absent=true`,
  `depth_adjacent_function_table_ready=true`, `rsm_depth_function_table_ready=true`,
  `sampleable_depth_dispatch_table_candidate_ready=true`, `d3d_import_table_ready=true`,
  `d3d_import_direct_calls_absent=true`, `exact_selector_control_flow_still_open=true`, and
  `sampleable_depth_runtime_selection_still_open=true`;
- CTest `yuengine_backend_material_program_binary_dispatch_contract` locks 8 PE sections, 2
  executable sections, 20225 `.pdata` functions, 9/9 binary probe tokens, 0 direct token xrefs,
  24 packed-depth adjacent pointers with 9 exact function starts, 6 RSM depth adjacent pointers
  with 5 exact function starts, 5/5 D3D/D3DX imports, and 0 direct IAT call hits;
- draw/present/capture/oracle remain tracked-open.

Boundary:

- L36c0 proves dispatch-table candidates and absence of simple xrefs, not selector CFG;
- L36c0 proves adjacent depth function-table evidence, not current-frame depth format selection;
- D3D/D3DX imports are present, but effect/texture creation wrapper control-flow is still open;
- L36c0 does not issue draw/present/capture/oracle calls.

### L36c: Selector CFG And Depth Runtime Selection Before Draw

Status: active after L36c0.

Deliver:

- reconstruct the material selector branch/control-flow around the original shader path table;
- reconstruct sampleable-depth format selection/copy behavior around `INTZ/RAWZ/DF24/DF16` and
  `depthTex2D`;
- use the L36c0 `.pdata` function-table candidates and D3D/D3DX import-wrapper gap as the next
  binary control-flow evidence, not as a completion claim;
- only after those are closed, unlock draw submission as the next edge.

Acceptance:

- selected program records must be backed by selector CFG evidence, not only binary strings;
- depth path must name the concrete selected format/copy/resolve path for the frame;
- draw/present/capture/oracle must remain deferred until the above evidence is locked.

## Stop Conditions

Do not stop unless:

- the full target is achieved;
- a hard blocker requires user action and no non-blocked loop task remains;
- the user explicitly interrupts or redirects.

## Verification Speed

Old full CTest was too slow because backend contract tests started separate CLI processes and
repeatedly rebuilt the runtime chain. Default CTest now registers only the second-level
`yuengine_smoke_validate_touhou` project manifest check. Current deepest runtime contracts are
explicit EDGE checks, not default CTest checks. Full aggregate CTest and legacy per-contract CTest
are explicit CMake modes. The default edit-loop verification path is:

```powershell
tools\verify_runtime.ps1
```

Use targeted edge verification when a named contract must be exercised:

```powershell
tools\verify_runtime.ps1 -Mode edge -Filter <contract-name> -Jobs 8
```

The current deepest edge can be run without spelling the filter:

```powershell
tools\verify_runtime.ps1 -Mode edge -Jobs 8
```

Use full parallel verification before major commits:

```powershell
tools\verify_runtime.ps1 -Mode full -Jobs 8
```

Use `-CleanBuild` after C++ header, ABI-like struct, or build-system changes. Bare
`ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure` now runs only the smoke validate
test, not the aggregate suite, not the current deepest edge, and not the old per-contract process
path. Configure `-DYUENGINE_CTEST_MODE=EDGE` only when CTest integration for the current deepest
contract must be checked, and pair it with `-DYUENGINE_ENABLE_SLOW_CTESTS=ON`. Full aggregate
CTest has the same slow-test opt-in. Legacy per-contract CTest requires
`-DYUENGINE_ENABLE_SLOW_CTESTS=ON`, `-DYUENGINE_CTEST_MODE=LEGACY`, and
`-DYUENGINE_ENABLE_LEGACY_CTESTS=ON`; stale EDGE/FULL/LEGACY cache values without the slow-test
opt-in are forced back to FAST smoke.

Current measured speed after caching and scene reuse:

- `ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure`: after the smoke split, 1/1
  `yuengine_smoke_validate_touhou` passed; CTest total 0.01 seconds, PowerShell outer measurement
  about 0.06 seconds.
- `tools\verify_runtime.ps1 -NoBuild`: after the smoke split, smoke validate plus Python unittest
  and `git diff --check`, about 0.354 seconds.
- `tools\verify_runtime.ps1 -Mode edge -NoBuild -SkipPython -SkipDiffCheck`: previous L36 edge
  filter, elapsed_ms=42281.
- `runtime-contract-suite --filter yuengine_backend_material_program_binary_contract`: L36b edge
  filter, elapsed_ms=64216.
- `tools\verify_runtime.ps1 -Mode edge -NoBuild -SkipPython -SkipDiffCheck`: L36b edge filter,
  elapsed_ms=65678.
- `runtime-contract-suite --filter yuengine_backend_material_program_binary_dispatch_contract`:
  current deepest L36c0 edge filter, elapsed_ms=53143.
- `tools\verify_runtime.ps1 -Mode edge -NoBuild -SkipPython -SkipDiffCheck`: current deepest
  L36c0 edge filter, elapsed_ms=48696.
- `yuengine_backend_device_creation_contract`: 1/1 CTest passed in 21.88 seconds.
- `yuengine_backend_resource_creation_contract`: 1/1 CTest passed in about 22 seconds.
- `yuengine_backend_upload_binding_contract`: 1/1 CTest passed in about 25 seconds.
- `yuengine_backend_surface_material_font_contract`: suite filter passed in 23.35-24.8 seconds.
- `yuengine_backend_shader_sampler_contract`: default bare CTest passed in 23.97-24.63 seconds.
- `yuengine_backend_program_depth_font_contract`: suite filter passed in 34.04 seconds; default
  bare CTest passed in 34.08-34.41 seconds.
- `yuengine_backend_font_atlas_contract`: direct `backend-font-atlas` passed in about 43.9 seconds;
  suite filter passed in 43.883 seconds; pre-smoke-split default bare CTest passed in 43.94
  seconds.
- `tools\verify_runtime.ps1 -SkipPython -SkipDiffCheck -NoBuild`: pre-smoke-split fast L35
  contract passed with elapsed_ms=43247.
- `tools\verify_runtime.ps1 -SkipPython -SkipDiffCheck -NoBuild`: fast L34 contract passed in 33.73
  seconds.
- `tools\verify_runtime.ps1 -Mode full -SkipPython -SkipDiffCheck -NoBuild`: direct
  `runtime-contract-suite` passed 39/39 contracts in 51.78 seconds.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: clean build, Python unittest, 40/40
  contracts, and `git diff --check` passed in about 86.8 seconds; runtime suite elapsed_ms=62881.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: after L35, clean build, Python
  unittest, 41/41 contracts, and `git diff --check` passed in about 93.6 seconds; runtime suite
  elapsed_ms=69923.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: after L36, clean build, Python
  unittest, 42/42 contracts, and `git diff --check` passed in about 138 seconds; runtime suite
  elapsed_ms=114358.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: after L36b, clean build, Python
  unittest, 43/43 contracts, and `git diff --check` passed in about 140 seconds; runtime suite
  elapsed_ms=107854.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: after L36c0, clean build, Python
  unittest, 44/44 contracts, and `git diff --check` passed in about 112 seconds; runtime suite
  elapsed_ms=84995.
- direct `backend-device-adapter` CLI: 113.06 seconds before caching, 21.46 seconds after caching.
- direct `runtime-contract-suite`: 44/44 contracts after L36c0 binary-dispatch checkpoint.
