# L36 Backend Material Program Runtime Status

L36 consumes the L35 font-atlas checkpoint and turns scene material program selection from a loose
open gate into per-material runtime records. It binds all recovered material texture roles to CTAB
sampler registers in selected deferred shader binaries, including the `ki:tree` lightmap slot.

This is still not draw permission. The selector records are rule-derived from original material
blocks, texture-role patterns, `.bfx` technique/sampler tokens, and renderer profile evidence. The
original C++ selector function and the sampleable depth texture implementation are still
tracked-open.

## Contract

```text
project.json
-> L35 font atlas runtime
-> original map/doujou/doujou.mdl material blocks
-> 16 stage material records
-> deferred/deferredGrass/deferredMulti .bfx CTAB sampler records
-> TNonSkin vs_3_0/ps_3_0 technique token evidence
-> 39 material texture roles resolved to selected shader sampler registers
-> ki:tree lightmap resolved to SamplerLight
-> SMAA depthTex source evidence plus no sampleable-depth format token
-> draw/present/capture/oracle remain deferred
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-material-program samples\touhou_new_world\project.json --repo-root .
build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_material_program_contract
tools\verify_runtime.ps1 -Mode edge -Jobs 8
```

Current metric:

```text
ok=true
font_atlas_ok=true
program_depth_font_ok=true
shader_sampler_ok=true
material_semantics_ok=true
material_block_program_token_probe_ready=true
shader_technique_token_evidence_ready=true
material_program_rule_selection_ready=true
material_sampler_slot_closure_ready=true
lightmap_program_binding_ready=true
smaa_depth_sampler_source_evidence_ready=true
sampleable_depth_negative_evidence_ready=true
exact_program_selector_function_still_open=true
sampleable_depth_implementation_still_open=true
downstream_draw_present_deferred=true
materials=16
material_block_scan_records=16
material_block_program_token_hits=0
program_selection_records=16
rule_derived_program_records=16
deferred_program_records=12
deferred_grass_program_records=2
deferred_multi_program_records=2
non_skin_technique_records=16
ps30_program_records=16
vs30_program_records=16
material_texture_slots=39
resolved_program_sampler_slots=39
missing_program_sampler_slots=0
lightmap_sampler_bindings=1
smaa_depth_texture_declarations=1
smaa_depth_sampler_declarations=1
smaa_depth_technique_passes=1
sampleable_depth_format_token_hits=0
preserved_depth_texture_bindings=1
preserved_material_program_bindings=38
draw_present_capture_records_deferred=124
resolved_material_program_contracts=8
tracked_material_program_obligations=4
open_material_program_obligations=4
```

Measured on 2026-06-09:

- Direct `backend-material-program`: about 43.8 seconds.
- `tools\verify_runtime.ps1 -Mode edge -NoBuild -SkipPython -SkipDiffCheck`: L36 edge filter,
  elapsed_ms=42281.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: clean build, Python unittest, 42/42
  contracts, and `git diff --check` passed; runtime suite elapsed_ms=114358, wall time about 138
  seconds.

## Contract State

- `material_block_program_token_probe`: `contract_ready`, 16 original material blocks were scanned
  and contain no direct shader/effect/technique/pass token.
- `bfx_static_stage_technique_tokens`: `contract_ready`, selected deferred shader binaries expose
  `TNonSkin`, `vs_3_0`, and `ps_3_0`.
- `deferred_stage_material_rule_selection`: `contract_ready`, 16 material records map to
  `deferred`, `deferredGrass`, or `deferredMulti` by recovered texture-role rules.
- `material_program_sampler_slot_closure`: `contract_ready`, all 39 material texture roles resolve
  to selected shader CTAB sampler registers, including `SamplerDiffuse2/Normal2/Specular2`.
- `lightmap_sampler_binding`: `contract_ready`, `ki:tree` resolves to `SamplerLight`.
- `smaa_depth_sampler_source_evidence`: `contract_ready`, `SMAA.fx` declares `depthTex2D`,
  `depthTex`, and `DepthEdgeDetection`.
- `sampleable_depth_negative_evidence`: `contract_ready`, shipped shader/filter resources expose no
  `INTZ`, `RAWZ`, `DF24`, `R32F`, or `RESZ` sampleable-depth format token.
- `exact_original_program_selector_function`: `tracked_open`, selector records are rule-derived and
  not yet the original engine C++ selector function.
- `sampleable_depth_texture_implementation`: `tracked_open`, `depthTex2D` is still only a D24S8
  depth/stencil surface in recovered evidence.
- `draw_present_capture_after_material_program`: `tracked_open`, draw/present/capture remains
  deferred.

## Boundary

L36 does not:

- claim the original C++ material selector function has been recovered;
- make `depthTex2D` sampleable or invent an unsupported DX9 depth texture format;
- execute scene draw calls;
- call `Present`;
- capture frames or run oracle image comparison.

## Next Edge

L36b should recover the exact original selector function or a stronger binary-level equivalent, and
recover the real sampleable depth path. Draw submission must stay deferred until those are closed.
