# Autonomous Loop Tasks

This file is the queue for long-running agents. Work top to bottom. When a task is completed,
commit it and continue to the next task without waiting for a new prompt.

## Execution Doctrine

Each loop closes one engine layer contract. It is not a minimal demo, a temporary entrypoint,
or a visual placeholder. A loop is only complete when it has:

- a named runtime/service contract;
- evidence inputs from scripts/resources/native/API/oracle where available;
- implementation in the C++ runtime spine or an explicitly staged tool feeding that spine;
- diagnostics or tests that fail on the important regression;
- documentation of remaining unknowns;
- a git commit.

After a loop is committed, continue to the next non-blocked loop. Do not stop at CLI statistics,
resource previews, mesh previews, handwritten UI, or blue-screen runtime output. Those can be
intermediate diagnostics only when they are connected to the next runtime layer.

The current long target remains:

```text
project.json
-> VFS/resource system
-> script module model
-> script VM/native service registry
-> script-driven title menu
-> save/new-game/profile flow
-> scene/stage load
-> actor/player/camera/input/tutorial flow
-> first playable sample flow
-> generic reusable engine runtime
```

## Completed Loops

### L1: C++ Runtime Spine

Status: completed as diagnostic runtime spine on 2026-06-09.

Deliver:

- `CMakeLists.txt`
- `src/yuengine/...`
- `apps/yuengine_cli`
- runtime boot diagnostics for both sample projects

Acceptance:

- CMake configure/build passes.
- `yuengine_cli validate samples/touhou_new_world/project.json` passes.
- `yuengine_cli boot samples/touhou_new_world/project.json` loads project, mounts VFS, loads
  preload and title `.sqasm`, and reports native/API obligations.
- `yuengine_cli boot samples/empty_project/project.json` boots without original resources.

Verified:

- CMake BuildTools 14.43/Ninja build passes.
- CTest passes 4/4.
- Python unittest suite passes 6/6.
- Original sample diagnostic boot reports:
  - loose mounts: 2
  - pack resources: 13,028
  - native registry APIs: 84
  - loaded modules: preload and titlemenu
  - title/preload obligations: 36, all `not_started`
- Empty sample diagnostic boot reports:
  - loose mounts: 1
  - native registry APIs: 84
  - native obligations: 0

Boundary: this is diagnostic boot only. It is not script execution or gameplay.

### L2: VFS And Pack Manifest Depth

Status: completed as resource dependency diagnostics on 2026-06-09.

Deliver:

- resource registry with normalized paths;
- pack-manifest indexed resources;
- loose resource lookup;
- stem expansion;
- title and first mission resource dependency diagnostics.

Acceptance:

- title background/logo/DLC stems resolve to language variants.
- `map/doujou/doujou.sge` and `map/doujou/doujou.rcm` resolve through project mounts.

Verified:

- `yuengine_cli resources samples/touhou_new_world/project.json` passes.
- `yuengine_cli resources samples/empty_project/project.json` passes.
- CTest passes 6/6 with resources tests included.
- Original sample required resources have 0 missing.
- Title background/logo/DLC stems resolve to pack and merged loose variants.
- Title script resource refs: 8, unresolved: 0.
- First mission script resource refs: 3, unresolved: 0.
- `map/doujou/doujou.sge` and `map/doujou/doujou.rcm` resolve as pack resources.
- Resource literal pollution from `/`, date placeholders, and script keys is filtered.

Boundary: this is VFS/dependency diagnostics only. It is not asset payload parsing or rendering.

### L3: Script Module Model

Status: completed as C++ script module diagnostics on 2026-06-09.

Deliver:

- `.sqasm` parser in C++ preserves functions, literals, instructions, callsites, resource refs;
- module dependency/call summary;
- obligation extraction by service owner.

Acceptance:

- title module reports 81 functions, 4466 instructions, 593 calls.
- first mission module reports 62 functions, 4068 instructions, 640 calls.

Verified:

- `yuengine_cli script samples/touhou_new_world/project.json script/menu/titlemenu.b64` reports
  81 functions, 4,466 instructions, 593 calls, 146 unique calls, 8 resource refs, 80 closure bindings.
- `yuengine_cli script samples/touhou_new_world/project.json mission/sc01/main/ms010_0.b64` reports
  62 functions, 4,068 instructions, 640 calls, 123 unique calls, 7 resource refs, 61 closure bindings.
