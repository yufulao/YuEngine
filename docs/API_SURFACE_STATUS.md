# API Surface Status

Lane: `api-surface`

P6 target: group title/new-game/first-mission script-visible API obligations into reusable
engine services so later C++ modules have clear ownership.

## Implemented

- `tools/api_surface.py` builds an API surface map from the same `.sqasm` inputs used by P4.
- `docs/engine_api_surface/title_first_mission.md` records the current generated surface.
- The current surface groups 84 APIs and 303 call sites into 11 services.

## Verification Command

From `YuEngine` root:

```powershell
python tools\api_surface.py `
  --script ..\Project\output\scripts\script\menu\titlemenu.b64.sqasm `
  --script ..\Project\output\scripts\mission\sc01\main\ms010_0.b64.sqasm `
  --json-out build\api_surface\title_first_mission.json `
  --md-out docs\engine_api_surface\title_first_mission.md
python -m unittest discover -s tests
```

## Current Boundary

This is P6 baseline, not runtime implementation.

It does not confirm:

- native/static status;
- argument shape;
- return shape;
- side effects;
- C++ class layout;
- service implementation.
