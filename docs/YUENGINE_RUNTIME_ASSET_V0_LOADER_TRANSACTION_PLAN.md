# YuEngine RuntimeAsset v0 Loader Transaction Plan

Status: planning / API-plan
Owner: @琪露诺
Task: #51
Base audit: `4b846aa` (`Preflight RuntimeAsset scene loader before registration`)
Related contract: `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
Related closure plan: `docs/YUENGINE_RUNTIME_ASSET_V0_PRODUCTION_GAP_CLOSURE_PLAN.md`
Format policy and validator vocabulary:
`docs/YUENGINE_RUNTIME_ASSET_V0_FORMAT_POLICY_AND_VALIDATOR_VOCABULARY.md`
Related gate: `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`

## Purpose

Define the production RuntimeAsset v0 loader transaction boundary before any
loader implementation begins.

The target loader must load a RuntimeAsset graph without using filename suffixes
or smoke fixture names as type truth. It must read bytes, validate internal
metadata and typed dependencies, preflight all output and registry writes, and
only then commit Resource, Asset, dependency, decoded-payload, and scene-loader
state.

This task is docs/API-plan only. It does not implement a loader, binary parser,
cook pipeline, editor/import bridge, Preview Host, Game Adapter, old package
parser, RenderScene bridge, RHI texture/shader upload path, or external
authoring workflow.

## RAV1-A Contract Terms

This plan consumes the #50 RAV1-A terms:

- **source artifact**: readable text or manifest-shaped input that normalizes to
  the RuntimeAsset v0 metadata vocabulary before cook;
- **cooked artifact**: runtime-optimized output, preferably binary, that carries
  internal metadata and bounded record/payload/dependency tables.

When this plan says "record", it means either a source artifact record, a cooked
artifact record, or an entry in an approved source/cooked record table. The
loader transaction rules are identical for both artifact classes: the path is a
locator, and internal metadata is authoritative.

## RAV0 Audit At `4b846aa`

The current `RuntimeAssetData.cpp` already proves useful smoke-route floors:

- `LoadRuntimeAssetDataGraph` validates request pointers and capacities before
  reading files.
- Scene and dependency bytes are read through `MountTable`.
- `ValidateRuntimeAssetDataBytes` validates common text metadata and family
  token rules.
- `BuildSceneLoaderStage` stages scene output before caller scene arrays are
  copied.
- `CommitSceneLoaderOutput` copies staged scene refs, cameras, entities, and
  transforms only after scene staging succeeds.

Those floors are not a production transaction. The current graph load still has
these split gaps:

| Current surface | Current behavior | Production split needed |
| --- | --- | --- |
| `RuntimeAssetFileDesc::path` | file path is read as storage locator | path stays locator-only; type truth comes from internal metadata and caller expectation |
| `RuntimeAssetFileDesc::kind` | caller-provided kind drives validation | expected kind becomes a preflight expectation checked against internal header `kind` |
| `ValidateCommonMetadata` | text header/schema/id smoke validator | production header parser validates magic, version, kind, schema, byte order, id, hash, tables |
| `ValidateDependencyRules` | token/prefix smoke dependency checks | production dependency table resolves typed record refs by kind/id/hash, not path or prefix |
| `BuildSceneLoaderStage` | fixed three-mesh scene and one animation path | production scene/animation records are bounded tables with caller-provided capacities |
| `RegisterLoadedFile` | registers Resource, commits upload, admits residency, stores payloads, decodes, and registers Asset in one function | split into immutable commit intents, commit preflight, and a deterministic commit sequence |
| `StoreSourcePayload` / `StoreDecodedPayload` | mutate Resource cache/decode state during per-file registration | plan decoded/source payload writes before commit; commit them only after graph validation passes |
| `AddLoadedDependency` | mutates Resource and Asset dependency edges immediately after each file | collect dependency-edge intents first; commit after all records and edges are preflighted |
| `CommitSceneLoaderOutput` | caller scene output commit is staged, but after Resource/Asset mutation | keep scene output as a final commit step after all registry commits succeed |

The exact production order must be:

```text
read bytes
-> parse header/schema/kind
-> validate dependency table and record tables
-> preflight all records, payloads, capacities, ids, hashes, and edge intents
-> stage scene/animation/material/shader/texture outputs
-> commit Resource records, Asset records, payload/decode records, dependency
   edges, then caller scene output
