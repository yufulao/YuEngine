// Module: Tests AnimationEditor
// File: Tests/AnimationEditor/AnimationEditorSurfaceTests.cpp

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/AnimationEditor/AnimationEditorSurface.h"
#include "YuEngine/Kernel/RuntimeFrameContext.h"
#include "YuEngine/Kernel/RuntimeFrameMode.h"
#include "YuEngine/PreviewHost/PreviewHost.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

using yuengine::animation::AnimationRuntimeChannel;
using yuengine::animation::AnimationRuntimeClipRecord;
using yuengine::animation::AnimationRuntimeInterpolation;
using yuengine::animation::AnimationRuntimeKeyframeRecord;
using yuengine::animation::AnimationRuntimeTrackRecord;
using yuengine::animationeditor::AnimationEditorPreviewFeedbackRecord;
using yuengine::animationeditor::AnimationEditorEventMarkerRecord;
using yuengine::animationeditor::AnimationEditorEventMarkerRow;
using yuengine::animationeditor::AnimationEditorSurfaceBlockedLayer;
using yuengine::animationeditor::AnimationEditorSurfaceStatus;
using yuengine::animationeditor::AnimationEditorStateEventPlaybackWorkflowRequest;
using yuengine::animationeditor::AnimationEditorStateEventPlaybackWorkflowResult;
using yuengine::animationeditor::AnimationEditorTimelineClipRow;
using yuengine::animationeditor::AnimationEditorTimelineKeyframeMarker;
using yuengine::animationeditor::AnimationEditorTimelineSelectionFeedbackRecord;
using yuengine::animationeditor::AnimationEditorTimelineSurfaceRequest;
using yuengine::animationeditor::AnimationEditorTimelineSurfaceResult;
using yuengine::animationeditor::AnimationEditorTimelineTrackRow;
using yuengine::animationeditor::AnimationEditorTimelineWorkflowCommand;
using yuengine::animationeditor::AnimationEditorTimelineWorkflowRequest;
using yuengine::animationeditor::AnimationEditorTimelineWorkflowResult;
using yuengine::animationeditor::AnimationEditorStatePlaybackRow;
using yuengine::animationeditor::AnimationEditorStatePreviewRecord;
using yuengine::animationeditor::BuildAnimationEditorStateEventPlaybackWorkflow;
using yuengine::animationeditor::BuildAnimationEditorTimelineSurface;
using yuengine::animationeditor::BuildAnimationEditorTimelineWorkflow;
using yuengine::kernel::RuntimeFrameContext;
using yuengine::kernel::RuntimeFrameMode;
using yuengine::previewhost::PreviewHostTransformFeedback;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldTransformState;

