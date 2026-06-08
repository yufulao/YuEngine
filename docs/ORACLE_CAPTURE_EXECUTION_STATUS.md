# L6 Oracle Capture Execution Status

Status: completed as oracle capture readiness on 2026-06-09.

This loop prepares original title boot capture without patching executables, replacing Steam
DLLs, or forcing entitlement state. It does not complete P1 because no user-driven original-game
title boot run has been recorded.

## Implemented

- `tools/oracle_title_boot.py readiness`
- Current install inspection JSON under `build/oracle_title_boot/inspect.json`
- Current capture readiness JSON under `build/oracle_title_boot/readiness.json`
- Current pre-title install/state snapshot under `build/oracle_title_boot/pre_title_snapshot.json`
- Regenerated `docs/oracle/title_boot_runbook.md`

Readiness records:

```text
can_launch_original_exe: true
requires_user_driven_launch: true
safe_to_bypass_platform: false
file_io_capture_tools: []
render_capture_tools: []
dump_capture_tools: []
p1_complete: false
```

Current blockers:

```text
no file-IO capture tool found on PATH
no D3D/render capture tool found on PATH
no user-driven title boot run has been recorded in this workspace
```

## Verified Install

Detected:

```text
bin/game.exe
bin/ConfigTool.exe
bin/steam_api64.dll
resource/ak3.json
resource/data/info.db3
resource/pack01.pak
resource/pack02.pak
resource/rpack01.dat
resource/rpack02.dat
resource/rpack03.dat
```

No save/config candidate directory exists yet in the checked standard locations.

## Verification

```powershell
python tools\oracle_title_boot.py inspect --out build\oracle_title_boot\inspect.json
python tools\oracle_title_boot.py readiness --out build\oracle_title_boot\readiness.json
python tools\oracle_title_boot.py snapshot --out build\oracle_title_boot\pre_title_snapshot.json
python tools\oracle_title_boot.py runbook --out docs\oracle\title_boot_runbook.md
python -m unittest discover -s tests
```

Verified result:

```text
Python unittest: 6/6 passed
```

## Boundary

P1 is still incomplete. A valid P1 completion needs three stable original title boot runs and
observations mapped back to `script/menu/titlemenu.b64.sqasm`.

Because this environment has no capture tools and no user-driven launch record, this loop stops
at readiness and hands the next non-blocked runtime task to L7. L7 must not invent oracle data;
it can build VM/service plumbing and keep all unknown native behavior as obligations.
