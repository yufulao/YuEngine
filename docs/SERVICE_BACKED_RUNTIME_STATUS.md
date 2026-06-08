# L5 Service-Backed Runtime Lifecycle Status

Status: completed as service-backed diagnostic boot lifecycle on 2026-06-09.

This loop moves boot state out of local variables and into a runtime context/service container.
It is still diagnostic boot, not Squirrel VM execution or gameplay.

## Implemented

- `yu::runtime::RuntimeContext`
- `yu::runtime::ServiceContainer`
- Runtime-owned Project Manifest service state.
- Runtime-owned VFS service state.
- Runtime-owned Native Registry state.
- Runtime-owned Native Service Catalog state.
- Deterministic boot phases in the boot report.
- Script module loading through the runtime service container's script roots.
- Native/API obligations emitted through the native service catalog.

Boot phases:

```text
load_project_manifest
mount_vfs
verify_required_resources
load_native_registry
bind_native_services
load_startup_scripts
collect_native_obligations
```

## Verified Boot Metrics

Original sample:

```text
ok=true
phases=7
failed_phases=0
native_apis=84
native_services=11
obligations=36
```

Empty sample:

```text
ok=true
phases=7
failed_phases=0
native_apis=84
native_services=11
obligations=0
```

Original sample phase details:

```text
load_project_manifest: ok, touhou_new_world_sample
mount_vfs: ok, loose=2 pack=13028
verify_required_resources: ok, required=5 missing=0
load_native_registry: ok, apis=84
bind_native_services: ok, services=11 unbound=0
load_startup_scripts: ok, modules=2
collect_native_obligations: ok, apis=36
```

## Verification

```powershell
cmd.exe /d /s /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -vcvars_ver=14.43 && "D:\app\CMake\bin\cmake.exe" --build build\cmake-bt143 --clean-first'
build\cmake-bt143\yuengine_cli.exe boot samples\touhou_new_world\project.json --repo-root .
build\cmake-bt143\yuengine_cli.exe boot samples\empty_project\project.json --repo-root .
ctest --test-dir build\cmake-bt143 --output-on-failure
python -m unittest discover -s tests
```

Verified result:

```text
CTest: 9/9 passed
Python unittest: 6/6 passed
```

## Boundary

This loop establishes runtime ownership and phase diagnostics. It does not execute original
Squirrel bytecode, implement native side effects, render title UI, or enter scenes.

Next loop is L6 Oracle Capture Execution: collect or prepare original-game title boot evidence
so later VM/service behavior is checked against observed runtime behavior instead of guessed.
