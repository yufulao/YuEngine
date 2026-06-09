# L7 Title Script Execution Status

Status: active. Title entry bytecode-state checkpoint completed on 2026-06-09.

This document tracks L7. The current implementation executes original `.sqasm` with a scoped
multi-module bytecode state pass for the title boot path. It is now PC-branch aware for
`_OP_JZ/_OP_JNZ/_OP_JMP`, handles `_OP_FOREACH/_OP_POSTFOREACH` table iteration for the title
scene list, resolves inherited methods across baseline modules, and no longer relies on a hard
stop inside `ModuleTitle.main`. It is still not a full Squirrel VM, title UI, or gameplay runtime.

## Implemented So Far

- `yu::script::ScriptRuntime`
- `yuengine_cli script-plan`
- Entry execution planning from `project.json` startup fields.
- Root class method table recovery from `_OP_CLASS` / `_OP_CLOSURE` / `_OP_NEWSLOTA`.
- Root object construction binding from `_OP_DLOAD` / `_OP_PREPCALL` / `_OP_CALL` / `_OP_NEWSLOT`.
- Direct call resolution for the title entry function:
  - native/API obligation;
  - builtin;
  - script object method;
  - unique script function;
  - ambiguous script function;
  - unresolved.
- `yuengine_cli script-run`
- Startup baseline module loading from manifest `preload_scripts` and `dependency_scripts`.
  Current Touhou title baseline loads `preload.b64` and `script/menu/menudef.b64` before
  `script/menu/titlemenu.b64`, so `ModuleTitle : ModuleBase` sees inherited class defaults.
- Cross-module method identity and dispatch:
  - method bindings retain the source `.sqasm` module path;
  - object/owner/scene method lookup walks class-base chains;
  - inherited methods execute against their defining module instead of the entry module.
- Root function export recovery from root `_OP_LOAD/_OP_CLOSURE/_OP_NEWSLOT`.
- Root object method slot recovery for `gMenu.*` closures.
- Static execution bridge:
  - constructs `modTitle -> ModuleTitle`;
  - executes `ModuleTitle.constructor`;
  - constructs `modTitle._scenes[0..3]` from original class constructor call sites;
  - executes `setupProc -> print + modTitle.init`;
  - dispatches `ModuleTitle.init` foreach scene init;
  - executes one boot-frame `main` wrapper when `--frames 1` is supplied;
  - dispatches native/API calls through `NativeServiceCatalog`.
- Runtime gap classification for the current trace:
  - engine object calls;
  - UI helper object calls;
  - value helper calls;
  - Squirrel/native value method calls;
  - inherited script methods, with lifecycle hooks only as an obsolete fallback category.
- Bytecode state pass for the executed title boot edge:
  - register slots for the recovered functions;
  - PC-indexed branch execution for `_OP_JZ`, `_OP_JNZ`, `_OP_JMP`, and `_OP_RETURN`;
  - table/object foreach execution for the title `_scenes` init loop;
  - method-context object lookup fallback to root/global slots so original bytecode resolves `gMenu`;
  - root/class/object/table slot writes;
  - class default materialization onto constructed script objects;
  - corrected `_OP_NEWSLOT/_OP_NEWSLOTA` semantics: these write slots but do not overwrite
    register `a0`;
  - typed runtime-contract returns for `MenuObject`, `GetSaveList`, demo/platform checks, vector helpers,
    save-list methods, and current side-effect calls.
- Service state event report sourced from bytecode state and original call trace:
  - deterministic empty save-list/profile contract;
  - platform/demo flags for local project mode;
  - `FadeIn` and `PlayBGM` side-effect commands;
  - tracked `MenuObject` creation and UI helper method calls;
  - decoded call arguments for confirmed paths such as `FadeIn(0.7, 0)`, `PlayBGM(3)`,
    save-list `get(0)`, and several UI parent/method calls.

## Verified Title Entry Plan

Command:

```powershell
build\cmake-bt143\yuengine_cli.exe script-plan samples\touhou_new_world\project.json --repo-root .
```

Result:

```text
entry=setupProc
found=true
direct_calls=2
builtin_calls=1
native_obligations=0
object_method_calls=1
script_calls=0
ambiguous_calls=0
unresolved_calls=0
class_method_tables=8
object_bindings=1
status=vm_not_implemented
```

