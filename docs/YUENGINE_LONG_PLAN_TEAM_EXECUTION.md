# YuEngine Long Plan And Team Execution

Status: active execution plan
Owner: Architect, lead engineer
Started: 2026-06-11
Original repository baseline: `fe586d2`
Current coordination checkpoint: `962c1cc`

## 1. Final Goal

Complete the YuEngine restart as a commercial-grade C++20 engine, using
TouhouNewWorld only as the first validation workload.

The execution goal is not to make an early visual demo. The execution goal is to
turn the accepted architecture gates into reviewed, testable engine modules in a
sequence that preserves ownership, performance bounds, and lower-layer
dependency rules.

## 2. Current State

The original execution baseline was clean on `main` at `fe586d2`. The
current coordination checkpoint is `962c1cc`, with P2-GATE-003 first slice
closed and QA cleared at `354f8e2`.

Phase 1 has one implemented gate and several implementation reviews still open:

- P1-GATE-001 is implemented.
- P1-GATE-002 memory accounting is in implementation review.
- P1-GATE-003 thread/task is in implementation review.
- P1-GATE-004 diagnostics channel is in implementation review.
- P1-GATE-005 file primitive/path normalization is in implementation review.
- P1-GATE-006 resource identity/lifetime is in implementation review.
- P1-GATE-007 input replay/action snapshot is in implementation review.

Phase 2 is active but must not expand before review pressure is reduced:

- P2-GATE-001 null RHI is in implementation review.
- P2-GATE-002 audio test backend and mixer is in implementation review.
- P2-GATE-003 package manifest/load plan first slice is closed and QA cleared;
  package expansion remains held without a new Architect decision.

Phase 3 is architecture-first:

- ADR-0014 object identity is accepted.
- P3-GATE-001 object identity/lifetime registry is in review and blocked from
  implementation until performance and PM sequencing are closed.
- ADR-0015 serialization value stream is proposed.
- P3-GATE-002 serialization value stream is proposed and blocked from
  implementation until ADR, performance, implementability, and PM sequencing are
  accepted.

## 3. Architecture Principles

YuEngine follows the layer rule from the restart plan:

```text
L0-L1 platform services
L2 engine kernel
L3 low-level runtime interfaces
L4 core asset and script framework
L5 runtime world systems
L6 game adapter
L7 verification and tools
```

The core rule is downward dependency only. Diagnostics may observe runtime
behavior, but diagnostics, reports, captures, and oracle output must not own
runtime behavior.

UE and Unity are references for responsibility boundaries, not APIs to copy:

- UE Core/CoreUObject informs the separation between object identity, reflection,
  and world ownership.
- UE RHI/RenderCore informs the split between command submission and scene
  rendering.
- UE AudioMixer informs the split between backend, mixer, voice, and resource
  decoding.
- Unity GameObject/scene concepts inform validation questions around instance
  identity and world ownership, without copying Unity API shape.
- Unity SRP informs the separation between low-level graphics commands and
  render pipeline ownership.

## 4. Execution Strategy

The next work is review closure before new implementation.

Order:

1. Close P1 implementation reviews that define vocabulary needed by upper
   gates, especially `YuMemory`, `YuFile`, `YuResource`, diagnostics, and input.
2. Close P2 null RHI and audio mixer implementation reviews without expanding
   into real backends, render scenes, business audio IDs, resources, UI, reports,
   or game adapter behavior.
3. Keep P2-GATE-003 first-slice closure as the package baseline. Do not expand
   package scope unless a new Architect decision approves a later slice.
4. Finish P3-GATE-001 performance and PM sequencing reviews. Do not implement
   `YuObject` until the gate receives explicit `APPROVED_FOR_FIRST_SLICE`.
5. Review ADR-0015 and P3-GATE-002 as architecture only. Do not implement
   `YuSerialize` until ADR-0015, gate, performance, implementability, and PM
   sequencing are accepted.
6. Create implementation slices only from approved gates, preferably in clean
   isolated worktrees when shared CMake or target registration would conflict.

## 5. Workstreams

### PM And Gate Control

AssistantPM owns execution rhythm under Architect direction:

