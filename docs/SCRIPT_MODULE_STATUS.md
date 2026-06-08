# L3 Script Module Model Status

Status: completed as C++ script module diagnostics on 2026-06-09.

This loop builds the C++ `.sqasm` module model that later VM and Native Service work must use.
It is not Squirrel bytecode execution yet, and it is not title/menu gameplay.

## Runtime Contract

`yu::script::SqasmModule` now preserves:

- module path and original input name;
- functions with ordinal, name, source, offset, stack, generator, varargs, parameters;
- literals, locals, instructions, instruction args, branch targets;
- callsites from `_OP_PREPCALL*`;
- closure bindings from `_OP_CLOSURE`;
- resource-like and script-dependency literals;
- state-machine candidates from title/menu naming patterns.

The CLI entry is:

```powershell
build\cmake-bt143\yuengine_cli.exe script samples\touhou_new_world\project.json script/menu/titlemenu.b64
build\cmake-bt143\yuengine_cli.exe script samples\touhou_new_world\project.json mission/sc01/main/ms010_0.b64
```

## Verified Modules

Title menu:

```text
functions: 81
instructions: 4466
calls: 593
unique_calls: 146
resource_refs: 8
closure_bindings: 80
state_candidate_count: 26
```

First mission candidate:

```text
functions: 62
instructions: 4068
calls: 640
unique_calls: 123
resource_refs: 7
closure_bindings: 61
state_candidate_count: 0
```

First mission resource-like refs include the stage/rail-camera files and script dependency stems:

```text
demos/Heal.png
demos/OP_demo.png
demos/guardBreak.png
map/Doujou/doujou.rcm
map/Doujou/doujou.sge
sc01/main/ms010_0
sc01/main/ms010_1
```

The parser rejects escaped dialogue text such as `\n` literals as resource paths. Resource
virtual paths are treated as script byte strings and do not use `std::filesystem::path` for
extension detection, avoiding Windows codepage failures on non-ASCII script literals.

## Verification

```powershell
cmd.exe /d /s /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -vcvars_ver=14.43 && "D:\app\CMake\bin\cmake.exe" -S . -B build\cmake-bt143 -G Ninja -DCMAKE_MAKE_PROGRAM="D:/opt/Python311/Scripts/ninja.exe" && "D:\app\CMake\bin\cmake.exe" --build build\cmake-bt143 --clean-first && "D:\app\CMake\bin\ctest.exe" --test-dir build\cmake-bt143 --output-on-failure'
python -m unittest discover -s tests
```

Verified result:

```text
CTest: 8/8 passed
Python unittest: 6/6 passed
```

## Boundary

This loop gives the engine a stable script module model. It does not execute Squirrel bytecode,
does not implement native API side effects, and does not prove R2 title menu behavior.

Next loop is L4 Native Service Interfaces: map the script-visible APIs to typed C++ service
interfaces and make unimplemented calls explicit obligations rather than silent no-ops.
