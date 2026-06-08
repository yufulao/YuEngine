# Runtime Spine Status

Lane: `project-runtime`

This document tracks L1 C++ Runtime Spine.

## Implemented

- CMake C++20 project.
- `yuengine_runtime` library.
- `yuengine_cli` executable.
- Core JSON parser.
- Project manifest loader.
- VFS with loose mounts and pack-manifest resource index.
- `.sqasm` module diagnostics loader.
- Native/API obligation registry loaded from the P4 boundary markdown.
- Runtime boot report for sample projects.
- CTest coverage for manifest validation and diagnostic boot of both sample projects.

## Verified On 2026-06-09

- Build environment: VS BuildTools 2022 MSVC 19.43.34809, Ninja.
- Original sample diagnostic boot:
  - `ok`: true
  - loose mounts: 2
  - pack-manifest resources: 13,028
  - native registry APIs: 84
  - loaded modules: `preload.b64.sqasm`, `script/menu/titlemenu.b64.sqasm`
  - script stats: 82 functions, 4,476 instructions, 596 calls
  - native/API obligations: 36 APIs from title/preload scope, all `not_started`
- Empty sample diagnostic boot:
  - `ok`: true
  - loose mounts: 1
  - native registry APIs: 84
  - loaded modules: `script/main.b64.sqasm`
  - native/API obligations: 0

## Verification Commands

Use `--clean-first` with the current Ninja/MSVC build on this machine. CMake's localized
`/showIncludes` dependency prefix is mis-encoded, so header changes can otherwise leave stale
object files.

```powershell
cmd.exe /d /s /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -vcvars_ver=14.43 && "D:\app\CMake\bin\cmake.exe" -S . -B build\cmake-bt143 -G Ninja -DCMAKE_MAKE_PROGRAM="D:/opt/Python311/Scripts/ninja.exe" && "D:\app\CMake\bin\cmake.exe" --build build\cmake-bt143 --clean-first'
build\cmake-bt143\yuengine_cli.exe validate samples\touhou_new_world\project.json
build\cmake-bt143\yuengine_cli.exe boot samples\touhou_new_world\project.json --repo-root .
build\cmake-bt143\yuengine_cli.exe boot samples\empty_project\project.json --repo-root .
ctest --test-dir build\cmake-bt143 --output-on-failure
python -m unittest discover -s tests
```

## Boundary

This is runtime spine, not game execution.

The original title script is loaded as `.sqasm` diagnostics, not executed in a Squirrel VM yet.
Native/API calls are reported as obligations and remain `not_started`.
