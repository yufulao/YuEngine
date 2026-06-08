# Oracle Title Boot Runbook

Purpose: capture original title boot behavior without patching or bypassing platform checks.

## Detected Install

- Game root: `C:\Steam\steamapps\common\TouhouNewWorld`
- Game exe exists: `True`
- Config tool exists: `True`
- Steam API DLL exists: `True`
- Pack files: `5`

## Capture Tool Status

- Available: `none`
- Missing: `procmon, procmon64, apitrace, renderdoccmd, pixwin, procdump`

## Safe Protocol

1. Record pre-run snapshot:

```powershell
python tools\oracle_title_boot.py snapshot --out build\oracle_title_boot\pre_title_snapshot.json
```

2. Launch the original game normally through Steam or use the explicit local launch command:

```powershell
python tools\oracle_title_boot.py launch --duration 20 --out build\oracle_title_boot\launch_title.json
```

3. Do not patch executables, replace Steam DLLs, or force entitlement state.

4. Capture or manually record:

- first visible title frame screenshot/hash;
- file/resource reads if Procmon or equivalent is available;
- D3D9 first present/render events if apitrace/RenderDoc/PIX is available;
- audio/BGM file or event if available;
- save list state before and after title boot.

5. Record post-run snapshot:

```powershell
python tools\oracle_title_boot.py snapshot --out build\oracle_title_boot\post_title_snapshot.json
```

## Acceptance For P1

- Three stable title boot runs.
- Trace maps back to `script/menu/titlemenu.b64.sqasm`.
- Title resource stems are observed or explicitly marked unobservable with current tools.
- Save list query behavior is sampled or explicitly marked pending.

## Current Limitation

This environment currently lacks dedicated file-IO/render capture tools unless one is installed later.