namespace {
constexpr const char *TEST_TIMELINE =
    "AnimationEditorSurface_BuildsClipTrackTimelineFromRuntimeRecords";
constexpr const char *TEST_PREVIEW =
    "AnimationEditorSurface_ConsumesPreviewHostTransformFeedbackForScrub";
constexpr const char *TEST_NO_MUTATION =
    "AnimationEditorSurface_RejectsMissingPreviewFeedbackWithoutMutation";
constexpr const char *TEST_WORKFLOW_SCRUB =
    "AnimationEditorWorkflow_ScrubUpdatesSampleTimeAndSelectedFeedback";
constexpr const char *TEST_WORKFLOW_PLAYBACK =
    "AnimationEditorWorkflow_PlaybackTickAdvancesSampleTimeAndTrackRowState";
constexpr const char *TEST_WORKFLOW_SELECTION =
    "AnimationEditorWorkflow_EmitsSelectedTrackAndKeyFeedback";
constexpr const char *TEST_WORKFLOW_MISSING_PREVIEW =
    "AnimationEditorWorkflow_MissingPreviewFeedbackDoesNotMutateOutputs";
constexpr const char *TEST_WORKFLOW_SMALL_OUTPUT =
    "AnimationEditorWorkflow_OutputCapacityFailureDoesNotMutateOutputs";
constexpr const char *TEST_STATE_EVENT_VISIBLE =
    "AnimationEditorStateEventWorkflow_BindsVisiblePlaybackStateAndEvents";
constexpr const char *TEST_STATE_EVENT_LOOP =
    "AnimationEditorStateEventWorkflow_PlaybackTickLoopsAndEmitsWrappedEvents";
constexpr const char *TEST_STATE_EVENT_MISSING_PREVIEW =
    "AnimationEditorStateEventWorkflow_MissingPreviewFeedbackDoesNotMutateOutputs";
constexpr const char *TEST_STATE_EVENT_INVALID_EVENT =
    "AnimationEditorStateEventWorkflow_InvalidEventDoesNotMutateOutputs";
constexpr const char *TEST_STATE_EVENT_SMALL_OUTPUT =
    "AnimationEditorStateEventWorkflow_OutputCapacityFailureDoesNotMutateOutputs";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t CLIP_ID = 4101U;
constexpr std::uint32_t TRACK_ID = 4201U;
constexpr std::uint32_t STATE_ID = 4401U;
constexpr std::uint32_t EVENT_ID = 4501U;
constexpr std::uint32_t WORLD_OBJECT_VALUE = 4301U;
constexpr std::uint64_t HALF_SECOND_NANOSECONDS = 500000000ULL;
constexpr std::uint64_t QUARTER_SECOND_NANOSECONDS = 250000000ULL;
constexpr std::uint64_t ONE_SECOND_NANOSECONDS = 1000000000ULL;
constexpr float TOLERANCE = 0.0001F;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool Approx(float left, float right) {
    const float delta = std::fabs(left - right);
    return delta <= TOLERANCE;
}

RuntimeFrameContext FrameContextAt(
    std::uint64_t fixed_time_nanoseconds,
    std::uint64_t delta_time_nanoseconds=HALF_SECOND_NANOSECONDS) {
    RuntimeFrameContext context{};
    context.frame_index = 11U;
    context.delta_time_nanoseconds = delta_time_nanoseconds;
    context.fixed_time_nanoseconds = fixed_time_nanoseconds;
    context.frame_mode = RuntimeFrameMode::Fixed;
    return context;
}

RuntimeFrameContext FrameContext() {
    return FrameContextAt(HALF_SECOND_NANOSECONDS);
}

WorldTransformState TransformState(float translation_x, float rotation_y) {
    WorldTransformState transform{};
    transform.translation_x = translation_x;
    transform.translation_y = 0.0F;
    transform.translation_z = 0.0F;
    transform.rotation_x = 0.0F;
    transform.rotation_y = rotation_y;
    transform.rotation_z = 0.0F;
    transform.rotation_w = 1.0F;
    transform.scale_x = 1.0F;
    transform.scale_y = 1.0F;
    transform.scale_z = 1.0F;
    return transform;
}

AnimationRuntimeClipRecord ClipRecord(std::size_t track_count=2U) {
    AnimationRuntimeClipRecord clip{};
    clip.clip_id = CLIP_ID;
    clip.duration_seconds = 1.0F;
    clip.first_track_index = 0U;
    clip.track_count = track_count;
    clip.layer_count = 1U;
    clip.is_valid = true;
    return clip;
}

AnimationRuntimeTrackRecord TrackRecord(
    AnimationRuntimeChannel channel,
    std::size_t first_keyframe_index,
    std::size_t keyframe_count=2U,
    AnimationRuntimeInterpolation interpolation=AnimationRuntimeInterpolation::Linear) {
    AnimationRuntimeTrackRecord track{};
    track.track_id = TRACK_ID + static_cast<std::uint32_t>(first_keyframe_index);
    track.target = WorldObjectId{WORLD_OBJECT_VALUE};
    track.channel = channel;
    track.interpolation = interpolation;
    track.first_keyframe_index = first_keyframe_index;
    track.keyframe_count = keyframe_count;
    track.is_valid = true;
    return track;
}

AnimationRuntimeKeyframeRecord Keyframe(float time_seconds, float value) {
    AnimationRuntimeKeyframeRecord keyframe{};
    keyframe.time_seconds = time_seconds;
    keyframe.value = value;
    keyframe.is_valid = true;
    return keyframe;
}

PreviewHostTransformFeedback PreviewFeedback() {
    PreviewHostTransformFeedback feedback{};
    feedback.world_object_id = WorldObjectId{WORLD_OBJECT_VALUE};
    feedback.transform = TransformState(5.0F, 0.5F);
    feedback.transform_available = true;
    return feedback;
}

AnimationEditorTimelineSurfaceRequest SurfaceRequest(
    std::span<const AnimationRuntimeClipRecord> clips,
    std::span<const AnimationRuntimeTrackRecord> tracks,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes,
    std::span<const PreviewHostTransformFeedback> preview_feedback,
    std::span<AnimationEditorTimelineClipRow> clip_rows,
    std::span<AnimationEditorTimelineTrackRow> track_rows,
    std::span<AnimationEditorTimelineKeyframeMarker> keyframe_markers,
    std::span<AnimationEditorPreviewFeedbackRecord> preview_output,
    bool require_preview_feedback=true) {
    AnimationEditorTimelineSurfaceRequest request{};
    request.clip_id = CLIP_ID;
    request.clips = clips;
    request.tracks = tracks;
    request.keyframes = keyframes;
    request.frame_context = FrameContext();
    request.preview_transform_feedback = preview_feedback;
    request.require_preview_feedback = require_preview_feedback;
    request.clip_rows = clip_rows;
    request.track_rows = track_rows;
    request.keyframe_markers = keyframe_markers;
    request.preview_feedback_output = preview_output;
    return request;
}

AnimationEditorTimelineWorkflowRequest WorkflowRequest(
    std::span<const AnimationRuntimeClipRecord> clips,
    std::span<const AnimationRuntimeTrackRecord> tracks,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes,
    std::span<const PreviewHostTransformFeedback> preview_feedback,
    std::span<AnimationEditorTimelineClipRow> clip_rows,
    std::span<AnimationEditorTimelineTrackRow> track_rows,
    std::span<AnimationEditorTimelineKeyframeMarker> keyframe_markers,
    std::span<AnimationEditorPreviewFeedbackRecord> preview_output,
    std::span<AnimationEditorTimelineSelectionFeedbackRecord> selection_output,
    RuntimeFrameContext frame_context,
    AnimationEditorTimelineWorkflowCommand command,
    float current_sample_time_seconds,
    float requested_sample_time_seconds,
    std::uint32_t selected_track_id,
    std::size_t selected_keyframe_index) {
    AnimationEditorTimelineWorkflowRequest request{};
    request.command = command;
    request.clip_id = CLIP_ID;
    request.clips = clips;
    request.tracks = tracks;
    request.keyframes = keyframes;
    request.frame_context = frame_context;
    request.current_sample_time_seconds = current_sample_time_seconds;
    request.requested_sample_time_seconds = requested_sample_time_seconds;
    request.selected_track_id = selected_track_id;
    request.selected_keyframe_index = selected_keyframe_index;
    request.preview_transform_feedback = preview_feedback;
    request.clip_rows = clip_rows;
    request.track_rows = track_rows;
    request.keyframe_markers = keyframe_markers;
    request.preview_feedback_output = preview_output;
    request.selection_feedback_output = selection_output;
    return request;
}

AnimationEditorStatePreviewRecord StateRecord(
    bool loop_playback=false,
    float speed_multiplier=1.0F) {
    AnimationEditorStatePreviewRecord state{};
    state.state_id = STATE_ID;
    state.clip_id = CLIP_ID;
    state.speed_multiplier = speed_multiplier;
    state.loop_playback = loop_playback;
    state.is_valid = true;
    return state;
}

AnimationEditorEventMarkerRecord EventMarker(
    float time_seconds,
    std::uint32_t offset=0U) {
    AnimationEditorEventMarkerRecord event_marker{};
    event_marker.event_id = EVENT_ID + offset;
    event_marker.clip_id = CLIP_ID;
    event_marker.payload_id = 8001U + offset;
    event_marker.time_seconds = time_seconds;
    event_marker.is_valid = true;
    return event_marker;
}

AnimationEditorStateEventPlaybackWorkflowRequest StateEventWorkflowRequest(
    std::span<const AnimationEditorStatePreviewRecord> states,
    std::span<const AnimationEditorEventMarkerRecord> event_markers,
    std::span<const AnimationRuntimeClipRecord> clips,
    std::span<const AnimationRuntimeTrackRecord> tracks,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes,
    std::span<const PreviewHostTransformFeedback> preview_feedback,
    std::span<AnimationEditorStatePlaybackRow> state_rows,
    std::span<AnimationEditorEventMarkerRow> event_rows,
    std::span<AnimationEditorTimelineClipRow> clip_rows,
    std::span<AnimationEditorTimelineTrackRow> track_rows,
    std::span<AnimationEditorTimelineKeyframeMarker> keyframe_markers,
    std::span<AnimationEditorPreviewFeedbackRecord> preview_output,
    std::span<AnimationEditorTimelineSelectionFeedbackRecord> selection_output,
    RuntimeFrameContext frame_context,
    AnimationEditorTimelineWorkflowCommand command,
    float current_sample_time_seconds,
    float requested_sample_time_seconds,
    std::uint32_t selected_track_id,
    std::size_t selected_keyframe_index) {
    AnimationEditorStateEventPlaybackWorkflowRequest request{};
    request.command = command;
    request.state_id = STATE_ID;
    request.states = states;
    request.event_markers = event_markers;
    request.clips = clips;
    request.tracks = tracks;
    request.keyframes = keyframes;
    request.frame_context = frame_context;
    request.current_sample_time_seconds = current_sample_time_seconds;
    request.requested_sample_time_seconds = requested_sample_time_seconds;
    request.selected_track_id = selected_track_id;
    request.selected_keyframe_index = selected_keyframe_index;
    request.preview_transform_feedback = preview_feedback;
    request.state_rows = state_rows;
    request.event_rows = event_rows;
    request.clip_rows = clip_rows;
    request.track_rows = track_rows;
    request.keyframe_markers = keyframe_markers;
    request.preview_feedback_output = preview_output;
    request.selection_feedback_output = selection_output;
    return request;
}

bool SentinelClipUnchanged(const AnimationEditorTimelineClipRow &row) {
    return row.clip_id == 9001U && row.track_count == 77U && !row.selected;
}

bool SentinelTrackUnchanged(const AnimationEditorTimelineTrackRow &row) {
    return row.track_id == 9002U && row.keyframe_count == 88U && !row.active;
}

bool SentinelKeyframeUnchanged(const AnimationEditorTimelineKeyframeMarker &marker) {
    return marker.track_id == 9003U && marker.keyframe_index == 99U && !marker.valid;
}

bool SentinelPreviewUnchanged(const AnimationEditorPreviewFeedbackRecord &record) {
    return record.track_id == 9004U && !record.matched_preview_host_feedback;
}

bool SentinelSelectionUnchanged(
    const AnimationEditorTimelineSelectionFeedbackRecord &record) {
    return record.track_id == 9005U && !record.selected_track &&
        !record.selected_keyframe;
}

bool SentinelStateUnchanged(const AnimationEditorStatePlaybackRow &row) {
    return row.state_id == 9006U && row.event_marker_count == 55U && !row.active;
}

bool SentinelEventUnchanged(const AnimationEditorEventMarkerRow &row) {
    return row.event_id == 9007U && row.payload_id == 9008U &&
        !row.visible_on_timeline;
}

int AnimationEditorSurfaceBuildsClipTrackTimelineFromRuntimeRecords() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord()};
    const std::array<AnimationRuntimeTrackRecord, 2U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U),
        TrackRecord(AnimationRuntimeChannel::RotationY, 2U)};
    const std::array<AnimationRuntimeKeyframeRecord, 4U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F),
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 1.0F)};
    const std::array<PreviewHostTransformFeedback, 0U> preview_feedback{};
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{};
    std::array<AnimationEditorTimelineTrackRow, 2U> track_rows{};
    std::array<AnimationEditorTimelineKeyframeMarker, 4U> keyframe_markers{};
    std::array<AnimationEditorPreviewFeedbackRecord, 0U> preview_output{};

    AnimationEditorTimelineSurfaceResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorTimelineSurface(
            SurfaceRequest(
                clips,
                tracks,
                keyframes,
                preview_feedback,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output,
                false),
            &result);
    if (status != AnimationEditorSurfaceStatus::Success || !result.Succeeded()) {
        return Fail("animation editor timeline surface failed");
    }

    if (result.clip_row_count != 1U || result.track_row_count != 2U ||
        result.keyframe_marker_count != 4U) {
        return Fail("animation editor timeline counters mismatch");
    }

    if (!result.consumed_runtime_records || !result.sampled_runtime_values ||
        !result.built_timeline_rows) {
        return Fail("animation editor timeline flags mismatch");
    }

    if (clip_rows[0U].clip_id != CLIP_ID || clip_rows[0U].track_count != 2U ||
        !clip_rows[0U].selected || !clip_rows[0U].runtime_valid) {
        return Fail("animation editor clip row mismatch");
    }

    if (track_rows[0U].track_id != TRACK_ID ||
        track_rows[0U].channel != AnimationRuntimeChannel::TranslationX ||
        track_rows[1U].channel != AnimationRuntimeChannel::RotationY) {
        return Fail("animation editor track row mismatch");
    }

    if (keyframe_markers[3U].track_id != TRACK_ID + 2U ||
        !Approx(keyframe_markers[3U].value, 1.0F)) {
        return Fail("animation editor keyframe marker mismatch");
    }

    if (result.mutated_runtime_data || result.opened_native_window ||
        result.used_deprecated_timeline) {
        return Fail("animation editor boundary flags changed");
    }

    return 0;
}

