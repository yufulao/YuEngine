# YuEngine Animation Editor RAV5 Timeline Preview Feedback

Status: implemented first slice
Task: #86
Owner: Architecture / Implementation

## Scope

This slice adds the Animation Editor data surface needed to expose runtime clip,
track, and keyframe records as timeline rows while consuming Preview Host
transform feedback for scrub/sample status.

It is a data/API slice only:

- runtime animation records remain the source of truth
- timeline clip/track/keyframe rows are caller-owned output
- sampled values come from `AnimationRuntimeSampler`
- preview feedback is read from `PreviewHostTransformFeedback`
- failed validation does not mutate caller output

## Runtime Path

```text
AnimationRuntimeClipRecord / TrackRecord / KeyframeRecord
-> AnimationRuntimeSampler sample at frame context time
-> AnimationEditor timeline rows and keyframe markers
-> PreviewHostTransformFeedback match by WorldObjectId
-> AnimationEditor preview feedback records
```

The surface does not create a native editor shell. It also does not open a
window, draw a timeline, mutate runtime data, use rejected editor route/static form /canvas, or own
RuntimeAsset/ResourceBrowser/RenderScene/RHI behavior.

## Acceptance

The accepted behavior is:

- valid clip/track/keyframe records build bounded timeline rows
- scrub time is resolved through `RuntimeFrameContext`
- Preview Host transform feedback is required when requested
- missing Preview Host feedback rejects without writing partial timeline output
- boundary flags remain false for runtime mutation, native window, and rejected editor route

This is not Animation Editor completion. Remaining work includes playback
controls, event markers, state preview, visible target workflow, Resource
Browser/import settings integration, package/run validation, and full review.