Direct calls:

```text
print -> builtin
init -> script_object_method, receiver: modTitle, owner class: ModuleTitle, candidate ordinal: 78
```

## Meaning

The title entry does not directly call a native start-menu API. It enters object/method dispatch:
`setupProc` reads global `modTitle` and calls `init` on that receiver. The root script builds
`ModuleTitle`, then constructs `modTitle` from that class. A correct runtime must model Squirrel
object environment and method lookup before executing this bytecode.

Current binding evidence:

```text
class tables: 8
method bindings: 64
object bindings: 1
modTitle -> ModuleTitle
ModuleTitle.init -> function #78
```

This removes the previous name-only ambiguity among `init` ordinals `15, 30, 61, 78`.

## Verified Title Execution Trace

Passive boot-frame command:

```powershell
build\cmake-bt143\yuengine_cli.exe script-run samples\touhou_new_world\project.json --repo-root . --frames 1
```

Passive boot-frame result:

```text
entry=setupProc found=true executed=true frames=1 baseline_modules=2 constructed_objects=3
script_methods=86 script_functions=5 builtin_calls=3 native_obligations=8 unique_native_apis=4
engine_object_calls=14 ui_object_calls=16 value_helper_calls=6 value_method_calls=6
bytecode_state_functions=95 bytecode_state_instructions=4129 object_field_writes=186
typed_call_returns=165 service_state_events=41 save_service_queries=4 platform_state_queries=3
audio_service_commands=1 scene_service_commands=1 ui_objects_tracked=20 ui_service_commands=0
unresolved_calls=0 truncated=false status=trace_ready_not_full_vm
```

Passive boot-frame service state:

```text
mutations=41
save.empty_save_list_queries=4
audio.current_bgm_id=3
scene.fade_in_duration=0.7
scene.fade_in_blend=0
ui.created_objects=20
ui.command_count=0
```

Passive second-frame command:

```powershell
build\cmake-bt143\yuengine_cli.exe script-run samples\touhou_new_world\project.json --repo-root . --frames 2
```

Passive second-frame result:

```text
entry=setupProc found=true executed=true frames=2 baseline_modules=2 constructed_objects=3
script_methods=89 script_functions=6 builtin_calls=3 native_obligations=8 unique_native_apis=4
engine_object_calls=15 ui_object_calls=17 value_helper_calls=6 value_method_calls=6
bytecode_state_functions=99 bytecode_state_instructions=4207 object_field_writes=191
typed_call_returns=167 service_state_events=43 save_service_queries=4 platform_state_queries=3
audio_service_commands=1 scene_service_commands=1 ui_objects_tracked=20 ui_service_commands=0
decoded_service_arguments=8 unresolved_calls=0 truncated=false status=trace_ready_not_full_vm
```

New-game scenario command:

```powershell
build\cmake-bt143\yuengine_cli.exe script-run samples\touhou_new_world\project.json --repo-root . --frames 5 --input-scenario title-new-game
```

New-game scenario result:

```text
entry=setupProc found=true executed=true frames=5 baseline_modules=2 constructed_objects=3
script_methods=101 script_functions=12 builtin_calls=3 native_obligations=18 unique_native_apis=11
engine_object_calls=18 ui_object_calls=22 value_helper_calls=6 value_method_calls=8
bytecode_state_functions=117 bytecode_state_instructions=4515 object_field_writes=203
typed_call_returns=190 service_state_events=64 save_service_queries=4 platform_state_queries=4
audio_service_commands=4 scene_service_commands=3 ui_objects_tracked=20 ui_service_commands=0
value_state_queries=14 decoded_service_arguments=21 unresolved_calls=0 truncated=false
status=trace_ready_not_full_vm
```

The original script path now reaches:

```text
ModuleTitle.main
-> modTitle._scenes[0].main
-> TitleSceneBase.main
-> TitleScene.state0
-> gMenu.isDecide(false) in passive mode
-> gMenu.isDecide(true) with --input-scenario title-new-game
-> NewGameScene.state0
-> NewGameScene.state1
-> IsSaveFull(false)
-> setMissionKey + SetDifficultyMode
-> return 200
-> ModuleTitle fadeOut
-> gMenu.startGame4Menu
-> MakeNewGame(sc01)
-> StartGame(mission:sc01/main/ms010_0, true)
-> queue_scene_stage_load(
     mission/sc01/main/ms010_0.b64.sqasm,
     map/Doujou/doujou.sge,
     map/Doujou/doujou.rcm)
```