int AnimationEditorSurfaceConsumesPreviewHostTransformFeedbackForScrub() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord()};
    const std::array<AnimationRuntimeTrackRecord, 2U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U),
        TrackRecord(AnimationRuntimeChannel::RotationY, 2U)};
    const std::array<AnimationRuntimeKeyframeRecord, 4U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F),
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 1.0F)};
    const std::array<PreviewHostTransformFeedback, 1U> preview_feedback{
        PreviewFeedback()};
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{};
    std::array<AnimationEditorTimelineTrackRow, 2U> track_rows{};
    std::array<AnimationEditorTimelineKeyframeMarker, 4U> keyframe_markers{};
    std::array<AnimationEditorPreviewFeedbackRecord, 2U> preview_output{};

    AnimationEditorTimelineSurfaceResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorTimelineSurface(
            SurfaceRequest(
                clips,
                tracks,
                keyframes,
                preview_feedback,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output),
            &result);
    if (status != AnimationEditorSurfaceStatus::Success) {
        return Fail("animation editor preview feedback surface failed");
    }

    if (!result.consumed_preview_host_feedback || !result.emitted_preview_feedback ||
        result.preview_feedback_count != 2U) {
        return Fail("animation editor preview feedback counters mismatch");
    }

    if (!Approx(preview_output[0U].sample_time_seconds, 0.5F) ||
        !Approx(preview_output[0U].sampled_value, 5.0F) ||
        !Approx(preview_output[1U].sampled_value, 0.5F)) {
        return Fail("animation editor sampled preview values mismatch");
    }

    if (!preview_output[0U].matched_preview_host_feedback ||
        !preview_output[1U].feedback_from_preview_host) {
        return Fail("animation editor preview feedback flags mismatch");
    }

    if (!Approx(preview_output[0U].preview_transform.translation_x, 5.0F) ||
        !Approx(preview_output[1U].preview_transform.rotation_y, 0.5F)) {
        return Fail("animation editor preview transform mismatch");
    }

    return 0;
}

