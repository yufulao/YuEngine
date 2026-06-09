# TouhouNewWorld Evidence Catalog

Status: Phase 0 evidence input
Owner: 射命丸文, original evidence
Created: 2026-06-10

This catalog separates TouhouNewWorld evidence from YuEngine runtime design.

The evidence below may become future Game Adapter acceptance input only after the lower engine
gates in `docs/YUENGINE_ARCHITECTURE_RESTART_PLAN.md` pass. It must not define Platform, Kernel,
RHI, Resource, Script, Scene, UI, Audio, Input, or Diagnostics APIs.

## Scope

| Input | Classification rule | Included |
| --- | --- | --- |
| `C:\Steam\steamapps\common\TouhouNewWorld\bin` | Original-game fact | Shipped executable, DLL, and binary config files. |
| `C:\Steam\steamapps\common\TouhouNewWorld\resource` | Original-game fact | Shipped JSON, SQLite, shader/filter files, and package blobs. |
| `C:\Steam\steamapps\common\TouhouNewWorld\YuEngine_BACKUP_20260610_022934\docs` | Diagnostic/report output | Old status reports, runbooks, and architectural notes. |
| `...\YuEngine_BACKUP_20260610_022934\src`, `apps`, `tools` | Old implementation artifact | Old engine, CLI, and analysis-tool source code. |
| `...\YuEngine_BACKUP_20260610_022934\tests`, `samples` | Future acceptance fixture | Old tests and sample manifests that can be rewritten after gates. |

Excluded from the catalog:

- `YuEngine_BACKUP_20260610_022934\.git`, because Git object storage is repository metadata.
- `YuEngine_BACKUP_20260610_022934\build`, because it is generated build output, not source
  evidence. Old docs can cite previous build/test results, but those reports remain diagnostic
  output.
- The current clean `YuEngine` implementation, except for this catalog and the generated inventory.

## Evidence Control Matrix

| Source path | Classification | Can become future fixture? | Prohibited use |
| --- | --- | --- | --- |
| `bin/AnkakeConfig.dll` | Original-game fact | Yes, for binary fingerprint/config-runtime observation fixtures after Verification/Tools gates. | Do not derive YuEngine config ownership or plugin/runtime API from the DLL shape. |
| `bin/ConfigTool.exe` | Original-game fact | Yes, for original configuration behavior/oracle capture after Verification/Tools gates. | Do not copy UI/config workflow into YuEngine tools architecture. |
| `bin/game.exe` | Original-game fact | Yes, for original executable fingerprint, import, dispatch, and capture fixtures after Verification/Tools gates. | Do not infer lower-layer engine APIs, renderer ownership, script VM ownership, or module boundaries directly from executable behavior. |
| `bin/steam_api64.dll` | Original-game fact | Yes, for dependency/runtime-environment observation after Platform/Verification gates. | Do not make Steam/platform integration a core YuEngine requirement. |
| `resource/ak3.json` | Original-game fact | Yes, for Game Adapter scenario/save/profile acceptance fixtures after Resource, Script, Scene, and Save/Profile facade gates. | Do not define YuEngine serialization, database, scenario, or save APIs from its schema. |
| `resource/dialogs.json` | Original-game fact | Yes, for dialog/tutorial adapter fixtures after UI text/dialog, Script, and Event gates. | Do not define YuEngine localization, UI widget, or dialog runtime ownership from its schema. |
| `resource/fxData.json` | Original-game fact | Yes, for adapter effect/stat fixtures after Resource and gameplay facade gates. | Do not define YuEngine effect, gameplay-stat, or data-table APIs from it. |
| `resource/data/info.db3` | Original-game fact | Yes, for gameplay-data adapter fixtures after Resource, Serialization, and adapter gates. | Do not introduce SQLite or original table names into YuEngine core/runtime APIs by default. |
| `resource/pack*.pak`, `resource/rpack*.dat` | Original-game fact | Yes, for File/VFS/package-reader fixtures after File/VFS and Resource gates. | Do not make package layout or report counts the Resource module API. |
| `resource/SMAA.fx`, `resource/SMAA.h`, `resource/system/**/*.bfx` | Original-game fact | Yes, for Renderer/RHI/material compatibility fixtures after RenderCore/RHI gates. | Do not derive shader/material ownership or backend submission model directly from these files. |
| `YuEngine_BACKUP_20260610_022934/docs/*` | Diagnostic/report output | Conditional; individual metrics can become acceptance expectations only after the owning module gate exists. | Do not treat old report schemas, command names, or status docs as runtime APIs or architecture decisions. |
| `YuEngine_BACKUP_20260610_022934/src/**` | Old implementation artifact | Conditional; small algorithms may be reconsidered only after owner, interface, tests, and gate approval exist. | Do not directly port `FrameRuntime.*`, report-shaped APIs, service-state shortcuts, or monolithic ownership. |
| `YuEngine_BACKUP_20260610_022934/apps/**` | Old implementation artifact | Conditional; CLI behavior can inform future external tools after Tools gate. | Do not make old CLI/report commands runtime dependencies. |
| `YuEngine_BACKUP_20260610_022934/tools/**` | Old implementation artifact | Conditional; scripts can inform offline evidence tools after Verification/Tools gates. | Do not run tool output as the runtime source of truth. |
| `YuEngine_BACKUP_20260610_022934/tests/**` | Future acceptance fixture | Yes, but only after rewriting around new module-owned public interfaces. | Do not import old tests as gates while they validate report existence or old architecture. |
| `YuEngine_BACKUP_20260610_022934/samples/**` | Future acceptance fixture | Yes, for clean fixture manifests after File/VFS, Project Manifest, Resource, and Script gates. | Do not let sample manifest fields define core engine API shape. |
| `YuEngine_BACKUP_20260610_022934/CMakeLists.txt`, `AGENTS.md`, `LICENSE`, `.gitignore` | Old implementation artifact | No direct runtime fixture use. | Do not copy old build layout, coding constraints, or repo metadata into architecture without separate approval. |

