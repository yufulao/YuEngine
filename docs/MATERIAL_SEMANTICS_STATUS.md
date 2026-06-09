# L19 Material Semantics Status

L19 turns the previous material count into runtime-owned material semantics. It consumes
`scene-runtime`, `renderer-submit`, and `backend-obligations`, then decodes `doujou.mdl` material
blocks into material handles, texture slots, mesh bindings, and shader/effect obligations.

This is still not a GPU backend. It is the contract that a GPU backend must consume.

## Contract

```text
project.json
-> scene-runtime model handles
-> renderer backend submission
-> backend texture/material obligations
-> model mat blocks
-> material texture slots
-> mesh-name material binding
-> shader/effect obligation tracking
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe material-semantics samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_material_semantics_contract --output-on-failure
```

Current metric:

```text
ok=true
scene_runtime_ok=true
renderer_submission_ok=true
backend_obligations_ok=true
material_semantics_contract_ready=true
texture_slot_contract_ready=true
mesh_material_contract_ready=true
shader_effect_contract_tracked=true
post_effect_source_tracked=true
materials=16
material_parameter_blocks=16
texture_slots=39
resolved_texture_slots=39
mesh_submissions=111
named_mesh_submissions=110
mesh_material_bindings=110
unresolved_mesh_material_bindings=1
post_effect_techniques=5
post_effect_passes=5
post_effect_samplers=7
```

## Evidence

- `map/doujou/doujou.mdl` contains 16 `mat` blocks.
- Each material block carries header flags, color candidates, texture string slots, and a material
  name near the block tail.
- 39 material texture slots resolve through VFS.
- Texture slot roles are inferred from original filenames only:
  `base_color_candidate`, `normal_candidate`, `specular_candidate`, `emissive_candidate`, and
  `lightmap_candidate`.
- 110 named `msh` submissions bind to materials through original mesh-name suffixes.
- One mesh has no length-prefixed name before the `msh` tag; it is tracked as an open gap instead
  of being assigned to a guessed material.
- `resource/SMAA.fx` is tracked as DX9 HLSL postprocess source: 5 techniques, 5 passes, and
  7 samplers.

## Material Binding Distribution

```text
kabe1=22
kabe2=2
blinn4=1
kamidana:blinn16_kamidana=1
ukoku_katana1=1
kakejiku=1
mado=2
light:light=4
lambert3=5
blinn8=1
rock=1
ki1=12
ki:tree=12
hana1=35
kusa:blinn1kusa01=10
blinn10=0
unnamed_mesh_gap=1
```

## Boundary

L19 does not:

- compile shaders;
- create a D3D device;
- upload textures to GPU memory;
- bind actual samplers/blend/depth state;
- prove per-material shader/effect program identity;
- compare rendered frames against the original game.

Do not invent shader names or assign the unnamed mesh to a material without byte-level evidence.

## Next Edges

- L20: device/swapchain and render-state presentation contract.
- Then: texture upload implementation, sampler/blend/depth state, font atlas/glyph metrics, and
  original-frame oracle parity.
