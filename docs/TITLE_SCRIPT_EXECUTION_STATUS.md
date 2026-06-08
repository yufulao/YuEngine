# L7 Title Script Execution Status

Status: active. Title entry bytecode-state checkpoint completed on 2026-06-09.

This document tracks L7. The current implementation executes original `.sqasm` with a scoped
multi-module bytecode state pass for the title boot path. It is now PC-branch aware for
`_OP_JZ/_OP_JNZ/_OP_JMP`, handles `_OP_FOREACH/_OP_POSTFOREACH` table iteration for the title
scene list, and no longer relies on a hard stop inside `ModuleTitle.main`. It is still not a full
Squirrel VM, title UI, or gameplay runtime.

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
  - Module lifecycle hooks.
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

Command:

```powershell
build\cmake-bt143\yuengine_cli.exe script-run samples\touhou_new_world\project.json --repo-root . --frames 1
```

Result:

```text
entry=setupProc
found=true
executed=true
frames=1
baseline_modules=2
constructed_objects=5
script_methods=29
script_functions=5
builtin_calls=3
native_obligations=10
native_implemented_calls=0
unique_native_apis=5
engine_object_calls=12
ui_object_calls=21
value_helper_calls=8
value_method_calls=8
module_lifecycle_calls=1
bytecode_state_functions=38
bytecode_state_instructions=3627
root_slot_writes=31
class_slot_writes=405
object_field_writes=121
table_slot_writes=198
typed_call_returns=144
ui_object_mutations=0
service_state_events=57
save_service_queries=4
platform_state_queries=2
audio_service_commands=1
scene_service_commands=1
ui_objects_tracked=20
ui_service_commands=24
value_state_queries=4
decoded_service_arguments=2
optional_unbound_globals=0
unresolved_calls=0
truncated=false
status=trace_ready_not_full_vm
```

`unresolved_calls=0`, `control_flow_unknown=0`, and `truncated=false` mean every call in the
current trace has a runtime category, and the title boot path no longer depends on linear
branch scanning. The bytecode-state counters prove the runtime is now carrying original script
state through the boot edge with the preload/menudef/title module baseline. Service calls now also
mutate a runtime-owned service state snapshot instead of existing only as report events.
`FadeIn` is now decoded through inherited `ModuleBase._fadeInTime` from `menudef` as
`duration=0.7; blend=0`. `IsOverDemo` is no longer counted for this path because branch-aware
execution does not enter the later unselected branch that previously polluted the trace. Real UI
command geometry/text and later state transitions are still not complete.

Runtime service state produced by the same original title boot path:

```text
mutations=57
save.empty_save_list_queries=4
save.save_list_count_queries=4
save.save_list_entries=0
platform.flags.IsFreeDemo=false
audio.current_bgm_id=3
scene.fade_in_duration=0.7
scene.fade_in_blend=0
ui.created_objects=20
ui.command_count=24
```

Runtime script state produced by the same original title boot path:

```text
root_field_count=31
object_count=28
table_count=17
class_slot_table_count=24
class_base_count=16
root.gMenu=table#1
root.modTitle=object:modTitle
gMenu.table_fields=87
modTitle._nextState=300
modTitle._scenes=table#15
modTitle._scenes[0]=TitleScene
modTitle._scenes[1]=NewGameScene
modTitle._scenes[2]=LoadScene
modTitle._scenes[3]=OverwriteSaveScene
```

Constructed original script objects:

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
MenuObject -> UI And 2D Render Service
GetSaveList -> Save/Profile/Scenario Service
IsFreeDemo -> Platform Service
```

`gMenu.continueDisabled` and `gMenu.savesIsEmpty` are now resolved as root object script
closures before their native/API calls are dispatched.

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

- Advance `ModuleTitle.main` from the boot state into later real state transitions
  (`_nextState == 200/100/scene dispatch`) without reintroducing linear branch pollution.
- Convert current classified categories and typed placeholders into concrete service behavior:
  - 12 engine object calls;
  - 21 UI helper object calls;
  - 8 value helper calls;
  - 8 value methods;
  - 1 Module lifecycle hook.
- Consume runtime-owned `gMenu` table state and canonical `modTitle`/title scene objects in the
  next `ModuleTitle.main` state transition.
- Extend concrete UI helper object layouts into decoded UI command payloads.
- UI command buffer for helper objects such as `_menuWindow`, `_listWindow`, and `MenuObject`.
- Finish decoding argument payloads for `MenuObject`, `_menuWindow`, `_listWindow`,
  `setSelectCursor`, `bl`, `tr`, and `renderHorizontal` into real UI command data.
- Expand typed service behavior for `GetSaveList`, `IsFreeDemo`, `FadeIn`, `PlayBGM`,
  and `MenuObject` from reported contracts into runtime-owned services.
- Save/new-game transition through `MakeNewGame` and `StartGame`.
- UI/render command buffer sourced from original script calls, not handwritten UI.

L7 is not complete until original title script execution reaches native/UI obligations through
the runtime path with value semantics sufficient to drive the real menu state.