Important semantic fixes in this checkpoint:

- `_scenes[index] = TitleScene/NewGameScene/...` writes canonical scene objects such as
  `modTitle._scenes[0]`, not disposable constructor temporaries.
- Receiver calls carry runtime receivers from `_OP_PREPCALL/_OP_CALL`, so table/object method
  dispatch uses the concrete receiver instead of static string guesses.
- Receiver-less owner calls are virtual over the concrete object class. This is why
  `TitleSceneBase.main` reaches `TitleScene.state0` and `NewGameScene.state0/state1`.
- Script method calls that affect caller control flow now execute synchronously enough to
  propagate `_OP_RETURN` values back into parent bytecode. This is required for
  `ModuleTitle.main` to consume scene `main()` returns.
- Class method lookup now prefers recovered method bindings over raw class closure slots, which
  fixes `NewGameScene.main -> TitleSceneBase.main` tailcall returns.
- `_OP_CALL` arguments are captured and passed into recovered methods/constructors. The title
  menu window records `_elemCount=4` from `selectCursorY(titleMenuCount)`.
- Root table aliases are recognized: `root.gMenu=table#1` maps to receiver `gMenu`, so
  engine-root methods such as `isDecide`, `isCancel`, `continueDisabled`, and `savesIsEmpty`
  return deterministic runtime input/profile state to bytecode.
- `--input-scenario` profiles are runtime state, not branch-result tables. `title-new-game`
  supplies `menu_selected_index=1` and `menu_decide=true`; original title bytecode performs the
  branch, state transitions, `PlaySE(2)`, `GetScenarioKeys`, `IsSaveFull(false)`,
  `SetDifficultyMode`, `fadeOut`, and `startGame4Menu`.
- Scenario selection is sourced from original project data (`resource/ak3.json` `_scenarios`),
  not from a synthetic placeholder key. The current first new-game path selects `sc01`.
- `MakeNewGame` and `StartGame` now mutate runtime-owned Save/Profile/Scenario state and queue
  the first mission script/stage/rail-camera load request instead of remaining script-only events.
- Runtime-owned UI helper methods such as `selectCursorY`, `selectCursorX`, `setFadeIn`,
  `setSelectCursor`, `move`, and `resetAnim` are intercepted as service/helper contracts instead
  of being expanded into unrelated UI implementation bytecode.

Runtime script state produced by the current original title path:

```text
root_field_count=31
object_count=26
table_count=17
class_slot_table_count=24
class_base_count=16
root.gMenu=table#1
gMenu.table_fields=87
modTitle._nextState=0
modTitle._scenes=table#15
modTitle._scenes[0]=TitleScene
modTitle._scenes[1]=NewGameScene
modTitle._scenes[2]=LoadScene
modTitle._scenes[3]=OverwriteSaveScene
script.ScrollWindow@constructor:50._elemCount=4
script.ScrollWindow@constructor:50._sel=0
```

Constructed original script objects recorded as canonical objects:

```text
modTitle -> ModuleTitle
modTitle._scenes[0] -> TitleScene
modTitle._scenes[1] -> NewGameScene
modTitle._scenes[2] -> LoadScene
modTitle._scenes[3] -> OverwriteSaveScene
```

Native/API obligations reached through the passive script execution trace:

```text
FadeIn -> Scene And Stage Service
PlayBGM -> Audio Service
GetSaveList -> Save/Profile/Scenario Service
IsFreeDemo -> Platform Service
```

Additional native/API obligations reached by `--input-scenario title-new-game`:

```text
PlaySE
GetScenarioKeys
GetCountActiveDLC
IsSaveFull
SetDifficultyMode
```

`ModuleTitle.main -> stateInit` is now real inherited script execution:

```text
modTitle class: ModuleTitle
ModuleTitle base: ModuleBase
stateInit binding module: script/menu/menudef.b64
stateInit function ordinal: 221
event category: script_inherited_owner_method
post-state: modTitle._nextState=0
module_lifecycle_calls=0
```