int AnimationEditorSurfaceRejectsMissingPreviewFeedbackWithoutMutation() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord(1U)};
    const std::array<AnimationRuntimeTrackRecord, 1U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U)};
    const std::array<AnimationRuntimeKeyframeRecord, 2U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F)};
    const std::array<PreviewHostTransformFeedback, 0U> preview_feedback{};
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{
        AnimationEditorTimelineClipRow{9001U, 0.0F, 0U, 77U, 0U, false, false}};
    std::array<AnimationEditorTimelineTrackRow, 1U> track_rows{};
    track_rows[0U].track_id = 9002U;
    track_rows[0U].keyframe_count = 88U;
    track_rows[0U].active = false;
    std::array<AnimationEditorTimelineKeyframeMarker, 2U> keyframe_markers{};
    keyframe_markers[0U].track_id = 9003U;
    keyframe_markers[0U].keyframe_index = 99U;
    keyframe_markers[0U].valid = false;
    std::array<AnimationEditorPreviewFeedbackRecord, 1U> preview_output{};
    preview_output[0U].track_id = 9004U;
    preview_output[0U].matched_preview_host_feedback = false;

    AnimationEditorTimelineSurfaceResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorTimelineSurface(
            SurfaceRequest(
                clips,
                tracks,
                keyframes,
                preview_feedback,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output),
            &result);
    if (status != AnimationEditorSurfaceStatus::PreviewFeedbackMissing) {
        return Fail("animation editor missing preview feedback status mismatch");
    }

    if (result.blocked_layer != AnimationEditorSurfaceBlockedLayer::PreviewHostFeedback) {
        return Fail("animation editor missing preview feedback layer mismatch");
    }

    if (!SentinelClipUnchanged(clip_rows[0U]) ||
        !SentinelTrackUnchanged(track_rows[0U]) ||
        !SentinelKeyframeUnchanged(keyframe_markers[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U])) {
        return Fail("animation editor missing preview feedback mutated output");
    }

    return 0;
}

int AnimationEditorWorkflowScrubUpdatesSampleTimeAndSelectedFeedback() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord()};
    const std::array<AnimationRuntimeTrackRecord, 2U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U),
        TrackRecord(AnimationRuntimeChannel::RotationY, 2U)};
    const std::array<AnimationRuntimeKeyframeRecord, 4U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F),
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 1.0F)};
    const std::array<PreviewHostTransformFeedback, 1U> preview_feedback{
        PreviewFeedback()};
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{};
    std::array<AnimationEditorTimelineTrackRow, 2U> track_rows{};
    std::array<AnimationEditorTimelineKeyframeMarker, 4U> keyframe_markers{};
    std::array<AnimationEditorPreviewFeedbackRecord, 2U> preview_output{};
    std::array<AnimationEditorTimelineSelectionFeedbackRecord, 1U> selection_output{};

    AnimationEditorTimelineWorkflowResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorTimelineWorkflow(
            WorkflowRequest(
                clips,
                tracks,
                keyframes,
                preview_feedback,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output,
                selection_output,
                FrameContextAt(HALF_SECOND_NANOSECONDS),
                AnimationEditorTimelineWorkflowCommand::Scrub,
                0.0F,
                0.25F,
                TRACK_ID,
                0U),
            &result);
    if (status != AnimationEditorSurfaceStatus::Success || !result.Succeeded()) {
        return Fail("animation editor workflow scrub failed");
    }

    if (!result.scrub_applied || result.playback_tick_applied ||
        !result.updated_sample_time || !Approx(result.sample_time_seconds, 0.25F)) {
        return Fail("animation editor workflow scrub state mismatch");
    }

    if (!track_rows[0U].selected || !track_rows[0U].sampled ||
        !track_rows[0U].preview_feedback_visible ||
        !Approx(track_rows[0U].sample_time_seconds, 0.25F) ||
        !Approx(track_rows[0U].sampled_value, 2.5F)) {
        return Fail("animation editor workflow scrub track row mismatch");
    }

    if (!keyframe_markers[0U].selected ||
        !keyframe_markers[0U].at_or_before_sample_time) {
        return Fail("animation editor workflow scrub key marker mismatch");
    }

    if (!selection_output[0U].selected_track ||
        !selection_output[0U].selected_keyframe ||
        selection_output[0U].track_id != TRACK_ID ||
        !Approx(selection_output[0U].sampled_value, 2.5F) ||
        !Approx(selection_output[0U].preview_transform.translation_x, 5.0F)) {
        return Fail("animation editor workflow scrub selection feedback mismatch");
    }

    return 0;
}

int AnimationEditorWorkflowPlaybackTickAdvancesSampleTimeAndTrackRowState() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord()};
    const std::array<AnimationRuntimeTrackRecord, 2U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U),
        TrackRecord(AnimationRuntimeChannel::RotationY, 2U)};
    const std::array<AnimationRuntimeKeyframeRecord, 4U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F),
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 1.0F)};
    const std::array<PreviewHostTransformFeedback, 1U> preview_feedback{
        PreviewFeedback()};
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{};
    std::array<AnimationEditorTimelineTrackRow, 2U> track_rows{};
    std::array<AnimationEditorTimelineKeyframeMarker, 4U> keyframe_markers{};
    std::array<AnimationEditorPreviewFeedbackRecord, 2U> preview_output{};
    std::array<AnimationEditorTimelineSelectionFeedbackRecord, 1U> selection_output{};

    AnimationEditorTimelineWorkflowResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorTimelineWorkflow(
            WorkflowRequest(
                clips,
                tracks,
                keyframes,
                preview_feedback,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output,
                selection_output,
                FrameContextAt(ONE_SECOND_NANOSECONDS, HALF_SECOND_NANOSECONDS),
                AnimationEditorTimelineWorkflowCommand::PlaybackTick,
                0.25F,
                0.0F,
                TRACK_ID + 2U,
                2U),
            &result);
    if (status != AnimationEditorSurfaceStatus::Success) {
        return Fail("animation editor workflow playback failed");
    }

    if (!result.playback_tick_applied || result.scrub_applied ||
        !result.updated_track_row_state || !Approx(result.sample_time_seconds, 0.75F)) {
        return Fail("animation editor workflow playback state mismatch");
    }

    if (track_rows[0U].selected || !track_rows[1U].selected ||
        !track_rows[1U].sampled ||
        !Approx(track_rows[1U].sample_time_seconds, 0.75F) ||
        !Approx(track_rows[1U].sampled_value, 0.75F)) {
        return Fail("animation editor workflow playback track row mismatch");
    }

    if (!keyframe_markers[2U].selected ||
        !keyframe_markers[2U].at_or_before_sample_time ||
        selection_output[0U].track_id != TRACK_ID + 2U ||
        !Approx(selection_output[0U].sample_time_seconds, 0.75F)) {
        return Fail("animation editor workflow playback selection mismatch");
    }

    return 0;
}

