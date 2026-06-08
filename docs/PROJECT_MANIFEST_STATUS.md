# Project Manifest Status

Lane: `project-runtime`

P5 target: define the minimal project manifest needed to describe the original game as the
first sample/oracle project and later describe new projects without hard-coded game paths.

## Implemented

- `samples/touhou_new_world/project.json`: YuEngine-owned sample manifest for the original game.
- `samples/empty_project/project.json`: generic sample manifest that does not depend on the
  original game's resources.
- `tools/project_manifest.py validate`: manifest validator.
- Required checks:
  - schema and top-level fields;
  - language default/support list;
  - mount existence and mount type;
  - script root existence;
  - preload and entry script resolution;
  - data file existence;
  - runtime profile fields;
  - required resource path/stem resolution through loose mounts or pack manifest.

## Verification Command

From `YuEngine` root:

```powershell
python tools\project_manifest.py validate samples\touhou_new_world\project.json --json-out build\project_manifest\touhou_new_world.validation.json
python tools\project_manifest.py validate samples\empty_project\project.json --json-out build\project_manifest\empty_project.validation.json
python -m unittest discover -s tests
```

## Current Scope

This proves the original game can be represented as project data for a future runtime boot
contract, and that the schema can also describe a new non-original sample project. It does not
boot the runtime yet.

## Residual Gaps

- No runtime consumes `project.json` yet.
- Pack reader and VFS are not implemented yet.
- Save/profile policy is named but not implemented.