- produce one live gate board with P1, P2, and P3 states;
- identify which reviews block P3 object and serialization work;
- prevent new implementation tasks without explicit gate approval;
- assign sequencing windows so CMake and module-boundary churn is isolated;
- report exact blockers as `NEEDS_ARCHITECTURE`, `NEEDS_PERFORMANCE`,
  `NEEDS_EVIDENCE`, or `BLOCKED`.

### SeniorEngineerA

SeniorEngineerA owns lower-layer review closure:

- audit P1-GATE-002, P1-GATE-003, P1-GATE-004, P1-GATE-005, P1-GATE-006, and
  P1-GATE-007 for scope creep and dependency violations;
- confirm whether Memory/File/Resource vocabulary is stable enough for
  P2-GATE-003 and P3-GATE-001;
- propose gate amendments instead of allowing unclear implementation.

### SeniorEngineerB

SeniorEngineerB owns engine-reference and next-gate architecture support:

- compare P3-GATE-001 against UE CoreUObject responsibility boundaries and Unity
  instance/scene separation;
- compare P3-GATE-002 against UE archive/serialization responsibility and Unity
  serialization persistence boundaries where useful;
- produce only boundary conclusions that affect YuEngine decisions.

### CodeReviewerQA

CodeReviewerQA owns review and verification pressure:

- review active implementation slices for forbidden dependencies, hidden
  allocation, diagnostics-owned behavior, and insufficient tests;
- keep fast-gate verification commands explicit;
- block implementation changes that validate behavior by logs, reports, old
  runtime files, or original-game evidence;
- require review findings to cite file and line references.

### Architect

Architect owns final architecture direction and integration:

- decide gate order and stop conditions;
- merge PM sequencing, engine-reference review, performance review, and code
  review into one execution plan;
- amend ADRs/gates when lower-layer vocabulary changes;
- approve or reject first-slice handoffs;
- keep the project aligned with commercial engine architecture rather than demo
  progress.

## 6. Immediate 72-Hour Plan

Day 1:

- PM builds the gate board and marks all review blockers.
- SeniorEngineerA audits P1 review closures and names blockers that affect
  Memory/File/Resource vocabulary.
- CodeReviewerQA starts review of P2 null RHI and audio mixer implementation
  boundaries.
- SeniorEngineerB prepares object and serialization reference notes from UE and
  Unity responsibility boundaries.

Day 2:

- Architect keeps P2-GATE-003 closed for the approved first slice and decides
  only on any separately proposed package expansion.
- Architect decides whether P3-GATE-001 can move toward
  `APPROVED_FOR_FIRST_SLICE` or must remain `NEEDS_PERFORMANCE` /
  `NEEDS_ARCHITECTURE`.
- CodeReviewerQA reports whether P1/P2 implementation reviews are stable enough
  for new isolated work.

Day 3:

- If gates are approved, assign one clean first-slice implementation task at a
  time.
- If gates are not approved, amend the owning ADR/gate and keep implementation
  blocked.
- Run the strongest available non-interactive verification command after every
  code change:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate
```

## 7. Stop Conditions

Stop and route back to architecture when any of these happen:

- a module depends upward or sideways across the layer boundary;
- original-game evidence shapes lower-layer API design;
- reports, logs, captures, or oracle output become behavior transport;
- implementation starts without `APPROVED_FOR_FIRST_SLICE`;
- hot paths allocate, grow containers, format strings, or perform file IO
  without an accepted gate exception;
- object identity is represented by raw pointers, resource handles, strings, or
  file paths;
- serialization tries to own File, Resource, object construction, reflection,
  scene persistence, tools, saves, or Game Adapter behavior in the first slice;
- a visual demo or UI/title business task is proposed before lower systems pass
  their gates.

## 8. Next Implementation Candidates

No new implementation candidate is authorized by this document alone.

Candidates after review closure:

- `YuObject` first slice from P3-GATE-001, only after explicit approval and PM
  sequencing.
- `YuSerialize` first slice from P3-GATE-002, only after ADR-0015 is accepted
  and the gate is approved.
- P2 package manifest/load plan expansion, only after a new explicit Architect
  decision; the first slice is already closed and QA cleared.

Each candidate must include exact files allowed, exact tests required, exact
verification commands, and explicit non-goals before code begins.