int AnimationEditorWorkflowEmitsSelectedTrackAndKeyFeedback() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord(1U)};
    const std::array<AnimationRuntimeTrackRecord, 1U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U)};
    const std::array<AnimationRuntimeKeyframeRecord, 2U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F)};
    const std::array<PreviewHostTransformFeedback, 1U> preview_feedback{
        PreviewFeedback()};
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{};
    std::array<AnimationEditorTimelineTrackRow, 1U> track_rows{};
    std::array<AnimationEditorTimelineKeyframeMarker, 2U> keyframe_markers{};
    std::array<AnimationEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<AnimationEditorTimelineSelectionFeedbackRecord, 1U> selection_output{};

    AnimationEditorTimelineWorkflowResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorTimelineWorkflow(
            WorkflowRequest(
                clips,
                tracks,
                keyframes,
                preview_feedback,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output,
                selection_output,
                FrameContextAt(HALF_SECOND_NANOSECONDS),
                AnimationEditorTimelineWorkflowCommand::Scrub,
                0.0F,
                0.5F,
                TRACK_ID,
                1U),
            &result);
    if (status != AnimationEditorSurfaceStatus::Success) {
        return Fail("animation editor workflow selected feedback failed");
    }

    if (!result.emitted_selected_track_feedback ||
        !result.emitted_selected_key_feedback ||
        result.selection_feedback_count != 1U) {
        return Fail("animation editor workflow selected feedback flags mismatch");
    }

    if (!selection_output[0U].selected_track ||
        !selection_output[0U].selected_keyframe ||
        selection_output[0U].keyframe_index != 1U ||
        !Approx(selection_output[0U].keyframe_time_seconds, 1.0F) ||
        !Approx(selection_output[0U].keyframe_value, 10.0F) ||
        !selection_output[0U].matched_preview_host_feedback) {
        return Fail("animation editor workflow selected feedback record mismatch");
    }

    if (!keyframe_markers[1U].selected ||
        keyframe_markers[1U].at_or_before_sample_time) {
        return Fail("animation editor workflow selected key marker mismatch");
    }

    return 0;
}

