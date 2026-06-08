# L4 Native Service Interfaces Status

Status: completed as C++ native service contract baseline on 2026-06-09.

This loop connects the P4/P6 API surface to C++ runtime service interfaces. It does not
implement native behavior yet. Every `not_started` API remains a tracked obligation.

## Implemented

- `yu::native::NativeRegistry` now exposes API/service counts, call counts, evidence and
  implementation status queries.
- `yu::native::NativeServiceCatalog` registers the 11 service interfaces used by the
  title/new-game/first-mission surface.
- Runtime boot dispatches known native/API calls through the service catalog instead of only
  reading the markdown table.
- Unbound native APIs fail boot diagnostics instead of silently disappearing.
- `yuengine_cli native-services` reports service/interface binding status.

Service interfaces:

```text
Actor And Task Service
Audio Service
Camera Service
Collision And Physics-Lite Service
Event/Quest/Flag Service
Platform Service
Resource Service
Save/Profile/Scenario Service
Scene And Stage Service
Script Service
UI And 2D Render Service
```

## Verified Surface

```text
services: 11
native_apis: 84
unowned_apis: 0
unbound_apis: 0
not_started: 84
```

Original sample diagnostic boot still reports:

```text
ok: true
native_apis: 84
title/preload native obligations: 36
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe native-services samples\touhou_new_world\project.json --repo-root .
build\cmake-bt143\yuengine_cli.exe boot samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 --output-on-failure
python -m unittest discover -s tests
```

Verified result:

```text
CTest: 9/9 passed
Python unittest: 6/6 passed
```

## Boundary

This loop proves ownership and service binding for the current script-visible native/API
surface. It does not confirm argument shapes, return shapes, side effects, or oracle behavior.

Next loop is L5 Service-Backed Runtime Lifecycle: move boot phases into an explicit service
container and make project/VFS/script/native ownership part of runtime state.