## Inventory

Detailed file-level classification is in
`docs/TOUHOU_NEWWORLD_EVIDENCE_INVENTORY.csv`.

The inventory contains 160 classified items:

| Classification | Count | Meaning |
| --- | ---: | --- |
| Original-game fact | 54 | Directly observed shipped `bin` and `resource` files. |
| Old implementation artifact | 40 | Old code, tools, build entry points, and repo metadata. |
| Diagnostic/report output | 52 | Old status docs and runbooks. |
| Future acceptance fixture | 14 | Old tests and samples that can inform future fixture design. |

Hash policy:

- Files at or below 50 MiB have SHA-256 in the CSV.
- Large package blobs above 50 MiB are recorded by path, size, and timestamp only in Phase 0.
  Full hashing can be added as a separate evidence-integrity task if required.

## Original-Game Facts

### Binary Set

| File | Size bytes | Evidence use |
| --- | ---: | --- |
| `bin/AnkakeConfig.dll` | 3,239,936 | Binary config/runtime behavior evidence source. |
| `bin/ConfigTool.exe` | 46,592 | Original configuration tool evidence source. |
| `bin/game.exe` | 8,549,144 | Primary original executable evidence source. |
| `bin/steam_api64.dll` | 298,384 | Shipped Steam integration dependency. |

These files are not implementation references for YuEngine core. They can support future binary
fingerprint, import-table, runtime-observation, and oracle-capture work in Verification/Tools.

### Resource Set

| File or directory | Size / count | Evidence use |
| --- | ---: | --- |
| `resource/ak3.json` | 240,013 bytes | Original structured scenario/config data. |
| `resource/dialogs.json` | 3,645,008 bytes | Original dialog/event text payloads. |
| `resource/fxData.json` | 32,479 bytes | Original effect/stat data. |
| `resource/data/info.db3` | 168,960 bytes | Original gameplay data tables. |
| `resource/pack01.pak` | 574,197,624 bytes | Original package blob. |
| `resource/pack02.pak` | 75,164,267 bytes | Original package blob. |
| `resource/rpack01.dat` | 6,686,065,166 bytes | Original resource package blob. |
| `resource/rpack02.dat` | 536,306,300 bytes | Original resource package blob. |
| `resource/rpack03.dat` | 545,253,682 bytes | Original resource package blob. |
| `resource/SMAA.fx`, `resource/SMAA.h` | 2 files | Original shader/anti-aliasing material evidence. |
| `resource/system/filter` | 11 `.bfx` files | Original post/filter shader binaries. |
| `resource/system/shader` | 28 `.bfx` files | Original material/render shader binaries. |

Structured data extracted in Phase 0:

- `ak3.json` contains `_msDescs` with 339 entries, `_pcInitDatas` with 12 entries,
  `_scenarios` with keys `sc01`, `sc02`, and `sc03`, plus 5 `global` keys.
- `dialogs.json` contains 1,451 dialog entries.
- `fxData.json` contains `_counter=25` and 1,239 `_stats` entries.
- `info.db3` contains 13 SQLite objects/tables. Row counts observed:
  `dropCategory=7`, `dropTable=409`, `enemy=158`, `enemyStatus=603`, `enemyType=5`,
  `equipment=170`, `playerEnhancement=73`, `playerInfo=12`, `playerStatus=94`,
  `skill=615`, `text=695`, `textCategory=8`, `sqlite_sequence=0`.

