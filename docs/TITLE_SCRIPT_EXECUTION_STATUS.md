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
  - typed placeholder returns for `MenuObject`, `GetSaveList`, demo/platform checks, vector helpers,
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

Boot-frame command:

```powershell
build\cmake-bt143\yuengine_cli.exe script-run samples\touhou_new_world\project.json --repo-root . --frames 1
```

Boot-frame result:

```text
entry=setupProc found=true executed=true frames=1 baseline_modules=2 constructed_objects=3
script_methods=58 script_functions=1 builtin_calls=3 native_obligations=2 unique_native_apis=2
engine_object_calls=14 ui_object_calls=3 value_helper_calls=6 value_method_calls=2
bytecode_state_functions=63 bytecode_state_instructions=3906 object_field_writes=159
typed_call_returns=147 service_state_events=29 save_service_queries=0 platform_state_queries=0
audio_service_commands=1 scene_service_commands=1 ui_objects_tracked=20 ui_service_commands=6
unresolved_calls=0 truncated=false status=trace_ready_not_full_vm
```

Boot-frame service state:

```text
mutations=29
save.empty_save_list_queries=0
audio.current_bgm_id=3
scene.fade_in_duration=0.7
scene.fade_in_blend=0
ui.created_objects=20
ui.command_count=6
```

Second-frame title-scene command:

```powershell
build\cmake-bt143\yuengine_cli.exe script-run samples\touhou_new_world\project.json --repo-root . --frames 2
```

Second-frame result:

```text
entry=setupProc found=true executed=true frames=2 baseline_modules=2 constructed_objects=3
script_methods=62 script_functions=2 builtin_calls=3 native_obligations=2 unique_native_apis=2
engine_object_calls=15 ui_object_calls=3 value_helper_calls=6 value_method_calls=2
bytecode_state_functions=68 bytecode_state_instructions=3992 object_field_writes=164
typed_call_returns=151 service_state_events=31 save_service_queries=0 platform_state_queries=0
audio_service_commands=1 scene_service_commands=1 ui_objects_tracked=20 ui_service_commands=6
decoded_service_arguments=4 unresolved_calls=0 truncated=false status=trace_ready_not_full_vm
```

The second frame now dispatches through the original object graph:

```text
ModuleTitle.main
-> modTitle._scenes[0].main
-> TitleSceneBase.main
-> TitleScene.state0
-> script.ScrollWindow@constructor:50.selectCursorY(titleMenuCount)
-> gMenu.isDecide(false)
```

Important semantic fixes in this checkpoint:

- `_scenes[index] = TitleScene/NewGameScene/...` writes canonical scene objects such as
  `modTitle._scenes[0]`, not disposable constructor temporaries.
- Receiver calls carry runtime receivers from `_OP_PREPCALL/_OP_CALL`, so table/object method
  dispatch uses the concrete receiver instead of static string guesses.
- Receiver-less owner calls are virtual over the concrete object class. This is why
  `TitleSceneBase.main` now reaches `TitleScene.state0` instead of the base placeholder `state0`.
- `_OP_CALL` arguments are captured and passed into recovered methods/constructors. The title
  menu window now records `_elemCount=5` from `selectCursorY(titleMenuCount)`.
- Root table aliases are recognized: `root.gMenu=table#1` still maps to receiver `gMenu`, so
  engine-root methods such as `isDecide` can return deterministic input state to bytecode.
- Passive title frame input is deterministic: `gMenu.isDecide(false)` jumps to the idle path, so
  the incorrect `TitleScene.state0 -> PlaySE(3)` fallthrough is gone.

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
script.ScrollWindow@constructor:50._elemCount=5
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

Native/API obligations reached through the script execution trace:

```text
FadeIn -> Scene And Stage Service
PlayBGM -> Audio Service
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

The save/profile and platform calls are no longer counted in the passive title path because the
runtime no longer linearly scans unselected branches. They must reappear only when input/service
state selects Continue/Load/New Game through original menu logic.

## Verification

```powershell
ctest --test-dir build\cmake-bt143 --output-on-failure
python -m unittest discover -s tests
```

Verified result:

```text
CTest: 11/11 passed
Python unittest: 6/6 passed
```

## Remaining L7 Work

- Add deterministic runtime input scenarios for `gMenu`:
  passive frame, cursor movement, decide on Continue/New Game/Load/Option/Exit.
- Drive `TitleScene.state0` through original selected branches without reintroducing linear branch
  pollution. `GetSaveList`, `IsFreeDemo`, `MakeNewGame`, and `StartGame` must appear only when
  the selected branch reaches them.
- Convert current classified categories and typed placeholders into concrete service behavior:
  - 15 engine object calls in the two-frame passive trace;
  - 3 UI helper object calls;
  - 6 value helper calls;
  - 2 value methods;
  - 0 Module lifecycle hooks, because `ModuleBase.stateInit` is now executed as inherited bytecode.
- Extend concrete UI helper object layouts into decoded UI command payloads, especially
  `MenuObject`, `_menuWindow`, `_listWindow`, `setSelectCursor`, `bl`, `tr`, and
  `renderHorizontal`.
- Finish decoding argument payloads for `MenuObject`, `_menuWindow`, `_listWindow`,
  `setSelectCursor`, `bl`, `tr`, and `renderHorizontal` into real UI command data.
- Expand typed service behavior for `FadeIn`, `PlayBGM`, `MenuObject`, `GetSaveList`,
  `IsFreeDemo`, `MakeNewGame`, and `StartGame` from reported contracts into runtime-owned
  services.
- Save/new-game transition through `MakeNewGame` and `StartGame`.
- UI/render command buffer sourced from original script calls, not handwritten UI.

L7 is not complete until original title script execution can drive menu selection, save/new-game
transition, and service state through original bytecode with value semantics sufficient to feed
the next scene-loading loop.
