# YuEngine External Authoring Bridge RAV7 Contract

Owner: Architecture
Target: RuntimeAsset import/cook and Resource Browser boundary
Status: contract slice
Task: #101

## Purpose

This contract defines how Unity, Unreal, and DCC tools may be used as external
authoring sources for YuEngine without becoming YuEngine runtime or editor
dependencies.

The accepted direction is:

```text
External authoring tool project
-> offline exporter or converter owned outside YuEngine runtime/editor
-> YuExternalAuthoringExport manifest plus source payload files
-> YuEngine RuntimeAsset import/cook command
-> Resource Browser diagnostic and import rows
-> Preview Host and editor workflows from cooked YuEngine data
```

YuEngine remains the source of runtime truth after import. External projects,
asset databases, scene graphs, editor object models, and tool SDK state are
source inputs only.

## Contract Boundary

External authoring bridge inputs must be represented as data files and manifest
records. A future implementation may add a converter CLI or exporter adapter,
but the YuEngine runtime and native editor must be able to build and run without
Unity, Unreal, Blender, Maya, FBX SDK, or glTF SDK linked into runtime/editor
modules.

The importer boundary is:

- external exporters produce source payloads and a manifest;
- YuEngine validates the manifest before mutating Resource, Asset, RuntimeAsset,
  or editor state;
- import/cook translates accepted payloads into YuEngine RuntimeAsset records;
- Resource Browser exposes unsupported or incomplete bridge data as diagnostics;
- Preview Host and editor surfaces consume only YuEngine RuntimeAsset, Resource,
  Asset, World, Animation, UI, and Package records.

The bridge must not copy Unity Scene/GameObject/Component, Unity Animator,
Unreal World/Actor/Component, Unreal Sequencer, Blueprint, UMG, Content Browser,
AssetDatabase, or DCC API shapes into YuEngine public APIs.

## Manifest Shape

A future `YuExternalAuthoringExport` manifest should carry stable data, not
foreign runtime handles:

| Field group | Required meaning |
| --- | --- |
| Export identity | source tool kind, exporter version, project identity, export timestamp or deterministic export id |
| Coordinate policy | unit scale, handedness, up axis, transform bake policy, negative-scale handling |
| Asset entries | stable external id, source path, source kind, target RuntimeAsset family, content hash, dependency list |
| Scene entries | stable node id, parent id, transform, component refs, resource binding refs |
| Material entries | shader intent, texture slots, color-space policy, numeric parameters, unsupported feature diagnostics |
| Animation entries | clip id, target binding id, sample rate, duration, track kind, event markers |
| UI entries | optional source UI document records, style/template/state mapping intent |
| Diagnostics | unsupported source feature records with severity and target blocked layer |

The manifest does not define YuEngine runtime memory layout. It is an import
contract that feeds existing RuntimeAsset validation and cook/load paths.

## Runtime Path

The accepted runtime path after import is:

```text
YuExternalAuthoringExport manifest
-> preflight validation
-> RuntimeAsset source records
-> RuntimeAsset cook outputs
-> File/VFS/Resource/Asset graph load
-> RenderScene/RenderCore/RHI, World, Animation, UI, PreviewHost, Editor
```

No Scene, Animation, UI, Preview Host, Package, or RuntimeApp workflow may read
Unity, Unreal, or DCC project state directly. If an imported asset cannot be
expressed in YuEngine RuntimeAsset records, the import must fail with a blocked
layer and caller-owned diagnostics before committing partial state.

## Resource Browser Boundary

Resource Browser may show external authoring exports as import candidates. The
row must distinguish:

- manifest readable and RuntimeAsset-importable;
- manifest readable but blocked by unsupported source features;
- missing source payload;
- invalid dependency graph;
- coordinate/material/animation/UI mapping gap;
- external SDK or tool unavailable, which is not a runtime/editor blocker.

Selecting a blocked external import row must not mint Resource handles, Asset
records, Preview Host requests, editor scene records, or package artifacts.

## Preview And Editor Boundary

Preview Host and editor surfaces may only preview imported YuEngine data. The
allowed preview path is:

```text
external export
-> YuEngine import/cook
-> loaded YuEngine RuntimeAsset graph
-> Preview Host frame/session/result
-> editor workflow rows
```

The disallowed preview paths are:

- Unity or Unreal rendering embedded in YuEngine editor;
- DCC viewport screenshot as Preview Host proof;
- static screenshot or manual visual claim;
- CPU/GDI oracle as final proof;
- direct fixture structs that bypass File/VFS/RuntimeAsset import/cook.

## Acceptance For A Future Implementation Slice

A future implementation slice must provide:

- a manifest parser with no-mutation invalid-data tests;
- explicit blocked layers for manifest, payload set, converter feature, import
  cook, Resource Browser row, Preview Host readiness, and editor workflow input;
- at least one deterministic source fixture that imports through File/VFS and
  RuntimeAsset validation;
- tests proving unsupported Unity, Unreal, or DCC features remain diagnostics
  instead of partial Resource/Asset/editor mutation;
- documentation of which external tool features are accepted, mapped, or
  rejected.

The future slice must not require external tools to be installed for the normal
`windows-fast-gate` path. Tool-specific integration tests, if added, must be
separate opt-in evidence.

## RAV8 Implementation Slice

The first implementation slice is
`docs/YUENGINE_EXTERNAL_AUTHORING_BRIDGE_RAV8_MANIFEST_IMPORT_FIXTURE.md`.
It adds a deterministic manifest parser, File/VFS payload checks, mapping and
dependency validation, no-mutation rejection tests, and RuntimeAsset
import/cook command input emission without Unity, Unreal, DCC, live sync, or
external viewport dependencies.

## Non-Goals

This contract does not implement:

- Unity Editor plugin code;
- Unreal Editor plugin code;
- Blender, Maya, FBX, glTF, or other DCC parser code;
- original TouhouNewWorld package parser compatibility;
- live sync from external editors into YuEngine runtime;
- shared memory, RPC, or embedded external editor viewport;
- YuEngine AssetManager mutation from external manifest rows;
- production material graph conversion;
- final product visual closure.

## RAV7 Gate Use

#102 should treat this document as the #101 artifact. The acceptable RAV7 claim
is that the external authoring bridge has a reviewed importer boundary contract.
The unacceptable claim is that Unity, Unreal, or DCC import is implemented, or
that YuEngine can depend on those tools for runtime/editor correctness.
