# L35 Backend Font Atlas Runtime Status

L35 consumes the L34 FMP/DDS evidence and removes the font atlas placeholder from the active edge.
It creates real D3D9 textures for the shipped atlas DDS files, uploads their payloads, and validates
the FMP glyph record container layout. It still does not render text or infer non-ASCII glyph key
semantics beyond the evidence that is currently proven.

## Contract

```text
project.json
-> L34 program/depth/font evidence
-> persistent D3D9 device service
-> 4 FMP font maps
-> 20854 32-byte glyph records
-> 7 length-prefixed atlas links
-> 7 4096x4096 two-mip D3DFMT_A8 atlas DDS files
-> 7 IDirect3DDevice9::CreateTexture results
-> 14 LockRect/UnlockRect mip uploads
-> draw/present/capture/oracle remain deferred
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-font-atlas samples\touhou_new_world\project.json --repo-root .
build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_font_atlas_contract
tools\verify_runtime.ps1
```

Current metric:

```text
ok=true
program_depth_font_ok=true
device_creation_ok=true
persistent_device_service_ready=true
font_atlas_resource_evidence_ready=true
fmp_glyph_layout_probe_ready=true
font_atlas_texture_records_ready=true
font_atlas_d3d_texture_creation_executed=true
font_atlas_payload_upload_executed=true
font_atlas_upload_payload_parity_ready=true
exact_codepoint_encoding_tracked_open=true
text_draw_backend_binding_deferred=true
material_program_selection_still_deferred=true
sampleable_depth_still_deferred=true
downstream_draw_present_deferred=true
backbuffer_extent_carried=true
font_map_files=4
fmp_glyph_records=20854
fmp_glyph_record_stride32_files=4
fmp_atlas_tail_links=7
fmp_metric_records_in_range=20854
fmp_ascii_glyph_records=380
fmp_ascii_packed_key_matches=380
fmp_monotonic_glyph_maps=4
font_atlas_texture_records=7
font_atlas_textures_created=7
font_atlas_upload_records=7
font_atlas_uploaded_subresources=14
font_atlas_failed_records=0
font_atlas_4096=7
font_atlas_a8=7
font_atlas_mip2=7
font_atlas_payload_matches=7
font_atlas_payload_bytes=146800640
font_atlas_uploaded_payload_bytes=146800640
font_query_records=6
text_draw_commands=6
string_size_queries=5
preserved_depth_texture_bindings=1
preserved_material_program_bindings=38
draw_present_capture_records_deferred=124
backbuffer_width=1280
backbuffer_height=720
resolved_font_atlas_contracts=5
tracked_font_atlas_obligations=6
open_font_atlas_obligations=6
```

Measured on 2026-06-09:

- Direct `backend-font-atlas`: about 43.9 seconds.
- `runtime-contract-suite --filter yuengine_backend_font_atlas_contract`: 43.883 seconds.
- `tools\verify_runtime.ps1 -SkipPython -SkipDiffCheck -NoBuild`: 43.247 seconds.
- Pre-smoke-split `ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure`: 43.94
  seconds.
- Post-smoke-split default `ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure` does
  not run this L35 edge; use `tools\verify_runtime.ps1 -Mode edge` for the current deepest edge.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: 41/41 contracts, runtime suite
  elapsed_ms=69923, wall time about 93.6 seconds.

## Contract State

- `fmp_glyph_record_layout_probe`: `contract_ready`, 4 FMP files expose 20854 32-byte glyph records,
  7 length-prefixed atlas links, 380 ASCII glyph records, and all metric fields stay in 0..1 range.
- `font_atlas_texture_creation_execution`: `contract_ready`, 7 recovered atlas DDS files create real
  `IDirect3DTexture9` handles as `D3DFMT_A8`.
- `font_atlas_payload_upload_execution`: `contract_ready`, 14 mip subresources upload through
  `LockRect/UnlockRect`.
- `font_atlas_payload_parity`: `contract_ready`, 146800640 uploaded bytes match the DDS payload
  bytes exactly.
- `exact_fmp_codepoint_encoding`: `tracked_open`, ASCII packed keys are verified but non-ASCII
  packed key semantics are not named yet.
- `font_text_draw_backend_binding`: `tracked_open`, title text draw and string-size commands are
  preserved but not converted into glyph quads.
- `material_program_selection_binding`: `tracked_open`, font work does not guess scene material
  shader programs or passes.
- `sampleable_depth_texture_implementation`: `tracked_open`, D24S8 depth remains not sampleable.
- `draw_present_capture_after_font_atlas`: `tracked_open`, draw/present/capture remain blocked.

## Boundary

L35 does not:

- fully define the non-ASCII glyph key encoding in the FMP records;
- build glyph quads or execute a text draw backend;
- bind text rendering into the title/menu draw pass;
- choose material shader programs/passes;
- bind lightmap material slots;
- convert the D24S8 depth surface into a sampleable texture;
- issue draw calls, call `Present`, capture frames, or run oracle comparison.

## Next Edge

L36 should close material program/pass selection, lightmap material binding, and sampleable depth
texture evidence enough to allow draw submission work to start without guessing shader ownership.