int AnimationEditorWorkflowMissingPreviewFeedbackDoesNotMutateOutputs() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord(1U)};
    const std::array<AnimationRuntimeTrackRecord, 1U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U)};
    const std::array<AnimationRuntimeKeyframeRecord, 2U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F)};
    const std::array<PreviewHostTransformFeedback, 0U> preview_feedback{};
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{
        AnimationEditorTimelineClipRow{9001U, 0.0F, 0U, 77U, 0U, false, false}};
    std::array<AnimationEditorTimelineTrackRow, 1U> track_rows{};
    track_rows[0U].track_id = 9002U;
    track_rows[0U].keyframe_count = 88U;
    track_rows[0U].active = false;
    std::array<AnimationEditorTimelineKeyframeMarker, 2U> keyframe_markers{};
    keyframe_markers[0U].track_id = 9003U;
    keyframe_markers[0U].keyframe_index = 99U;
    keyframe_markers[0U].valid = false;
    std::array<AnimationEditorPreviewFeedbackRecord, 1U> preview_output{};
    preview_output[0U].track_id = 9004U;
    preview_output[0U].matched_preview_host_feedback = false;
    std::array<AnimationEditorTimelineSelectionFeedbackRecord, 1U> selection_output{};
    selection_output[0U].track_id = 9005U;
    selection_output[0U].selected_track = false;
    selection_output[0U].selected_keyframe = false;

    AnimationEditorTimelineWorkflowResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorTimelineWorkflow(
            WorkflowRequest(
                clips,
                tracks,
                keyframes,
                preview_feedback,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output,
                selection_output,
                FrameContextAt(HALF_SECOND_NANOSECONDS),
                AnimationEditorTimelineWorkflowCommand::Scrub,
                0.0F,
                0.5F,
                TRACK_ID,
                0U),
            &result);
    if (status != AnimationEditorSurfaceStatus::PreviewFeedbackMissing) {
        return Fail("animation editor workflow missing preview status mismatch");
    }

    if (result.blocked_layer != AnimationEditorSurfaceBlockedLayer::PreviewHostFeedback) {
        return Fail("animation editor workflow missing preview layer mismatch");
    }

    if (!SentinelClipUnchanged(clip_rows[0U]) ||
        !SentinelTrackUnchanged(track_rows[0U]) ||
        !SentinelKeyframeUnchanged(keyframe_markers[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelSelectionUnchanged(selection_output[0U])) {
        return Fail("animation editor workflow missing preview mutated output");
    }

    return 0;
}

int AnimationEditorWorkflowOutputCapacityFailureDoesNotMutateOutputs() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord()};
    const std::array<AnimationRuntimeTrackRecord, 2U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U),
        TrackRecord(AnimationRuntimeChannel::RotationY, 2U)};
    const std::array<AnimationRuntimeKeyframeRecord, 4U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F),
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 1.0F)};
    const std::array<PreviewHostTransformFeedback, 1U> preview_feedback{
        PreviewFeedback()};
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{
        AnimationEditorTimelineClipRow{9001U, 0.0F, 0U, 77U, 0U, false, false}};
    std::array<AnimationEditorTimelineTrackRow, 1U> track_rows{};
    track_rows[0U].track_id = 9002U;
    track_rows[0U].keyframe_count = 88U;
    track_rows[0U].active = false;
    std::array<AnimationEditorTimelineKeyframeMarker, 4U> keyframe_markers{};
    keyframe_markers[0U].track_id = 9003U;
    keyframe_markers[0U].keyframe_index = 99U;
    keyframe_markers[0U].valid = false;
    std::array<AnimationEditorPreviewFeedbackRecord, 2U> preview_output{};
    preview_output[0U].track_id = 9004U;
    preview_output[0U].matched_preview_host_feedback = false;
    std::array<AnimationEditorTimelineSelectionFeedbackRecord, 1U> selection_output{};
    selection_output[0U].track_id = 9005U;
    selection_output[0U].selected_track = false;
    selection_output[0U].selected_keyframe = false;

    AnimationEditorTimelineWorkflowResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorTimelineWorkflow(
            WorkflowRequest(
                clips,
                tracks,
                keyframes,
                preview_feedback,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output,
                selection_output,
                FrameContextAt(HALF_SECOND_NANOSECONDS),
                AnimationEditorTimelineWorkflowCommand::Scrub,
                0.0F,
                0.5F,
                TRACK_ID,
                0U),
            &result);
    if (status != AnimationEditorSurfaceStatus::OutputCapacityExceeded) {
        return Fail("animation editor workflow output capacity status mismatch");
    }

    if (result.blocked_layer != AnimationEditorSurfaceBlockedLayer::TimelineOutput) {
        return Fail("animation editor workflow output capacity layer mismatch");
    }

    if (!SentinelClipUnchanged(clip_rows[0U]) ||
        !SentinelTrackUnchanged(track_rows[0U]) ||
        !SentinelKeyframeUnchanged(keyframe_markers[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelSelectionUnchanged(selection_output[0U])) {
        return Fail("animation editor workflow output capacity mutated output");
    }

    return 0;
}

int AnimationEditorStateEventWorkflowBindsVisiblePlaybackStateAndEvents() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord()};
    const std::array<AnimationRuntimeTrackRecord, 2U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U),
        TrackRecord(AnimationRuntimeChannel::RotationY, 2U)};
    const std::array<AnimationRuntimeKeyframeRecord, 4U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F),
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 1.0F)};
    const std::array<AnimationEditorStatePreviewRecord, 1U> states{StateRecord()};
    const std::array<AnimationEditorEventMarkerRecord, 2U> events{
        EventMarker(0.25F),
        EventMarker(0.75F, 1U)};
    const std::array<PreviewHostTransformFeedback, 1U> preview_feedback{
        PreviewFeedback()};
    std::array<AnimationEditorStatePlaybackRow, 1U> state_rows{};
    std::array<AnimationEditorEventMarkerRow, 2U> event_rows{};
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{};
    std::array<AnimationEditorTimelineTrackRow, 2U> track_rows{};
    std::array<AnimationEditorTimelineKeyframeMarker, 4U> keyframe_markers{};
    std::array<AnimationEditorPreviewFeedbackRecord, 2U> preview_output{};
    std::array<AnimationEditorTimelineSelectionFeedbackRecord, 1U> selection_output{};

    AnimationEditorStateEventPlaybackWorkflowResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorStateEventPlaybackWorkflow(
            StateEventWorkflowRequest(
                states,
                events,
                clips,
                tracks,
                keyframes,
                preview_feedback,
                state_rows,
                event_rows,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output,
                selection_output,
                FrameContextAt(ONE_SECOND_NANOSECONDS, HALF_SECOND_NANOSECONDS),
                AnimationEditorTimelineWorkflowCommand::PlaybackTick,
                0.25F,
                0.0F,
                TRACK_ID + 2U,
                2U),
            &result);
    if (status != AnimationEditorSurfaceStatus::Success || !result.Succeeded()) {
        return Fail("animation editor state event workflow failed");
    }

    if (!result.consumed_state_records || !result.bound_state_to_clip ||
        !result.consumed_event_records || !result.consumed_timeline_workflow ||
        !result.built_visible_playback_feedback ||
        result.used_gameplay_fsm || result.opened_native_window ||
        result.used_deprecated_timeline || result.mutated_runtime_data) {
        return Fail("animation editor state event workflow flags mismatch");
    }

    if (result.state_row_count != 1U || result.event_row_count != 2U ||
        result.emitted_event_count != 1U ||
        !Approx(result.sample_time_seconds, 0.75F) ||
        !Approx(result.event_window_start_seconds, 0.25F) ||
        !Approx(result.event_window_end_seconds, 0.75F)) {
        return Fail("animation editor state event workflow counters mismatch");
    }

    if (state_rows[0U].state_id != STATE_ID || state_rows[0U].clip_id != CLIP_ID ||
        !state_rows[0U].active || !state_rows[0U].selected ||
        !state_rows[0U].preview_feedback_visible ||
        state_rows[0U].event_marker_count != 2U ||
        state_rows[0U].emitted_event_count != 1U) {
        return Fail("animation editor state row mismatch");
    }

    if (!event_rows[0U].visible_on_timeline || event_rows[0U].emitted_this_frame ||
        !event_rows[1U].visible_on_timeline || !event_rows[1U].emitted_this_frame ||
        event_rows[1U].payload_id != 8002U ||
        !event_rows[1U].at_or_before_sample_time) {
        return Fail("animation editor event row mismatch");
    }

    if (!track_rows[1U].selected || !track_rows[1U].preview_feedback_visible ||
        !Approx(track_rows[1U].sampled_value, 0.75F) ||
        selection_output[0U].track_id != TRACK_ID + 2U ||
        !Approx(selection_output[0U].sample_time_seconds, 0.75F)) {
        return Fail("animation editor state event selected feedback mismatch");
    }

    return 0;
}

int AnimationEditorStateEventWorkflowPlaybackTickLoopsAndEmitsWrappedEvents() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord()};
    const std::array<AnimationRuntimeTrackRecord, 2U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U),
        TrackRecord(AnimationRuntimeChannel::RotationY, 2U)};
    const std::array<AnimationRuntimeKeyframeRecord, 4U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F),
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 1.0F)};
    const std::array<AnimationEditorStatePreviewRecord, 1U> states{
        StateRecord(true)};
    const std::array<AnimationEditorEventMarkerRecord, 2U> events{
        EventMarker(0.10F),
        EventMarker(0.90F, 1U)};
    const std::array<PreviewHostTransformFeedback, 1U> preview_feedback{
        PreviewFeedback()};
    std::array<AnimationEditorStatePlaybackRow, 1U> state_rows{};
    std::array<AnimationEditorEventMarkerRow, 2U> event_rows{};
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{};
    std::array<AnimationEditorTimelineTrackRow, 2U> track_rows{};
    std::array<AnimationEditorTimelineKeyframeMarker, 4U> keyframe_markers{};
    std::array<AnimationEditorPreviewFeedbackRecord, 2U> preview_output{};
    std::array<AnimationEditorTimelineSelectionFeedbackRecord, 1U> selection_output{};

    AnimationEditorStateEventPlaybackWorkflowResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorStateEventPlaybackWorkflow(
            StateEventWorkflowRequest(
                states,
                events,
                clips,
                tracks,
                keyframes,
                preview_feedback,
                state_rows,
                event_rows,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output,
                selection_output,
                FrameContextAt(ONE_SECOND_NANOSECONDS + QUARTER_SECOND_NANOSECONDS,
                    HALF_SECOND_NANOSECONDS),
                AnimationEditorTimelineWorkflowCommand::PlaybackTick,
                0.75F,
                0.0F,
                TRACK_ID,
                0U),
            &result);
    if (status != AnimationEditorSurfaceStatus::Success) {
        return Fail("animation editor state event loop workflow failed");
    }

    if (!result.event_window_wrapped || !state_rows[0U].loop_playback ||
        result.emitted_event_count != 2U ||
        !Approx(result.sample_time_seconds, 0.25F) ||
        !Approx(result.event_window_start_seconds, 0.75F) ||
        !Approx(result.event_window_end_seconds, 0.25F)) {
        return Fail("animation editor loop event window mismatch");
    }

    if (!event_rows[0U].emitted_this_frame ||
        !event_rows[1U].emitted_this_frame ||
        event_rows[1U].at_or_before_sample_time) {
        return Fail("animation editor wrapped event rows mismatch");
    }

    if (!track_rows[0U].selected ||
        !Approx(track_rows[0U].sample_time_seconds, 0.25F) ||
        !Approx(track_rows[0U].sampled_value, 2.5F)) {
        return Fail("animation editor loop selected track mismatch");
    }

    return 0;
}

