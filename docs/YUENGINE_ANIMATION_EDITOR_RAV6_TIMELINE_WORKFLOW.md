# YuEngine Animation Editor RAV6 Timeline Workflow

Status: implemented first slice
Task: #92
Owner: Architecture / Implementation

## Scope

This slice adds the first visible timeline workflow data contract after the
Preview Host feedback route. It extends the RAV5 timeline surface with explicit
scrub and playback-tick commands, selected track/keyframe feedback, and a
caller-owned no-partial-output rule.

Inputs remain bounded value records:

- `AnimationRuntimeClipRecord`, `AnimationRuntimeTrackRecord`, and
  `AnimationRuntimeKeyframeRecord`
- `RuntimeFrameContext`
- `PreviewHostTransformFeedback`

Outputs remain caller-owned spans:

- clip rows
- track rows with selected/sample state
- keyframe markers with selected/cursor-relative state
- Preview Host feedback records
- one selected track/keyframe feedback record

## Workflow

```text
workflow command
-> resolve target sample time
-> calculate clip_start_time for RuntimeFrameContext
-> BuildAnimationEditorTimelineSurface against staged output
-> select track and keyframe rows from staged timeline data
-> join selected track feedback to PreviewHostTransformFeedback
-> copy staged output to caller spans only after all checks pass
```

`Scrub` uses `requested_sample_time_seconds`.

`PlaybackTick` advances `current_sample_time_seconds` by
`RuntimeFrameContext::delta_time_nanoseconds`.

The runtime sampler still owns clip/track/keyframe validation and channel
sampling. The workflow layer only resolves the command time and visible editor
feedback state.

## Failure Contract

The workflow stages all timeline, preview, and selection records before writing
to caller output. These failures do not write partial output:

- missing clip, track, or keyframe
- invalid time
- unsupported interpolation or channel
- output capacity failure
- missing Preview Host transform feedback

The result reports the blocking layer as runtime records, timeline output,
runtime sampler, or Preview Host feedback.

## Boundaries

This is not the full Animation Editor. It does not implement a curve editor,
state machine, event authoring UI, native shell, or rendering ownership. The
workflow consumes Preview Host feedback as input and returns value records for
the next editor layer to present.

## Tests

Covered by `YuAnimationEditorSurfaceTests`:

- `AnimationEditorWorkflow_ScrubUpdatesSampleTimeAndSelectedFeedback`
- `AnimationEditorWorkflow_PlaybackTickAdvancesSampleTimeAndTrackRowState`
- `AnimationEditorWorkflow_EmitsSelectedTrackAndKeyFeedback`
- `AnimationEditorWorkflow_MissingPreviewFeedbackDoesNotMutateOutputs`
- `AnimationEditorWorkflow_OutputCapacityFailureDoesNotMutateOutputs`