- CTest passes 8/8 with script tests included.
- Python unittest suite passes 6/6.

Boundary: this is script module diagnostics only. It is not Squirrel VM execution or native side effects.

### L4: Native Service Interfaces

Status: completed as C++ native service contract baseline on 2026-06-09.

Deliver:

- C++ service interfaces for the 11 P6 services;
- native registry maps API name -> service -> implementation state;
- unimplemented calls raise tracked obligations.

Acceptance:

- 84 title/first-mission APIs are present in registry.
- no call used by current boot scope is unowned.

Verified:

- `yuengine_cli native-services samples/touhou_new_world/project.json --repo-root .` reports
  11 services, 84 native APIs, 0 unowned APIs, 0 unbound APIs, 84 `not_started` obligations.
- Original sample diagnostic boot remains `ok: true` and still reports 36 title/preload
  native obligations through the service catalog.
- CTest passes 9/9.
- Python unittest suite passes 6/6.

Boundary: this is service ownership and obligation dispatch only. It is not native behavior
implementation, argument/return confirmation, or oracle-diff acceptance.

### L5: Service-Backed Runtime Lifecycle

Status: completed as service-backed diagnostic boot lifecycle on 2026-06-09.

Deliver:

- service container for core/project/VFS/script/native layers;
- deterministic boot phases with explicit diagnostics;
- preload and entry-module ownership by Script Service;
- native/API obligation emission through Native Service interfaces.

Acceptance:

- original and empty sample still pass boot diagnostics.
- no script-visible native/API call can disappear as a silent no-op.
- R1 remains diagnostic boot until original scripts execute; do not mark R2 here.

Verified:

- Boot report now has deterministic phases:
  `load_project_manifest`, `mount_vfs`, `verify_required_resources`, `load_native_registry`,
  `bind_native_services`, `load_startup_scripts`, `collect_native_obligations`.
- Original sample reports `ok=true phases=7 failed_phases=0 native_apis=84 native_services=11 obligations=36`.
- Empty sample reports `ok=true phases=7 failed_phases=0 native_apis=84 native_services=11 obligations=0`.
- CTest passes 9/9.
- Python unittest suite passes 6/6.

Boundary: this is service-backed diagnostic boot only. It is not script VM execution, title UI,
native behavior implementation, or gameplay.

### L6: Oracle Capture Execution

Status: completed as oracle capture readiness on 2026-06-09. P1 title boot sampling remains incomplete.

Deliver:

- actual title boot captures if capture tools or user-driven run are available;
- three-run stability report.

Acceptance:

- title boot trace maps to `titlemenu.b64`.
- save list/resource/audio/render observations are recorded or explicitly blocked.

Verified:

- `oracle_title_boot.py readiness` reports `can_launch_original_exe=true` and `safe_to_bypass_platform=false`.
- Current blockers are explicit: no file-IO capture tool, no D3D/render capture tool, and no
  user-driven title boot run recorded in this workspace.
- `inspect`, `readiness`, `snapshot`, and `runbook` commands complete.
- Python unittest suite passes 6/6.

Boundary: P1 is not complete. No original-game process was launched and no title frame/file-IO/
render/audio trace was sampled in this loop.

## Active Loop

### L7: Title Script Execution

Status: active. Entry execution plan slice completed on 2026-06-09.

Deliver:

- Squirrel VM plan/embedding or bytecode execution bridge;
- title script dispatch through native service registry;
- UI/render command buffer begins from script calls.

Acceptance:

- no hand-written replacement menu.
- every missing native call is a named obligation.

Progress:

- `yuengine_cli script-plan` resolves the manifest startup entry.
- Title entry `setupProc` is found with 2 direct calls.
- `print` is currently `unresolved_or_builtin`.
- `init` is `ambiguous_script_function` with candidate ordinals `15, 30, 61, 78`.
- CTest passes 10/10 with the title entry plan test included.

Boundary: L7 is not complete. The current slice is a VM entry plan, not bytecode execution.

### L8: Save/New Game And Scene Entry

Deliver:

- save/profile/scenario service behavior from evidence/oracle;
- `MakeNewGame` / `StartGame` path;
- scene/stage service load diagnostics.

Acceptance:

- StartGame selects a mission candidate through data/service state, not hard-coded scene boot.

## Stop Conditions

Do not stop unless:

- the full target is achieved;
- a hard blocker requires user action and no non-blocked loop task remains;
- the user explicitly interrupts or redirects.
