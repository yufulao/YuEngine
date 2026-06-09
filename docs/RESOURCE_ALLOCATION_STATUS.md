# L23 Resource Allocation Status

L23 converts L21 typed texture uploads and L22 backend sampler/pass/font records into
D3D9-compatible resource allocation records. It still does not create a D3D device or call
`CreateTexture`. This checkpoint exists so the later device backend cannot invent resource shape
or collapse unknown render targets and font atlas state into a generic renderer gap.

## Contract

```text
project.json
-> L21 typed texture upload records
-> L22 backend sampler/pass/font records
-> SMAA texture declarations and lookup DDS payloads
-> D3D9-compatible resource allocation records
-> device creation/binding/oracle obligations
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe resource-allocation samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_resource_allocation_contract --output-on-failure
```

Current metric:

```text
ok=true
texture_upload_ok=true
backend_state_ok=true
resource_allocation_runtime_ready=true
stage_texture_allocation_records_ready=true
smaa_lookup_allocation_records_ready=true
transient_surface_allocation_gate_tracked=true
font_atlas_allocation_gate_tracked=true
d3d_resource_creation_gate_tracked=true
oracle_parity_gate_tracked=true
allocation_records=46
ready_allocation_records=41
tracked_open_allocation_records=5
stage_texture_allocations=39
d3d_dxt1_allocations=31
d3d_dxt5_allocations=8
cube_texture_allocations=1
smaa_lookup_allocations=2
lookup_l8_allocations=1
lookup_a8l8_allocations=1
transient_surface_candidates=4
font_atlas_placeholders=1
sampler_texture_declarations=6
material_texture_consumers=39
ready_allocation_byte_total=23955042
ready_allocation_payload_bytes=23949794
ready_expected_payload_bytes=23949794
resolved_allocation_contracts=2
tracked_allocation_obligations=5
open_allocation_obligations=5
```

## Contract State

- `stage_texture_allocation_records`: `contract_ready`, 39 stage DDS uploads map to typed
  D3D9 texture/cube allocation records.
- `smaa_lookup_texture_allocation_records`: `contract_ready`, `areatexdx9.dds` maps to
  `D3DFMT_A8L8` and `searchtex.dds` maps to `D3DFMT_L8`.
- `smaa_transient_surface_allocation_candidates`: `tracked_open`, `colorTex2D`, `depthTex2D`,
  `edgesTex2D`, and `blendTex2D` are 1280x720 candidates but exact ownership/formats are not
  proven.
- `font_atlas_allocation_placeholder`: `tracked_open`, title font metric inputs exist but atlas
  dimensions and glyph cache ownership are not recovered.
- `d3d_resource_creation_execution`: `tracked_open`, no `IDirect3DDevice9` allocation calls exist
  in YuEngine yet.
- `material_shader_resource_binding`: `tracked_open`, stage textures have material consumers but
  no recovered per-material shader program token.
- `original_frame_oracle_trace`: `tracked_open`, allocation records still lack original graphics
  API trace or frame capture parity.

## Evidence Now Locked

- 39 stage allocations preserve DDS format, width, height, mip count, cube faces, payload bytes,
  expected payload bytes, and material consumer counts from L21.
- Stage formats are 31 `D3DFMT_DXT1` and 8 `D3DFMT_DXT5`.
- `cubeenvmap/doujou_1.dds` is a six-face `D3DFMT_DXT1` cube allocation record.
- `system/glsl/smaa/areatexdx9.dds` is 160x560, one mip level, `D3DFMT_A8L8`, payload 179200.
- `system/glsl/smaa/searchtex.dds` is 66x33, one mip level, `D3DFMT_L8`, payload 2178.
- SMAA declares 6 `texture2D` resources; 2 are ready lookup textures and 4 are tracked-open
  transient surface candidates.
- The font atlas is a tracked-open allocation placeholder, not an implemented texture.

## Boundary

L23 is not a device backend. It does not:

- create an HWND or `IDirect3DDevice9`;
- call `CreateTexture`, `CreateCubeTexture`, `CreateRenderTarget`, or `CreateDepthStencilSurface`;
- upload texture levels to GPU memory;
- bind sampler/pass state records to a GPU device;
- allocate or populate a font atlas;
- draw or compare an original frame.

Future backend work must consume these records. It cannot rescan the resource tree, infer texture
shape from filenames, or treat SMAA transient surfaces and font atlas state as already solved.

## Next Edges

- L24 has since created device resource creation and state-binding execution records from L23
  allocation records.
- L25 has since created swapchain/present/frame-oracle parity records that consume the L24 device
  execution edge.
- L26 has since created the platform/backend bridge submission queue.
- L27 has since created backend executor results and diagnostic D3D9 adapter accounting.
- L28 has since created real HWND/D3D9 device adapter records.
- L29: concrete YuEngine-owned HWND and D3D9 device creation execution before allocation records may
  become real device resources.