These facts are resource and adapter acceptance input. They do not decide YuEngine's resource
manager, database layer, localization layer, save format, or script API.

## Old Backup Evidence

### Diagnostic And Report Outputs

The old `docs` directory is diagnostic/report output. It records what the old YuEngine prototype
claimed to verify, along with boundaries. It is not architecture authority.

Important report facts:

| Old report | Useful evidence | Boundary |
| --- | --- | --- |
| `docs/VFS_RESOURCE_STATUS.md` | Old resource diagnostics reported 13,028 pack-manifest resources, 5 required resources, 0 required missing resources, title resource refs resolved, and first-mission stage/camera paths. | Dependency diagnostics only; not asset loading, rendering, or Resource API shape. |
| `docs/TITLE_SCRIPT_EXECUTION_STATUS.md` | Old script trace reached original title setup, title scenes, passive boot, and `title-new-game` path through recovered bytecode state. | Trace-ready state only; not a full VM, title UI, or gameplay runtime. |
| `docs/TITLE_BRANCHES_RUNTIME_STATUS.md` | Old branch matrix covered Continue disabled, Continue, New Game, Load empty, Load, Option, Exit denied, and Exit allowed. | Branch reachability and service-owned side effects only; not save UI, platform behavior, or serialization. |
| `docs/SCENE_ENTRY_RUNTIME_STATUS.md` | Old report connected title new-game to first mission setup, stage, event script, player, and rail-camera bindings. | Scene-entry contract only; not a rendered playable scene. |
| `docs/SCENE_RUNTIME_MATERIALIZATION_STATUS.md` | Old report read real payloads for `map/doujou/doujou.sge`, `.mdl`, `.col`, and `.rcm`; observed 42 stage dependencies, 111 mesh candidates, 150 collision triangles, and 3 rail-node candidates. | Runtime-handle materialization only; not renderer submission or gameplay simulation. |
| `docs/FIRST_FRAME_RUNTIME_STATUS.md` | Old report assembled first-frame renderer/actor/camera/input/event contract counts from original-driven state. | No window, no present, no playable frame. |
| `docs/FIRST_MISSION_TUTORIAL_STATUS.md` | Old report executed `threadEvent0020_00`, tutorial/dialog commands, player-control commands, and `updateUnits`. | One tutorial branch and update edge only; not a full gameplay loop. |
| `docs/GAMEPLAY_FRAME_RUNTIME_STATUS.md` | Old report joined title, scene, actor, camera, input, event, tutorial, audio, UI, and save/profile report lanes. | Aggregated service report only; not renderer backend, real input, animation, save serialization, or visual parity. |

### Old Implementation Artifacts

Old source directories observed:

- `src/yuengine/core`
- `src/yuengine/native`
- `src/yuengine/project`
- `src/yuengine/resource`
- `src/yuengine/runtime`
- `src/yuengine/script`

Notable quarantine files:

| Old file | Classification | Allowed use |
| --- | --- | --- |
| `src/yuengine/runtime/FrameRuntime.cpp` | Old implementation artifact | Failure example and responsibility inventory only. |
| `src/yuengine/runtime/FrameRuntime.h` | Old implementation artifact | Failure example and responsibility inventory only. |
| `src/yuengine/script/ScriptRuntime.cpp` | Old implementation artifact | Script evidence and future VM requirements only. |
| `src/yuengine/script/ScriptRuntime.h` | Old implementation artifact | Script evidence and future VM requirements only. |
| `src/yuengine/resource/ResourceDiagnostics.cpp` | Old implementation artifact | Fixture facts only; not Resource runtime API. |
| `src/yuengine/resource/VirtualFileSystem.*` | Old implementation artifact | Possible future VFS fixture reference only after File/VFS gate. |
| `tools/*.py`, `tools/*.ps1` | Old implementation artifact | Tool behavior reference only; not runtime core. |
| `apps/yuengine_cli.cpp` | Old implementation artifact | Old CLI/report entry point reference only. |

No old code is approved for direct porting by this catalog.

### Future Acceptance Fixtures

The old `samples` and `tests` directories are future fixture sources, not runnable gate
definitions for the clean YuEngine.

Observed fixture candidates:

- `samples/touhou_new_world/project.json` declares original resource/data paths, startup scripts,
  language keys, and required resource paths.
- `samples/empty_project/project.json` can become a minimal negative/empty fixture after File/VFS
  and Project Manifest gates.
