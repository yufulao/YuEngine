# YuEngine Animation Editor RAV7 State/Event Playback Workflow

Status: implemented first slice
Task: #99
Owner: Architecture / Implementation

## Scope

This slice extends the RAV6 Animation Editor timeline workflow with bounded
state-preview and event-marker playback feedback.

Inputs are still value records owned by the caller:

- runtime animation clip, track, and keyframe records
- state-preview records that bind a state id to a runtime clip id
- event-marker records with clip id, event id, payload id, and event time
- Preview Host transform feedback

Outputs are caller-owned spans:

- one active state playback row
- event marker rows with timeline visibility and emitted-this-frame state
- existing timeline clip, track, keyframe, preview, and selection feedback rows

## Workflow

```text
state/event workflow command
-> validate state-preview record and bind it to a runtime clip
-> validate event markers for that clip
-> run AnimationEditor timeline workflow against staged output
-> consume Preview Host transform feedback for visible target state
-> calculate event timing window for scrub or playback tick
-> copy staged state, event, timeline, preview, and selection rows
```

`PlaybackTick` applies the state speed multiplier. A looping state wraps the
sample time to the clip duration and reports a wrapped event window when the
tick crosses the clip end.

## Failure Contract

The workflow stages all outputs before writing to caller spans. These failures
do not write partial state, event, timeline, preview, or selection output:

- missing or invalid state-preview records
- missing or invalid event records
- missing or invalid runtime clip/track/keyframe records
- invalid playback time
- missing Preview Host transform feedback
- output capacity failure

The result reports the blocking layer as state records, event records, runtime
animation records, timeline output, runtime sampler, or Preview Host feedback.

## Boundaries

This is not a gameplay finite-state machine, Animator Controller, blend tree,
curve editor, event authoring UI, or native editor shell. It does not open a
window, mutate runtime data, use a browser route, or claim final Animation
Editor completion.

State records are editor playback-preview records only: state id, clip id,
speed, loop flag, and validity. Event rows expose timing feedback only; they do
not dispatch gameplay behavior.

## Tests

Covered by `YuAnimationEditorSurfaceTests`:

- `AnimationEditorStateEventWorkflow_BindsVisiblePlaybackStateAndEvents`
- `AnimationEditorStateEventWorkflow_PlaybackTickLoopsAndEmitsWrappedEvents`
- `AnimationEditorStateEventWorkflow_MissingPreviewFeedbackDoesNotMutateOutputs`
- `AnimationEditorStateEventWorkflow_InvalidEventDoesNotMutateOutputs`
- `AnimationEditorStateEventWorkflow_OutputCapacityFailureDoesNotMutateOutputs`
