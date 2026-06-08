# L7 Title Script Execution Status

Status: active. Title entry execution-trace bridge checkpoint completed on 2026-06-09.

This document tracks L7. The current implementation executes a branch-aware boot-edge call trace
from original `.sqasm`, but it is not a full Squirrel value VM yet.

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
engine_object_calls=28
optional_unbound_globals=4
unresolved_calls=28
truncated=false
status=trace_ready_not_full_vm
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
CTest: 10/10 passed
Python unittest: 6/6 passed
```

## Remaining L7 Work

- Full Squirrel value/register/table VM semantics for the executed opcode subset.
- Branch/value correctness for `ModuleTitle.main` beyond the `_nextState == 300` boot edge.
- Object/value model for UI helper objects such as `_menuWindow`, `_listWindow`, and `MenuObject`.
- Typed behavior for `GetSaveList`, `IsFreeDemo`, `IsOverDemo`, `FadeIn`, `PlayBGM`, and `MenuObject`.
- Save/new-game transition through `MakeNewGame` and `StartGame`.
- UI/render command buffer sourced from original script calls, not handwritten UI.

L7 is not complete until original title script execution reaches native/UI obligations through
the runtime path with value semantics sufficient to drive the real menu state.
