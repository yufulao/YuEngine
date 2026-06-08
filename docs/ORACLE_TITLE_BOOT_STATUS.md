# Oracle Title Boot Status

Lane: `oracle`

P1 target: sample original title boot behavior without patching Steam, replacing DLLs, or
forcing entitlement state.

## Implemented

- `tools/oracle_title_boot.py inspect`: detects install paths, key files, pack files, save/config
  candidates, and local capture tool availability.
- `tools/oracle_title_boot.py snapshot`: records install and save/config candidate snapshots.
- `tools/oracle_title_boot.py runbook`: writes a concrete title boot sampling protocol.
- `tools/oracle_title_boot.py launch`: optional explicit launch helper. It is not used by tests
  and should only be run when an interactive original-game launch is intended.

## Verification Commands

From `YuEngine` root:

```powershell
python tools\oracle_title_boot.py inspect --out build\oracle_title_boot\inspect.json
python tools\oracle_title_boot.py snapshot --out build\oracle_title_boot\pre_title_snapshot.json
python tools\oracle_title_boot.py runbook --out docs\oracle\title_boot_runbook.md
python -m unittest discover -s tests
```

## Current Local Finding

The install root contains:

- `bin/game.exe`
- `bin/ConfigTool.exe`
- `bin/steam_api64.dll`
- `resource/ak3.json`
- `resource/data/info.db3`
- `resource/pack01.pak`
- `resource/pack02.pak`
- `resource/rpack01.dat`
- `resource/rpack02.dat`
- `resource/rpack03.dat`

PATH did not expose Procmon, apitrace, RenderDoc, PIX, or procdump during this preparation.

## Residual Gaps

- No original-game process was launched in this slice.
- No D3D9 frame, screenshot, audio, or file-IO trace has been sampled yet.
- P1 is not complete until three stable title boot runs are captured and mapped back to
  `script/menu/titlemenu.b64.sqasm`.
