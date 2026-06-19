# YuEngine UI Stage 2 Invalidation Model

This document records the Stage 2 invalidation rules required by `UI-S2-001` and
the cache counter evidence required by `UI-S2-002`.

## Scope

The first slice stays inside backend-independent `YuUiCore`.

It does not introduce component widgets, atlas ownership, batching policy,
RenderCore, RHI, D3D11, Dear ImGui, project UI runtime, or editor lifecycle
dependencies.

## Invalidation Rules

`UiInvalidationModel` accepts a target node, a dirty change type, and an
invalidation scope.

The supported scopes are:

- `Self`: only the target node is invalidated.
- `Subtree`: the target node and all descendants exported by `UiNodeTree` are
  invalidated in deterministic tree order.

Dirty change types map to rebuild work as follows:

| Change type | Layout cache rebuild | Paint cache rebuild | Notes |
| --- | --- | --- | --- |
| `Layout` | yes | yes | Layout changes also invalidate paint bounds. |
| `PaintOnly` | no | yes | Paint data changes keep layout cache valid. |
| `Transform` | no | yes | Transform changes rebuild hit-test and paint cache. |
| `HitTest` | no | no | Hit-test only change does not rebuild layout or paint. |
| `Text` | no | yes | Text shaping is deferred; first slice treats it as paint dirty. |
| `HoverState` | no | yes | Interaction visual state does not rebuild layout. |
| `ScrollOffset` | no | yes | Scroll moves hit-test and paint cache without layout rebuild. |
| `AtlasPage` | no | yes | Atlas page changes keep layout cache valid. |

## Counter Evidence

`UiInvalidationResult` exposes `UiCacheCounters`.

`layout_rebuild_count` and `paint_rebuild_count` are affected-node counts scaled
by the dirty rule for the requested change. For example, layout-invalidating a
five-node subtree reports five layout rebuilds and five paint rebuilds, while
paint-invalidating one node reports zero layout rebuilds and one paint rebuild.

The focused tests are:

- `UiCore_DirtyTracker_PaintOnlyDoesNotTriggerLayoutRebuild`
- `UiCore_InvalidationModel_SubtreeRulesExposeCacheCounters`
- `UiCore_InvalidationModel_RejectsSmallOutputWithoutMutation`
