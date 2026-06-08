# Import Audit

This audit is the first controlled migration step from `..\Project2` into `YuEngine`.
It intentionally avoids bulk-copying historical generated artifacts.

## Source Material Kept External For Now

These files remain source material in `..\Project2` until a concrete lane needs them:

- `docs/ENGINE_EXTRACTION_PLAN.md`
- `docs/PROJECT_RUNTIME_SPEC.md`
- `docs/ENGINE_SERVICE_MAP.md`
- `docs/GAME_RUNTIME_AND_PRODUCTION_MODEL.md`
- `docs/REBUILD_PLAN.md`
- `docs/PRE_RECONSTRUCTION_READINESS.md`
- `docs/PERFECT_RECONSTRUCTION_AUTOMATION.md`
- `docs/AI_NATIVE_ACCELERATED_PLAN.md`
- `docs/ORACLE_TRACE_PREP.md`
- `docs/SCRIPT_IR_PREP.md`
- `research/RECONSTRUCTION_FRONTIER.md`

Reason: the handoff already points to these documents. Copying them before wiring them into
YuEngine would create plan drift.

## Imported In This Slice

- `tools/sqir.py`: new YuEngine-owned Script IR tool, implemented from the observed `.sqasm`
  text format and optionally enriched from `..\Project2\research\evidence_graph.sqlite`.
- `docs/SCRIPT_IR_STATUS.md`: current P3 status, verification commands, and residual gaps.

Reason: P3 needs a runnable tool before runtime implementation can start. This is a narrow
implementation slice with direct verification.

## Generated Artifacts Not Imported

These stay outside git unless a later milestone justifies committing a small summary:

- `research/evidence_graph.sqlite`
- `research/evidence_graph.jsonl`
- `research/evidence_manifest.json`
- entrypoint closure JSON
- generated Script IR JSON
- trace captures
- screenshots/frame dumps

Generated Script IR should be produced into `build/script_ir` during verification.

## Next Import Candidates

Import only when the corresponding lane starts:

- `ENGINE_SERVICE_MAP.md` excerpts into a native-spec table when P4 starts.
- `PROJECT_RUNTIME_SPEC.md` excerpts into project-runtime docs when P5/X1 starts.
- Small closure markdown summaries into lane docs if they become verification fixtures.

## Current Decision

YuEngine owns new tools and compact status documents. Project2 remains the external evidence
cache and historical preparation area.