int AnimationEditorStateEventWorkflowMissingPreviewFeedbackDoesNotMutateOutputs() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord(1U)};
    const std::array<AnimationRuntimeTrackRecord, 1U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U)};
    const std::array<AnimationRuntimeKeyframeRecord, 2U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F)};
    const std::array<AnimationEditorStatePreviewRecord, 1U> states{StateRecord()};
    const std::array<AnimationEditorEventMarkerRecord, 1U> events{EventMarker(0.50F)};
    const std::array<PreviewHostTransformFeedback, 0U> preview_feedback{};
    std::array<AnimationEditorStatePlaybackRow, 1U> state_rows{};
    state_rows[0U].state_id = 9006U;
    state_rows[0U].event_marker_count = 55U;
    state_rows[0U].active = false;
    std::array<AnimationEditorEventMarkerRow, 1U> event_rows{};
    event_rows[0U].event_id = 9007U;
    event_rows[0U].payload_id = 9008U;
    event_rows[0U].visible_on_timeline = false;
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{
        AnimationEditorTimelineClipRow{9001U, 0.0F, 0U, 77U, 0U, false, false}};
    std::array<AnimationEditorTimelineTrackRow, 1U> track_rows{};
    track_rows[0U].track_id = 9002U;
    track_rows[0U].keyframe_count = 88U;
    track_rows[0U].active = false;
    std::array<AnimationEditorTimelineKeyframeMarker, 2U> keyframe_markers{};
    keyframe_markers[0U].track_id = 9003U;
    keyframe_markers[0U].keyframe_index = 99U;
    keyframe_markers[0U].valid = false;
    std::array<AnimationEditorPreviewFeedbackRecord, 1U> preview_output{};
    preview_output[0U].track_id = 9004U;
    preview_output[0U].matched_preview_host_feedback = false;
    std::array<AnimationEditorTimelineSelectionFeedbackRecord, 1U> selection_output{};
    selection_output[0U].track_id = 9005U;
    selection_output[0U].selected_track = false;
    selection_output[0U].selected_keyframe = false;

    AnimationEditorStateEventPlaybackWorkflowResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorStateEventPlaybackWorkflow(
            StateEventWorkflowRequest(
                states,
                events,
                clips,
                tracks,
                keyframes,
                preview_feedback,
                state_rows,
                event_rows,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output,
                selection_output,
                FrameContextAt(HALF_SECOND_NANOSECONDS),
                AnimationEditorTimelineWorkflowCommand::Scrub,
                0.0F,
                0.5F,
                TRACK_ID,
                0U),
            &result);
    if (status != AnimationEditorSurfaceStatus::PreviewFeedbackMissing) {
        return Fail("animation editor state event missing preview status mismatch");
    }

    if (result.blocked_layer != AnimationEditorSurfaceBlockedLayer::PreviewHostFeedback ||
        !SentinelStateUnchanged(state_rows[0U]) ||
        !SentinelEventUnchanged(event_rows[0U]) ||
        !SentinelClipUnchanged(clip_rows[0U]) ||
        !SentinelTrackUnchanged(track_rows[0U]) ||
        !SentinelKeyframeUnchanged(keyframe_markers[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelSelectionUnchanged(selection_output[0U])) {
        return Fail("animation editor state event missing preview mutated output");
    }

    return 0;
}

int AnimationEditorStateEventWorkflowInvalidEventDoesNotMutateOutputs() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord(1U)};
    const std::array<AnimationRuntimeTrackRecord, 1U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U)};
    const std::array<AnimationRuntimeKeyframeRecord, 2U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F)};
    const std::array<AnimationEditorStatePreviewRecord, 1U> states{StateRecord()};
    std::array<AnimationEditorEventMarkerRecord, 1U> events{EventMarker(1.50F)};
    const std::array<PreviewHostTransformFeedback, 1U> preview_feedback{
        PreviewFeedback()};
    std::array<AnimationEditorStatePlaybackRow, 1U> state_rows{};
    state_rows[0U].state_id = 9006U;
    state_rows[0U].event_marker_count = 55U;
    state_rows[0U].active = false;
    std::array<AnimationEditorEventMarkerRow, 1U> event_rows{};
    event_rows[0U].event_id = 9007U;
    event_rows[0U].payload_id = 9008U;
    event_rows[0U].visible_on_timeline = false;
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{
        AnimationEditorTimelineClipRow{9001U, 0.0F, 0U, 77U, 0U, false, false}};
    std::array<AnimationEditorTimelineTrackRow, 1U> track_rows{};
    track_rows[0U].track_id = 9002U;
    track_rows[0U].keyframe_count = 88U;
    track_rows[0U].active = false;
    std::array<AnimationEditorTimelineKeyframeMarker, 2U> keyframe_markers{};
    keyframe_markers[0U].track_id = 9003U;
    keyframe_markers[0U].keyframe_index = 99U;
    keyframe_markers[0U].valid = false;
    std::array<AnimationEditorPreviewFeedbackRecord, 1U> preview_output{};
    preview_output[0U].track_id = 9004U;
    preview_output[0U].matched_preview_host_feedback = false;
    std::array<AnimationEditorTimelineSelectionFeedbackRecord, 1U> selection_output{};
    selection_output[0U].track_id = 9005U;
    selection_output[0U].selected_track = false;
    selection_output[0U].selected_keyframe = false;

    AnimationEditorStateEventPlaybackWorkflowResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorStateEventPlaybackWorkflow(
            StateEventWorkflowRequest(
                states,
                events,
                clips,
                tracks,
                keyframes,
                preview_feedback,
                state_rows,
                event_rows,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output,
                selection_output,
                FrameContextAt(HALF_SECOND_NANOSECONDS),
                AnimationEditorTimelineWorkflowCommand::Scrub,
                0.0F,
                0.5F,
                TRACK_ID,
                0U),
            &result);
    if (status != AnimationEditorSurfaceStatus::InvalidEvent) {
        return Fail("animation editor state event invalid event status mismatch");
    }

    if (result.blocked_layer != AnimationEditorSurfaceBlockedLayer::EventRecords ||
        !SentinelStateUnchanged(state_rows[0U]) ||
        !SentinelEventUnchanged(event_rows[0U]) ||
        !SentinelClipUnchanged(clip_rows[0U]) ||
        !SentinelTrackUnchanged(track_rows[0U]) ||
        !SentinelKeyframeUnchanged(keyframe_markers[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelSelectionUnchanged(selection_output[0U])) {
        return Fail("animation editor state event invalid event mutated output");
    }

    return 0;
}

int AnimationEditorStateEventWorkflowOutputCapacityFailureDoesNotMutateOutputs() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord(1U)};
    const std::array<AnimationRuntimeTrackRecord, 1U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U)};
    const std::array<AnimationRuntimeKeyframeRecord, 2U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F)};
    const std::array<AnimationEditorStatePreviewRecord, 1U> states{StateRecord()};
    const std::array<AnimationEditorEventMarkerRecord, 2U> events{
        EventMarker(0.25F),
        EventMarker(0.50F, 1U)};
    const std::array<PreviewHostTransformFeedback, 1U> preview_feedback{
        PreviewFeedback()};
    std::array<AnimationEditorStatePlaybackRow, 1U> state_rows{};
    state_rows[0U].state_id = 9006U;
    state_rows[0U].event_marker_count = 55U;
    state_rows[0U].active = false;
    std::array<AnimationEditorEventMarkerRow, 1U> event_rows{};
    event_rows[0U].event_id = 9007U;
    event_rows[0U].payload_id = 9008U;
    event_rows[0U].visible_on_timeline = false;
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{
        AnimationEditorTimelineClipRow{9001U, 0.0F, 0U, 77U, 0U, false, false}};
    std::array<AnimationEditorTimelineTrackRow, 1U> track_rows{};
    track_rows[0U].track_id = 9002U;
    track_rows[0U].keyframe_count = 88U;
    track_rows[0U].active = false;
    std::array<AnimationEditorTimelineKeyframeMarker, 2U> keyframe_markers{};
    keyframe_markers[0U].track_id = 9003U;
    keyframe_markers[0U].keyframe_index = 99U;
    keyframe_markers[0U].valid = false;
    std::array<AnimationEditorPreviewFeedbackRecord, 1U> preview_output{};
    preview_output[0U].track_id = 9004U;
    preview_output[0U].matched_preview_host_feedback = false;
    std::array<AnimationEditorTimelineSelectionFeedbackRecord, 1U> selection_output{};
    selection_output[0U].track_id = 9005U;
    selection_output[0U].selected_track = false;
    selection_output[0U].selected_keyframe = false;

    AnimationEditorStateEventPlaybackWorkflowResult result{};
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorStateEventPlaybackWorkflow(
            StateEventWorkflowRequest(
                states,
                events,
                clips,
                tracks,
                keyframes,
                preview_feedback,
                state_rows,
                event_rows,
                clip_rows,
                track_rows,
                keyframe_markers,
                preview_output,
                selection_output,
                FrameContextAt(HALF_SECOND_NANOSECONDS),
                AnimationEditorTimelineWorkflowCommand::Scrub,
                0.0F,
                0.5F,
                TRACK_ID,
                0U),
            &result);
    if (status != AnimationEditorSurfaceStatus::OutputCapacityExceeded) {
        return Fail("animation editor state event output capacity status mismatch");
    }

    if (result.blocked_layer != AnimationEditorSurfaceBlockedLayer::EventRecords ||
        !SentinelStateUnchanged(state_rows[0U]) ||
        !SentinelEventUnchanged(event_rows[0U]) ||
        !SentinelClipUnchanged(clip_rows[0U]) ||
        !SentinelTrackUnchanged(track_rows[0U]) ||
        !SentinelKeyframeUnchanged(keyframe_markers[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelSelectionUnchanged(selection_output[0U])) {
        return Fail("animation editor state event output capacity mutated output");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_TIMELINE) {
        return AnimationEditorSurfaceBuildsClipTrackTimelineFromRuntimeRecords();
    }

    if (name == TEST_PREVIEW) {
        return AnimationEditorSurfaceConsumesPreviewHostTransformFeedbackForScrub();
    }

    if (name == TEST_NO_MUTATION) {
        return AnimationEditorSurfaceRejectsMissingPreviewFeedbackWithoutMutation();
    }

    if (name == TEST_WORKFLOW_SCRUB) {
        return AnimationEditorWorkflowScrubUpdatesSampleTimeAndSelectedFeedback();
    }

    if (name == TEST_WORKFLOW_PLAYBACK) {
        return AnimationEditorWorkflowPlaybackTickAdvancesSampleTimeAndTrackRowState();
    }

    if (name == TEST_WORKFLOW_SELECTION) {
        return AnimationEditorWorkflowEmitsSelectedTrackAndKeyFeedback();
    }

    if (name == TEST_WORKFLOW_MISSING_PREVIEW) {
        return AnimationEditorWorkflowMissingPreviewFeedbackDoesNotMutateOutputs();
    }

    if (name == TEST_WORKFLOW_SMALL_OUTPUT) {
        return AnimationEditorWorkflowOutputCapacityFailureDoesNotMutateOutputs();
    }

    if (name == TEST_STATE_EVENT_VISIBLE) {
        return AnimationEditorStateEventWorkflowBindsVisiblePlaybackStateAndEvents();
    }

    if (name == TEST_STATE_EVENT_LOOP) {
        return AnimationEditorStateEventWorkflowPlaybackTickLoopsAndEmitsWrappedEvents();
    }

    if (name == TEST_STATE_EVENT_MISSING_PREVIEW) {
        return AnimationEditorStateEventWorkflowMissingPreviewFeedbackDoesNotMutateOutputs();
    }

    if (name == TEST_STATE_EVENT_INVALID_EVENT) {
        return AnimationEditorStateEventWorkflowInvalidEventDoesNotMutateOutputs();
    }

    if (name == TEST_STATE_EVENT_SMALL_OUTPUT) {
        return AnimationEditorStateEventWorkflowOutputCapacityFailureDoesNotMutateOutputs();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
