# L7 Title Script Execution Status

Status: active. Title entry bytecode-state checkpoint completed on 2026-06-09.

This document tracks L7. The current implementation executes a branch-aware boot-edge call trace
from original `.sqasm` and now runs a scoped bytecode state pass for the title boot edge. It is
still not a full Squirrel VM, title UI, or gameplay runtime.

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
  - root/class/object/table slot writes;
  - class default materialization onto constructed script objects;
  - typed placeholder returns for `MenuObject`, `GetSaveList`, demo/platform checks, vector helpers,
    save-list methods, and current side-effect calls.

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
constructed_objects=5
script_methods=29
script_functions=5
builtin_calls=1
native_obligations=12
native_implemented_calls=0
unique_native_apis=6
engine_object_calls=12
ui_object_calls=21
value_helper_calls=8
value_method_calls=16
module_lifecycle_calls=1
bytecode_state_functions=36
bytecode_state_instructions=1859
root_slot_writes=4
class_slot_writes=32
object_field_writes=116
table_slot_writes=4
typed_call_returns=131
ui_object_mutations=19
optional_unbound_globals=4
unresolved_calls=0
truncated=false
status=trace_ready_not_full_vm
```

`unresolved_calls=0` means every call in the current trace has a runtime category. The new
bytecode-state counters prove the runtime is now carrying original script state through the boot
edge, but concrete native side effects, UI command output, and later branch correctness are still
not complete.

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
IsOverDemo -> Platform Service
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

- Branch/value correctness for `ModuleTitle.main` beyond the `_nextState == 300` boot edge.
- Convert current classified categories and typed placeholders into concrete service behavior:
  - 12 engine object calls;
  - 21 UI helper object calls;
  - 8 value helper calls;
  - 16 value methods;
  - 1 Module lifecycle hook.
- UI command buffer for helper objects such as `_menuWindow`, `_listWindow`, and `MenuObject`.
- Typed service behavior for `GetSaveList`, `IsFreeDemo`, `IsOverDemo`, `FadeIn`, `PlayBGM`, and
  `MenuObject`.
- Save/new-game transition through `MakeNewGame` and `StartGame`.
- UI/render command buffer sourced from original script calls, not handwritten UI.

L7 is not complete until original title script execution reaches native/UI obligations through
the runtime path with value semantics sufficient to drive the real menu state.