```

## Suffix-Free Type Truth

RuntimeAsset v0 production code must not infer file family from `.yumesh`,
`.yumat`, `.yutex`, `.yuprogram`, `.yuscene`, `.yuanim`, or any other extension.
Those names remain smoke fixture examples only.

Production type truth is allowed from:

- internal file or record metadata: `magic`, `version`, `kind`, `schema`,
  `byteOrder`, `id`, `sourceHash`, `payloadHash`, `dependencyTable`, and
  `recordTable`;
- caller-provided expected descriptors, used as expectations rather than truth;
- an approved manifest record whose own metadata passes the same validator
  rules.

Production type truth is not allowed from:

- file suffixes or parent directory names;
- smoke fixture display names such as `Mesh/` or `Texture/`;
- dependency string prefixes as a substitute for typed refs;
- test helper construction of RenderScene records.

The loader may still receive a `VirtualPath` or locator per source/cooked record.
That locator is only an input to File/Mount/VFS reads and diagnostics. A record
loaded from `Artifacts/runtime_asset/a.bin` must succeed or fail exactly the same
way as the same bytes loaded from a `.yu*` smoke name.

## Proposed API Surface

The following names are the proposed production API shape. They should land in a
later implementation task only after #50 freezes the contract terminology.

```cpp
enum class RuntimeAssetLoadTransactionPhase {
    Preflight,
    ReadBytes,
    ParseHeader,
    ValidateRecord,
    ValidateDependencies,
    PreflightCommit,
    StageSceneOutput,
    CommitResources,
    CommitAssets,
    CommitPayloads,
    CommitDependencies,
    CommitSceneOutput
};
```

```cpp
struct RuntimeAssetRecordLocator final {
    yuengine::file::VirtualPath path;
    RuntimeAssetFileKind expected_kind;
    std::uint64_t expected_id;
    std::uint64_t expected_source_hash;
};

struct RuntimeAssetLoadTransactionRequest final {
    yuengine::file::MountTable *mount_table;
    yuengine::file::MountId mount;
    const RuntimeAssetRecordLocator *records;
    std::uint32_t record_count;
    RuntimeAssetFileKind root_kind;
    std::uint64_t root_id;
};

struct RuntimeAssetLoadTransactionScratch final {
    std::uint8_t *byte_storage;
    std::uint32_t byte_storage_capacity;
    RuntimeAssetRecordHeader *headers;
    std::uint32_t header_capacity;
    RuntimeAssetDependencyRecord *dependencies;
    std::uint32_t dependency_capacity;
    RuntimeAssetCommitIntent *commit_intents;
    std::uint32_t commit_intent_capacity;
};

struct RuntimeAssetLoadTransactionPlan final {
    RuntimeAssetDataStatus status;
    RuntimeAssetLoadTransactionPhase phase;
    std::uint32_t record_count;
    std::uint32_t dependency_count;
    std::uint32_t resource_commit_count;
    std::uint32_t asset_commit_count;
    std::uint32_t payload_commit_count;
    std::uint32_t dependency_edge_count;
};

struct RuntimeAssetLoadCommitRequest final {
    yuengine::resource::ResourceRegistry *resource_registry;
    yuengine::asset::AssetManager *asset_manager;
    RuntimeAssetLoadedFile *loaded_files;
    std::uint32_t loaded_file_capacity;
    RuntimeAssetSceneLoaderOutput *scene_output;
    RuntimeAssetSceneResourceRef *scene_resource_refs;
    std::uint32_t scene_resource_ref_capacity;
    RuntimeAssetSceneCameraRecord *scene_cameras;
    std::uint32_t scene_camera_capacity;
    RuntimeAssetSceneEntityRecord *scene_entities;
    std::uint32_t scene_entity_capacity;
    RuntimeAssetSceneTransformOutputRecord *scene_transforms;
    std::uint32_t scene_transform_capacity;
};

