# L7 Title Script Execution Status

Status: active. First entry-plan slice completed on 2026-06-09.

This document tracks L7. The current implementation does not execute Squirrel bytecode yet.

## Implemented So Far

- `yu::script::ScriptRuntime`
- `yuengine_cli script-plan`
- Entry execution planning from `project.json` startup fields.
- Direct call resolution for the title entry function:
  - native/API obligation;
  - unique script function;
  - ambiguous script function;
  - unresolved or builtin.

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
native_obligations=0
script_calls=0
ambiguous_calls=1
unresolved_calls=1
status=vm_not_implemented
```

Direct calls:

```text
print -> unresolved_or_builtin
init -> ambiguous_script_function, candidate ordinals: 15, 30, 61, 78
```

## Meaning

The title entry does not directly call a native start-menu API. It enters object/method dispatch:
`setupProc` calls `init`, but `init` is not globally unique in the module. A correct runtime must
model Squirrel object environment and method lookup. Hard-binding this to one `init` would repeat
the old Project failure.

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
- Builtin handling for calls like `print`.
- Object environment/method lookup for ambiguous script function names.
- Native dispatch during script execution through `NativeServiceCatalog`.
- UI/render command buffer sourced from original script calls, not handwritten UI.

L7 is not complete until original title script execution reaches native/UI obligations through
the runtime path.
