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

- continue immediately into L14 save/load/continue/options branch coverage;
- preserve L13 title UI command payloads as the regression gate for any menu branch change;
- start L15 gameplay-frame work once branch service state can feed actor/camera/input/event.

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

Status: active after L13.

Deliver:

- execute original title bytecode branches for Continue, Load/Overwrite, Options, and Exit where
  platform policy allows;
- expand Save/Profile/Scenario Service return schemas beyond the current empty/new-game path;
- keep Steam/login/entitlement as replaceable Platform Service state, not bypass logic.

Acceptance:

- no fake save menu state;
- branch transitions and enabled/disabled states are driven by title scripts and service inputs;
- every unsupported platform/save behavior is an explicit runtime obligation.

### L15: Gameplay Frame Update Loop Contract

Status: queued after L14 has enough branch/service state, but renderer/input/event pieces can
advance in parallel if they consume existing scene-runtime and title-ui payloads.

Deliver:

- join input, actor/task, camera, event, renderer, audio, and save services into a repeated
  gameplay-frame update contract;
- consume scene-runtime handles and mission event state in one frame report;
- begin replacing diagnostics-only frame reports with backend-ready command buffers.

Acceptance:

- no blue-screen, mesh-only, or T-pose completion claim;
- frame state is sourced from project/VFS/script/native services;
- residual mismatches are named and become the next loop, not hidden.

### L16: Renderer/Backend Submission Contract

Status: queued after L13, can advance in parallel with L14/L15 if it consumes existing contracts.

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

## Stop Conditions

Do not stop unless:

- the full target is achieved;
- a hard blocker requires user action and no non-blocked loop task remains;
- the user explicitly interrupts or redirects.