struct RuntimeAssetLoadTransactionResult final {
    RuntimeAssetDataStatus status;
    RuntimeAssetLoadTransactionPhase phase;
    std::uint32_t first_failed_record_index;
    std::uint32_t first_failed_dependency_index;
    std::uint32_t required_byte_storage;
    std::uint32_t required_header_capacity;
    std::uint32_t required_dependency_capacity;
    std::uint32_t required_commit_intent_capacity;
    std::uint32_t committed_resource_count;
    std::uint32_t committed_asset_count;
    std::uint32_t committed_payload_count;
    std::uint32_t committed_dependency_edge_count;
    bool mutated_state;
};
```

Proposed functions:

```cpp
RuntimeAssetDataStatus BuildRuntimeAssetLoadTransactionPlan(
    const RuntimeAssetLoadTransactionRequest &request,
    RuntimeAssetLoadTransactionScratch *scratch,
    RuntimeAssetLoadTransactionPlan *out_plan,
    RuntimeAssetLoadTransactionResult *out_result);

RuntimeAssetDataStatus PreflightRuntimeAssetLoadCommit(
    const RuntimeAssetLoadTransactionPlan &plan,
    const RuntimeAssetLoadCommitRequest &request,
    RuntimeAssetLoadTransactionResult *out_result);

RuntimeAssetDataStatus CommitRuntimeAssetLoadTransaction(
    const RuntimeAssetLoadTransactionPlan &plan,
    const RuntimeAssetLoadCommitRequest &request,
    RuntimeAssetLoadTransactionResult *out_result);
