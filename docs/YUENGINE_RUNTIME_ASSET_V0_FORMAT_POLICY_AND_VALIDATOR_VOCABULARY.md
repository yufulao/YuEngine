# YuEngine RuntimeAsset v0 Format Policy And Validator Vocabulary

Status: accepted vocabulary; used by RAV1 production contract gate
Owner: Architecture
Task: #41 baseline; #50 RAV1 contract consumer
Related closure plan: `docs/YUENGINE_RUNTIME_ASSET_V0_PRODUCTION_GAP_CLOSURE_PLAN.md`

## Purpose

This document freezes the common RuntimeAsset v0 format policy and validator
vocabulary before the RAV0 implementation tasks fan out across mesh, material,
texture, shader/program, scene, camera, animation, payload bridge, and scene
loader work.

It is below editor, Resource Browser, import UI, Preview Host, Game Adapter, old
package parser, and external authoring bridge scope.

## Format Policy

### Smoke Fixture Names

The current `.yumesh`, `.yumat`, `.yutex`, `.yuprogram`, `.yuscene`, and
`.yuanim` names are smoke-fixture names only. They are allowed in tests and
documentation as current fixture examples, but they are not production naming
policy.

RuntimeAsset v0 production code must not infer file type from a `.yu*` suffix or
from any other extension.

### Source / Authoring Side

Source-side data should optimize for single-team authoring and review:

- AI- and human-readable schema-shaped text is preferred when it helps iteration;
- manifest files are acceptable for grouping related source assets;
- type identity comes from internal metadata such as `kind`, `version`, `schema`,
  record tables, and typed dependency refs;
- validators must reject missing or mismatched internal metadata without trusting
  path names.

This policy does not authorize editor UI, import UI, hot reload, or external DCC
bridge implementation.

### Runtime / Cook / Export Side

Cooked runtime data should optimize for fast YuEngine loading and validation:

- binary output is preferred for runtime/cook/export artifacts;
- binary files must carry internal magic, version, kind, hash, dependency, and
  table metadata;
- runtime loaders validate internal metadata before mutation;
- records must be deterministic under repeated cook from identical source input.

The runtime format should serve YuEngine runtime performance and data cleanliness,
not external ecosystem compatibility, plugin marketplace conventions, or
UE/Unity/commercial-engine asset database shape.

## Shared Metadata Vocabulary

Every RuntimeAsset v0 source or cooked record family must define:

| Field | Meaning |
| --- | --- |
| `magic` | internal file/record marker, not filename suffix |
| `version` | format version and minimum compatible version |
| `kind` | internal family identity: mesh, material, texture, shader/program, scene, camera, animation, or approved extension |
| `schema` | source-side schema identifier or cooked-table layout identifier |
| `byteOrder` | explicit byte order for binary data |
| `sourceHash` | deterministic hash over source bytes or manifest input |
| `payloadHash` | deterministic hash over payload bytes referenced by the record |
| `dependencyTable` | typed refs to other RuntimeAsset records |
| `recordTable` | bounded table of records and payload ranges |
| `coordinateSpec` | units, handedness, transform order, winding, UV origin where relevant |

Text source data may spell these fields differently only if the validator maps
them into the same internal vocabulary.

## Shared Validator Status Vocabulary

RuntimeAsset validators use `RuntimeAssetDataStatus` as the common status surface.
The v0 vocabulary is:

| Status | Use |
| --- | --- |
| `InvalidArgument` | caller passed null pointers, empty spans, invalid capacities, or invalid API arguments |
| `InvalidHeader` | magic/header shape is absent or malformed |
| `UnsupportedVersion` | internal version is known but unsupported |
| `InvalidKind` | internal `kind` is missing, unknown, or does not match the expected family |
| `InvalidSchema` | internal `schema` is missing, unknown, or incompatible |
| `InvalidCount` | record/table/dependency count is zero or outside the family bound |
| `InvalidSize` | byte count, payload range, or file size is invalid |
| `InvalidAlignment` | binary table or payload offset violates alignment rules |
| `InvalidBounds` | numeric geometry, extent, range, coordinate, or transform bounds are invalid |
| `InvalidDependency` | dependency table shape or typed ref syntax is invalid |
| `MissingDependency` | a required typed dependency is absent |
| `DuplicateDependency` | the dependency table contains a duplicate where uniqueness is required |
| `TypeMismatch` | dependency or expected family has the wrong internal type |
| `HashMismatch` | source, payload, or dependency hash does not match bytes read |
| `UnsupportedFieldValue` | field is syntactically valid but unsupported in v0 |
| `CapacityExceeded` | caller-provided output span/table capacity is too small |
| `BudgetExceeded` | operation would exceed an approved runtime/cook budget |

Existing integration statuses such as `FileReadFailed`,
`ResourceRegistrationFailed`, `CachePayloadStoreFailed`, and dependency-edge
failures remain valid after the validator phase.

## Required No-Mutation Rule

Every validator failure above must be no-mutation:

- no Resource registration;
- no Asset registration;
- no decoded payload store;
- no RenderScene output records;
- no RenderCore/RHI object creation;
- no partial scene loader output.

Cook/load may mutate only after all relevant family validators and dependency
checks have succeeded.

## Implementation-Prep API Notes

RAV0-0 adds the shared `RuntimeAssetDataStatus` values needed by downstream
tasks. It does not require immediate validator implementation for every status.
Each RAV0 implementation task must add focused tests that make the relevant
status observable for its family.

Downstream tasks should not introduce parallel status enums for mesh, material,
texture, shader/program, scene, camera, or animation validators unless a later
architecture task explicitly replaces this vocabulary.

## RAV1 Contract Usage

The RAV1 production contract and cook/load/render gate consume this document
without replacing it:

- source artifacts and cooked artifacts both identify family by internal
  `magic`/`version`/`kind`/`schema` metadata, never by filename suffix;
- source artifacts may be readable text or manifests, while cooked artifacts
  should be runtime-optimized binary unless a later gate approves a narrower
  exception;
- family validators use the shared status vocabulary above for format,
  dependency, hash, capacity, and budget failures;
- File, Resource, Asset, decoded payload, input-layout, and RHI statuses remain
  integration statuses after validator/cook preflight, not substitutes for
  validator failures;
- no-mutation failure semantics apply to staged scene loader output and RHI
  shader/pipeline or texture-object creation as well as Resource and Asset
  registration.
