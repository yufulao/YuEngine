# ADR-0013: Package Manifest And Load Plan Boundary

Status: Proposed
Owner: 八云紫
Reviewers: 红美铃, 八云蓝, 博丽灵梦, 大妖精, 射命丸文, 雾雨魔理沙 when implementation exists
Date: 2026-06-10
Depends on: ADR-0002, ADR-0005, ADR-0006, ADR-0008, ADR-0009, docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md, docs/YUENGINE_PHASE0_SUBSYSTEM_REFERENCE_MAP.md

## Context

YuEngine needs a package/load boundary before renderer, audio, script, scene,
UI, or Game Adapter code can ask for assets by package facts. Phase 1 separated
generic File/VFS behavior from Resource identity. Phase 2 can now define the
next boundary without parsing TouhouNewWorld package formats or loading real
asset bytes.

The first package/load decision must prove manifest identity, entry lookup,
resource-key association, dependency load-plan ordering, explicit failure
statuses, and bounded cost. It must not turn original `.pak` / `.dat` files into
runtime API shape, and it must not hide package parsing inside Resource or File.

## Decision

YuEngine introduces `YuPackage` as the owner of:

- package manifest identity vocabulary;
- bounded package entry metadata;
- package entry to `(ResourceTypeId, ResourceLogicalKey)` association;
- deterministic load-plan records for resource entries and dependencies;
- explicit package/load status values;
- package/load snapshots and counters for tests.

The first package slice is a synthetic manifest and load-plan boundary only. It
does not read package files, parse original package formats, decode assets,
allocate Resource objects, read File bytes, schedule async jobs, or upload data
to RHI/Audio.

## Manifest Identity

A package manifest is not an original-game package file.

First-slice manifest identity contains:

- a fixed package ID;
- bounded package-local entry IDs;
- bounded logical source keys for generic fixtures;
- a resource type and logical key for each entry;
- declared entry byte range metadata;
- declared direct entry dependencies.

Rules:

- manifest descriptors are synthetic test inputs;
- package entry keys are case-sensitive bounded lowercase ASCII fixture values;
- duplicate `(package ID, entry key)` registration fails explicitly;
- duplicate `(ResourceTypeId, ResourceLogicalKey)` within one manifest fails
  explicitly;
- invalid package IDs, entry IDs, resource types, or logical keys fail
  explicitly and do not mutate manifest state;
- entry lookup uses stable IDs and bounded key comparison, not pointer identity;
- package metadata is setup/load data, not a frame-path string lookup surface.

## Load Plan

A load plan is deterministic metadata. It is not loaded asset content.

The first slice may create load-plan records that contain:

- package ID;
- entry ID;
- resource type and logical key;
- declared byte offset and byte size;
- dependency entry IDs in deterministic declaration order.

Rules:

- resolving a missing entry returns explicit not-found status;
- resolving with an expected type mismatch returns explicit type-mismatch status
  and does not mutate counters;
- entry byte offset and size are metadata only and must stay within the gate
  bounds;
- load-plan record capacity overflow returns explicit capacity status and does
  not mutate the accepted plan counters;
- dependency references must point at registered entries in the same manifest;
- dependency cycles fail explicitly;
- dependency load plans preserve declaration order after validation;
- disabled diagnostics/logging does not change package or load-plan results.

The first slice must not call a decoder, allocate an asset object, or read bytes
from File/VFS. Future gates may connect a validated load plan to `YuFile` byte
reads and `YuResource` registration only after the involved implementations are
accepted.

## File And Resource Boundary

`YuPackage` sits between File/VFS facts and Resource identity, but the first
slice keeps the dependency narrow.

Allowed first-slice relationships:

- use Resource public value vocabulary, such as resource type and logical key;
- use File/VFS path terminology only as bounded source-key metadata;
- create deterministic load plans that later File/Resource gates can consume.

Blocked in this ADR:

- direct `YuFile` reads;
- direct package file IO;
- package parser for `.pak`, `.dat`, `.rpack`, or any original format;
- Resource registry mutation or acquire/release/retire behavior;
- File mount resolution on the hot path;
- loose original resource lookup.

P2-GATE-003 may not be approved until task #21 `YuFile` and task #33
`YuResource` implementation code/semantic review closures are stable enough for
their public value vocabulary to be used as dependencies.

## Memory And Thread Boundary

Memory:

- manifest, entry, dependency, and load-plan storage is bounded;
- setup/load registration may initialize fixed-capacity storage;
- load-plan resolve must not allocate or grow storage;
- if `YuMemory` vocabulary is accepted, package accounting signals use it;
- if `YuMemory` receives a blocking vocabulary rewrite, P2-GATE-003 must amend
  before implementation.

Thread:

- P2-GATE-003 is synchronous and caller-driven;
- no background loader, worker queue, async IO, streaming thread, lock-free
  package index, or callback execution is introduced.

Future async package loading requires accepted Thread/File/Resource semantics
and a separate gate.

## Diagnostics Boundary

Diagnostics may observe counters later. Diagnostics must not own package/load
behavior.

Rules:

- Package and load-plan results are explicit statuses.
- Tests observe statuses, snapshots, and planned metadata, not log text.
- Disabled diagnostics/logging does not change results.
- Reports, captures, or oracle records are not package runtime APIs.

## Evidence Boundary

TouhouNewWorld package names, loose resources, shaders, DB files, old backup
runtime files, and old reports are future validation evidence only. They must
not define P2-GATE-003 APIs or fast tests.

P2-GATE-003 fast tests must not read:

- `C:\Steam\steamapps\common\TouhouNewWorld\resource`;
- original `.pak`, `.dat`, `.rpack`, JSON, shader, DB, or media files;
- old backup runtime files;
- old report/status JSON or markdown files.

Original package facts can enter only after an evidence-approved package/parser
gate defines the owning interface and fixture boundary.

## P2-GATE-003 Compatibility

P2-GATE-003 may implement:

- `YuPackage` target;
- package manifest value types;
- bounded manifest registry;
- bounded package entry table;
- deterministic load-plan creation;
- dependency validation for package entries;
- package/load status values;
- deterministic package snapshots and counters;
- tests with synthetic manifest descriptors only.

P2-GATE-003 must not implement:

- original package parser;
- File/VFS reads;
- Resource registry mutation;
- asset decoder;
- cache eviction or hot reload;
- async load, streaming, or worker scheduling;
- RHI/audio upload scheduling;
- script, scene, UI, gameplay, or Game Adapter behavior;
- report, capture, oracle, dashboard, or tool output.

## Consequences

YuEngine gains a package/load plan boundary that can later connect File byte
sources to Resource identity without making original package formats the first
runtime API.

The cost is that no real package file is loaded yet. That is intentional. Real
package parsing, byte reads, decompression, and adapter-specific resource lookup
need later gates after this manifest/load-plan contract is accepted.

## Requested Review

Reviewers should decide whether this ADR is accepted, rejected, or needs
amendment.

If accepted, ADR-0013 becomes the architecture input for P2-GATE-003 Package
Manifest And Load Plan Boundary.