- `tests/test_project_manifest.py`, `tests/test_sqir.py`, `tests/test_oracle_title_boot.py`,
  `tests/test_native_spec.py`, and `tests/test_api_surface.py` can inform future tests, but must
  be rewritten around new module-owned interfaces.

## Adapter Acceptance Input Candidates

These are not implementation permission. They are candidate facts for the future Game Adapter
layer after lower gates pass.

| Adapter area | Evidence source | Candidate acceptance input | Required lower gates first |
| --- | --- | --- | --- |
| Title boot | `samples/touhou_new_world/project.json`, `TITLE_SCRIPT_EXECUTION_STATUS.md` | Startup loads `preload.b64`, `script/menu/menudef.b64`, and `script/menu/titlemenu.b64`; entry function is `setupProc`. | File/VFS, Resource, Script VM/native bridge, Kernel diagnostics. |
| Title resources | `resource/*`, `VFS_RESOURCE_STATUS.md` | Title background/logo/DLC stems resolve through pack plus loose resources; language set is `jp`, `en`, `sc`, `tc`. | File/VFS, Resource handles, UI texture path, Renderer capture. |
| Title menu branch matrix | `TITLE_BRANCHES_RUNTIME_STATUS.md` | Continue disabled, Continue, New Game, Load empty, Load, Option, Exit denied, and Exit allowed branches are acceptance scenarios. | Input replay, UI framework, Script, Save/Profile facade, Platform shutdown facade. |
| New game transition | `ak3.json`, `TITLE_SCRIPT_EXECUTION_STATUS.md` | New-game path selects scenario `sc01`, calls save/profile/scenario services, and queues first mission load. | Script bridge, Save/Profile facade, Scene/World streaming, Resource. |
| First mission scene entry | `SCENE_ENTRY_RUNTIME_STATUS.md` | First mission setup binds `mission/sc01/main/ms010_0.b64.sqasm`, `map/Doujou/doujou.sge`, and `map/Doujou/doujou.rcm`. | Scene/World, Resource handles, Script bridge. |
| Stage payload | `SCENE_RUNTIME_MATERIALIZATION_STATUS.md` | Stage evidence includes SGE/MDL/COL/RCM payloads, 42 dependencies, 111 mesh candidates, 150 collision triangles, and 3 rail-node candidates. | Resource, RenderCore, RHI, Physics/collision boundary. |
| Player actor | `SCENE_ENTRY_RUNTIME_STATUS.md`, `SCENE_RUNTIME_MATERIALIZATION_STATUS.md` | Player evidence includes `reimuEx`, `player/reimuex.b64`, `player/reimuex_pcg.b64`, marker-derived spawn expression, and rotY `0`. | Actor/entity model, Animation, Script bridge, Input. |
| Camera | `SCENE_ENTRY_RUNTIME_STATUS.md`, `FIRST_FRAME_RUNTIME_STATUS.md` | Rail camera `map/doujou/doujou.rcm` and default target `ev_sc01_main_ms010_0` are acceptance data. | Camera system, Scene/World, Input, Render scene. |
| Tutorial/dialog | `dialogs.json`, `FIRST_MISSION_TUTORIAL_STATUS.md` | Tutorial event `threadEvent0020_00`, dialog show/speak/wait/hide counts, tutorial actor/page, and player-control commands are candidate checks. | UI text/dialog framework, Actor, Script, Input, Event system. |
| First-frame contract | `FIRST_FRAME_RUNTIME_STATUS.md`, `GAMEPLAY_FRAME_RUNTIME_STATUS.md` | Mesh/material/texture, UI, actor, camera, input, event, audio, and save/profile counts can become oracle expectations. | Renderer submit/present/capture, Audio mixer, Input replay, Scene/World, Diagnostics. |

## Non-Claims

This catalog does not approve:

- Direct migration of old `FrameRuntime.*` or report-shaped runtime APIs.
- A TouhouNewWorld-first engine architecture.
- A mesh viewer, blue-screen demo, title-menu rewrite, or report dashboard as engine progress.
- Game Adapter code before Platform, Kernel, File/VFS, Resource, Script, Scene/World, UI, Audio,
  Input, Renderer, and Diagnostics gates are met for the behavior being tested.
- Using diagnostic JSON/report schemas as public runtime interfaces.

## Gate Rule For Future Use

When a future task wants to use an item in this catalog:

1. Name the catalog item and its source file.
2. State whether it is an original-game fact, old implementation artifact, diagnostic/report
   output, or future acceptance fixture.
3. Name the owning YuEngine module and public interface being validated.
4. Prove the lower-layer gate has passed.
5. Keep the implementation in the owning module or Game Adapter layer; do not bypass engine
   systems for the TouhouNewWorld workload.

If any step is missing, the item stays evidence-only.
