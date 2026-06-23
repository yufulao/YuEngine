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
using yuengine::animationeditor::AnimationEditorSurfaceBlockedLayer;
using yuengine::animationeditor::AnimationEditorSurfaceStatus;
using yuengine::animationeditor::AnimationEditorTimelineClipRow;
using yuengine::animationeditor::AnimationEditorTimelineKeyframeMarker;
using yuengine::animationeditor::AnimationEditorTimelineSurfaceRequest;
using yuengine::animationeditor::AnimationEditorTimelineSurfaceResult;
using yuengine::animationeditor::AnimationEditorTimelineTrackRow;
using yuengine::animationeditor::BuildAnimationEditorTimelineSurface;
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
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t CLIP_ID = 4101U;
constexpr std::uint32_t TRACK_ID = 4201U;
constexpr std::uint32_t WORLD_OBJECT_VALUE = 4301U;
constexpr std::uint64_t HALF_SECOND_NANOSECONDS = 500000000ULL;
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

RuntimeFrameContext FrameContext() {
    RuntimeFrameContext context{};
    context.frame_index = 11U;
    context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    context.fixed_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    context.frame_mode = RuntimeFrameMode::Fixed;
    return context;
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
        result.used_web_timeline) {
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

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
