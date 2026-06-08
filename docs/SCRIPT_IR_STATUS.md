# Script IR Status

Lane: `script-ir`

P3 target: convert `.sqasm` into stable JSON IR without losing function identity, literals,
locals, source lines, instructions, callsites, resource references, state-machine candidates,
or native/engine boundary candidates.

## Implemented

- Parser for the observed `.sqasm` text format:
  - script input path
  - function ordinal, name, source path, bytecode offset
  - stack/generator/varargs metadata
  - parameters
  - literals
  - locals
  - instruction list with pc/source line/op/args/comment references
- Callsite extraction from `_OP_PREPCALL*` instructions.
- Conservative resource literal/stem extraction from `.sqasm` text when evidence graph is absent.
- Closure binding extraction from `_OP_CLOSURE` plus nearby slot literals.
- Optional enrichment from `..\Project2\research\evidence_graph.sqlite`:
  - evidence script counts
  - boundary call classifications
  - direct resource literals
  - resource stem candidates
- State-machine summary output.

## Verification Commands

From `YuEngine` root:

```powershell
python tools\sqir.py ..\Project\output\scripts\script\menu\titlemenu.b64.sqasm --out-dir build\script_ir
python tools\sqir.py ..\Project\output\scripts\mission\sc01\main\ms010_0.b64.sqasm --out-dir build\script_ir
python tools\sqir.py ..\Project\output\scripts\mission\ms0915.b64.sqasm --out-dir build\script_ir
python tools\sqir.py ..\Project\output\scripts\mission\ms0916.b64.sqasm --out-dir build\script_ir
python -m unittest discover -s tests
```

## Acceptance For This Slice

The generated title menu IR must preserve these names:

- `TitleSceneBase`
- `TitleScene`
- `NewGameScene`
- `LoadScene`
- `OverwriteSaveScene`
- `startGame4Menu`
- `savesIsEmpty`
- `continueDisabled`
- `GetSaveList`
- `LoadAutoSave`
- `MakeNewGame`
- `StartGame`
- `menu/title/title_back`

The generated first mission IR must preserve these names:

- `setupProcess`
- `LoadStage`
- `LoadEventsScriptViaMission`
- `CallSetupEvents`
- `PushPlayerChara`
- `PushTaskGameCamera`
- `GetFlag`
- `ActorTutorial`
- `map/Doujou/doujou.sge`

## Residual Gaps

- This is not a Squirrel VM and does not execute bytecode.
- It does not prove native behavior; classifications remain evidence labels only.
- It does not reconstruct high-level C++ or Squirrel source.
- It does not yet infer argument shapes or side effects for native calls.
- State-machine extraction is currently candidate-level, not semantic control-flow recovery.
