# L22 Backend State Status

L22 converts the L21 texture upload gates into backend-facing sampler, pass render-state, and font
glyph metric records. It still does not bind any GPU state. This checkpoint exists so the later
D3D backend cannot treat "render state" or "font" as a generic open box.

## Contract

```text
project.json
-> L21 typed texture upload records
-> L20 device presentation contract
-> L19 material semantics
-> title UI command payload
-> SMAA.fx sampler/pass state
-> backend state records
-> GPU binding/font atlas/oracle obligations
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-state samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_backend_state_contract --output-on-failure
```

Current metric:

```text
ok=true
texture_upload_ok=true
device_presentation_ok=true
material_semantics_ok=true
title_ui_ok=true
backend_state_runtime_ready=true
sampler_state_records_ready=true
pass_render_state_records_ready=true
font_atlas_records_ready=true
material_shader_program_gate_tracked=true
gpu_state_binding_gate_tracked=true
oracle_parity_gate_tracked=true
sampler_state_records=7
sampler_texture_bindings=7
sampler_clamp_address_records=7
sampler_linear_min_filters=6
sampler_point_min_filters=1
sampler_srgb_true_records=1
sampler_srgb_false_records=6
pass_state_records=5
pass_vs30_shaders=5
pass_ps30_shaders=5
z_disabled_passes=5
alpha_blend_disabled_passes=5
alpha_test_disabled_passes=5
srgb_write_enabled_passes=1
srgb_write_disabled_passes=4
stencil_enabled_passes=4
stencil_disabled_passes=1
stencil_replace_passes=3
stencil_keep_passes=1
stencil_equal_passes=1
font_query_records=6
text_draw_commands=6
graph_string_commands=5
string_size_queries=5
localized_menu_text_commands=10
texture_upload_records=39
material_texture_consumers=39
resolved_backend_state_contracts=4
tracked_backend_state_obligations=4
open_backend_state_obligations=4
```

## Contract State

- `smaa_sampler_state_records`: `contract_ready`, 7 sampler blocks expose texture binding,
  address mode, filter mode, and SRGB state.
- `smaa_pass_render_state_records`: `contract_ready`, 5 pass blocks expose `vs_3_0`/`ps_3_0`,
  depth, alpha, SRGB, and stencil state.
- `title_font_glyph_metric_inputs`: `contract_ready`, title UI has 6 font queries, 6 text draw
  commands, 5 graph strings, and 5 string-size queries.
- `backend_state_consumes_texture_uploads`: `contract_ready`, backend state consumes 39 texture
  upload records and 39 material slot consumers.
- `material_shader_program_binding`: `tracked_open`, model materials still lack a per-material
  shader program token.
- `gpu_state_binding_execution`: `tracked_open`, sampler/pass state records are decoded but not
  submitted to a GPU device.
- `font_atlas_texture_implementation`: `tracked_open`, glyph metric inputs exist but no atlas
  texture, glyph cache, or text draw backend exists.
- `original_frame_oracle_trace`: `tracked_open`, backend state still lacks original-frame capture
  or graphics API trace parity.

## Boundary

L22 is not a device backend. It does not:

- create or reset a D3D9 device;
- call `SetSamplerState`, `SetRenderState`, or `SetTexture`;
- compile material shaders;
- allocate a font atlas texture;
- draw text glyphs;
- compare a captured original frame.

Future backend work must consume these typed records. It cannot collapse sampler, pass state, and
font state back into one generic renderer obligation.

## Next Edges

- L23 has since created D3D9-compatible backend resource allocation records from L21/L22 evidence.
- L24 has since created device resource creation/upload/state-binding execution records.
- L25 has since created swapchain/present/original-frame oracle parity records that consume L24
  device execution records.
- L26 has since created the platform/backend bridge submission queue.
- L27: concrete backend executor interface only after resource allocation, state binding,
  presentation, and oracle gates can fail independently.