```

`LoadRuntimeAssetDataGraph` should eventually become either a compatibility
wrapper over this transaction flow or be replaced by a production graph-load API
that uses the same plan/commit split.

## Transaction Phases

### 1. Request Preflight

Validate all null pointers, counts, capacities, root record identity, duplicate
expected ids, scratch spans, and output span capacities that can be known before
I/O.

Failure statuses: `InvalidArgument`, `InvalidCount`, `CapacityExceeded`, or
`DuplicateDependency`.

Mutation rule: diagnostics may be written to `RuntimeAssetLoadTransactionResult`;
caller output spans, `ResourceRegistry`, `AssetManager`, dependency edges, and
scene output must remain unchanged.

### 2. Read Bytes

Read every locator through `MountTable` and `VirtualPath`. The path is recorded
only as a locator/diagnostic value.

Failure statuses: `FileReadFailed`, `BudgetExceeded`, or `CapacityExceeded`.

Mutation rule: scratch byte storage may change; runtime registries and caller
outputs remain unchanged.

### 3. Parse Header And Schema

Parse internal magic, version, kind, schema, byte order, record table, dependency
table, ids, and hashes. Compare internal `kind` and `id` against caller
expectations.

Failure statuses: `InvalidHeader`, `UnsupportedVersion`, `InvalidKind`,
`InvalidSchema`, `InvalidSize`, `InvalidAlignment`, `InvalidBounds`,
`TypeMismatch`, or `HashMismatch`.

Mutation rule: only transaction scratch/header tables may change.

### 4. Validate Records

Run family validators for mesh, material, texture, shader/program, scene, and
animation records. The validators must inspect internal tables and payload
ranges, not suffixes.

Failure statuses use the #41 vocabulary: `InvalidCount`, `InvalidSize`,
`InvalidAlignment`, `InvalidBounds`, `InvalidDependency`, `MissingDependency`,
`DuplicateDependency`, `TypeMismatch`, `HashMismatch`,
`UnsupportedFieldValue`, `CapacityExceeded`, and `BudgetExceeded`.

Mutation rule: no Resource, Asset, decoded payload, scene output, RenderScene,
RenderCore, or RHI mutation.

### 5. Validate Dependencies

Resolve every typed dependency ref against the staged record table by
`(kind, id, hash)` or the #50-approved equivalent. Scene/material/shader/
animation references must not resolve through `Mesh/`, `Texture/`, suffixes, or
display paths.

Failure statuses: `InvalidDependency`, `MissingDependency`,
`DuplicateDependency`, `TypeMismatch`, or `HashMismatch`.

Mutation rule: no runtime mutation; dependency-edge intents only.

### 6. Preflight Commit

Build commit intents for:

- Resource descriptors and upload/residency transitions;
- source cache payloads;
- decode plan/result/decoded payload records;
- Asset descriptors;
- Resource dependency edges;
- Asset dependency edges;
- scene loader output records.

Preflight must catch duplicate ids, insufficient output capacities, known
Resource/Asset budget overflow, payload capacity overflow, decoded payload
capacity overflow, and scene output capacity failures before any commit begins.

Failure statuses: `CapacityExceeded`, `BudgetExceeded`,
`ResourceRegistrationFailed`, `ResourceLoadCommitFailed`,
`CachePayloadStoreFailed`, `DecodePlanFailed`, `DecodeResultFailed`,
`DecodedPayloadStoreFailed`, `AssetRegistrationFailed`,
`ResourceDependencyFailed`, or `AssetDependencyFailed`.

Mutation rule: preflight itself is no-mutation. If an existing lower API cannot
preflight a failure without mutating, the production implementation must not
claim no-mutation for that commit path until a lower preflight or rollback
ledger exists.

### 7. Stage Scene Output

Build scene refs, camera records, entity records, transform records, animation
sample results, and diagnostics into transaction-owned staging. Do not write
caller scene arrays yet.

Failure statuses: validator statuses plus `CapacityExceeded` and
`BudgetExceeded`.

Mutation rule: caller scene arrays and `RuntimeAssetSceneLoaderOutput` remain
unchanged until final commit.

### 8. Commit

Commit in deterministic order:

1. Resource descriptors and upload/residency state.
2. Resource source/cache/decode/decoded-payload records.
3. Asset descriptors.
4. Resource dependency edges.
5. Asset dependency edges.
6. `RuntimeAssetLoadedFile` output records.
7. scene refs/cameras/entities/transforms/output summary.

If commit begins, `RuntimeAssetLoadTransactionResult::mutated_state` must become
true as soon as the first runtime mutation succeeds. A later implementation may
add rollback, but this plan does not assume rollback exists.

Production no-mutation acceptance should therefore target all pre-commit
failures. Commit-phase failures are acceptable only if preflight proves they are
not reachable for known deterministic inputs, or if a later rollback ledger is
approved and tested.

## No-Mutation Contract

The production loader must preserve these rules:

- invalid header/schema/kind/dependency/entity/keyframe/payload failures do not
  write `loaded_files`, scene refs, cameras, entities, transforms, scene output,
  Resource records, Asset records, decoded payloads, dependency edges,
  RenderScene, RenderCore, or RHI;
- only transaction scratch and explicit diagnostics may change before commit;
- successful plan building is still no-mutation against runtime state;
- commit is the first function allowed to mutate Resource/Asset/dependency
  state;
- scene output is the last mutation, after Resource and Asset graph commits;
- result diagnostics must state the phase and first failed record/dependency so
  tests do not infer failure location from side effects.

This preserves the #48 staged-output guarantee and extends it to Resource,
Asset, dependency-edge, and decoded-payload state.

## Required Tests Before Implementation

Before a production implementation can be accepted, add focused tests for these
cases or approved equivalent names:

| Test area | Required proof |
| --- | --- |
| missing schema | internal metadata without `schema` returns `InvalidSchema` with no mutation |
| wrong kind | bytes with internal `kind=Texture` but expected mesh return `InvalidKind` or `TypeMismatch`; suffix must not affect result |
| unsupported version | internal version 2+ returns `UnsupportedVersion` before mutation |
| hash mismatch | declared source/payload/dependency hash mismatch returns `HashMismatch` before mutation |
| duplicate deps | duplicate typed dependency refs return `DuplicateDependency` before mutation |
| missing deps | unresolved typed dependency refs return `MissingDependency` before mutation |
| invalid payload size | zero, out-of-range, or table-overflow payload ranges return `InvalidSize` or `InvalidBounds` |
| invalid alignment | misaligned binary table or payload range returns `InvalidAlignment` |
| invalid scene records | bad entity/camera/material/mesh/texture/shader refs return exact dependency status |
| invalid animation records | bad clip, target, track, keyframe range, interpolation, or sample capacity returns exact status |
| capacity failures | insufficient scratch/output capacities return `CapacityExceeded` before runtime mutation |
| successful path-independent load | same valid bytes load from non-`.yu*` paths and from smoke fixture paths with identical results |
| misleading suffix | `.yutex` path containing internal mesh bytes behaves according to internal kind and expected descriptor, not suffix |
| no-mutation probes | snapshots of ResourceRegistry, AssetManager, dependency counts, decoded payload counts, loaded output spans, and scene output stay unchanged for every pre-commit failure |

Commit-phase tests must either prove preflight catches all reachable failures
before mutation or explicitly assert `mutated_state == true` for a partial
commit failure. They must not call partial commit failure a no-mutation pass.

## Acceptance Commands

For this docs-only plan, required validation is:

```powershell
git diff --check -- docs\YUENGINE_RUNTIME_ASSET_V0_LOADER_TRANSACTION_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_PRODUCTION_GAP_CLOSURE_PLAN.md docs\YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md docs\gates\L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md
git show --check --format=short HEAD
```

For the later implementation slices, acceptance should include:

```powershell
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "^RuntimeAssetData_" --output-on-failure
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate --output-on-failure
git diff --check
git show --check --format=short HEAD
```

## Implementation Slices

The follow-up work should be split so agents can work in parallel after #50
freezes the production contract terms:

| Slice | Work |
| --- | --- |
| RAV1-B1 | Add transaction structs, phase enum, diagnostics, and no-mutation API docs without behavior change |
| RAV1-B2 | Add suffix-free header/record discovery for source/cooked records and path-independent validator tests |
| RAV1-B3 | Build transaction-plan preflight over scratch capacities, ids, hashes, record tables, and dependencies |
| RAV1-B4 | Stage Resource/Asset/payload/dependency commit intents without mutating runtime state |
| RAV1-B5 | Add commit preflight and deterministic commit sequence over Resource, Asset, payload, and dependency edges |
| RAV1-B6 | Route scene/animation output through transaction staging and make scene output the final commit |
| RAV1-B7 | Wrap or replace `LoadRuntimeAssetDataGraph` so legacy smoke usage cannot bypass the transaction contract |

## Coordination Notes

- #50 owns the production RuntimeAsset v0 contract and cook/load/render gate.
  This plan must adopt #50 terminology if that task changes names for records,
  headers, manifests, or gates.
- #52 should consume staged texture/material/shader payload and program outputs;
  it should not depend on paths or suffixes.
- #53 should consume bounded scene/animation records and staged output
  capacities; it should not depend on the current fixed three-entity fixture.
- #54 should lift this test matrix and the implementation acceptance commands
  into the evidence matrix.
- #55 should reject any implementation that validates by suffix, mutates before
  dependency/record preflight completes, or claims no-mutation after a partial
  commit without rollback evidence.

## Exit Criteria

This plan is ready for review when:

1. it identifies the current `RuntimeAssetData.cpp` split at `4b846aa`;
2. it states suffix-free type truth and path-independent load behavior;
3. it proposes concrete transaction/API names and phases;
4. it preserves no-mutation guarantees for invalid metadata, dependency, scene,
   animation, and payload failures;
5. it lists required tests before implementation;
6. it is committed with whitespace validation.
