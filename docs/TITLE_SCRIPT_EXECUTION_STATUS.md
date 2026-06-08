# L7 Title Script Execution Status

Status: active. Entry object/method binding checkpoint completed on 2026-06-09.

This document tracks L7. The current implementation does not execute Squirrel bytecode yet.

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

- Squirrel VM plan/embedding or bytecode execution bridge.
- Runtime execution of `setupProc` through `print` builtin and `modTitle.init`.
- VM support for the opcode subset used by `ModuleTitle.constructor`, `ModuleTitle.init`,
  `ModuleTitle.main`, and scene method dispatch.
- Native dispatch during script execution through `NativeServiceCatalog`.
- UI/render command buffer sourced from original script calls, not handwritten UI.

L7 is not complete until original title script execution reaches native/UI obligations through
the runtime path.