The save/profile and platform calls in the passive boot frame now come from original scene/menu
initialization contracts, not from linear scanning of unselected title menu branches. Branch-only
calls such as `PlaySE(2)`, `GetScenarioKeys`, `IsSaveFull`, `SetDifficultyMode`, `fadeOut`, and
`startGame4Menu` appear when `--input-scenario title-new-game` drives the original bytecode there.
The current `startGame4Menu` path also records `MakeNewGame(sc01)`, `StartGame`, and a
Scene/Stage queue for `mission/sc01/main/ms010_0.b64.sqasm`,
`map/Doujou/doujou.sge`, and `map/Doujou/doujou.rcm`.

## Verification

```powershell
ctest --test-dir build\cmake-bt143 --output-on-failure
python -m unittest discover -s tests
```

Verified result:

```text
CTest: 16/16 passed
Python unittest: 6/6 passed
```

## Remaining L7 Work

- Extend deterministic runtime input scenarios beyond `title-new-game`:
  Continue enabled/disabled, Load empty/non-empty, Option, Exit denied/allowed, cursor up/down.
- Promote the current `MakeNewGame` and `StartGame` runtime mutations into fuller service
  contracts: save slot/profile payload shape, DLC/scenario gate behavior, failure paths, and
  transition ownership.
- Convert current classified categories and typed placeholders into concrete service behavior:
  - 17 engine object calls in the two-frame passive trace;
  - 17 runtime-owned UI helper object calls;
  - 6 value helper calls;
  - 6 value methods;
  - 0 Module lifecycle hooks, because `ModuleBase.stateInit` is now executed as inherited bytecode.
- Extend concrete UI helper object layouts into decoded UI command payloads, especially
  `MenuObject`, `_menuWindow`, `_listWindow`, `setSelectCursor`, `bl`, `tr`, and
  `renderHorizontal`.
- Finish decoding argument payloads for `MenuObject`, `_menuWindow`, `_listWindow`,
  `setSelectCursor`, `bl`, `tr`, and `renderHorizontal` into real UI command data.
- Expand typed service behavior for `FadeIn`, `PlayBGM`, `PlaySE`, `MenuObject`, `GetSaveList`,
  `GetScenarioKeys`, `IsSaveFull`, `SetDifficultyMode`, `MakeNewGame`, and `StartGame` from
  current runtime-owned behavior into complete original-compatible contracts.
- The queued first mission script now has its own completed L8 checkpoint in
  `docs/LOOP_TASKS.md` and `docs/SCENE_ENTRY_RUNTIME_STATUS.md`:
  `mission/sc01/main/ms010_0.b64.sqasm -> setupProcess -> LoadStage ->
  LoadEventsScriptViaMission -> CallSetupEvents -> PushPlayerChara -> LoadRailCamera`
  executes with 0 unresolved calls and produces runtime-owned stage/player/camera/checkpoint
  state. `yuengine_cli scene-entry` now binds the title new-game transition and first mission
  setup into one runtime contract with `stage_ready=true`, `actor_ready=true`,
  `camera_ready=true`, `event_ready=true`, `missing_resources=0`, and `missing_scripts=0`.
  The remaining work is to materialize those bindings into scene graph, actor task, camera task,
  event marker, renderer, audio, and input subsystems.
- L9 now has its own completed checkpoint in `docs/SCENE_RUNTIME_MATERIALIZATION_STATUS.md`:
  `yuengine_cli scene-runtime` consumes scene-entry and emits ready stage/actor/camera/event
  handles with 42 stage dependencies, 111 model mesh candidates, 150 collision triangles,
  3 rail-node candidates, and 0 missing stage dependencies. The remaining title-lane work is to
  keep broadening original menu scenarios while L10 consumes those handles into renderer/input
  frame contracts.
- L10 now has its own completed checkpoint in `docs/FIRST_FRAME_RUNTIME_STATUS.md`:
  `yuengine_cli first-frame` consumes scene-runtime and emits ready renderer/actor/camera/input/
  event first-frame contracts with 111 mesh draw candidates, 39 texture bindings, 150 collision
  triangles, 1 actor instance, 3 rail nodes, and 1 event marker. The remaining work is first
  mission event thread/tutorial/player-control behavior plus broader title menu scenarios.
- UI/render command buffer sourced from original script calls, not handwritten UI.

L7 is not complete until original title script execution can drive menu selection, save/new-game
transition, and service state through original bytecode with value semantics sufficient to feed
the next scene-loading loop.
