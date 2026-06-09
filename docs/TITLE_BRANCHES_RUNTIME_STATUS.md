# L14 Title Branches Runtime Status

L14 turns the existing title input scenarios into a formal runtime contract. It executes original
`script/menu/titlemenu.b64.sqasm` bytecode for each branch; it is not a replacement menu or a
hand-written state table.

## Contract

The runtime executes this scenario matrix:

- `title-continue-disabled`
- `title-continue`
- `title-new-game`
- `title-load-empty`
- `title-load`
- `title-option`
- `title-exit-denied`
- `title-exit-allowed`

Each scenario uses the same startup baseline modules and `setupProc`, then advances five title
frames through the original branch code. Service state captures:

- Save/Profile/Scenario: save-list entries, autosave load, save-capacity query, MakeNewGame,
  StartGame, queued stage load evidence;
- Platform: CanShutdown and ShutdownGame outcomes;
- UI: option branch menu mutations;
- Audio/Scene/UI command counts as branch evidence.

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe title-branches samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_title_branches_runtime_contract --output-on-failure
```

Current metric:

```text
ok=true
scenario_count=8
executed_scenarios=8
start_game_scenarios=3
load_auto_save_scenarios=2
make_new_game_scenarios=1
shutdown_permission_scenarios=2
shutdown_game_scenarios=1
option_ui_mutations=2
unresolved_calls=0
truncated=false
```

## Branch Evidence

- Continue disabled: save list remains empty and no StartGame occurs.
- Continue: autosave load path reaches StartGame and queues stage load.
- New Game: scenario key, save-capacity check, MakeNewGame, StartGame, and stage queue all occur.
- Load empty: save list remains empty and no StartGame occurs.
- Load: autosave load path reaches StartGame and queues stage load.
- Option: UI object mutation occurs without gameplay start or shutdown request.
- Exit denied: CanShutdown returns false and ShutdownGame is not requested.
- Exit allowed: CanShutdown returns true and ShutdownGame is requested.

## Boundary

L14 proves branch reachability and service-owned side effects. It does not prove:

- real save serialization/deserialization;
- a full save browser UI;
- platform/Steam behavior;
- final StartGame argument schema for Continue/Load beyond the recovered autosave branch;
- a repeated gameplay frame loop.

Continue/Load preserve the recovered autosave branch shape instead of forcing it into the New
Game mission-label schema.

## Next Edges

- L15: join title/scene/actor/camera/input/event/audio/UI service state into a repeated
  gameplay-frame update contract.
- L16: consume title UI and scene mesh/material/texture command payloads in a backend-facing
  renderer contract.
