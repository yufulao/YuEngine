# Native Boundary Spec Status

Lane: `native-spec`

P4 target: track every script-visible boundary needed by title/new-game and first-mission
without pretending that native behavior is already confirmed.

## Implemented

- `tools/native_spec.py` builds a deterministic boundary table from `.sqasm` inputs.
- It reuses `tools/sqir.py` and optional Project2 evidence graph enrichment.
- It assigns `proposed_owner` service ownership using service-map-derived rules.
- It records script examples, classification counts, residual unknowns, and implementation status.
- It keeps all rows at `confirmed_native=unknown`, `oracle_sampled=no`, and
  `implementation_status=not_started`.

## Verification Command

From `YuEngine` root:

```powershell
python tools\native_spec.py `
  --script ..\Project\output\scripts\script\menu\titlemenu.b64.sqasm `
  --script ..\Project\output\scripts\mission\sc01\main\ms010_0.b64.sqasm `
  --json-out build\native_spec\title_first_mission.json `
  --md-out docs\native_boundary_spec\title_first_mission.md
```

## Current Boundary

This is P4 baseline, not P4 completion.

P4 is not complete until rows have stronger evidence for:

- confirmed native/static status;
- argument shape;
- return shape;
- side effects;
- oracle/static evidence status;
- implementation status;
- residual mismatch notes after tests or oracle diffs.
